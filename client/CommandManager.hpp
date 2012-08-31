//
// CommandManager.hpp
//

#pragma once

#include "ManagerAccessor.hpp"
#include <string>

namespace network {
    class Client;
    class Command;
}

typedef std::unique_ptr<network::Client> ClientUniqPtr;

class CommandManager {
    public:
        CommandManager(const ManagerAccessorPtr& manager_accessor = ManagerAccessorPtr());
        void Update();
        void Write(const network::Command& command);
        void WriteUDP(const network::Command& command);

        void SendJSON(const std::string&);

        void set_client(ClientUniqPtr client);
        unsigned int user_id();

    private:
        ManagerAccessorPtr manager_accessor_;
        ClientUniqPtr client_;
};

typedef std::shared_ptr<CommandManager> CommandManagerPtr;
typedef std::weak_ptr<CommandManager> CommandManagerWeakPtr;
