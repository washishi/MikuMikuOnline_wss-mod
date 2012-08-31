//
// Input.hpp
//

#pragma once

#include <array>
#include <functional>
#include "../ResourceManager.hpp"
#include "../InputManager.hpp"

class Input {
        typedef std::function<bool(const std::string&)> CallbackFunc;

    public:
        Input();

        void Draw();
        void Update();
        void ProcessInput(InputManager* input);

        std::string GetInputString();

        bool active();
        void set_active(bool flag);

        int x() const;
        void set_x(int x);
        int y() const;
        void set_y(int y);
        int width() const;
        void set_width(int width);
        int height() const;
        void set_height(int height);

        tstring text() const;
        void set_text(const tstring& text);

        tstring message() const;
        void set_message(const tstring& message);

        void set_on_enter(const CallbackFunc& func);

    private:
        void ResetCursorCount();
        void CancelSelect();

    private:
        int x_, y_, width_, height_;
        int cursor_x_, cursor_y_;

        std::array<ImageHandlePtr, 4> input_bg_image_handle_;
        std::array<ImageHandlePtr, 4> ime_image_handle_;

        int input_handle_, font_handle_, font_height_;

        std::vector<std::string> message_lines_;
        std::vector<std::string> lines_;
        std::vector<tstring> candidates_;

        int candidate_x_, candidate_y_;
        int selecting_candidate_, selecting_clause_;
        int cursor_moveto_x_, cursor_moveto_y_;
        int prev_cursor_pos_, cursor_drag_count_;
        int multiline_;

        std::vector<std::pair<int, int>> selecting_lines_;
        std::vector<std::pair<int, int>> clause_lines_;
        std::vector<std::pair<int, int>> selecting_clause_lines_;

        CallbackFunc on_enter_;
        tstring message_;

        int blink_count_;

    private:
        const static size_t HISTORY_MAX_SIZE;

        const static int DEFAULT_MAX_WIDTH;

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
