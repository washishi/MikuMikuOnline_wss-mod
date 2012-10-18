//
// Command.cpp
//

#include "Command.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace network {

header::CommandHeader Command::header() const
{
    return header_;
}

const std::string& Command::body() const
{
    return body_;
}

SessionWeakPtr Command::session()
{
    return session_;
}

boost::asio::ip::udp::endpoint Command::udp_endpoint() const
{
	return udp_endpoint_;
}

bool Command::plain() const
{
	return plain_;
}

}
