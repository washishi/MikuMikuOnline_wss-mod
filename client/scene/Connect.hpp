//
// Connect.hpp
//

#pragma once

#include <memory>
#include "Base.hpp"
#include "../ManagerAccessor.hpp"

namespace scene {

class Connect : public Base {

    public:
        Connect(const ManagerAccessorPtr&);
        ~Connect();
        void Begin();
        void Update();
        void Draw();
        void End();
        BasePtr NextScene();

    private:
        void AsyncInitialize();

    private:
        ManagerAccessorPtr manager_accesor_;
        CardManagerPtr card_manager_;
        AccountManagerPtr account_manager_;
        ConfigManagerPtr config_manager_;
		CommandManagerPtr command_manager_;

        BasePtr next_scene_;
};

}
