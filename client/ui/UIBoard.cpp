//
// UIBoard.cpp
//

/**
* @module global
* @submodule UI
 */

/**
 * @class Board
 * @namespace UI
 * @extends UI.Base
 */

#include "UIBoard.hpp"
#include "../ScriptEnvironment.hpp"
#include "../../common/Logger.hpp"

const int UIBoard::BASE_BLOCK_SIZE = 24;

UIBoard::UIBoard() :
                max_width_(800),
                min_width_(100),
                max_height_(800),
                min_height_(100),
                drag_offset_rect_(-1, -1, -1, -1),
                drag_resize_offset_rect_(-1, -1, -1, -1)
{
    base_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("system/images/gui/gui_board_bg.png"), 2, 2, 24, 24);
}

Handle<Value> UIBoard::Property_max_width(Local<String> property, const AccessorInfo &info)
{
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UIBoard>(
            *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);
    return Integer::New(self->max_width_);
}

void UIBoard::Property_set_max_width(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UIBoard>(
            *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);
    self->max_width_ = value->ToInteger()->IntegerValue();
}

Handle<Value> UIBoard::Property_min_width(Local<String> property, const AccessorInfo &info)
{
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UIBoard>(
            *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);
    return Integer::New(self->min_width_);
}

void UIBoard::Property_set_min_width(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UIBoard>(
            *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);
    self->min_width_ = value->ToInteger()->IntegerValue();
}

Handle<Value> UIBoard::Property_max_height(Local<String> property, const AccessorInfo &info)
{
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UIBoard>(
            *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);
    return Integer::New(self->max_height_);
}

void UIBoard::Property_set_max_height(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UIBoard>(
            *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);
    self->max_height_ = value->ToInteger()->IntegerValue();
}

Handle<Value> UIBoard::Property_min_height(Local<String> property, const AccessorInfo &info)
{
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UIBoard>(
            *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);
    return Integer::New(self->min_height_);
}

void UIBoard::Property_set_min_height(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UIBoard>(
            *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);
    self->min_height_ = value->ToInteger()->IntegerValue();
}

void UIBoard::DefineInstanceTemplate(Handle<ObjectTemplate>* object)
{
    UIBase::DefineInstanceTemplate(object);

    /**
     * 幅の最大値(px)
     *
     * @property max_width
     * @type Integer
     * @default 800
     */
    SetProperty(object, "max_width", Property_max_width, Property_set_max_width);

    /**
     * 幅の最小(px)
     *
     * @property min_width
     * @type Integer
     * @default 100
     */
    SetProperty(object, "min_width", Property_min_width, Property_set_min_width);

    /**
     * 高さの最大値(px)
     *
     * @property max_height
     * @type Integer
     * @default 800
     */
    SetProperty(object, "max_height", Property_max_height, Property_set_max_height);

    /**
     * 高さの最小(px)
     *
     * @property min_height
     * @type Integer
     * @default 100
     */
    SetProperty(object, "min_height", Property_min_height, Property_set_min_height);

}

void UIBoard::UpdateDrag(InputManager* input, bool resizeable)
{
    int screen_width, screen_height;
    GetScreenState(&screen_width, &screen_height, nullptr);

    bool hover = (absolute_x()<= input->GetMouseX() && input->GetMouseX() <= absolute_x()+ absolute_width()
            && absolute_y() <= input->GetMouseY() && input->GetMouseY() <= absolute_y() + absolute_height());

    bool corner_hover = (absolute_x() + absolute_width() - 18 <= input->GetMouseX()
            && input->GetMouseX() <= absolute_x() + absolute_width()
            && absolute_y() + absolute_height() - 18 <= input->GetMouseY()
            && input->GetMouseY() <= absolute_y() + absolute_height());

    // アクティブ
    if (hover && input->GetMouseLeftCount() == 1) {
        Focus();
    }

    //Logger::Log("%d, %d, %d", hover, input->GetMouseLeftCount(), drag_resize_offset_rect_.x);

    // ドラッグ処理
    if (input->GetMouseLeft()) {
        if (input->GetMouseLeftCount() == 1) {
            if (drag_offset_rect_.x < 0 && hover
                    && !corner_hover) {
                //drag_offset_rect_.x = input->GetMouseX() - x_;
                //drag_offset_rect_.y = input->GetMouseY() - y_;
                drag_offset_rect_.x = input->GetMouseX() - offset_rect_.x;
                drag_offset_rect_.y = input->GetMouseY() - offset_rect_.y;
            }
            if (drag_resize_offset_rect_.x < 0
                    && corner_hover) {
                //drag_resize_offset_rect_.x = x_ + width_ - input->GetMouseX();
                //drag_resize_offset_rect_.y = y_ + height_ - input->GetMouseY();
                drag_resize_offset_rect_.x = input->GetMouseX() - offset_rect_.width;
                drag_resize_offset_rect_.y = input->GetMouseY() - offset_rect_.height;
            }
        }
    } else {
        drag_offset_rect_.x = -1;
        drag_offset_rect_.y = -1;
        drag_resize_offset_rect_.x = -1;
        drag_resize_offset_rect_.y = -1;
    }

    if (drag_offset_rect_.x >= 0) {
        offset_rect_.x= input->GetMouseX() - drag_offset_rect_.x;
        offset_rect_.y= input->GetMouseY() - drag_offset_rect_.y;
        input->CancelMouseLeft();
    } else if (drag_resize_offset_rect_.x >= 0) {
        offset_rect_.width= input->GetMouseX() -  drag_resize_offset_rect_.x;
        offset_rect_.height= input->GetMouseY() - drag_resize_offset_rect_.y;
        input->CancelMouseLeft();

        if (width_ + offset_rect_.width < min_width_) {
            offset_rect_.width = min_width_ - width_;
        } else if (width_ + offset_rect_.width > max_width_) {
            offset_rect_.width = max_width_ - width_;
        }

        if (height_ + offset_rect_.height < min_height_) {
            offset_rect_.height = min_height_ - height_;
        } if (height_ + offset_rect_.height > max_height_) {
            offset_rect_.height = max_height_ - height_;
        }
    }
}

void UIBoard::ProcessInput(InputManager* input)
{
    if (!visible_) {
        return;
    }

    ProcessInputChildren(input);
    UpdateDrag(input, resizable_);
}

void UIBoard::Update()
{
    if (!visible_) {
        return;
    }

    UpdatePosition();
    UpdateChildren();
}

void UIBoard::Draw()
{
    if (!visible_) {
        return;
    }

    SetDrawBlendMode(DX_BLENDMODE_ADD, 255);

    int x = absolute_x();
    int y = absolute_y();
    int width = absolute_width();
    int height = absolute_height();

    DrawGraph(x, y, *base_image_handle_[0], TRUE);
    DrawGraph(x + width - BASE_BLOCK_SIZE, y, *base_image_handle_[1], TRUE);
    DrawGraph(x, y + height - BASE_BLOCK_SIZE, *base_image_handle_[2], TRUE);
    DrawGraph(x + width - BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE, *base_image_handle_[3], TRUE);

    DrawRectExtendGraphF(x + BASE_BLOCK_SIZE, y,
                         x + width - BASE_BLOCK_SIZE, y + BASE_BLOCK_SIZE,
                         0, 0, 1, BASE_BLOCK_SIZE, *base_image_handle_[1], TRUE);

    DrawRectExtendGraphF(x + BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE,
                         x + width - BASE_BLOCK_SIZE, y + height,
                         0, 0, 1, BASE_BLOCK_SIZE, *base_image_handle_[3], TRUE);

    DrawRectExtendGraphF(x, y + BASE_BLOCK_SIZE,
                         x + BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE,
                         0, 0, BASE_BLOCK_SIZE, 1, *base_image_handle_[2], TRUE);

    DrawRectExtendGraphF(x + width - BASE_BLOCK_SIZE, y + BASE_BLOCK_SIZE,
                         x + width, y + height - BASE_BLOCK_SIZE,
                         0, 0, BASE_BLOCK_SIZE, 1, *base_image_handle_[3], TRUE);

    DrawRectExtendGraphF(x + BASE_BLOCK_SIZE, y + BASE_BLOCK_SIZE,
                         x + width - BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE,
                         0, 0, 1, 1, *base_image_handle_[3], TRUE);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    DrawChildren();
}

bool UIBoard::resizable() const
{
    return resizable_;
}

void UIBoard::set_resizable(bool resizable)
{
    resizable_ = resizable;
}

