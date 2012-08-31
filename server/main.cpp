//
// MMO Server
//

#include <iostream>
#include <sstream>
#include <ctime>
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/foreach.hpp>
#include "Server.hpp"
#include "../common/network/Encrypter.hpp"
#include "../common/network/Signature.hpp"
#include "../common/Logger.hpp"
#include "Config.hpp"
#include "Account.hpp"

using namespace boost::posix_time;

int main(int argc, char* argv[])
{

 try {

    // 署名
    network::Signature sign("server_key");

    // 設定を読み込み
    Config config("config.json");

    // アカウント
    Account account("account.db");

    int port = config.port();
    network::Server server(port);

    // サーバー設定
    server.set_max_total_read_average(config.max_total_read_average());
    server.set_max_session_read_average(config.max_session_read_average());
    server.set_min_session_read_average(config.min_session_read_average());

    auto callback = std::make_shared<std::function<void(network::Command)>>(
            [&server, &account, &sign](network::Command c){

        // ログを出力
        auto msg = (boost::format("Receive: 0x%08x %dbyte") % c.header() % c.body().size()).str();
        if (auto session = c.session().lock()) {
            msg += " from " + session->global_ip();
        }

        // if (auto session = c.session().lock()) {
        //     std::cout << "Write Average: " << session->GetReadByteAverage() << "bytes" << std::endl;
        // }

        switch (c.header()) {

        // JSONメッセージ受信
        case network::header::ServerReceiveJSON:
        {
            if (auto session = c.session().lock()) {
                unsigned int id = static_cast<unsigned int>(session->id());
                ptime now = second_clock::universal_time();
                auto time_string = to_iso_extended_string(now);

                std::string info_json;
                info_json += "{";
                info_json += (boost::format("\"id\":\"%d\",") % id).str();
                info_json += (boost::format("\"time\":\"%s\"") % time_string).str();
                info_json += "}";

                std::string message_json = c.body();

                server.SendAll(network::ClientReceiveJSON(info_json, message_json));
                Logger::Info("Receive JSON: %s", message_json);
                Logger::Info(msg);
            }
        }
            break;

        // 位置情報受信
        case network::header::ServerUpdatePlayerPosition:
        {
            if (auto session = c.session().lock()) {
                PlayerPosition pos;
                network::Utils::Deserialize(c.body(), &pos.x, &pos.y, &pos.z, &pos.theta);
                account.SetUserPosition(session->id(), pos);
                server.SendOthers(network::ClientUpdatePlayerPosition(session->id(),
                        pos.x,pos.y,pos.z,pos.theta), c.session());
            }
        }
            break;

        // 公開鍵フィンガープリント受信
        case network::header::ServerReceiveClientInfo:
        {
            if (auto session = c.session().lock()) {

                std::string finger_print;
                uint32_t version;
                uint16_t udp_port;

                network::Utils::Deserialize(c.body(), &finger_print, &version, &udp_port);

                // クライアントのプロトコルバージョンをチェック
                if (version != 1) {
                    Logger::Info("Unsupported Client Version : v%d", version);
                    session->Send(network::ClientReceiveUnsupportVersionError(1));
                    return;
                }

                // UDPパケットの宛先を設定
                session->set_udp_port(udp_port);

                Logger::Info("UDP destination is %s:%d", session->global_ip(), session->udp_port());

                // テスト送信
                // server.SendUDPTestPacket(session->global_ip(), session->udp_port());

                long id = account.GetUserIdFromFingerPrint(finger_print);
                if (id < 1) {
                    // 未登録の場合、公開鍵を要求
                    session->Send(network::ClientRequestedPublicKey());
                } else {
                    unsigned int user_id = static_cast<unsigned int>(id);
                    // ログイン
                    session->set_id(user_id);
                    account.LogIn(user_id);
                    session->encrypter().SetPublicKey(account.GetPublicKey(user_id));

                    account.SetUserIPAddress(session->id(), session->global_ip());
                    account.SetUserUDPPort(session->id(), session->udp_port());

                    // 共通鍵を送り返す
                    auto key = session->encrypter().GetCryptedCommonKey();
                    session->Send(network::ClientReceiveCommonKey(key, sign.Sign(key), user_id));

                }
                Logger::Info(msg);
            }
        }
            break;

        // 公開鍵受信
        case network::header::ServerReceivePublicKey:
        {
            if (auto session = c.session().lock()) {
                long id = account.RegisterPublicKey(c.body());
                unsigned int user_id = static_cast<unsigned int>(id);
                // ログイン
                session->set_id(user_id);
                account.LogIn(user_id);
                session->encrypter().SetPublicKey(account.GetPublicKey(user_id));

                account.SetUserIPAddress(session->id(), session->global_ip());
                account.SetUserUDPPort(session->id(), session->udp_port());

                // 共通鍵を送り返す
                auto key = session->encrypter().GetCryptedCommonKey();
                session->Send(network::ClientReceiveCommonKey(key, sign.Sign(key), user_id));

            }
            Logger::Info(msg);
        }
            break;

        // 暗号化通信開始
        case network::header::ServerStartEncryptedSession:
        {
            if (auto session = c.session().lock()) {
                session->Send(network::ClientStartEncryptedSession());
                session->EnableEncryption();

                Logger::Info(msg);
            }
        }
            break;

        // アカウント初期化情報の受信
        case network::header::ServerReceiveAccountInitializeData:
        {
            if (auto session = c.session().lock()) {
                account.LoadInitializeData(session->id(), c.body());

                const auto& list = account.GetIDList();
                BOOST_FOREACH(UserID user_id, list) {
                    session->Send(network::ClientReceiveAccountRevisionUpdateNotify(user_id,
                            account.GetUserRevision(user_id)));
                }

                server.SendOthers(
                        network::ClientReceiveAccountRevisionUpdateNotify(session->id(),
                                account.GetUserRevision(session->id())), c.session());

                Logger::Info(msg);
            }
        }
        break;

        // アカウント更新情報の要求
        case network::header::ServerRequestedAccountRevisionPatch:
        {
            if (auto session = c.session().lock()) {
                unsigned int user_id;
                unsigned int client_revision;
                network::Utils::Deserialize(c.body(), &user_id, &client_revision);

                if (client_revision < account.GetUserRevision(user_id)) {
                    session->Send(network::ClientReceiveAccountRevisionPatch(
                            account.GetUserRevisionPatch(user_id, client_revision)));
                }
                Logger::Info(msg);
            }
        }
        break;

        // ユーザー名の更新
        case network::header::ServerUpdatePlayerName:
        {
            if (auto session = c.session().lock()) {
                std::string name = c.body();
                if (name.size() > 0 && name.size() <= 64) {
                    account.SetUserName(session->id(), name);
                    server.SendAll(
                            network::ClientReceiveAccountRevisionUpdateNotify(session->id(),
                                    account.GetUserRevision(session->id())));
                }
                Logger::Info(msg);
            }
        }
        break;

        // トリップの更新
        case network::header::ServerUpdatePlayerTrip:
        {
            if (auto session = c.session().lock()) {
                std::string trip = c.body();
                if (trip.size() > 0 && trip.size() <= 64) {
                    account.SetUserTrip(session->id(), trip);
                    server.SendAll(
                            network::ClientReceiveAccountRevisionUpdateNotify(session->id(),
                                    account.GetUserRevision(session->id())));
                }
                Logger::Info(msg);
            }
        }
        break;

        // モデル名の更新
        case network::header::ServerUpdatePlayerModelName:
        {
            if (auto session = c.session().lock()) {
                std::string name = c.body();
                if (name.size() > 0 && name.size() <= 64) {
                    account.SetUserModelName(session->id(), name);
                    server.SendAll(
                            network::ClientReceiveAccountRevisionUpdateNotify(session->id(),
                                    account.GetUserRevision(session->id())));
                }
                Logger::Info(msg);
            }
        }
        break;

       // ログアウト
       case network::header::PlayerLogoutNotify:
       {
           Logger::Info(msg);
       }
       break;

       // エラー
       case network::header::FatalConnectionError:
       {
           if (c.body().size() > 0) {
               int user_id;
               network::Utils::Deserialize(c.body(), &user_id);
               account.LogOut(user_id);

               server.SendAll(
                       network::ClientReceiveAccountRevisionUpdateNotify(user_id,
                               account.GetUserRevision(user_id)));

               Logger::Info("Logout User: %d", user_id);
           }
       }
       Logger::Info(msg);
       break;

        default:
            break;
        }

    });

    bool execute_with_client;
    try {
        boost::interprocess::managed_shared_memory 
            shm(boost::interprocess::open_read_only, "MMO_SERVER_WITH_CLIENT");
        execute_with_client = true;
    } catch(std::exception& e) {
        Logger::Info("Standalone Mode");
        execute_with_client = false;
    }

    // クライアントから起動している場合、クライアントの状態を監視
    if (execute_with_client) {
        boost::thread([&server](){
            while (1) {
                boost::this_thread::sleep(boost::posix_time::milliseconds(4000));
                try {
                    boost::interprocess::managed_shared_memory 
                        shm(boost::interprocess::open_read_only, "MMO_SERVER_WITH_CLIENT");
                } catch(std::exception& e) {
                    server.Stop();
                    break;
                }
            }
        });
    }

    server.Start(callback);

  } catch (std::exception& e) {
      Logger::Error(e.what());
      Logger::Info("Stop Server");
  }

  return 0;

}
