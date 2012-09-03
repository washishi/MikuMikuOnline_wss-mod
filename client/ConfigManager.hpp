//
// ConfigManager.hpp
//

#pragma once

#include <fstream>
#include <string>
#include <iostream>
#include "../common/unicode.hpp"
#include "ManagerAccessor.hpp"

class ConfigManager
{
    public:
        ConfigManager(const ManagerAccessorPtr& manager_accessor = ManagerAccessorPtr());
        void Load(const std::string&);

    private:
        bool fullscreen_;
        int screen_width_;
        int screen_height_;
        bool antialias_;
        std::string host_;
        int port_;
        int max_script_execution_time_;
        int max_local_storage_size_;
        bool upnp_;
        int udp_port_;

		bool shader_blur_;
		bool shader_bloom_;
		bool shader_shadow_;
		bool shader_depth_field_;

    public:
        bool fullscreen() const;
        int screen_width() const;
        int screen_height() const;
        bool antialias() const;
        std::string host() const;
        int port() const;
        int max_script_execution_time() const;
        int max_local_storage_size() const;
        bool upnp() const;
        int udp_port() const;

		bool shader_blur() const;
		bool shader_bloom() const;
		bool shader_shadow() const;
		bool shader_depth_field() const;

    private:
        ManagerAccessorPtr manager_accessor_;
};

typedef std::shared_ptr<ConfigManager> ConfigManagerPtr;
typedef std::weak_ptr<ConfigManager> ConfigManagerWeakPtr;
