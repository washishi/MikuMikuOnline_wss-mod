//
// Command.hpp
//

#pragma once

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/asio.hpp>
#include <stdint.h>
#include "CommandHeader.hpp"
#include "../database/AccountProperty.hpp"

namespace network {

class Session;
typedef boost::weak_ptr<Session> SessionWeakPtr;

    class Command {
        public:
            Command(header::CommandHeader header, const std::string body) :
                header_(header), body_(body) {}

            Command(header::CommandHeader header, const std::string body, const SessionWeakPtr& session) :
                header_(header), body_(body), session_(session) {}

            Command(header::CommandHeader header, const std::string body,
				const boost::asio::ip::udp::endpoint& udp_endpoint) :
                header_(header), body_(body), udp_endpoint_(udp_endpoint) {}

            header::CommandHeader header() const;
            const std::string& body() const;
            SessionWeakPtr session();
			boost::asio::ip::udp::endpoint udp_endpoint() const;

        private:
            header::CommandHeader header_;

        protected:
            std::string body_;
            SessionWeakPtr session_;
			boost::asio::ip::udp::endpoint udp_endpoint_;
    };

	template<header::CommandHeader Header>
	class CommandTemplate : public Command {
		public:
			CommandTemplate() :
			  Command(Header, "") {}
	};

	typedef CommandTemplate<header::ServerStartEncryptedSession>		ServerStartEncryptedSession;
	typedef CommandTemplate<header::ClientStartEncryptedSession>		ClientStartEncryptedSession;
	typedef CommandTemplate<header::ClientRequestedPublicKey>			ClientRequestedPublicKey;
	typedef CommandTemplate<header::ClientRequestedClientInfo>			ClientRequestedClientInfo;
	typedef CommandTemplate<header::ClientReceiveServerCrowdedError>	ClientReceiveServerCrowdedError;
	typedef CommandTemplate<header::ServerRequestedFullServerInfo>		ServerRequestedFullServerInfo;

    // コネクションの切断
    class FatalConnectionError : public Command {
    public:
        FatalConnectionError();
        FatalConnectionError(uint32_t user_id);
    };

    // クライアントからの公開鍵を受信
    class ServerReceivePublicKey : public Command {
    public:
        ServerReceivePublicKey(const std::string& key);
    };

    // サーバーからの共通鍵を受信
    class ClientReceiveCommonKey : public Command {
    public:
        ClientReceiveCommonKey(const std::string& key,
                const std::string& sign,
                uint32_t user_id);
    };

    // 他のプレイヤーの位置情報が更新された
    class ClientUpdatePlayerPosition : public Command {
    public:
        ClientUpdatePlayerPosition(uint32_t id, int16_t x, int16_t y, int16_t z, uint8_t theta, uint8_t vy);
    };

    // プレイヤーの位置情報が更新された
    class ServerUpdatePlayerPosition : public Command {
    public:
        ServerUpdatePlayerPosition(int16_t x, int16_t y, int16_t z, uint8_t theta, uint8_t vy);
    };

    // クライアントの情報を受信した　（公開鍵のフィンガープリント, UDPポート）
    class ServerReceiveClientInfo : public Command {
    public:
        ServerReceiveClientInfo(const std::string& key, uint16_t version, uint16_t udp_port);
    };

    class ServerRequestedAccountRevisionPatch : public Command {
    public:
        ServerRequestedAccountRevisionPatch(uint32_t user_id, int revision);
    };

    // アカウントデータベースの更新データ
    class ClientReceiveAccountRevisionPatch : public Command {
    public:
        ClientReceiveAccountRevisionPatch(const std::string& patch);
    };

    // アカウントデータベースの更新通知
    class ClientReceiveAccountRevisionUpdateNotify : public Command {
    public:
        ClientReceiveAccountRevisionUpdateNotify(uint32_t user_id, int revision);
    };

    class ClientReceiveWriteAverageLimitUpdate : public Command {
    public:
        ClientReceiveWriteAverageLimitUpdate(uint16_t byte);
    };

    class ClientReceiveUnsupportVersionError : public Command {
    public:
        ClientReceiveUnsupportVersionError(uint32_t require_version);
    };

    class ServerReceiveAccountInitializeData : public Command {
    public:
        ServerReceiveAccountInitializeData(const std::string& data);
    };

    class ServerUpdateAccountProperty : public Command {
    public:
        ServerUpdateAccountProperty(AccountProperty uint16_t, const std::string& value);
    };

    class ServerReceiveJSON : public Command {
    public:
        ServerReceiveJSON(const std::string& json);
    };

    class ClientReceiveJSON : public Command {
    public:
        ClientReceiveJSON(const std::string& info_json, const std::string& message_json);
    };

	class ClientReceiveServerInfo : public Command {
    public:
        ClientReceiveServerInfo(const std::string& stage);
	};

	class ClientReceiveFullServerInfo : public Command {
    public:
        ClientReceiveFullServerInfo(const std::string& xml);
	};

}
