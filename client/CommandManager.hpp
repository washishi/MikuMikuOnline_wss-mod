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
		enum Status {
			STATUS_STANDBY,
			STATUS_CONNECTING,
			STATUS_READY,
			STATUS_ERROR,
			STATUS_ERROR_CROWDED,
			STATUS_ERROR_NOSTAGE,
			STATUS_ERROR_VERSION
		};

    public:
        CommandManager(const ManagerAccessorPtr& manager_accessor = ManagerAccessorPtr());
        void Update();
        void Write(const network::Command& command);
        void WriteUDP(const network::Command& command);

        void SendJSON(const std::string&);

        void set_client(ClientUniqPtr client);
        unsigned int user_id();
		std::string stage() const;

		Status status() const;		

    private:
        ManagerAccessorPtr manager_accessor_;
        ClientUniqPtr client_;
		Status status_;
		std::string stage_;
};

typedef std::shared_ptr<CommandManager> CommandManagerPtr;
typedef std::weak_ptr<CommandManager> CommandManagerWeakPtr;
