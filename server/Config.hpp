//
// Config.hpp
//

#pragma once

#include <fstream>
#include <string>

class Config
{
    public:
        Config(const std::string& filename);

    private:
		unsigned short port_;
        std::string server_name_;
        std::string stage_;
		int capacity_;

    public:
        unsigned short port() const;
        const std::string& server_name() const;
        const std::string& stage() const;
        int capacity() const;
};
