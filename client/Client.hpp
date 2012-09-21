//
// Client.hpp
//

#pragma once

#include <string>
#include <queue>
#include "../common/network/Session.hpp"
#include "../common/network/Signature.hpp"

#define UDP_MAX_RECEIVE_LENGTH (512)
#define DEFAULT_WRITE_AVERAGE_LIMIT (200)

namespace network {

    class Client {
        private:
            class ClientSession : public Session {
                public:
                ClientSession(boost::asio::io_service& io_service,
                        tcp::resolver::iterator endpoint_iterator,
                        uint16_t udp_port, udp::resolver::iterator iterator_udp) :
                                Session(io_service),
                                io_service_(io_service),
                                endpoint_iterator_(endpoint_iterator),
                                socket_udp_(io_service, udp::endpoint(udp::v4(), udp_port)),
                                iterator_udp_(iterator_udp)
                {
                }
                ;
                ~ClientSession();

                    void Start();
                    void Close();
                    void Connect(const boost::system::error_code& error);
                    void SendUDP(const std::string& data);

                    void ReceiveUDP(const boost::system::error_code& error, size_t bytes_recvd);
                    void DoWriteUDP(std::shared_ptr<std::string> data, const udp::endpoint& endpoint);
                    void WriteUDP(const boost::system::error_code& error);

                private:
                    boost::asio::io_service& io_service_;
                    tcp::resolver::iterator endpoint_iterator_;
                    udp::socket socket_udp_;

                    udp::resolver::iterator iterator_udp_;
                    udp::endpoint sender_endpoint_;

                    char receive_buf_udp_[UDP_MAX_RECEIVE_LENGTH];
            };

            typedef boost::shared_ptr<ClientSession> ClientSessionPtr;

        public:
           Client(const std::string& host,
                   uint16_t remote_tcp_port,
                   uint16_t local_udp_port,
                   const std::string& public_key,
                   const std::string& private_key,
                   const std::string& server_public_key_filename,
                   bool upnp);
           ~Client();

           void Write(const Command&);
           void WriteUDP(const Command&);
           void Close();

		   size_t GetCommandSize();
           std::shared_ptr<Command> Client::PopCommand();
           bool command_empty();

           unsigned int id() const;

           double GetReadByteAverage() const;
           double GetWriteByteAverage() const;

           bool online() const;

        private:
           boost::asio::io_service io_service_;
           tcp::resolver resolver_;
           tcp::resolver::iterator iterator_;

           boost::thread thread_;
           ClientSessionPtr session_;

           Signature signature_;

           int write_average_limit_;

           std::queue<Command> command_queue_;
           boost::mutex mutex_;

           uint16_t udp_port_;
           std::string global_ip_;

    };

}
