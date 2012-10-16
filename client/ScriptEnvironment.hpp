//
// ScriptEnvironment.hpp
//

#pragma once
#include <iostream>
#include <string>
#include <functional>
#include <memory>
#include <random>
#include <v8.h>
#include <boost/thread.hpp>
#include <boost/timer.hpp>
#include "ResourceManager.hpp"

using namespace v8;
typedef std::function<void(const Handle<Value>&, const std::string)> V8ValueCallBack;
typedef std::function<void(const Handle<Context>&)> V8Block;
typedef std::shared_ptr<std::function<void(const Handle<Context>&)>> V8BlockPtr;

class ScriptEnvironment {
    private:
        struct TimerEvent {
                v8::Persistent<v8::Function> function;
                std::vector<v8::Persistent<v8::Value>> args;
                int delay;
                bool interval;
                boost::timer timer;
        };
        typedef std::shared_ptr<TimerEvent> TimerEventPtr;

    public:
        ScriptEnvironment();
        ~ScriptEnvironment();

        void Execute(const std::string& script,
                const std::string& filename = "",
                const V8ValueCallBack& callback = V8ValueCallBack());

        std::string CompileCoffeeScript(const std::string& script);

        void Load(const std::string& filename, const V8ValueCallBack& callback =
                V8ValueCallBack());

        void ParseJSON(const std::string& json,
                const V8ValueCallBack& callback);

        void With(const V8Block& block);
        void TimedWith(const V8Block& block);

        std::string GetInfo();

        bool allow_eval();
        void set_allow_eval(bool allow);

        static void SetMaxExecutionTime(unsigned int second);

    public:
        template<class T>
        void SetFunction(const std::string& name, T func);

        template<class G, class S>
        void SetProperty(const std::string& name, G getter, S setter);

        inline void SetConstant(const std::string& name, int value);
        inline void SetConstant(const std::string& name, Handle<Value> value);

    private:
        void Terminate();
        void SetBuiltins();
        void UpdateTimerEvents();

    private:
        static Handle<Value> Function_Script_print(const Arguments& args);
        static Handle<Value> Function_Script_info(const Arguments& args);
        static Handle<Value> Function_Script_setTimeout(const Arguments& args);
        static Handle<Value> Function_Script_setInterval(const Arguments& args);
        static Handle<Value> Function_Script_clearTimeout(const Arguments& args);
        static Handle<Value> Function_Script_clearInterval(const Arguments& args);

        static Handle<Value> Function_Math_random(const Arguments& args);

    private:
        void Error(const Handle<Value>& error);

        Persistent<Context> context_;
        Persistent<Object> global_;
        Isolate *isolate_;

        std::map<int, TimerEventPtr> timer_events_;
        boost::thread timer_events_thread_;
        int event_id_;

        boost::thread timelimit_thread_;
        boost::condition_variable condition_;
        boost::mutex mutex_;

		static std::mt19937 random_engine;

        static unsigned int max_execution_time;
        static char SCRIPT_PATH[];

	public:
		void *operator new(size_t size)
		{
			return tlsf_new(ResourceManager::memory_pool(), size);
		}
		void *operator new(size_t, void *p){return p;}
		void operator delete(void *p)
		{
			tlsf_delete(ResourceManager::memory_pool(), p);
		}
		void operator delete(void *, void *){};
};

typedef std::shared_ptr<ScriptEnvironment> ScriptEnvironmentPtr;
typedef std::weak_ptr<ScriptEnvironment> ScriptEnvironmentWeakPtr;

template<class T>
void ScriptEnvironment::SetFunction(const std::string& name,
        T func)
{
    //Locker locker(isolate_);
    //Isolate::Scope isolate_scope(isolate_);
    //HandleScope handle;
    //Context::Scope scope(context_);

    std::string name_buffer(name);
    std::vector<std::string> name_list;
    size_t cursor;
    while ((cursor = name_buffer.find('.', 0)) != std::string::npos) {
        name_list.push_back(name_buffer.substr(0, cursor));
        name_buffer.erase(0, cursor + 1);
    }

    Handle<Object> object = global_;
    for (auto it = name_list.begin(); it != name_list.end(); ++it) {
        const std::string& key_string = *it;
        auto key = String::New(key_string.c_str());
        if (!object->Has(key)) {
            object->Set(key, Object::New());
        }
        object = object->Get(key)->ToObject();
    }

    object->Set(String::New(name_buffer.c_str()),
            FunctionTemplate::New(func)->GetFunction());
}

template<class G, class S>
void ScriptEnvironment::SetProperty(const std::string& name, G getter, S setter)
{
    //Locker locker(isolate_);
    //Isolate::Scope isolate_scope(isolate_);
    //HandleScope handle;
    //Context::Scope scope(context_);

    std::string name_buffer(name);
    std::vector<std::string> name_list;
    size_t cursor;
    while ((cursor = name_buffer.find('.', 0)) != std::string::npos) {
        name_list.push_back(name_buffer.substr(0, cursor));
        name_buffer.erase(0, cursor + 1);
    }

    Handle<Object> object = global_;
    for (auto it = name_list.begin(); it != name_list.end(); ++it) {
        const std::string& key_string = *it;
        auto key = String::New(key_string.c_str());
        if (!object->Has(key)) {
            object->Set(key, Object::New());
        }
        object = object->Get(key)->ToObject();
    }

    object->SetAccessor(String::New(name_buffer.c_str()), getter, setter);
}

void ScriptEnvironment::SetConstant(const std::string& name, int value)
{
    //Locker locker(isolate_);
    //Isolate::Scope isolate_scope(isolate_);
    //HandleScope handle;
    //Context::Scope scope(context_);

    std::string name_buffer(name);
    std::vector<std::string> name_list;
    size_t cursor;
    while ((cursor = name_buffer.find('.', 0)) != std::string::npos) {
        name_list.push_back(name_buffer.substr(0, cursor));
        name_buffer.erase(0, cursor + 1);
    }

    Handle<Object> object = global_;
    for (auto it = name_list.begin(); it != name_list.end(); ++it) {
        const std::string& key_string = *it;
        auto key = String::New(key_string.c_str());
        if (!object->Has(key)) {
            object->Set(key, Object::New());
        }
        object = object->Get(key)->ToObject();
    }

    object->Set(String::New(name_buffer.c_str()), Integer::New(value));
}

void ScriptEnvironment::SetConstant(const std::string& name, Handle<Value> value)
{
    //Locker locker(isolate_);
    //Isolate::Scope isolate_scope(isolate_);
    //HandleScope handle;
    //Context::Scope scope(context_);

    std::string name_buffer(name);
    std::vector<std::string> name_list;
    size_t cursor;
    while ((cursor = name_buffer.find('.', 0)) != std::string::npos) {
        name_list.push_back(name_buffer.substr(0, cursor));
        name_buffer.erase(0, cursor + 1);
    }

    Handle<Object> object = global_;
    for (auto it = name_list.begin(); it != name_list.end(); ++it) {
        const std::string& key_string = *it;
        auto key = String::New(key_string.c_str());
        if (!object->Has(key)) {
            object->Set(key, Object::New());
        }
        object = object->Get(key)->ToObject();
    }

    object->Set(String::New(name_buffer.c_str()), value);
}
