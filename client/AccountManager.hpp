//
// AccountManager.hpp
//

#pragma once

#include "ManagerHeader.hpp"
#include <string>
#include <stdint.h>

class AccountManager {
    public:
        AccountManager(const ManagerAccessorPtr& manager_accessor);

        void Load(const std::string& filename);
        void Save(const std::string& filename) const;

        std::string GetSerializedData() const;

        std::string public_key() const;
        std::string private_key() const;

        std::string name() const;
        void set_name(const std::string& name);
        std::string model_name() const;
        void set_model_name(const std::string& name);

        uint16_t udp_port() const;
        void set_udp_port(uint16_t port);

        std::string host() const;
        void set_host(const std::string& host);

    private:
        ManagerAccessorPtr manager_accessor_;

        std::string public_key_;
        std::string private_key_;

        std::string name_;
        std::string trip_;
        std::string model_name_;

        std::string host_;

        uint16_t udp_port_;
};

typedef std::shared_ptr<AccountManager> AccountManagerPtr;
typedef std::weak_ptr<AccountManager> AccountManagerWeakPtr;
