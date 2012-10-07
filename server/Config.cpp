//
// Config.cpp
//

#include "Config.hpp"
#include <boost/filesystem.hpp>
#include <stdint.h>

using namespace boost::filesystem;

const char* Config::CONFIG_JSON = "./config.json";

namespace {
    using boost::property_tree::ptree;

	void MergePtree(ptree* dst,
		const ptree& source,
		const ptree::path_type& current_path = ptree::path_type(""))
	{
		BOOST_FOREACH(const auto& tree, source) {
			if (!tree.first.empty()) {
				auto new_path = current_path;
				new_path /= tree.first;
				if (tree.second.empty() || tree.second.front().first.empty()) {
					if (!dst->get_child_optional(new_path)) {
						dst->put_child(new_path, tree.second);
					}
				} else {
					MergePtree(dst, tree.second, new_path);
				}
			}
		}
	}
};

Config::Config()
{
	Load();
}

void Config::Load()
{

	try {
		std::ifstream ifs;
		ifs.open(CONFIG_JSON);
		read_json(ifs, pt_);
	} catch(std::exception& e) {
		Logger::Error(unicode::ToTString(e.what()));
	}
	
    port_ =             pt_.get<uint16_t>("port", 39390);
    server_name_ =		pt_.get<std::string>("server_name", "MMO Server");
    server_note_ =		pt_.get<std::string>("server_note", "");
	stage_ =			pt_.get<std::string>("stage", unicode::ToString(_T("stage:ケロリン町")));
    capacity_ =			pt_.get<int>("capacity", 20);

	public_ =			pt_.get<bool>("public", false);

	receive_limit_1_ =	pt_.get<int>("receive_limit_1", 60);
	receive_limit_2_ =	pt_.get<int>("receive_limit_2", 100);

	auto patterns =		pt_.get_child("blocking_address_patterns", ptree());
	BOOST_FOREACH(const auto& item, patterns) {
		blocking_address_patterns_.push_back(item.second.get_value<std::string>());
	}

	auto lobby_servers = pt_.get_child("lobby_servers", ptree());
	BOOST_FOREACH(const auto& item, lobby_servers) {
		lobby_servers_.push_back(item.second.get_value<std::string>());
	}
	
	if (exists(CONFIG_JSON)) {
		timestamp_ = last_write_time(CONFIG_JSON);
	}
}

void Config::Reload()
{
	if (exists(CONFIG_JSON) &&
		timestamp_ < last_write_time(CONFIG_JSON)) {
		Config::Load();
		Logger::Info(_T("Configuration reloaded."));
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

const std::string& Config::server_note() const
{
    return server_note_;
}

bool Config::is_public() const
{
	return public_;
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

const std::list<std::string>& Config::lobby_servers() const
{
	return lobby_servers_;
}

const boost::property_tree::ptree& Config::pt() const
{
	return pt_;
}