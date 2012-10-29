//
// Input.cpp
//

#include "Input.hpp"
#include <DxLib.h>
#include <stdint.h>

const size_t Input::TEXT_BUFFER_SIZE = 1024;
const size_t Input::HISTORY_MAX_SIZE = 50;
const int Input::KEY_REPEAT_FRAME = 8;

const int Input::INPUT_MARGIN_X = 8;
const int Input::INPUT_MARGIN_Y = 6;

const int Input::IME_BLOCK_SIZE = 32;
const int Input::IME_MARGIN_BASE = 12;
const int Input::IME_MARGIN_Y = 16;
const int Input::IME_MAX_PAGE_SIZE = 6;
const int Input::IME_MIN_WIDTH = 120;

Input::Input(ConfigManagerPtr config_manager) :
	multiline_(true),
    reverse_color_(false),
	drag_flag_(false),
	rightmenu_show_(false),
	config_manager_(config_manager)
{
    input_bg_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("system/images/gui/gui_inputbox_input_bg.png"), 2, 2, 12, 12);

    ime_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("system/images/gui/gui_inputbox_ime_bg.png"), 2, 2, 12, 12);

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

	click_cancel_count_ = 0;

}

void Input::Init()
{
	auto shd_right_click_list_ = std::make_shared<UIList>(right_click_list_);
	right_click_list_.addItem(UIBasePtr(new UILabel([&]()->UILabel{
		UILabel label;
		label.set_parent_c(shd_right_click_list_);
		label.set_input_adaptor(this);
		label.set_text(unicode::ToTString(unicode::sjis2utf8("切り取り")));
		label.set_width(120);
		label.set_top(12);
		label.set_left(0);
		label.set_textcolor(UISuper::Color(0,0,0,255));
		label.set_bgcolor(UISuper::Color(255,255,255,180));
		label.set_on_click_function_([](UIBase* ptr)->void{
			auto input_ = ptr->input_adpator();
			auto sel_txt = input_->selecting_text();
			SetClipboardText(sel_txt.c_str());
			auto text = input_->text();
			auto pos = text.find(sel_txt);
			tstring res;
			if( pos != std::string::npos )
			{
				res = text.substr(0,pos);
				res += text.substr(pos + sel_txt.size(),text.size() - pos + sel_txt.size());
			}
			input_->set_text(res);
			input_->set_click_chancel_count(8);
		});
		label.set_on_hover_function_([](UIBase* ptr)->void{
			auto label_ptr = (UILabel *)ptr;
			label_ptr->set_bgcolor(UISuper::Color(0,0,0,180));
			label_ptr->set_textcolor(UISuper::Color(255,255,255));
		});
		label.set_on_out_function_([](UIBase* ptr)->void{
			auto label_ptr = (UILabel *)ptr;
			label_ptr->set_bgcolor(UISuper::Color(255,255,255,180));
			label_ptr->set_textcolor(UISuper::Color(0,0,0));
		});
		return label;
	}())));
	right_click_list_.addItem(UIBasePtr(new UILabel([&]()->UILabel{
		UILabel label;
		label.set_parent_c(shd_right_click_list_);
		label.set_input_adaptor(this);
		label.set_text(unicode::ToTString(unicode::sjis2utf8("コピー")));
		label.set_width(120);
		label.set_top(12);
		label.set_left(0);
		label.set_textcolor(UISuper::Color(0,0,0,255));
		label.set_bgcolor(UISuper::Color(255,255,255,180));
		label.set_on_click_function_([&](UIBase* ptr)->void{
			auto input_ = ptr->input_adpator();
			SetClipboardText(input_->selecting_text().c_str());
			input_->set_click_chancel_count(8);
		});
		label.set_on_hover_function_([](UIBase* ptr)->void{
			auto label_ptr = (UILabel *)ptr;
			label_ptr->set_bgcolor(UISuper::Color(0,0,0,180));
			label_ptr->set_textcolor(UISuper::Color(255,255,255));
		});
		label.set_on_out_function_([](UIBase* ptr)->void{
			auto label_ptr = (UILabel *)ptr;
			label_ptr->set_bgcolor(UISuper::Color(255,255,255,180));
			label_ptr->set_textcolor(UISuper::Color(0,0,0));
		});
		return label;
	}())));
	right_click_list_.addItem(UIBasePtr(new UILabel([&]()->UILabel{
		UILabel label;
		label.set_parent_c(shd_right_click_list_);
		label.set_input_adaptor(this);
		label.set_text(unicode::ToTString(unicode::sjis2utf8("貼り付け")));
		label.set_width(120);
		label.set_top(12);
		label.set_left(0);
		label.set_textcolor(UISuper::Color(0,0,0,255));
		label.set_bgcolor(UISuper::Color(255,255,255,180));
		label.set_on_click_function_([&](UIBase* ptr)->void{
			auto input_ = ptr->input_adpator();
			auto size = GetClipboardText(NULL);
			if(size > 0){
				TCHAR *buf = new TCHAR[size];
				GetClipboardText(buf);
				input_->paste_text(buf);
				delete []buf;
			}
			input_->set_click_chancel_count(8);
		});
		label.set_on_hover_function_([](UIBase* ptr)->void{
			auto label_ptr = (UILabel *)ptr;
			label_ptr->set_bgcolor(UISuper::Color(0,0,0,180));
			label_ptr->set_textcolor(UISuper::Color(255,255,255));
		});
		label.set_on_out_function_([](UIBase* ptr)->void{
			auto label_ptr = (UILabel *)ptr;
			label_ptr->set_bgcolor(UISuper::Color(255,255,255,180));
			label_ptr->set_textcolor(UISuper::Color(0,0,0));
		});
		return label;
	}())));
	right_click_list_.set_height((font_height_ + 2)* 3);
	right_click_list_.set_width(120);
	right_click_list_.set_visible(false);
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

		int select_start = 0,select_end = 0;
		GetKeyInputSelectArea(&select_start,&select_end,input_handle_);
		if( select_start > select_end )std::swap(select_start,select_end);

		if( select_start > -1 && select_end != select_start ) {
			if ( multiline_ ) {
				BOOST_FOREACH(auto it,lines_){
					int width = 0;
					TCHAR c[2] = {0};
					if( select_start >= it.size() && select_start != -1 ) {
						DrawStringToHandle(internal_x, internal_y + current_line * font_height_,
							it.c_str(), text_color, font_handle_);
						if(select_start == it.size()){
							select_start -= it.size();
						}else{
							select_start -= it.size() + 1;
						}
						select_end -= it.size() + 1;
						++current_line;
					}else if(select_start != -1){
						for(int i = 0;i < select_start;++i){
							c[0] = it[i];
							DrawStringToHandle(internal_x + width, internal_y + current_line * font_height_,
								c, text_color, font_handle_);
							width += GetDrawStringWidthToHandle(c,1,font_handle_);
						}
						for(int i = select_start;i < ((select_end > static_cast<int>(it.size())) ? it.size() : select_end); ++i){
							c[0] = it[i];
							SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
							DrawBox(internal_x + width,internal_y + current_line * font_height_,
								internal_x + width + GetDrawStringWidthToHandle(c,1,font_handle_),internal_y + ( current_line + 1 ) * font_height_,text_color,1);
							DrawStringToHandle(internal_x + width, internal_y + current_line * font_height_,
								c, !reverse_color_ ? GetColor(255, 255, 255) : GetColor(0, 0, 0), font_handle_);
							width += GetDrawStringWidthToHandle(c,1,font_handle_);
							SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
						}
						for(int i = ((select_end > static_cast<int>(it.size())) ? it.size() : select_end);i < it.size(); ++i){
							c[0] = it[i];
							DrawStringToHandle(internal_x + width, internal_y + current_line * font_height_,
								c, text_color, font_handle_);
							width += GetDrawStringWidthToHandle(c,1,font_handle_);
						}
						if(select_end > it.size()){
							select_end -= it.size() + 1;
							select_start = 0;
						}else{
							select_start = -1;
						}
						++current_line;
					}else if(select_start == -1){
						DrawStringToHandle(internal_x + width, internal_y + current_line * font_height_,
							it.c_str(), text_color, font_handle_);
						++current_line;
					}
				}
			}else{
				BOOST_FOREACH(auto it,lines_){
					int width = 0;
					TCHAR c[2] = {0};
					for(int i = 0;i < select_start;++i){
						c[0] = it[i];
						DrawStringToHandle(internal_x + width, internal_y + current_line * font_height_,
							c, text_color, font_handle_);
						width += GetDrawStringWidthToHandle(c,1,font_handle_);
					}
					for(int i = select_start;i < select_end; ++i){
						c[0] = it[i];
				        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
						DrawBox(internal_x + width,internal_y + current_line * font_height_,
							internal_x + width + GetDrawStringWidthToHandle(c,1,font_handle_),internal_y + ( current_line + 1 ) * font_height_,text_color,1);
						DrawStringToHandle(internal_x + width, internal_y + current_line * font_height_,
							c, !reverse_color_ ? GetColor(255, 255, 255) : GetColor(0, 0, 0), font_handle_);
						width += GetDrawStringWidthToHandle(c,1,font_handle_);
						SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
					}
					for(unsigned int i = select_end;i < it.size(); ++i){
						c[0] = it[i];
						DrawStringToHandle(internal_x + width, internal_y + current_line * font_height_,
							c, text_color, font_handle_);
						width += GetDrawStringWidthToHandle(c,1,font_handle_);
					}
				}
			}
		}else{
			for (auto it = lines_.begin(); it != lines_.end(); ++it) {
				auto line = *it;
				DrawStringToHandle(internal_x, internal_y + current_line * font_height_,
					line.c_str(), text_color, font_handle_);
				current_line++;
			}
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
	{
		if ( right_click_list_.visible() ) {
			right_click_list_.Draw();
		}
	}
}

void Input::Update()
{
    blink_count_ = (blink_count_ + 1) % 60;
	{
		if ( right_click_list_.visible() ) {
			right_click_list_.Update();
		}
	}
}

void Input::ProcessInput(InputManager* input)
{
    if (!input) {
        return;
    }

	if (click_cancel_count_ > 0)
	{
		input->CancelMouseLeft();
		input->CancelMouseRight();
		--click_cancel_count_;
	}

    bool push_mouse_left = (input->GetMouseLeftCount() > 0);
	bool prev_mouse_left = input->GetPrevMouseLeft();

	bool push_mouse_right = (input->GetMouseRightCount() > 0);
	bool prev_mouse_right = input->GetPrevMouseRight();

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

	if ( right_click_list_.visible() ) {
		right_click_list_.ProcessInput(input);
		if( push_mouse_left ) {
			//rightmenu_show_ = false;
			right_click_list_.set_visible(false);
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
		if(input->GetMouseX() >= x() && input->GetMouseX() <= x() + width() &&
		input->GetMouseY() >= y() && input->GetMouseY() <= y() + height()){
		if ( !right_click_list_.visible() &&
			!( right_click_list_.visible() && right_click_list_.absolute_x()<= input->GetMouseX() && input->GetMouseX() <= right_click_list_.absolute_x()+ right_click_list_.absolute_width()
			&& right_click_list_.absolute_y() <= input->GetMouseY() && input->GetMouseY() <= right_click_list_.absolute_y() + right_click_list_.absolute_height())) {
				// マウス左ボタンが押された時
				if (push_mouse_left && !prev_mouse_left) {
					auto mpos = input->GetMousePos();
					auto offset_x = mpos.first - (x_ + INPUT_MARGIN_X);
					auto offset_y = mpos.second - (y_ + INPUT_MARGIN_Y);
					// カレット変更
					if( multiline_ ) {
						auto line_num = offset_y / font_height_;
						//if( ( offset_y % font_height_ ) != 0 )++line_num;
						int tmp = 0,cnt = 0;
						if( line_num < (int)lines_.size() && line_num >= 0 ){
							for(int i = 0;i < line_num; ++i){
								cnt += lines_[i].size();
							}
							for(unsigned int i = 0;i < lines_[line_num].size(); ++i){
								auto tmp_x = GetDrawStringWidthToHandle(&lines_[line_num][i],1,font_handle_);
								if( tmp + tmp_x < offset_x ){
									tmp += tmp_x;
									++cnt;
								}
							}
							SetKeyInputCursorPosition(line_num + cnt,input_handle_);
						}
					}else{
						int tmp = 0,cnt = 0;
						for(unsigned int i = 0;i < lines_[0].size(); ++i){
							auto tmp_x = GetDrawStringWidthToHandle(&lines_[0][i],1,font_handle_);
							if( tmp + tmp_x < offset_x ){
								tmp += tmp_x;
								++cnt;
							}
						}
						if( selecting_coursorpoint_.first = selecting_coursorpoint_.second ) {
							SetKeyInputSelectArea( -1, -1, input_handle_ );
						}else{
							SetKeyInputSelectArea(selecting_coursorpoint_.first,selecting_coursorpoint_.second,input_handle_);
						}
						SetKeyInputCursorPosition(cnt,input_handle_);
					}
				}
				// マウス左ボタンがドラッグされた時
				if (push_mouse_left && prev_mouse_left ) {
					int prev_cursor_pos = 0;
					if( !drag_flag_ ){
						prev_cursor_pos = GetKeyInputCursorPosition(input_handle_);
					}else{
						prev_cursor_pos = selecting_coursorpoint_.first;
					}
					auto mpos = input->GetMousePos();
					auto offset_x = mpos.first - (x_ + INPUT_MARGIN_X);
					auto offset_y = mpos.second - (y_ + INPUT_MARGIN_Y);
					// カレット変更
					if( multiline_ ) {
						auto line_num = offset_y / font_height_;
						int tmp = 0,cnt = 0;
						if( line_num < (int)lines_.size() && line_num >= 0){
							for(int i = 0;i < line_num; ++i,++cnt){
								cnt += lines_[i].size();
							}
							for(unsigned int i = 0;i < lines_[line_num].size(); ++i){
								auto tmp_x = GetDrawStringWidthToHandle(&lines_[line_num][i],1,font_handle_);
								if( tmp + tmp_x < offset_x ){
									tmp += tmp_x;
									++cnt;
								}
							}
						}
						selecting_coursorpoint_ = std::make_pair<int,int>(prev_cursor_pos,cnt);
					}else{
						int tmp = 0,cnt = 0;
						for(unsigned int i = 0;i < lines_[0].size(); ++i){
							auto tmp_x = GetDrawStringWidthToHandle(&lines_[0][i],1,font_handle_);
							if( tmp + tmp_x < offset_x ){
								tmp += tmp_x;
								++cnt;
							}
						}
						selecting_coursorpoint_ = std::make_pair<int,int>(prev_cursor_pos,cnt);
					}
					SetKeyInputSelectArea(selecting_coursorpoint_.first,selecting_coursorpoint_.second,input_handle_);
					SetKeyInputCursorPosition(selecting_coursorpoint_.second,input_handle_);
					drag_flag_ = true;
					input->CancelMouseLeft();
				}
				// マウス左ボタンが離され、且つ前回ドラッグされていた時
				if (!push_mouse_left && prev_mouse_left && drag_flag_ ) {
					drag_flag_ = false;
					if( selecting_coursorpoint_.first == selecting_coursorpoint_.second ) {
						SetKeyInputSelectArea( -1, -1, input_handle_ );
					}else{
						if(selecting_coursorpoint_.first > selecting_coursorpoint_.second)
						{
							std::swap(selecting_coursorpoint_.first , selecting_coursorpoint_.second);
						}
						SetKeyInputSelectArea(selecting_coursorpoint_.first,selecting_coursorpoint_.second,input_handle_);
					}
					SetKeyInputCursorPosition(selecting_coursorpoint_.second,input_handle_);
				}
		}else{
			if( push_mouse_left ){
				auto mpos = input->GetMousePos();
				auto offset_x = mpos.first - (x_ + INPUT_MARGIN_X);
				auto offset_y = mpos.second - (y_ + INPUT_MARGIN_Y);
				// カレット変更
				if( multiline_ ) {
					auto line_num = offset_y / font_height_;
					//if( ( offset_y % font_height_ ) != 0 )++line_num;
					int tmp = 0,cnt = 0;
					if( line_num < (int)lines_.size() && line_num >= 0 ){
						for(int i = 0;i < line_num; ++i){
							cnt += lines_[i].size();
						}
						for(unsigned int i = 0;i < lines_[line_num].size(); ++i){
							auto tmp_x = GetDrawStringWidthToHandle(&lines_[line_num][i],1,font_handle_);
							if( tmp + tmp_x < offset_x ){
								tmp += tmp_x;
								++cnt;
							}
						}
						SetKeyInputCursorPosition(line_num + cnt,input_handle_);
					}
					input->CancelMouseLeft();
				}else{
					int tmp = 0,cnt = 0;
					for(unsigned int i = 0;i < lines_[0].size(); ++i){
						auto tmp_x = GetDrawStringWidthToHandle(&lines_[0][i],1,font_handle_);
						if( tmp + tmp_x < offset_x ){
							tmp += tmp_x;
							++cnt;
						}
					}
					if( selecting_coursorpoint_.first = selecting_coursorpoint_.second ) {
						SetKeyInputSelectArea( -1, -1, input_handle_ );
					}else{
						SetKeyInputSelectArea(selecting_coursorpoint_.first,selecting_coursorpoint_.second,input_handle_);
					}
					SetKeyInputCursorPosition(cnt,input_handle_);
				}
			}
		}
		// マウス右ボタンが押されたとき
		if ( push_mouse_right && !prev_mouse_right ) {
			if( x() <= input->GetMouseX() && input->GetMouseX() <= x() + width() && 
				y() <= input->GetMouseY() && input->GetMouseY() <= y() + height()){
					auto mouse_pos_ = input->GetMousePos();
					if( mouse_pos_.second + right_click_list_.absolute_height() > config_manager_->screen_height()){
						right_click_list_.set_top(mouse_pos_.second - right_click_list_.absolute_height());
						if ( mouse_pos_.first + right_click_list_.absolute_width() > config_manager_->screen_width()){
							right_click_list_.set_left(mouse_pos_.first - right_click_list_.absolute_width());
							BOOST_FOREACH(auto it,right_click_list_.getItems()){
								it->set_left(mouse_pos_.first - right_click_list_.absolute_width());
								it->set_top(mouse_pos_.second - right_click_list_.absolute_height() + 12);
							}
						}else{
							right_click_list_.set_left(mouse_pos_.first);
							BOOST_FOREACH(auto it,right_click_list_.getItems()){
								it->set_left(mouse_pos_.first);
								it->set_top(mouse_pos_.second - right_click_list_.absolute_height() + 12);
							}
						}
					}else{
						right_click_list_.set_top(mouse_pos_.second);
						if ( mouse_pos_.first + right_click_list_.absolute_width() > config_manager_->screen_width()){
							right_click_list_.set_left(mouse_pos_.first - right_click_list_.absolute_width());
							BOOST_FOREACH(auto it,right_click_list_.getItems()){
								it->set_left(mouse_pos_.first - right_click_list_.absolute_width());
								it->set_top(mouse_pos_.second + 12);
							}
						}else{
							right_click_list_.set_left(mouse_pos_.first);
							BOOST_FOREACH(auto it,right_click_list_.getItems()){
								it->set_left(mouse_pos_.first);
								it->set_top(mouse_pos_.second + 12);
							}
						}
					}
					right_click_list_.set_visible(true);
					//rightmenu_show_ = true;
			}
		}
		}

        // カーソル位置（文字単位）を取得
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
                SetKeyInputCursorPosition(char_count - 1, input_handle_);
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
		SetUseIMEFlag(flag);
        ReStartKeyInput(input_handle_);
        SetActiveKeyInput(-1);
    } else if (!active() && flag) {
		SetUseIMEFlag(flag);
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

tstring Input::selecting_text() const
{
		int select_start = 0,select_end = 0;
		GetKeyInputSelectArea(&select_start,&select_end,input_handle_);
		if( select_start > select_end )std::swap(select_start,select_end);
		tstring selecting_text;

		if( select_start > -1 && select_end != select_start ) {
			if ( multiline_ ) {
				BOOST_FOREACH(auto it,lines_){
					TCHAR c[2] = {0};
					if( select_start > it.size() && select_start != -1 ) {
						select_start -= it.size();
						select_end -= it.size();
					}else{
						for(int i = select_start;i < select_end; ++i){
							c[0] = it[i];
						    selecting_text += c;
						}
						select_start = -1;
					}
				}
			}else{
				BOOST_FOREACH(auto it,lines_){
					TCHAR c[2] = {0};
					for(int i = select_start;i < select_end; ++i){
						c[0] = it[i];
				        selecting_text += c;
					}
				}
			}
		}
		return selecting_text;
}

void Input::paste_text(tstring text)
{
	auto pos = GetKeyInputCursorPosition(input_handle_);
	TCHAR String[TEXT_BUFFER_SIZE];
    GetKeyInputString(String, input_handle_);
    tstring dat(String, _tcslen(String));
	dat.insert(pos,text);
	SetKeyInputString(dat.c_str(), input_handle_);
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

void Input::set_click_chancel_count(int count)
{
	click_cancel_count_ = count;
}