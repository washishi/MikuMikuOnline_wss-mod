//
// Title.hpp
//

#pragma once

#include <memory>
#include "Base.hpp"
#include "../ManagerAccessor.hpp"
#include "../ui/Input.hpp"
#include "../ui/UILabel.hpp"
#include "../ui/UIButton.hpp"

namespace scene {

class Title : public Base {

    public:
        Title(const ManagerAccessorPtr&);
        ~Title();
        void Begin();
        void Update();
		void ProcessInput(InputManager*);
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

        Input input_host_;
        UILabel label_;
        UIButton button_;

        bool connect_flag_;
        int screen_count_;

        BasePtr next_scene_;
};

}
