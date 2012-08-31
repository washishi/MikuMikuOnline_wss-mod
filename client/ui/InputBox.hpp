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

        void UpdateCursorCount();
        void ResetCursorCount();

        void DrawBase();
        void DrawTabs();
        void DrawInputBase();
        void DrawInputText();
        void DrawCandidates();

        void ProcessInputTabs(InputManager* input);
        void UpdateTabs();

        void UpdateBase(InputManager* input);

        void AddInputHistory(const std::string& text);

        void SwitchTab(int index);
        std::string GetInputString() const;
        void SetInputString(const std::string& text);

        bool IsScriptMode();
        void SetScriptMode();

    private:
        int x_, y_, width_, height_;
        int multiline_;

    private:
        struct Tab {
            CardWeakPtr card;
            std::string name;
            std::string message;
            std::string text;
            int x, y, width, height;
        };

        int input_handle_;
        int font_handle_;
        int font_height_;
        std::array<ImageHandlePtr, 4> bg_image_handle_;
        std::array<ImageHandlePtr, 4> input_bg_image_handle_;
        std::array<ImageHandlePtr, 4> tab_bg_image_handle_;
        std::array<ImageHandlePtr, 4> tab_bg_inactive_image_handle_;
        std::array<ImageHandlePtr, 4> ime_image_handle_;
        ImageHandlePtr script_icon_image_handle_;

        int drag_offset_x_, drag_offset_y_;
        int drag_resize_offset_x_, drag_resize_offset_y_;
        int min_input_height_;

        int input_x_, input_y_, input_width_, input_height_;
        int cursor_x_, cursor_y_;
        int cursor_moveto_x_, cursor_moveto_y_;
        int prev_cursor_pos_, cursor_drag_count;
        int selecting_candidate_, selecting_clause_;
        int candidate_x_, candidate_y_;

        int selecting_tab_index_;
        std::vector<InputBox::Tab> tabs_;
        Tab script_tab_;

        std::vector<std::string> message_lines_;
        std::vector<std::string> lines_;
        std::vector<tstring> candidates_;

        std::list<std::string> history_;

        std::vector<std::pair<int, int>> selecting_lines_;
        std::vector<std::pair<int, int>> clause_lines_;
        std::vector<std::pair<int, int>> selecting_clause_lines_;

        std::vector<std::string> tab_texts_;

        int blink_count_;
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
