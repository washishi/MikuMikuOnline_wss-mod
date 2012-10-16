//
// ManagerAccessor.hpp
//

#pragma once

#include <memory>
#include "ManagerHeader.hpp"

class ManagerAccessor {
    public:
        ManagerAccessor(const CardManagerWeakPtr& card_manager,
                const PlayerManagerWeakPtr& player_manager);
        ManagerAccessor();

        const CardManagerWeakPtr& card_manager();
        void set_card_manager(CardManagerWeakPtr card_manager);
        const PlayerManagerWeakPtr& player_manager();
        void set_player_manager(PlayerManagerWeakPtr player_manager);
        const CommandManagerWeakPtr& command_manager();
        void set_command_manager(CommandManagerWeakPtr command_manager);
        const WorldManagerWeakPtr& world_manager();
        void set_world_manager(WorldManagerWeakPtr world_manager);
        const AccountManagerWeakPtr& account_manager();
        void set_account_manager(AccountManagerWeakPtr account_manager);
        const ConfigManagerWeakPtr& config_manager();
        void set_config_manager(ConfigManagerWeakPtr config_manager);
        const WindowManagerWeakPtr& window_manager();
        void set_window_manager(WindowManagerWeakPtr window_manager);
	    const SocketServerManagerWeakPtr& socket_server_manager();
        void set_socket_server_manager(SocketServerManagerWeakPtr socket_server_manager);

    private:
        CardManagerWeakPtr card_manager_;
        PlayerManagerWeakPtr player_manager_;
        CommandManagerWeakPtr command_manager_;
        WorldManagerWeakPtr world_manager_;
        AccountManagerWeakPtr account_manager_;
        ConfigManagerWeakPtr config_manager_;
        WindowManagerWeakPtr window_manager_;
		SocketServerManagerWeakPtr socket_server_manager_;
};

typedef std::shared_ptr<ManagerAccessor> ManagerAccessorPtr;
