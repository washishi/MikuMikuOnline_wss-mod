//
// Title.cpp
//

#include "Connect.hpp"
#include "Title.hpp"
#include <vector>
#include <algorithm>
#include "../ResourceManager.hpp"
#include "../AccountManager.hpp"
#include "../../common/Logger.hpp"

namespace scene {
Title::Title(const ManagerAccessorPtr& manager_accessor) :
              manager_accessor_(manager_accessor),
              card_manager_(manager_accessor->card_manager().lock()),
              account_manager_(manager_accessor->account_manager().lock()),
              config_manager_(manager_accessor->config_manager().lock()),
              connect_flag_(false),
              screen_count_(0)
{
}

Title::~Title()
{
}

void Title::Begin()
{
    int width, height;
    GetScreenState(&width, &height, nullptr);
    input_host_.set_active(true);
    input_host_.set_text(unicode::ToTString(account_manager_->host()));
    input_host_.set_width(200);
	input_host_.set_multiline(false);
    input_host_.set_x((width - input_host_.width()) / 2);
    input_host_.set_y((height - input_host_.height()) / 2 - 30);
    input_host_.set_message(_T("ホストアドレスを入力："));

    input_host_.set_on_enter([this](const std::string& text) -> bool{
        connect_flag_ = true;
        return false;
    });

    label_.set_width(30);
    label_.set_text(_T("接続"));
    label_.set_bgcolor(UIBase::Color(0,0,0,0));
    label_.set_textcolor(UIBase::Color(0,0,0,255));
    label_.set_left((width - 26) / 2);
    label_.set_top((height - input_host_.height()) / 2 + 40);

    button_.set_height(32);
    button_.set_left((width - button_.width()) / 2);
    button_.set_top((height - button_.height()) / 2 + 40);

    button_.set_on_click([this](){
        connect_flag_ = true;
    });

}

void Title::Update()
{
    input_host_.Update();

    button_.Update();
    label_.Update();

    if (connect_flag_) {
        screen_count_++;
    }

    if (screen_count_ > 30) {
        account_manager_->set_host(unicode::ToString(input_host_.text()));
        next_scene_ = BasePtr(new scene::Connect(manager_accessor_));
    }
}

void Title::ProcessInput(InputManager* input)
{
    input_host_.ProcessInput(input);
    button_.ProcessInput(input);
}

void Title::Draw()
{

    int width, height;
    GetScreenState(&width, &height, nullptr);
    DrawBox(0, 0, width, height, GetColor(157, 212, 187), TRUE);

    input_host_.Draw();
    button_.Draw();
    label_.Draw();

	int alpha = std::min(255, screen_count_ * 10);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	DrawBox(0, 0, width, height, GetColor(157, 212, 187), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Title::End()
{
}

}
