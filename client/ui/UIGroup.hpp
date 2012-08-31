//
// UIGroup.hpp
//

#pragma once

#include "UIBase.hpp"
#include <array>
#include <string>

class UIGroup : public UIBase {
    public:
        UIGroup();
        ~UIGroup();
        void ProcessInput(InputManager* input);
        void Update();
        void Draw();

    private:
        int auto_height_;

    public:
        static void DefineInstanceTemplate(Handle<ObjectTemplate>* object);

};
