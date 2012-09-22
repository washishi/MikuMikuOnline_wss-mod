//
// ServerChange.h
//

#pragma once

#include <memory>
#include "Base.hpp"
#include "../ManagerAccessor.hpp"

namespace scene {

class ServerChange : public Base{

	public:
		ServerChange(const ManagerAccessorPtr&);
		~ServerChange();
        void Begin();
        void Update();
		void ProcessInput(InputManager*);
        void Draw();
        void End();
        BasePtr NextScene();
    private:
        ManagerAccessorPtr manager_accessor_;
        CardManagerPtr card_manager_;
        AccountManagerPtr account_manager_;
        ConfigManagerPtr config_manager_;
		WorldManagerPtr world_manager_;

		BasePtr next_scene_;
};

}