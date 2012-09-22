//
// CardManager.hpp
//

#pragma once

#include <string>
#include "Card.hpp"
#include <vector>
#include <deque>
#include <memory>
#include <v8.h>
#include "ManagerAccessor.hpp"
#include "InputManager.hpp"
#include "ScriptEnvironment.hpp"

namespace scene {
    class Main;
};

class Player;
typedef std::shared_ptr<Player> PlayerPtr;

class CardManager : public std::enable_shared_from_this<CardManager> {
    public:
        CardManager(const ManagerAccessorPtr& manager_accessor = ManagerAccessorPtr());
        void Load();

        void ProcessInput(InputManager* input);
        void Update();
        void Draw();

        void AddCard(const CardPtr& card);

        void SendJSON(const std::string& json);
        void OnReceiveJSON(const std::string& info_json, const std::string& msg_json);
        void OnLogin(const PlayerPtr& player);
        void OnLogout(const PlayerPtr& player);
		void OnModelReload();

        bool IsGUIActive();
        void FocusPlayer();

        v8::Handle<v8::Object> GetGlobal();

        std::vector<CardPtr>& cards();

    private:
        ManagerAccessorPtr manager_accessor_;

        std::vector<CardPtr> cards_;
        ScriptEnvironment global_;

        bool focus_player_flag;
        int icon_base_handle_;

    private:
        void ParseScriptFile(const std::string&);
        static void Error(const v8::Handle<v8::Value>& error);

    private:
        static char CARDS_DIR[];
        static char START_METADATA[];
        static char END_METADATA[];
};

typedef std::shared_ptr<CardManager> CardManagerPtr;
typedef std::weak_ptr<CardManager> CardManagerWeakPtr;
