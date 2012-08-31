//
// Config.hpp
//

#pragma once

#include <fstream>
#include <string>

class Config
{
    public:
        Config(const std::string&);

    private:
        std::string server_name_;
        int port_;
        int room_capacity_;
        int channel_capacity_;
        int max_total_read_average_;
        int max_session_read_average_;
        int min_session_read_average_;
        std::string download_path_;
        std::string scan_command_;

    public:
        const std::string& server_name() const;
        int port() const;
        int room_capacity() const;
        int channel_capacity() const;
        int max_total_read_average() const;
        int max_session_read_average() const;
        int min_session_read_average() const;
        std::string download_path() const;
        std::string scan_command() const;
};
