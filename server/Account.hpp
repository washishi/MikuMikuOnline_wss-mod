//
// Account.hpp
//

#pragma once

#include <string>
#include <map>
#include <list>
#include <unordered_map>
#include <stdint.h>
#include "../common/database/AccountProperty.hpp"
#include "../common/network/Utils.hpp"
#include "../common/Logger.hpp"
#include <boost/thread.hpp>

typedef unsigned int UserID;

class Account {
    public:
        Account(const std::string&);
        ~Account();

        void LoadInitializeData(UserID user_id, std::string data);

        unsigned int GetCurrentRevision();
        std::string GetUserRevisionPatch(UserID user_id, unsigned int revision);

        UserID GetUserIdFromFingerPrint(const std::string&);
        std::string GetPublicKey(UserID);
        UserID RegisterPublicKey(const std::string&);

        void LogIn(UserID);
        void LogOut(UserID);
        void LogOutAll();

        std::string GetUserName(UserID) const;
        void SetUserName(UserID, const std::string&);
        std::string GetUserTrip(UserID) const;
        void SetUserTrip(UserID, const std::string&);
        std::string GetUserModelName(UserID) const;
        void SetUserModelName(UserID, const std::string&);

        std::string GetUserIPAddress(UserID) const;
        void SetUserIPAddress(UserID, const std::string&);
        uint16_t GetUserUDPPort(UserID) const;
        void SetUserUDPPort(UserID, uint16_t);
        uint32_t GetUserRevision(UserID) const;

        void SetUserPosition(UserID, const PlayerPosition&);
        PlayerPosition GetUserPosition(UserID) const;

        std::vector<UserID> GetIDList() const;

    private:
        template <class T>
        void Set(UserID user_id, AccountProperty property, T value, bool revision = true)
        {
			if (user_id == 0) {
				return;
			}

            T old_value;
            if (!Get(user_id, property, &old_value) || old_value != value) {
				boost::unique_lock<boost::recursive_mutex> lock(mutex_);

                if (user_map_.find(user_id) == user_map_.end()) {
                    user_map_[user_id] = PropertyMap();
                }

                user_map_[user_id][property].value = network::Utils::Serialize(value);

                if (revision) {
                    uint32_t new_revision = GetUserRevision(user_id) + 1;
                    Logger::Debug("Userdata Update %d %d Revision: %d",
                              user_id, property, new_revision);

                    user_map_[user_id][property].revision = new_revision;
                    Set(user_id, REVISION, new_revision, false);
                }
            }
        }

        template <class T>
        bool Get(UserID user_id, AccountProperty property, T* value) const
        {
            UserMap::const_iterator usermap_it;
            if ((usermap_it = user_map_.find(user_id)) != user_map_.end()) {
                PropertyMap::const_iterator property_it;
                if ((property_it = usermap_it->second.find(property)) != usermap_it->second.end()) {
                    network::Utils::Deserialize(property_it->second.value, value);
                    return true;
                }
            }
            return false;
        }

        struct PropertyValue {
            PropertyValue() : revision(0) {}
            uint32_t revision;
            std::string value;
        };

        typedef std::map<AccountProperty, PropertyValue> PropertyMap;
        typedef std::map<UserID, PropertyMap> UserMap;
        UserMap user_map_;

        typedef std::unordered_map<std::string, UserID> FingerprintMap;
        FingerprintMap fingerprint_map_;

        typedef std::map<UserID, PlayerPosition> PositionMap;
        PositionMap position_map_;

        unsigned int revision_;
        UserID max_user_id_;

		boost::recursive_mutex mutex_;
};
