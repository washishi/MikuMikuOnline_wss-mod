//
// Client.cpp
//

#include <sstream>
#include <boost/make_shared.hpp>
#include "Client.hpp"
#include "../common/network/Utils.hpp"
#include "../common/network/Command.hpp"
#include "../common/Logger.hpp"
#include "version.hpp"

namespace network {

Client::Client(const std::string& host,
        uint16_t remote_tcp_port,
        uint16_t local_udp_port,
        const std::string& public_key,
        const std::string& private_key,
        const std::string& server_public_key_filename,
        bool upnp) :
                resolver_(io_service_),
                signature_(server_public_key_filename),
                write_average_limit_(DEFAULT_WRITE_AVERAGE_LIMIT)
{
    uint16_t udp_port = local_udp_port;
    std::string global_ip = "127.0.0.1";

    // NAT越えのためにポートを開ける
    if (upnp) {
        // TODO: VC移行のため無効化
        //if (PortMapper::MapUDP(&udp_port, &global_ip)) {
        //    Logger::Info("Open udp port %d", udp_port);
        //    Logger::Info("Global IP address is %s", global_ip);
        //} else {
        //    Logger::Error("Cannot open udp port");
        //}
    } else {
#ifdef _WIN32
        //HOSTENT *lpHost;
        //IN_ADDR inaddr;
        //WSADATA wsadata;
        //char szBuf[256], szIP[256];

        //if (WSAStartup(MAKEWORD(1, 1), &wsadata)) {
        //    Logger::Error(_T("WSAStartup() Failed"));
        //} else {
        //    gethostname(szBuf, (int)sizeof(szBuf));
        //    lpHost = gethostbyname(szBuf);
        //    memcpy(&inaddr, lpHost->h_addr_list[0], 4);
        //    strcpy(szIP, inet_ntoa(inaddr));
        //    global_ip = szIP;
        //    Logger::Info(_T("IP address is %s"), unicode::ToTString(global_ip));
        //    WSACleanup();
        //}
#endif
    }

    std::stringstream port_str;
    port_str << remote_tcp_port;

    try {
        tcp::resolver::query query(host, port_str.str());
        iterator_ = resolver_.resolve(query);
    } catch (const std::exception& e) {
        Logger::Error(_T("%s"), unicode::ToTString(e.what()));
		command_queue_.push(network::FatalConnectionError());
        return;
    }

    udp::resolver resolver(io_service_);
    udp::resolver::iterator iterator = resolver.resolve(
            udp::resolver::query(udp::v4(), host.c_str(), port_str.str().c_str()));

    using boost::asio::ip::udp;

    session_ = boost::make_shared<ClientSession>(io_service_, iterator_, udp_port, iterator);

    session_->set_global_ip(global_ip);
    session_->set_udp_port(udp_port);

    // 認証鍵をセット
    auto &encrypter = session_->encrypter();
    encrypter.SetPublicKey(public_key);
    encrypter.SetPrivateKey(private_key);

    Logger::Info(_T("Starting common key exchange..."));
    Logger::Info(_T("Checking local public key encryption key pair..."));

    if (!encrypter.CheckKeyPair()) {
        Logger::Error(_T("Incorrect local key pair"));
    }

    session_->set_on_receive(std::make_shared<CallbackFunc>(
            [this, public_key](network::Command c) {

                switch (c.header()) {

                    // クライアント情報要求
                    case network::header::ClientRequestedClientInfo:
                    {
                        if (auto session = c.session().lock()) {
                            Logger::Info(_T("Receive local public key fingerprint request"));
                            Logger::Info(_T("Send local public key fingerprint..."));

                            session->Send(network::ServerReceiveClientInfo(
                                            network::Encrypter::GetHash(public_key),
                                            (uint16_t)MMO_PROTOCOL_VERSION,
                                            session->udp_port()
                                    ));
							
							session->Send(network::ServerRequestedFullServerInfo());
                        }
                    }
                    break;

                    // 公開鍵要求
                    case network::header::ClientRequestedPublicKey:
                    {
                        if (auto session = c.session().lock()) {
                            Logger::Info(_T("Receive local public key request"));
                            Logger::Info(_T("Send local public key..."));
                            session->Send(ServerReceivePublicKey(public_key));
                        }
                    }
                    break;

                    // 共通鍵を受信
                    case header::ClientReceiveCommonKey:
                    {
                        if (auto session = c.session().lock()) {
                            Logger::Info(_T("Receive common key and server signature"));

                            // std::tuple<std::string, std::string, unsigned int> result_tuple;
                            // Utils::Deserialize(c.body(), &result_tuple);

                            std::string body = c.body();
                            int a = *reinterpret_cast<const int*>(body.data());

                            std::string key;
                            std::string sign;
                            unsigned int user_id;
                            int r = Utils::Deserialize(c.body(), &key, &sign, &user_id);

                            session->set_id(user_id);

                            /*
                            Logger::Info(_T("Checking server signature..."));
                            if (signature_.Verify(key, sign)) {
                                Logger::Info(_T("Valid server signature"));
                            } else {
                                Logger::Error(_T("Invalid server signature"));
                            }
                            */

                            session->encrypter().SetCryptedCommonKey(key);
                            session->Send(ServerStartEncryptedSession());

                        }
                    }
                    break;

                    // 暗号化通信開始
                    case network::header::ClientStartEncryptedSession:
                    {
                        if (auto session = c.session().lock()) {
                            Logger::Info(_T("Start encrypted session"));
                            session->EnableEncryption();
                        }
                    }
                    break;

                    // 通信量制限
                    case network::header::ClientReceiveWriteAverageLimitUpdate:
                    {
                        unsigned short byte;
                        network::Utils::Deserialize(c.body(), &byte);
                        write_average_limit_ = byte;
                        // Logger::Debug((boost::format(_T("Write limit update: %dbyte/s")) % byte).str());
                    }
                    break;

                    // 接続拒否
                    case network::header::ClientReceiveServerCrowdedError:
                    {
                        Logger::Error(_T("Server too crowded"));
                    }
                    break;

                    default:
                    break;
                }

                {
					boost::mutex::scoped_lock lock(mutex_);
                    command_queue_.push(c);
                }

            }));

    Logger::Info(_T("Starting a new session..."));
    // Logger::Info((boost::format(_T("Server: %s:%d")) % host % remote_tcp_port).str());

    session_->Start();
    thread_ = boost::thread([this]() {
        try {
            io_service_.run();
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    });


}

Client::~Client()
{
	if (session_) {
		session_->Close();
	}
    thread_.join();
    //Close();
}

void Client::Write(const Command& msg)
{
    if (session_) {
        // 送信制限を超えていないかチェック
         // if (GetWriteByteAverage() <= write_average_limit_) {
        if (true) {
            session_->Send(msg);
         } else {
             Logger::Error(_T("Write limit exceeded"));
             Logger::Info(_T("Command ignored"));
         }
    }
}

void Client::WriteUDP(const Command& msg)
{
    if (session_) {
        // 送信制限を超えていないかチェック
        // if (GetWriteByteAverage() <= write_average_limit_) {
        if (true) {
            session_->SendUDP(msg.body());
        } else {
            Logger::Error(_T("Write limit exceeded"));
            Logger::Info(_T("Command ignored"));
        }
    }
}

void Client::Close()
{
}

size_t Client::GetCommandSize()
{
    boost::mutex::scoped_lock lock(mutex_);
	return command_queue_.size();
}

std::shared_ptr<Command> Client::PopCommand()
{
    boost::mutex::scoped_lock lock(mutex_);
	std::shared_ptr<Command> command_ptr;
	if (!command_queue_.empty()) {
		Command command = command_queue_.front();
		command_ptr = std::make_shared<Command>(command);
		command_queue_.pop();
	}
	return command_ptr;
}

unsigned int Client::id() const
{
    if (session_) {
        return session_->id();
    } else {
        return 0;
    }
}

bool Client::online() const
{
    if (session_) {
        return session_->online();
    } else {
        return false;
    }
}

double Client::GetReadByteAverage() const
{
    if (session_) {
        return session_->GetReadByteAverage();
    } else {
        return -1;
    }
}

double Client::GetWriteByteAverage() const
{
    if (session_) {
        return session_->GetWriteByteAverage();
    } else {
        return -1;
    }
}

void Client::ClientSession::Start()
{
    boost::asio::async_connect(socket_tcp_, endpoint_iterator_,
            boost::bind(&ClientSession::Connect, this,
                    boost::asio::placeholders::error));

    socket_udp_.async_receive_from(
        boost::asio::buffer(receive_buf_udp_, UDP_MAX_RECEIVE_LENGTH), sender_endpoint_,
        boost::bind(&Client::ClientSession::ReceiveUDP, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));

}

void Client::ClientSession::Close()
{
    Session::Close();
    socket_udp_.close();
}

Client::ClientSession::~ClientSession()
{
}

void Client::ClientSession::Connect(const boost::system::error_code& error)
{
    if (!error) {
        online_ = true;

        // Nagleアルゴリズムを無効化
        socket_tcp_.set_option(boost::asio::ip::tcp::no_delay(true));

		// バッファサイズを変更 1MiB
		boost::asio::socket_base::receive_buffer_size option(1048576);
		socket_tcp_.set_option(option);

        boost::asio::async_read_until(socket_tcp_, receive_buf_, NETWORK_UTILS_DELIMITOR,
                boost::bind(&ClientSession::ReceiveTCP, shared_from_this(),
                        boost::asio::placeholders::error));

        if (on_receive_) {
            // (*on_receive_)(ConnectionSucceeded());
        }
    } else {
        if (on_receive_) {
            // (*on_receive_)(ConnectionFailed());
        }
    }
}

void Client::ClientSession::SendUDP(const std::string& message)
{
	auto holder = std::make_shared<std::string>(message.data(), message.size());
    io_service_.post(boost::bind(&Client::ClientSession::DoWriteUDP, this, holder, *iterator_udp_));
}

void Client::ClientSession::ReceiveUDP(const boost::system::error_code& error, size_t bytes_recvd)
{
    if (bytes_recvd > 0) {
        Logger::Debug(_T("UDP Receive %d"), bytes_recvd);
    }
    if (!error) {
      socket_udp_.async_receive_from(
          boost::asio::buffer(receive_buf_udp_, UDP_MAX_RECEIVE_LENGTH), sender_endpoint_,
          boost::bind(&Client::ClientSession::ReceiveUDP, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    } else {
        Logger::Error(_T("UDP　Error"));
    }
}

void Client::ClientSession::DoWriteUDP(std::shared_ptr<std::string> data, const udp::endpoint& endpoint)
{
    socket_udp_.async_send_to(
        boost::asio::buffer(data->data(), data->size()), endpoint,
        boost::bind(&Client::ClientSession::WriteUDP, this,
          boost::asio::placeholders::error));
}

void Client::ClientSession::WriteUDP(const boost::system::error_code& error)
{

}

}

