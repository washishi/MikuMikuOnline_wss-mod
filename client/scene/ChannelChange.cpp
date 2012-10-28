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
    player_manager_(manager_accessor->player_manager().lock()),
	channel_(channel),
	fade_counter_(0)
{

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
	if (fade_counter_ < 120) {
		fade_counter_++;
	} else {
		// 古いステージを削除
		player_manager_->ResetStage();
		manager_accessor_->set_world_manager(WorldManagerPtr());

		command_manager_->Write(network::ServerUpdateAccountProperty(CHANNEL, network::Utils::Serialize(channel_)));

		auto channel_ptr = command_manager_->channels().at(channel_);
		StagePtr stage = std::make_shared<Stage>(channel_ptr,manager_accessor_->config_manager().lock());
		world_manager_ = std::make_shared<WorldManager>(stage, manager_accessor_);
		manager_accessor_->set_world_manager(world_manager_);

		next_scene_ = std::make_shared<scene::MainLoop>(manager_accessor_);
	}
}

void ChannelChange::ProcessInput(InputManager* input)
{

}

void ChannelChange::Draw()
{
	if (fade_counter_ < 120) {
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255 * fade_counter_ / 120);
		int width, height;
		GetScreenState(&width, &height, nullptr);
		DrawBox(0, 0, width, height, GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

}