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
                multiline_(true),
                font_height_(ResourceManager::default_font_size()),
                drag_offset_x_(-1),
                drag_offset_y_(-1),
                drag_resize_offset_x_(-1),
                drag_resize_offset_y_(-1),
                min_input_height_(font_height_ + INPUT_MARGIN_Y * 2),
                selecting_tab_index_(0),
                manager_accessor_(manager_accessor),
                card_(std::make_shared<Card>(manager_accessor_, "", "immo", "", "",
                            std::vector<std::string>())),
				input_(manager_accessor->config_manager().lock())

{
	absolute_rect_ = Rect(100, 100, 800, 100);

    font_handle_ = ResourceManager::default_font_handle();
    bg_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("system/images/gui/gui_inputbox_bg.png"), 2, 2, 24, 24);
    tab_bg_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("system/images/gui/gui_inputbox_tab_bg.png"), 2, 2, 12, 12);
    tab_bg_inactive_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("system/images/gui/gui_inputbox_tab_inactive_bg.png"), 2, 2, 12, 12);
    ime_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("system/images/gui/gui_inputbox_ime_bg.png"), 2, 2, 12, 12);

    script_icon_image_handle_ = ResourceManager::LoadCachedGraph(_T("system/images/gui/gui_inputbox_tab_script_icon.png"));

    int screen_width, screen_height;
    GetScreenState(&screen_width, &screen_height, nullptr);
    absolute_rect_.width = std::min(DEFAULT_MAX_WIDTH + 0, (int)(screen_width * 0.4));
    absolute_rect_.height = input_.height() + BOX_TOP_MARGIN + BOX_BOTTOM_MARGIN;
    absolute_rect_.x = (screen_width - absolute_rect_.width) / 2;
    absolute_rect_.y = screen_height - absolute_rect_.height - BOX_BOTTOM_MARGIN;

    selecting_tab_index_ = 0;

    Activate();
    if (auto card_manager = manager_accessor_->card_manager().lock()) {
        card_manager->AddCard(card_);
    }

    input_.set_on_enter([this](const std::string& text) -> bool{

        if (input_.text().empty()) {
            input_.set_active(false);
        } else {
            if (selecting_tab_index_ == -1) {
                card_->Execute(unicode::ToString(input_.text()), "mmo",
                        [&](const v8::Handle<v8::Value>& result, const std::string& error){
                    if (error.size() > 0) {
                        script_tab_.message = unicode::ToTString(error);
                    } else if (!result.IsEmpty()) {
                        script_tab_.message = _T("=> ") +
                            unicode::ToTString(std::string(*String::Utf8Value(result->ToString())));
                    } else {
                        script_tab_.message = _T("");
                    }
                    input_.set_message(script_tab_.message);
                });
            } else if (!tabs_.empty()) {
                if (auto card =
                        tabs_.at(selecting_tab_index_).card.lock()) {
                    card->onEnter(unicode::ToString(input_.text()));
                }
            }
        }

        return true;
    });
	input_.Init();
}

InputBox::~InputBox()
{

}

void InputBox::DrawTabs()
{

    {
        int tab_index = 0;
        SetDrawArea(absolute_rect_.x, script_tab_.y, script_tab_.x, script_tab_.y + script_tab_.height);
        BOOST_FOREACH(auto& tab, tabs_) {

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
            DrawStringToHandle(tab_x + 9, absolute_rect_.y + TAB_TOP_MARGIN + 1, unicode::ToTString(tab.name).c_str(),
                    GetColor(255, 255, 255), font_handle_);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

            DrawStringToHandle(tab_x + 9, absolute_rect_.y + TAB_TOP_MARGIN, unicode::ToTString(tab.name).c_str(),
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

        DrawGraph(absolute_rect_.x + absolute_rect_.width - TAB_SIDE_MARGIN - 16, absolute_rect_.y + TAB_TOP_MARGIN,
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

    DrawGraph(absolute_rect_.x, absolute_rect_.y, *bg_image_handle_[0], TRUE);
    DrawGraph(absolute_rect_.x + absolute_rect_.width - 24, absolute_rect_.y, *bg_image_handle_[1], TRUE);
    DrawGraph(absolute_rect_.x, absolute_rect_.y + absolute_rect_.height - 24, *bg_image_handle_[2], TRUE);
    DrawGraph(absolute_rect_.x + absolute_rect_.width - 24, absolute_rect_.y + absolute_rect_.height - 24, *bg_image_handle_[3], TRUE);

    DrawRectExtendGraphF(absolute_rect_.x + 24, absolute_rect_.y,
                         absolute_rect_.x + absolute_rect_.width - 24, absolute_rect_.y + 24,
                         0, 0, 1, 24, *bg_image_handle_[1], TRUE);

    DrawRectExtendGraphF(absolute_rect_.x + 24, absolute_rect_.y + absolute_rect_.height - 24,
                         absolute_rect_.x + absolute_rect_.width - 24, absolute_rect_.y + absolute_rect_.height,
                         0, 0, 1, 24, *bg_image_handle_[3], TRUE);

    DrawRectExtendGraphF(absolute_rect_.x, absolute_rect_.y + 24,
                         absolute_rect_.x + 24, absolute_rect_.y + absolute_rect_.height - 24,
                         0, 0, 24, 1, *bg_image_handle_[2], TRUE);

    DrawRectExtendGraphF(absolute_rect_.x + absolute_rect_.width - 24, absolute_rect_.y + 24,
                         absolute_rect_.x + absolute_rect_.width, absolute_rect_.y + absolute_rect_.height - 24,
                         0, 0, 24, 1, *bg_image_handle_[3], TRUE);

    DrawRectExtendGraphF(absolute_rect_.x +  24, absolute_rect_.y + 24,
                         absolute_rect_.x + absolute_rect_.width - 24, absolute_rect_.y + absolute_rect_.height - 24,
                         0, 0, 1, 1, *bg_image_handle_[3], TRUE);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void InputBox::Draw()
{
    DrawBase();
    DrawTabs();
    input_.Draw();
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
    input_.CancelSelect();
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
        input_.set_message(script_tab_.message);
        input_.set_reverse_color(true);
    } else if (!tabs_.empty()) {
        SetInputString(tabs_.at(selecting_tab_index_).text);
        input_.set_message(tabs_.at(selecting_tab_index_).message);
        input_.set_reverse_color(false);
    }
}

tstring InputBox::GetInputString() const {
    return input_.text();
}

void InputBox::SetInputString(const tstring& text)
{
    input_.set_text(text);
}

void InputBox::ProcessInput(InputManager* input)
{
    if (!input) {
        return;
    }

    bool push_mouse_left = (input->GetMouseLeftCount() > 0);

    bool first_key_shift = (input->GetKeyCount(KEY_INPUT_RSHIFT) == 1
            || input->GetKeyCount(KEY_INPUT_LSHIFT) == 1);
	
    bool push_key_esc = input->GetKeyCount(KEY_INPUT_ESCAPE) > 0;

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

    bool empty = input_.text().empty();
	input_.ProcessInput(input);

	//  IME変換途中で入力欄が非アクティブになってしまうため修正
	//  if (IsActive() && ((first_key_return && !push_key_shift && empty) || push_key_esc)) {
	if (IsActive() && input_.lines_[0].empty() && ((first_key_return && !push_key_shift && empty) || push_key_esc)) {
		Inactivate();
    } else if (!IsActive() && first_key_return) {
        Activate();
    } else if (!IsActive() && push_key_v && push_key_ctrl) {
        Activate();
    }
	
    ProcessInputTabs(input);
    UpdateBase(input);
    UpdateTabs();


    int new_height = input_.height() + BOX_TOP_MARGIN + BOX_BOTTOM_MARGIN;

    int screen_height;
    GetScreenState(nullptr, &screen_height, nullptr);
    if (absolute_rect_.y + absolute_rect_.height > screen_height - BOX_TOP_MARGIN) {
        absolute_rect_.y -= new_height - absolute_rect_.height;
        // input_.set_y(y_ + BOX_TOP_MARGIN);
    }

    absolute_rect_.height = new_height;
        
    input_.set_y(absolute_rect_.y + BOX_TOP_MARGIN);
    input_.set_x(absolute_rect_.x + INPUT_MARGIN_X);
    input_.set_width(absolute_rect_.width - INPUT_MARGIN_X * 2);

        //line_buffer.clear();
        //line_width = 0;

        //if (push_key_shift && (push_repeat_key_up || push_repeat_key_down)) {
        //    SetKeyInputSelectArea(GetKeyInputCursorPosition(input_handle_),
        //            prev_cursor_pos_, input_handle_);
        //}

    //}

    if (IsActive()) {
        input->CancelKeyCountAll();
    }
}

void InputBox::Update()
{
    input_.Update();
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
                tab.name = unicode::ToTString(card->name());

                tab.x = absolute_rect_.x + TAB_SIDE_MARGIN + tab_offset_x - 9;
                tab.y = absolute_rect_.y - 3;
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
    BOOST_FOREACH(auto& tab, tabs_) {
        tab.x = absolute_rect_.x + TAB_SIDE_MARGIN + tab_offset_x - 9;
        tab.y = absolute_rect_.y - 3;
        tab.height = font_height_ + TAB_TOP_MARGIN * 2 + 3 * 2;

        auto name = unicode::ToTString(tab.name);
        tab.width = GetDrawStringWidthToHandle(name.c_str(), name.size(),
                font_handle_) + 18;

        tab_offset_x += tab.width - 3;
    }

    script_tab_.x = absolute_rect_.x + absolute_rect_.width - TAB_SIDE_MARGIN - 16 - 9;
    script_tab_.y = absolute_rect_.y - 3;
    script_tab_.width = 16 + 18;
    script_tab_.height = font_height_ + TAB_TOP_MARGIN * 2 + 3 * 2;
}


void InputBox::UpdateBase(InputManager* input)
{
    int screen_width, screen_height;
    GetScreenState(&screen_width, &screen_height, nullptr);

    bool hover = (absolute_rect_.x <= input->GetMouseX() && input->GetMouseX() <= absolute_rect_.x + absolute_rect_.width
            && absolute_rect_.y <= input->GetMouseY() && input->GetMouseY() <= absolute_rect_.y + absolute_rect_.height);

    //bool input_hover = (input_x_ <= input->GetMouseX()
    //        && input->GetMouseX() <= input_x_ + input_width_
    //        && input_y_ <= input->GetMouseY()
    //        && input->GetMouseY() <= input_y_ + input_height_);

    bool corner_hover = (absolute_rect_.x + absolute_rect_.width - 18 <= input->GetMouseX()
            && input->GetMouseX() <= absolute_rect_.x + absolute_rect_.width
            && absolute_rect_.y + absolute_rect_.height - 18 <= input->GetMouseY()
            && input->GetMouseY() <= absolute_rect_.y + absolute_rect_.height);

    // ドラッグ処理
    if (input->GetMouseLeft()) {
        if (input->GetPrevMouseLeft() == 0 && drag_offset_x_ < 0 && hover
                 && !corner_hover) {
            drag_offset_x_ = input->GetMouseX() - absolute_rect_.x;
            drag_offset_y_ = input->GetMouseY() - absolute_rect_.y;
        }
        if (input->GetPrevMouseLeft() == 0 && drag_resize_offset_x_ < 0
                && corner_hover) {
            drag_resize_offset_x_ = absolute_rect_.x + absolute_rect_.width - input->GetMouseX();
            drag_resize_offset_y_ = absolute_rect_.y + absolute_rect_.height - input->GetMouseY();
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
        absolute_rect_.x = input->GetMouseX() - drag_offset_x_;
        absolute_rect_.y = input->GetMouseY() - drag_offset_y_;
        absolute_rect_.x = std::max(0, absolute_rect_.x); absolute_rect_.x = std::min(screen_width - absolute_rect_.width, absolute_rect_.x);
        absolute_rect_.y = std::max(0, absolute_rect_.y); absolute_rect_.y = std::min(screen_height - absolute_rect_.height, absolute_rect_.y);
        input->CancelMouseLeft();
    } else if (drag_resize_offset_x_ >= 0) {
        int new_width = input->GetMouseX() - absolute_rect_.x + drag_resize_offset_x_;
        new_width = std::max(new_width, (BOX_MIN_WIDTH + 0));
        new_width = std::min(new_width, screen_width - absolute_rect_.x);
        absolute_rect_.width = new_width;
        input->CancelMouseLeft();
    }
}


bool InputBox::IsActive()
{
    return input_.active();
}

void InputBox::Activate()
{
    input_.set_active(true);
}

void InputBox::Inactivate()
{
    input_.set_active(false);
}