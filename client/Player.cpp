//
// Player.cpp
//

/**
* @module global
*/

/**
* プレイヤークラス
*
* @class Player
*/

#include "Player.hpp"
#include "PlayerManager.hpp"

using namespace v8;

scene::Main* Player::scene_ = nullptr;

const int Player::BALLOON_BASE_BLOCK_SIZE = 24;
const int Player::BALLOON_OFFSET_X = -50;
const int Player::BALLOON_MIN_WIDTH = 80;

const int Player::NAME_TIP_BASE_BLOCK_SIZE = 12;
const int Player::NAME_TIP_MARGIN_SIZE = 6;

Player::Player(unsigned int id, const PlayerManagerWeakPtr& player_manager) :
player_manager_(player_manager),
font_handle_(ResourceManager::default_font_handle()),
id_(id),
name_(""),
model_name_(""),
login_(false),
revision_(0)
{
    name_tip_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("resources/images/gui/gui_name_tip_base.png"), 2, 2, 12, 12);

    balloon_base_image_handle_ = ResourceManager::LoadCachedDivGraph<9>(
            _T("resources/images/gui/gui_balloon_base.png"), 3, 3, 24, 24);

}

void Player::Update()
{
    if (ballon_content_) {
        ballon_content_->Update();
    }
}

void Player::Draw()
{
    VECTOR pos = {pos_.x, pos_.y + 22, pos_.z};
    auto screen_pos = ConvWorldPosToScreenPos(pos);

    // 画面内に映っているかチェック
    if (screen_pos.z > 0.0f && screen_pos.z < 1.0f) {

        balloon_root_point_.x = (screen_pos.x / 2) * 2;
        balloon_root_point_.y = (screen_pos.y / 2) * 2 - 16;
        balloon_root_point_current_.x += (balloon_root_point_.x - balloon_root_point_current_.x) / 4;
        balloon_root_point_current_.y += (balloon_root_point_.y - balloon_root_point_current_.y) / 4;

//        if (std::abs(balloon_root_point_.x - balloon_root_point_current_.x) <= 2)
//            balloon_root_point_current_.x = balloon_root_point_.x;
//        if (std::abs(balloon_root_point_.y - balloon_root_point_current_.y) <= 2)
//            balloon_root_point_current_.y = balloon_root_point_.y;

        auto name = unicode::ToTString(name_);
        int text_width = GetDrawStringWidthToHandle(name.data(), name.size(), font_handle_);

        name_tip_rect_.width = text_width + NAME_TIP_MARGIN_SIZE * 2;
        name_tip_rect_.height = ResourceManager::default_font_size() + NAME_TIP_MARGIN_SIZE * 2;
        name_tip_rect_.x = balloon_root_point_.x - text_width / 2 - NAME_TIP_MARGIN_SIZE;
        name_tip_rect_.y = balloon_root_point_.y - name_tip_rect_.height;

        if (ballon_content_) {

            auto content_ptr = ballon_content_;

            Rect balloon_rect(
                balloon_root_point_current_.x + BALLOON_OFFSET_X - BALLOON_BASE_BLOCK_SIZE / 2,
                balloon_root_point_current_.y - content_ptr->absolute_height() - NAME_TIP_MARGIN_SIZE * 2 - BALLOON_BASE_BLOCK_SIZE / 2,
                std::max(content_ptr->absolute_width(), BALLOON_MIN_WIDTH + 0) + BALLOON_BASE_BLOCK_SIZE,
                content_ptr->absolute_height() + BALLOON_BASE_BLOCK_SIZE
            );

            content_ptr->set_top(balloon_root_point_current_.y - content_ptr->absolute_height() - NAME_TIP_MARGIN_SIZE * 2);
            content_ptr->set_left(balloon_root_point_current_.x + BALLOON_OFFSET_X);

            name_tip_rect_.y = balloon_root_point_.y - name_tip_rect_.height - content_ptr->absolute_height() - NAME_TIP_MARGIN_SIZE * 2;
            name_tip_rect_.x = balloon_root_point_.x - text_width / 2 + BALLOON_OFFSET_X;

            DrawGraph(balloon_rect.x, balloon_rect.y,
                    *balloon_base_image_handle_[0], TRUE);
            DrawGraph(
                    balloon_rect.x + balloon_rect.width - BALLOON_BASE_BLOCK_SIZE,
                    balloon_rect.y, *balloon_base_image_handle_[2], TRUE);
            DrawGraph(balloon_rect.x,
                    balloon_rect.y + balloon_rect.height - BALLOON_BASE_BLOCK_SIZE,
                    *balloon_base_image_handle_[3], TRUE);
            DrawGraph(
                    balloon_rect.x + balloon_rect.width - BALLOON_BASE_BLOCK_SIZE,
                    balloon_rect.y + balloon_rect.height - BALLOON_BASE_BLOCK_SIZE,
                    *balloon_base_image_handle_[5], TRUE);

            DrawGraph(
                    balloon_root_point_current_.x - BALLOON_BASE_BLOCK_SIZE / 2,
                    balloon_rect.y + balloon_rect.height - BALLOON_BASE_BLOCK_SIZE,
                    *balloon_base_image_handle_[4], TRUE);

            DrawGraph(
                    balloon_root_point_current_.x - BALLOON_BASE_BLOCK_SIZE / 2,
                    balloon_rect.y + balloon_rect.height,
                    *balloon_base_image_handle_[7], TRUE);

            DrawRectExtendGraphF(
                    balloon_rect.x + BALLOON_BASE_BLOCK_SIZE, balloon_rect.y,
                    balloon_rect.x + balloon_rect.width
                            - BALLOON_BASE_BLOCK_SIZE,
                    balloon_rect.y + BALLOON_BASE_BLOCK_SIZE, 0, 0, 1,
                    BALLOON_BASE_BLOCK_SIZE, *balloon_base_image_handle_[2], TRUE);

            DrawRectExtendGraphF(
                    balloon_rect.x + BALLOON_BASE_BLOCK_SIZE,
                    balloon_rect.y + balloon_rect.height
                            - BALLOON_BASE_BLOCK_SIZE,
                            balloon_root_point_current_.x - BALLOON_BASE_BLOCK_SIZE / 2,
                    balloon_rect.y + balloon_rect.height, 0, 0, 1,
                    BALLOON_BASE_BLOCK_SIZE, *balloon_base_image_handle_[5], TRUE);

            DrawRectExtendGraphF(
                    balloon_root_point_current_.x - BALLOON_BASE_BLOCK_SIZE / 2 + BALLOON_BASE_BLOCK_SIZE,
                    balloon_rect.y + balloon_rect.height
                            - BALLOON_BASE_BLOCK_SIZE,
                            balloon_rect.x + balloon_rect.width
                                                        - BALLOON_BASE_BLOCK_SIZE,
                    balloon_rect.y + balloon_rect.height, 0, 0, 1,
                    BALLOON_BASE_BLOCK_SIZE, *balloon_base_image_handle_[5], TRUE);

            DrawRectExtendGraphF(balloon_rect.x,
                    balloon_rect.y + BALLOON_BASE_BLOCK_SIZE,
                    balloon_rect.x + BALLOON_BASE_BLOCK_SIZE,
                    balloon_rect.y + balloon_rect.height
                            - BALLOON_BASE_BLOCK_SIZE, 0, 0,
                    BALLOON_BASE_BLOCK_SIZE, 1, *balloon_base_image_handle_[3], TRUE);

            DrawRectExtendGraphF(
                    balloon_rect.x + balloon_rect.width
                            - BALLOON_BASE_BLOCK_SIZE,
                    balloon_rect.y + BALLOON_BASE_BLOCK_SIZE,
                    balloon_rect.x + balloon_rect.width,
                    balloon_rect.y + balloon_rect.height
                            - BALLOON_BASE_BLOCK_SIZE, 0, 0,
                    BALLOON_BASE_BLOCK_SIZE, 1, *balloon_base_image_handle_[5], TRUE);

            DrawRectExtendGraphF(
                    balloon_rect.x + BALLOON_BASE_BLOCK_SIZE,
                    balloon_rect.y + BALLOON_BASE_BLOCK_SIZE,
                    balloon_rect.x + balloon_rect.width
                            - BALLOON_BASE_BLOCK_SIZE,
                    balloon_rect.y + balloon_rect.height
                            - BALLOON_BASE_BLOCK_SIZE, 0, 0, 1, 1,
                    *balloon_base_image_handle_[5], TRUE);

            content_ptr->Draw();
        }

        name_tip_rect_current_.x += (name_tip_rect_.x - name_tip_rect_current_.x) / 4;
        name_tip_rect_current_.y += (name_tip_rect_.y - name_tip_rect_current_.y) / 4;
        name_tip_rect_current_.width += (name_tip_rect_.width - name_tip_rect_current_.width) / 4;
        name_tip_rect_current_.height += (name_tip_rect_.height - name_tip_rect_current_.height) / 4;

        Rect name_tip_rect(
            name_tip_rect_current_.x,
            name_tip_rect_current_.y,
            name_tip_rect_current_.width,
            name_tip_rect_current_.height
        );

        DrawGraph(name_tip_rect.x, name_tip_rect.y, *name_tip_image_handle_[0],
                TRUE);
        DrawGraph(
                name_tip_rect.x + name_tip_rect.width
                        - NAME_TIP_BASE_BLOCK_SIZE, name_tip_rect.y,
                *name_tip_image_handle_[1], TRUE);
        DrawGraph(name_tip_rect.x,
                name_tip_rect.y + name_tip_rect.height
                        - NAME_TIP_BASE_BLOCK_SIZE, *name_tip_image_handle_[2],
                TRUE);
        DrawGraph(
                name_tip_rect.x + name_tip_rect.width
                        - NAME_TIP_BASE_BLOCK_SIZE,
                name_tip_rect.y + name_tip_rect.height
                        - NAME_TIP_BASE_BLOCK_SIZE, *name_tip_image_handle_[3],
                TRUE);

        DrawRectExtendGraphF(
                name_tip_rect.x + NAME_TIP_BASE_BLOCK_SIZE, name_tip_rect.y,
                name_tip_rect.x + name_tip_rect.width
                        - NAME_TIP_BASE_BLOCK_SIZE,
                name_tip_rect.y + NAME_TIP_BASE_BLOCK_SIZE, 0, 0, 1,
                NAME_TIP_BASE_BLOCK_SIZE, *name_tip_image_handle_[1], TRUE);

        DrawRectExtendGraphF(
                name_tip_rect.x + NAME_TIP_BASE_BLOCK_SIZE,
                name_tip_rect.y + name_tip_rect.height
                        - NAME_TIP_BASE_BLOCK_SIZE,
                name_tip_rect.x + name_tip_rect.width
                        - NAME_TIP_BASE_BLOCK_SIZE,
                name_tip_rect.y + name_tip_rect.height, 0, 0, 1,
                NAME_TIP_BASE_BLOCK_SIZE, *name_tip_image_handle_[3], TRUE);

        DrawRectExtendGraphF(name_tip_rect.x,
                name_tip_rect.y + NAME_TIP_BASE_BLOCK_SIZE,
                name_tip_rect.x + NAME_TIP_BASE_BLOCK_SIZE,
                name_tip_rect.y + name_tip_rect.height
                        - NAME_TIP_BASE_BLOCK_SIZE, 0, 0,
                NAME_TIP_BASE_BLOCK_SIZE, 1, *name_tip_image_handle_[2], TRUE);

        DrawRectExtendGraphF(
                name_tip_rect.x + name_tip_rect.width
                        - NAME_TIP_BASE_BLOCK_SIZE,
                name_tip_rect.y + NAME_TIP_BASE_BLOCK_SIZE,
                name_tip_rect.x + name_tip_rect.width,
                name_tip_rect.y + name_tip_rect.height
                        - NAME_TIP_BASE_BLOCK_SIZE, 0, 0,
                NAME_TIP_BASE_BLOCK_SIZE, 1, *name_tip_image_handle_[3], TRUE);

        DrawRectExtendGraphF(
                name_tip_rect.x + NAME_TIP_BASE_BLOCK_SIZE,
                name_tip_rect.y + NAME_TIP_BASE_BLOCK_SIZE,
                name_tip_rect.x + name_tip_rect.width
                        - NAME_TIP_BASE_BLOCK_SIZE,
                name_tip_rect.y + name_tip_rect.height
                        - NAME_TIP_BASE_BLOCK_SIZE, 0, 0, 1, 1,
                *name_tip_image_handle_[3], TRUE);

        DrawStringToHandle(name_tip_rect.x + NAME_TIP_MARGIN_SIZE, name_tip_rect.y + NAME_TIP_MARGIN_SIZE,
            unicode::ToTString(name).c_str(), GetColor(255, 255, 255), font_handle_);
    }
}

Handle<Object> Player::GetJSObject()
{
    auto object = GetFunctionTemplate()->GetFunction()->CallAsConstructor(0, nullptr)->ToObject();
    auto ptr = new PlayerPtr(shared_from_this());
    object->SetInternalField(0, External::New(ptr));
    return object;
}

Player::~Player()
{
}

Handle<Value> Player::Function_Player_id(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = *static_cast<PlayerPtr*>(args.This()->GetPointerFromInternalField(0));
    assert(self);
    return Integer::NewFromUnsigned(self->id_);
}

Handle<Value> Player::Function_Player_name(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = *static_cast<PlayerPtr*>(args.This()->GetPointerFromInternalField(0));
    assert(self);
    return String::New(self->name_.c_str());
}
Handle<Value> Player::Function_Player_trip(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = *static_cast<PlayerPtr*>(args.This()->GetPointerFromInternalField(0));
    assert(self);
    auto value = self->trip_;
    if (value.size() == 0) {
        return Undefined();
    } else {
        return String::New(value.c_str());
    }
}

Handle<Value> Player::Function_Player_login(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = *static_cast<PlayerPtr*>(args.This()->GetPointerFromInternalField(0));
    assert(self);
    return Boolean::New(self->login_);
}

Handle<Value> Player::Function_Player_setBalloonContent(const Arguments& args)
{

    assert(args.This()->InternalFieldCount() > 0);
    auto self = std::dynamic_pointer_cast<Player>(
            *static_cast<PlayerPtr*>(args.This()->GetPointerFromInternalField(0))
    );
    assert(self);

    if (args.Length() > 0 && args[0]->IsObject()) {
        auto child = args[0]->ToObject();

        if (args.This() != child) {
            UIBasePtr child_ptr = *static_cast<UIBasePtr*>(child->GetPointerFromInternalField(0));
            self->ballon_content_ = child_ptr;
        }
    } else {
        self->ballon_content_.reset();
    }

 /*   assert(args.This()->InternalFieldCount() > 0);
    auto self = *static_cast<PlayerPtr*>(args.This()->GetPointerFromInternalField(0));
    assert(self);
    if (args.Length() > 0) {
        self->ballon_content_ = Persistent<Value>::New(args[0]->);

        if (self->ballon_content_->IsObject()) {
            auto obj = self->ballon_content_->ToObject();
            assert(obj->InternalFieldCount() > 0);

            auto content_ptr = *static_cast<UIBasePtr*>(obj->GetPointerFromInternalField(0));
            content_ptr->Update();
        }
    }*/
    return Undefined();
}

Handle<Value> Player::Function_Player_position(const Arguments& args)
{
    assert(args.This()->InternalFieldCount() > 0);
    auto self = *static_cast<PlayerPtr*>(args.This()->GetPointerFromInternalField(0));
    assert(self);

	const auto pos = self->position();

	HandleScope scope;
	auto array = Array::New(0);
	array->Set(String::New("x"), Number::New(pos.x));
	array->Set(String::New("y"), Number::New(pos.y));
	array->Set(String::New("z"), Number::New(pos.z));
	return array;
}

void Player::RegisterFunctionTemplate(Handle<FunctionTemplate>& func)
{

    auto object = func->InstanceTemplate();

    /**
    * プレイヤーのIDを返します
    *
    * このIDは1から始まる連番値です。
    *
    * @method id
    * @return {Integer} プレイヤーのID
    */
    object->Set(String::New("id"), FunctionTemplate::New(Function_Player_id));


    /**
    * プレイヤーの名前を返します
    *
    * @method name
    * @return {String} プレイヤーの名前
    */
    object->Set(String::New("name"), FunctionTemplate::New(Function_Player_name));


    /**
    * プレイヤーのトリップを返します
    *
    * @method trip
    * @return {String|Undefined}　プレイヤーのトリップ。トリップが指定されていないときは、Undefinedを返します。
    */
    object->Set(String::New("trip"), FunctionTemplate::New(Function_Player_trip));


    /**
    * プレイヤーのログイン状態を返します
    *
    * @method login
    * @return {Boolean}　ログインしている時trueを返します
    */
    object->Set(String::New("login"), FunctionTemplate::New(Function_Player_login));


    /**
    * プレイヤーの頭上の吹き出しに表示する内容を設定します
    *
    * @method setBalloonContent
    * @param {String} text 表示するテキスト
    */
    object->Set(String::New("setBalloonContent"), FunctionTemplate::New(Function_Player_setBalloonContent));

    /**
    * プレイヤーの座標を返します
    *
    * @method position
	* @return {Array}
    */
    object->Set(String::New("position"), FunctionTemplate::New(Function_Player_position));


    ///**
    //* プレイヤーにタグを追加します
    //*
    //* @method addTag
    //* @param {String} tag タグ
    //*/

    ///**
    //* プレイヤーにタグがついていればtrueを追加します
    //*
    //* @method hasTag
    //* @param {String} tag タグ
    //* @return {Boolean} タグがついているかどうか
    //*/

    ///**
    //* プレイヤーからタグを削除します
    //*
    //* @method removeTag
    //* @param {String} tag タグ
    //*/

    ///**
    //* プレイヤーからタグをすべて削除します
    //*
    //* @method clearTag
    //*/

    object->SetInternalFieldCount(1);
}

Handle<FunctionTemplate> Player::GetFunctionTemplate()
{
    auto func = FunctionTemplate::New();
    RegisterFunctionTemplate(func);
    func->SetClassName(String::New("Player"));
    return func;
}

PlayerPtr Player::GetPointerFromObj(Handle<v8::Object> obj)
{
    auto wrap = Local<External>::Cast(obj->GetInternalField(0));
    PlayerPtr self = *static_cast<PlayerPtr*>(wrap->Value());
    return self;
}

void Player::Dispose(Persistent<Value> handle, void* parameter)
{
    auto instance = static_cast<PlayerPtr*>(parameter);
    delete instance;
    handle.Dispose();
}

/*
Handle<Value> Player::position(Local<String> property, const AccessorInfo &info) {
  HandleScope handle_scope;
  auto self = info.Holder();
  auto wrap = Local<External>::Cast(self->GetInternalField(0));
  auto ptr = wrap->Value();
  auto player_ptr = static_cast<Player*>(ptr);
  Handle<Array> array = Array::New(3);
  array->Set(String::New("x"), Number::New(player_ptr->pos_.x));
  array->Set(String::New("y"), Number::New(player_ptr->pos_.y));
  array->Set(String::New("z"), Number::New(player_ptr->pos_.z));
  return array;
}
*/


//Handle<Value> Player::login(const v8::Arguments& args) {
//  auto self = args.Holder();
//  auto wrap = Local<External>::Cast(self->GetInternalField(0));
//  auto ptr = wrap->Value();
//  auto value = static_cast<Player*>(ptr)->login_;
//  return Boolean::New(value);
//}
//
///**
//* プレイヤーの頭上の吹き出しに表示するメッセージを取得します
//*
//* @method baloonText
//* @return {String} text 表示するテキスト
//*/
//Handle<Value> Player::login(const v8::Arguments& args) {
//  auto self = args.Holder();
//  auto wrap = Local<External>::Cast(self->GetInternalField(0));
//  auto ptr = wrap->Value();
//  auto value = static_cast<Player*>(ptr)->login_;
//  return Boolean::New(value);
//}

/**
* プレイヤーをブラックリストに追加します
*
* @method addToBlackList
*/

unsigned int Player::id() const
{
    return id_;
}

void Player::set_id(unsigned int id)
{
    id_ = id;
}

std::string Player::name() const
{
    return name_;
}

void Player::set_name(const std::string& name)
{
    name_ = name;
}

std::string Player::trip() const
{
    return trip_;
}

void Player::set_trip(const std::string& trip)
{
    trip_ = trip;
}

std::string Player::model_name() const
{
    return model_name_;
}

void Player::set_model_name(const std::string& trip)
{
    model_name_ = trip;
}

bool Player::login() const
{
    return login_;
}

void Player::set_login(bool login)
{
    login_ = login;
}

uint32_t Player::revision() const
{
    return revision_;
}

void Player::set_revision(uint32_t revision)
{
    revision_ = revision;
}

const PlayerPosition& Player::position() const
{
    return pos_;
}

void Player::set_position(const PlayerPosition& pos)
{
    pos_ = pos;
}

std::string Player::ip_address() const
{
    return ip_address_;
}

void Player::set_ip_address(const std::string& ip_address)
{
    ip_address_ = ip_address;
}

uint16_t Player::udp_port() const
{
    return udp_port_;
}

void Player::set_udp_port(uint16_t udp_port)
{
    udp_port_ = udp_port;
}
