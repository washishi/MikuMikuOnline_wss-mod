//
// MainLoop.cpp
//

#include "MainLoop.hpp"
#include <vector>
#include <algorithm>
#include "../ResourceManager.hpp"

namespace scene {
MainLoop::MainLoop(const ManagerAccessorPtr& manager_accesor) :
      manager_accesor_(manager_accesor),
      player_manager_(std::make_shared<PlayerManager>(manager_accesor_)),
      card_manager_(manager_accesor->card_manager().lock()),
      command_manager_(std::make_shared<CommandManager>(manager_accesor_)),
      world_manager_(std::make_shared<WorldManager>(manager_accesor_)),
      account_manager_(manager_accesor->account_manager().lock()),
      config_manager_(manager_accesor->config_manager().lock()),
      inputbox_(manager_accesor_)
{
    manager_accesor_->set_player_manager(player_manager_);
    manager_accesor_->set_command_manager(command_manager_);
    manager_accesor_->set_world_manager(world_manager_);

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

    world_manager_->ProcessInput(&input);
    world_manager_->Update();
}

void MainLoop::Draw()
{
    world_manager_->Draw();
    player_manager_->Draw();
    card_manager_->Draw();
    inputbox_.Draw();
}

void MainLoop::End()
{
}

}
