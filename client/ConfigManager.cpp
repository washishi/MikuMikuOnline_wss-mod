//
// ConfigManager.cpp
//

#include "ConfigManager.hpp"
#include "../common/Logger.hpp"
#include <stdint.h>

const int ConfigManager::MIN_SCREEN_WIDTH = 800;
const int ConfigManager::MIN_SCREEN_HEIGHT = 600;

ConfigManager::ConfigManager(const ManagerAccessorPtr& manager_accessor) :
manager_accessor_(manager_accessor)
{
}

void ConfigManager::Load(const std::string& filename)
{
    read_json(filename, pt_);

    fullscreen_ = pt_.get<bool>("fullscreen", false);
    screen_width_ = pt_.get<int>("screen_width", 800);
    screen_height_ = pt_.get<int>("screen_height", 600);
    antialias_ = pt_.get<bool>("antialias", false);
    host_ = pt_.get<std::string>("host", "127.0.0.1");
    port_ = pt_.get<uint16_t>("port", 39390);
    max_script_execution_time_ = pt_.get<int>("max_script__execution_time", 5000);
    max_local_storage_size_ = pt_.get<int>("max_local_storage_size", 512000);
    upnp_ = pt_.get<bool>("upnp", false);
    udp_port_ = pt_.get<uint16_t>("udp_port", 39391);
	model_edge_size_ = pt_.get<float>("edge_size",1.0f);
	stage_ = pt_.get<std::string>("stage","ÉPÉçÉäÉìí¨");

	screen_width_ =  std::max(screen_width_, MIN_SCREEN_WIDTH);
	screen_height_ = std::max(screen_height_, MIN_SCREEN_HEIGHT);
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