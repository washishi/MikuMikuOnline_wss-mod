//
// MainLoop.cpp
//

#include "MainLoop.hpp"
#include "Option.hpp"
#include "Dashboard.hpp"
#include "ChannelChange.hpp"
#include <vector>
#include <algorithm>
#include "../PlayerManager.hpp"
#include "../CardManager.hpp"
#include "../CommandManager.hpp"
#include "../WorldManager.hpp"
#include "../AccountManager.hpp"
#include "../ConfigManager.hpp"
#include "../ResourceManager.hpp"
#include "../WindowManager.hpp"
#include "../SocketServerManager.hpp"
#include "../Core.hpp"
#include <shlwapi.h>
#include "ServerChange.hpp"
#include "../Music.hpp"

namespace scene {
MainLoop::MainLoop(const ManagerAccessorPtr& manager_accessor) :
      manager_accessor_(manager_accessor),
      player_manager_(manager_accessor->player_manager().lock()),
      card_manager_(manager_accessor->card_manager().lock()),
      command_manager_(manager_accessor->command_manager().lock()),
      world_manager_(manager_accessor->world_manager().lock()),
      account_manager_(manager_accessor->account_manager().lock()),
      config_manager_(manager_accessor->config_manager().lock()),
      window_manager_(std::make_shared<WindowManager>(manager_accessor_)),
	  socket_server_manager_(std::make_shared<SocketServerManager>(manager_accessor_)),
      inputbox_(std::make_shared<InputBox>(manager_accessor_)),
	  minimap_(std::make_shared<MiniMap>(manager_accessor)),
	  snapshot_number_(0),
	  snapshot_(false)
{
    manager_accessor_->set_window_manager(window_manager_);

    inputbox_->ReloadTabs();
	inputbox_->Activate();
	card_manager_->AddNativeCard("inputbox", inputbox_);

	minimap_->UIPlacement(config_manager_->screen_width() - MINIMAP_MINSIZE - 12, 12);
	card_manager_->AddNativeCard("radar", minimap_);

	window_manager_->RestorePosition();

    player_manager_->Init();
    world_manager_->Init();	

    world_manager_->myself()->Init(unicode::ToTString(account_manager_->model_name()));

	socket_server_manager_->Start();
}

MainLoop::~MainLoop()
{
	window_manager_->SavePosition();
    account_manager_->Save("./user/account.xml");
}

void MainLoop::Begin()
{

}

void MainLoop::Update()
{
	if (auto window_manager = manager_accessor_->window_manager().lock()) {
		window_manager->Update();
	}
    command_manager_->Update();
    player_manager_->Update();
    card_manager_->Update();
    world_manager_->Update();
	ResourceManager::music()->Update();

}

void MainLoop::ProcessInput(InputManager* input)
{
	if(world_manager_->stage()->host_change_flag())
	{
		//account_manager_->set_host(world_manager_->stage()->host_change_flag().second);
		next_scene_ = std::make_shared<scene::ServerChange>(manager_accessor_);
	} else if (input->GetKeyCount(KEY_INPUT_F1) == 1) {
		inputbox_->Inactivate();
		next_scene_ = std::make_shared<scene::Option>(manager_accessor_, shared_from_this());
	} else if (input->GetKeyCount(KEY_INPUT_F2) == 1) {
		inputbox_->Inactivate();
		next_scene_ = std::make_shared<scene::Dashboard>(manager_accessor_, shared_from_this());
	}

	if (auto window_manager = manager_accessor_->window_manager().lock()) {
		window_manager->ProcessInput(input);
	}
    player_manager_->ProcessInput(input);
    card_manager_->ProcessInput(input);
    world_manager_->ProcessInput(input);

	if(input->GetKeyCount(InputManager::KEYBIND_SCREEN_SHOT) == 1)
	{
		snapshot_ = true;
	}

	if (const auto& channel = command_manager_->current_channel()) {
		BOOST_FOREACH(const auto& warp_point, channel->warp_points) {
			auto point = VGet(warp_point.x, warp_point.y + 30, warp_point.z);
			const auto& pos = player_manager_->GetMyself()->position();

			auto distance = VSize(VGet(warp_point.x - pos.x, warp_point.y - pos.y, warp_point.z - pos.z));
			if (distance < 50 && input->GetKeyCount(KEY_INPUT_M) == 1) {
				next_scene_ = std::make_shared<scene::ChannelChange>(warp_point.channel, manager_accessor_);
			}
		}
	}

}

void MainLoop::Draw()
{
    world_manager_->Draw();
    player_manager_->Draw();
    card_manager_->Draw();
	if (auto window_manager = manager_accessor_->window_manager().lock()) {
		window_manager->Draw();
	}

	if (snapshot_) {
		using namespace boost::filesystem;
		if (!exists("./screenshot")) {
			create_directory("./screenshot");
		}
		TCHAR tmp_str[MAX_PATH];
		_stprintf( tmp_str , _T(".\\screenshot\\ss%03d.png") , snapshot_number_ );
		if(PathFileExists(tmp_str))
		{
			while(1)
			{
				snapshot_number_++;
				_stprintf( tmp_str , _T(".\\screenshot\\ss%03d.png") , snapshot_number_ );
				if(!PathFileExists(tmp_str))break;
			}
		}
		SaveDrawScreenToPNG( 0, 0, config_manager_->screen_width(), config_manager_->screen_height(),tmp_str);
		snapshot_number_++;
		snapshot_ = false;
	}
	
	if (const auto& channel = command_manager_->current_channel()) {
		BOOST_FOREACH(const auto& warp_point, channel->warp_points) {
			auto point = VGet(warp_point.x, warp_point.y + 30, warp_point.z);
			const auto& pos = player_manager_->GetMyself()->position();

			auto distance = VSize(VGet(warp_point.x - pos.x, warp_point.y - pos.y, warp_point.z - pos.z));

			if (world_manager_->stage()->IsVisiblePoint(point)) {
				auto screen_pos = ConvWorldPosToScreenPos(point);
				int x = (screen_pos.x / 2) * 2;
				int y = (screen_pos.y / 2) * 2 - 16;

				UILabel label_;
				label_.set_width(160);
				if (distance < 50) {
					label_.set_text(unicode::ToTString(warp_point.name) + _T("\n‚lƒL[‚Å“]‘—‚µ‚Ü‚·"));
					label_.set_bgcolor(UIBase::Color(255,0,0,150));
				} else {
					label_.set_text(unicode::ToTString(warp_point.name));
					label_.set_bgcolor(UIBase::Color(0,0,0,150));
				}
				label_.set_textcolor(UIBase::Color(255,255,255,255));

				label_.set_left(x - 60);
				label_.set_top(y + 10);

				label_.Update();
				label_.Draw();
			}
		}
	}
}

void MainLoop::End()
{
	next_scene_ = BasePtr();
}

}

