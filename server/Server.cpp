//
// Server.cpp
//

#include "Server.hpp"
#include "version.hpp"
#include <algorithm>
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include "../common/Logger.hpp"
#include "../common/network/Command.hpp"
#include "../common/network/Utils.hpp"

namespace network {

    Server::Server(const Config& config) :
			config_(config),
            endpoint_(tcp::v4(), config.port()),
            acceptor_(io_service_, endpoint_),
            socket_udp_(io_service_, udp::endpoint(udp::v4(), config.port())),
            udp_packet_count_(0)
    {
    }

    void Server::Start(CallbackFuncPtr callback)
    {
        callback_ = std::make_shared<CallbackFunc>(
                [&](network::Command c){

            // ログアウト
            if (c.header() == network::header::FatalConnectionError) {
                if (callback) {
					(*callback)(c);
				}
            } else if (auto session = c.session().lock()) {
				auto read_average = session->GetReadByteAverage();
				if (read_average > config_.receive_limit_2()) {
					Logger::Info(_T("Banished session: %d"), session->id());
					session->Close();
				} else if(read_average > config_.receive_limit_1()) {
					Logger::Info(_T("Receive limit exceeded: %d: %d byte/s"), session->id(), read_average);
				} else {
					if (callback) {
						(*callback)(c);
					}
                }
            }

        });

        {
        auto new_session = boost::make_shared<ServerSession>(io_service_);
        acceptor_.async_accept(new_session->tcp_socket(),
                              boost::bind(&Server::ReceiveSession, this, new_session, boost::asio::placeholders::error));
        }

        {
            socket_udp_.async_receive_from(
                boost::asio::buffer(receive_buf_udp_, UDP_MAX_RECEIVE_LENGTH), sender_endpoint_,
                boost::bind(&Server::ReceiveUDP, this,
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
        }

        boost::asio::io_service::work work(io_service_);
        io_service_.run();
    }

    void Server::Stop()
    {
        io_service_.stop();
		Logger::Info("stop server");
    }
    void Server::Stop(int innterrupt_type)
    {
        io_service_.stop();
	Logger::Info(_T("stop server innterrupt_type=%d"),innterrupt_type);
    }

	int Server::GetUserCount() const
	{
		auto count = std::count_if(sessions_.begin(), sessions_.end(),
			[](const SessionWeakPtr& s){ return !s.expired() && s.lock()->online(); });

		return count;
	}

	std::string Server::GetStatusJSON() const
	{
		auto msg = (
					boost::format("{\"nam\":\"%s\",\"ver\":\"%d.%d.%d\",\"cnt\":%d,\"cap\":%d,\"stg\":\"%s\"}")
						% config_.server_name()
						% MMO_VERSION_MAJOR % MMO_VERSION_MINOR % MMO_VERSION_REVISION
						% GetUserCount()
						% config_.capacity()
						% config_.stage()
					).str();

		return msg;
	}

    bool Server::Empty() const
    {
        return GetUserCount() == 0;
    }

	bool Server::IsBlockedAddress(const boost::asio::ip::address& address)
	{
		BOOST_FOREACH(const auto& pattern, config_.blocking_address_patterns()) {
			if (network::Utils::MatchWithWildcard(pattern, address.to_string())) {
				return true;
			}
		}
		return false;
	}

    void Server::ReceiveSession(const SessionPtr& session, const boost::system::error_code& error)
    {
		const auto address = session->tcp_socket().remote_endpoint().address();
		if(IsBlockedAddress(address)) {
			Logger::Info("Blocked IP Address: %s", address);
            session->Close();

		} else if (GetUserCount() >= config_.capacity()) {
			Logger::Info("Refused Session");
            session->SyncSend(ClientReceiveServerCrowdedError());
            session->Close();

        } else {
            session->set_on_receive(callback_);
            session->Start();
            sessions_.push_back(SessionWeakPtr(session));

            // クライアント情報を要求
            session->Send(ClientRequestedClientInfo());
        }

        auto new_session = boost::make_shared<ServerSession>(io_service_);
         acceptor_.async_accept(new_session->tcp_socket(),
                 boost::bind(&Server::ReceiveSession, this, new_session, boost::asio::placeholders::error));

		RefreshSession();
    }

	void Server::RefreshSession()
	{
		// 使用済のセッションのポインタを破棄
        auto it = std::remove_if(sessions_.begin(), sessions_.end(),
                [](const SessionWeakPtr& ptr){
            return ptr.expired();
        });
        sessions_.erase(it, sessions_.end());
		Logger::Info("Active connection: %d", GetUserCount());
	}

    void Server::SendAll(const Command& command, int channel, bool limited)
    {
        BOOST_FOREACH(SessionWeakPtr& ptr, sessions_) {
            if (auto session = ptr.lock()) {
				if (channel < 0 || (channel >= 0 && session->channel() == channel)) {
					if (!limited || session->write_average_limit() > session->GetWriteByteAverage()) {
						session->Send(command);
					}
				}
            }
        }
    }

    void Server::SendOthers(const Command& command, uint32_t self_id, int channel, bool limited)
    {
        BOOST_FOREACH(SessionWeakPtr& ptr, sessions_) {
            if (auto session = ptr.lock()) {
				if (channel < 0 || (channel >= 0 && session->channel() == channel)) {
					if (!limited || session->write_average_limit() > session->GetWriteByteAverage()) {
						if (session->id() != self_id) {
							session->Send(command);
						}
					}
				}
            }
        }
    }
	
    void Server::SendTo(const Command& command, uint32_t user_id)
	{
		auto it = std::find_if(sessions_.begin(), sessions_.end(),
			[user_id](SessionWeakPtr& ptr){
				return ptr.lock()->id() == user_id;
			});
		
		if (it != sessions_.end()) {
			it->lock()->Send(command);
		}
	}

    void Server::SendUDPTestPacket(const std::string& ip_address, uint16_t port)
    {
        using boost::asio::ip::udp;

        std::stringstream port_str;
        port_str << (int)port;

        udp::resolver resolver(io_service_);
        udp::resolver::query query(udp::v4(), ip_address.c_str(), port_str.str().c_str());
        udp::resolver::iterator iterator = resolver.resolve(query);

        static char request[] = "MMO UDP Test Packet";
        for (int i = 0; i < UDP_TEST_PACKET_TIME; i++) {

            io_service_.post(boost::bind(&Server::DoWriteUDP, this, request, *iterator));
        }
    }

    void Server::SendUDP(const std::string& message, const boost::asio::ip::udp::endpoint endpoint)
    {
		io_service_.post(boost::bind(&Server::DoWriteUDP, this, message, endpoint));
    }

    void Server::ReceiveUDP(const boost::system::error_code& error, size_t bytes_recvd)
    {
        if (bytes_recvd > 0) {
            std::string buffer(receive_buf_udp_, bytes_recvd);
            FetchUDP(buffer, sender_endpoint_);
        }
        if (!error) {
          socket_udp_.async_receive_from(
              boost::asio::buffer(receive_buf_udp_, UDP_MAX_RECEIVE_LENGTH), sender_endpoint_,
              boost::bind(&Server::ReceiveUDP, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
        } else {
            Logger::Error("%s", error.message());
        }
    }

    void Server::DoWriteUDP(const std::string& msg, const udp::endpoint& endpoint)
    {
        boost::shared_ptr<std::string> s = 
              boost::make_shared<std::string>(msg.data(), msg.size());

        socket_udp_.async_send_to(
            boost::asio::buffer(s->data(), s->size()), endpoint,
            boost::bind(&Server::WriteUDP, this,
              boost::asio::placeholders::error, s));
    }

    void Server::WriteUDP(const boost::system::error_code& error, boost::shared_ptr<std::string> holder)
    {
//        if (!error) {
//            if (!send_queue_.empty()) {
//                  send_queue_.pop();
//                  if (!send_queue_.empty())
//                  {
//                    boost::asio::async_write(socket_tcp_,
//                        boost::asio::buffer(send_queue_.front().data(),
//                          send_queue_.front().size()),
//                        boost::bind(&Session::WriteTCP, this,
//                          boost::asio::placeholders::error));
//                  }
//            }
//        } else {
//            FatalError();
//        }
    }

    void Server::FetchUDP(const std::string& buffer, const boost::asio::ip::udp::endpoint endpoint)
    {
        uint8_t header;
        std::string body;
        SessionWeakPtr session;

		// IPアドレスとポートからセッションを特定
		auto it = std::find_if(sessions_.begin(), sessions_.end(),
			[&endpoint](const SessionWeakPtr& session) -> bool {
				if (auto session_ptr = session.lock()) {

					const auto session_endpoint = session_ptr->tcp_socket().remote_endpoint();
					const auto session_port = session_ptr->udp_port();

					return (session_endpoint.address() == endpoint.address() &&
						session_port == endpoint.port());

				} else {
					return false;
				}
			});

		if (it != sessions_.end()) {
			session = *it;
			Logger::Debug("Receive UDP Command: %d", session.lock()->id());
		} else {
			Logger::Debug("Receive anonymous UDP Command");
		}

        if (buffer.size() > network::Utils::Deserialize(buffer, &header)) {
			body = buffer.substr(sizeof(header));
		}

        // 復号
        if (session.lock() && header == header::ENCRYPT_HEADER) {
            body.erase(0, sizeof(header));
			body = session.lock()->encrypter().Decrypt(body);
            Utils::Deserialize(body, &header);
			body = buffer.substr(sizeof(header));
        }

		if (header == network::header::ServerRequstedStatus) {
			SendUDP(GetStatusJSON(), endpoint);
		} else {
			if (callback_) {
				(*callback_)(Command(static_cast<network::header::CommandHeader>(header), body, session));
			}
		}

    }

    void Server::ServerSession::Start()
    {
        online_ = true;

        // Nagleアルゴリズムを無効化
        socket_tcp_.set_option(boost::asio::ip::tcp::no_delay(true));

		// バッファサイズを変更 1MiB
		boost::asio::socket_base::receive_buffer_size option(1048576);
		socket_tcp_.set_option(option);

        // IPアドレスを取得
        global_ip_ = socket_tcp_.remote_endpoint().address().to_string();

        boost::asio::async_read_until(socket_tcp_,
            receive_buf_, NETWORK_UTILS_DELIMITOR,
            boost::bind(
              &ServerSession::ReceiveTCP, shared_from_this(),
              boost::asio::placeholders::error));
    }
}
