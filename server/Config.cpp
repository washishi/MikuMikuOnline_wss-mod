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
	
    port_ =			pt.get<unsigned short>("port", 39390);
    server_name_ =	pt.get<std::string>("server_name", "MMO Server");
    stage_ =		pt.get<std::string>("stage", "stage:ケロリン町");
    capacity_ =		pt.get<int>("capacity", 20);
}

//
// アクセサ
//

unsigned short Config::port() const
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