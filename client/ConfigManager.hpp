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
		std::list<std::string> lobby_servers_; // ※ロビーサーバをjsonで変更するために追加
        bool antialias_;
        std::string host_;
        int port_;
        int max_script_execution_time_;
        int max_local_storage_size_;
        bool upnp_;
        int udp_port_;

		std::string language_;

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
		const std::list<std::string>& lobby_servers() const; // ※ロビーサーバをjsonで変更するために追加
        bool antialias() const;
        std::string host() const;
        int port() const;

		const std::string& language() const;

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
			modelload_mode_, // ※ モデルのロード方法を変更できるように追加
			gamepad_type_,
			gamepad_enable_, // ※ ゲームパッド有効をウインドウアクティブ時のみにもできる様に追加
			// ※ ここから  ゲームパッドのボタンを変更できるように追加
			gamepad_jump_,
			gamepad_speed_,
			gamepad_warp_,
			gamepad_sshot_,
			// ※ ここまで
			camera_direction_,
			walk_change_type_;

	public:
		int show_nametag() const;
		void set_show_nametag(int value);
		int show_modelname() const;
		void set_show_modelname(int value);
		// ※ ここから モデルのロード方法を変更できるように追加
		int modelload_mode() const;
		void set_modelload_mode(int value);
		// ※ ここまで
		int walk_change_type() const;
		void set_walk_change_type(int value);

		int gamepad_type() const;
		void set_gamepad_type(int value);

		// ※ ここから ゲームパッド有効をウインドウアクティブ時のみにもできる様に追加
		int gamepad_enable() const;
		void set_gamepad_enable(int value);
		// ※ ここまで

		// ※ ここから ゲームパッドのボタンを変更できるように追加
		int gamepad_jump() const;
		void set_gamepad_jump(int value);
		int gamepad_speed() const;
		void set_gamepad_speed(int value);
		int gamepad_warp() const;
		void set_gamepad_warp(int value);
		int gamepad_sshot() const;
		void set_gamepad_sshot(int value);
		// ※ ここまで

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
