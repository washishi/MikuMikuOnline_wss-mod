//
// ChannelChange.cpp
//

#include "MainLoop.hpp"
#include "ChannelChange.hpp"
#include "../CommandManager.hpp"
#include "../../common/Logger.hpp"
#include "../../common/network/Utils.hpp"
#include "../3d/Stage.hpp"

namespace scene {
ChannelChange::ChannelChange(unsigned char channel, const ManagerAccessorPtr& manager_accessor) :
	manager_accessor_(manager_accessor),
    card_manager_(manager_accessor->card_manager().lock()),
    command_manager_(manager_accessor->command_manager().lock()),
    account_manager_(manager_accessor->account_manager().lock()),
    config_manager_(manager_accessor->config_manager().lock()),
    player_manager_(manager_accessor->player_manager().lock())
{

	auto channel_str = (unsigned char)channel;
	command_manager_->Write(network::ServerUpdateAccountProperty(CHANNEL, network::Utils::Serialize(channel_str)));

	auto channel_ptr = command_manager_->channels().at(channel);
	StagePtr stage = std::make_shared<Stage>(channel_ptr);
	world_manager_ = std::make_shared<WorldManager>(stage, manager_accessor);
    manager_accessor_->set_world_manager(world_manager_);
}

ChannelChange::~ChannelChange()
{
}

void ChannelChange::Begin()
{

}

void ChannelChange::End()
{
}

void ChannelChange::Update()
{
	//if(world_manager_->stage()->host_change_flag())
	//{
		//account_manager_->set_host(world_manager_->stage()->host_change_flag().second);
		next_scene_ = std::make_shared<scene::MainLoop>(manager_accessor_);
	//}
}

void ChannelChange::ProcessInput(InputManager* input)
{

}

void ChannelChange::Draw()
{
}

}