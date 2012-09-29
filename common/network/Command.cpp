//
// Command.cpp
//

#include "Command.hpp"
#include "Utils.hpp"
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

FatalConnectionError::FatalConnectionError() :
        Command(header::FatalConnectionError, "")
{

}

FatalConnectionError::FatalConnectionError(uint32_t user_id) :
        Command(header::FatalConnectionError, "")
{
    body_ = Utils::Serialize(user_id);
}

ServerStartEncryptedSession::ServerStartEncryptedSession() :
        Command(header::ServerStartEncryptedSession, "")
{

}

ClientStartEncryptedSession::ClientStartEncryptedSession() :
        Command(header::ClientStartEncryptedSession, "")
{

}

ServerReceivePublicKey::ServerReceivePublicKey(const std::string& key) :
        Command(header::ServerReceivePublicKey, key)
{

}

ClientReceiveCommonKey::ClientReceiveCommonKey(const std::string& key,
        const std::string& sign,
        uint32_t user_id) :
        Command(header::ClientReceiveCommonKey, Utils::Serialize(key, sign, user_id))
{
}

ClientUpdatePlayerPosition::ClientUpdatePlayerPosition
(uint32_t id, int16_t x, int16_t y, int16_t z, uint8_t theta, uint8_t vy) :
Command(header::ClientUpdatePlayerPosition, Utils::Serialize(id, x, y, z, theta, vy))
{

}

ServerUpdatePlayerPosition::ServerUpdatePlayerPosition(int16_t x, int16_t y, int16_t z, uint8_t theta, uint8_t vy) :
        Command(header::ServerUpdatePlayerPosition, Utils::Serialize(x, y, z, theta, vy))
{
}

ServerReceiveClientInfo::ServerReceiveClientInfo(const std::string& key, uint16_t version, uint16_t udp_port) :
                Command(header::ServerReceiveClientInfo, Utils::Serialize(key, version, udp_port))
{

}

ClientRequestedPublicKey::ClientRequestedPublicKey() :
        Command(header::ClientRequestedPublicKey, "")
{

}

ClientRequestedClientInfo::ClientRequestedClientInfo() :
        Command(header::ClientRequestedClientInfo, "")
{

}

ServerRequestedAccountRevisionPatch::ServerRequestedAccountRevisionPatch(uint32_t user_id, int revision) :
                                           Command(header::ServerRequestedAccountRevisionPatch,
                                                   Utils::Serialize(user_id, revision))
{
}

ClientReceiveAccountRevisionPatch::ClientReceiveAccountRevisionPatch(const std::string& patch) :
    Command(header::ClientReceiveAccountRevisionPatch, patch)
{

}

ClientReceiveAccountRevisionUpdateNotify::
ClientReceiveAccountRevisionUpdateNotify(uint32_t user_id, int revision) :
        Command(header::ClientReceiveAccountRevisionUpdateNotify, Utils::Serialize(user_id, revision))
{
}

ClientReceiveWriteAverageLimitUpdate::ClientReceiveWriteAverageLimitUpdate(uint16_t byte) :
        Command(header::ClientReceiveWriteAverageLimitUpdate, Utils::Serialize(byte))
{
}

ClientReceiveServerCrowdedError::ClientReceiveServerCrowdedError() :
        Command(header::ClientReceiveServerCrowdedError, "")
{

}

ClientReceiveUnsupportVersionError::ClientReceiveUnsupportVersionError(uint32_t require_version) :
        Command(header::ClientReceiveUnsupportVersionError, Utils::Serialize(require_version))
{
}

ServerReceiveAccountInitializeData::ServerReceiveAccountInitializeData(const std::string& data):
        Command(header::ServerReceiveAccountInitializeData, data)
{

}

ServerUpdateAccountProperty::ServerUpdateAccountProperty(AccountProperty property, const std::string& value) :
Command(header::ServerUpdateAccountProperty, Utils::Serialize(property) + value)
{

}

ServerReceiveJSON::ServerReceiveJSON(const std::string& json) :
        Command(header::ServerReceiveJSON, json)
{
}

ClientReceiveJSON::ClientReceiveJSON(const std::string& info_json, const std::string& message_json) :
        Command(header::ClientReceiveJSON, Utils::Serialize(info_json, message_json))
{
}

ClientReceiveServerInfo::ClientReceiveServerInfo(const std::string& stage) :
	Command(header::ClientReceiveServerInfo, Utils::Serialize(stage))
{

}

}
