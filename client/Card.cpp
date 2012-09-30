//
// Card.cpp
//

#include "Card.hpp"
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <DxLib.h>
#include <Psapi.h>
#include <fstream>
#include "CardManager.hpp"
#include "InputManager.hpp"
#include "CommandManager.hpp"
#include "PlayerManager.hpp"
#include "WorldManager.hpp"
#include "AccountManager.hpp"
#include "Player.hpp"
#include "ResourceManager.hpp"
#include "../common/unicode.hpp"
#include "../common/network/Command.hpp"
#include "../common/network/Utils.hpp"
#include "../common/database/AccountProperty.hpp"
#include "Profiler.hpp"
#include "GenerateJSON.hpp"
#include "Music.hpp"


char Card::STORAGE_DIR[] = "storage";
char Card::SCRIPT_PATH[] = "system/js";
int Card::max_local_storage_size;
std::set<Card*> Card::ptr_set;

bool Card::force_gc_flag;

Card::Card(
    const ManagerAccessorPtr& manager_accessor,
    std::string source_folder,
    std::string name,
    std::string icon,
    const std::vector<std::string>& scripts,
	bool native) :
    manager_accessor_(manager_accessor),
    source_folder_(source_folder),
    name_(name),
    icon_(icon),
    scripts_(scripts),
	folding_(false),
	native_(native),
    event_id_(0),
    icon_handle_(-1),
    running_(false),
    close_flag_(false)
{
    using namespace v8;

	icon_base_handle_  = ResourceManager::LoadCachedGraph(_T("system/images/gui/gui_icon_base.png"));
	icon_base_close_handle_  = ResourceManager::LoadCachedGraph(_T("system/images/gui/gui_icon_base_close.png"));

    // 入力ウィンドウのセットアップ
    inputbox.enable_ = true;
    inputbox.tabname_ = name_;

    // Cardへのポインタをコンテキストに保持
    script_.With([&](const Handle<Context>& context){
        Handle<ObjectTemplate> script_template = ObjectTemplate::New();
        script_template->SetInternalFieldCount(1);
        auto script_object = script_template->NewInstance();
        script_object->SetPointerInInternalField(0, this);

        // デバッグ用にコンテキストに登録したポインタを記録
        ptr_set.insert(this);

        context->Global()->Set(String::New("Network"),  script_object->Clone());
        context->Global()->Set(String::New("Player"),  script_object->Clone());
        context->Global()->Set(String::New("Model"),  script_object->Clone());
        context->Global()->Set(String::New("Account"), script_object->Clone());
        context->Global()->Set(String::New("Music"),  script_object->Clone());
        context->Global()->Set(String::New("InputBox"),   script_object->Clone());
        context->Global()->Set(String::New("Card"),    script_object->Clone());
        context->Global()->Set(String::New("Screen"),  script_object->Clone());
        context->Global()->Set(String::New("UI"),      script_object->Clone());
    });

    // 関数を登録
	script_.With([&](const Handle<Context>& context){
		HandleScope scope;
		SetFunctions();
	});

    // 補助スクリプトをロード
    script_.Load("mmo.js");

    // ストレージを読み込み
    LoadStorage();

    // UIBoardをセット
	if (!ui_board_) {
		script_.Execute("(new UI.Board)", "",
				[&](const Handle<Value>& value, const std::string error) {
					ui_board_obj_ = Persistent<Object>::New(value->ToObject());
					assert(!ui_board_obj_.IsEmpty() && ui_board_obj_->IsObject());
				});

		ui_board_ = *static_cast<UIBasePtr*>(ui_board_obj_->GetPointerFromInternalField(0));
	}
	
	ui_board_->set_icon_image_handle(
		ResourceManager::LoadCachedGraph(unicode::ToTString(source_folder_ + "/" + icon_)));
}

Card::~Card()
{
    OnClose();
    SaveStorage();

    script_.With([&](const Handle<Context>& context)
    {
        ui_board_obj_.Dispose();
        local_storage_.Dispose();
        network_on_receive_json_.Dispose();
        inputbox.on_enter_.Dispose();
    });

    ptr_set.erase(this);
}


Handle<Value> Card::Function_Network_online(const Arguments& args)
{
    return String::New("online");
}

Handle<Value> Card::Function_Network_sendJSONAll(const Arguments& args)
{
    auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));

    if (args.Length() > 0) {
        HandleScope handle;
        v8::String::Utf8Value utf8(args[0]);

        if (auto command_manager = self->manager_accessor_->command_manager().lock()) {
            command_manager->SendJSON(*utf8);
        }
    }
    return Undefined();
}

Handle<Value> Card::Function_Model_all(const Arguments& args)
{
    HandleScope handle;
    auto array = Array::New();

    int i = 0;
	BOOST_FOREACH(const std::string& model_name, ResourceManager::GetModelNameList()) {
		array->Set(i, String::New(model_name.c_str()));
        i++;
    }
    return array;
}


Handle<Value> Card::Function_Player_all(const Arguments& args)
{
    auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));

    HandleScope handle;
    auto array = Array::New();

    if (auto player_manager = self->manager_accessor_->player_manager().lock()) {
        auto players = player_manager->GetAll();
        int i = 0;
        for (auto it = players.begin(); it != players.end(); ++it) {
            auto player = *it;
            array->Set(i, player->GetJSObject());
            i++;
        }
    }
    return array;
}

Handle<Value> Card::Function_Player_getFromId(const Arguments& args)
{
    auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));

    if (args.Length() > 0) {
        if (auto player_manager = self->manager_accessor_->player_manager().lock()) {
            if (auto player = player_manager->GetFromId(args[0]->ToUint32()->Uint32Value())) {
                return player->GetJSObject();
            }
        }
    }
    return Undefined();
}

Handle<Value> Card::Function_Player_escape(const Arguments& args)
{
    auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));
    auto world_manager = self->manager_accessor_->world_manager().lock();
    world_manager->myself()->RescuePosition();
    world_manager->ResetCameraPosition();
    return Undefined();
}

Handle<Value> Card::Function_Player_playMotion(const Arguments& args)
{
    auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));

    if (args.Length() >= 1 && args[0]->IsString()) {
        auto name = std::string(*String::Utf8Value(args[0]->ToString()));
        auto world_manager = self->manager_accessor_->world_manager().lock();
		auto isloop = bool(args[1]->BooleanValue());
        world_manager->myself()->PlayMotion(unicode::ToTString(name),isloop);
    }

    return Undefined();
}

Handle<Value> Card::Function_Player_stopMotion(const Arguments& args)
{

    auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));
    auto world_manager = self->manager_accessor_->world_manager().lock();
    world_manager->myself()->ResetMotion();

    return Undefined();
}

Handle<Value> Card::Function_Music_playBGM(const Arguments& args)
{

	if (args.Length() >= 2 && args[0]->IsString()) {
		auto name = std::string(*String::Utf8Value(args[0]->ToString()));
		auto fade = args[1]->ToBoolean()->BooleanValue();
		ResourceManager::music()->Play(unicode::ToTString(name),fade);
	}

    return Undefined();
}

Handle<Value> Card::Function_Music_stopBGM(const Arguments& args)
{

	if (args.Length() == 1 && args[0]->IsBoolean()) {
		auto fade = args[0]->ToBoolean()->BooleanValue();
		ResourceManager::music()->Stop(fade);
	}else if(args.Length() == 0) {
		ResourceManager::music()->Stop(true);
	}

    return Undefined();
}

Handle<Value> Card::Function_Music_playME(const Arguments& args)
{

	if (args.Length() == 1 && args[0]->IsString()) {
		auto name = std::string(*String::Utf8Value(args[0]->ToString()));
		ResourceManager::music()->PlayME(unicode::ToTString(name));
	}

    return Undefined();
}


Handle<Value> Card::Function_Music_playSE(const Arguments& args)
{

	if (args.Length() == 1 && args[0]->IsString()) {
		auto name = std::string(*String::Utf8Value(args[0]->ToString()));
		ResourceManager::music()->PlaySE(unicode::ToTString(name));
	}

    return Undefined();
}

Handle<Value> Card::Function_Music_all(const Arguments& args)
{
    HandleScope handle;
    auto array = Array::New();

    int i = 0;
	BOOST_FOREACH(const boost::filesystem::path& music_path, ResourceManager::music()->GetMusicList()) {
		array->Set(i, String::New(music_path.stem().string().c_str()));
        i++;
    }
    return array;
}


Handle<Value> Card::Function_Music_IsLoadingDone(const Arguments& args)
{
	if (args.Length() == 1 && args[0]->IsString()) {
	auto name = std::string(*String::Utf8Value(args[0]->ToString()));
	auto check = ResourceManager::music()->CheckLoadedBGM(unicode::ToTString(name));
	return Boolean::New(check);
	}
	return Boolean::New(false);
}

Handle<Value> Card::Function_Account_id(const Arguments& args)
{
    auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));

    if (auto command_manager = self->manager_accessor_->command_manager().lock()) {
        return Integer::NewFromUnsigned(command_manager->user_id());
    }

    return Undefined();
}

Handle<Value> Card::Function_Account_name(const Arguments& args)
{
    auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));

    int id = 0;
    if (auto command_manager = self->manager_accessor_->command_manager().lock()) {
        id = command_manager->user_id();
    }
    if (auto player_manager = self->manager_accessor_->player_manager().lock()) {
        if (auto player = player_manager->GetFromId(id)) {
            return String::New(player->name().c_str());
        }
    }

    return Undefined();
}

Handle<Value> Card::Function_Account_updateName(const Arguments& args)
{
    auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));

    if (args.Length() >= 1 && args[0]->IsString()) {
        auto command_manager = self->manager_accessor_->command_manager().lock();
        auto account_manager = self->manager_accessor_->account_manager().lock();
        auto name = std::string(*String::Utf8Value(args[0]->ToString()));
        account_manager->set_name(name);
        command_manager->Write(network::ServerUpdateAccountProperty(NAME, name));
    }

    return Undefined();
}

Handle<Value> Card::Function_Account_modelName(const Arguments& args)
{
    auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));

    if (auto world_manager = self->manager_accessor_->world_manager().lock()) {
        return String::New(world_manager->myself()->model_handle().name().c_str());
    }

    return Undefined();
}

Handle<Value> Card::Function_Account_updateModelName(const Arguments& args)
{
    auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));

    if (args.Length() >= 1 && args[0]->IsString()) {
        auto name = std::string(*String::Utf8Value(args[0]->ToString()));
        if (auto world_manager = self->manager_accessor_->world_manager().lock()) {
            world_manager->myself()->LoadModel(unicode::ToTString(name));
			world_manager->myself()->ResetMotion();
        }
        auto account_manager = self->manager_accessor_->account_manager().lock();
        auto command_manager = self->manager_accessor_->command_manager().lock();
        account_manager->set_model_name(name);
        command_manager->Write(network::ServerUpdateAccountProperty(MODEL_NAME, name));
    }

    return Undefined();
}

Handle<Value> Card::Function_Account_trip(const Arguments& args)
{
    auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));

    int id = 0;
    if (auto command_manager = self->manager_accessor_->command_manager().lock()) {
        id = command_manager->user_id();
    }
    if (auto player_manager = self->manager_accessor_->player_manager().lock()) {
        if (auto player = player_manager->GetFromId(id)) {
            return String::New(player->trip().c_str());
        }
    }

    return Undefined();
}

Handle<Value> Card::Function_Account_updateTrip(const Arguments& args)
{
    auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));

    if (args.Length() >= 1 &&
            args[0]->IsString() &&
            args[0]->ToString()->Length() > 0) {

        if (auto command_manager = self->manager_accessor_->command_manager().lock()) {
            auto trip = std::string(*String::Utf8Value(args[0]->ToString()));
            command_manager->Write(network::ServerUpdateAccountProperty(TRIP, trip));
        }
    }

    return Undefined();
}

Handle<Value> Card::Function_Account_channel(const Arguments& args)
{
    auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));

    int id = 0;
    if (auto command_manager = self->manager_accessor_->command_manager().lock()) {
        id = command_manager->user_id();
    }
    if (auto player_manager = self->manager_accessor_->player_manager().lock()) {
        if (auto player = player_manager->GetFromId(id)) {
            return Integer::New(player->channel());
        }
    }

    return Undefined();
}

Handle<Value> Card::Function_Account_updateChannel(const Arguments& args)
{
    auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));

	if (args.Length() >= 1 && args[0]->IsNumber()) {

		int channel = args[0]->ToInteger()->Int32Value();

		if (channel >= 0 && channel <= UCHAR_MAX) {
			if (auto command_manager = self->manager_accessor_->command_manager().lock()) {
				auto channel_str = network::Utils::Serialize((unsigned char)channel);
				command_manager->Write(network::ServerUpdateAccountProperty(CHANNEL, channel_str));
			}
		}
    }

    return Undefined();
}


Handle<Value> Card::Function_Screen_width(const Arguments& args)
{
    int width;
    GetScreenState(&width, nullptr, nullptr);
    return Integer::New(width);
}

Handle<Value> Card::Function_Screen_height(const Arguments& args)
{
    int height;
    GetScreenState(nullptr, &height, nullptr);
    return Integer::New(height);
}

Handle<Value> Card::Function_Screen_mouse_x(const Arguments& args)
{
    int x;
    GetMousePoint(&x, nullptr);
    return Integer::New(x);
}

Handle<Value> Card::Function_Screen_mouse_y(const Arguments& args)
{
    int y;
    GetMousePoint(&y, nullptr);
    return Integer::New(y);
}

Handle<Value> Card::Function_Screen_player_focus(const Arguments& args)
{
    auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));

    if (auto card_manager = self->manager_accessor_->card_manager().lock()) {
        card_manager->FocusPlayer();
    }
    return Undefined();
}

Handle<Value> Card::Function_Model_Rebuild(const Arguments& args)
{
	JsonGen jsongen;
	ResourceManager::BuildModelFileTree();

    auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));
    if (auto card_manager = self->manager_accessor_->card_manager().lock()) {
		card_manager->OnModelReload();
    }

	return Undefined();
}

Handle<Value> Card::Function_Music_Rebuild(const Arguments& args)
{
	ResourceManager::music()->Init();

    auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));
    if (auto card_manager = self->manager_accessor_->card_manager().lock()) {
		card_manager->OnMusicReload();
    }

	return Undefined();
}



Handle<Value> Card::Property_global(Local<String> property, const AccessorInfo &info)
{
     return Context::GetCurrent()->Global();
}

Handle<Value> Card::Property_onReceiveJSON(Local<String> property, const AccessorInfo &info)
{
    assert(info.Holder()->InternalFieldCount() > 0);
    auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
    assert(ptr_set.find(self) != ptr_set.end());

    return self->network_on_receive_json_;
}

void Card::Property_set_onReceiveJSON(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    if (value->IsFunction()) {
        assert(info.Holder()->InternalFieldCount() > 0);
        auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
        assert(ptr_set.find(self) != ptr_set.end());

        self->network_on_receive_json_ = Persistent<Function>::New(value.As<Function>());
    }
}

Handle<Value> Card::Property_onLogin(Local<String> property, const AccessorInfo &info)
{
    assert(info.Holder()->InternalFieldCount() > 0);
    auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
    return self->player.on_login_;
}

void Card::Property_set_onLogin(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    if (value->IsFunction()) {
        assert(info.Holder()->InternalFieldCount() > 0);
        auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
        self->player.on_login_ = Persistent<Function>::New(value.As<Function>());
    }
}

Handle<Value> Card::Property_onLogout(Local<String> property, const AccessorInfo &info)
{
    assert(info.Holder()->InternalFieldCount() > 0);
    auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
    return self->player.on_logout_;
}

void Card::Property_set_onLogout(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    if (value->IsFunction()) {
        assert(info.Holder()->InternalFieldCount() > 0);
        auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
        self->player.on_logout_ = Persistent<Function>::New(value.As<Function>());
    }
}

Handle<Value> Card::Property_Card_board(Local<String> property, const AccessorInfo &info)
{
    assert(info.Holder()->InternalFieldCount() > 0);
    auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
    assert(ptr_set.find(self) != ptr_set.end());

    return self->ui_board_obj_;
}

void Card::Property_set_Card_board(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.Holder()->InternalFieldCount() > 0);
    auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
    assert(ptr_set.find(self) != ptr_set.end());
}

Handle<Value> Card::Property_InputBox_onEnter(Local<String> property, const AccessorInfo &info)
{
    assert(info.Holder()->InternalFieldCount() > 0);
    auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
    return self->inputbox.on_enter_;
}

void Card::Property_set_InputBox_onEnter(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    if (value->IsFunction()) {
        assert(info.Holder()->InternalFieldCount() > 0);
        auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
        self->inputbox.on_enter_ = Persistent<Function>::New(value.As<Function>());
    }
}

Handle<Value> Card::Property_InputBox_enable(Local<String> property, const AccessorInfo &info)
{
    assert(info.Holder()->InternalFieldCount() > 0);
    auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
    return Boolean::New(self->inputbox.enable_);
}

void Card::Property_set_InputBox_enable(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.Holder()->InternalFieldCount() > 0);
    auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
    self->inputbox.enable_ = value->ToBoolean()->BooleanValue();
}

Handle<Value> Card::Property_InputBox_message(Local<String> property, const AccessorInfo &info)
{
    assert(info.Holder()->InternalFieldCount() > 0);
    auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
    return String::New(self->inputbox.message_.c_str());
}

void Card::Property_set_InputBox_message(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    assert(info.Holder()->InternalFieldCount() > 0);
    auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
    self->inputbox.message_ = *String::Utf8Value(value->ToString());
}

Handle<Value> Card::Property_Model_onReload(Local<String> property, const AccessorInfo &info)
{
    assert(info.Holder()->InternalFieldCount() > 0);
    auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
	return self->model.on_reload_;
}

void Card::Property_set_Model_onReload(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    if (value->IsFunction()) {
        assert(info.Holder()->InternalFieldCount() > 0);
        auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
        self->model.on_reload_ = Persistent<Function>::New(value.As<Function>());
    }
}

Handle<Value> Card::Property_Music_onReload(Local<String> property, const AccessorInfo &info)
{
    assert(info.Holder()->InternalFieldCount() > 0);
    auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
	return self->music.on_reload_;
}

void Card::Property_set_Music_onReload(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
    if (value->IsFunction()) {
        assert(info.Holder()->InternalFieldCount() > 0);
        auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
        self->music.on_reload_ = Persistent<Function>::New(value.As<Function>());
    }
}

void Card::SetFunctions()
{

    /**
    * @module global
    */

    script_.SetProperty("global", Property_global, (AccessorSetter)0);

    /**
    * スクリプト
    *
    * @class Script
    * @static
    */

    ///**
    // * ローカルデータへのアクセサ
    // *
    // * このオブジェクトに設定された文字列、数値、ブール値はスクリプトやクライアントが終了してもローカルに保存され、次回実行時に復元されます。
    // *
    // * 配列やオブジェクトはそのまま保存できないため、JSON.stringifyなどを使う必要があります。
    // *
    // * @property localStorage
    // * @type Object
    // * @example
    // *     // 保存
    // *     localStorage['savedata'] = "でーた";
    // *
    // *     // JSONで保存
    // *     localStorage['json'] = JSON.stringify({"data": 1234});
    // *
    // * @static
    // */
    //script_.SetProperty("Card.localStorage",
    //        [](Local<String> property, const AccessorInfo &info) -> Handle<Value> {
    //            assert(info.Holder()->InternalFieldCount() > 0);
    //            auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
    //            assert(ptr_set.find(self) != ptr_set.end());

    //            if (self->local_storage_.IsEmpty()) {
    //                self->local_storage_ = Persistent<Object>::New(Object::New());
    //            }
    //            return self->local_storage_;
    //        },
    //        [](Local<String> property, Local<Value> value, const AccessorInfo& info) {
    //            assert(info.Holder()->InternalFieldCount() > 0);
    //            auto self = static_cast<Card*>(info.Holder()->GetPointerFromInternalField(0));
    //            assert(ptr_set.find(self) != ptr_set.end());

    //            self->local_storage_ = Persistent<Object>::New(value->ToObject());
    //        });

    /**
    * ネットワーク
    *
    * @class Network
    * @static
    */

    /**
     * サーバーへの接続状態を返します
     *
     * @method status
     * @return {String} 接続状態
     * @static
     */
    script_.SetFunction("Network.online", Function_Network_online);

    script_.SetFunction("Network._sendJSONAll", Function_Network_sendJSONAll);

    script_.SetProperty("Network._onReceiveJSON", Property_onReceiveJSON, Property_set_onReceiveJSON);

    /**
     * モデル
     *
     * @class Model
     * @static
     */

    /**
     * モデル名の一覧を返します
     *
     * @method all
     * @return {Array} 
     * @static
     */
    script_.SetFunction("Model.all", Function_Model_all);

    /**
     * モデルファイルの構造を再構築します
     *
     * @method rebuild
     * @static
     */
	script_.SetFunction("Model.rebuild", Function_Model_Rebuild);
	
    script_.SetProperty("Model.onReload", Property_Model_onReload, Property_set_Model_onReload);

    /**
     * プレイヤー
     *
     * @class Player
     * @static
     */

    /**
     * ログインしているプレイヤーの一覧を返します
     *
     * @method all
     * @return {Array} Playerオブジェクトの配列
     * @static
     */
    script_.SetFunction("Player.all", Function_Player_all);

    /**
    * プレイヤーのIDからプレイヤーオブジェクトを取得します
    *
    * 存在しないプレイヤーやログインしていないプレイヤーの場合は Undefined を返します
    *
    * @method getFromId
    * @param {Integer} プレイヤーのID
    * @return {Player} プレイヤーオブジェクト
    */
    script_.SetFunction("Player.getFromId", Function_Player_getFromId);

    /**
     * プレイヤーがログインした時に呼ばれます
     *
     * @event onLogin
     * @static
     */
    script_.SetProperty("Player.onLogin", Property_onLogin, Property_set_onLogin);

    /**
     * プレイヤーがログアウトした時に呼ばれます
     *
     * @event onLogout
     * @static
     */
    script_.SetProperty("Player.onLogout", Property_onLogout, Property_set_onLogout);

    /**
    * 自プレイヤーの座標を初期位置にリセットします
    *
    * @method escape
    * @static
    */
    script_.SetFunction("Player.escape", Function_Player_escape);

    /**
     * モーションを再生します
     *
     * @method playMotion
     * @param {String} name モーション名
     * @static
     */
    script_.SetFunction("Player.playMotion", Function_Player_playMotion);

    /**
     * モーションを停止します
     *
     * @method playMotion
     * @param {String} name モーション名
     * @static
     */
    script_.SetFunction("Player.stopMotion", Function_Player_stopMotion);

    /**
     * 音楽を再生します
     *
     * @method playMotion
     * @param {String} name BGM名
     * @static
     */
    script_.SetFunction("Music.play", Function_Music_playBGM);

    /**
     * 音楽を停止します
     *
     * @method playMotion
     * @param
     * @static
     */
    script_.SetFunction("Music.stop", Function_Music_stopBGM);

    /**
     * MEを再生します
     *
     * @method playMotion
     * @param {String} name ME名
     * @static
     */
    script_.SetFunction("Music.playME", Function_Music_playME);

	/**
     * SEを再生します
     *
     * @method playMotion
     * @param {String} name SE名
     * @static
     */
    script_.SetFunction("Music.playSE", Function_Music_playSE);

    /**
     * BGM,MEのロードが終了しているかチェックします
     *
     * @method playMotion
     * @param {String} name BGM、ME名
     * @static
     */
    script_.SetFunction("Music.loadCheck", Function_Music_IsLoadingDone);

    /**
     * BGMのリストを返します
     *
     * @method playMotion
     * @param
     * @static
     */
    script_.SetFunction("Music.all", Function_Music_all);

    /**
     * 音楽の構造を再構築します
     *
     * @method rebuild
     * @static
     */
	script_.SetFunction("Music.rebuild", Function_Music_Rebuild);

    script_.SetProperty("Music.onReload", Property_Music_onReload, Property_set_Music_onReload);
	/**
     * アカウント
     *
     * @class Account
     * @static
     */

    /**
     * ユーザーIDを返します
     *
     * @method id
     * @return {Integer} ユーザーID
     *
     * @static
     */
    script_.SetFunction("Account.id", Function_Account_id);

    /**
     * 現在のニックネームを返します
     *
     * @method name
     * @return {String} ニックネーム
     *
     * @static
     */
    script_.SetFunction("Account.name", Function_Account_name);

    /**
     * ニックネームを新しく設定します
     *
     * @method updateName
     * @param {String} name 名前
     *
     * @static
     */
    script_.SetFunction("Account.updateName", Function_Account_updateName);

    /**
     * 現在のモデル名を返します
     *
     * @method modelName
     * @return {String} モデル名
     *
     * @static
     */
    script_.SetFunction("Account.modelName", Function_Account_modelName);

    /**
     * モデル名を新しく設定します
     *
     * @method updateModelName
     * @param {String} name モデル名
     *
     * @static
     */
    script_.SetFunction("Account.updateModelName", Function_Account_updateModelName);

    /**
     * 現在設定されているトリップを返します
     *
     * トリップは20文字のASCII文字列です
     *
     * @method trip
     * @return {String} トリップ
     *
     * @static
     */
    script_.SetFunction("Account.trip", Function_Account_trip);

    /**
     * トリップを新しく設定します
     *
     * @method updateTrip
     * @param {String} password トリップパスワード
     * @example
     *     Account.updateTrip('秘密の文字列');
     *
     * @static
     */
    script_.SetFunction("Account.updateTrip", Function_Account_updateTrip);

    /**
     * 現在のチャンネルを返します
     *
     * @method channel
     * @return {Integer} チャンネル
     *
     * @static
     */
    script_.SetFunction("Account.channel", Function_Account_channel);

    /**
     * 現在のチャンネルを設定します
     *
     * @method updateChannel
     * @param {Integer} channel チャンネル
     *
     * @static
     */
    script_.SetFunction("Account.updateChannel", Function_Account_updateChannel);

    /**
     * カード
     *
     * @class Card
     * @static
     */

    script_.SetProperty("Card.board", Property_Card_board, Property_set_Card_board);

    /**
     * スクリーン情報
     *
     * @class Screen
     * @static
     */

    /**
     * クライアントウィンドウの幅を返します
     *
     * @method width
     * @return {Integer} クライアントウィンドウの幅(px)
     * @static
     */
    script_.SetFunction("Screen.width", Function_Screen_width);

    /**
     * クライアントウィンドウの高さを返します
     *
     * @method height
     * @return {Integer} クライアントウィンドウの高さ(px)
     * @static
     */
    script_.SetFunction("Screen.height", Function_Screen_height);

    /**
     * マウスのクライアントX座標を返します
     *
     * @method mouse_x
     * @return {Integer} マウスX座標(px)
     * @static
     */
    script_.SetFunction("Screen.mouse_x", Function_Screen_mouse_x);

    /**
     * マウスのクライアントY座標を返します
     *
     * @method mouse_y
     * @return {Integer} マウスY座標(px)
     * @static
     */
    script_.SetFunction("Screen.mouse_y", Function_Screen_mouse_y);

    /**
     * プレイヤーをアクティブにして、操作できるようにします
     *
     * @method player_focus
     * @static
     */
    script_.SetFunction("Screen.player_focus", Function_Screen_player_focus);

//    /**
//     * ワールド座標をスクリーン座標に変換します
//     *
//     * @method worldToScreen
//     * @param {Float} x
//     * @param {Float} y
//     * @param {Float} z
//     * @retrun {Array} スクリーン座標
//     * @static
//     */
//    script_.SetFunction("Screen.worldToScreen",
//            [](const Arguments& args) -> Handle<Value> {
//                auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));
//
//                return Undefined();
//            });

    /**
     * 入力ウィンドウへのアクセスを提供するクラス
     *
     * @class InputBox
     * @static
     */

    /**
     * テキストボックスの内容が確定された時に呼ばれます
     *
     * @event onEnter
     * @return {String} text テキストボックスの内容
     *
     * @static
     */
    script_.SetProperty("InputBox.onEnter", Property_InputBox_onEnter, Property_set_InputBox_onEnter);

    /**
     * テキストボックスに入力できるかどうか
     *
     * このプロパティに trueが設定されていて、
     * 且つinputbox.onEnterに関数がセットされている場合に入力ウィンドウにタブが表示されます。
     *
     * @property enable
     * @type Boolean
     * @default true
     * @static
     */
    script_.SetProperty("InputBox.enable", Property_InputBox_enable, Property_set_InputBox_enable);

    /**
     * テキストボックスに表示するメッセージ
     *
     * @property message
     * @type String
     * @static
     */
    script_.SetProperty("InputBox.message", Property_InputBox_message, Property_set_InputBox_message);

    /**
     * 指定されたファンクションキーが押された時に実行される関数を指定します
     *
     * @class FunctionKey
     * @static
     */

    /**
     * @property {Function} F1
     * @static
     */
    /**
     * @property {Function} F2
     * @static
     */
    /**
     * @property {Function} F3
     * @static
     */
    /**
     * @property {Function} F4
     * @static
     */
    /**
     * @property {Function} F5
     * @static
     */
    /**
     * @property {Function} F6
     * @static
     */
    /**
     * @property {Function} F7
     * @static
     */
    /**
     * @property {Function} F8
     * @static
     */
    /**
     * @property {Function} F9
     * @static
     */
    /**
     * @property {Function} F10
     * @static
     */
    /**
     * @property {Function} F11
     * @static
     */
    /**
     * @property {Function} F12
     * @static
     */

    script_.SetConstant("FunctionKey.F1",  Undefined());
    script_.SetConstant("FunctionKey.F2",  Undefined());
    script_.SetConstant("FunctionKey.F3",  Undefined());
    script_.SetConstant("FunctionKey.F4",  Undefined());
    script_.SetConstant("FunctionKey.F5",  Undefined());
    script_.SetConstant("FunctionKey.F6",  Undefined());
    script_.SetConstant("FunctionKey.F7",  Undefined());
    script_.SetConstant("FunctionKey.F8",  Undefined());
    script_.SetConstant("FunctionKey.F9",  Undefined());
    script_.SetConstant("FunctionKey.F10", Undefined());
    script_.SetConstant("FunctionKey.F11", Undefined());
    script_.SetConstant("FunctionKey.F12", Undefined());

    /**
     * GUI
     *
     * @class UI
     * @static
     */

    script_.With(
            [&](const Handle<Context>& context) {
                v8::Handle<v8::ObjectTemplate> object_template = v8::ObjectTemplate::New();
                object_template->SetInternalFieldCount(1);

                UIBase::SetObjectTemplate<UIButton>("Button", &object_template);
                UIBase::SetObjectTemplate<UIBoard>("Board", &object_template);
                UIBase::SetObjectTemplate<UILabel>("Label", &object_template);
                UIBase::SetObjectTemplate<UIList>("List", &object_template);
                UIBase::SetObjectTemplate<UIGroup>("Group", &object_template);
                UIBase::SetObjectTemplate<UICustom>("Custom", &object_template);
                auto script_object = object_template->NewInstance();
                script_object->SetPointerInInternalField(0, this);
                context->Global()->Set(String::New("UI"), script_object);
            });

    // TODO: GetPointerFromObject

    /**
     * @property DOCKING_LEFT
     * @type Integer
     * @static
     * @final
    **/
    script_.SetConstant("UI.DOCKING_LEFT", UIBase::DOCKING_LEFT);

    /**
     * @property DOCKING_TOP
     * @type Integer
     * @static
     * @final
    **/
    script_.SetConstant("UI.DOCKING_TOP", UIBase::DOCKING_TOP);

    /**
     * @property DOCKING_BOTTOM
     * @type Integer
     * @static
     * @final
    **/
    script_.SetConstant("UI.DOCKING_BOTTOM", UIBase::DOCKING_BOTTOM);

    /**
     * @property DOCKING_RIGHT
     * @type Integer
     * @static
     * @final
    **/
    script_.SetConstant("UI.DOCKING_RIGHT", UIBase::DOCKING_RIGHT);


//    /**
//     * GUIツリーにオブジェクトを追加します
//     *
//     * @method addChildren
//     * @param {GUI} object GUIオブジェクト
//     * @static
//     */
//    script_.SetFunction("UI.addChildren",
//            [](const Arguments& args) -> Handle<Value> {
//                auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));
//
//                if (args.Length() >= 1 && args[0]->IsObject()) {
//                    v8::HandleScope handle;
//                    self->children_.push_back(std::make_shared<gui::GUIHolder<gui::Object>>(args[0]));
//                }
//                return v8::Undefined();
//            });

//    /**
//    * GUIツリーからオブジェクトを削除します
//    *
//    * @method removeChildren
//    * @param {GUI} object GUIオブジェクト
//    * @static
//    */
//    script_.SetFunction("GUI.removeChildren",
//            [](const Arguments& args) -> Handle<Value> {
//                auto self = static_cast<Card*>(args.Holder()->GetPointerFromInternalField(0));
//
//                if (args.Length() >= 1 && args[0]->IsObject()) {
//                    v8::HandleScope handle;
//                    auto it = std::remove(
//                            self->children_.begin(),
//                            self->children_.end(),
//                            std::make_shared<gui::GUIHolder<gui::Object>>(args[0]));
//                    self->children_.erase(it);
//                }
//                return v8::Undefined();
//            });

}

void Card::Run()
{
    using namespace boost::filesystem;
    path source_path(source_folder_);

    if (is_directory(source_path)) {
        for (auto it = scripts_.begin(); it != scripts_.end(); ++it) {
        const std::string& script_name = *it;
            auto script_path = source_path / path(script_name);
            if (exists(script_path)) {
                std::ifstream ifs(script_path.string());
                std::string script((std::istreambuf_iterator<char>(ifs)),
                        std::istreambuf_iterator<char>());
                script_.Execute(script, script_path.string(),
                        [](const Handle<Value>& v, const std::string e) {
                    std::cout << "Error: " << e << std::endl;
                });

            } else {
                std::cout << "Error: " << script_path.string()
                        << " No such file." << std::endl;
            }
        }
    }

    running_ = true;
}

void Card::Execute(const std::string& script, const std::string& filename,
             const V8ValueCallBack& callback)
{
    auto compiled_code = script_.CompileCoffeeScript(script);
    script_.Execute(compiled_code, filename, callback);
}

void Card::Update()
{
	MMO_PROFILE_FUNCTION;

  //  static int cnt = 0;
  //  if (!ui_board_obj_.IsEmpty() && ui_board_obj_->IsObject()) {
		//auto ptr = *static_cast<UIBasePtr*>(ui_board_obj_->GetPointerFromInternalField(0));
		//if (ptr->children_size() > 0) {
		//	ptr->Update();
		//	if (cnt % 2 == 0) {
		//		ptr->AsyncUpdate();
		//	}
		//}
  //  }
  //  cnt++;
}

void Card::Draw()
{
	MMO_PROFILE_FUNCTION;

   // if (!ui_board_obj_.IsEmpty() && ui_board_obj_->IsObject()) {
   //     auto ptr = *static_cast<UIBasePtr*>(ui_board_obj_->GetPointerFromInternalField(0));

   //     if (ptr->children_size() > 0) {
			//ptr->Draw();
   //     }
   // }
}

UISuperPtr Card::GetWindow() const
{
	if (native_) {
		return ui_board_;
	} else {
	    if (ui_board_obj_->IsObject()) {
		    auto ptr = *static_cast<UIBasePtr*>(ui_board_obj_->GetPointerFromInternalField(0));
		    if (ptr->children_size() > 0) {
				return ptr;
		    }
	    }
	}
	return UISuperPtr();
}

void Card::ProcessInput(InputManager* input)
{
	MMO_PROFILE_FUNCTION;

 //   CheckFunctionKey(KEY_INPUT_F1,  "F1",  *input);
 //   CheckFunctionKey(KEY_INPUT_F2,  "F2",  *input);
 //   CheckFunctionKey(KEY_INPUT_F3,  "F3",  *input);
 //   CheckFunctionKey(KEY_INPUT_F4,  "F4",  *input);
 //   CheckFunctionKey(KEY_INPUT_F5,  "F5",  *input);
 //   CheckFunctionKey(KEY_INPUT_F6,  "F6",  *input);
 //   CheckFunctionKey(KEY_INPUT_F7,  "F7",  *input);
 //   CheckFunctionKey(KEY_INPUT_F8,  "F8",  *input);
 //   CheckFunctionKey(KEY_INPUT_F9,  "F9",  *input);
 //   CheckFunctionKey(KEY_INPUT_F10, "F10", *input);
 //   CheckFunctionKey(KEY_INPUT_F11, "F11", *input);
 //   CheckFunctionKey(KEY_INPUT_F12, "F12", *input);

	//script_.With([&](const Handle<Context>& context){
	//	if (!ui_board_obj_.IsEmpty() && ui_board_obj_->IsObject()) {
	//		auto ptr = *static_cast<UIBasePtr*>(ui_board_obj_->GetPointerFromInternalField(0));
	//		if (ptr->children_size() > 0) {
	//			ptr->ProcessInput(input);
	//		}
	//	}
	//});
}

void Card::CheckFunctionKey(int keynum, const std::string& name, const InputManager& input)
{
    if (input.GetKeyCount(keynum) == 1) {
        script_.TimedWith([&](const Handle<Context>& context){
            auto obj = context->Global()->Get(String::New("FunctionKey"));
            if (!obj.IsEmpty() && obj->IsObject()) {
                auto func = obj->ToObject()->Get(String::New(name.c_str()));
                if (!func.IsEmpty() && func->IsFunction()) {
                    func.As<Function>()->CallAsFunction(context->Global(), 0, nullptr);
                }
            }
        });
    }
}

bool Card::HasInputEvent()
{
    bool flag = false;
    script_.With([&](const Handle<Context>& context)
    {
        flag = !inputbox.on_enter_.IsEmpty() && inputbox.on_enter_->IsFunction();
    });
    return flag & inputbox.enable_;
}

void Card::onEnter(const std::string& text)
{
    script_.TimedWith([&](const Handle<Context>& context)
    {
        if (!inputbox.on_enter_.IsEmpty() && inputbox.on_enter_->IsFunction()) {
            Handle<Value> args[1];
            args[0] = String::New(text.data(), text.size());

            inputbox.on_enter_->Call(context->Global(), 1, args);
        }
    });
}

void Card::OnReceiveJSON(const std::string& info_json, const std::string& msg_json)
{
    if (!network_on_receive_json_.IsEmpty() && network_on_receive_json_->IsFunction()) {
        script_.TimedWith(
                [&](const Handle<Context>& context)
                {
                    // HandleScope handle;

                    /*
                    v8::Handle<v8::Value> player = v8::Undefined();
                    if (auto player_manager = manager_accessor_->player_manager().lock()) {
                        if (auto player_ptr = player_manager->GetFromId(user_id)) {
                            player = player_ptr->GetJSObject();
                        }
                    }
                    */

                    Handle<Value> args[2];
                    args[0] = String::New(info_json.data(), info_json.size());
                    args[1] = String::New(msg_json.data(), msg_json.size());

                    network_on_receive_json_->Call(context->Global(), 2, args);
                });
    }
}

void Card::OnLogin(const PlayerPtr& player_ptr)
{
    if (!player.on_login_.IsEmpty() && player.on_login_->IsFunction()) {
        script_.TimedWith(
                [&](const Handle<Context>& context)
                {
                    HandleScope handle;
                    Handle<Value> arg = player_ptr->GetJSObject();
                    player.on_login_->Call(context->Global(), 1, &arg);
                });
    }
}

void Card::OnLogout(const PlayerPtr& player_ptr)
{
    if (!player.on_logout_.IsEmpty() && player.on_logout_->IsFunction()) {
        script_.TimedWith(
                [&](const Handle<Context>& context)
                {
                    HandleScope handle;
                    Handle<Value> arg = player_ptr->GetJSObject();
                    player.on_logout_->Call(context->Global(), 1, &arg);
                });
    }
}

void Card::OnClose()
{
    script_.TimedWith([&](const Handle<Context>& context)
    {
        HandleScope handle;
        auto global = context->Global();
        auto chat = global->Get(String::New("Card"));
        if (chat->IsObject()) {
            auto func = chat->ToObject()->Get(String::New("onClose"));
            if (func->IsFunction()) {
                Handle<Function>::Cast(func)->Call(global, 0, nullptr);
            }
        }
    });
}

void Card::OnModelReload()
{
    if (!model.on_reload_.IsEmpty() && model.on_reload_->IsFunction()) {
        script_.TimedWith(
                [&](const Handle<Context>& context)
                {
                    model.on_reload_->Call(context->Global(), 0, nullptr);
                });
    }
}

void Card::OnMusicReload()
{
    if (!music.on_reload_.IsEmpty() && music.on_reload_->IsFunction()) {
        script_.TimedWith(
                [&](const Handle<Context>& context)
                {
                    music.on_reload_->Call(context->Global(), 0, nullptr);
                });
    }
}

void Card::LoadStorage()
{
    std::string storage_path = std::string(STORAGE_DIR) + "/"
            + unicode::utf82sjis(name_) + ".json";
    if (boost::filesystem::exists(storage_path)) {

        std::ifstream ifs(storage_path);
        std::string script((std::istreambuf_iterator<char>(ifs)),
                std::istreambuf_iterator<char>());

        script_.ParseJSON(script,
                [this](const Handle<Value>& value, const std::string& error) {
                    if (value->IsObject()) {
                        auto storage = value->ToObject();
                        local_storage_ = Persistent<Object>::New(storage);
                    }
                });
    }
}

void Card::SaveStorage()
{

    script_.With(
            [&](const Handle<Context>& context) {
                if (!local_storage_.IsEmpty() && local_storage_->IsObject()) {
                    auto object = local_storage_->ToObject();
                    auto names = object->GetOwnPropertyNames();
                    uint32_t length = names->Length();
                    if (length > 0) {

                        std::string storage_path = std::string(STORAGE_DIR) + "/" + unicode::utf82sjis(name_) + ".json";
                        std::ofstream ofs(storage_path);

                        // データ最大サイズ
                        int max_length = max_local_storage_size;

                        ofs << "{";

                        for (uint32_t i = 0; (i < length && ofs.tellp() < max_length); i++) {
                            auto key = names->Get(i)->ToString();
                            auto value = object->Get(key);
                            if (i > 0) {
                                ofs << ",";
                            }
                            if (value->IsString()) {
                                ofs << "\"" << *v8::String::Utf8Value(key) << "\": ";
                                ofs << "\"" << EscapeString(*v8::String::Utf8Value(value->ToString())) << "\"";
                            } else if (value->IsBoolean()) {
                                ofs << "\"" << *v8::String::Utf8Value(key) << "\": ";
                                ofs << (value->ToBoolean()->BooleanValue() ? "true" : "false");
                            } else if (value->IsNumber()) {
                                ofs << "\"" << *v8::String::Utf8Value(key) << "\": ";
                                ofs << value->ToNumber()->Value();
                            }
                        }

                        ofs << "}";
                    }
                }
            });

}

std::string Card::EscapeString(const std::string& str)
{
    std::string result(str);

    uint32_t pos = 0;
    while ((pos = result.find('\\', pos)) != std::string::npos) {
        result.replace(pos, 1, "\\\\");
        pos += 2;
    }
    while ((pos = result.find('\n', pos)) != std::string::npos) {
        result.replace(pos, 1, "\\n");
        pos += 2;
    }
    while ((pos = result.find('\r', pos)) != std::string::npos) {
        result.replace(pos, 1, "\\r");
        pos += 2;
    }
    while ((pos = result.find('"', pos)) != std::string::npos) {
        result.replace(pos, 1, "\\\"");
        pos += 2;
    }

    return result;
}

std::string Card::author() const {
    return author_;
}

bool Card::autorun() const {
    return autorun_;
}

std::string Card::caption() const {
    return caption_;
}

bool Card::group() const {
    return group_;
}

std::string Card::icon() const {
    return icon_;
}

std::string Card::name() const {
    return name_;
}

std::vector<std::string> Card::scripts() const {
    return scripts_;
}

std::string Card::source_folder() const {
    return source_folder_;
}

bool Card::running() const {
    return running_;
}

int Card::icon_handle() {
    return icon_handle_;
}

bool Card::close_flag() const {
    return close_flag_;
}

std::string Card::input_message() const
{
    return inputbox.message_;
}

int Card::focus_index() const
{
    if (!ui_board_obj_.IsEmpty() && ui_board_obj_->IsObject()) {
        auto ptr = *static_cast<UIBasePtr*>(ui_board_obj_->GetPointerFromInternalField(0));
        return ptr->focus_index();
    }
    return -1;
}

ScriptEnvironment& Card::script()
{
	return script_;
}

void Card::set_max_local_storage_size(int size)
{
    max_local_storage_size = size;
}

void Card::set_ui_board(const UISuperPtr& ui_board)
{
	ui_board_ = ui_board;
}