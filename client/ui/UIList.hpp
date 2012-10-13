//
// UIList.hpp
//

#pragma once

#include "UIBase.hpp"
#include "../ResourceManager.hpp"
#include <array>
#include <string>

class UIList : public UIBase {
    public:
        UIList();
        ~UIList();
        void ProcessInput(InputManager* input);
        void Update();
        void Draw();

    public:
        static void DefineInstanceTemplate(Handle<ObjectTemplate>* object);

    private:
        /* function */
        static Handle<Value> Function_addItem(const Arguments& args);
        static Handle<Value> Function_removeItem(const Arguments& args);
        static Handle<Value> Function_clearItems(const Arguments& args);

        /* property */
        static Handle<Value> Property_scroll_y(Local<String> property, const AccessorInfo &info);
        static void Property_set_scroll_y(Local<String> property, Local<Value> value, const AccessorInfo& info);

	public:
        /* function */
		void addItem(UIBasePtr item);
		void removeItem(UIBasePtr item);
		void clearItems();
		std::vector<UIBasePtr> getItems() const;

        /* property */
		void set_scroll_y(int scroll_y);

    private:
        std::array<ImageHandlePtr,4> scrollbar_base_image_handle_;

        std::vector<UIBasePtr> items_;

        int scroll_y_, max_scroll_y_, scroll_y_target_;
        int scrollbar_height_, scrollbar_y_;
        int drag_offset_y_;
        int item_start_, item_end_;

    private:
        void UpdateScrollBar(InputManager* input);
        void NormalizeScrollY();

};
