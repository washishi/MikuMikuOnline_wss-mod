//
// Config.hpp
//

#pragma once

#include <time.h>
#include <fstream>
#include <istream>
#include <string>
#include <list>

class Config
{
    public:
		Config();
		void Reload();

    private:
		void Load();

        uint16_t port_;
        std::string server_name_;
        std::string server_note_;
        std::string stage_;
		int capacity_;

		bool public_;

		int receive_limit_1_;
		int receive_limit_2_;
		
		std::list<std::string> blocking_address_patterns_;
		std::list<std::string> lobby_servers_;

		boost::property_tree::ptree pt_;

    public:
        uint16_t port() const;
        const std::string& server_name() const;
        const std::string& server_note() const;

        bool is_public() const;

        const std::string& stage() const;
        int capacity() const;

		int receive_limit_1() const;
		int receive_limit_2() const;

		const std::list<std::string>& blocking_address_patterns() const;
		const std::list<std::string>& lobby_servers() const;

		const boost::property_tree::ptree& pt() const;

	private:
		static const char* CONFIG_JSON;
		time_t timestamp_;
};
