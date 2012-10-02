//
// Server.hpp
//

#pragma once

#include <string>
#include <list>
#include <functional>
#include "../common/network/Session.hpp"
#include "Config.hpp"

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
        Server(Config& config);
        void Start(CallbackFuncPtr callback);
        void Stop();
        void Stop(int interrupt_type);

        void SendAll(const Command&, int channel = -1, bool limited = false);
        void SendOthers(const Command&, uint32_t self_id, int channel = -1, bool limited = false);
        void SendTo(const Command&, uint32_t);

        bool Empty() const;
		std::string GetStatusJSON() const;

        int GetSessionReadAverageLimit();
		int GetUserCount() const;
		void RefreshSession();

        void SendUDPTestPacket(const std::string& ip_address, uint16_t port);
		void SendUDP(const std::string& message, const boost::asio::ip::udp::endpoint endpoint);

		bool IsBlockedAddress(const boost::asio::ip::address& address);

    private:
        void ReceiveSession(const SessionPtr&, const boost::system::error_code&);

        void ReceiveUDP(const boost::system::error_code& error, size_t bytes_recvd);
        void DoWriteUDP(const std::string& msg, const udp::endpoint& endpoint);
        void WriteUDP(const boost::system::error_code& error, boost::shared_ptr<std::string> holder);

        void FetchUDP(const std::string& buffer, const boost::asio::ip::udp::endpoint endpoint);

    private:
	   Config& config_;

       boost::asio::io_service io_service_;
       tcp::endpoint endpoint_;
       tcp::acceptor acceptor_;

       udp::socket socket_udp_;
       udp::endpoint sender_endpoint_;

       char receive_buf_udp_[2048];
       uint8_t udp_packet_count_;

       CallbackFuncPtr callback_;

       boost::mutex mutex_;
       std::list<SessionWeakPtr> sessions_;

};

}
