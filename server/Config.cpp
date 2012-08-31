//
// Config.cpp
//

#include "Config.hpp"
#include <boost/property_tree/json_parser.hpp>
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

    server_name_ =              pt.get<std::string>("server_name", "MMO Server");
    port_ =                     pt.get<uint16_t>("port", 39390);
    room_capacity_ =            pt.get<int>("room_capacity", 10);
    channel_capacity_ =         pt.get<int>("channel_capacity", 30);
    max_total_read_average_ =   pt.get<int>("max_total_read_average", 5000);
    max_session_read_average_ = pt.get<int>("max_session_read_average", 500);
    min_session_read_average_ = pt.get<int>("min_session_read_average", 100);
}

//
// アクセサ
//

const std::string& Config::server_name() const
{
    return server_name_;
}

int Config::port() const
{
    return port_;
}

int Config::room_capacity() const
{
    return room_capacity_;
}

int Config::channel_capacity() const
{
    return channel_capacity_;
}

int Config::max_total_read_average() const
{
    return max_total_read_average_;
}

int Config::max_session_read_average() const
{
    return max_session_read_average_;
}

int Config::min_session_read_average() const
{
    return min_session_read_average_;
}

std::string Config::download_path() const
{
    return download_path_;
}

std::string Config::scan_command() const
{
    return scan_command_;
}
