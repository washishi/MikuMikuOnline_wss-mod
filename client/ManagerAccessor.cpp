//
// ManagerAccessor.cpp
//

#include "ManagerAccessor.hpp"

ManagerAccessor::ManagerAccessor(const CardManagerWeakPtr& card_manager,
        const PlayerManagerWeakPtr& player_manager) :
card_manager_(card_manager),
player_manager_(player_manager)
{

}

ManagerAccessor::ManagerAccessor()
{

}

const CardManagerWeakPtr& ManagerAccessor::card_manager()
{
    return card_manager_;
}

void ManagerAccessor::set_card_manager(CardManagerWeakPtr card_manager)
{
    card_manager_ = card_manager;
}

const PlayerManagerWeakPtr& ManagerAccessor::player_manager()
{
    return player_manager_;
}

void ManagerAccessor::set_player_manager(PlayerManagerWeakPtr player_manager)
{
    player_manager_ = player_manager;
}

const CommandManagerWeakPtr& ManagerAccessor::command_manager()
{
    return command_manager_;
}

void ManagerAccessor::set_command_manager(CommandManagerWeakPtr command_manager)
{
    command_manager_ = command_manager;
}

const WorldManagerWeakPtr& ManagerAccessor::world_manager()
{
    return world_manager_;
}

void ManagerAccessor::set_world_manager(WorldManagerWeakPtr world_manager)
{
    world_manager_ = world_manager;
}

const AccountManagerWeakPtr& ManagerAccessor::account_manager()
{
    return account_manager_;
}

void ManagerAccessor::set_account_manager(AccountManagerWeakPtr account_manager)
{
    account_manager_ = account_manager;
}

const ConfigManagerWeakPtr& ManagerAccessor::config_manager()
{
    return config_manager_;
}

void ManagerAccessor::set_config_manager(ConfigManagerWeakPtr config_manager)
{
    config_manager_ = config_manager;
}

const WindowManagerWeakPtr& ManagerAccessor::window_manager()
{
    return window_manager_;
}

void ManagerAccessor::set_window_manager(WindowManagerWeakPtr window_manager)
{
    window_manager_ = window_manager;
}

const SocketServerManagerWeakPtr& ManagerAccessor::socket_server_manager()
{
    return socket_server_manager_;
}

void ManagerAccessor::set_socket_server_manager(SocketServerManagerWeakPtr socket_server_manager)
{
    socket_server_manager_ = socket_server_manager;
}