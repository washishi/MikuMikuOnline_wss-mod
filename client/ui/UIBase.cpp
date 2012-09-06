//
// UIBase.cpp
//

/**
* @module global
* @submodule UI
 */

/**
 * @class Base
 * @namespace UI
 */

#include "UIBase.hpp"
#include "../InputManager.hpp"
#include "../ScriptEnvironment.hpp"
#include "../../common/unicode.hpp"
#include <DxLib.h>
#include <algorithm>

int UIBase::max_focus_index = 0;

int Hex2Int(const std::string& hex)
{
    int dec = 0;
    int base = 1;
    for (auto it = hex.rbegin(); it != hex.rend(); ++it) {
        auto c = *it;
        int d = 0;
        if ('A' <= c && c <= 'F') {
            d = (c - 'A') + 10;
        } else
        if ('a' <= c && c <= 'f'){
            d = (c - 'a') + 10;
        } else
        if ('0' <= c && c <= '9'){
            d = (c - '0');
        }
        dec += d * base;
        base *= 16;
    }
    return dec;
}

std::string UIBase::Color::ToString() const
{
    std::stringstream stream;
    stream << "#";
    stream << std::hex << std::setw(2) << std::setfill('0') << r;
    stream << std::hex << std::setw(2) << std::setfill('0') << g;
    stream << std::hex << std::setw(2) << std::setfill('0') << b;
    stream << std::hex << std::setw(2) << std::setfill('0') << a;
    return stream.str();
}

UIBase::Color UIBase::Color::FromString(const std::string & str)
{
    Color color;
    if (str.length() >= 7) {
        color.r = (unsigned char)Hex2Int(str.substr(1, 2));
        color.b = (unsigned char)Hex2Int(str.substr(3, 2));
        color.g = (unsigned char)Hex2Int(str.substr(5, 2));
        if (str.length() >= 9) {
            color.a = (unsigned char)Hex2Int(str.substr(7, 2));
        } else {
            color.a = 255;
        }
    }
    return color;
}

UIBase::UIBase() :
                width_(100),
                height_(100),
                top_(12),
                left_(12),
                bottom_(12),
                right_(12),
                docking_(DOCKING_TOP | DOCKING_LEFT),
                visible_(true),
                focus_index_(0)
{

}

UIBase::~UIBase()
{
    parent_.Dispose();
    for (auto it = children_.begin(); it != children_.end(); ++it) {
        auto child = *it;
        child.Dispose();
    }
}

void UIBase::ProcessInput(InputManager* input)
{

}


void UIBase::AsyncUpdate()
{
    AsyncUpdateChildren();
}

int UIBase::absolute_x() const
{
    return absolute_rect_.x + offset_rect_.x;
}

int UIBase::absolute_y() const
{
    return absolute_rect_.y + offset_rect_.y;
}
int UIBase::absolute_width() const
{
    return absolute_rect_.width + offset_rect_.width;
}

int UIBase::absolute_height() const
{
    return absolute_rect_.height + offset_rect_.height;
}

void UIBase::UpdatePosition()
{
    int parent_x_,
        parent_y_,
        parent_width_,
        parent_height_;

    // 親のサイズを取得
    if (parent_.IsEmpty()) {
        parent_x_ = 0;
        parent_y_ = 0;
        GetScreenState(&parent_width_, &parent_height_, nullptr);
    } else {
        UIBasePtr parent_ptr = *static_cast<UIBasePtr*>(parent_->GetPointerFromInternalField(0));
        parent_x_ = parent_ptr->absolute_x();
        parent_y_ = parent_ptr->absolute_y();
        parent_width_ = parent_ptr->absolute_width();
        parent_height_ = parent_ptr->absolute_height();
    }

    // 幅を計算
    if ((docking_ & DOCKING_LEFT) && (docking_ & DOCKING_RIGHT)) {
        int left = parent_x_ + left_;
        int right = parent_x_ + parent_width_ - right_;
        absolute_rect_.width = right - left;
    } else {
        absolute_rect_.width = width_;
    }

    // 高さを計算
    if ((docking_ & DOCKING_TOP) && (docking_ & DOCKING_BOTTOM)) {
        int top = parent_y_ + top_;
        int bottom = parent_y_ + parent_height_ - bottom_;
        absolute_rect_.height = bottom - top;
    } else {
        absolute_rect_.height = height_;
    }

    // 左上X座標を計算
    if (docking_ & DOCKING_HCENTER) {
        absolute_rect_.x = parent_x_ + parent_width_ / 2 - absolute_rect_.width / 2;
    } else if (docking_ & DOCKING_RIGHT) {
        absolute_rect_.x = parent_x_ + parent_width_ - right_ - absolute_rect_.width;
    } else {
        absolute_rect_.x = parent_x_ + left_;
    }

    // 左上Y座標を計算
    if (docking_ & DOCKING_VCENTER) {
        absolute_rect_.y = parent_y_ + parent_height_ / 2 - absolute_rect_.height / 2;
    } else if (docking_ & DOCKING_BOTTOM) {
        absolute_rect_.y = parent_y_ + parent_height_ - bottom_ - absolute_rect_.height;
    } else {
        absolute_rect_.y = parent_y_ + top_;
    }
}

Handle<Value> UIBase::Function_addChild(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0));
    assert(self);

    if (args.Length() > 0 && args[0]->IsObject()) {
        auto child = args[0]->ToObject();

        if (args.This() != child) {
            UIBasePtr child_ptr = *static_cast<UIBasePtr*>(child->GetPointerFromInternalField(0));
            child_ptr->set_parent(args.This());
            self->children_.push_back(Persistent<Object>::New(child));
        }
    }
    return args.This();
}

Handle<Value> UIBase::Function_removeChild(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0));
    assert(self);

    if (args.Length() > 0 && args[0]->IsObject()) {
        auto child = args[0]->ToObject();
        auto it = std::find(self->children_.begin(), self->children_.end(), child);
        if (it != self->children_.end()) {
            UIBasePtr child_ptr = *static_cast<UIBasePtr*>((*it)->GetPointerFromInternalField(0));
            child_ptr->set_parent(Handle<Object>());
            self->children_.erase(it);
        }
    }
    return args.This();
}

Handle<Value> UIBase::Function_parent(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0));
    assert(self);

    if (!self->parent_.IsEmpty()) {
        return self->parent_;
    } else {
        return Undefined();
    }
}

Handle<Value> UIBase::Property_visible(Local<String> property, const AccessorInfo &info)
{
    assert(info.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0));
    return Boolean::New(self->visible_);
}

void UIBase::Property_set_visible(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0));
    self->visible_ = value->ToBoolean()->BooleanValue();
}

Handle<Value> UIBase::Property_width(Local<String> property, const AccessorInfo &info)
{
    assert(info.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0));
    return Integer::New(self->width_);
}
void UIBase::Property_set_width(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0));
    self->width_ = value->ToInteger()->IntegerValue();
}
Handle<Value> UIBase::Property_height(Local<String> property, const AccessorInfo &info)
{
    assert(info.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0));
    return Integer::New(self->height_);
}
void UIBase::Property_set_height(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0));
    self->height_ = value->ToInteger()->IntegerValue();
}

Handle<Value> UIBase::Property_top(Local<String> property, const AccessorInfo &info)
{
    assert(info.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0));
    return Integer::New(self->top_);
}

void UIBase::Property_set_top(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0));
    self->top_ = value->ToInteger()->IntegerValue();
}

Handle<Value> UIBase::Property_left(Local<String> property, const AccessorInfo &info)
{
    assert(info.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0));
    return Integer::New(self->left_);
}

void UIBase::Property_set_left(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0));
    self->left_ = value->ToInteger()->IntegerValue();
}

Handle<Value> UIBase::Property_right(Local<String> property, const AccessorInfo &info)
{

    assert(info.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0));
    return Integer::New(self->right_);
}

void UIBase::Property_set_right(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0));
    self->right_ = value->ToInteger()->IntegerValue();
}

Handle<Value> UIBase::Property_bottom(Local<String> property, const AccessorInfo &info)
{
    assert(info.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0));
    return Integer::New(self->bottom_);
}

void UIBase::Property_set_bottom(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0));
    self->bottom_ = value->ToInteger()->IntegerValue();
}

Handle<Value> UIBase::Property_docking(Local<String> property, const AccessorInfo &info)
{
    assert(info.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0));
    return Integer::New(self->docking_);
}

void UIBase::Property_set_docking(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0));
    self->docking_ = value->ToInteger()->IntegerValue();
}

Handle<Value> UIBase::Property_on_click(Local<String> property, const AccessorInfo &info)
{
    assert(info.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0));
	return self->on_click_;
}

void UIBase::Property_set_on_click(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.This()->InternalFieldCount() > 0);
    UIBasePtr self = *static_cast<UIBasePtr*>(info.This()->GetPointerFromInternalField(0));
	if (value->IsFunction()) {
		self->on_click_ = Persistent<Function>::New(value.As<Function>());
	}
}

void UIBase::DefineInstanceTemplate(Handle<ObjectTemplate>* object)
{
    Handle<ObjectTemplate>& instance_template = *object;

    /**
     * UIオブジェクトに子供を追加します
     *
     * @method addChild
     * @param {UIObject} child
     * @return {UIObject}　自分自身
     * @chainable
     */
    SetFunction(&instance_template, "addChild", Function_addChild);


    /**
     * UIオブジェクトから子供を削除します
     *
     * @method removeChild
     * @param {UIObject} child
     * @return {UIObject}　自分自身
     * @chainable
     */
    SetFunction(&instance_template, "removeChild", Function_removeChild);


    /**
     * 親オブジェクトを返します
     *
     * @method parent
     * @return {UIObject|Undefined} 親オブジェクト
     */
    SetFunction(&instance_template, "parent", Function_parent);

    /**
     * UIObjectの幅(px)
     *
     * @property width
     * @type Integer
     */
    SetProperty(&instance_template, "width", Property_width, Property_set_width);

    /**
     * UIObjectの高さ(px)
     *
     * @property height
     * @type Integer
     */
    SetProperty(&instance_template, "height", Property_height, Property_set_height);

    /**
     * UIObjectの上余白(px)
     *
     * @property top
     * @type Integer
     * @default 12
     */
    SetProperty(&instance_template, "top", Property_top, Property_set_top);

    /**
     * UIObjectの左余白(px)
     *
     * @property left
     * @type Integer
     * @default 12
     */
    SetProperty(&instance_template, "left", Property_left, Property_set_left);

    /**
     * UIObjectの下余白(px)
     *
     * @property bottom
     * @type Integer
     * @default 12
     */
    SetProperty(&instance_template, "bottom", Property_bottom, Property_set_bottom);

    /**
     * UIObjectの右余白(px)
     *
     * @property right
     * @type Integer
     * @default 12
     */
    SetProperty(&instance_template, "right", Property_right, Property_set_right);

    /**
     * UIObjectがどの方向に対して固定余白を持つか指定します
     *
     * @property docking
     * @type Integer
     * @default UI.DOCKING_TOP | UI.DOCKING_LEFT
     */
    SetProperty(&instance_template, "docking", Property_docking, Property_set_docking);

    /**
     * UIObjectの可視状態
     *
     * @property visible
     * @type Boolean
     * @default true
     */
    SetProperty(&instance_template, "visible", Property_visible, Property_set_visible);
	
    SetProperty(&instance_template, "onclick", Property_on_click, Property_set_on_click);

}

void UIBase::ProcessInputChildren(InputManager* input)
{
    for (auto it = children_.begin(); it != children_.end(); ++it) {
        auto child = *it;
        UIBasePtr child_ptr = *static_cast<UIBasePtr*>(child->GetPointerFromInternalField(0));
        child_ptr->ProcessInput(input);
    }
}

void UIBase::UpdateChildren()
{
    for (auto it = children_.begin(); it != children_.end(); ++it) {
        auto child = *it;
        UIBasePtr child_ptr = *static_cast<UIBasePtr*>(child->GetPointerFromInternalField(0));
        child_ptr->Update();
    }
}

void UIBase::DrawChildren()
{
    for (auto it = children_.begin(); it != children_.end(); ++it) {
        auto child = *it;
        UIBasePtr child_ptr = *static_cast<UIBasePtr*>(child->GetPointerFromInternalField(0));
        child_ptr->Draw();
    }
}

void UIBase::AsyncUpdateChildren()
{
    for (auto it = children_.begin(); it != children_.end(); ++it) {
        auto child = *it;
        UIBasePtr child_ptr = *static_cast<UIBasePtr*>(child->GetPointerFromInternalField(0));
        child_ptr->AsyncUpdate();
    }
}

void UIBase::UpdateBaseImage()
{

}

void UIBase::GetParam(const Handle<Object>& object, const std::string& name, int* value)
{
    Handle<String> key = String::New(name.c_str());
    if (object->Has(key)) {
        *value = object->Get(key)->ToInteger()->Value();
    }
}

void UIBase::GetParam(const Handle<Object>& object, const std::string& name, bool* value)
{
    Handle<String> key = String::New(name.c_str());
    if (object->Has(key)) {
        *value = object->Get(key)->ToBoolean()->Value();
    }
}

void UIBase::GetParam(const Handle<Object>& object, const std::string& name,
        std::string* value)
{
    Handle<String> key = String::New(name.c_str());
    if (object->Has(key)) {
        *value = std::string(
                *v8::String::Utf8Value(object->Get(key)->ToString()));
    }
}

void UIBase::Focus()
{
    if (!parent_.IsEmpty()) {
        UIBasePtr parent_ptr = *static_cast<UIBasePtr*>(parent_->GetPointerFromInternalField(0));
        parent_ptr->Focus();
    }
    focus_index_ = ++max_focus_index;
}

int UIBase::height() const { return height_; }
void UIBase::set_height(int height) { height_ = height; }
int UIBase::width() const { return width_; }
void UIBase::set_width(int width) { width_ = width; }

int UIBase::top() const { return top_; }
void UIBase::set_top(int top) { top_ = top; }
int UIBase::left() const { return left_; }
void UIBase::set_left(int left) { left_ = left; }
int UIBase::right() const { return right_; }
void UIBase::set_right(int right) { right_ = right; }
int UIBase::bottom() const { return bottom_; }
void UIBase::set_bottom(int bottom) { bottom_ = bottom; }

int UIBase::docking() const { return docking_; }
void UIBase::set_docking(int docking) { docking_ = docking; }

bool UIBase::visible() const
{
    return visible_;
}

void UIBase::set_visible(bool visible)
{
    visible_ = visible;
}

Handle<Object> UIBase::parent() const
{
    return parent_;
}

int UIBase::focus_index() const
{
    return focus_index_;
}

void UIBase::set_parent(const Handle<Object>& parent)
{
    parent_ = Persistent<Object>::New(parent);
}

int UIBase::offset_x() const
{
    return offset_rect_.x;
}

void UIBase::set_offset_x(int offset_x)
{
    offset_rect_.x = offset_x;
}

int UIBase::offset_y() const
{
    return offset_rect_.y;
}

void UIBase::set_offset_y(int offset_y)
{
    offset_rect_.y = offset_y;
}

int UIBase::offset_width() const
{
    return offset_rect_.width;
}

void UIBase::set_offset_width(int offset_width)
{
    offset_rect_.width = offset_width;
}

int UIBase::offset_height() const
{
    return offset_rect_.height;
}

void UIBase::set_offset_height(int offset_height)
{
    offset_rect_.height = offset_height;
}

size_t UIBase::children_size() const
{
    return children_.size();
}
