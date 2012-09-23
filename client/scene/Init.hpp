//
// Init.hpp
//

#pragma once

#include <memory>
#include <array>
#include <boost/thread.hpp>
#include "Base.hpp"
#include "../ManagerHeader.hpp"

class ImageHandle;
typedef std::shared_ptr<ImageHandle> ImageHandlePtr;

namespace scene {

class Init : public Base {

    public:
        Init();
        ~Init();
        void Begin();
        void Update();
        void Draw();
		void ProcessInput(InputManager*);
        void End();
        BasePtr NextScene();

    private:
        void AsyncInitialize();

    private:
        ManagerAccessorPtr manager_accessor_;
        ConfigManagerPtr config_manager_;
        CardManagerPtr card_manager_;
        AccountManagerPtr account_manager_;
		WindowManagerPtr window_manager_;

        int start_count_;

        boost::thread loading_thread_;
        std::shared_ptr<BasePtr> next_scene_;

        std::array<ImageHandlePtr, 5> loading_image_handle_;

        boost::mutex mutex_;
        bool loaded_;
};

}
