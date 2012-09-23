//
// Config.cpp
//

#include "Config.hpp"
#include <boost/program_options.hpp>
#include <stdint.h>

Config::Config(const std::string& filename)
{
    using boost::property_tree::ptree;
    ptree pt;

    try {
        read_json(filename, pt);
    } catch(std::exception& e) {
        pt = ptree();
    }
	
    port_ =             pt.get<uint16_t>("port", 39390);
    server_name_ =		pt.get<std::string>("server_name", "MMO Server");
    stage_ =			pt.get<std::string>("stage", "stage:ケロリン町");
    capacity_ =			pt.get<int>("capacity", 20);

	public_ =			pt.get<bool>("public", false);

	receive_limit_1_ =	pt.get<int>("receive_limit_1", 60);
	receive_limit_2_ =	pt.get<int>("receive_limit_2", 100);

	auto patterns =		pt.get_child("blocking_address_patterns", ptree());
	BOOST_FOREACH(const auto& item, patterns) {
		blocking_address_patterns_.push_back(item.second.get_value<std::string>());
	}

}

//
// アクセサ
//

uint16_t Config::port() const
{
	return port_;
}

const std::string& Config::server_name() const
{
    return server_name_;
}

const std::string& Config::stage() const
{
    return stage_;
}

int Config::capacity() const
{
	return capacity_;
}

int Config::receive_limit_1() const
{
	return receive_limit_1_;
}

int Config::receive_limit_2() const
{
	return receive_limit_2_;
}

const std::list<std::string>& Config::blocking_address_patterns() const
{
	return blocking_address_patterns_;
}