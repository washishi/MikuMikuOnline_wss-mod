//
// MainLoop.cpp
//

#include "MainLoop.hpp"
#include <vector>
#include <algorithm>
#include "../ResourceManager.hpp"
#include "../Core.hpp"
#include <shlwapi.h>
#include "ServerChange.hpp"

namespace scene {
MainLoop::MainLoop(const ManagerAccessorPtr& manager_accessor) :
      manager_accessor_(manager_accessor),
      player_manager_(std::make_shared<PlayerManager>(manager_accessor_)),
      card_manager_(manager_accessor->card_manager().lock()),
      command_manager_(std::make_shared<CommandManager>(manager_accessor_)),
      world_manager_(std::make_shared<WorldManager>(manager_accessor_)),
      account_manager_(manager_accessor->account_manager().lock()),
      config_manager_(manager_accessor->config_manager().lock()),
      inputbox_(manager_accessor_),
	  minimap_(manager_accessor_),
	  snapshot_number_(0)
{
    manager_accessor_->set_player_manager(player_manager_);
    manager_accessor_->set_command_manager(command_manager_);
    manager_accessor_->set_world_manager(world_manager_);

    inputbox_.ReloadTabs();

    command_manager_->set_client(
            ClientUniqPtr(
                new network::Client(
                        account_manager_->host(),
                        config_manager_->port(),
                        (uint16_t)config_manager_->udp_port(),
                        account_manager_->public_key(),
                        account_manager_->private_key(),
                        "server_key.pub",
                        config_manager_->upnp()
                )
            )
    );

	minimap_.UIPlacement(config_manager_->screen_width() - MINIMAP_MINSIZE - 12, 12);
    player_manager_->Init();
    world_manager_->Init();	

    world_manager_->myself()->Init(unicode::ToTString(account_manager_->model_name()));
}

MainLoop::~MainLoop()
{
    account_manager_->Save("account.xml");
}

void MainLoop::Begin()
{

}

void MainLoop::Update()
{
    command_manager_->Update();

    InputManager input;

    inputbox_.ProcessInput(&input);
    inputbox_.Update();

    player_manager_->ProcessInput(&input);
    player_manager_->Update();
    
    card_manager_->ProcessInput(&input);
    card_manager_->Update();

	minimap_.ProcessInput(&input);
	minimap_.Update();

    world_manager_->ProcessInput(&input);
    world_manager_->Update();

}

void MainLoop::Draw()
{
    world_manager_->Draw();
    player_manager_->Draw();
    card_manager_->Draw();
    inputbox_.Draw();
	minimap_.Draw();

    InputManager input;

	ProcessInput(&input);
}

void MainLoop::End()
{
}

void MainLoop::ProcessInput(InputManager *input)
{
	if(input->GetKeyCount(InputManager::KEYBIND_SCREEN_SHOT) > 0 && !inputbox_.IsActive())
	{
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
		while(input->GetKeyCount(InputManager::KEYBIND_SCREEN_SHOT) > 0)
		{
			input->Update();
		}
	}
}

BasePtr MainLoop::NextScene()
{
	if(world_manager_->stage()->host_change_flag().first)
	{
		account_manager_->set_host(world_manager_->stage()->host_change_flag().second);
		return BasePtr(new scene::ServerChange(manager_accessor_));
	}else{
		return nullptr;
	}
}

}

