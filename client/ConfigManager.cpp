//
// ConfigManager.cpp
//

#include "ConfigManager.hpp"
#include "../common/Logger.hpp"
#include <boost/property_tree/json_parser.hpp>
#include <stdint.h>

ConfigManager::ConfigManager(const ManagerAccessorPtr& manager_accessor) :
manager_accessor_(manager_accessor)
{
}

void ConfigManager::Load(const std::string& filename)
{
    using boost::property_tree::ptree;
    ptree pt;
    read_json(filename, pt);

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
