//
// Server.hpp
//

#pragma once

#include <string>
#include <list>
#include <functional>
#include "../common/network/Session.hpp"

#define UDP_MAX_RECEIVE_LENGTH (2048)
#define UDP_TEST_PACKET_TIME (5)

namespace network {

class Server {
    private:
        class ServerSession : public Session {
            public:
                ServerSession(boost::asio::io_service& io_service) :
                    Session(io_service) {};

                void Start();
        };

    public:
        Server(uint16_t port);
        void Start(CallbackFuncPtr callback);
        void Stop();
        void SendAll(const Command&);
        void SendOthers(const Command&, SessionWeakPtr);

        bool Empty() const;

        int GetSessionReadAverageLimit();

        void SendUDPTestPacket(const std::string& ip_address, uint16_t port);

        int max_total_read_average() const;
        int max_session_read_average() const;
        int min_session_read_average() const;
        void set_max_total_read_average(int byte);
        void set_max_session_read_average(int byte);
        void set_min_session_read_average(int byte);

    private:
        void ReceiveSession(const SessionPtr&, const boost::system::error_code&);

        void ReceiveUDP(const boost::system::error_code& error, size_t bytes_recvd);
        void DoWriteUDP(const std::string& msg, const udp::endpoint& endpoint);
        void WriteUDP(const boost::system::error_code& error, boost::shared_ptr<std::string> holder);

        Command FetchUDP(const std::string& buffer);

    private:
       boost::asio::io_service io_service_;
       tcp::endpoint endpoint_;
       tcp::acceptor acceptor_;

       udp::socket socket_udp_;
       udp::endpoint sender_endpoint_;

       char receive_buf_udp_[2048];
       unsigned char udp_packet_count_;

       CallbackFuncPtr callback_;

       int max_total_read_average_;
       int max_session_read_average_;
       int min_session_read_average_;
       int session_read_average_;

       boost::mutex mutex_;
       std::list<SessionWeakPtr> sessions_;

};

}
