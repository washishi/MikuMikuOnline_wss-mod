//
// UIBase.hpp
//

#pragma once

#include <memory>
#include <string>
#include <v8.h>
#include <iostream>
#include <vector>
#include "../InputManager.hpp"

using namespace v8;

class ScriptEnvironment;
typedef std::weak_ptr<ScriptEnvironment> ScriptEnvironmentWeakPtr;

class UIBase;
typedef std::shared_ptr<UIBase> UIBasePtr;
typedef std::weak_ptr<UIBase> UIBaseWeakPtr;

struct Rect {
    Rect(int x_ = 0, int y_ = 0, int width_ = 0, int height_ = 0) : x(x_), y(y_), width(width_), height(height_) {}
    int x, y, width, height;
};

struct Point {
    Point(int x_ = 0, int y_ = 0) : x(x_), y(y_) {}
    int x, y;
};

class UIBase : public std::enable_shared_from_this<UIBase> {
    public:
        struct Color{
                Color(unsigned char _r = 255, unsigned char _g = 255,
                        unsigned char _b = 255, unsigned char _a = 255) :
                                r(_r),
                                g(_g),
                                b(_b),
                                a(_a)
                {
                }
            unsigned char r, g, b, a;
            std::string ToString() const;
            static Color FromString(const std::string & str);
        };

    public:
        UIBase();
        virtual ~UIBase();

        virtual void ProcessInput(InputManager* input);
        virtual void Update() = 0;
        virtual void Draw() = 0;
        virtual void AsyncUpdate(); // 毎ループ実行する必要のない処理

        int absolute_x() const;
        int absolute_y() const;
        int absolute_width() const;
        int absolute_height() const;

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
        static Handle<Value> on_click(Local<String> property, const AccessorInfo &info);
        static void set_on_click(Local<String> property, Local<Value> value, const AccessorInfo& info);

        int height() const;
        void set_height(int height);
        int width() const;
        void set_width(int width);

        int top() const;
        void set_top(int top);
        int left() const;
        void set_left(int left);
        int right() const;
        void set_right(int right);
        int bottom() const;
        void set_bottom(int bottom);

        int docking() const;
        void set_docking(int docking);
        bool visible() const;
        void set_visible(bool visible);
        Handle<Object> parent() const;
        void set_parent(const Handle<Object>& parent);

        int offset_x() const;
        void set_offset_x(int offset_x);
        int offset_y() const;
        void set_offset_y(int offset_y);
        int offset_width() const;
        void set_offset_width(int offset_width);
        int offset_height() const;
        void set_offset_height(int offset_height);

        int focus_index() const;
        size_t children_size() const;

        enum {
            DOCKING_NONE = 0,
            DOCKING_TOP = 1,
            DOCKING_LEFT = 2,
            DOCKING_RIGHT = 4,
            DOCKING_BOTTOM = 8,
            DOCKING_HCENTER = 16,
            DOCKING_VCENTER = 32,
            DOCKING_ALL = 1 | 2 | 4 | 8
        };

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

        int width_, height_;
        int top_, left_, bottom_, right_;
        Rect absolute_rect_, offset_rect_;
        int docking_;
        bool visible_;
        std::string base_image_;

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
