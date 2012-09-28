//
// MMO Server
//

#include <iostream>
#include <sstream>
#include <ctime>
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include "version.hpp"
#include "Server.hpp"
#include "../common/network/Encrypter.hpp"
#include "../common/network/Signature.hpp"
#include "../common/database/AccountProperty.hpp"
#include "../common/Logger.hpp"
#include "Config.hpp"
#include "Account.hpp"
#include "version.hpp"
#ifdef __linux__
#include "ServerSigHandler.hpp"
#include <csignal>
#endif

#ifdef _WIN32
#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>
#endif

using namespace boost::posix_time;

int main(int argc, char* argv[])
{
    // 設定を読み込み
    Config config("config.json");

	Logger::Info(_T("%s"), unicode::ToTString(MMO_VERSION_TEXT));

 try {

    // 署名
    network::Signature sign("server_key");

    // アカウント
    Account account("account.db");
    network::Server server(config);

    auto callback = std::make_shared<std::function<void(network::Command)>>(
            [&server, &account, &sign, &config](network::Command c){

        // ログを出力
        auto msg = (boost::format("Receive: 0x%08x %dbyte") % c.header() % c.body().size()).str();
        if (auto session = c.session().lock()) {
            msg += " from " + session->global_ip();
        }

        // if (auto session = c.session().lock()) {
        //     std::cout << "Write Average: " << session->GetReadByteAverage() << "bytes" << std::endl;
        // }

        switch (c.header()) {

		// ステータス要求
		case network::header::ServerRequstedStatus:
		{
			// ステータスを送り返す
			server.SendUDP(server.GetStatusJSON(), c.udp_endpoint());
		}
		break;

        // JSONメッセージ受信
        case network::header::ServerReceiveJSON:
        {
            if (auto session = c.session().lock()) {
                uint32_t id = static_cast<unsigned int>(session->id());
				if (id == 0) {
					Logger::Error(_T("Invalid session id"));
					break;
				}
				
				std::stringstream message_json(c.body());

				using namespace boost::property_tree;
				ptree message_tree;
				json_parser::read_json(message_json, message_tree);

				// プライベートメッセージの処理
				std::list<uint32_t> destination_list;
				auto private_list_tree =  message_tree.get_child("private", ptree());
				BOOST_FOREACH(const auto& user_id, private_list_tree) {
					destination_list.push_back(user_id.second.get_value<uint32_t>());
				}

                ptime now = second_clock::universal_time();
                auto time_string = to_iso_extended_string(now);

                std::string info_json;
                info_json += "{";
                info_json += (boost::format("\"id\":\"%d\",") % id).str();
                info_json += (boost::format("\"time\":\"%s\"") % time_string).str();
                info_json += "}";

				auto send_command = network::ClientReceiveJSON(info_json, message_json.str());

				if (destination_list.size() > 0) {
					BOOST_FOREACH(uint32_t user_id, destination_list) {
						server.SendTo(send_command, user_id);
					}
				} else {
					server.SendAll(send_command);
				}

				Logger::Info("Receive JSON: %s", message_json.str());
            }
        }
            break;


        // 位置情報受信
        case network::header::ServerUpdatePlayerPosition:
        {
            if (auto session = c.session().lock()) {
                PlayerPosition pos;
                network::Utils::Deserialize(c.body(), &pos.x, &pos.y, &pos.z, &pos.theta, &pos.vy);
                account.SetUserPosition(session->id(), pos);
                server.SendOthersLimited(network::ClientUpdatePlayerPosition(session->id(),
                        pos.x,pos.y,pos.z,pos.theta, pos.vy), c.session());
            }
        }
            break;

        // 公開鍵フィンガープリント受信
        case network::header::ServerReceiveClientInfo:
        {
            if (auto session = c.session().lock()) {

				session->ResetReadByteAverage();

                std::string finger_print;
                uint16_t version;
                uint16_t udp_port;

                network::Utils::Deserialize(c.body(), &finger_print, &version, &udp_port);

                // クライアントのプロトコルバージョンをチェック
                if (version != MMO_PROTOCOL_VERSION) {
                    Logger::Info("Unsupported Client Version : v%d", version);
                    session->Send(network::ClientReceiveUnsupportVersionError(1));
                    return;
                }

                // UDPパケットの宛先を設定
                session->set_udp_port(udp_port);

                Logger::Info("UDP destination is %s:%d", session->global_ip(), session->udp_port());

                // テスト送信
                server.SendUDPTestPacket(session->global_ip(), session->udp_port());

                uint32_t id = account.GetUserIdFromFingerPrint(finger_print);
                if (id == 0) {
                    // 未登録の場合、公開鍵を要求
                    session->Send(network::ClientRequestedPublicKey());
                } else {
                    uint32_t user_id = static_cast<uint32_t>(id);
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
                uint32_t user_id = account.RegisterPublicKey(c.body());
				assert(user_id > 0);

				session->ResetReadByteAverage();

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
				
				session->Send(network::ClientReceiveServerInfo(config.stage()));

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
                uint32_t user_id;
                uint32_t client_revision;
                network::Utils::Deserialize(c.body(), &user_id, &client_revision);

                if (client_revision < account.GetUserRevision(user_id)) {
                    session->Send(network::ClientReceiveAccountRevisionPatch(
                            account.GetUserRevisionPatch(user_id, client_revision)));
                }
                Logger::Info(msg);
            }
        }
        break;

        case network::header::ServerUpdateAccountProperty:
        {
            if (auto session = c.session().lock()) {
                AccountProperty property;
                std::string value;
                network::Utils::Deserialize(c.body(), &property, &value);

                auto old_revision = account.GetUserRevision(session->id());

                switch (property) {

                case NAME:
                    {
                        account.SetUserName(session->id(), value);
                    }
                    break;
                case TRIP:
                    {
                        account.SetUserTrip(session->id(), value);
                    }
                    break;
                case MODEL_NAME:
                    {
                        account.SetUserModelName(session->id(), value);
                    }
                    break;
                default:
                    ;
                }

                auto new_revison = account.GetUserRevision(session->id());
                if (new_revison > old_revision) {
                    server.SendAll(
                            network::ClientReceiveAccountRevisionUpdateNotify(
                            session->id(),new_revison));
                }

                Logger::Info(msg);
            }
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
				account.Remove(user_id);
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
		#ifdef _WIN32
		using namespace boost::interprocess;
        windows_shared_memory shm(open_only, "MMO_SERVER_WITH_CLIENT", read_only);
		#endif
        execute_with_client = true;
    } catch(std::exception& e) {
        Logger::Info("Stand-alone Mode");
        execute_with_client = false;
    }

    // クライアントから起動している場合、クライアントの状態を監視
	#ifdef _WIN32
    if (execute_with_client) {
        boost::thread([&server](){
            while (1) {
                boost::this_thread::sleep(boost::posix_time::milliseconds(4000));
                try {
					using namespace boost::interprocess;
					windows_shared_memory shm(open_only, "MMO_SERVER_WITH_CLIENT", read_only);
                } catch(std::exception& e) {
                    server.Stop();
                    break;
                }
            }
        });
    }
	#endif
    #ifdef __linux__
    network::ServerSigHandler handler(SIGINT,&server);
    #endif
    server.Start(callback);

  } catch (std::exception& e) {
      Logger::Error(e.what());
      
      Logger::Info("Stop Server");
  }

  return 0;

}
