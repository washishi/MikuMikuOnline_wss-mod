//
// InputBox.cpp
//

#include "InputBox.hpp"
#include "../CardManager.hpp"
#include "../../common/Logger.hpp"
#include "../../common/unicode.hpp"

const size_t InputBox::HISTORY_MAX_SIZE = 50;

const int InputBox::DEFAULT_MAX_WIDTH = 600;

const int InputBox::TAB_TOP_MARGIN = 7;
const int InputBox::TAB_SIDE_MARGIN = 30;
const int InputBox::TAB_X_MARGIN = 15;
const int InputBox::MESSAGE_TOP_MARGIN = 30;

const int InputBox::BOX_MIN_WIDTH = 200;
const int InputBox::BOX_TOP_MARGIN = 36;
const int InputBox::BOX_BOTTOM_MARGIN = 16;
const int InputBox::BOX_SIDE_MARGIN = 6;

const int InputBox::KEY_REPEAT_FRAME = 6;

const int InputBox::INPUT_MARGIN_X = 8;
const int InputBox::INPUT_MARGIN_Y = 6;

const int InputBox::IME_BLOCK_SIZE = 32;
const int InputBox::IME_MARGIN_BASE = 12;
const int InputBox::IME_MARGIN_Y = 16;
const int InputBox::IME_MAX_PAGE_SIZE = 6;
const int InputBox::IME_MIN_WIDTH = 120;

InputBox::InputBox(const ManagerAccessorPtr& manager_accessor) :
                x_(100),
                y_(100),
                width_(800),
                height_(100),
                multiline_(true),
                input_handle_(MakeKeyInput(400, FALSE, FALSE, FALSE)),
                font_height_(ResourceManager::default_font_size()),
                drag_offset_x_(-1),
                drag_offset_y_(-1),
                drag_resize_offset_x_(-1),
                drag_resize_offset_y_(-1),
                min_input_height_(font_height_ + INPUT_MARGIN_Y * 2),
                cursor_moveto_x_(-1),
                cursor_moveto_y_(-1),
                selecting_candidate_(-1),
                candidate_x_(-1),
                candidate_y_(-1),
                selecting_tab_index_(0),
                blink_count_(0),
                manager_accessor_(manager_accessor),
                card_(std::make_shared<Card>(manager_accessor_, "", "immo", "", "", "",
                            std::vector<std::string>(), false, false))

{
    font_handle_ = ResourceManager::default_font_handle();
    bg_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("resources/images/gui/gui_inputbox_bg.png"), 2, 2, 24, 24);
    input_bg_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("resources/images/gui/gui_inputbox_input_bg.png"), 2, 2, 12, 12);
    tab_bg_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("resources/images/gui/gui_inputbox_tab_bg.png"), 2, 2, 12, 12);
    tab_bg_inactive_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("resources/images/gui/gui_inputbox_tab_inactive_bg.png"), 2, 2, 12, 12);
    ime_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("resources/images/gui/gui_inputbox_ime_bg.png"), 2, 2, 12, 12);

    script_icon_image_handle_ = ResourceManager::LoadCachedGraph(_T("resources/images/gui/gui_inputbox_tab_script_icon.png"));

    input_height_ = font_height_ + INPUT_MARGIN_Y * 2;

    int screen_width, screen_height;
    GetScreenState(&screen_width, &screen_height, nullptr);
    width_ = min(DEFAULT_MAX_WIDTH + 0, (int)(screen_width * 0.4));
    height_ = input_height_ + BOX_TOP_MARGIN + BOX_BOTTOM_MARGIN;
    x_ = (screen_width - width_) / 2;
    y_ = screen_height - height_ - BOX_BOTTOM_MARGIN;

    selecting_tab_index_ = 0;


    if (auto card_manager = manager_accessor_->card_manager().lock()) {
        card_manager->AddCard(card_);
    }
}

InputBox::~InputBox()
{
    DeleteKeyInput(input_handle_);
}

void InputBox::DrawTabs()
{

    {
        int tab_index = 0;
        SetDrawArea(x_, script_tab_.y, script_tab_.x, script_tab_.y + script_tab_.height);
        for (auto it = tabs_.begin(); it != tabs_.end(); ++it) {
            auto tab = *it;

            std::array<ImageHandlePtr, 4>* image_handle;
            if (selecting_tab_index_ == tab_index) {
                image_handle = &tab_bg_image_handle_;
            } else {
                image_handle = &tab_bg_inactive_image_handle_;
            }
            int tab_x = tab.x;
            int tab_y = tab.y;
            int tab_width = tab.width;
            int tab_height = tab.height;

            DrawGraph(tab_x, tab_y, *(*image_handle)[0], TRUE);
            DrawGraph(tab_x + tab_width - 12, tab_y, *(*image_handle)[1], TRUE);
            DrawGraph(tab_x, tab_y + tab_height - 12, *(*image_handle)[2], TRUE);
            DrawGraph(tab_x + tab_width - 12, tab_y + tab_height - 12,
                    *(*image_handle)[3], TRUE);

            DrawRectExtendGraphF(tab_x + 12, tab_y, tab_x + tab_width - 12,
                    tab_y + 12, 0, 0, 1, 12, *(*image_handle)[1], TRUE);

            DrawRectExtendGraphF(tab_x + 12, tab_y + tab_height - 12,
                    tab_x + tab_width - 12, tab_y + tab_height, 0, 0, 1, 12,
                    *(*image_handle)[3], TRUE);

            DrawRectExtendGraphF(tab_x, tab_y + 12, tab_x + 12,
                    tab_y + tab_height - 12, 0, 0, 12, 1, *(*image_handle)[2],
                    TRUE);

            DrawRectExtendGraphF(tab_x + tab_width - 12, tab_y + 12,
                    tab_x + tab_width, tab_y + tab_height - 12, 0, 0, 12, 1,
                    *(*image_handle)[3], TRUE);

            DrawRectExtendGraphF(tab_x + 12, tab_y + 12, tab_x + tab_width - 12,
                    tab_y + tab_height - 12, 0, 0, 1, 1, *(*image_handle)[3],
                    TRUE);

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
            DrawStringToHandle(tab_x + 9, y_ + TAB_TOP_MARGIN + 1, unicode::ToTString(tab.name).c_str(),
                    GetColor(255, 255, 255), font_handle_);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

            DrawStringToHandle(tab_x + 9, y_ + TAB_TOP_MARGIN, unicode::ToTString(tab.name).c_str(),
                    GetColor(0, 0, 0), font_handle_);

            tab_index++;
        }
        SetDrawAreaFull();
    }

    {
        SetDrawBright(210, 47, 47);
        // スクリプトタブ
        int tab_x = script_tab_.x;
        int tab_y = script_tab_.y;
        int tab_width = script_tab_.width;
        int tab_height = script_tab_.height;

        std::array<ImageHandlePtr, 4>* image_handle;
        if (IsScriptMode()) {
            image_handle = &tab_bg_image_handle_;
        } else {
            image_handle = &tab_bg_inactive_image_handle_;
        }

        DrawGraph(tab_x, tab_y, *(*image_handle)[0], TRUE);
        DrawGraph(tab_x + tab_width - 12, tab_y, *(*image_handle)[1], TRUE);
        DrawGraph(tab_x, tab_y + tab_height - 12, *(*image_handle)[2], TRUE);
        DrawGraph(tab_x + tab_width - 12, tab_y + tab_height - 12,
                *(*image_handle)[3], TRUE);

        DrawRectExtendGraphF(tab_x + 12, tab_y, tab_x + tab_width - 12,
                tab_y + 12, 0, 0, 1, 12, *(*image_handle)[1], TRUE);

        DrawRectExtendGraphF(tab_x + 12, tab_y + tab_height - 12,
                tab_x + tab_width - 12, tab_y + tab_height, 0, 0, 1, 12,
                *(*image_handle)[3], TRUE);

        DrawRectExtendGraphF(tab_x, tab_y + 12, tab_x + 12,
                tab_y + tab_height - 12, 0, 0, 12, 1, *(*image_handle)[2], TRUE);

        DrawRectExtendGraphF(tab_x + tab_width - 12, tab_y + 12,
                tab_x + tab_width, tab_y + tab_height - 12, 0, 0, 12, 1,
                *(*image_handle)[3], TRUE);

        DrawRectExtendGraphF(tab_x + 12, tab_y + 12, tab_x + tab_width - 12,
                tab_y + tab_height - 12, 0, 0, 1, 1, *(*image_handle)[3], TRUE);

        SetDrawBright(255, 255, 255);

        DrawGraph(x_ + width_ - TAB_SIDE_MARGIN - 16, y_ + TAB_TOP_MARGIN,
                *script_icon_image_handle_, TRUE);
    }
}

void InputBox::DrawBase()
{
    if (IsActive()) {
        SetDrawBlendMode(DX_BLENDMODE_ADD, 255);
    } else {
        SetDrawBlendMode(DX_BLENDMODE_ADD, 150);
    }

    DrawGraph(x_, y_, *bg_image_handle_[0], TRUE);
    DrawGraph(x_ + width_ - 24, y_, *bg_image_handle_[1], TRUE);
    DrawGraph(x_, y_ + height_ - 24, *bg_image_handle_[2], TRUE);
    DrawGraph(x_ + width_ - 24, y_ + height_ - 24, *bg_image_handle_[3], TRUE);

    DrawRectExtendGraphF(x_ + 24, y_,
                         x_ + width_ - 24, y_ + 24,
                         0, 0, 1, 24, *bg_image_handle_[1], TRUE);

    DrawRectExtendGraphF(x_ + 24, y_ + height_ - 24,
                         x_ + width_ - 24, y_ + height_,
                         0, 0, 1, 24, *bg_image_handle_[3], TRUE);

    DrawRectExtendGraphF(x_, y_ + 24,
                         x_ + 24, y_ + height_ - 24,
                         0, 0, 24, 1, *bg_image_handle_[2], TRUE);

    DrawRectExtendGraphF(x_ + width_ - 24, y_ + 24,
                         x_ + width_, y_ + height_ - 24,
                         0, 0, 24, 1, *bg_image_handle_[3], TRUE);

    DrawRectExtendGraphF(x_ +  24, y_ + 24,
                         x_ + width_ - 24, y_ + height_ - 24,
                         0, 0, 1, 1, *bg_image_handle_[3], TRUE);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void InputBox::DrawInputBase()
{
    int input_bg_alpha;
    if (IsActive()) {
        input_bg_alpha = 255;
    } else {
        input_bg_alpha = 100;
    }

    if (IsScriptMode()) {
        SetDrawBlendMode(DX_BLENDMODE_INVSRC, input_bg_alpha);
    } else {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, input_bg_alpha);
    }

    DrawGraph(input_x_, input_y_, *input_bg_image_handle_[0], TRUE);
    DrawGraph(input_x_ + input_width_ - 12, input_y_, *input_bg_image_handle_[1], TRUE);
    DrawGraph(input_x_, input_y_ + input_height_ - 12, *input_bg_image_handle_[2], TRUE);
    DrawGraph(input_x_ + input_width_ - 12, input_y_ + input_height_ - 12, *input_bg_image_handle_[3], TRUE);

    DrawRectExtendGraphF(input_x_ + 12, input_y_,
                         input_x_ + input_width_ - 12, input_y_ + 12,
                         0, 0, 1, 12, *input_bg_image_handle_[1], TRUE);

    DrawRectExtendGraphF(input_x_ + 12, input_y_ + input_height_ - 12,
                         input_x_ + input_width_ - 12, input_y_ + input_height_,
                         0, 0, 1, 12, *input_bg_image_handle_[3], TRUE);

    DrawRectExtendGraphF(input_x_, input_y_ + 12,
                         input_x_ + 12, input_y_ + input_height_ - 12,
                         0, 0, 12, 1, *input_bg_image_handle_[2], TRUE);

    DrawRectExtendGraphF(input_x_ + input_width_ - 12, input_y_ + 12,
                         input_x_ + input_width_, input_y_ + input_height_ - 12,
                         0, 0, 12, 1, *input_bg_image_handle_[3], TRUE);

    DrawRectExtendGraphF(input_x_ +  12, input_y_ + 12,
                         input_x_ + input_width_ - 12, input_y_ + input_height_ - 12,
                         0, 0, 1, 1, *input_bg_image_handle_[3], TRUE);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void InputBox::DrawInputText()
{
    int internal_x = input_x_ + INPUT_MARGIN_X;
    int internal_y = input_y_ + INPUT_MARGIN_Y;
    // int internal_width = input_width_ - INPUT_MARGIN * 2;
    // int internal_height = input_height_ - INPUT_MARGIN * 2;

    // 選択範囲の背景を描画
    int current_line = 0;
    for (auto it = selecting_lines_.begin(); it != selecting_lines_.end(); ++it) {
        auto line = *it;
        if (line.first < line.second) {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
            DrawBox(internal_x + line.first,
                    internal_y + current_line * font_height_,
                    internal_x + line.second,
                    internal_y + (current_line + 1) * font_height_,
                    GetColor(255, 0, 255), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }
        current_line++;
    }

    current_line = 0;
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

    int text_color = IsScriptMode() ? GetColor(255, 255, 255) : GetColor(0, 0, 0);
    current_line = 0;

    if (message_lines_.size() > 0) {
    for (auto it = message_lines_.begin(); it != message_lines_.end(); ++it) {
        auto line = *it;
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 140);
            DrawStringToHandle(internal_x,
                    internal_y + current_line * font_height_ - 3, unicode::ToTString(line).c_str(),
                    text_color, font_handle_);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
            current_line++;
        }
    }

    for (auto it = lines_.begin(); it != lines_.end(); ++it) {
        auto line = *it;
        DrawStringToHandle(internal_x, internal_y + current_line * font_height_,
            unicode::ToTString(line).c_str(), text_color, font_handle_);
        current_line++;
    }

    // カーソルを描画
    if (clause_lines_.size() <= 0 && IsActive() && blink_count_ < 30) {
        DrawBox(internal_x + cursor_x_, internal_y + cursor_y_,
                internal_x + cursor_x_ + 2,
                internal_y + cursor_y_ + font_height_, text_color,
                TRUE);
    }
}

void InputBox::DrawCandidates()
{
    int internal_x = input_x_ + INPUT_MARGIN_X;
    int internal_y = input_y_ + INPUT_MARGIN_Y;

    if (selecting_candidate_ >= 0) {

        int start_index = (selecting_candidate_ / IME_MAX_PAGE_SIZE)
                * IME_MAX_PAGE_SIZE;
        int end_index = min(start_index + IME_MAX_PAGE_SIZE,
                static_cast<int>(candidates_.size() - 1));

        int x = candidate_x_;
        int y;
        int width = IME_MIN_WIDTH;
        int height = (end_index - start_index) * font_height_;

        for (int i = start_index; i < end_index; i++) {
            auto candidate = unicode::ToTString(candidates_[i]);
            width = max(width,
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
                    unicode::ToTString(candidates_[i]).c_str(), GetColor(0, 0, 0),
                    font_handle_);
            line_top += font_height_;
        }
    }
}

void InputBox::Draw()
{
    DrawBase();
    DrawInputBase();
    DrawInputText();
    DrawTabs();
    DrawCandidates();
    // card_->Draw();
}

int InputBox::multiline() const
{
    return multiline_;
}

void InputBox::set_multiline(int multiline)
{
    multiline_ = multiline;
}

void InputBox::CancelSelect()
{
    int pos = GetKeyInputCursorPosition(input_handle_);
    prev_cursor_pos_ = pos;
    SetKeyInputSelectArea(-1, -1, input_handle_);
}

void InputBox::UpdateCursorCount()
{
    blink_count_ += 1;
    blink_count_ %= 60;
}

void InputBox::ResetCursorCount()
{
    blink_count_ = 0;
}

bool InputBox::IsScriptMode()
{
    return selecting_tab_index_ == -1;
}

void InputBox::SetScriptMode()
{
    SwitchTab(-1);
}

void InputBox::SwitchTab(int index)
{
    if (selecting_tab_index_ == -1) {
        script_tab_.text = GetInputString();
    } else if (!tabs_.empty()) {
        tabs_.at(selecting_tab_index_).text = GetInputString();
    }

    selecting_tab_index_ = index;

    if (selecting_tab_index_ == -1) {
        SetInputString(script_tab_.text);
    } else if (!tabs_.empty()) {
        SetInputString(tabs_.at(selecting_tab_index_).text);
    }
}

std::string InputBox::GetInputString() const {
    TCHAR String[1024];
    GetKeyInputString(String, input_handle_);
    return unicode::ToString(tstring(String, _tcslen(String)));
}

void InputBox::SetInputString(const std::string& text)
{
    SetKeyInputString(unicode::ToTString(text).c_str(), input_handle_);
}


void InputBox::AddInputHistory(const std::string& text)
{
    history_.push_back(text);
    if (history_.size() > HISTORY_MAX_SIZE) {
        history_.pop_front();
    }
}

void InputBox::ProcessInput(InputManager* input)
{
    if (!input) {
        return;
    }

    bool push_mouse_left = (input->GetMouseLeftCount() > 0);

    bool first_key_shift = (input->GetKeyCount(KEY_INPUT_RSHIFT) == 1
            || input->GetKeyCount(KEY_INPUT_LSHIFT) == 1);

    bool push_key_shift = (input->GetKeyCount(KEY_INPUT_RSHIFT) > 0
            || input->GetKeyCount(KEY_INPUT_LSHIFT) > 0);

    bool push_key_v = (input->GetKeyCount(KEY_INPUT_V) > 0);
    bool push_key_ctrl = (input->GetKeyCount(KEY_INPUT_LCONTROL) > 0
            || input->GetKeyCount(KEY_INPUT_RCONTROL) > 0);

    bool first_key_return = (input->GetKeyCount(KEY_INPUT_RETURN) == 1);

    bool push_repeat_key_return = (input->GetKeyCount(KEY_INPUT_RETURN)
            + KEY_REPEAT_FRAME) % (KEY_REPEAT_FRAME + 1) == 0;
    bool push_repeat_key_up = (input->GetKeyCount(KEY_INPUT_UP)
            + KEY_REPEAT_FRAME) % (KEY_REPEAT_FRAME + 1) == 0;
    bool push_repeat_key_down = (input->GetKeyCount(KEY_INPUT_DOWN)
            + KEY_REPEAT_FRAME) % (KEY_REPEAT_FRAME + 1) == 0;
    bool push_long_backspace = (input->GetKeyCount(KEY_INPUT_BACK) > 60 * 1.5);

    auto input_text = GetInputString();
    if (!IsActive() && first_key_return && CheckKeyInput(input_handle_) == 0) {
        Activate();
    } else if (!IsActive() && push_key_v && push_key_ctrl) {
        Activate();
    } else {
        if (push_key_shift && first_key_return) {
            SetActiveKeyInput(input_handle_);
        } else if (first_key_return) {
            if (input_text.size() == 0) {
                Inactivate();
            } else if (CheckKeyInput(input_handle_) == 1) {
                AddInputHistory(input_text);
                if (selecting_tab_index_ == -1) {
                    SetKeyInputString(_T(""), input_handle_);
                    card_->Execute(input_text, "immo",
                            [&](const v8::Handle<v8::Value>& result, const std::string& error){
                        if (error.size() > 0) {
                            script_tab_.message = error;
                        } else if (!result.IsEmpty()) {
                            script_tab_.message = "=> " + std::string(*String::Utf8Value(result->ToString()));
                        } else {
                            script_tab_.message = "";
                        }
                    });
                } else if (!tabs_.empty()) {
                    SetKeyInputString(_T(""), input_handle_);
                    if (auto card =
                            tabs_.at(selecting_tab_index_).card.lock()) {
                        card->onEnter(input_text);
                    }
                }
                SetActiveKeyInput(input_handle_);
            }
        }
    }

    if (IsActive() && push_long_backspace && GetKeyInputCursorPosition(input_handle_) == 0) {
        if (selecting_tab_index_ == -1) {
            script_tab_.message.clear();
        } else {
            tabs_.at(selecting_tab_index_).message.clear();
        }
    }

    ProcessInputTabs(input);
    UpdateBase(input);
    UpdateTabs();

    if (push_mouse_left) {
        if (cursor_drag_count == 1) {
            prev_cursor_pos_ = GetKeyInputCursorPosition(input_handle_);
        }
        cursor_drag_count++;
        //input->CancelMouseLeft();
    } else {
        cursor_drag_count = 0;
    }

    if (first_key_shift) {
        prev_cursor_pos_ = GetKeyInputCursorPosition(input_handle_);
    }

    input_x_ = x_ + BOX_SIDE_MARGIN;
    input_y_ = y_ + BOX_TOP_MARGIN;
    input_width_ = width_ - BOX_SIDE_MARGIN * 2;

    bool hover = (x_ <= input->GetMouseX() && input->GetMouseX() <= x_ + width_
            && y_ <= input->GetMouseY() && input->GetMouseY() <= y_ + height_);
    if (push_mouse_left && hover) {
        auto pos = input->GetMousePos();
        int internal_x = input_x_ + INPUT_MARGIN_X;
        int internal_y = input_y_ + INPUT_MARGIN_Y;
        cursor_moveto_x_ = pos.first - internal_x;
        cursor_moveto_y_ = pos.second - internal_y;
        ResetCursorCount();
        input->CancelMouseLeft();
    }

    if (cursor_drag_count > 1) {
        SetKeyInputSelectArea(GetKeyInputCursorPosition(input_handle_),
                prev_cursor_pos_, input_handle_);
    }

    // カーソル移動中は強制表示
    if (input->GetKeyCount(KEY_INPUT_RIGHT) > 0
            || input->GetKeyCount(KEY_INPUT_LEFT) > 0) {

        ResetCursorCount();
    }

    if (push_repeat_key_up) {
        cursor_moveto_y_ = cursor_y_ - font_height_ / 2;
        cursor_moveto_x_ = cursor_x_ + 2;
    } else if (push_repeat_key_down) {
        cursor_moveto_y_ = cursor_y_ + font_height_ + font_height_ / 2;
        cursor_moveto_x_ = cursor_x_ + 2;
    }

    if (push_key_shift && push_repeat_key_return && multiline_) {

        auto buffer = GetInputString();
        uint32_t pos = GetKeyInputCursorPosition(input_handle_);

        std::string cursor_front_str = buffer.substr(0, pos);       // カーソル前の文字列
        std::string cursor_back_str = buffer.substr(pos);           // カーソル後の文字列
        auto new_string = cursor_front_str + '\n' + cursor_back_str;

        SetKeyInputString(unicode::ToTString(new_string).c_str(), input_handle_);
        SetKeyInputCursorPosition(pos + 1, input_handle_);
        CancelSelect();
        cursor_drag_count = 0;
    }

    int cursor_byte_pos, cursor_dot_pos;
    int draw_dot_pos = 0;

    TCHAR String[1024];
    GetKeyInputString(String, input_handle_);

    // 単一行設定の時、最初の行だけを表示
    if (!multiline_) {
        tstring buffer(String, _tcslen(String));
        size_t pos;
        if ((pos = buffer.find('\n')) != std::string::npos) {
            SetKeyInputString(buffer.substr(0, pos).c_str(), input_handle_);
        }
    }

    int internal_width = input_width_ - INPUT_MARGIN_X * 2;
    // int internal_height = input_height_ - INPUT_MARGIN * 2;

    std::string* message = nullptr;
    if (selecting_tab_index_ == -1) {
        message = &script_tab_.message;
    } else if (!tabs_.empty()) {
        message = &tabs_[selecting_tab_index_].message;
    }

    message_lines_.clear();

    if (message && message->size() > 0) {
        int current_line = 0;
        std::string line_buffer;
        int line_width = 0;
        int char_count = 0;
        for (auto it = message->begin(); it != message->end(); ++it) {
        #ifdef UNICODE
            TCHAR c = *it;
            int width = GetDrawStringWidthToHandle(&c, 1, font_handle_);
            line_buffer += unicode::ToString(tstring(&c, 1));
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
            if (c == '\n'
                    || line_width > internal_width - font_height_ / 2) {
                message_lines_.push_back(line_buffer);
                current_line++;
                line_buffer.clear();
                line_width = 0;
            }
        }

        message_lines_.push_back(line_buffer);
    }

    if (IsActive()) {
        // カーソル位置（byte）を取得
        cursor_byte_pos = GetKeyInputCursorPosition(input_handle_);

        // カーソルのドット単位の位置を取得する
        cursor_dot_pos = GetDrawStringWidthToHandle(String, cursor_byte_pos,
                font_handle_);
        draw_dot_pos += cursor_dot_pos;

        tstring cursor_front_str(String, cursor_byte_pos); // カーソル前の文字列
        tstring cursor_back_str(String + cursor_byte_pos); // カーソル後の文字列

        std::vector<tstring> clauses;                  // 入力中の文節
        std::vector<tstring> candidates;               // 変換候補

        lines_.clear();
        selecting_lines_.clear();
        clause_lines_.clear();
        selecting_clause_lines_.clear();
        candidates_.clear();

        // 文節データを取得
        // int selecting_clause = -1;
        selecting_candidate_ = -1;

        IMEINPUTDATA *ImeData = GetIMEInputData();
        if (ImeData && IsActive()) {
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

        std::string line_buffer;
        int line_width = 0;
        int char_count = 0;

        // カーソル前のデータを描画
        for (auto it = cursor_front_str.begin(); it != cursor_front_str.end();
                ++it) {
            int prev_char_count = char_count;

        #ifdef UNICODE
            TCHAR c = *it;
            int width = GetDrawStringWidthToHandle(&c, 1, font_handle_);
            line_buffer += unicode::ToString(tstring(&c, 1));
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
                selecting_lines_[static_cast<int>(lines_.size() + message_lines_.size())].first = min(
                        selecting_lines_[static_cast<int>(lines_.size() + message_lines_.size())].first, line_width);
                selecting_lines_[static_cast<int>(lines_.size() + message_lines_.size())].second = max(
                        selecting_lines_[static_cast<int>(lines_.size() + message_lines_.size())].second,
                        line_width + max(width, 3));
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
            if (c == '\n' || line_width > internal_width - font_height_ / 2) {
                lines_.push_back(line_buffer);

                if (cursor_moveto_x_ >= line_width
                        && static_cast<int>(lines_.size() + message_lines_.size()) * font_height_ <= cursor_moveto_y_
                        && cursor_moveto_y_
                                <= (static_cast<int>(lines_.size() + message_lines_.size()) + 1) * font_height_) {
                    if (c == '\n') {
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
            line_buffer += unicode::ToString(tstring(&c, 1));
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
                clause_lines_[static_cast<int>(lines_.size() + message_lines_.size())].first = min(
                        clause_lines_[static_cast<int>(lines_.size() + message_lines_.size())].first, line_width);
                clause_lines_[static_cast<int>(lines_.size() + message_lines_.size())].second = max(
                        clause_lines_[static_cast<int>(lines_.size() + message_lines_.size())].second, line_width + width);

                if (index == static_cast<uint32_t>(selecting_clause_)) {
                    selecting_clause_lines_.resize(static_cast<int>(lines_.size() + message_lines_.size()) + 1,
                            std::pair<int, int>(99999, 0));
                    selecting_clause_lines_[static_cast<int>(lines_.size() + message_lines_.size())].first = min(
                            selecting_clause_lines_[static_cast<int>(lines_.size() + message_lines_.size())].first,
                            line_width);
                    selecting_clause_lines_[static_cast<int>(lines_.size() + message_lines_.size())].second = max(
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
            line_buffer += unicode::ToString(tstring(&c, 1));
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
                selecting_lines_[static_cast<int>(lines_.size() + message_lines_.size())].first = min(
                        selecting_lines_[static_cast<int>(lines_.size() + message_lines_.size())].first, line_width);
                selecting_lines_[static_cast<int>(lines_.size() + message_lines_.size())].second = max(
                        selecting_lines_[static_cast<int>(lines_.size() + message_lines_.size())].second,
                        line_width + max(width, 3));
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
            if (c == '\n' || line_width > internal_width - font_height_ / 2) {
                lines_.push_back(line_buffer);

                if (cursor_moveto_x_ >= line_width
                        && static_cast<int>(lines_.size() + message_lines_.size()) * font_height_ <= cursor_moveto_y_
                        && cursor_moveto_y_
                                <= (static_cast<int>(lines_.size() + message_lines_.size()) + 1) * font_height_) {
                    if (c == '\n') {
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

        input_height_ = static_cast<int>(lines_.size() + message_lines_.size()) * font_height_ + INPUT_MARGIN_Y * 2;
        input_height_ = max(input_height_, min_input_height_);

        int new_height = input_height_ + BOX_TOP_MARGIN + BOX_BOTTOM_MARGIN;

        int screen_height;
        GetScreenState(nullptr, &screen_height, nullptr);
        if (y_ + height_ > screen_height - BOX_TOP_MARGIN) {
            y_ -= new_height - height_;
            input_y_ = y_ + BOX_TOP_MARGIN;
        }

        height_ = new_height;

        line_buffer.clear();
        line_width = 0;

        if (push_key_shift && (push_repeat_key_up || push_repeat_key_down)) {
            SetKeyInputSelectArea(GetKeyInputCursorPosition(input_handle_),
                    prev_cursor_pos_, input_handle_);
        }

    }

    if (IsActive()) {
        input->CancelKeyCountAll();
    }
}

void InputBox::Update()
{
    UpdateCursorCount();
 // if (IsActive) {
}

void InputBox::ReloadTabs()
{
    tabs_.clear();
    if (auto card_manager = manager_accessor_->card_manager().lock()) {
        auto card_list = card_manager->cards();
        int tab_offset_x = 0;
        for (auto it = card_list.begin(); it != card_list.end(); ++it) {
            auto card = *it;
            if (card->HasInputEvent()) {
                InputBox::Tab tab;
                tab.card = card;
                tab.name = card->name();

                tab.x = x_ + TAB_SIDE_MARGIN + tab_offset_x - 9;
                tab.y = y_ - 3;
                tab.height = font_height_ + TAB_TOP_MARGIN * 2 + 3 * 2;

                auto name = unicode::ToTString(tab.name);
                tab.width = GetDrawStringWidthToHandle(name.c_str(), name.size(),
                        font_handle_) + 18;

                tab_offset_x += tab.width + TAB_X_MARGIN;

                tabs_.push_back(tab);
            }
        }
    }
}

void InputBox::ProcessInputTabs(InputManager* input)
{
    if (input->GetMouseLeftCount() == 1) {
        int tab_index = 0;

        for (auto it = tabs_.begin(); it != tabs_.end(); ++it) {
            auto tab = *it;

            bool tab_hover = (tab.x <= input->GetMouseX()
                    && input->GetMouseX() <= tab.x + tab.width
                    && tab.y <= input->GetMouseY()
                    && input->GetMouseY() <= tab.y + tab.height);

            if (tab_hover) {
                SwitchTab(tab_index);
            }

            tab_index++;
        }

        {
            bool tab_hover = (script_tab_.x <= input->GetMouseX()
                    && input->GetMouseX() <= script_tab_.x + script_tab_.width
                    && script_tab_.y <= input->GetMouseY()
                    && input->GetMouseY() <= script_tab_.y + script_tab_.height);

            if (tab_hover) {
                SetScriptMode();
            }
        }
    }
}

void InputBox::UpdateTabs()
{
    // スクリプトモードショートカット
    // if (input->GetKeyCount(InputManager::KEYBIND_SCRIPT_MODE) == 1) {
    //    SetScriptMode();
    // }

    // タブ選択
    int tab_offset_x = 0;
        for (auto it = tabs_.begin(); it != tabs_.end(); ++it) {
            auto tab = *it;
        tab.x = x_ + TAB_SIDE_MARGIN + tab_offset_x - 9;
        tab.y = y_ - 3;
        tab.height = font_height_ + TAB_TOP_MARGIN * 2 + 3 * 2;

        auto name = unicode::ToTString(tab.name);
        tab.width = GetDrawStringWidthToHandle(name.c_str(), name.size(),
                font_handle_) + 18;

        tab_offset_x += tab.width - 3;
    }

    script_tab_.x = x_ + width_ - TAB_SIDE_MARGIN - 16 - 9;
    script_tab_.y = y_ - 3;
    script_tab_.width = 16 + 18;
    script_tab_.height = font_height_ + TAB_TOP_MARGIN * 2 + 3 * 2;
}


void InputBox::UpdateBase(InputManager* input)
{
    int screen_width, screen_height;
    GetScreenState(&screen_width, &screen_height, nullptr);

    bool hover = (x_ <= input->GetMouseX() && input->GetMouseX() <= x_ + width_
            && y_ <= input->GetMouseY() && input->GetMouseY() <= y_ + height_);

    bool input_hover = (input_x_ <= input->GetMouseX()
            && input->GetMouseX() <= input_x_ + input_width_
            && input_y_ <= input->GetMouseY()
            && input->GetMouseY() <= input_y_ + input_height_);

    bool corner_hover = (x_ + width_ - 18 <= input->GetMouseX()
            && input->GetMouseX() <= x_ + width_
            && y_ + height_ - 18 <= input->GetMouseY()
            && input->GetMouseY() <= y_ + height_);

    // ドラッグ処理
    if (input->GetMouseLeft()) {
        if (input->GetPrevMouseLeft() == 0 && drag_offset_x_ < 0 && hover
                && !input_hover && !corner_hover) {
            drag_offset_x_ = input->GetMouseX() - x_;
            drag_offset_y_ = input->GetMouseY() - y_;
        }
        if (input->GetPrevMouseLeft() == 0 && drag_resize_offset_x_ < 0
                && corner_hover) {
            drag_resize_offset_x_ = x_ + width_ - input->GetMouseX();
            drag_resize_offset_y_ = y_ + height_ - input->GetMouseY();
        }
        if (hover && !IsActive()) {
            Activate();
        } else if (!hover && IsActive()
                && (drag_offset_x_ < 0 && drag_resize_offset_x_ < 0)) {
            Inactivate();
        }
    } else {
        drag_offset_x_ = -1;
        drag_offset_y_ = -1;
        drag_resize_offset_x_ = -1;
        drag_resize_offset_y_ = -1;
    }

    if (drag_offset_x_ >= 0) {
        x_ = input->GetMouseX() - drag_offset_x_;
        y_ = input->GetMouseY() - drag_offset_y_;
        x_ = max(0, x_); x_ = min(screen_width - width_, x_);
        y_ = max(0, y_); y_ = min(screen_height - height_, y_);
        input->CancelMouseLeft();
    } else if (drag_resize_offset_x_ >= 0) {
        int new_width = input->GetMouseX() - x_ + drag_resize_offset_x_;
        new_width = max(new_width, (BOX_MIN_WIDTH + 0));
        new_width = min(new_width, screen_width - x_);
        width_ = new_width;
        input->CancelMouseLeft();
    }
}


bool InputBox::IsActive()
{
    return GetActiveKeyInput() == input_handle_;
}

void InputBox::Activate()
{
    SetActiveKeyInput(input_handle_);
    ResetCursorCount();
}

void InputBox::Inactivate()
{
    ReStartKeyInput(input_handle_);
    SetActiveKeyInput(-1);
}
