//
// Input.cpp
//

#include "Input.hpp"
#include <DxLib.h>
#include <stdint.h>

const size_t Input::TEXT_BUFFER_SIZE = 1024;
const size_t Input::HISTORY_MAX_SIZE = 50;
const int Input::KEY_REPEAT_FRAME = 6;

const int Input::INPUT_MARGIN_X = 8;
const int Input::INPUT_MARGIN_Y = 6;

const int Input::IME_BLOCK_SIZE = 32;
const int Input::IME_MARGIN_BASE = 12;
const int Input::IME_MARGIN_Y = 16;
const int Input::IME_MAX_PAGE_SIZE = 6;
const int Input::IME_MIN_WIDTH = 120;

Input::Input() :
	multiline_(true),
    reverse_color_(false)
{
    input_bg_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("resources/images/gui/gui_inputbox_input_bg.png"), 2, 2, 12, 12);

    ime_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("resources/images/gui/gui_inputbox_ime_bg.png"), 2, 2, 12, 12);

    input_handle_ = MakeKeyInput(400, FALSE, FALSE, FALSE);
    font_handle_ = ResourceManager::default_font_handle();
    font_height_ = ResourceManager::default_font_size();

    cursor_moveto_x_ = -1;
    cursor_moveto_y_ = -1;
    selecting_candidate_ = -1;
    candidate_x_ = -1;
    candidate_y_ = -1;
    blink_count_ = 0;
	prev_cursor_pos_ = -1;

    x_ = 100;
    y_ = 100;
    width_ = 160;
    height_ = font_height_ + 4;
}

void Input::Draw()
{
    {
        int alpha = active() ? 255 : 150;

        if (reverse_color_) {
            SetDrawBlendMode(DX_BLENDMODE_INVSRC, alpha);
        } else {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        }

        DrawGraph(x_, y_, *input_bg_image_handle_[0], TRUE);
        DrawGraph(x_ + width_ - 12, y_, *input_bg_image_handle_[1], TRUE);
        DrawGraph(x_, y_ + height_ - 12, *input_bg_image_handle_[2], TRUE);
        DrawGraph(x_ + width_ - 12, y_ + height_ - 12, *input_bg_image_handle_[3], TRUE);

        DrawRectExtendGraphF(x_ + 12, y_,
                             x_ + width_ - 12, y_ + 12,
                             0, 0, 1, 12, *input_bg_image_handle_[1], TRUE);

        DrawRectExtendGraphF(x_ + 12, y_ + height_ - 12,
                             x_ + width_ - 12, y_ + height_,
                             0, 0, 1, 12, *input_bg_image_handle_[3], TRUE);

        DrawRectExtendGraphF(x_, y_ + 12,
                             x_ + 12, y_ + height_ - 12,
                             0, 0, 12, 1, *input_bg_image_handle_[2], TRUE);

        DrawRectExtendGraphF(x_ + width_ - 12, y_ + 12,
                             x_ + width_, y_ + height_ - 12,
                             0, 0, 12, 1, *input_bg_image_handle_[3], TRUE);

        DrawRectExtendGraphF(x_ +  12, y_ + 12,
                             x_ + width_ - 12, y_ + height_ - 12,
                             0, 0, 1, 1, *input_bg_image_handle_[3], TRUE);

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    {
        int internal_x = x_ + INPUT_MARGIN_X;
        int internal_y = y_ + INPUT_MARGIN_Y;
        // int internal_width = width_ - INPUT_MARGIN * 2;
        // int internal_height = height_ - INPUT_MARGIN * 2;

        // 選択範囲の背景を描画
        //int current_line = 0;
        //for (auto it = selecting_lines_.begin(); it != selecting_lines_.end(); ++it) {
        //    auto line = *it;
        //    if (line.first < line.second) {
        //        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
        //        DrawBox(internal_x + line.first,
        //                internal_y + current_line * font_height_,
        //                internal_x + line.second,
        //                internal_y + (current_line + 1) * font_height_,
        //                GetColor(255, 0, 255), TRUE);
        //        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        //    }
        //    current_line++;
        //}

       int current_line = 0;
       for (auto it = clause_lines_.begin(); it != clause_lines_.end(); ++it) {
            auto line = *it;
            if (line.first < line.second) {
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
                DrawBox(internal_x + line.first,
                        internal_y + current_line * font_height_,
                        internal_x + line.second,
                        internal_y + (current_line + 1) * font_height_,
                        GetColor(0, 255, 255), TRUE);
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
            }
            current_line++;
        }

        current_line = 0;
        candidate_x_ = -1, candidate_y_ = -1;
       for (auto it = selecting_clause_lines_.begin(); it != selecting_clause_lines_.end(); ++it) {
            auto line = *it;
            if (line.first < line.second) {
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
                DrawBox(internal_x + line.first,
                        internal_y + current_line * font_height_,
                        internal_x + line.second,
                        internal_y + (current_line + 1) * font_height_,
                        GetColor(255, 0, 255), TRUE);
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

                if (candidate_x_ < 0) {
                    candidate_x_ = line.first;
                    candidate_y_ = (current_line + 1) * font_height_;
                }
            }
            current_line++;
        }

        int text_color = reverse_color_ ? GetColor(255, 255, 255) : GetColor(0, 0, 0);
        current_line = 0;

        if (message_lines_.size() > 0) {
            for (auto it = message_lines_.begin(); it != message_lines_.end(); ++it) {
                auto line = *it;
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, 140);
                DrawStringToHandle(internal_x,
                    internal_y + current_line * font_height_ - 3, line.c_str(),
                        text_color, font_handle_);
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
                current_line++;
            }
        }

        for (auto it = lines_.begin(); it != lines_.end(); ++it) {
            auto line = *it;
            DrawStringToHandle(internal_x, internal_y + current_line * font_height_,
                    line.c_str(), text_color, font_handle_);
            current_line++;
        }

        // カーソルを描画
        if (clause_lines_.size() <= 0 && active() && blink_count_ < 30) {
            DrawBox(internal_x + cursor_x_, internal_y + cursor_y_,
                    internal_x + cursor_x_ + 2,
                    internal_y + cursor_y_ + font_height_, text_color,
                    TRUE);
        }
    }

    {
        int internal_x = x_ + INPUT_MARGIN_X;
        int internal_y = y_ + INPUT_MARGIN_Y;

        if (selecting_candidate_ >= 0) {

            int start_index = (selecting_candidate_ / IME_MAX_PAGE_SIZE)
                    * IME_MAX_PAGE_SIZE;
            int end_index = std::min(start_index + IME_MAX_PAGE_SIZE,
                    static_cast<int>(candidates_.size() - 1));

            int x = candidate_x_;
            int y;
            int width = IME_MIN_WIDTH;
            int height = (end_index - start_index) * font_height_;

            for (int i = start_index; i < end_index; i++) {
                auto candidate = candidates_[i];
                width = std::max(width,
                        GetDrawStringWidthToHandle(candidate.c_str(),
                                candidate.size(), font_handle_));
            }

            int screen_height;
            GetScreenState(nullptr, &screen_height, nullptr);

            if (internal_y + height + candidate_x_ >= screen_height - IME_MARGIN_Y * 2) {
                y = candidate_y_ - font_height_ - height - IME_MARGIN_Y;
            } else {
                y = candidate_y_ + IME_MARGIN_Y;
            }

            if (height > 0) {
                int base_x = internal_x + x - IME_MARGIN_BASE;
                int base_y = internal_y + y - IME_MARGIN_BASE;
                int base_width = width + IME_MARGIN_BASE * 2;
                int base_height = height + IME_MARGIN_BASE * 2;

                DrawGraph(base_x, base_y, *ime_image_handle_[0], TRUE);
                DrawGraph(base_x + base_width - 12, base_y, *ime_image_handle_[1], TRUE);
                DrawGraph(base_x, base_y + base_height - 12, *ime_image_handle_[2], TRUE);
                DrawGraph(base_x + base_width - 12, base_y + base_height - 12,
                        *ime_image_handle_[3], TRUE);

                DrawRectExtendGraphF(base_x + 12, base_y, base_x + base_width - 12,
                        base_y + 12, 0, 0, 1, 12, *ime_image_handle_[1], TRUE);

                DrawRectExtendGraphF(base_x + 12, base_y + base_height - 12,
                        base_x + base_width - 12, base_y + base_height, 0, 0, 1, 12,
                        *ime_image_handle_[3], TRUE);

                DrawRectExtendGraphF(base_x, base_y + 12, base_x + 12,
                        base_y + base_height - 12, 0, 0, 12, 1, *ime_image_handle_[2],
                        TRUE);

                DrawRectExtendGraphF(base_x + base_width - 12, base_y + 12,
                        base_x + base_width, base_y + base_height - 12, 0, 0, 12, 1,
                        *ime_image_handle_[3], TRUE);

                DrawRectExtendGraphF(base_x + 12, base_y + 12, base_x + base_width - 12,
                        base_y + base_height - 12, 0, 0, 1, 1, *ime_image_handle_[3],
                        TRUE);
            }

            int line_top = 0;
            for (int i = start_index; i < end_index; i++) {
                if (i == selecting_candidate_) {
                    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
                    DrawBox(internal_x + x - 2,
                            internal_y + line_top + y,
                            internal_x + x + width + 2,
                            internal_y + line_top + y + font_height_,
                            GetColor(255, 0, 255), TRUE);
                    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
                }
                DrawStringToHandle(internal_x + x,
                        internal_y + line_top + y,
                        candidates_[i].c_str(), GetColor(0, 0, 0),
                        font_handle_);
                line_top += font_height_;
            }
        }
    }
}

void Input::Update()
{
    blink_count_ = (blink_count_ + 1) % 60;
}

void Input::ProcessInput(InputManager* input)
{
    if (!input) {
        return;
    }

    // bool push_mouse_left = (input->GetMouseLeftCount() > 0);

    // bool first_key_shift = (input->GetKeyCount(KEY_INPUT_RSHIFT) == 1
    //        || input->GetKeyCount(KEY_INPUT_LSHIFT) == 1);

    bool push_key_shift = (input->GetKeyCount(KEY_INPUT_RSHIFT) > 0
            || input->GetKeyCount(KEY_INPUT_LSHIFT) > 0);

    bool push_key_v = (input->GetKeyCount(KEY_INPUT_V) > 0);
    bool push_key_ctrl = (input->GetKeyCount(KEY_INPUT_LCONTROL) > 0
            || input->GetKeyCount(KEY_INPUT_RCONTROL) > 0);
	
    bool push_key_return = (input->GetKeyCount(KEY_INPUT_RETURN) > 0);
    bool first_key_return = (input->GetKeyCount(KEY_INPUT_RETURN) == 1);

    bool push_repeat_key_return = (input->GetKeyCount(KEY_INPUT_RETURN)
            + KEY_REPEAT_FRAME) % (KEY_REPEAT_FRAME + 1) == 0;
    bool push_repeat_key_up = (input->GetKeyCount(KEY_INPUT_UP)
            + KEY_REPEAT_FRAME) % (KEY_REPEAT_FRAME + 1) == 0;
    bool push_repeat_key_down = (input->GetKeyCount(KEY_INPUT_DOWN)
            + KEY_REPEAT_FRAME) % (KEY_REPEAT_FRAME + 1) == 0;
    // bool push_long_backspace = (input->GetKeyCount(KEY_INPUT_BACK) > 60 * 1.5);

    auto input_text = text();
    if (!active() && push_key_v && push_key_ctrl) {
        set_active(true);
    } else {
        if (push_key_shift && push_key_return) {
            SetActiveKeyInput(input_handle_);
        } else if (first_key_return) {
            if (CheckKeyInput(input_handle_) == 1) {
                if (on_enter_) {
                    if (on_enter_(unicode::ToString(input_text))) {
                        SetKeyInputString(_T(""), input_handle_);
                    }
                }
                SetActiveKeyInput(input_handle_);
            }
        }
    }

    if (push_repeat_key_up) {
        cursor_moveto_y_ = cursor_y_ - font_height_ / 2;
        cursor_moveto_x_ = cursor_x_ + 2;
    } else if (push_repeat_key_down) {
        cursor_moveto_y_ = cursor_y_ + font_height_ + font_height_ / 2;
        cursor_moveto_x_ = cursor_x_ + 2;
    }

    if (push_key_shift && push_repeat_key_return && multiline_) {

        auto buffer = text();
        uint32_t pos = GetKeyInputCursorPosition(input_handle_);

        tstring cursor_front_str = buffer.substr(0, pos);       // カーソル前の文字列
        tstring cursor_back_str = buffer.substr(pos);           // カーソル後の文字列
        auto new_string = cursor_front_str + _T('\n') + cursor_back_str;

        SetKeyInputString(new_string.c_str(), input_handle_);
        SetKeyInputCursorPosition(pos + 1, input_handle_);
        CancelSelect();
        cursor_drag_count_ = 0;
    }

    int cursor_byte_pos, cursor_dot_pos;
    int draw_dot_pos = 0;

    TCHAR String[TEXT_BUFFER_SIZE];
    GetKeyInputString(String, input_handle_);

    // 単一行設定の時、最初の行だけを表示
    if (!multiline_) {
        tstring buffer(String, _tcslen(String));
        size_t pos;
        if ((pos = buffer.find(_T('\n'))) != std::string::npos) {
            SetKeyInputString(buffer.substr(0, pos).c_str(), input_handle_);
        }
    }

    int internal_width = width_ - INPUT_MARGIN_X * 2;
    // int internal_height = height_ - INPUT_MARGIN * 2;

    message_lines_.clear();

    if (message_.size() > 0) {
        int current_line = 0;
        tstring line_buffer;
        int line_width = 0;
        int char_count = 0;
        for (auto it = message_.begin(); it != message_.end(); ++it) {

        #ifdef UNICODE
            TCHAR c = *it;
            int width = GetDrawStringWidthToHandle(&c, 1, font_handle_);
            line_buffer += c;
            char_count++;
        #else
            unsigned char c = *it;
            TCHAR string[2] = { 0, 0 };

            int width = 0;
            if ((c >= 0x81 && c <= 0x9f) || (c >= 0xe0 && c <= 0xfc)) {
                string[0] = c;
                string[1] = *(it + 1);
                ++it;
                width = GetDrawStringWidthToHandle(string, 2, font_handle_);
                line_buffer += string[0];
                line_buffer += string[1];
                char_count += 2;
            } else if (c == '\n') {
                char_count++;
            } else {
                string[0] = c;
                width = GetDrawStringWidthToHandle(string, 1, font_handle_);
                line_buffer += string[0];
                char_count++;
            }
        #endif

            line_width += width;
            if (c == _T('\n')
                    || line_width > internal_width - font_height_ / 2) {
                if (!line_buffer.empty() && line_buffer.back() == _T('\n')) {
                    line_buffer.pop_back();
                }
                message_lines_.push_back(line_buffer);
                current_line++;
                line_buffer.clear();
                line_width = 0;
            }
        }

        message_lines_.push_back(line_buffer);
    }

    if (active()) {
        // カーソル位置（byte）を取得
        cursor_byte_pos = GetKeyInputCursorPosition(input_handle_);
		if (prev_cursor_pos_ != cursor_byte_pos) {
			ResetCursorCount();
		}

		prev_cursor_pos_ = cursor_byte_pos;

        // カーソルのドット単位の位置を取得する
        cursor_dot_pos = GetDrawStringWidthToHandle(String, cursor_byte_pos,
                font_handle_);
        draw_dot_pos += cursor_dot_pos;

        tstring cursor_front_str(String, cursor_byte_pos); // カーソル前の文字列
        tstring cursor_back_str(String + cursor_byte_pos); // カーソル後の文字列

        std::vector<tstring> clauses;                              // 入力中の文節
        std::vector<tstring> candidates;                             // 変換候補

        lines_.clear();
        selecting_lines_.clear();
        clause_lines_.clear();
        selecting_clause_lines_.clear();
        candidates_.clear();

        // 文節データを取得
        // int selecting_clause = -1;
        selecting_candidate_ = -1;

        IMEINPUTDATA *ImeData = GetIMEInputData();
        if (ImeData && active()) {
            for (int i = 0; i < ImeData->ClauseNum; i++) {
                clauses.push_back(
                        tstring(
                                ImeData->InputString
                                        + ImeData->ClauseData[i].Position,
                                ImeData->ClauseData[i].Length));
            }
            selecting_clause_ = ImeData->SelectClause;

            if (ImeData->CandidateNum > 0) {
                for (int i = 0; i < ImeData->CandidateNum; i++) {
                    candidates_.push_back(
                            tstring(ImeData->CandidateList[i],
                                    _tcslen(ImeData->CandidateList[i])));
                }

                selecting_candidate_ = ImeData->SelectCandidate;
            }
        }

        // 選択範囲を取得
        int select_start, select_end;
        GetKeyInputSelectArea(&select_start, &select_end, input_handle_);
        if (select_start > select_end) {
            std::swap(select_start, select_end);
        }

        tstring line_buffer;
        int line_width = 0;
        int char_count = 0;

        // カーソル前のデータを描画
        for (auto it = cursor_front_str.begin(); it != cursor_front_str.end();
                ++it) {

        int prev_char_count = char_count;

        #ifdef UNICODE
            TCHAR c = *it;
            int width = GetDrawStringWidthToHandle(&c, 1, font_handle_);
            line_buffer += c;
			char_count++;
        #else
            unsigned char c = *it;
            TCHAR string[2] = { 0, 0 };

            int width = 0;
            if ((c >= 0x81 && c <= 0x9f) || (c >= 0xe0 && c <= 0xfc)) {
                string[0] = c;
                string[1] = *(it + 1);
                ++it;
                width = GetDrawStringWidthToHandle(string, 2, font_handle_);
                line_buffer += string[0];
                line_buffer += string[1];
                char_count += 2;
            } else if (c == '\n') {
                char_count++;
            } else {
                string[0] = c;
                width = GetDrawStringWidthToHandle(string, 1, font_handle_);
                line_buffer += string[0];
                char_count++;
            }
        #endif

            // 選択範囲を記録
            if (select_start < char_count && char_count <= select_end) {
                selecting_lines_.resize(static_cast<int>(lines_.size() + message_lines_.size()) + 1,
                        std::pair<int, int>(99999, 0));
                selecting_lines_[static_cast<int>(lines_.size() + message_lines_.size())].first = std::min(
                        selecting_lines_[static_cast<int>(lines_.size() + message_lines_.size())].first, line_width);
                selecting_lines_[static_cast<int>(lines_.size() + message_lines_.size())].second = std::max(
                        selecting_lines_[static_cast<int>(lines_.size() + message_lines_.size())].second,
                        line_width + std::max(width, 3));
            }

            if (line_width - width / 2 <= cursor_moveto_x_
                    && cursor_moveto_x_ <= line_width + width
                    && static_cast<int>(lines_.size() + message_lines_.size()) * font_height_ <= cursor_moveto_y_
                    && cursor_moveto_y_ <= (static_cast<int>(lines_.size() + message_lines_.size()) + 1) * font_height_) {
                SetKeyInputCursorPosition(prev_char_count, input_handle_);
                cursor_moveto_x_ = -1;
                cursor_moveto_y_ = -1;
            }

            line_width += width;
            if (c == _T('\n') || line_width > internal_width - font_height_ / 2) {
                if (!line_buffer.empty() && line_buffer.back() == _T('\n')) {
                    line_buffer.pop_back();
                }

                lines_.push_back(line_buffer);

                if (cursor_moveto_x_ >= line_width
                        && static_cast<int>(lines_.size() + message_lines_.size()) * font_height_ <= cursor_moveto_y_
                        && cursor_moveto_y_
                                <= (static_cast<int>(lines_.size() + message_lines_.size()) + 1) * font_height_) {
                    if (c == _T('\n')) {
                        SetKeyInputCursorPosition(char_count - 1,
                                input_handle_);
                    } else {
                        SetKeyInputCursorPosition(char_count, input_handle_);
                    }
                    cursor_moveto_x_ = -1;
                    cursor_moveto_y_ = -1;
                }

                line_buffer.clear();
                line_width = 0;
            }
        }

        cursor_x_ = line_width;
        cursor_y_ = static_cast<int>(lines_.size() + message_lines_.size()) * font_height_;

        // 変換中データを描画
        for (uint32_t index = 0; index < clauses.size(); index++) {
            for (auto it = clauses[index].begin(); it != clauses[index].end();
                    ++it) {
        #ifdef UNICODE
            TCHAR c = *it;
            int width = GetDrawStringWidthToHandle(&c, 1, font_handle_);
            line_buffer += tstring(&c, 1);
			char_count++;
        #else
            unsigned char c = *it;
            TCHAR string[2] = { 0, 0 };

            int width = 0;
            if ((c >= 0x81 && c <= 0x9f) || (c >= 0xe0 && c <= 0xfc)) {
                string[0] = c;
                string[1] = *(it + 1);
                ++it;
                width = GetDrawStringWidthToHandle(string, 2, font_handle_);
                line_buffer += string[0];
                line_buffer += string[1];
                char_count += 2;
            } else if (c == '\n') {
                char_count++;
            } else {
                string[0] = c;
                width = GetDrawStringWidthToHandle(string, 1, font_handle_);
                line_buffer += string[0];
                char_count++;
            }
        #endif

                clause_lines_.resize(static_cast<int>(lines_.size() + message_lines_.size()) + 1,
                        std::pair<int, int>(99999, 0));
                clause_lines_[static_cast<int>(lines_.size() + message_lines_.size())].first = std::min(
                        clause_lines_[static_cast<int>(lines_.size() + message_lines_.size())].first, line_width);
                clause_lines_[static_cast<int>(lines_.size() + message_lines_.size())].second = std::max(
                        clause_lines_[static_cast<int>(lines_.size() + message_lines_.size())].second, line_width + width);

                if (index == static_cast<uint32_t>(selecting_clause_)) {
                    selecting_clause_lines_.resize(static_cast<int>(lines_.size() + message_lines_.size()) + 1,
                            std::pair<int, int>(99999, 0));
                    selecting_clause_lines_[static_cast<int>(lines_.size() + message_lines_.size())].first = std::min(
                            selecting_clause_lines_[static_cast<int>(lines_.size() + message_lines_.size())].first,
                            line_width);
                    selecting_clause_lines_[static_cast<int>(lines_.size() + message_lines_.size())].second = std::max(
                            selecting_clause_lines_[static_cast<int>(lines_.size() + message_lines_.size())].second,
                            line_width + width);
                }

                line_width += width;
                if (line_width > internal_width - font_height_ / 2) {
                    lines_.push_back(line_buffer);
                    line_buffer.clear();
                    line_width = 0;
                }
            }
        }

        // カーソル後のデータを描画
        for (auto it = cursor_back_str.begin(); it != cursor_back_str.end();
                ++it) {
            int prev_char_count = char_count;

        #ifdef UNICODE
            TCHAR c = *it;
            int width = GetDrawStringWidthToHandle(&c, 1, font_handle_);
            line_buffer += tstring(&c, 1);
        #else
            unsigned char c = *it;
            TCHAR string[2] = { 0, 0 };

            int width = 0;
            if ((c >= 0x81 && c <= 0x9f) || (c >= 0xe0 && c <= 0xfc)) {
                string[0] = c;
                string[1] = *(it + 1);
                ++it;
                width = GetDrawStringWidthToHandle(string, 2, font_handle_);
                line_buffer += string[0];
                line_buffer += string[1];
                char_count += 2;
            } else if (c == '\n') {
                char_count++;
            } else {
                string[0] = c;
                width = GetDrawStringWidthToHandle(string, 1, font_handle_);
                line_buffer += string[0];
                char_count++;
            }
        #endif

            // 選択範囲を記録
            if (select_start < char_count && char_count <= select_end) {
                selecting_lines_.resize(static_cast<int>(lines_.size() + message_lines_.size()) + 1,
                        std::pair<int, int>(99999, 0));
                selecting_lines_[static_cast<int>(lines_.size() + message_lines_.size())].first = std::min(
                        selecting_lines_[static_cast<int>(lines_.size() + message_lines_.size())].first, line_width);
                selecting_lines_[static_cast<int>(lines_.size() + message_lines_.size())].second = std::max(
                        selecting_lines_[static_cast<int>(lines_.size() + message_lines_.size())].second,
                        line_width + std::max(width, 3));
            }

            if (line_width - width / 2 <= cursor_moveto_x_
                    && cursor_moveto_x_ <= line_width + width
                    && static_cast<int>(lines_.size() + message_lines_.size()) * font_height_ <= cursor_moveto_y_
                    && cursor_moveto_y_ <= (static_cast<int>(lines_.size() + message_lines_.size()) + 1) * font_height_) {
                SetKeyInputCursorPosition(prev_char_count, input_handle_);
                cursor_moveto_x_ = -1;
                cursor_moveto_y_ = -1;
            }

            line_width += width;
            if (c == _T('\n') || line_width > internal_width - font_height_ / 2) {
                if (!line_buffer.empty() && line_buffer.back() == _T('\n')) {
                    line_buffer.pop_back();
                }
                lines_.push_back(line_buffer);

                if (cursor_moveto_x_ >= line_width
                        && static_cast<int>(lines_.size() + message_lines_.size()) * font_height_ <= cursor_moveto_y_
                        && cursor_moveto_y_
                                <= (static_cast<int>(lines_.size() + message_lines_.size()) + 1) * font_height_) {
                    if (c == _T('\n')) {
                        SetKeyInputCursorPosition(char_count - 1,
                                input_handle_);
                    } else {
                        SetKeyInputCursorPosition(char_count, input_handle_);
                    }
                    cursor_moveto_x_ = -1;
                    cursor_moveto_y_ = -1;
                }
                line_buffer.clear();
                line_width = 0;
            }
        }

        // バッファの残りを描画
        lines_.push_back(line_buffer);

        if (cursor_moveto_x_ >= line_width
                && static_cast<int>(lines_.size() + message_lines_.size()) * font_height_ <= cursor_moveto_y_
                && static_cast<int>(cursor_moveto_y_) <= static_cast<int>(lines_.size() + message_lines_.size()) * font_height_) {
            SetKeyInputCursorPosition(char_count, input_handle_);
            cursor_moveto_x_ = -1;
            cursor_moveto_y_ = -1;
        }

        height_ = static_cast<int>(lines_.size() + message_lines_.size()) * font_height_ + INPUT_MARGIN_Y * 2;
        // height_ = max(height_, min_height_);

        line_buffer.clear();
        line_width = 0;

        //if (push_key_shift && (push_repeat_key_up || push_repeat_key_down)) {
        //    SetKeyInputSelectArea(GetKeyInputCursorPosition(input_handle_),
        //            prev_cursor_pos_, input_handle_);
        //}

    }

    if (active()) {
        input->CancelKeyCountAll();
    }
}

bool Input::active()
{
    return GetActiveKeyInput() == input_handle_;
}

void Input::set_active(bool flag)
{
    if (active() && !flag) {
        ReStartKeyInput(input_handle_);
        SetActiveKeyInput(-1);
    } else if (!active() && flag) {
        SetActiveKeyInput(input_handle_);
        ResetCursorCount();
    }
}

void Input::ResetCursorCount()
{
    blink_count_ = 0;
}

void Input::CancelSelect()
{
    int pos = GetKeyInputCursorPosition(input_handle_);
    // prev_cursor_pos_ = pos;
    SetKeyInputSelectArea(-1, -1, input_handle_);
}

int Input::x() const
{
    return x_;
}

void Input::set_x(int x)
{
    x_ = x;
}

int Input::y() const
{
    return y_;
}

void Input::set_y(int y)
{
    y_ = y;
}

int Input::width() const
{
    return width_;
}

void Input::set_width(int width)
{
    width_ = width;
}

int Input::height() const
{
    return height_;
}

void Input::set_height(int height)
{
    height_ = height;
}

tstring Input::text() const
{
    TCHAR String[TEXT_BUFFER_SIZE];
    GetKeyInputString(String, input_handle_);
    return tstring(String, _tcslen(String));
}

void Input::set_text(const tstring& text)
{
    SetKeyInputString(text.c_str(), input_handle_);
}

tstring Input::message() const
{
    return message_;
}

void Input::set_message(const tstring& message)
{
    message_ = message;
}

void Input::set_on_enter(const CallbackFunc& func)
{
    on_enter_ = func;
}

bool Input::reverse_color() const
{
    return reverse_color_;
}

void Input::set_reverse_color(bool flag)
{
    reverse_color_ = flag;
}

bool Input::multiline() const
{
    return multiline_;
}

void Input::set_multiline(bool flag)
{
    multiline_ = flag;
}