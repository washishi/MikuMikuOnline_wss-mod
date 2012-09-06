//
// Command.hpp
//

#pragma once

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
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

            header::CommandHeader header() const;
            const std::string& body() const;
            SessionWeakPtr session();

        private:
            header::CommandHeader header_;

        protected:
            std::string body_;
            SessionWeakPtr session_;
    };

    // コネクションの切断
    class FatalConnectionError : public Command {
    public:
        FatalConnectionError();
        FatalConnectionError(unsigned int user_id);
    };

    // 暗号化通信を開始
    class ServerStartEncryptedSession : public Command {
    public:
        ServerStartEncryptedSession();
    };

    // 暗号化通信を開始
    class ClientStartEncryptedSession : public Command {
    public:
        ClientStartEncryptedSession();
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
                unsigned int user_id);
    };

    // 他のプレイヤーの位置情報が更新された
    class ClientUpdatePlayerPosition : public Command {
    public:
        ClientUpdatePlayerPosition(unsigned int id, short x, short y, short z, unsigned char theta, unsigned char vy);
    };

    // プレイヤーの位置情報が更新された
    class ServerUpdatePlayerPosition : public Command {
    public:
        ServerUpdatePlayerPosition(short x, short y, short z, unsigned char theta, unsigned char vy);
    };

    // クライアントの情報を受信した　（公開鍵のフィンガープリント, UDPポート）
    class ServerReceiveClientInfo : public Command {
    public:
        ServerReceiveClientInfo(const std::string& key, uint32_t version, uint16_t udp_port);
    };

    // 公開鍵を要求された
    class ClientRequestedPublicKey : public Command {
    public:
        ClientRequestedPublicKey();
    };

    // クライアントの情報を要求された
    class ClientRequestedClientInfo : public Command {
    public:
        ClientRequestedClientInfo();
    };

    class ServerRequestedAccountRevisionPatch : public Command {
    public:
        ServerRequestedAccountRevisionPatch(unsigned int user_id, int revision);
    };

    // アカウントデータベースの更新データ
    class ClientReceiveAccountRevisionPatch : public Command {
    public:
        ClientReceiveAccountRevisionPatch(const std::string& patch);
    };

    // アカウントデータベースの更新通知
    class ClientReceiveAccountRevisionUpdateNotify : public Command {
    public:
        ClientReceiveAccountRevisionUpdateNotify(unsigned int user_id, int revision);
    };

    class ClientReceiveWriteAverageLimitUpdate : public Command {
    public:
        ClientReceiveWriteAverageLimitUpdate(unsigned short byte);
    };

    class ClientReceiveServerCrowdedError : public Command {
    public:
        ClientReceiveServerCrowdedError();
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
        ServerUpdateAccountProperty(AccountProperty propery, const std::string& value);
    };

    class ServerReceiveJSON : public Command {
    public:
        ServerReceiveJSON(const std::string& json);
    };

    class ClientReceiveJSON : public Command {
    public:
        ClientReceiveJSON(const std::string& info_json, const std::string& message_json);
    };

}
