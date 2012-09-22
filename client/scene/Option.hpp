//
// Option.hpp
//

#pragma once

#include "Base.hpp"
#include "../ManagerHeader.hpp"
#include "../ManagerAccessor.hpp"

namespace scene {
class Option : public Base {

    public:
        Option(const ManagerAccessorPtr& manager_accessor,
			const BasePtr& background_scene);
        ~Option();
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
        ConfigManagerPtr config_manager_;
        CardManagerPtr card_manager_;
        AccountManagerPtr account_manager_;

		BasePtr background_scene_;

        int start_count_;
};
}