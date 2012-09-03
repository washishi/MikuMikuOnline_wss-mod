//
// Command.hpp
//

#pragma once

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <stdint.h>
#include "CommandHeader.hpp"

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


    // コネクションの確立に成功
    class ConnectionSucceeded : public Command {
    public:
        ConnectionSucceeded();
    };

    // コネクションの確立に失敗
    class ConnectionFailed : public Command {
    public:
        ConnectionFailed();
    };

    // コネクションの切断
    class FatalConnectionError : public Command {
    public:
        FatalConnectionError();
        FatalConnectionError(unsigned int user_id);
    };

    // チャットメッセージを受信した
    class ClientReceiveChatMessage : public Command {
    public:
        ClientReceiveChatMessage(unsigned int user_id, const std::string& time_string, const std::string& msg);
    };

    // チャットメッセージを受信した
    class ServerReceiveChatMessage : public Command {
    public:
        ServerReceiveChatMessage(const std::string& msg);
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

    // チャットログを受信した
    class ClientReceiveChatLog : public Command {
    public:
        ClientReceiveChatLog();
    };

    // チャットログを要求された
    class ServerRequestedChatLog : public Command {
    public:
        ServerRequestedChatLog();
    };

    // ユーザーが入室した
    class ClientJoinPlayer : public Command {
    public:
        ClientJoinPlayer();
    };

    // ユーザーが退室した
    class ClientLeavePlayer : public Command {
    public:
        ClientLeavePlayer();
    };

    // 他のプレイヤーの位置情報が更新された
    class ClientUpdatePlayerPosition : public Command {
    public:
        ClientUpdatePlayerPosition(unsigned int id, float x, float y, float z, float theta);
    };

    // プレイヤーの位置情報が更新された
    class ServerUpdatePlayerPosition : public Command {
    public:
        ServerUpdatePlayerPosition(float x, float y, float z, float theta);
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

    // チャンネルのプレイヤーリストが更新された
    class ClientUpdateChannelUserList : public Command {
    public:
        ClientUpdateChannelUserList();
    };

    // セッションを切断された
    class ServerCloseSession : public Command {
    public:
        ServerCloseSession();
    };

    // カードの登録要求
    class ServerReceiveNewCard : public Command {
    public:
        ServerReceiveNewCard(const std::string& name,
                             const std::string& note,
                             const std::string& url);
    };

    //
    class ServerRequestedCardRevisionPatch : public Command {
    public:
        ServerRequestedCardRevisionPatch(int revision);
    };

    class ServerRequestedAccountRevisionPatch : public Command {
    public:
        ServerRequestedAccountRevisionPatch(unsigned int user_id, int revision);
    };

    // カードデータベースの更新データ
    class ClientReceiveCardRevisionPatch : public Command {
    public:
        ClientReceiveCardRevisionPatch(const std::string& patch);
    };

    // カードデータベースの更新通知
    class ClientReceiveCardRevisionUpdateNotify : public Command {
    public:
        ClientReceiveCardRevisionUpdateNotify(int revision);
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

    class PlayerLogoutNotify : public Command {
    public:
        PlayerLogoutNotify(unsigned int user_id);
    };

    class ServerUpdatePlayerName : public Command {
    public:
        ServerUpdatePlayerName(const std::string& name);
    };

    class ServerUpdatePlayerTrip : public Command {
    public:
        ServerUpdatePlayerTrip(const std::string& trip);
    };

    class ClientReceiveUnsupportVersionError : public Command {
    public:
        ClientReceiveUnsupportVersionError(uint32_t require_version);
    };

    class ServerReceiveAccountInitializeData : public Command {
    public:
        ServerReceiveAccountInitializeData(const std::string& data);
    };

    class ServerUpdatePlayerModelName : public Command {
    public:
        ServerUpdatePlayerModelName(const std::string& name);
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
