//
// MainLoop.hpp
//

#pragma once

#include <memory>
#include "Base.hpp"
#include "../Client.hpp"
#include "../PlayerManager.hpp"
#include "../CardManager.hpp"
#include "../CommandManager.hpp"
#include "../WorldManager.hpp"
#include "../AccountManager.hpp"
#include "../ConfigManager.hpp"
#include "../ManagerAccessor.hpp"
#include "../ui/InputBox.hpp"
#include "../MiniMap.hpp"

namespace scene {

class MainLoop : public Base {

    public:
        MainLoop(const ManagerAccessorPtr&);
        ~MainLoop();
        void Begin();
        void Update();
        void Draw();
        void End();

    private:
        std::function<void(const tstring&)> push_message_;
		void ProcessInput(InputManager *input);

    private:
        // アクセサ
        ManagerAccessorPtr manager_accessor_;
        PlayerManagerPtr player_manager_;
        CardManagerPtr card_manager_;
        CommandManagerPtr command_manager_;
        WorldManagerPtr world_manager_;
        AccountManagerPtr account_manager_;
        ConfigManagerPtr config_manager_;
		MiniMap minimap_;
        InputBox inputbox_;
		int snapshot_number_;
};

}
