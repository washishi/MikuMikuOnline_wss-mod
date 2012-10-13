//
// UILabel.cpp
//

/**
* @module global
* @submodule UI
 */

/**
 * @class Label
 * @namespace UI
 * @extends UI.Base
 */

#include "UILabel.hpp"
#include "../ScriptEnvironment.hpp"
#include "../ResourceManager.hpp"
#include "../../common/Logger.hpp"
#include <numeric>

const int UILabel::BASE_BLOCK_SIZE = 12;

UILabel::UILabel() :
                font_handle_(ResourceManager::default_font_handle())
{
}

UILabel::~UILabel()
{
}

void UILabel::set_text(const tstring& text)
{
    text_ = text;

    // 文字幅を計算
    char_width_list_.clear();
    for (auto it = text_.begin(); it != text_.end(); ++it) {
        #ifdef UNICODE
            TCHAR c = *it;
            int width = GetDrawStringWidthToHandle(&c, 1, font_handle_);
        #else
            unsigned char c = *it;
            TCHAR string[2] = {0, 0};

            int width;
            if (((c>=0x81 && c<=0x9f) || (c>=0xe0 && c<=0xfc)) && (it + 1) != text_.end()) {
                string[0] = c;
                string[1] = *(it + 1);
                ++it;
                width = GetDrawStringWidthToHandle(string, 2, font_handle_);
            } else {
                string[0] = c;
                width = GetDrawStringWidthToHandle(string, 1, font_handle_);
            }
        #endif
            char_width_list_.push_back(width);
    }
}

tstring UILabel::text() const
{
    return unicode::ToTString(text_);
}

void UILabel::set_textcolor(const Color& color)
{
    textcolor_ = color;
}

UIBase::Color UILabel::textcolor() const
{
    return textcolor_;
}

void UILabel::set_bgcolor(const Color& color)
{
    bgcolor_ = color;
}

UIBase::Color UILabel::bgcolor() const
{
    return bgcolor_;
}

 Handle<Value> UILabel::Property_text(Local<String> property, const AccessorInfo &info)
 {
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UILabel>(
            *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);
    return String::New(unicode::ToString(self->text()).c_str());
 }

 void UILabel::Property_set_text(Local<String> property, Local<Value> value, const AccessorInfo& info)
 {
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UILabel>(
            *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);

    self->set_text(unicode::ToTString(*String::Utf8Value(value->ToString())));
 }

 Handle<Value> UILabel::Property_bgcolor(Local<String> property, const AccessorInfo &info)
 {
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UILabel>(
            *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);
    return String::New(self->bgcolor().ToString().c_str());
 }

 void UILabel::Property_set_bgcolor(Local<String> property, Local<Value> value, const AccessorInfo& info)
 {
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UILabel>(
            *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);

    self->set_bgcolor(UIBase::Color::FromString(*String::Utf8Value(value->ToString())));
 }

 Handle<Value> UILabel::Property_color(Local<String> property, const AccessorInfo &info)
 {
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UILabel>(
            *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);
    return String::New(self->textcolor().ToString().c_str());
 }

 void UILabel::Property_set_color(Local<String> property, Local<Value> value, const AccessorInfo& info)
 {
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UILabel>(
            *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);

    self->set_textcolor(UIBase::Color::FromString(*String::Utf8Value(value->ToString())));
 }


void UILabel::DefineInstanceTemplate(Handle<ObjectTemplate>* object)
{
    UIBase::DefineInstanceTemplate(object);

    Handle<ObjectTemplate>& instance_template = *object;

    /**
     * 表示テキスト
     *
     * @property text
     * @type String
     */
    SetProperty(&instance_template, "text", Property_text, Property_set_text);

    /**
     * 背景色
     *
     * '#FFFFFFF' (RGB) または　'#FFFFFFFF' (RGBA) のフォーマットの文字列で指定します
     *
     * @property bgcolor
     * @type String
     */
    SetProperty(&instance_template, "bgcolor" , Property_bgcolor, Property_set_bgcolor);

    /**
     * 文字色
     *
     * @property color
     * @type String
     */
    SetProperty(&instance_template, "color", Property_color, Property_set_color);
}

void UILabel::ProcessInput(InputManager* input)
{
	bool hover = (absolute_x()<= input->GetMouseX() && input->GetMouseX() <= absolute_x()+ absolute_width()
            && absolute_y() <= input->GetMouseY() && input->GetMouseY() <= absolute_y() + absolute_height());

	if (input->GetMouseLeftCount() == 1 && hover) {  
		if (!on_click_.IsEmpty() && on_click_->IsFunction()) {
			on_click_.As<Function>()->CallAsFunction(Context::GetCurrent()->Global(), 0, nullptr);
		}else if(!on_click_function_._Empty()) {
			on_click_function_(this);
			input->CancelMouseLeft();
		}
	}else if(hover && !hover_flag_){
		if(!on_hover_function_._Empty()) {
			on_hover_function_(this);
			hover_flag_ = true;
		}
	}else if(!hover && hover_flag_){
		if(!on_out_function_._Empty()) {
			on_out_function_(this);
			hover_flag_ = false;
		}
	}
}

void UILabel::UpdatePosition()
{
    int parent_x,
        parent_y,
        parent_width,
        parent_height;

    // 親のサイズを取得
    if (parent_.IsEmpty()) {
        parent_x = 0;
        parent_y = 0;
        GetScreenState(&parent_width, &parent_height, nullptr);
    } else {
        UIBasePtr parent_ptr = *static_cast<UIBasePtr*>(parent_->GetPointerFromInternalField(0));
        parent_x = parent_ptr->absolute_x();
        parent_y = parent_ptr->absolute_y();
        parent_width = parent_ptr->absolute_width();
        parent_height = parent_ptr->absolute_height();
    }

    // 幅を計算
    if((docking_ & DOCKING_LEFT) &&
            (docking_ & DOCKING_RIGHT)) {
        int left = parent_x + left_;
        int right = parent_x + parent_width - right_;
        absolute_rect_.width = right - left;
    } else {
		if (width_ > 0) { 
			absolute_rect_.width = width_;
		} else {
			absolute_rect_.width = std::accumulate(char_width_list_.begin(), char_width_list_.end(), 0);
		}
    }

    // 行の折り返しを計算
    int line_width = 0;
    //int line_width_max = 0;
    int line_num = 1;
    substr_list_.clear();
    substr_list_.push_back(0);
    auto text_cursor = 0;

    for (auto it = char_width_list_.begin(); it != char_width_list_.end(); ++it) {
        if ( text_[text_cursor] == _T('\n')) {
            line_num++;
            substr_list_.push_back(text_cursor);
            substr_list_.push_back(text_cursor + 1);
        } else if (line_width + *it > absolute_width()) {
            line_width = *it;
            line_num++;
            substr_list_.push_back(text_cursor);
            substr_list_.push_back(text_cursor);
        } else {
            line_width += *it;
        }

        text_cursor++;
    }

//    if (!stable_width) {
//        line_width_max = max(line_width_max, line_width);
//        absolute_rect_.width = line_width_max;
//    }

    substr_list_.push_back(text_.length());

    // 高さを計算
    if((docking_ & DOCKING_TOP) &&
            (docking_ & DOCKING_BOTTOM)) {
        int top = parent_y + top_;
        int bottom = parent_y + parent_height - bottom_;
        absolute_rect_.height = bottom - top;
    } else {
        absolute_rect_.height = (ResourceManager::default_font_size() + 2) * line_num;
    }

    // 左上X座標を計算
    // TODO: 正しく計算されないバグあり
//    if(docking_ & DOCKING_HCENTER) {
//        absolute_rect_.x = parent_x + parent_width / 2 - offset_rect_.width / 2;
//    } else if(docking_ & DOCKING_RIGHT) {
//        absolute_rect_.x = parent_x + parent_width - right_ - offset_rect_.width;
//    } else {
//        absolute_rect_.x = parent_x + left_;
//    }
    absolute_rect_.x = parent_x + left_;

    // 左上Y座標を計算
    if(docking_ & DOCKING_VCENTER) {
        absolute_rect_.y = parent_y + parent_height / 2 - absolute_rect_.height / 2;
    } else if(docking_ & DOCKING_BOTTOM) {
        absolute_rect_.y = parent_y + parent_height - bottom_ - absolute_rect_.height;
    } else {
        absolute_rect_.y = parent_y + top_;
    }
}

void UILabel::Update()
{
    UpdatePosition();
}

void UILabel::Draw()
{
    if (!visible_) {
        return;
    }

    int color =   GetColor(textcolor_.r, textcolor_.g ,textcolor_.b);
    int bgcolor = GetColor(bgcolor_.r, bgcolor_.g ,bgcolor_.b);

    if (bgcolor_.a > 0) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, bgcolor_.a);
        DrawBox(absolute_x(), absolute_y(),
                absolute_x() + absolute_width(), absolute_y() + absolute_height(), bgcolor, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    int line_y = 1;
    for (auto it = substr_list_.begin(); it != substr_list_.end(); ++it) {
        int begin = *it;
        int end = *(++it);
        DrawStringToHandle(absolute_x() , absolute_y() + line_y,
            unicode::ToTString(text_.substr(begin, end - begin)).c_str(), color, font_handle_ );
        line_y += ResourceManager::default_font_size() + 2;
    }

}

