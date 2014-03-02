//
// PlayerManager.cpp
//

#include "PlayerManager.hpp"
#include "CommandManager.hpp"
#include "CardManager.hpp"
#include "WorldManager.hpp"
#include "AccountManager.hpp"
#include "ConfigManager.hpp"
#include "../common/network/Utils.hpp"
#include "../common/network/Command.hpp"
#include "../common/database/AccountProperty.hpp"
#include "../common/Logger.hpp"
#include "../common/unicode.hpp"

#include "3d/CharacterDataProvider.hpp"
#include "3d/CharacterManager.hpp"
#include "3d/Timer.hpp"
#include "3d/gameloop.hpp"
#include "3d/PlayerCharacter.hpp"

#include "ui/UILabel.hpp"

#include "Profiler.hpp"

#include <list>
#include <boost/foreach.hpp>

PlayerManager::PlayerManager(const ManagerAccessorPtr& manager_accessor) :
timer_(std::make_shared<Timer>()),
manager_accessor_(manager_accessor),
charmgr_(std::make_shared<CharacterManager>()),
stage_ptr_holder_(std::make_shared<StagePtr>()),
show_action_list_(false)
{
}

PlayerManager::~PlayerManager()
{

}

void PlayerManager::ResetStage()
{
	stage_ptr_holder_->reset();
}

void PlayerManager::Init()
{
	auto world_manager = manager_accessor_->world_manager().lock();
	*stage_ptr_holder_ = world_manager->stage();

	if (char_data_providers_.find(0) == char_data_providers_.end()) {
		auto account_manager = manager_accessor_->account_manager().lock();
		AddCharacter<FieldPlayer>(0, unicode::ToTString(account_manager->model_name()));
		charmgr_->set_my_character_id(0);
	}
	user_action_list_.addItem(UIBasePtr( new UILabel([this]()->UILabel{
		UILabel label;
		label.set_text(unicode::ToTString(unicode::sjis2utf8("周りのユーザー")));
		label.set_width(120);
		label.set_top(12);
		label.set_left(0);
		label.set_textcolor(UISuper::Color(0,0,0,255));
		label.set_bgcolor(UISuper::Color(255,255,255,180));
		label.set_on_click_function_([](UIBase* ptr)->void{
		});
		label.set_on_hover_function_([](UIBase* ptr)->void{
			auto label_ptr = (UILabel *)ptr;
			label_ptr->set_bgcolor(UISuper::Color(0,0,0,180));
			label_ptr->set_textcolor(UISuper::Color(255,255,255));
		});
		label.set_on_out_function_([](UIBase* ptr)->void{
			auto label_ptr = (UILabel *)ptr;
			label_ptr->set_bgcolor(UISuper::Color(255,255,255,180));
			label_ptr->set_textcolor(UISuper::Color(0,0,0));
		});
		return label;
	}())));
}

void PlayerManager::ProcessInput(InputManager* input)
{
	bool push_mouse_middle = (input->GetMouseMiddleCount() > 0);
	bool prev_mouse_middle = input->GetPrevMouseMiddle();
	bool push_mouse_right = (input->GetMouseRightCount() > 0);
	bool prev_mouse_right = input->GetPrevMouseRight();
	bool push_mouse_left = (input->GetMouseLeftCount() > 0);
	bool prev_mouse_left = input->GetPrevMouseLeft();

	if( push_mouse_middle && !prev_mouse_middle ){
		auto pos = input->GetMousePos();
		user_action_list_.set_left(pos.first);
		user_action_list_.set_top(pos.second);
		show_action_list_ = true;
		auto mouse_pos_ = input->GetMousePos();
		auto config_manager_ = manager_accessor_->config_manager().lock();
		if( mouse_pos_.second + user_action_list_.absolute_height() > config_manager_->screen_height()){
			user_action_list_.set_top(mouse_pos_.second - user_action_list_.absolute_height());
			if ( mouse_pos_.first + user_action_list_.absolute_width() > config_manager_->screen_width()){
				user_action_list_.set_left(mouse_pos_.first - user_action_list_.absolute_width());
				BOOST_FOREACH(auto it,user_action_list_.getItems()){
					it->set_left(mouse_pos_.first - user_action_list_.absolute_width());
					it->set_top(mouse_pos_.second - user_action_list_.absolute_height() + 12);
				}
			}else{
				user_action_list_.set_left(mouse_pos_.first);
				BOOST_FOREACH(auto it,user_action_list_.getItems()){
					it->set_left(mouse_pos_.first);
					it->set_top(mouse_pos_.second - user_action_list_.absolute_height() + 12);
				}
			}
		}else{
			user_action_list_.set_top(mouse_pos_.second);
			if ( mouse_pos_.first + user_action_list_.absolute_width() > config_manager_->screen_width()){
				user_action_list_.set_left(mouse_pos_.first - user_action_list_.absolute_width());
				BOOST_FOREACH(auto it,user_action_list_.getItems()){
					it->set_left(mouse_pos_.first - user_action_list_.absolute_width());
					it->set_top(mouse_pos_.second + 12);
				}
			}else{
				user_action_list_.set_left(mouse_pos_.first);
				BOOST_FOREACH(auto it,user_action_list_.getItems()){
					it->set_left(mouse_pos_.first);
					it->set_top(mouse_pos_.second + 12);
				}
			}
		}
	}
	if( ( push_mouse_left || push_mouse_right ) && show_action_list_ ){
		show_action_list_ = false;
	}
	if( show_action_list_ ){
		user_action_list_.ProcessInput(input);
	}
}

void PlayerManager::Update()
{
	MMO_PROFILE_FUNCTION;
	if (!GetMyself()) return;

    timer_->Next();

    // 自分自身のキャラクターIDは常に0, それ以外はアカウントのID
    // プレイヤーの位置情報を更新
    if (auto player = GetMyself()) {
        const VECTOR& pos = char_data_providers_[charmgr_->my_character_id()]->position();
        const float theta = char_data_providers_[charmgr_->my_character_id()]->theta();
        player->set_position(PlayerPosition(pos.x, pos.y, pos.z, theta, 0));
    }

    // 位置情報を送信
    static int count = 0;
    if (count % 30 == 0) {
        const VECTOR& pos = char_data_providers_[charmgr_->my_character_id()]->position();
        const float theta = char_data_providers_[charmgr_->my_character_id()]->theta();
        const float vy = char_data_providers_[charmgr_->my_character_id()]->vy();

		char vy_char = std::max(std::min(vy, static_cast<float>(SCHAR_MAX)), static_cast<float>(SCHAR_MIN));
        if (auto command_manager = manager_accessor_->command_manager().lock()) {
             command_manager->Write(network::ServerUpdatePlayerPosition(pos.x, pos.y, pos.z, theta, vy_char));
        }
        // Logger::Debug("PlayerPos %f %f %f", pos.x, pos.y, pos.z);
    }
    count++;

    BOOST_FOREACH (auto pair, login_players_) {

		auto& player = pair.second;

		const auto& current_model = player->current_model_name();
		auto model = player->model_name();

		// ※チャンネル移動時にモデルをRemoveしないようにコメント
		//if (player->channel() != GetMyself()->channel()) {
		//	model = "";
		//}

		if (player->current_model_name() != model) {
			if (player->id() != 0) {
				if (char_data_providers_.find(player->id()) != char_data_providers_.end()) {
					RemoveCharacter(player->id());
					Logger::Debug(_T("Remove char %d %s"), player->id(), unicode::ToTString(player->current_model_name()));	
				}
			}
			player->set_current_model_name("");
		}

		// ※他チャンネルのユーザのモデルチェンジ時にモデルを読み込まないように追加
		if (player->channel() != GetMyself()->channel()) {
			model = "";
		}
		// ※ここまで

		if (player->current_model_name().empty() && !model.empty()) {
			if (player->id() != GetMyself()->id()) {
				AddCharacter<PlayerCharacter>(player->id(), unicode::ToTString(player->model_name()));
				player->set_current_model_name(player->model_name());
			}
		}

        if (player->login()) {
            player->Update();
        }
    }

	if ( show_action_list_ ) {
		user_action_list_.Update();
	}

}

void PlayerManager::Draw()
{
	MMO_PROFILE_FUNCTION;
	if (!GetMyself()) return;

	auto config_manager = manager_accessor_->config_manager().lock();
	unsigned int channel = GetMyself()->channel();

    // TODO: モデルの高さを取得する必要あり
    BOOST_FOREACH(auto pair, login_players_) {
        if (pair.second-> channel() == channel && pair.second->login()) {
            if (char_data_providers_.find(pair.second->id()) != char_data_providers_.end()) {
                const VECTOR& pos = char_data_providers_[pair.second->id()]->position();
                const float theta = char_data_providers_[pair.second->id()]->theta();
                pair.second->set_position(PlayerPosition(pos.x, pos.y, pos.z, theta, 0));

				if (config_manager->show_modelname() == 1) {

					auto world_manager = manager_accessor_->world_manager().lock();
					if (!world_manager->IsVisiblePoint(pos + VGet(0, 25, 0))) {
						continue;
					}

					if (world_manager->IsNearPoint(pos + VGet(0, 25, 0))) {
						auto model_name = unicode::ToTString(pair.second->model_name());
						if (ResourceManager::NameToFullPath(model_name).empty()) {
								auto screen_pos = ConvWorldPosToScreenPos(pos+ VGet(0, 10, 0));
								int x = (screen_pos.x / 2) * 2;
								int y = (screen_pos.y / 2) * 2 - 16;

								UILabel label_;
								label_.set_width(120);
								label_.set_text( model_name);
								label_.set_bgcolor(UIBase::Color(255,255,255,150));
								label_.set_textcolor(UIBase::Color(0,0,0,255));
								label_.set_left(x - 60);
								label_.set_top(y + 10);

								label_.Update();
								label_.Draw();
						}
					}
				}
            }

			if (config_manager->show_nametag() == 1) {
				pair.second->Draw();
			}

        }
    }

	if ( show_action_list_ ){
		user_action_list_.Draw();
	}
}

uint32_t PlayerManager::GetCurrentUserRevision(uint32_t user_id)
{
    if (auto player = GetFromId(user_id)) {
        return player->revision();
    } else {
        return 0;
    }
}

void PlayerManager::ApplyRevisionPatch(const std::string& patch)
{
	MMO_PROFILE_FUNCTION;

    std::string buffer(patch);
    Logger::Debug(_T("%s"), unicode::ToTString(network::Utils::ToHexString(buffer)));

    uint32_t user_id;
    uint32_t new_revision;
    buffer.erase(0, network::Utils::Deserialize(buffer, &user_id, &new_revision));

	auto command_manager = manager_accessor_->command_manager().lock();

	// TODO: なぜかUserIDが0番のデータが送信されてくる
	if (user_id == 0) {
		return;
	}

    PlayerPtr player;
    auto it_player = login_players_.find(user_id);
    if (it_player != login_players_.end()) {
        player = it_player->second;
    } else {
        player = std::make_shared<Player>(user_id);
        login_players_[user_id] = player;
    }

    assert(player);
    player->set_revision(new_revision);

    while (buffer.size()) {
        uint16_t property_int;
        buffer.erase(0, network::Utils::Deserialize(buffer, &property_int));

        AccountProperty property = static_cast<AccountProperty>(property_int);
        Logger::Debug(_T("UpdatePlayer : %d %d"), user_id, property);

        switch (property) {

            case LOGIN:
            {
                char value;
                buffer.erase(0, network::Utils::Deserialize(buffer, &value));
                bool login = value;

				auto card_manager = manager_accessor_->card_manager().lock();
                if (!player->login() && login) {
                    if (!player->name().empty()) {
                        card_manager->OnLogin(player);
                    }
                } else if(player->login() && !login) {
					card_manager->OnLogout(login_players_[user_id]);
					player->set_model_name("");
                    // RemoveCharacter(user_id);
                }

                player->set_login(login);
            }
                Logger::Debug(_T("UpdateLoginStatus %d"), user_id);
                break;

            case CHANNEL:
            {
                unsigned char channel;
                buffer.erase(0, network::Utils::Deserialize(buffer, &channel));
                player->set_channel(channel);
                Logger::Debug(_T("UpdateChannel %d : %d"), user_id, channel);
            }
                break;

            case NAME:
            {
                auto initialize = player->name().empty();

                std::string value;
                buffer.erase(0, network::Utils::Deserialize(buffer, &value));
                player->set_name(value);

                // 名前を受信した時にログインを通知
                if (initialize && player->login()) {
                    auto card_manager = manager_accessor_->card_manager().lock();
                    card_manager->OnLogin(player);
                }
            }
                Logger::Debug(_T("UpdateName %d"), user_id);
                break;

            case TRIP:
            {
                std::string value;
                buffer.erase(0, network::Utils::Deserialize(buffer, &value));
                player->set_trip(value);
            }
                Logger::Debug(_T("UpdateTrip %d"), user_id);
                break;

            case MODEL_NAME:
            {
                std::string value;
                buffer.erase(0, network::Utils::Deserialize(buffer, &value));

                if (player->model_name() != value) {
                    //auto command_manager = manager_accessor_->command_manager().lock();
                    //if (user_id != 0) {
                    //    if (char_data_providers_.find(user_id) != char_data_providers_.end()) {
                    //        RemoveCharacter(user_id);
                    //    }
                    //}
                    player->set_model_name(value);
                }
            }
                Logger::Debug(_T("UpdateModelName %d"), user_id);
                break;

            case IP_ADDRESS:
            {
                std::string value;
                buffer.erase(0, network::Utils::Deserialize(buffer, &value));
                player->set_ip_address(value);
            }
                Logger::Debug(_T("UpdateIPAddress %d"), user_id);
                break;

            case UDP_PORT:
            {
                uint16_t port;
                buffer.erase(0, network::Utils::Deserialize(buffer, &port));
                player->set_udp_port(port);
            }
                Logger::Debug(_T("UpdateUDPPort %d"), user_id);
                break;

            default:
                ;
        }
    }

    //if (char_data_providers_.find(user_id) == char_data_providers_.end()) {
    //    if (player->login() && player->model_name().size() > 0) {
    //        auto command_manager = manager_accessor_->command_manager().lock();
    //        if (user_id != command_manager->user_id()) {
    //            AddCharacter<PlayerCharacter>(user_id, unicode::ToTString(player->model_name()));
    //        }
    //    }
    //}

}

PlayerPtr PlayerManager::GetMyself()
{
    if (auto command_manager = manager_accessor_->command_manager().lock()) {
        return GetFromId(command_manager->user_id());
    } else {
        return PlayerPtr();
    }
}

PlayerPtr PlayerManager::GetFromId(unsigned int user_id)
{
    if (login_players_.find(user_id) != login_players_.end()) {
        return login_players_[user_id];
    } else {
        return PlayerPtr();
    }
}

std::vector<PlayerPtr> PlayerManager::GetAll()
{
    std::vector<PlayerPtr> players;
    BOOST_FOREACH (auto pair, login_players_) {
        if (pair.second->login()) {
            players.push_back(pair.second);
        }
    }
    return players;
}

void PlayerManager::UpdatePlayerPosition(unsigned int user_id, const PlayerPosition& pos)
{
    if (char_data_providers_.find(user_id) != char_data_providers_.end()) {
        char_data_providers_[user_id]->set_target_position(VGet(pos.x, pos.y, pos.z));
		char_data_providers_[user_id]->set_vy(pos.vy);
    }
}

std::shared_ptr<CharacterManager> PlayerManager::charmgr() const
{
    return charmgr_;
}

TimerPtr PlayerManager::timer() const
{
    return timer_;
}

std::map<unsigned int, std::unique_ptr<CharacterDataProvider>>& PlayerManager::char_data_providers()
{
    return char_data_providers_;
}

// 指定されたキャラIDとモデル番号を持つキャラをcharmgrに追加する
template <typename CharacterType>
void PlayerManager::AddCharacter(unsigned int user_id, const tstring& model_name)
{
    auto world_manager = manager_accessor_->world_manager().lock();
    std::unique_ptr<CharacterDataProvider> cdp_ptr(new CharacterDataProvider());
    auto& cdp = *cdp_ptr;
    char_data_providers_[user_id] = move(cdp_ptr);
    cdp.set_id(user_id);
    cdp.set_model(ResourceManager::LoadModelFromName(model_name));
	ResourceManager::SetModelEdgeSize(cdp.model().handle());
    auto character = std::make_shared<CharacterType>(cdp, stage_ptr_holder_, timer_);
    charmgr_->Add(user_id, character);
}

void PlayerManager::RemoveCharacter(unsigned int user_id)
{
    charmgr_->Remove(user_id);

    assert(char_data_providers_.find(user_id) != char_data_providers_.end());
    if(char_data_providers_.find(user_id) != char_data_providers_.end()) {
		char_data_providers_.erase(user_id);
	}
}
