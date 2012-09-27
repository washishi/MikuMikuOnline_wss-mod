//
// ConfigManager.cpp
//

#include "ConfigManager.hpp"
#include "../common/Logger.hpp"
#include <stdint.h>

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
	stage_ = pt.get<std::string>("stage","ÉPÉçÉäÉìí¨");

	screen_width_ =  std::max(screen_width_, MIN_SCREEN_WIDTH);
	screen_height_ = std::max(screen_height_, MIN_SCREEN_HEIGHT);
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
    bouyomi_chan_ =		pt.get("bouyomi_chan", 0);
}

void ConfigManager::Save(const std::string& filename)
{
	ptree pt;

    pt.put("show_nametag", show_nametag_);
    pt.put("show_modelname", show_modelname_);
    pt.put("gamepad_type", gamepad_type_);
    pt.put("bouyomi_chan", bouyomi_chan_);

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

int ConfigManager::gamepad_type() const
{
	return gamepad_type_;
}

void ConfigManager::set_gamepad_type(int value)
{
	gamepad_type_ = value;
}

int ConfigManager::bouyomi_chan() const
{
	return bouyomi_chan_;
}

void ConfigManager::set_bouyomi_chan(int value)
{
	bouyomi_chan_ = value;
}
