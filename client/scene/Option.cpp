//
// Option.cpp
//

#include "Option.hpp"

namespace scene {

Option::Option(const ManagerAccessorPtr& manager_accessor,
			const BasePtr& background_scene) :
      manager_accesor_(manager_accessor),
      config_manager_(manager_accessor->config_manager().lock()),
      card_manager_(manager_accessor->card_manager().lock()),
      account_manager_(manager_accessor->account_manager().lock()),
	  background_scene_(background_scene),
      start_count_(0)
{
    manager_accesor_->set_config_manager(config_manager_);
    manager_accesor_->set_card_manager(card_manager_);
    manager_accesor_->set_account_manager(account_manager_);

}

Option::~Option()
{
}

void Option::Begin()
{
 
}

void Option::Update()
{
    start_count_++;
	if (background_scene_) {
		background_scene_->Update();
	}
}

void Option::ProcessInput(InputManager* input)
{

}

void Option::Draw()
{
	if (background_scene_) {
		background_scene_->Draw();
	}

    int width, height;
    GetScreenState(&width, &height, nullptr);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
	DrawBox(0, 0, width, height, GetColor(157, 212, 187), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Option::End()
{

}

BasePtr Option::NextScene()
{
	InputManager input;
    if (input.GetKeyCount(KEY_INPUT_F1) == 1) {
        return background_scene_;
    } else {
        return BasePtr();
    }
}

}