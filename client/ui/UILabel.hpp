//
// UILabel.hpp
//

#pragma once

#include "UIBase.hpp"
#include <array>
#include <string>
#include "../../common/unicode.hpp"

class UILabel : public UIBase {
    public:
        UILabel();
        ~UILabel();
        void ProcessInput(InputManager* input);
        void Update();
        void Draw();

        void set_text(const tstring& text);
        tstring text() const;

        void set_textcolor(const Color& color);
        Color textcolor() const;
        void set_bgcolor(const Color& color);
        Color bgcolor() const;

    public:
        static void DefineInstanceTemplate(Handle<ObjectTemplate>* object);
        
    private:
        /* property */
        static Handle<Value> Property_text(Local<String> property, const AccessorInfo &info);
        static void Property_set_text(Local<String> property, Local<Value> value, const AccessorInfo& info);
        static Handle<Value> Property_bgcolor(Local<String> property, const AccessorInfo &info);
        static void Property_set_bgcolor(Local<String> property, Local<Value> value, const AccessorInfo& info);
        static Handle<Value> Property_color(Local<String> property, const AccessorInfo &info);
        static void Property_set_color(Local<String> property, Local<Value> value, const AccessorInfo& info);

    private:
        void UpdatePosition();

    private:
        tstring text_;
        std::vector<int> substr_list_;
        std::vector<int> char_width_list_;
        int font_handle_;

		int clickchancel_count_;

        Color textcolor_, bgcolor_;

    private:
        const static int BASE_BLOCK_SIZE;

};
