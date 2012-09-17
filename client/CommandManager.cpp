//
// CommandManager.cpp
//

#include "CommandManager.hpp"
#include "CardManager.hpp"
#include "PlayerManager.hpp"
#include "AccountManager.hpp"
#include "ResourceManager.hpp"
#include "../common/Logger.hpp"
#include "Client.hpp"
#include "../common/network/Utils.hpp"
#include "Profiler.hpp"

CommandManager::CommandManager(const ManagerAccessorPtr& manager_accessor) :
	manager_accessor_(manager_accessor),
	status_(STATUS_STANDBY)
{
}

void CommandManager::Update()
{
	MMO_PROFILE_FUNCTION;

    if (client_)
    {
        auto command = client_->PopCommand();
		
		if (command) {
			unsigned int header =  command->header();

			CardManagerPtr card_manager = manager_accessor_->card_manager().lock();
			PlayerManagerPtr player_manager = manager_accessor_->player_manager().lock();
			AccountManagerPtr account_manager = manager_accessor_->account_manager().lock();

			Logger::Debug(_T("Receive: 0x%08x %d byte"), header, command->body().size());

			switch (header) {
			using namespace network::header;

			// 暗号化通信を開始
			case ClientStartEncryptedSession:
			{
				const std::string& data = account_manager->GetSerializedData();
				if (data.size() > 0) {
					client_->Write(network::ServerReceiveAccountInitializeData(data));
				}
			}
				break;

			case ClientReceiveServerCrowdedError:
			{
				status_ = STATUS_ERROR_CROWDED;
			}
			break;

			case ClientReceiveUnsupportVersionError:
			{
				status_ = STATUS_ERROR_VERSION;
			}
			break;

			// サーバーデータ受信
			case ClientReceiveServerInfo:
			{
				network::Utils::Deserialize(command->body(), & stage_);
				if (ResourceManager::NameToFullPath(unicode::ToTString(stage_)).empty()) {
					status_ = STATUS_ERROR_NOSTAGE;
				} else {
					status_ = STATUS_READY;
				}
			}
			break;

			case ClientReceiveJSON:
			// case ClientReceiveChatLog:
			{
				std::string info_json, msg_json;
				network::Utils::Deserialize(command->body(), &info_json, &msg_json);

				Logger::Info(_T("Receive JSON: %s %s"), unicode::ToTString(info_json), unicode::ToTString(msg_json));

				card_manager->OnReceiveJSON(info_json, msg_json);
			}
				break;

			// プレイヤー位置更新
			case ClientUpdatePlayerPosition:
			{
				if (status_ == STATUS_READY) {
					PlayerPosition pos;
					uint32_t user_id;
					network::Utils::Deserialize(command->body(), &user_id, &pos.x, &pos.y, &pos.z, &pos.theta, &pos.vy);

					player_manager->UpdatePlayerPosition(user_id, pos);
				}
			}
				break;

			case ClientReceiveAccountRevisionUpdateNotify:
			{
				uint32_t user_id;
				uint32_t server_revision;
				network::Utils::Deserialize(command->body(), &user_id, &server_revision);

				auto current_revision = player_manager->GetCurrentUserRevision(user_id);

				Logger::Info(_T("Receive account database update notify　%d %d [%d]"), user_id, server_revision, current_revision);

				if (server_revision > current_revision) {
					client_->Write(network::ServerRequestedAccountRevisionPatch(user_id, current_revision));
				}
			}
				break;

			case ClientReceiveAccountRevisionPatch:
			{
				Logger::Info(_T("Receive account database update data"));
				assert(command->body().size() > 0);
				player_manager->ApplyRevisionPatch(command->body());
			}
				break;

			case FatalConnectionError:
			{
				status_ = STATUS_ERROR;
			}

			default:
				break;
			}
		}
	}
}

void CommandManager::set_client(ClientUniqPtr client)
{
    client_= std::move(client);
	// status_ = STATUS_CONNECTING;
}


void CommandManager::Write(const network::Command& command)
{
	MMO_PROFILE_FUNCTION;

    if (client_) {
        client_->Write(command);
    }
}

void CommandManager::WriteUDP(const network::Command& command)
{
    if (client_) {
        client_->WriteUDP(command);
    }
}

unsigned int CommandManager::user_id()
{
    if (client_) {
        return client_->id();
    } else {
        return 0;
    }
}

void CommandManager::SendJSON(const std::string& msg) {
	MMO_PROFILE_FUNCTION;

    if (client_ && msg.size() > 0) {
        client_->Write(network::ServerReceiveJSON(msg));
    }
}

CommandManager::Status CommandManager::status() const
{
	return status_;
}

std::string CommandManager::stage() const
{
	return stage_;
}