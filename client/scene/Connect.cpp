//
// Connect.cpp
//

#include "MainLoop.hpp"
#include "Connect.hpp"
#include <vector>
#include <algorithm>
#include "../ResourceManager.hpp"
#include "../../common/Logger.hpp"

namespace scene {
Connect::Connect(const ManagerAccessorPtr& manager_accesor) :
              manager_accesor_(manager_accesor),
              card_manager_(manager_accesor->card_manager().lock()),
              account_manager_(manager_accesor->account_manager().lock()),
              config_manager_(manager_accesor->config_manager().lock()),
			  command_manager_(std::make_shared<CommandManager>(manager_accesor_))
{
    manager_accesor_->set_command_manager(command_manager_);
}

Connect::~Connect()
{
}

void Connect::Begin()
{
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
}

void Connect::Update()
{
	command_manager_->Update();
}

void Connect::Draw()
{
    int width, height;
    GetScreenState(&width, &height, nullptr);
    DrawBox(0, 0, width, height, GetColor(157, 212, 187), TRUE);

}

void Connect::End()
{
}

BasePtr Connect::NextScene()
{
    InputManager input;
    if (command_manager_->status() == CommandManager::STATUS_READY) {
        return BasePtr(new scene::MainLoop(manager_accesor_));
    } else {
      return BasePtr();
    }
}

}
