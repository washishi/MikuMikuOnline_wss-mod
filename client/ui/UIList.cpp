//
// UIList.cpp
//

/**
* @module global
* @submodule UI
 */

/**
 * @class List
 * @namespace UI
 * @extends UI.Base
 */

#include "UIList.hpp"
#include "../ScriptEnvironment.hpp"

namespace {
static int BASE_BLOCK_SIZE = 12;
static int SCROLLBAR_MIN_HEIGHT = 24;
}

UIList::UIList() :
                scroll_y_(0),
                max_scroll_y_(0),
                scroll_y_target_(-1),
                drag_offset_y_(-1)
{
    scrollbar_base_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("resources/images/gui/gui_button_base.png"), 2, 2, 12, 12);
}

UIList::~UIList()
{
}

Handle<Value> UIList::Function_addItem(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UIList>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);

    if (args.Length() > 0 && args[0]->IsObject()) {
        auto child = args[0]->ToObject();

        if (args.This() != child) {
            UIBasePtr child_ptr = *static_cast<UIBasePtr*>(child->GetPointerFromInternalField(0));
            child_ptr->set_parent(args.This());
            self->items_.push_back(Persistent<Object>::New(child));
        }
    }
    return args.This();
}

Handle<Value> UIList::Function_removeItem(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UIList>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);

    if (args.Length() > 0 && args[0]->IsObject()) {
        auto child = args[0]->ToObject();
        auto it = std::find(self->items_.begin(), self->items_.end(), child);
        if (it != self->items_.end()) {
            UIBasePtr chid_ptr = *static_cast<UIBasePtr*>((*it)->GetPointerFromInternalField(0));
            chid_ptr->set_parent(Handle<Object>());
            self->items_.erase(it);
        }
    }
    return args.This();
}

Handle<Value> UIList::Property_scroll_y(Local<String> property, const AccessorInfo &info)
{
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UIList>(
            *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);

    return Integer::New(self->scroll_y_);
}

void UIList::Property_set_scroll_y(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UIList>(
            *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);

    self->scroll_y_target_ = value->ToInteger()->IntegerValue();
}

void UIList::DefineInstanceTemplate(Handle<ObjectTemplate>* object)
{
    UIBase::DefineInstanceTemplate(object);

    /**
     * アイテムを追加します
     *
     * @method addItem
     * @param {UIObject} item
     * @return {UIObject}　自分自身
     * @chainable
     */
    SetFunction(object, "addItem", Function_addItem);


    /**
     * アイテムを削除します
     *
     * @method removeItem
     * @param {UIObject} item
     * @return {UIObject}　自分自身
     * @chainable
     */
    SetFunction(object, "removeItem", Function_removeItem);

    /**
     * スクロール量(px)
     *
     * @property scroll_y
     * @type Integer
     */
    SetProperty(object, "scroll_y", Property_scroll_y, Property_set_scroll_y);
}

void UIList::ProcessInput(InputManager* input)
{
    for (auto it = items_.begin(); it != items_.end(); ++it) {
        auto item = *it;
        UIBasePtr item_ptr = *static_cast<UIBasePtr*>(item->GetPointerFromInternalField(0));
        item_ptr->ProcessInput(input);
    }

    UpdateScrollBar(input);
}

void UIList::UpdateScrollBar(InputManager* input)
{
    int screen_width, screen_height;
    GetScreenState(&screen_width, &screen_height, nullptr);

    bool hover = (absolute_x()<= input->GetMouseX() && input->GetMouseX() <= absolute_x()+ absolute_width()
            && absolute_y() <= input->GetMouseY() && input->GetMouseY() <= absolute_y() + absolute_height());

    bool hover_scrollbar = (absolute_width() + absolute_x() - 22 <= input->GetMouseX()
            && input->GetMouseX() <= absolute_x() + absolute_width() - 6
            && absolute_y() + scrollbar_y_ <= input->GetMouseY()
            && input->GetMouseY()
                    <= absolute_y() + scrollbar_y_ + scrollbar_height_);

    // アクティブ
    if (hover_scrollbar && input->GetMouseLeftCount() == 1) {
        Focus();
    }

    // ドラッグ処理
    if (input->GetMouseLeft()) {
        if (input->GetPrevMouseLeft() == 0 && drag_offset_y_ < 0 && hover_scrollbar) {
            drag_offset_y_ = input->GetMouseY() - (absolute_y() + scrollbar_y_);
        }
    } else {
        drag_offset_y_ = -1;
    }

    if (drag_offset_y_ >= 0) {
        scroll_y_ = 1.0 * (input->GetMouseY() - absolute_y() - drag_offset_y_)
                / (absolute_height() - scrollbar_height_) * max_scroll_y_;
        input->CancelMouseLeft();

    } else if (hover) {
        int wheel = input->GetMouseWheel();
        if (wheel != 0) {
            scroll_y_ -= wheel * 20;
            input->CancelMouseWheel();
        }
    }

    NormalizeScrollY();
}

void UIList::Update()
{
    // TODO: 子供に対してabsolute_系とは別のサイズを与える必要あり
    UpdatePosition();

    int offset_y = -scroll_y_;
    item_start_ = -1;
    item_end_ = -1;

    int content_height = 0;
    int index = 0;
    for (auto it = items_.begin(); it != items_.end(); ++it) {
        auto item = *it;
        UIBasePtr item_ptr = *static_cast<UIBasePtr*>(item->GetPointerFromInternalField(0));
        item_ptr->set_offset_y(offset_y - BASE_BLOCK_SIZE);
        // item_ptr->set_offset_width(-18);
        item_ptr->Update();
        offset_y += item_ptr->absolute_height();
        content_height += item_ptr->absolute_height();
        if (item_start_ < 0 && offset_y > 0) {
            item_start_ = index;
        } else if (offset_y > absolute_height() && item_end_ < 0) {
            item_end_ = index;
        }
        index++;
    }
    if (item_end_ < 0) {
        item_end_ = items_.size() - 1;
    }

    max_scroll_y_ = max(content_height - absolute_height(), 0);
    if (content_height > absolute_height()) {
        scrollbar_height_ = absolute_height() * absolute_height() / content_height;
        scrollbar_height_ = max(scrollbar_height_, 0 + SCROLLBAR_MIN_HEIGHT);
        scrollbar_y_ = (absolute_height() - scrollbar_height_) * (1.0 * scroll_y_ / max_scroll_y_);
    } else {
        scrollbar_height_ = scrollbar_y_ = 0;
    }

    if (scroll_y_target_ >= 0) {
        scroll_y_ = scroll_y_target_;
        scroll_y_target_ = -1;
        NormalizeScrollY();
    }

}

void UIList::Draw()
{
    if (!visible_) {
        return;
    }

    SetDrawArea(absolute_x(), absolute_y(),
            absolute_x() + absolute_width(), absolute_y() + absolute_height());
    if (item_start_ >= 0) {
        for (int i = item_start_; i <= item_end_; i++) {
            UIBasePtr item_ptr = *static_cast<UIBasePtr*>(items_[i]->GetPointerFromInternalField(0));
            item_ptr->Draw();
        }
    }
    SetDrawAreaFull();

    if (scrollbar_height_ > 0) {
        int x = absolute_width() + absolute_x() - 22;
        int y = absolute_y() + scrollbar_y_;
        int width = 16;
        int height = scrollbar_height_;

        DrawGraph(x, y, *scrollbar_base_image_handle_[0], TRUE);
        DrawGraph(x + width - BASE_BLOCK_SIZE, y, *scrollbar_base_image_handle_[1], TRUE);
        DrawGraph(x, y + height - BASE_BLOCK_SIZE, *scrollbar_base_image_handle_[2], TRUE);
        DrawGraph(x + width - BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE, *scrollbar_base_image_handle_[3], TRUE);

        DrawRectExtendGraphF(x + BASE_BLOCK_SIZE, y,
                             x + width - BASE_BLOCK_SIZE, y + BASE_BLOCK_SIZE,
                             0, 0, 1, BASE_BLOCK_SIZE, *scrollbar_base_image_handle_[1], TRUE);

        DrawRectExtendGraphF(x + BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE,
                             x + width - BASE_BLOCK_SIZE, y + height,
                             0, 0, 1, BASE_BLOCK_SIZE, *scrollbar_base_image_handle_[3], TRUE);

        DrawRectExtendGraphF(x, y + BASE_BLOCK_SIZE,
                             x + BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE,
                             0, 0, BASE_BLOCK_SIZE, 1, *scrollbar_base_image_handle_[2], TRUE);

        DrawRectExtendGraphF(x + width - BASE_BLOCK_SIZE, y + BASE_BLOCK_SIZE,
                             x + width, y + height - BASE_BLOCK_SIZE,
                             0, 0, BASE_BLOCK_SIZE, 1, *scrollbar_base_image_handle_[3], TRUE);

        DrawRectExtendGraphF(x + BASE_BLOCK_SIZE, y + BASE_BLOCK_SIZE,
                             x + width - BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE,
                             0, 0, 1, 1, *scrollbar_base_image_handle_[3], TRUE);
    }
}

void UIList::NormalizeScrollY()
{
    scroll_y_ = min(max(scroll_y_, 0), max_scroll_y_);
}
