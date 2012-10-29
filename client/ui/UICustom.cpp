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
	if( !graphic_handles_.empty() ){
		BOOST_FOREACH(auto handle,graphic_handles_){
			DeleteGraph(handle);
		}
	}
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

	SetFunction(object, "DrawLine", Function_DrawLine);
	SetFunction(object, "DrawBox", Function_DrawBox);
	SetFunction(object, "DrawEdgeBox", Function_DrawEdgeBox);
	SetFunction(object, "DrawCircle", Function_DrawCircle);
	SetFunction(object, "DrawOval", Function_DrawOval);
	SetFunction(object, "DrawTriangle", Function_DrawTriangle);
	SetFunction(object, "DrawQuadrangle", Function_DrawQuadrangle);
	SetFunction(object, "DrawPixel", Function_DrawPixel);
	SetFunction(object, "Paint", Function_DrawLine);
	SetFunction(object, "DrawPixelSet", Function_DrawBox);
	SetFunction(object, "DrawLineSet", Function_DrawEdgeBox);
	SetFunction(object, "DrawPixel3D", Function_DrawCircle);
	SetFunction(object, "DrawCube", Function_DrawOval);
	SetFunction(object, "DrawSphere", Function_DrawTriangle);
	SetFunction(object, "DrawCapsule3D", Function_DrawQuadrangle);
	SetFunction(object, "DrawCone3D", Function_DrawPixel);

	SetFunction(object, "LoadGraph", Function_LoadGraph);
	SetFunction(object, "DeleteGraph", Function_DeleteGraph);

	SetFunction(object, "DrawText", Function_DrawText);
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


Handle<Value> UICustom::Function_DrawLine(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);
	if(	args[0]->IsInt32() &&
		args[1]->IsInt32() &&
		args[2]->IsInt32() &&
		args[3]->IsInt32() &&
		args[4]->IsInt32() &&
		args[5]->IsInt32() &&
		args[6]->IsInt32() )
	{
		auto x0 = args[0]->Int32Value();
		auto y0 = args[1]->Int32Value();
		auto x1 = args[2]->Int32Value();
		auto y1 = args[3]->Int32Value();
		auto r = args[4]->Int32Value();
		auto g = args[5]->Int32Value();
		auto b = args[6]->Int32Value();

		if( args[7]->IsInt32() )
		{
			auto t = args[7]->Int32Value();
			DrawLine(x0, y0, x1, y1, GetColor(r, g, b),t);
		}else{
			DrawLine(x0, y0, x1, y1, GetColor(r, g, b));
		}

	}

    return Undefined();
}

Handle<Value> UICustom::Function_DrawBox(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);
	if(	args[0]->IsInt32() &&
		args[1]->IsInt32() &&
		args[2]->IsInt32() &&
		args[3]->IsInt32() &&
		args[4]->IsInt32() &&
		args[5]->IsInt32() &&
		args[6]->IsInt32() )
	{
		auto x0 = args[0]->Int32Value();
		auto y0 = args[1]->Int32Value();
		auto x1 = args[2]->Int32Value();
		auto y1 = args[3]->Int32Value();
		auto r = args[4]->Int32Value();
		auto g = args[5]->Int32Value();
		auto b = args[6]->Int32Value();

		if( args[7]->IsBoolean() )
		{
			auto fillflag = args[7]->BooleanValue();
			DrawBox(x0, y0, x1, y1, GetColor(r, g, b),fillflag ? 1 : 0);
		}else{
			DrawBox(x0, y0, x1, y1, GetColor(r, g, b),1);
		}
	}

    return Undefined();
}

Handle<Value> UICustom::Function_DrawEdgeBox(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);
	if(	args[0]->IsInt32() &&
		args[1]->IsInt32() &&
		args[2]->IsInt32() &&
		args[3]->IsInt32() &&
		args[4]->IsInt32() &&
		args[5]->IsInt32() &&
		args[6]->IsInt32() )
	{
		auto x0 = args[0]->Int32Value();
		auto y0 = args[1]->Int32Value();
		auto x1 = args[2]->Int32Value();
		auto y1 = args[3]->Int32Value();
		auto r = args[4]->Int32Value();
		auto g = args[5]->Int32Value();
		auto b = args[6]->Int32Value();

		auto DrawOfOnlyEdge = [](int x, int y, int width, int height, int Color, int thickness)
		{
			DrawBox( x, y, x + width, y + thickness, Color, TRUE);
			DrawBox( x, y, x + thickness, y + height, Color, TRUE);
			DrawBox( x + width - thickness, y, x + width, y + height, Color, TRUE);
			DrawBox( x, y + height - thickness, x + width, y + height, Color, TRUE);
		};// thickness‚ÅŽ¦‚µ‚½‘¾‚³‚Å‰‚Ì‚Ý‚ÌŽlŠpŒ`‚ð•`‰æ

		if( args[7]->IsInt32() )
		{
			auto t = args[7]->Int32Value();
			if( args[8]->IsBoolean() )
			{
				auto fillflag = args[8]->BooleanValue();
				DrawOfOnlyEdge(x0 - t, y0 - t, x1 + t, y1 + t, GetColor(r, g, b),t);
				DrawBox(x0, y0, x1, y1, GetColor(r, g, b),fillflag ? 1 : 0);
			}else{
				DrawOfOnlyEdge(x0 - t, y0 - t, x1 + t, y1 + t, GetColor(r, g, b),t);
			}
		}else{
			DrawOfOnlyEdge(x0 - 1, y0 - 1, x1 + 1, y1 + 1, GetColor(r, g, b),1);
		}

	}

    return Undefined();
}

Handle<Value> UICustom::Function_DrawCircle(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);
	if(	args[0]->IsInt32() &&
		args[1]->IsInt32() &&
		args[2]->IsInt32() &&
		args[3]->IsInt32() &&
		args[4]->IsInt32() &&
		args[5]->IsInt32() )
	{
		auto x0 = args[0]->Int32Value();
		auto y0 = args[1]->Int32Value();
		auto r0 = args[2]->Int32Value();
		auto r = args[3]->Int32Value();
		auto g = args[4]->Int32Value();
		auto b = args[5]->Int32Value();

		if( args[6]->IsBoolean() )
		{
			auto fillflag = args[6]->BooleanValue();
			DrawCircle(x0, y0, r0, GetColor(r, g, b),fillflag ? 1 : 0);
		}else{
			DrawCircle(x0, y0, r0, GetColor(r, g, b),1);
		}
	}

    return Undefined();
}

Handle<Value> UICustom::Function_DrawOval(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);
	if(	args[0]->IsInt32() &&
		args[1]->IsInt32() &&
		args[2]->IsInt32() &&
		args[3]->IsInt32() &&
		args[4]->IsInt32() &&
		args[5]->IsInt32() &&
		args[6]->IsInt32() )
	{
		auto x0 = args[0]->Int32Value();
		auto y0 = args[1]->Int32Value();
		auto rx = args[2]->Int32Value();
		auto ry = args[3]->Int32Value();
		auto r = args[4]->Int32Value();
		auto g = args[5]->Int32Value();
		auto b = args[6]->Int32Value();

		if( args[7]->IsBoolean() )
		{
			auto fillflag = args[7]->BooleanValue();
			DrawOval(x0, y0, rx, ry, GetColor(r, g, b),fillflag ? 1 : 0);
		}else{
			DrawOval(x0, y0, rx, ry, GetColor(r, g, b),1);
		}

	}

    return Undefined();
}

Handle<Value> UICustom::Function_DrawTriangle(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);
	if(	args[0]->IsInt32() &&
		args[1]->IsInt32() &&
		args[2]->IsInt32() &&
		args[3]->IsInt32() &&
		args[4]->IsInt32() &&
		args[5]->IsInt32() &&
		args[6]->IsInt32() &&
		args[7]->IsInt32() &&
		args[8]->IsInt32() )
	{
		auto x0 = args[0]->Int32Value();
		auto y0 = args[1]->Int32Value();
		auto x1 = args[2]->Int32Value();
		auto y1 = args[3]->Int32Value();
		auto x2 = args[4]->Int32Value();
		auto y2 = args[5]->Int32Value();
		auto r = args[6]->Int32Value();
		auto g = args[7]->Int32Value();
		auto b = args[8]->Int32Value();

		if( args[9]->IsBoolean() )
		{
			auto fillflag = args[9]->BooleanValue();
			DrawTriangle(x0, y0, x1, y1, x2, y2, GetColor(r, g, b),fillflag ? 1 : 0);
		}else{
			DrawTriangle(x0, y0, x1, y1, x2, y2, GetColor(r, g, b),1);
		}

	}

    return Undefined();
}

Handle<Value> UICustom::Function_DrawQuadrangle(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);
	if(	args[0]->IsInt32() &&
		args[1]->IsInt32() &&
		args[2]->IsInt32() &&
		args[3]->IsInt32() &&
		args[4]->IsInt32() &&
		args[5]->IsInt32() &&
		args[6]->IsInt32() &&
		args[7]->IsInt32() &&
		args[8]->IsInt32() )
	{
		auto x0 = args[0]->Int32Value();
		auto y0 = args[1]->Int32Value();
		auto x1 = args[2]->Int32Value();
		auto y1 = args[3]->Int32Value();
		auto x2 = args[4]->Int32Value();
		auto y2 = args[5]->Int32Value();
		auto x3 = args[6]->Int32Value();
		auto y3 = args[7]->Int32Value();
		auto r = args[8]->Int32Value();
		auto g = args[9]->Int32Value();
		auto b = args[10]->Int32Value();

		if( args[11]->IsBoolean() )
		{
			auto fillflag = args[9]->BooleanValue();
			DrawQuadrangle(x0, y0, x1, y1, x2, y2, x3, y3, GetColor(r, g, b),fillflag ? 1 : 0);
		}else{
			DrawQuadrangle(x0, y0, x1, y1, x2, y2, x3, y3, GetColor(r, g, b),1);
		}

	}

    return Undefined();
}

Handle<Value> UICustom::Function_DrawPixel(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);
	if(	args[0]->IsInt32() &&
		args[1]->IsInt32() &&
		args[2]->IsInt32() &&
		args[3]->IsInt32() &&
		args[4]->IsInt32() )
	{
		auto x0 = args[0]->Int32Value();
		auto y0 = args[1]->Int32Value();
		auto r = args[2]->Int32Value();
		auto g = args[3]->Int32Value();
		auto b = args[4]->Int32Value();

		DrawPixel(x0, y0, GetColor(r, g, b));

	}

    return Undefined();
}

Handle<Value> UICustom::Function_Paint(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);
	if(	args[0]->IsInt32() &&
		args[1]->IsInt32() &&
		args[2]->IsInt32() &&
		args[3]->IsInt32() &&
		args[4]->IsInt32() )
	{
		auto x0 = args[0]->Int32Value();
		auto y0 = args[1]->Int32Value();
		auto r0 = args[2]->Int32Value();
		auto g0 = args[3]->Int32Value();
		auto b0 = args[4]->Int32Value();
		if(	args[5]->IsInt32() &&
			args[6]->IsInt32() &&
			args[7]->IsInt32() )
		{
			auto r1 = args[5]->Int32Value();
			auto g1 = args[6]->Int32Value();
			auto b1 = args[7]->Int32Value();
			Paint(x0, y0, GetColor(r0, g0, b0), GetColor(r1, g1, b1));
		}else{
			Paint(x0, y0, GetColor(r0, g0, b0));
		}

	}

    return Undefined();
}

Handle<Value> UICustom::Function_DrawPixelSet(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);
	if(	args[0]->IsArray() )
	{
		auto array = Local<Array>::Cast(args[0]);
		auto length = array->Length();
		auto data = static_cast<POINTDATA*>(array->GetPointerFromInternalField(0));
		
		DrawPixelSet(data,length);
	}

    return Undefined();
}

Handle<Value> UICustom::Function_DrawLineSet(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);
	if(	args[0]->IsArray() )
	{
		auto array = Local<Array>::Cast(args[0]);
		auto length = array->Length();
		auto data = static_cast<LINEDATA*>(array->GetPointerFromInternalField(0));
		
		DrawLineSet(data,length);
	}

    return Undefined();
}

Handle<Value> UICustom::Function_DrawPixel3D(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);
	if(	args[0]->IsInt32() &&
		args[1]->IsInt32() &&
		args[2]->IsInt32() &&
		args[3]->IsInt32() &&
		args[4]->IsInt32() &&
		args[5]->IsInt32() )
	{
		auto x0 = args[0]->Int32Value();
		auto y0 = args[1]->Int32Value();
		auto z0 = args[2]->Int32Value();
		auto r = args[3]->Int32Value();
		auto g = args[4]->Int32Value();
		auto b = args[5]->Int32Value();

		DrawPixel3D(VGet(x0, y0, z0), GetColor(r, g, b));

	}

    return Undefined();
}

Handle<Value> UICustom::Function_DrawLine3D(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);
	if(	args[0]->IsInt32() &&
		args[1]->IsInt32() &&
		args[2]->IsInt32() &&
		args[3]->IsInt32() &&
		args[4]->IsInt32() &&
		args[5]->IsInt32() &&
		args[6]->IsInt32() &&
		args[7]->IsInt32() &&
		args[8]->IsInt32() )
	{
		auto x0 = args[0]->Int32Value();
		auto y0 = args[1]->Int32Value();
		auto z0 = args[2]->Int32Value();
		auto x1 = args[3]->Int32Value();
		auto y1 = args[4]->Int32Value();
		auto z1 = args[5]->Int32Value();
		auto r = args[6]->Int32Value();
		auto g = args[7]->Int32Value();
		auto b = args[8]->Int32Value();

		DrawLine3D(VGet(x0, y0, z0), VGet(x1, y1, z1), GetColor(r, g, b));

	}

    return Undefined();
}

Handle<Value> UICustom::Function_DrawCube3D(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);
	if(	args[0]->IsInt32() &&
		args[1]->IsInt32() &&
		args[2]->IsInt32() &&
		args[3]->IsInt32() &&
		args[4]->IsInt32() &&
		args[5]->IsInt32() &&
		args[6]->IsInt32() &&
		args[7]->IsInt32() &&
		args[8]->IsInt32() &&
		args[9]->IsInt32() &&
		args[10]->IsInt32() &&
		args[11]->IsInt32() )
	{
		auto x0 = args[0]->Int32Value();
		auto y0 = args[1]->Int32Value();
		auto z0 = args[2]->Int32Value();
		auto x1 = args[3]->Int32Value();
		auto y1 = args[4]->Int32Value();
		auto z1 = args[5]->Int32Value();
		auto dr = args[6]->Int32Value();
		auto dg = args[7]->Int32Value();
		auto db = args[8]->Int32Value();
		auto sr = args[9]->Int32Value();
		auto sg = args[10]->Int32Value();
		auto sb = args[11]->Int32Value();

		if( args[12]->IsBoolean() )
		{
			auto fillflag = args[9]->BooleanValue();
			DrawCube3D(VGet(x0, y0, z0), VGet(x1, y1, z1), GetColor(dr, dg, db), GetColor(sr, sg, db), fillflag ? 1 : 0);
		}else{
			DrawCube3D(VGet(x0, y0, z0), VGet(x1, y1, z1), GetColor(dr, dg, db), GetColor(sr, sg, db), 1);
		}

	}

    return Undefined();
}

Handle<Value> UICustom::Function_DrawSphere3D(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);
	if(	args[0]->IsInt32() &&
		args[1]->IsInt32() &&
		args[2]->IsInt32() &&
		args[3]->IsInt32() &&
		args[4]->IsInt32() &&
		args[5]->IsInt32() &&
		args[6]->IsInt32() &&
		args[7]->IsInt32() &&
		args[8]->IsInt32() &&
		args[9]->IsInt32() &&
		args[10]->IsInt32() )
	{
		auto x0 = args[0]->Int32Value();
		auto y0 = args[1]->Int32Value();
		auto z0 = args[2]->Int32Value();
		auto r = args[3]->Int32Value();
		auto divnum = args[4]->Int32Value();
		auto dr = args[5]->Int32Value();
		auto dg = args[6]->Int32Value();
		auto db = args[7]->Int32Value();
		auto sr = args[8]->Int32Value();
		auto sg = args[9]->Int32Value();
		auto sb = args[10]->Int32Value();

		if( args[11]->IsBoolean() )
		{
			auto fillflag = args[9]->BooleanValue();
			DrawSphere3D(VGet(x0, y0, z0), r, divnum, GetColor(dr, dg, db), GetColor(sr, sg, db), fillflag ? 1 : 0);
		}else{
			DrawSphere3D(VGet(x0, y0, z0), r, divnum, GetColor(dr, dg, db), GetColor(sr, sg, db), 1);
		}

	}

    return Undefined();
}

std::list<int> UICustom::graphic_handles_ = std::list<int>();
Handle<Value> UICustom::Function_LoadGraph(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);
	if(args[0]->IsString())
	{
		int handle = LoadGraph(unicode::ToTString(*String::Utf8Value(args[0]->ToString())).c_str());
		graphic_handles_.push_back(handle);
		return Int32::New(handle);
	}

	return Undefined();
}

Handle<Value> UICustom::Function_DeleteGraph(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);
	if(args[0]->IsObject())
	{
		auto tmp = args[0]->ToObject();
		assert(tmp->InternalFieldCount() > 0);
		//auto obj = *static_cast<GraphicObjectPtr>(tmp->GetPointerFromInternalField(0));
		// ‚ß‚ñ‚Ç‚¢‚©‚çŒã‚Å
		//auto it = std::find_if(graphics_handle_.begin(),graphics_handle_.end(),[&](GraphicObject src)->bool{return src == obj;});
		//if(it != graphics_handle_.end())
		//{
		//	it = graphics_handle_.erase(it);
		//}
	}

	return Undefined();
}


Handle<Value> UICustom::Function_DrawText(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<UICustom>(
            *static_cast<UIBasePtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);
	if(	args[0]->IsInt32() &&
		args[1]->IsInt32() &&
		args[2]->IsString())
	{
		auto x = args[0]->ToInt32()->Int32Value();
		auto y = args[1]->ToInt32()->Int32Value();
		auto str = unicode::ToTString(*String::Utf8Value(args[2]->ToString()));
		DrawStringToHandle(x,y,str.c_str(),GetColor(0,0,0),ResourceManager::default_font_handle());
	}

	return Undefined();
}