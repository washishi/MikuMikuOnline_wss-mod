//
// Account.cpp
//

#include "Account.hpp"
#include "../common/network/Encrypter.hpp"
#include <iostream>
#include <string.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <assert.h>

Account::Account(const std::string& logfile) :
revision_(0),
max_user_id_(0)
{
}

Account::~Account()
{
}

void Account::LoadInitializeData(UserID user_id, std::string data)
{
    std::string buffer(data);

    while (buffer.size()) {
        uint16_t property_int;
        buffer.erase(0, network::Utils::Deserialize(buffer, &property_int));

        AccountProperty property = static_cast<AccountProperty>(property_int);
        switch (property) {

            case NAME:
            {
                std::string value;
                buffer.erase(0, network::Utils::Deserialize(buffer, &value));
                SetUserName(user_id, value);
            }
                break;

            case TRIP:
            {
                std::string value;
                buffer.erase(0, network::Utils::Deserialize(buffer, &value));
                SetUserTrip(user_id, value);
            }
                break;

            case MODEL_NAME:
            {
                std::string value;
                buffer.erase(0, network::Utils::Deserialize(buffer, &value));
                SetUserModelName(user_id, value);
            }
                break;

            default:
                ;
        }
    }
}

uint32_t Account::GetCurrentRevision()
{
    return revision_;
}

std::string Account::GetUserRevisionPatch(UserID user_id, uint32_t revision)
{
    auto user_revison = GetUserRevision(user_id);
    std::string patch;

    if (user_revison > revision) {
        patch += network::Utils::Serialize(user_id, user_revison);

        UserMap::iterator usermap_it;
        if ((usermap_it = user_map_.find(user_id)) != user_map_.end()) {

            PropertyMap& property_map = usermap_it->second;

            for (auto it = property_map.begin(); it != property_map.end(); ++it) {
                if (it->second.revision > revision) {
                    patch += network::Utils::Serialize((uint16_t)it->first) + it->second.value;
                }
            }
        }
    }

    return patch;
}

void Account::Remove(UserID user_id)
{
	boost::unique_lock<boost::recursive_mutex> lock(mutex_);
	user_map_.erase(user_id);
}

/*
void Account::ApplyRevisionPatch(const std::string& patch)
{
    std::string buffer(patch);
    while (buffer.size() > 0) {
        std::tuple<std::string, std::string> result_tuple;
        size_t readed = network::Utils::Deserialize(buffer, &result_tuple);
        buffer.erase(0, readed);

        const std::string& key = std::get<0>(result_tuple);
        const std::string& value = std::get<1>(result_tuple);

        leveldb::Status s = db_value_->Put(leveldb::WriteOptions(), key, value);
        assert(s.ok());
    }

    std::tuple<uint32_t> result_tuple;
    network::Utils::Deserialize(buffer, &result_tuple);

    uint32_t new_revision = std::get<0>(result_tuple);
    assert(new_revision >= revision_);

    revision_ = new_revision;

    leveldb::Status s = db_revision_->Put(leveldb::WriteOptions(), "_MAX_",
            std::string(reinterpret_cast<const char*>(&revision_), sizeof(uint32_t)));
    assert(s.ok());
}
*/

UserID Account::GetUserIdFromFingerPrint(const std::string& finger_print)
{
	return 0;
    //FingerprintMap::iterator it;
    //if ((it = fingerprint_map_.find(finger_print)) != fingerprint_map_.end()) {
    //    return it->second;
    //} else {
    //    return 0;
    //}
}

std::string Account::GetPublicKey(UserID user_id)
{
    std::string value;
    Get(user_id, PUBLIC_KEY, &value);
    return value;
}

UserID Account::RegisterPublicKey(const std::string& public_key)
{
    UserID user_id = 0;
    std::string finger_print = network::Encrypter::GetHash(public_key);

    if (GetUserIdFromFingerPrint(finger_print) == 0) {
        // ユーザーIDを発行
        user_id = ++max_user_id_;
        fingerprint_map_[finger_print] = user_id;

        SetUserName(user_id, "???");
        Set(user_id, PUBLIC_KEY, public_key, false);
        Set(user_id, REVISION, (uint32_t)1, false);
    }

    return user_id;
}

void Account::LogIn(UserID user_id)
{
    Set(user_id, LOGIN, (char)1);
}

void Account::LogOut(UserID user_id)
{
    Set(user_id, LOGIN, (char)0);
}

void Account::LogOutAll()
{
    //for (uint32_t user_id = 1; user_id <= max_user_id_; user_id++) {
    //    Set(user_id, LOGIN, (char)0);
    //}
}

std::string Account::GetUserName(UserID user_id) const
{
    std::string name;
    Get(user_id, NAME, &name);
    return name;
}

void Account::SetUserName(UserID user_id, const std::string& name)
{
    if (name.size() > 0 && name.size() <= 32) {
        Set(user_id, NAME, name);
    }
}

std::string Account::GetUserTrip(UserID user_id) const
{
    std::string trip;
    Get(user_id, TRIP, &trip);
    return trip;
}

void Account::SetUserTrip(UserID user_id, const std::string& trip)
{
    if (trip.size() > 0 && trip.size() <= 64) {
        std::string crypted_trip = network::Encrypter::GetTrip(trip);
        Set(user_id, TRIP, crypted_trip);
    }
}

std::string Account::GetUserModelName(UserID user_id) const
{
    std::string name;
    Get(user_id, MODEL_NAME, &name);
    return name;
}

void Account::SetUserModelName(UserID user_id, const std::string& name)
{
    if (name.size() > 0 && name.size() <= 64) {
        Set(user_id, MODEL_NAME, name);
    }
}

std::string Account::GetUserIPAddress(UserID user_id) const
{
    std::string ip_address;
    Get(user_id, IP_ADDRESS, &ip_address);
    return ip_address;
}
void Account::SetUserIPAddress(UserID user_id, const std::string& ip_address)
{
    Set(user_id, IP_ADDRESS, ip_address);
}

uint16_t Account::GetUserUDPPort(UserID user_id) const
{
    uint16_t udp_port = 0;
    Get(user_id, UDP_PORT, &udp_port);
    return udp_port;
}

void Account::SetUserUDPPort(UserID user_id, uint16_t udp_port)
{
    Set(user_id, UDP_PORT, udp_port);
}

uint32_t Account::GetUserRevision(UserID user_id) const
{
    uint32_t revision = 0;
    Get(user_id, REVISION, &revision);
    return revision;
}

void Account::SetUserPosition(UserID user_id, const PlayerPosition& pos)
{
    auto it = position_map_.find(user_id);
    if (it == position_map_.end()) {
        position_map_[user_id] = PlayerPosition();
    }
    position_map_[user_id] = pos;
}

PlayerPosition Account::GetUserPosition(UserID user_id) const
{
    auto it = position_map_.find(user_id);
    if (it == position_map_.end()) {
        return PlayerPosition();
    }
    return it->second;
}

std::vector<UserID> Account::GetIDList() const
{
    std::vector<UserID> list;
    for (auto it = user_map_.begin(); it != user_map_.end(); ++it) {
		if (it->first != 0) {
			list.push_back(it->first);
		}
    }
    return list;
}
