//
// UIBase.hpp
//

#pragma once

#include "UISuper.hpp"
#include <v8.h>
#include "../InputManager.hpp"

using namespace v8;

class ScriptEnvironment;
typedef std::weak_ptr<ScriptEnvironment> ScriptEnvironmentWeakPtr;

class UIBase;
typedef std::shared_ptr<UIBase> UIBasePtr;
typedef std::weak_ptr<UIBase> UIBaseWeakPtr;

class UIBase : public UISuper {

	public:
        UIBase();
        virtual ~UIBase();

        virtual void ProcessInput(InputManager* input);
        virtual void Update() = 0;
        virtual void Draw() = 0;
        virtual void AsyncUpdate(); // 毎ループ実行する必要のない処理

        /* function */
        static Handle<Value> Function_addChild(const Arguments& args);
        static Handle<Value> Function_removeChild(const Arguments& args);
        static Handle<Value> Function_parent(const Arguments& args);

        /* property */
        static Handle<Value> Property_visible(Local<String> property, const AccessorInfo &info);
        static void Property_set_visible(Local<String> property, Local<Value> value, const AccessorInfo& info);
        static Handle<Value> Property_width(Local<String> property, const AccessorInfo &info);
        static void Property_set_width(Local<String> property, Local<Value> value, const AccessorInfo& info);
        static Handle<Value> Property_height(Local<String> property, const AccessorInfo &info);
        static void Property_set_height(Local<String> property, Local<Value> value, const AccessorInfo& info);
        static Handle<Value> Property_top(Local<String> property, const AccessorInfo &info);
        static void Property_set_top(Local<String> property, Local<Value> value, const AccessorInfo& info);
        static Handle<Value> Property_left(Local<String> property, const AccessorInfo &info);
        static void Property_set_left(Local<String> property, Local<Value> value, const AccessorInfo& info);
        static Handle<Value> Property_right(Local<String> property, const AccessorInfo &info);
        static void Property_set_right(Local<String> property, Local<Value> value, const AccessorInfo& info);
        static Handle<Value> Property_bottom(Local<String> property, const AccessorInfo &info);
        static void Property_set_bottom(Local<String> property, Local<Value> value, const AccessorInfo& info);
        static Handle<Value> Property_docking(Local<String> property, const AccessorInfo &info);
        static void Property_set_docking(Local<String> property, Local<Value> value, const AccessorInfo& info);

        /* event */
        static Handle<Value> Property_on_click(Local<String> property, const AccessorInfo &info);
        static void Property_set_on_click(Local<String> property, Local<Value> value, const AccessorInfo& info);

        Handle<Object> parent() const;
        void set_parent(const Handle<Object>& parent);

        int focus_index() const;
        size_t children_size() const;

        template<class T>
        static void SetObjectTemplate(const std::string& classname,
                Handle<ObjectTemplate>* object);

        template<class F>
        static void SetFunction(Handle<ObjectTemplate>* object, const std::string& name, F func);

        template<class G, class S>
        static void SetProperty(Handle<ObjectTemplate>* object, const std::string& name, G getter, S setter);

        template<class T>
        static void SetConstant(Handle<ObjectTemplate>* object, const std::string& name, T value);

    public:
        static void DefineInstanceTemplate(Handle<ObjectTemplate>* object);

    protected:
        virtual void UpdatePosition();

        void ProcessInputChildren(InputManager* input);
        void UpdateChildren();
        void DrawChildren();
        void AsyncUpdateChildren();

        virtual void UpdateBaseImage();

        void GetParam(const Handle<Object>& object, const std::string& name, int* value);
        void GetParam(const Handle<Object>& object, const std::string& name, bool* value);
        void GetParam(const Handle<Object>& object, const std::string& name, std::string* value);

        void Focus();

    protected:
        static int max_focus_index;

    protected:

        int focus_index_;

        Persistent<Object> parent_;
        std::vector<Persistent<Object>> children_;

        Persistent<Function> on_click_;

};

inline void Destruct(Persistent<Value> handle, void* parameter) {
    auto instance = static_cast<UIBasePtr*>(parameter);
    delete instance;
    handle.Dispose();
}

template<class T>
Handle<Value> Construct(const Arguments& args) {
    UIBasePtr* instance = new UIBasePtr(new T());
    Local<v8::Object> thisObject = args.This();
    assert(thisObject->InternalFieldCount() > 0);
    thisObject->SetInternalField(0, External::New(instance));
    Persistent<v8::Object> holder = Persistent<v8::Object>::New(thisObject);
    holder.MakeWeak(instance, Destruct);
    return thisObject;
}

template<class T>
void UIBase::SetObjectTemplate(const std::string& classname,
        Handle<ObjectTemplate>* object)
{
    auto func = FunctionTemplate::New(Construct<T>);

    func->SetClassName(String::New(classname.c_str()));

    auto instance_template = func->InstanceTemplate();
    instance_template->SetInternalFieldCount(1);

    T::DefineInstanceTemplate(&instance_template);

    (*object)->Set(String::New(("_" + classname).c_str()), func,
            PropertyAttribute(ReadOnly));
}

template<class F>
void UIBase::SetFunction(Handle<ObjectTemplate>* object, const std::string& name, F func)
{
    Handle<ObjectTemplate>& instance_template = *object;
    instance_template->Set(String::New(name.c_str()), FunctionTemplate::New(func));
}

template<class G, class S>
void UIBase::SetProperty(Handle<ObjectTemplate>* object, const std::string& name, G getter, S setter)
{
    Handle<ObjectTemplate>& instance_template = *object;
    instance_template->SetAccessor(String::New(name.c_str()), getter, setter);
}

template<class T>
void UIBase::SetConstant(Handle<ObjectTemplate>* object, const std::string& name, T value)
{
    Handle<ObjectTemplate>& instance_template = *object;
    instance_template->Set(String::New(name.c_str()), value);
}
