//
// MainLoop.hpp
//

#pragma once

#include <memory>
#include "Base.hpp"
#include "../Client.hpp"
#include "../ManagerAccessor.hpp"
#include "../ui/InputBox.hpp"
#include "../MiniMap.hpp"
//#include "../SocketServer.hpp"

namespace scene {

class MainLoop : public Base {

    public:
        MainLoop(const ManagerAccessorPtr&);
        ~MainLoop();
        void Begin();
        void Update();
        void Draw();
		void ProcessInput(InputManager*);
        void End();

    private:
        std::function<void(const tstring&)> push_message_;

    private:
        // アクセサ
        ManagerAccessorPtr manager_accessor_;
        PlayerManagerPtr player_manager_;
        CardManagerPtr card_manager_;
        CommandManagerPtr command_manager_;
        WorldManagerPtr world_manager_;
        AccountManagerPtr account_manager_;
        ConfigManagerPtr config_manager_;
        WindowManagerPtr window_manager_;
        SocketServerManagerPtr socket_server_manager_;
		MiniMapPtr minimap_;
        InputBoxPtr inputbox_;

		int snapshot_number_;
		bool snapshot_;

		int fade_counter_;
		//SocketServer socket_server_;
};

}
