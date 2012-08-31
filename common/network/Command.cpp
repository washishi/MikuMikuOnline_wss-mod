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

// コネクションの確立に成功
ConnectionSucceeded::ConnectionSucceeded() :
        Command(header::ConnectionSucceeded, "")
{

}

ConnectionFailed::ConnectionFailed() :
        Command(header::ConnectionFailed, "")
{

}

FatalConnectionError::FatalConnectionError() :
        Command(header::FatalConnectionError, "")
{

}

FatalConnectionError::FatalConnectionError(unsigned int user_id) :
        Command(header::FatalConnectionError, "")
{
    body_ = Utils::Serialize(user_id);
}

ClientReceiveChatMessage::ClientReceiveChatMessage(unsigned int user_id,
        const std::string& time_string,
        const std::string& msg) :
        Command(header::ClientReceiveChatMessage, Utils::Serialize<unsigned int, std::string, std::string>(user_id, time_string, msg))
{
}

ServerReceiveChatMessage::ServerReceiveChatMessage(const std::string& msg) :
        Command(header::ServerReceiveChatMessage, msg)
{

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
        unsigned int user_id) :
        Command(header::ClientReceiveCommonKey, Utils::Serialize(key, sign, user_id))
{
}

ServerRequestedChatLog::ServerRequestedChatLog() :
        Command(header::ServerRequestedChatLog, "")
{

}

ClientJoinPlayer::ClientJoinPlayer() :
        Command(header::ClientJoinPlayer, "")
{

}

ClientLeavePlayer::ClientLeavePlayer() :
        Command(header::ClientLeavePlayer, "")
{

}

ClientUpdatePlayerPosition::ClientUpdatePlayerPosition
(unsigned int id, float x, float y, float z, float theta) :
Command(header::ClientUpdatePlayerPosition, Utils::Serialize(id, x, y, z, theta))
{

}

ServerUpdatePlayerPosition::ServerUpdatePlayerPosition(float x, float y, float z, float theta) :
        Command(header::ServerUpdatePlayerPosition, Utils::Serialize(x, y, z, theta))
{
}

ServerReceiveClientInfo::ServerReceiveClientInfo(const std::string& key, uint32_t version, uint16_t udp_port) :
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

ClientUpdateChannelUserList::ClientUpdateChannelUserList() :
        Command(header::ClientUpdateChannelUserList, "")
{

}

ServerCloseSession::ServerCloseSession() :
        Command(header::ServerCloseSession, "")
{

}

ServerReceiveNewCard::ServerReceiveNewCard(const std::string& name,
                                           const std::string& note,
                                           const std::string& url) :
                                           Command(header::ServerReceiveNewCard, Utils::Serialize(name, note, url))
{
}

ServerRequestedCardRevisionPatch::ServerRequestedCardRevisionPatch(int revision) :
                                           Command(header::ServerRequestedCardRevisionPatch, Utils::Serialize(revision))
{
}

ServerRequestedAccountRevisionPatch::ServerRequestedAccountRevisionPatch(unsigned int user_id, int revision) :
                                           Command(header::ServerRequestedAccountRevisionPatch,
                                                   Utils::Serialize(user_id, revision))
{
}


ClientReceiveCardRevisionPatch::ClientReceiveCardRevisionPatch(const std::string& patch) :
    Command(header::ClientReceiveCardRevisionPatch, patch)
{

}

ClientReceiveCardRevisionUpdateNotify::ClientReceiveCardRevisionUpdateNotify(int revision) :
        Command(header::ClientReceiveCardRevisionUpdateNotify, Utils::Serialize(revision))
{
}

ClientReceiveAccountRevisionPatch::ClientReceiveAccountRevisionPatch(const std::string& patch) :
    Command(header::ClientReceiveAccountRevisionPatch, patch)
{

}

ClientReceiveAccountRevisionUpdateNotify::
ClientReceiveAccountRevisionUpdateNotify(unsigned int user_id, int revision) :
        Command(header::ClientReceiveAccountRevisionUpdateNotify, Utils::Serialize(user_id, revision))
{
}

ClientReceiveWriteAverageLimitUpdate::ClientReceiveWriteAverageLimitUpdate(unsigned short byte) :
        Command(header::ClientReceiveWriteAverageLimitUpdate, Utils::Serialize(byte))
{
}

ClientReceiveServerCrowdedError::ClientReceiveServerCrowdedError() :
        Command(header::ClientReceiveServerCrowdedError, "")
{

}

PlayerLogoutNotify::PlayerLogoutNotify(unsigned int user_id) :
        Command(header::PlayerLogoutNotify, Utils::Serialize(user_id))
{
}

ServerUpdatePlayerName::ServerUpdatePlayerName(const std::string& name) :
        Command(header::ServerUpdatePlayerName, name)
{
}

ServerUpdatePlayerTrip::ServerUpdatePlayerTrip(const std::string& trip) :
        Command(header::ServerUpdatePlayerTrip, trip)
{
}

ServerUpdatePlayerModelName::ServerUpdatePlayerModelName(const std::string& name) :
        Command(header::ServerUpdatePlayerModelName, name)
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

ServerReceiveJSON::ServerReceiveJSON(const std::string& json) :
        Command(header::ServerReceiveJSON, json)
{
}

ClientReceiveJSON::ClientReceiveJSON(const std::string& info_json, const std::string& message_json) :
        Command(header::ClientReceiveJSON, Utils::Serialize(info_json, message_json))
{
}


}
