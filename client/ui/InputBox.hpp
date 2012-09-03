//
// InputBox.hpp
//

#pragma once
#include <array>
#include <vector>
#include <list>
#include <string>
#include "../ManagerAccessor.hpp"
#include "../InputManager.hpp"
#include "../ScriptEnvironment.hpp"
#include "../ResourceManager.hpp"
#include "../Card.hpp"
#include "Input.hpp"

class InputBox {
    public:
        InputBox(const ManagerAccessorPtr& manager_accessor);
        ~InputBox();

        void Draw();
        void Update();
        void ProcessInput(InputManager* input);
        void ReloadTabs();

        bool IsActive();
        void Activate();
        void Inactivate();

        int multiline() const;
        void set_multiline(int multiline);

    private:
        void CancelSelect();

        void DrawBase();
        void DrawTabs();

        void ProcessInputTabs(InputManager* input);
        void UpdateTabs();

        void UpdateBase(InputManager* input);

        void SwitchTab(int index);
        tstring GetInputString() const;
        void SetInputString(const tstring& text);

        bool IsScriptMode();
        void SetScriptMode();

    private:
        int x_, y_, width_, height_;
        int multiline_;

    private:
        struct Tab {
            CardWeakPtr card;
            tstring name;
            tstring message;
            tstring text;
            int x, y, width, height;
        };

        Input input_;

        int font_handle_;
        int font_height_;
        std::array<ImageHandlePtr, 4> bg_image_handle_;
        std::array<ImageHandlePtr, 4> tab_bg_image_handle_;
        std::array<ImageHandlePtr, 4> tab_bg_inactive_image_handle_;
        std::array<ImageHandlePtr, 4> ime_image_handle_;
        ImageHandlePtr script_icon_image_handle_;

        int drag_offset_x_, drag_offset_y_;
        int drag_resize_offset_x_, drag_resize_offset_y_;
        int min_input_height_;

        int selecting_tab_index_;
        std::vector<InputBox::Tab> tabs_;
        Tab script_tab_;

        std::vector<std::string> tab_texts_;

        ManagerAccessorPtr manager_accessor_;
        CardPtr card_;

        const static size_t HISTORY_MAX_SIZE;

        const static int DEFAULT_MAX_WIDTH;

        const static int TAB_TOP_MARGIN;
        const static int TAB_SIDE_MARGIN;
        const static int TAB_X_MARGIN;
        const static int MESSAGE_TOP_MARGIN;

        const static int BOX_MIN_WIDTH;
        const static int BOX_TOP_MARGIN;
        const static int BOX_BOTTOM_MARGIN;
        const static int BOX_SIDE_MARGIN;

        const static int KEY_REPEAT_FRAME;

        const static int INPUT_MARGIN_X;
        const static int INPUT_MARGIN_Y;

        const static int IME_BLOCK_SIZE;
        const static int IME_MARGIN_BASE;
        const static int IME_MARGIN_Y;
        const static int IME_MAX_PAGE_SIZE;
        const static int IME_MIN_WIDTH;
};
