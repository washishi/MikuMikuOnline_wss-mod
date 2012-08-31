//
// CardManager.cpp
//

#include "CardManager.hpp"
#include "../common/unicode.hpp"
#include "ResourceManager.hpp"
#include <iostream>
#include <algorithm>
#include <boost/filesystem.hpp>
#include "ManagerAccessor.hpp"
#include "CommandManager.hpp"
#include "../common/Logger.hpp"

char CardManager::CARDS_DIR[] = "cards";
char CardManager::INFO_FILE[] = "info.json";

CardManager::CardManager(const ManagerAccessorPtr& manager_accessor) :
manager_accessor_(manager_accessor)
{
    v8::V8::LowMemoryNotification();
}

void CardManager::Load()
{
    using namespace boost::filesystem;
    // info.jsonを検索
    if (exists(CARDS_DIR) && is_directory(CARDS_DIR)) {

    } else {
        Logger::Error(_T("Connot find card directory"));
    }

    directory_iterator end_itr;
    for ( directory_iterator itr(CARDS_DIR); itr != end_itr; ++itr) {

        path itr_path(*itr);

        // サブディレクトリを検索
        if ( is_directory( *itr ) ) {
            path info_file_path = itr_path / path(INFO_FILE);

            // info.jsonの存在を確認する
            if (exists(info_file_path)) {
                ParseInfoFile(info_file_path.string());
            }

        }
    }

    // icon_base_handle_ = ResourceManager::LoadCachedGraph("resources/images/gui/card_icon_base.png");
}

void CardManager::ProcessInput(InputManager* input)
{
    std::sort(cards_.begin(), cards_.end(),
            [](const CardPtr& a, const CardPtr& b) {
                return a->focus_index() < b->focus_index();
            });

    for (auto it = cards_.rbegin(); it != cards_.rend(); ++it) {
        (*it)->ProcessInput(input);
    }
}

void CardManager::Update()
{
    for (auto it = cards_.rbegin(); it != cards_.rend(); ++it) {
        (*it)->Update();
    }
}

void CardManager::Draw()
{
    for (auto it = cards_.begin(); it != cards_.end(); ++it) {
        const CardPtr& card = *it;
        card->Draw();
    }
}

void CardManager::AddCard(const CardPtr& card)
{
    cards_.push_back(card);
}

bool CardManager::IsGUIActive()
{
    //return !dummy_window->focus_front();
    return true;
}

void CardManager::FocusPlayer()
{
    focus_player_flag = true;
}

void CardManager::SendJSON(const std::string& json)
{
    if (auto command_manager = manager_accessor_->command_manager().lock()) {
        command_manager->SendJSON(json);
    }
}

void CardManager::OnReceiveJSON(const std::string& info_json, const std::string& msg_json)
{
    for (auto it = cards_.begin(); it != cards_.end(); ++it) {
        const CardPtr& card = *it;
        card->OnReceiveJSON(info_json, msg_json);
    }
}

void CardManager::OnLogin(const PlayerPtr& player)
{
    for (auto it = cards_.begin(); it != cards_.end(); ++it) {
        const CardPtr& card = *it;
        card->OnLogin(player);
    }
}

void CardManager::OnLogout(const PlayerPtr& player)
{
    for (auto it = cards_.begin(); it != cards_.end(); ++it) {
        const CardPtr& card = *it;
        card->OnLogout(player);
    }
}

std::vector<CardPtr>& CardManager::cards()
{
    return cards_;
}

void CardManager::ParseInfoFile(const std::string& filename)
{
    std::ifstream ifs(filename);
    std::string source_str = std::string((std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());

    std::string name;
    std::string author;
    std::string caption;
    std::string icon;
    std::vector<std::string> scripts;
    bool group = false;
    bool autorun = false;

    ScriptEnvironment script_env;
    script_env.Execute(
            source_str,
            filename,
            [&](const v8::Handle<v8::Value>& result, const std::string& error){
                auto object = result->ToObject();

                // カード名
                {
                    auto obj = object->Get(v8::String::New("name"));
                    if (obj->IsString() && (obj->ToString()->Length() > 0)) {
                        name = *v8::String::Utf8Value(obj->ToString());
                    } else {
                        Logger::Error(_T("Connot read card name: %s"), unicode::ToTString(filename));
                        return;
                    }
                }

                // 作者名
                {
                    auto obj = object->Get(v8::String::New("author"));
                    if (obj->IsString() && (obj->ToString()->Length() > 0)) {
                        author = *v8::String::Utf8Value(obj->ToString());
                    }
                }

                // 説明文
                {
                    auto obj = object->Get(v8::String::New("caption"));
                    if (obj->IsString() && (obj->ToString()->Length() > 0)) {
                        caption = *v8::String::Utf8Value(obj->ToString());
                    }
                }

                // アイコン
                {
                    auto obj = object->Get(v8::String::New("icon"));
                    if (obj->IsString() && (obj->ToString()->Length() > 0)) {
                        icon = *v8::String::Utf8Value(obj->ToString());
                    }
                }

                // スクリプトファイル
                {
                    auto obj = object->Get(v8::String::New("scripts"));
                    if (obj->IsArray()) {
                        auto array = v8::Array::Cast(*obj);
                        auto length = array->Length();
                        for (uint32_t i = 0; i < length; i++) {
                            auto item = array->Get(i);
                            if (item->IsString()) {
                                std::string str = *v8::String::Utf8Value(item->ToString());
                                scripts.push_back(str);
                            }
                        }
                    } else {
                        Logger::Error(_T("Connot read script list: %s"), unicode::ToTString(filename));
                        return;
                    }
                }

                // グループ
                {
                    auto obj = object->Get(v8::String::New("group"));
                    if (obj->IsBoolean()) {
                        group = obj->ToBoolean()->BooleanValue();
                    }
                }

                // 自動実行
                {
                    auto obj = object->Get(v8::String::New("autorun"));
                    if (obj->IsBoolean()) {
                        autorun = obj->ToBoolean()->BooleanValue();
                    }
                }

            }
    );

    auto source_folder = (boost::filesystem::path(filename)).parent_path().string();

    v8::Isolate::Scope scope(v8::Isolate::New());
    auto card = std::make_shared<Card>(
        manager_accessor_,
        source_folder,
        name,
        author,
        caption,
        icon,
        scripts,
        group,
        autorun);
    if (autorun) {
        card->Run();
    }

    cards_.push_back(card);
}

void CardManager::Error(const v8::Handle<v8::Value>& error)
{
    v8::String::Utf8Value exception_str(error);
    std::cout << "Javascript Error >>>" << std::endl;
    std::cout << unicode::utf82sjis(*exception_str) << std::endl;
    std::cout << "<<<" << std::endl;
}

v8::Handle<v8::Object> CardManager::GetGlobal()
{
    HandleScope handle;
    v8::Isolate::Scope scope(v8::Isolate::New());
    v8::Context::Scope scope2(v8::Context::New());
    v8::Handle<v8::Object> obj = Object::New();
    return obj;
}
