//
// Card.hpp
//

#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <v8.h>
#include <boost/timer.hpp>
#include "ui/include.hpp"
#include "ManagerAccessor.hpp"
#include "InputManager.hpp"
#include "ScriptEnvironment.hpp"

class CardManager;
typedef std::weak_ptr<CardManager> CardManagerWeakPtr;

class Player;
typedef std::shared_ptr<Player> PlayerPtr;

struct TimerEvent {
        v8::Persistent<v8::Function> function;
        std::vector<v8::Persistent<v8::Value>> args;
        int delay;
        bool interval;
        boost::timer timer;
};
typedef std::shared_ptr<TimerEvent> TimerEventPtr;

//typedef std::shared_ptr<std::function<void(const std::string&)>> SocketCallbackPtr;
typedef std::shared_ptr<std::function<bool(const std::string&)>> SocketCallbackPtr; // ※ エラー発生を認識できる様に修正

class Card {
    public:
        Card(const ManagerAccessorPtr& manager_accessor,
             std::string source_folder,
             std::string name,
             std::string icon,
			 std::string type,
             const std::vector<std::string>& scripts = std::vector<std::string>(0),
			 bool native = false);

        ~Card();

        void Run();
        void Execute(const std::string& script, const std::string& filename = "",
                     const V8ValueCallBack& callback = V8ValueCallBack());

        void Update();
        void Draw();
        void ProcessInput(InputManager* input);

		UISuperPtr GetWindow() const;

        bool HasInputEvent();
        void onEnter(const std::string& text);

        std::string author() const;
        bool autorun() const;
        std::string caption() const;
        bool group() const;
        std::string icon() const;
        std::string name() const;
        std::vector<std::string> scripts() const;
        std::string source_folder() const;
        int icon_handle();
        bool running() const;
        bool close_flag() const;
        std::string input_message() const;

        int focus_index() const;
		ScriptEnvironment& script();

        static void set_max_local_storage_size(int size);
		
		void set_ui_board(const UISuperPtr& ui_board);
		void set_on_socket_reply(const SocketCallbackPtr& func);

    private:
        static Card* GetPointerFromObject(const Handle<Object>& object);
        v8::Handle<v8::ObjectTemplate> GenerateGlobalTemplate();
        void SetFunctions();

        void LoadStorage();
        void SaveStorage();

        std::string EscapeString(const std::string& str);
        void CheckFunctionKey(int keynum, const std::string& name, const InputManager& input);

    public:
        /* function */
        static Handle<Value> Function_Network_online(const Arguments& args);
        static Handle<Value> Function_Network_sendJSONAll(const Arguments& args);
        
        static Handle<Value> Function_Model_all(const Arguments& args);

        static Handle<Value> Function_Player_all(const Arguments& args);
        static Handle<Value> Function_Player_getFromId(const Arguments& args);
        static Handle<Value> Function_Player_escape(const Arguments& args);
        static Handle<Value> Function_Player_playMotion(const Arguments& args);
        static Handle<Value> Function_Player_stopMotion(const Arguments& args);
		// ※ 他プレイヤーのモーションを変更できるように追加(MODクライアント互換)
		static Handle<Value> Function_Player_playMotionId(const Arguments& args);
		// ここまで
        static Handle<Value> Function_Music_playBGM(const Arguments& args);
        static Handle<Value> Function_Music_stopBGM(const Arguments& args);
        static Handle<Value> Function_Music_playME(const Arguments& args);
        static Handle<Value> Function_Music_playSE(const Arguments& args);
        static Handle<Value> Function_Music_IsLoadingDone(const Arguments& args);
        static Handle<Value> Function_Music_all(const Arguments& args);

        static Handle<Value> Function_Plugin_Run(const Arguments& args);

		static Handle<Value> Function_Account_id(const Arguments& args);
        static Handle<Value> Function_Account_name(const Arguments& args);
        static Handle<Value> Function_Account_updateName(const Arguments& args);
        static Handle<Value> Function_Account_modelName(const Arguments& args);
        static Handle<Value> Function_Account_updateModelName(const Arguments& args);
        static Handle<Value> Function_Account_trip(const Arguments& args);
        static Handle<Value> Function_Account_updateTrip(const Arguments& args);
        static Handle<Value> Function_Account_channel(const Arguments& args);
        static Handle<Value> Function_Account_updateChannel(const Arguments& args);
            
        static Handle<Value> Function_Screen_width(const Arguments& args);
        static Handle<Value> Function_Screen_height(const Arguments& args);
        static Handle<Value> Function_Screen_mouse_x(const Arguments& args);
        static Handle<Value> Function_Screen_mouse_y(const Arguments& args);
        static Handle<Value> Function_Screen_player_focus(const Arguments& args);

		static Handle<Value> Function_Model_Rebuild(const Arguments& args);
		static Handle<Value> Function_Music_Rebuild(const Arguments& args);
		
		static Handle<Value> Function_Socket_reply(const Arguments& args);

        /* property */
        static Handle<Value> Property_global(Local<String> property, const AccessorInfo &info);
        
		static Handle<Value> Property_onReceiveJSON(Local<String> property, const AccessorInfo &info);
        static void Property_set_onReceiveJSON(Local<String> property, Local<Value> value, const AccessorInfo& info);
        
		static Handle<Value> Property_onLogin(Local<String> property, const AccessorInfo &info);
        static void Property_set_onLogin(Local<String> property, Local<Value> value, const AccessorInfo& info);
        static Handle<Value> Property_onLogout(Local<String> property, const AccessorInfo &info);
        static void Property_set_onLogout(Local<String> property, Local<Value> value, const AccessorInfo& info);
        
		static Handle<Value> Property_Card_board(Local<String> property, const AccessorInfo &info);
        static void Property_set_Card_board(Local<String> property, Local<Value> value, const AccessorInfo& info);
		static Handle<Value> Property_Card_base(Local<String> property, const AccessorInfo &info);
        static void Property_set_Card_base(Local<String> property, Local<Value> value, const AccessorInfo& info);
        
		static Handle<Value> Property_InputBox_onEnter(Local<String> property, const AccessorInfo &info);
        static void Property_set_InputBox_onEnter(Local<String> property, Local<Value> value, const AccessorInfo& info);
        static Handle<Value> Property_InputBox_enable(Local<String> property, const AccessorInfo &info);
        static void Property_set_InputBox_enable(Local<String> property, Local<Value> value, const AccessorInfo& info);
        static Handle<Value> Property_InputBox_message(Local<String> property, const AccessorInfo &info);
        static void Property_set_InputBox_message(Local<String> property, Local<Value> value, const AccessorInfo& info);

		static Handle<Value> Property_Model_onReload(Local<String> property, const AccessorInfo &info);
        static void Property_set_Model_onReload(Local<String> property, Local<Value> value, const AccessorInfo& info);

		static Handle<Value> Property_Music_onReload(Local<String> property, const AccessorInfo &info);
        static void Property_set_Music_onReload(Local<String> property, Local<Value> value, const AccessorInfo& info);


        void OnReceiveJSON(const std::string& info_json, const std::string& msg_json);
        void OnLogin(const PlayerPtr& player);
        void OnLogout(const PlayerPtr& player);
        void OnClose();

		void OnModelReload();
		void OnMusicReload();

    private:
        ManagerAccessorPtr manager_accessor_;

        std::string source_folder_;
        std::string name_;
        std::string author_;
        std::string caption_;
        std::string icon_;
		std::string type_;
        std::vector<std::string> scripts_;
        bool group_;
        bool autorun_;
		bool folding_;
		bool native_;

        ScriptEnvironment script_;

        v8::Persistent<v8::Context> context_;
        std::map<int, TimerEventPtr> timer_events_;
        int event_id_;

		ImageHandlePtr icon_base_handle_, icon_base_close_handle_;

		UISuperPtr ui_board_;
        Persistent<Object> ui_board_obj_;
        Persistent<Object> local_storage_;
        Persistent<Function> network_on_receive_json_;

        struct {
            Persistent<Function> on_login_;
            Persistent<Function> on_logout_;
        } player;

        struct {
            Persistent<Function> on_reload_;
        } model;

		struct {
			Persistent<Function> on_reload_;
		} music;

        struct {
            Persistent<Function> on_enter_;
            bool enable_;
            std::string tabname_;
            std::string message_;
        } inputbox;

		SocketCallbackPtr on_socket_reply_;

        int icon_handle_;

        bool running_;
        bool close_flag_;

        static bool force_gc_flag;

    private:
        static char STORAGE_DIR[];
        static char SCRIPT_PATH[];

        static int max_local_storage_size;
        static std::set<Card*> ptr_set;

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

typedef std::shared_ptr<Card> CardPtr;
typedef std::weak_ptr<Card> CardWeakPtr;

