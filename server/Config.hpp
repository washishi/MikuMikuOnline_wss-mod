//
// Config.hpp
//

#pragma once

#include <fstream>
#include <string>
#include <list>

class Config
{
    public:
        Config(const std::string& filename);

    private:
        uint16_t port_;
        std::string server_name_;
        std::string stage_;
		int capacity_;

		int receive_limit_1_;
		int receive_limit_2_;
		
		std::list<std::string> blocking_address_patterns_;

    public:
        uint16_t port() const;
        const std::string& server_name() const;
        const std::string& stage() const;
        int capacity() const;

		int receive_limit_1() const;
		int receive_limit_2() const;

		const std::list<std::string>& blocking_address_patterns() const;
};
