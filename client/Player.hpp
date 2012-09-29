//
// Player.hpp
//

#pragma once

#include <string>
#include <array>
#include <list>
#include <memory>
#include <v8.h>
#include "ui/UILabel.hpp"
#include "ResourceManager.hpp"
#include "../common/database/AccountProperty.hpp"
using namespace v8;

namespace scene {
    class Main;
};

class ScriptEnvironment;

class PlayerManager;
typedef std::weak_ptr<PlayerManager> PlayerManagerWeakPtr;

class Player;
typedef std::shared_ptr<Player> PlayerPtr;

class Player : public std::enable_shared_from_this<Player> {
    public:
        Player(unsigned int id, const PlayerManagerWeakPtr& player_manager = PlayerManagerWeakPtr());
        ~Player();

        void Update();
        void Draw();

    public:
        static PlayerPtr GetPointerFromObj(Handle<Object>);
        Handle<Object> GetJSObject();

    private:
        static void RegisterFunctionTemplate(Handle<FunctionTemplate>& func);
        static Handle<FunctionTemplate> GetFunctionTemplate();
        void UpdatePosition(Object* parent);

    private:
        static Handle<Value> Construct(const Arguments& args);
        static void Dispose(Persistent<Value> handle, void* parameter);

        /* function */
        static Handle<Value> Function_Player_id(const Arguments& args);
        static Handle<Value> Function_Player_name(const Arguments& args);
        static Handle<Value> Function_Player_trip(const Arguments& args);
        static Handle<Value> Function_Player_login(const Arguments& args);
        static Handle<Value> Function_Player_channel(const Arguments& args);
        static Handle<Value> Function_Player_setBalloonContent(const Arguments& args);
        static Handle<Value> Function_Player_position(const Arguments& args);

    public:

        unsigned int id() const;
        void set_id(unsigned int id);
        std::string name() const;
        void set_name(const std::string& name);
        std::string trip() const;
        void set_trip(const std::string& trip);
        std::string model_name() const;
        void set_model_name(const std::string& model_name);

        bool login() const;
        void set_login(bool login);
		unsigned char channel() const;
		void set_channel(unsigned char channel);

        uint32_t revision() const;
        void set_revision(uint32_t revision);
        const PlayerPosition& position() const;
        void set_position(const PlayerPosition& pos);
		
        std::string current_model_name() const;
        void set_current_model_name(const std::string& model_name);

        std::string ip_address() const;
        void set_ip_address(const std::string& ip_address);
        uint16_t udp_port() const;
        void set_udp_port(uint16_t udp_port);

    private:
        static scene::Main* scene_;
        PlayerManagerWeakPtr player_manager_;

        int font_handle_;
        std::array<ImageHandlePtr, 4> name_tip_image_handle_;
        std::array<ImageHandlePtr, 9> balloon_base_image_handle_;

        unsigned int id_;
        std::string name_;
        std::string trip_;
        std::string model_name_, current_model_name_;
        bool login_;
		unsigned char channel_;
        unsigned int revision_;
        PlayerPosition pos_;

        UIBasePtr ballon_content_;

        Point balloon_root_point_, balloon_root_point_current_;
        Rect name_tip_rect_, name_tip_rect_current_;

        std::string ip_address_;
        uint16_t udp_port_;


    private:
        const static int BALLOON_BASE_BLOCK_SIZE;
        const static int BALLOON_OFFSET_X;
        const static int BALLOON_MIN_WIDTH;

        const static int NAME_TIP_BASE_BLOCK_SIZE;
        const static int NAME_TIP_MARGIN_SIZE;
};

