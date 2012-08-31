//
// UIButton.hpp
//

#pragma once

#include "UIBase.hpp"
#include "../ResourceManager.hpp"
#include <array>
#include <string>
#include <functional>

class UIButton : public UIBase {
        typedef std::function<void()> CallbackFunc;

    public:
        UIButton();
        ~UIButton();
        void ProcessInput(InputManager* input);
        void Update();
        void Draw();

        void set_on_click(const CallbackFunc& func);

    public:
        static void DefineInstanceTemplate(Handle<ObjectTemplate>* object);

    private:
        std::array<ImageHandlePtr,4> base_image_handle_;

        CallbackFunc on_click_;

    private:
        void DrawBase();

    private:
        const static int BASE_BLOCK_SIZE;

};
