//
// ConfigManager.cpp
//

#include "ConfigManager.hpp"
#include "../common/Logger.hpp"
#include <stdint.h>
#include <boost/filesystem.hpp> // ※ロビーサーバをjson参照するために追加

const int ConfigManager::MIN_SCREEN_WIDTH = 800;
const int ConfigManager::MIN_SCREEN_HEIGHT = 600;

const char* ConfigManager::CONFIG_JSON_PATH = "./config.json";

using namespace boost::property_tree;

ConfigManager::ConfigManager(const ManagerAccessorPtr& manager_accessor) :
manager_accessor_(manager_accessor)
{
	LoadConfigure();
	Load("./user/options.xml");
}

ConfigManager::~ConfigManager()
{
	Save("./user/options.xml");
}

void ConfigManager::LoadConfigure()
{
	ptree pt;
    read_json(CONFIG_JSON_PATH, pt);

    fullscreen_ = pt.get<bool>("fullscreen", false);
    screen_width_ = pt.get<int>("screen_width", 800);
    screen_height_ = pt.get<int>("screen_height", 600);
    antialias_ = pt.get<bool>("antialias", false);
    host_ = pt.get<std::string>("host", "127.0.0.1");
    port_ = pt.get<uint16_t>("port", 39390);
    max_script_execution_time_ = pt.get<int>("max_script_execution_time", 5000);
    max_local_storage_size_ = pt.get<int>("max_local_storage_size", 512000);
    upnp_ = pt.get<bool>("upnp", false);
    udp_port_ = pt.get<uint16_t>("udp_port", 39391);
	model_edge_size_ = pt.get<float>("edge_size",1.0f);
	stage_ = pt.get<std::string>("stage","ケロリン町");
	language_ = pt.get<std::string>("language","日本語");
	screen_width_ =  std::max(screen_width_, MIN_SCREEN_WIDTH);
	screen_height_ = std::max(screen_height_, MIN_SCREEN_HEIGHT);
	// ※ロビーサーバをjsonで変更できるように追加
	auto lobby_servers = pt.get_child("lobby_servers", ptree());
	BOOST_FOREACH(const auto& item, lobby_servers) {
		lobby_servers_.push_back(item.second.get_value<std::string>());
	}
	// ここまで
}

void ConfigManager::Load(const std::string& filename)
{
	ptree pt;

    namespace fs = boost::filesystem;
    if (fs::exists(filename)) {
        read_xml(filename, pt);
    }

    show_nametag_ =		pt.get("show_nametag", 1);
    show_modelname_ =	pt.get("show_modelname", 1);
    gamepad_type_ =		pt.get("gamepad_type", 0);
    gamepad_enable_ =	pt.get("gamepad_enable", 0); // ※ ゲームパッド有効をウインドウアクティブ時のみにもできる様に追加
    camera_direction_ =	pt.get("camera_direction", 0);
	walk_change_type_ = pt.get("walk_change_type",0);
}

void ConfigManager::Save(const std::string& filename)
{
    namespace fs = boost::filesystem;
	auto dir_path = fs::path(filename).parent_path();
	if (!fs::exists(dir_path)) {
		fs::create_directory(dir_path);
    }

	ptree pt;

    pt.put("show_nametag", show_nametag_);
    pt.put("show_modelname", show_modelname_);
    pt.put("gamepad_type", gamepad_type_);
    pt.put("gamepad_enable", gamepad_enable_); // ※ ゲームパッド有効をウインドウアクティブ時のみにもできる様に追加
    pt.put("camera_direction", camera_direction_);

	write_xml(filename, pt);
}

bool ConfigManager::fullscreen() const
{
    return fullscreen_;
}

int ConfigManager::screen_width() const
{
    return screen_width_;
}

int ConfigManager::screen_height() const
{
    return screen_height_;
}

bool ConfigManager::antialias() const
{
    return antialias_;
}

std::string ConfigManager::host() const
{
    return host_;
}

int ConfigManager::port() const
{
    return port_;
}

int ConfigManager::max_script_execution_time() const
{
    return max_script_execution_time_;
}

int ConfigManager::max_local_storage_size() const
{
    return max_local_storage_size_;
}

bool ConfigManager::upnp() const
{
    return upnp_;
}

int ConfigManager::udp_port() const
{
    return udp_port_;
}

const std::string& ConfigManager::language() const
{
	return language_;
}

bool  ConfigManager::shader_blur() const
{
	return shader_blur_;
}
bool  ConfigManager::shader_bloom() const
{
	return shader_bloom_;
}
bool  ConfigManager::shader_shadow() const
{
	return shader_shadow_;
}

bool  ConfigManager::shader_depth_field() const
{
	return shader_depth_field_;
}

float ConfigManager::model_edge_size() const
{
	return model_edge_size_;
}

std::string ConfigManager::stage() const
{
	return stage_;
}

int ConfigManager::show_nametag() const
{
	return show_nametag_;
}

void ConfigManager::set_show_nametag(int value)
{
	show_nametag_ = value;
}

int ConfigManager::show_modelname() const
{
	return show_modelname_;
}

void ConfigManager::set_show_modelname(int value)
{
	show_modelname_ = value;
}

int ConfigManager::walk_change_type() const
{
	return walk_change_type_;
}
void ConfigManager::set_walk_change_type(int value)
{
	walk_change_type_ = value;
}


int ConfigManager::gamepad_type() const
{
	return gamepad_type_;
}

void ConfigManager::set_gamepad_type(int value)
{
	gamepad_type_ = value;
}

// ※ ここから ゲームパッド有効をウインドウアクティブ時のみにもできる様に追加
int ConfigManager::gamepad_enable() const
{
	return gamepad_enable_;
}

void ConfigManager::set_gamepad_enable(int value)
{
	gamepad_enable_ = value;
}
// ※ ここまで

int ConfigManager::camera_direction() const
{
	return camera_direction_;
}

void ConfigManager::set_camera_direction(int value)
{
	camera_direction_ = value;
}

const std::list<std::string>& ConfigManager::lobby_servers() const
{
	return lobby_servers_;
}