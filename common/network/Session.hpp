//
// Session.hpp
//

#pragma once

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/timer.hpp>
#include <stdint.h>
#include <string>
#include <queue>
#include <memory>
#include "Encrypter.hpp"
#include "Command.hpp"

#define BYTE_AVERAGE_REFRESH_SECONDS (30)
#define COMPRESSED_FLAG (0x00010000)
#define COMPRESS_MIN_LENGTH (100)

namespace network {

    using boost::asio::ip::tcp;
    using boost::asio::ip::udp;
    typedef std::function<void(Command)> CallbackFunc;
    typedef std::shared_ptr<CallbackFunc> CallbackFuncPtr;
    typedef long UserID;

	class Session;
    typedef boost::weak_ptr<Session> SessionWeakPtr;
    typedef boost::shared_ptr<Session> SessionPtr;

    class Session : public boost::enable_shared_from_this<Session> {
        public:
            Session(boost::asio::io_service& io_service_tcp);
            virtual ~Session();

            virtual void Start() = 0;
            virtual void Close();
            void Send(const Command&);
            void SyncSend(const Command&);
            void UDPSend(const Command&);

            void EnableEncryption();

            double GetReadByteAverage() const;
            double GetWriteByteAverage() const;

            tcp::socket& tcp_socket();
            Encrypter& encrypter();

            void set_on_receive(CallbackFuncPtr);

            UserID id() const;
            void set_id(UserID id);
            bool online() const;

            std::string global_ip() const;
            uint16_t udp_port() const;
            void set_global_ip(const std::string& global_ip);
            void set_udp_port(uint16_t udp_port);

            int serialized_byte_sum() const;
            int compressed_byte_sum() const;

			int write_average_limit() const;
			void set_write_average_limit(int limit);

            bool operator==(const Session&);
            bool operator!=(const Session&);

        protected:
            void UpdateReadByteAverage();
            void UpdateWriteByteAverage();

            std::string Serialize(const Command& command);
            Command Deserialize(const std::string& msg);

            void ReceiveTCP(const boost::system::error_code& error);
            void DoWriteTCP(const std::string, SessionPtr session_holder);
            void WriteTCP(const boost::system::error_code& error,
					 boost::shared_ptr<std::string> holder, SessionPtr session_holder);
            void FetchTCP(const std::string&);

            void FatalError(SessionPtr session_holder = SessionPtr());

        protected:
            // ソケット
            boost::asio::io_service& io_service_tcp_;
            tcp::socket socket_tcp_;

            // 暗号化通信
            Encrypter encrypter_;
            bool encryption_;

            // 送受信のためのバッファ
            boost::asio::streambuf receive_buf_;
            std::queue<std::string> send_queue_;

            CallbackFuncPtr on_receive_;

            // UDPパケット送信の宛先
            std::string global_ip_;
            uint16_t udp_port_;

            bool online_;
            bool login_;

            time_t read_start_time_, write_start_time_;
            int read_byte_sum_, write_byte_sum_;
            int serialized_byte_sum_, compressed_byte_sum_;
			
			int write_average_limit_;

            UserID id_;
    };

}
