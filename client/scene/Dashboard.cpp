//
// Dashboard.cpp
//

#include "Dashboard.hpp"
#include "../ManagerAccessor.hpp"
#include "../AccountManager.hpp"
#include "../CommandManager.hpp"
#include "../PlayerManager.hpp"
#include "../CardManager.hpp"
#include "../AccountManager.hpp"
#include "../WindowManager.hpp"

namespace scene {

const int Dashboard::TAB_BLOCK_HEIGHT = 32;
const int Dashboard::TAB_BLOCK_WIDTH = 128;

Dashboard::Dashboard(const ManagerAccessorPtr& manager_accessor,
			const BasePtr& background_scene) :
      manager_accessor_(manager_accessor),
      config_manager_(manager_accessor->config_manager().lock()),
      card_manager_(manager_accessor->card_manager().lock()),
      account_manager_(manager_accessor->account_manager().lock()),
      window_manager_(manager_accessor->window_manager().lock()),
	  background_scene_(background_scene),
      start_count_(0),
	  end_count_(0)
{
	manager_accessor_->set_window_manager(WindowManagerWeakPtr());

    manager_accessor_->set_config_manager(config_manager_);
    manager_accessor_->set_card_manager(card_manager_);
    manager_accessor_->set_account_manager(account_manager_);

    base_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("resources/images/gui/gui_option_base.png"), 2, 2, 96, 96);

    tab_base_image_handle_ = ResourceManager::LoadCachedDivGraph<3>(
            _T("resources/images/gui/gui_option_tab_base.png"), 1, 3, 128, 32);

}

Dashboard::~Dashboard()
{
}

void Dashboard::Begin()
{
}

void Dashboard::Update()
{
	if (end_count_ > 0) {
		end_count_++;
	} else {
		start_count_++;
	}

	if (background_scene_) {
		background_scene_->Update();
	}

    int screen_width, screen_height;
    GetScreenState(&screen_width, &screen_height, nullptr);

	if (end_count_ > 0) {
		bg_alpha_ = 150 * ((10 - end_count_) / 10.0);
	} else {
		bg_alpha_ = 150 * (start_count_ / 10.0);
	}
	bg_alpha_ = std::min(150, bg_alpha_);
	
    base_rect_.width = 800;
    base_rect_.height = 160;
    base_rect_.x = (screen_width - base_rect_.width) / 2;

	if (end_count_ > 0) {
		base_rect_.y = -100 - base_rect_.height * (end_count_ / 10.0);
	} else {
		base_rect_.y = -100 - base_rect_.height * ((10 - start_count_) / 10.0);
	}
	base_rect_.y = std::min(-100, base_rect_.y);

	window_manager_->Update();
}

void Dashboard::ProcessInput(InputManager* input)
{
	window_manager_->ProcessInputIcons(base_rect_, input);
	window_manager_->ProcessInput(input);

	bool hover = (base_rect_.x <= input->GetMouseX() && input->GetMouseX() <= base_rect_.x + base_rect_.width
        && base_rect_.y <= input->GetMouseY() && input->GetMouseY() <= base_rect_.y + base_rect_.height);

	if (input->GetKeyCount(KEY_INPUT_F2) == 1 ||
		input->GetKeyCount(KEY_INPUT_ESCAPE) == 1 ||
		(!hover && (input->GetMouseLeftCount() == 1 || input->GetMouseRightCount() == 1))) {
		end_count_++;
	}
}

void Dashboard::Draw()
{
	if (background_scene_) {
		background_scene_->Draw();
	}

    int screen_width, screen_height;
    GetScreenState(&screen_width, &screen_height, nullptr);
	
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, bg_alpha_);

	DrawBox(0, 0, screen_width, screen_height, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	window_manager_->Draw();

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, bg_alpha_ * 255 / 150);
	window_manager_->DrawButtons();
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	int BASE_BLOCK_SIZE = 96;

	int x = base_rect_.x - 32;
	int y = base_rect_.y;
	int width = base_rect_.width + 64;
	int height = base_rect_.height + 32;

    DrawGraph(x, y, *base_image_handle_[0], TRUE);
    DrawGraph(x + width - BASE_BLOCK_SIZE, y, *base_image_handle_[1], TRUE);
    DrawGraph(x, y + height - BASE_BLOCK_SIZE, *base_image_handle_[2], TRUE);
    DrawGraph(x + width - BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE, *base_image_handle_[3], TRUE);

    DrawRectExtendGraphF(x + BASE_BLOCK_SIZE, y,
                         x + width - BASE_BLOCK_SIZE, y + BASE_BLOCK_SIZE,
                         0, 0, 1, BASE_BLOCK_SIZE, *base_image_handle_[1], TRUE);

    DrawRectExtendGraphF(x + BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE,
                         x + width - BASE_BLOCK_SIZE, y + height,
                         0, 0, 1, BASE_BLOCK_SIZE, *base_image_handle_[3], TRUE);

    DrawRectExtendGraphF(x, y + BASE_BLOCK_SIZE,
                         x + BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE,
                         0, 0, BASE_BLOCK_SIZE, 1, *base_image_handle_[2], TRUE);

    DrawRectExtendGraphF(x + width - BASE_BLOCK_SIZE, y + BASE_BLOCK_SIZE,
                         x + width, y + height - BASE_BLOCK_SIZE,
                         0, 0, BASE_BLOCK_SIZE, 1, *base_image_handle_[3], TRUE);

    DrawRectExtendGraphF(x + BASE_BLOCK_SIZE, y + BASE_BLOCK_SIZE,
                         x + width - BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE,
                         0, 0, 1, 1, *base_image_handle_[3], TRUE);

	window_manager_->DrawIcons(base_rect_);

}

void Dashboard::End()
{

}

BasePtr Dashboard::NextScene()
{
    if (end_count_ > 10) {
		manager_accessor_->set_window_manager(window_manager_);
        return background_scene_;
    } else {
        return BasePtr();
    }
}
}