//
// Connect.hpp
//

#pragma once

#include <memory>
#include "Base.hpp"
#include "../ui/UILabel.hpp"
#include "../ui/UIButton.hpp"
#include "../ManagerAccessor.hpp"

namespace scene {

class Connect : public Base {

    public:
        Connect(const ManagerAccessorPtr&);
        ~Connect();
        void Begin();
        void Update();
		void ProcessInput(InputManager*);
        void Draw();
        void End();
        BasePtr NextScene();

    private:
        void AsyncInitialize();

    private:
        ManagerAccessorPtr manager_accessor_;
        CardManagerPtr card_manager_;
        AccountManagerPtr account_manager_;
        ConfigManagerPtr config_manager_;
		CommandManagerPtr command_manager_;
		
		UILabel message_;
		UILabel button_label_;
        UIButton button_;

		bool return_flag_;

        BasePtr next_scene_;
};

}
