//
// UIBoard.hpp
//

#pragma once

#include "UIBase.hpp"
#include "../ResourceManager.hpp"
#include <array>

class UIBoard : public UIBase {
    public:
        UIBoard();
        void ProcessInput(InputManager* input);
        void Update();
        void Draw();

        bool resizable() const;
        void set_resizable(bool resizable);

    public:
        static void DefineInstanceTemplate(Handle<ObjectTemplate>* object);

    private:
        void UpdateDrag(InputManager* input, bool resizeable = true);

    private:
        static Handle<Value> Property_max_width(Local<String> property, const AccessorInfo &info);
        static void Property_set_max_width(Local<String> property, Local<Value> value, const AccessorInfo& info);
        static Handle<Value> Property_min_width(Local<String> property, const AccessorInfo &info);
        static void Property_set_min_width(Local<String> property, Local<Value> value, const AccessorInfo& info);
        static Handle<Value> Property_max_height(Local<String> property, const AccessorInfo &info);
        static void Property_set_max_height(Local<String> property, Local<Value> value, const AccessorInfo& info);
        static Handle<Value> Property_min_height(Local<String> property, const AccessorInfo &info);
        static void Property_set_min_height(Local<String> property, Local<Value> value, const AccessorInfo& info);

    private:
        std::array<ImageHandlePtr,4> base_image_handle_;

        bool resizable_;

        int max_width_, min_width_;
        int max_height_, min_height_;

        Rect drag_offset_rect_, drag_resize_offset_rect_;

    private:
        const static int BASE_BLOCK_SIZE;
};
