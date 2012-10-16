//
// ManagerHeader.hpp
//

#pragma once
#include <memory>

class ManagerAccessor;
typedef std::shared_ptr<ManagerAccessor> ManagerAccessorPtr;

class CardManager;
typedef std::shared_ptr<CardManager> CardManagerPtr;
typedef std::weak_ptr<CardManager> CardManagerWeakPtr;

class PlayerManager;
typedef std::shared_ptr<PlayerManager> PlayerManagerPtr;
typedef std::weak_ptr<PlayerManager> PlayerManagerWeakPtr;

class CommandManager;
typedef std::shared_ptr<CommandManager> CommandManagerPtr;
typedef std::weak_ptr<CommandManager> CommandManagerWeakPtr;

class WorldManager;
typedef std::shared_ptr<WorldManager> WorldManagerPtr;
typedef std::weak_ptr<WorldManager> WorldManagerWeakPtr;

class AccountManager;
typedef std::shared_ptr<AccountManager> AccountManagerPtr;
typedef std::weak_ptr<AccountManager> AccountManagerWeakPtr;

class ConfigManager;
typedef std::shared_ptr<ConfigManager> ConfigManagerPtr;
typedef std::weak_ptr<ConfigManager> ConfigManagerWeakPtr;

class WindowManager;
typedef std::shared_ptr<WindowManager> WindowManagerPtr;
typedef std::weak_ptr<WindowManager> WindowManagerWeakPtr;

class SocketServerManager;
typedef std::shared_ptr<SocketServerManager> SocketServerManagerPtr;
typedef std::weak_ptr<SocketServerManager> SocketServerManagerWeakPtr;