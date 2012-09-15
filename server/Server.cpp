//
// Server.cpp
//

#include "Server.hpp"
#include <algorithm>
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include "../common/Logger.hpp"
#include "../common/network/Command.hpp"
#include "../common/network/Utils.hpp"

namespace network {

    Server::Server(uint16_t port) :
            endpoint_(tcp::v4(), port),
            acceptor_(io_service_, endpoint_),
            socket_udp_(io_service_, udp::endpoint(udp::v4(), port)),
            udp_packet_count_(0),
            max_total_read_average_(5000),
            max_session_read_average_(600),
            min_session_read_average_(100),
            session_read_average_(200)
    {
    }

    void Server::Start(CallbackFuncPtr callback)
    {
        callback_ = std::make_shared<CallbackFunc>(
                [&](network::Command c){

            // ログアウト
            if (c.header() == network::header::FatalConnectionError) {
                // 受信制限量を更新
                /*
                auto new_average = GetSessionReadAverageLimit();
                if (session_read_average_ != new_average) {
                    session_read_average_ = new_average;
                    SendAll(network::ClientReceiveWriteAverageLimitUpdate(session_read_average_));
                }
                */
            }

            // 通信量制限を越えていた場合、強制的に切断
            else if (auto session = c.session().lock()) {
                if (session->GetReadByteAverage() > session_read_average_) {
					Logger::Info(_T("Banished session: %d"), session->id());
                    session->Close();
                }
            }

            if (callback) {
                (*callback)(c);
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


    bool Server::Empty() const
    {
        return sessions_.size() == 0;
    }

    void Server::ReceiveSession(const SessionPtr& session, const boost::system::error_code& error)
    {
        if (session_read_average_ > min_session_read_average_) {
            session->set_on_receive(callback_);
            session->Start();
            sessions_.push_back(SessionWeakPtr(session));

            // クライアント情報を要求
            session->Send(ClientRequestedClientInfo());

            // 受信制限量を更新
            auto new_average = GetSessionReadAverageLimit();
            session->Send(network::ClientReceiveWriteAverageLimitUpdate(session_read_average_));

            if (session_read_average_ != new_average) {
                session_read_average_ = new_average;
                SendOthers(network::ClientReceiveWriteAverageLimitUpdate(session_read_average_),
                        session);
            }

        } else {
            Logger::Info("Refuse Session");
            session->SyncSend(ClientReceiveServerCrowdedError());
            session->Close();
        }

        auto new_session = boost::make_shared<ServerSession>(io_service_);
         acceptor_.async_accept(new_session->tcp_socket(),
                 boost::bind(&Server::ReceiveSession, this, new_session, boost::asio::placeholders::error));

        // 使用済のセッションのポインタを破棄
        auto it = std::remove_if(sessions_.begin(), sessions_.end(),
                [](const SessionWeakPtr& ptr){
            return ptr.expired();
        });
        sessions_.erase(it, sessions_.end());

		Logger::Info("Active sessoin: %d", sessions_.size() - 1);
    }

    void Server::SendAll(const Command& command)
    {
        BOOST_FOREACH(SessionWeakPtr& ptr, sessions_) {
            if (auto session = ptr.lock()) {
                session->Send(command);
            }
        }
    }

    void Server::SendOthers(const Command& command, SessionWeakPtr self_ptr)
    {
        BOOST_FOREACH(SessionWeakPtr& ptr, sessions_) {
            if (auto session = ptr.lock()) {
                if (auto self = self_ptr.lock()) {
                    if (*session != *self) {
                        session->Send(command);
                    }
                }
            }
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

    void Server::ReceiveUDP(const boost::system::error_code& error, size_t bytes_recvd)
    {
        if (bytes_recvd > 0) {
            std::string buffer(receive_buf_udp_, bytes_recvd);
            FetchUDP(buffer);
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

    Command Server::FetchUDP(const std::string& buffer)
    {
        uint32_t user_id;
        unsigned char count;
        header::CommandHeader header;
        std::string body;
        SessionPtr session;

        size_t readed = network::Utils::Deserialize(buffer, &user_id, &count, &header);
        if (readed < buffer.size()) {
            body = buffer.substr(readed);
        }

        return Command(header, body, session);
    }

    void Server::ServerSession::Start()
    {
        online_ = true;

        // Nagleアルゴリズムを無効化
        socket_tcp_.set_option(boost::asio::ip::tcp::no_delay(true));

        // IPアドレスを取得
        global_ip_ = socket_tcp_.remote_endpoint().address().to_string();

        boost::asio::async_read_until(socket_tcp_,
            receive_buf_, NETWORK_UTILS_DELIMITOR,
            boost::bind(
              &ServerSession::ReceiveTCP, shared_from_this(),
              boost::asio::placeholders::error));
    }

    int Server::GetSessionReadAverageLimit()
    {
        int byte = max_total_read_average_ / (sessions_.size() + 1);
        byte = std::min(byte, max_session_read_average_);
        
        return byte;
    }

    int Server::max_total_read_average() const
    {
        return max_total_read_average_;
    }

    int Server::max_session_read_average() const
    {
        return max_session_read_average_;
    }

    int Server::min_session_read_average() const
    {
        return min_session_read_average_;
    }

    void Server::set_max_total_read_average(int byte)
    {
        max_total_read_average_ = byte;
    }

    void Server::set_max_session_read_average(int byte)
    {
        max_session_read_average_ = byte;
    }

    void Server::set_min_session_read_average(int byte)
    {
        min_session_read_average_ = byte;
    }

}
