//
// Init.cpp
//

#include "Init.hpp"
#include "Title.hpp"
#include <vector>
#include <algorithm>
#include "../../common/Logger.hpp"
#include "../ResourceManager.hpp"
#include "../ManagerAccessor.hpp"
#include "../ConfigManager.hpp"
#include "../CardManager.hpp"
#include "../AccountManager.hpp"
#include "../WindowManager.hpp"
#include "GenerateJSON.hpp"
#include "../Music.hpp"

namespace scene {
Init::Init() :
      manager_accessor_(std::make_shared<ManagerAccessor>()),
      config_manager_(std::make_shared<ConfigManager>(manager_accessor_)),
      card_manager_(std::make_shared<CardManager>(manager_accessor_)),
      account_manager_(std::make_shared<AccountManager>(manager_accessor_)),
	  window_manager_(std::make_shared<WindowManager>(manager_accessor_)),
      start_count_(0),
      loaded_(false)
{
    manager_accessor_->set_config_manager(config_manager_);
    manager_accessor_->set_card_manager(card_manager_);
    manager_accessor_->set_account_manager(account_manager_);
    manager_accessor_->set_window_manager(window_manager_);

    loading_image_handle_ = ResourceManager::LoadCachedDivGraph<5>(
            _T("system/images/loading.png"), 1, 5, 145, 23);

}

Init::~Init()
{
}

void Init::Begin()
{
    //loading_thread_ = boost::thread([this](){
        AsyncInitialize();
    //});
}

void Init::Update()
{
    start_count_++;
}

void Init::ProcessInput(InputManager* input)
{

}

void Init::Draw()
{
    int bright = start_count_ * 10;
    SetDrawBright(bright, bright, bright);
    int width, height;
    GetScreenState(&width, &height, nullptr);
    DrawBox(0, 0, width, height, GetColor(157, 212, 187), TRUE);

    {
        boost::mutex::scoped_lock(mutex_);
        if (!loaded_) {
            int image_index = (start_count_ / 20) % 5;
            DrawGraph((width - 145) / 2, (height - 23) / 2,
                    *loading_image_handle_[image_index], TRUE);
        }
    }
}

void Init::End()
{
    loading_thread_.join();
}

BasePtr Init::NextScene()
{
    if (loaded_ && start_count_ > 30) {
        return BasePtr(new scene::Title(manager_accessor_));
    } else {
        return BasePtr();
    }
}

void Init::AsyncInitialize()
{
    {
    boost::mutex::scoped_lock(mutex_);
    loaded_ = false;
    }
	JsonGen jsongen;
//
    config_manager_->Load("config.json");
	ResourceManager::set_model_edge_size(config_manager_->model_edge_size());
    ResourceManager::BuildModelFileTree();
	ResourceManager::music()->Init();

    account_manager_->Load("./user/account.xml");
//
//    // UDP受信用ポート開放
//    uint16_t udp_port = 39391;
////    std::string global_ip = "127.0.0.1";
////    if (PortMapper::MapUDP(&udp_port, &global_ip)) {
////        Logger::Info("Open udp port %d", udp_port);
////        Logger::Info("Global IP address is %s", global_ip);
////    } else {
////        Logger::Error("Cannot open udp port");
////    }
//    account_manager_->set_udp_port(udp_port);

    // スクリプト
    Card::set_max_local_storage_size(config_manager_->max_local_storage_size());
    card_manager_->Load("./wigets");
    card_manager_->Load("./system/wigets");

    {
    boost::mutex::scoped_lock(mutex_);
    loaded_ = true;
    }
}

}
