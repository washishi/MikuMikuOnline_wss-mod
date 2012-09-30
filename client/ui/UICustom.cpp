//
// UICustom.cpp
//

/**
* @module global
* @submodule UI
 */

/**
 * @class Custom
 * @namespace UI
 * @extends UI.Base
 */

#include "UICustom.hpp"
#include "../ScriptEnvironment.hpp"
#include "../../common/Logger.hpp"

UICustom::UICustom()
{
}

UICustom::~UICustom()
{
}

Handle<Value> UICustom::Property_processinput(Local<String> property, const AccessorInfo &info)
{
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UICustomPtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);
    return self->processinput_;
}

void UICustom::Property_set_processinput(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UICustomPtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);

    self->processinput_ = Persistent<Function>::New(value.As<Function>());
}

Handle<Value> UICustom::Property_update(Local<String> property, const AccessorInfo &info)
{
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UICustomPtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);
    return self->update_;
}

void UICustom::Property_set_update(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UICustomPtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);

    self->update_ = Persistent<Function>::New(value.As<Function>());
}

Handle<Value> UICustom::Property_draw(Local<String> property, const AccessorInfo &info)
{
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UICustomPtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);
    return self->draw_;
}

void UICustom::Property_set_draw(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UICustomPtr*>(info.This()->GetPointerFromInternalField(0))
    );
    assert(self);

    self->draw_ = Persistent<Function>::New(value.As<Function>());
}

void UICustom::DefineInstanceTemplate(Handle<ObjectTemplate>* object)
{
    UIBase::DefineInstanceTemplate(object);

	SetProperty(object, "_processinput", Property_processinput, Property_set_processinput);
	SetProperty(object, "_update", Property_update, Property_set_update);
	SetProperty(object, "_draw", Property_draw, Property_set_draw);

	SetFunction(object, "drawLine", Function_drawLine);
}

void UICustom::ProcessInput(InputManager* input)
{
    if (!visible_) {
        return;
    }
	if (!processinput_.IsEmpty()) {
		processinput_->Call(Context::GetCurrent()->Global(), 0, nullptr);
	}
}

void UICustom::Update()
{
    if (!visible_) {
        return;
    }

	if (!update_.IsEmpty()) {
		update_->Call(Context::GetCurrent()->Global(), 0, nullptr);
	}
}

void UICustom::Draw()
{
    if (!visible_) {
        return;
    }

	if (!draw_.IsEmpty()) {
		draw_->Call(Context::GetCurrent()->Global(), 0, nullptr);
	}
}

Handle<Value> UICustom::Function_drawLine(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);

	DrawLine(100, 100, 200, 200, GetColor(255,255,255));

    return Undefined();
}

