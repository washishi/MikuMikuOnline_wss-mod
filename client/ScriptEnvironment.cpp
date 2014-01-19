//
// ScriptEnvironment.cpp
//

#include "../common/unicode.hpp"
#include <boost/filesystem.hpp>
#include <boost/timer.hpp>
#include "ScriptEnvironment.hpp"
#include "../common/Logger.hpp"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "v8_base.lib")
#pragma comment(lib, "v8_snapshot.lib")
#pragma comment(lib, "preparser_lib.lib")

unsigned int ScriptEnvironment::max_execution_time = 5000;
char ScriptEnvironment::SCRIPT_PATH[] = "system/js";
std::mt19937 ScriptEnvironment::random_engine(static_cast<unsigned long>(GetNowCount()));

ScriptEnvironment::ScriptEnvironment() :
    isolate_(Isolate::New()),
    event_id_(0)
{
    {
        Locker locker(isolate_);
        Isolate::Scope isolate_scope(isolate_);

        HandleScope handle;
        Handle<ObjectTemplate> global_template = ObjectTemplate::New();

        context_ = Persistent<Context>::New(
                Context::New(nullptr, global_template));
        global_ = Persistent<Object>::New(context_->Global());

        set_allow_eval(false);
    }

    With([&](const Handle<Context>& context) {
        Handle<ObjectTemplate> script_template = ObjectTemplate::New();
        script_template->SetInternalFieldCount(1);
        auto script_object = script_template->NewInstance();
        script_object->SetPointerInInternalField(0, this);
        context->Global()->Set(String::New("Script"), script_object);
    });

    // 組み込み関数をセット
    SetBuiltins();

    // ライブラリをロード
    Load("sugar-1.3.5.min.js");

    timer_events_thread_ = boost::thread([&](){
        while(1) {
            UpdateTimerEvents();
            boost::this_thread::sleep(boost::posix_time::milliseconds(1));
        }
    });
}

ScriptEnvironment::~ScriptEnvironment()
{
    // タイマースレッドを中断
    timer_events_thread_.interrupt();
    timer_events_thread_.join();
    {
    Locker locker(isolate_);
    Isolate::Scope isolate_scope(isolate_);
    global_.Dispose();
    context_.Dispose();
    }

    isolate_->Dispose();
}

void ScriptEnvironment::SetBuiltins()
{
    Locker locker(isolate_);
    Isolate::Scope isolate_scope(isolate_);
    HandleScope handle;
    Context::Scope scope(context_);

    /**
    * @module global
    */

    /**
    * スクリプト
    *
    * @class Script
    * @static
    */

    /**
     * コンソールウィンドウに文字列を出力します
     *
     * @method print
     * @param {String} text テキスト
     * @static
     */
    SetFunction("Script.print", Function_Script_print);

    /**
     * スクリプトエンジン及びライブラリの情報を返します
     *
     * @method info
     * @static
     */
    SetFunction("Script.info", Function_Script_info);

    /**
     * 指定時間後に関数を実行します
     *
     * __MMOでは文字列からのコードの動的生成を禁じているため、第一引数に文字列を渡すことはできません__
     *
     * @method setTimeout
     * @param {Function} func 実行する関数オブジェクト
     * @param {Integer} time 遅延時間(ms)
     * @return {Integer} イベントID
     * @static
     */
    SetFunction("Script.setTimeout", Function_Script_setTimeout);

    /**
     * 指定時間毎に関数を実行します
     *
     * __MMOでは文字列からのコードの動的生成を禁じているため、第一引数に文字列を渡すことはできません__
     *
     * @method setInterval
     * @param {Function} func 実行する関数オブジェクト
     * @param {Integer} time 間隔時間(ms)
     * @return {Integer} イベントID
     * @static
     */
    SetFunction("Script.setInterval", Function_Script_setInterval);

    /**
     * setTimeoutで指定したイベントを解除します
     *
     * @method clearTimeout
     * @param {Integer} id イベントID
     * @static
     */
    SetFunction("Script.clearTimeout", Function_Script_clearTimeout);

    /**
     * setIntervalで指定したイベントを解除します
     *
     * @method clearInterval
     * @param {Integer} id イベントID
     * @static
     */
    SetFunction("Script.clearInterval", Function_Script_clearInterval);


	// 組み込みの乱数生成関数を上書き
	auto global = v8::Context::GetCurrent()->Global();
	auto math = global->Get(v8::String::New("Math"));
	math->ToObject()->ForceSet(v8::String::New("random"), 
		FunctionTemplate::New(Function_Math_random)->GetFunction());
}

Handle<Value> ScriptEnvironment::Function_Script_print(const Arguments& args)
{
    String::Utf8Value utf8(args[0]);
    std::cout << unicode::utf82sjis(*utf8) << std::endl;
    return Undefined();
}

Handle<Value> ScriptEnvironment::Function_Script_info(const Arguments& args)
{
    auto self = static_cast<ScriptEnvironment*>(args.Holder()->GetPointerFromInternalField(0));
    return String::New(self->GetInfo().c_str());
}

Handle<Value> ScriptEnvironment::Function_Script_setTimeout(const Arguments& args)
{
    auto self = static_cast<ScriptEnvironment*>(args.Holder()->GetPointerFromInternalField(0));

    if (args.Length() >= 2 && args[0]->IsFunction()) {
        auto event = std::make_shared<TimerEvent>();
        event->interval = false;

        event->function = Persistent<v8::Function>::New(args[0].As<Function>());
        for (int i = 2; i < args.Length(); i++) {
            event->args.push_back(Persistent<Value>::New(args[i]));
        }

        event->delay = args[1]->ToInteger()->Int32Value();
        {
            boost::mutex::scoped_lock lock(self->mutex_);
            self->timer_events_[self->event_id_] = event;
        }

        return Integer::New(self->event_id_++);
    } else {
        return Undefined();
    }
}

Handle<Value> ScriptEnvironment::Function_Script_setInterval(const Arguments& args)
{
    auto self = static_cast<ScriptEnvironment*>(args.Holder()->GetPointerFromInternalField(0));

    if (args.Length() >= 2 && args[0]->IsFunction()) {
        auto event = std::make_shared<TimerEvent>();
        event->interval = true;

        event->function = Persistent<v8::Function>::New(args[0].As<Function>());
        for (int i = 2; i < args.Length(); i++) {
            event->args.push_back(Persistent<Value>::New(args[i]));
        }

        event->delay = args[1]->ToInteger()->Int32Value();
        {
            boost::mutex::scoped_lock lock(self->mutex_);
            self->timer_events_[self->event_id_] = event;
        }

        return Integer::New(self->event_id_++);
    } else {
        return Undefined();
    }
}

Handle<Value> ScriptEnvironment::Function_Script_clearTimeout(const Arguments& args)
{
    auto self = static_cast<ScriptEnvironment*>(args.Holder()->GetPointerFromInternalField(0));

    if (args.Length() >= 1) {
        boost::mutex::scoped_lock lock(self->mutex_);
        self->timer_events_.erase(args[0]->ToInteger()->Int32Value());
    }
    return Undefined();
}

Handle<Value> ScriptEnvironment::Function_Script_clearInterval(const Arguments& args)
{
    auto self = static_cast<ScriptEnvironment*>(args.Holder()->GetPointerFromInternalField(0));

    if (args.Length() >= 1) {
        boost::mutex::scoped_lock lock(self->mutex_);
        self->timer_events_.erase(args[0]->ToInteger()->Int32Value());
    }
    return Undefined();
}

Handle<Value> ScriptEnvironment::Function_Math_random(const Arguments& args)
{
	std::uniform_real<double> distribution;
	return v8::Number::New(distribution(random_engine));
}

void ScriptEnvironment::UpdateTimerEvents()
{
    boost::mutex::scoped_lock lock(mutex_);
    if (timer_events_.size() > 0) {
        for (auto it = timer_events_.begin(); it != timer_events_.end(); ++it) {
            TimerEvent& event = *it->second;
            if (event.timer.elapsed() >= event.delay / 1000.0) {
                TimedWith(
                        [&](const Handle<Context>& context) {
                            if (!event.function.IsEmpty()) {
                                event.function->Call(context_->Global(), event.args.size(), event.args.data());
                            }
                        });
                if (event.interval) {
                    event.timer.restart();
                } else {
                    event.delay = 0;
                }
            }
        }

        for (auto it = timer_events_.begin(); it != timer_events_.end();) {
            TimerEvent& event = *it->second;
            if (event.delay <= 0) {
                With([&](const Handle<Context>& context) {
                    if (!event.function.IsEmpty()) {
                        event.function.Dispose();
                    }
                    for (auto it = event.args.begin(); it != event.args.end(); ++it) {
                        auto arg = *it;
                        if (!arg.IsEmpty()) {
                            arg.Dispose();
                        }
                    }
                });
                timer_events_.erase(it++);
            } else {
                ++it;
            }
        }
    }
}

void ScriptEnvironment::Execute(const std::string& script,
        const std::string& filename, const V8ValueCallBack& callback)
{
//    while (!v8::V8::IdleNotification());

    With(
            [&](const Handle<Context>& context)
            {
                auto source = String::New(script.c_str());

                v8::TryCatch trycatch;
                v8::Handle<v8::Value> result = Undefined();

                std::string compile_path;
                compile_path = filename;

                auto compiled_script = Script::Compile(source, String::New(compile_path.c_str()));
                if (compiled_script.IsEmpty()) {

                    // コンパイルエラー
                    auto exception = trycatch.Exception();
                    String::Utf8Value exception_str(exception);
                    Logger::Error(unicode::ToTString(*exception_str));
                    if (callback) {
                        callback(Undefined(), *exception_str);
                    }

                } else {
                    result = compiled_script->Run();
                    if (result.IsEmpty()) {

                        // ランタイムエラー
                        Handle<Value> exception;
                        HandleScope scope;

                        if (trycatch.CanContinue()) {
                            exception = trycatch.Exception();
                        } else {
                            exception = String::New("Execution limit exceeded.");
                        }

                        String::Utf8Value exception_str(exception);
                        Logger::Error(unicode::ToTString(*exception_str));
                        if (callback) {
                            callback(Undefined(), *exception_str);
                        }

                    } else {
                        if (callback) {
                            callback(result, "");
                        }
                    }
                }

            });

}

void ScriptEnvironment::With(const V8Block& block)
{
    if (block) {
        Locker locker(isolate_);
        Isolate::Scope isolate_scope(isolate_);
        HandleScope handle;
        Context::Scope scope(context_);
        block(context_);
    }
}

void ScriptEnvironment::TimedWith(const V8Block& block)
{
    if (block) {
        boost::thread timelimit_thread([&block, this]() {
            Locker locker(isolate_);
            Isolate::Scope isolate_scope(isolate_);
            HandleScope handle;
            Context::Scope scope(context_);
            block(context_);

        });

        // 時間を過ぎたら強制停止
        if (max_execution_time > 0) {
            auto waiting_time = boost::posix_time::milliseconds(max_execution_time);
            if (!timelimit_thread.timed_join(waiting_time)) {
                Terminate();
                timelimit_thread.join();
            }
        }
    }
}

void ScriptEnvironment::Load(const std::string& filename,
        const V8ValueCallBack& callback)
{
    using namespace boost::filesystem;
    path source_path(SCRIPT_PATH);

    boost::timer t;
    auto script_path = source_path / path(filename);
    if (exists(script_path)) {
        std::ifstream ifs(script_path.string());
        std::string script((std::istreambuf_iterator<char>(ifs)),
                std::istreambuf_iterator<char>());

        Execute(script, script_path.string(),
                [&callback](const Handle<Value>& result, const std::string& error)
                {
                    if (callback) {
                        callback(result, error);
                    }
                });

    } else {
        std::string error = "Error: " + script_path.string() + " No such file.";
        if (callback) {
            callback(Undefined(), error);
        }
    }

    Logger::Debug(_T("Running time for %s: %fsec"), unicode::ToTString(script_path.string()), t.elapsed());
}

void ScriptEnvironment::ParseJSON(const std::string& json,
        const V8ValueCallBack& callback)
{
    With(
            [&](const Handle<Context>& context)
            {
                auto JSONparser = Context::GetCurrent()->Global()->Get(String::New("JSON"));
                if (JSONparser->IsObject()) {
                    auto func = JSONparser->ToObject()->Get(String::New("parse")).As<Function>();

                    Handle<Value> json_string = String::New(json.c_str());
                    Handle<Value> object = func->CallAsFunction(context->Global(), 1, &json_string);
                    if (callback) {
                        if (!object.IsEmpty()) {
                            callback(object, "");
                        } else {
                            callback(Undefined(), "");
                        }
                    }
                }
            });
}

std::string ScriptEnvironment::GetInfo()
{
    std::string info;
    info += std::string("V8 Javascript Engine ") + V8::GetVersion() + std::string("\n");
// ※ CoffeeScriptを利用していないため例外で落ちるのでコメント また SugarLibraryのバージョンも修正
//  With(
//          [&](const Handle<Context>& context)
//          {
//              Handle<String> key = String::New("CoffeeScript");
//              Handle<Object> compiler = context->Global()->GetHiddenValue(key)->ToObject();
//              if (compiler->Has(String::New("VERSION"))) {
//                  info += "CoffeeScript ";
//                  info += *String::AsciiValue(compiler->Get(String::New("VERSION"))->ToString());
//                  info += "\n";
//              }
//          });
//
//  info += "Sugar Library v1.2.5";
    info += "Sugar Library v1.3.5";
// ここまで

    return info;
}

void ScriptEnvironment::Terminate()
{
    V8::TerminateExecution(isolate_);
}

bool ScriptEnvironment::allow_eval()
{
    return context_->IsCodeGenerationFromStringsAllowed();
}

void ScriptEnvironment::set_allow_eval(bool allow)
{
    context_->AllowCodeGenerationFromStrings(allow);
}

void ScriptEnvironment::SetMaxExecutionTime(unsigned int millisecond)
{
    max_execution_time = millisecond;
}

void ScriptEnvironment::Error(const Handle<Value>& error)
{
    std::cout << "Javascript Error >>>" << std::endl;

    String::Utf8Value exception_str(error);
    std::cout << unicode::utf82sjis(*exception_str) << std::endl;

    std::cout << "<<<" << std::endl;
}
