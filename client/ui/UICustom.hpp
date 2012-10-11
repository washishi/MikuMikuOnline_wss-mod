//
// UICustom.hpp
//

#pragma once

#include "UIBase.hpp"

class UICustom : public UIBase {
    public:
        UICustom();
        ~UICustom();
        void ProcessInput(InputManager* input);
        void Update();
        void Draw();

    public:
        static void DefineInstanceTemplate(Handle<ObjectTemplate>* object);

	private:
		Persistent<Function> processinput_, update_, draw_;

		static Handle<Value> Property_processinput(Local<String> property, const AccessorInfo &info);
		static void Property_set_processinput(Local<String> property, Local<Value> value, const AccessorInfo& info);
		static Handle<Value> Property_update(Local<String> property, const AccessorInfo &info);
		static void Property_set_update(Local<String> property, Local<Value> value, const AccessorInfo& info);
		static Handle<Value> Property_draw(Local<String> property, const AccessorInfo &info);
		static void Property_set_draw(Local<String> property, Local<Value> value, const AccessorInfo& info);

	private:
		/* function */
		static Handle<Value> Function_DrawLine(const Arguments& args);
		static Handle<Value> Function_DrawBox(const Arguments& args);
		static Handle<Value> Function_DrawEdgeBox(const Arguments& args);
		static Handle<Value> Function_DrawCircle(const Arguments& args);
		static Handle<Value> Function_DrawOval(const Arguments& args);
		static Handle<Value> Function_DrawTriangle(const Arguments& args);
		static Handle<Value> Function_DrawQuadrangle(const Arguments& args);
		static Handle<Value> Function_DrawPixel(const Arguments& args);
		static Handle<Value> Function_Paint(const Arguments& args);
		static Handle<Value> Function_DrawPixelSet(const Arguments& args);
		static Handle<Value> Function_DrawLineSet(const Arguments& args);
		static Handle<Value> Function_DrawPixel3D(const Arguments& args);
		static Handle<Value> Function_DrawLine3D(const Arguments& args);
		static Handle<Value> Function_DrawCube3D(const Arguments& args);
		static Handle<Value> Function_DrawSphere3D(const Arguments& args);
		static Handle<Value> Function_DrawCapsule3D(const Arguments& args);
		static Handle<Value> Function_DrawCone3D(const Arguments& args);
	
		static Handle<Value> Function_LoadGraph(const Arguments& args);

};

typedef std::shared_ptr<UICustom> UICustomPtr;