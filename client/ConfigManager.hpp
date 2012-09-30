//
// ConfigManager.hpp
//

#pragma once

#include <fstream>
#include <string>
#include <iostream>
#include <boost/property_tree/json_parser.hpp>
#include "../common/unicode.hpp"
#include "ManagerAccessor.hpp"

class ConfigManager
{
    public:
        ConfigManager(const ManagerAccessorPtr& manager_accessor = ManagerAccessorPtr());
		~ConfigManager();

	private:
        void LoadConfigure();
        void Load(const std::string&);
		void Save(const std::string&);

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

		float model_edge_size_;

		std::string stage_;

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

		float model_edge_size() const;

		std::string stage() const;

	private:
		int show_nametag_,
			show_modelname_,
			gamepad_type_,
			camera_direction_;

	public:
		int show_nametag() const;
		void set_show_nametag(int value);
		int show_modelname() const;
		void set_show_modelname(int value);

		int gamepad_type() const;
		void set_gamepad_type(int value);
	
		int camera_direction() const;
		void set_camera_direction(int value);

    private:
        ManagerAccessorPtr manager_accessor_;

		static const int MIN_SCREEN_WIDTH;
		static const int MIN_SCREEN_HEIGHT;
		static const char* CONFIG_JSON_PATH;
};

typedef std::shared_ptr<ConfigManager> ConfigManagerPtr;
typedef std::weak_ptr<ConfigManager> ConfigManagerWeakPtr;
