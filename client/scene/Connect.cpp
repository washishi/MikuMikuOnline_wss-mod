//
// Connect.cpp
//

#include "MainLoop.hpp"
#include "Title.hpp"
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
			  command_manager_(std::make_shared<CommandManager>(manager_accesor_)),
			  return_flag_(false)
{
    manager_accesor_->set_command_manager(command_manager_);
}

Connect::~Connect()
{
}

void Connect::Begin()
{

    int width, height;
    GetScreenState(&width, &height, nullptr);

    message_.set_width(400);
    message_.set_text(_T("�ڑ���..."));
    message_.set_bgcolor(UIBase::Color(0,0,0,0));
    message_.set_textcolor(UIBase::Color(0,0,0,255));
    message_.set_left(40);
    message_.set_top(40);

    button_label_.set_width(100);
    button_label_.set_text(_T("�L�����Z��"));
    button_label_.set_bgcolor(UIBase::Color(0,0,0,0));
    button_label_.set_textcolor(UIBase::Color(0,0,0,255));
    button_label_.set_left(width - button_.width() - 40 + 15);
    button_label_.set_top(height - button_.height() + 35);

    button_.set_height(32);
    button_.set_left(width - button_.width() - 40);
    button_.set_top(height - button_.height() - 40);

    button_.set_on_click([this](){
        return_flag_ = true;
    });


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

	switch (command_manager_->status()) {
	case CommandManager::STATUS_ERROR:
		message_.set_text(_T("�G���[�F�ڑ��Ɏ��s���܂���"));
		command_manager_->set_client(ClientUniqPtr());
		break;
	case CommandManager::STATUS_ERROR_CROWDED:
		message_.set_text(_T("�G���[�F�l�����������܂�"));
		command_manager_->set_client(ClientUniqPtr());
		break;
	case CommandManager::STATUS_ERROR_NOSTAGE:
		message_.set_text((tformat(_T("�G���[�F�ڑ�����ɂ̓X�e�[�W�f�[�^�u%s�v���K�v�ł�")) % 
			unicode::ToTString(command_manager_->stage())).str());
		command_manager_->set_client(ClientUniqPtr());
		break;
	}

    InputManager input;
    button_.ProcessInput(&input);
    button_.Update();
    button_label_.Update();
    message_.Update();
}

void Connect::Draw()
{
    int width, height;
    GetScreenState(&width, &height, nullptr);
    DrawBox(0, 0, width, height, GetColor(157, 212, 187), TRUE);
	
    message_.Draw();
    button_.Draw();
    button_label_.Draw();
}

void Connect::End()
{
}

BasePtr Connect::NextScene()
{
    InputManager input;
    if (command_manager_->status() == CommandManager::STATUS_READY) {
        return BasePtr(new scene::MainLoop(manager_accesor_));
	} else if (return_flag_) {
		return BasePtr(new scene::Title(manager_accesor_));
    } else {
      return BasePtr();
    }
}

}