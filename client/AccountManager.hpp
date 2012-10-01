//
// AccountManager.hpp
//

#pragma once

#include "ManagerHeader.hpp"
#include <string>
#include <stdint.h>
#include <boost/property_tree/xml_parser.hpp>

class AccountManager {
    public:
        AccountManager(const ManagerAccessorPtr& manager_accessor);

        void Load(const std::string& filename);
        void Save(const std::string& filename);
		
		boost::property_tree::ptree Get(const std::string& name) const;
		void Set(const std::string& name, const boost::property_tree::ptree& value);

        std::string GetSerializedData() const;

        std::string public_key() const;
        std::string private_key() const;

        std::string name() const;
        void set_name(const std::string& name);
        std::string model_name() const;
        void set_model_name(const std::string& name);

        std::string trip_passwd() const;
        void set_trip_passwd(const std::string& trip_passwd);

        uint16_t udp_port() const;
        void set_udp_port(uint16_t port);

        std::string host() const;
        void set_host(const std::string& host);

    private:
        ManagerAccessorPtr manager_accessor_;

        std::string public_key_;
        std::string private_key_;

        std::string name_;
        std::string trip_passwd_;
        std::string model_name_;

        std::string host_;
        uint16_t udp_port_;

		boost::property_tree::ptree pt_;
};

typedef std::shared_ptr<AccountManager> AccountManagerPtr;
typedef std::weak_ptr<AccountManager> AccountManagerWeakPtr;
