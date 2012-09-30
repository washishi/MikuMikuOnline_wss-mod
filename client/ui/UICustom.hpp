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
		static Handle<Value> Function_drawLine(const Arguments& args);

};

typedef std::shared_ptr<UICustom> UICustomPtr;