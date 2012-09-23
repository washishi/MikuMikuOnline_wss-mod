//
// ServerChange.cpp
//

#include "MainLoop.hpp"
#include "ServerChange.hpp"
#include "../../common/Logger.hpp"

namespace scene {
ServerChange::ServerChange(const ManagerAccessorPtr& manager_accessor) :
	manager_accessor_(manager_accessor),
	card_manager_(manager_accessor->card_manager().lock()),
	account_manager_(manager_accessor->account_manager().lock()),
	config_manager_(manager_accessor->config_manager().lock()),
    world_manager_(manager_accessor->world_manager().lock())
{
}

ServerChange::~ServerChange()
{
}

void ServerChange::Begin()
{
}

void ServerChange::End()
{
}

void ServerChange::Update()
{
}

void ServerChange::ProcessInput(InputManager* input)
{

}

void ServerChange::Draw()
{
}

BasePtr ServerChange::NextScene()
{
	if(world_manager_->stage()->host_change_flag())
	{
		//account_manager_->set_host(world_manager_->stage()->host_change_flag().second);
		return BasePtr(new scene::MainLoop(manager_accessor_));
	}else{
		return nullptr;
	}
}


}