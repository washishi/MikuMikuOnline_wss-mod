//
// Option.cpp
//

#include "Option.hpp"
#include "Dashboard.hpp"
#include "../ManagerAccessor.hpp"
#include "../ConfigManager.hpp"
#include "../CommandManager.hpp"
#include "../PlayerManager.hpp"
#include "../AccountManager.hpp"

namespace scene {

const int Option::TAB_BLOCK_HEIGHT = 32;
const int Option::TAB_BLOCK_WIDTH = 128;

int Option::selecting_tab_index;

Option::Option(const ManagerAccessorPtr& manager_accessor,
			const BasePtr& background_scene) :
      manager_accessor_(manager_accessor),
      config_manager_(manager_accessor->config_manager().lock()),
      card_manager_(manager_accessor->card_manager().lock()),
      account_manager_(manager_accessor->account_manager().lock()),
	  background_scene_(background_scene),
      start_count_(0),
	  end_count_(0)
{
    manager_accessor_->set_config_manager(config_manager_);
    manager_accessor_->set_card_manager(card_manager_);
    manager_accessor_->set_account_manager(account_manager_);

    base_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("system/images/gui/gui_option_base.png"), 2, 2, 96, 96);

    tab_base_image_handle_ = ResourceManager::LoadCachedDivGraph<3>(
            _T("system/images/gui/gui_option_tab_base.png"), 1, 3, 128, 32);

}

Option::~Option()
{
}

void Option::Begin()
{
	tabs_.push_back(std::make_shared<StatusTab>(manager_accessor_));
	tabs_.push_back(std::make_shared<DisplayTab>(manager_accessor_));
	tabs_.push_back(std::make_shared<InputTab>(manager_accessor_));
	// tabs_.push_back(std::make_shared<OtherTab>(manager_accessor_));
}

void Option::Update()
{
	if (end_count_ > 0) {
		end_count_++;
	} else {
		start_count_++;
	}

	if (background_scene_) {
		background_scene_->Update();
	}

    int screen_width, screen_height;
    GetScreenState(&screen_width, &screen_height, nullptr);

	if (end_count_ > 0) {
		bg_alpha_ = 150 * ((10 - end_count_) / 10.0);
	} else {
		bg_alpha_ = 150 * (start_count_ / 10.0);
	}
	bg_alpha_ = std::min(150, bg_alpha_);

    base_rect_.width = 800;
    base_rect_.height = 480;
    base_rect_.x = (screen_width - base_rect_.width) / 2;

	if (end_count_ > 0) {
		base_rect_.y = -72 - base_rect_.height * (end_count_ / 10.0);
	} else {
		base_rect_.y = -72 - base_rect_.height * ((10 - start_count_) / 10.0);
	}
	base_rect_.y = std::min(-72, base_rect_.y);

	tabs_[selecting_tab_index]->set_base_rect(base_rect_);
	tabs_[selecting_tab_index]->Update();
}

void Option::ProcessInput(InputManager* input)
{
    if (end_count_ > 10) {
        next_scene_ = background_scene_;
	} else if(input->GetKeyCount(KEY_INPUT_F2) == 1) {
		next_scene_ = std::make_shared<Dashboard>(manager_accessor_, background_scene_); 
	}

	bool hover = (base_rect_.x <= input->GetMouseX() && input->GetMouseX() <= base_rect_.x + base_rect_.width
        && base_rect_.y <= input->GetMouseY() && input->GetMouseY() <= base_rect_.y + base_rect_.height);

	if (input->GetKeyCount(KEY_INPUT_F1) == 1 ||
		input->GetKeyCount(KEY_INPUT_ESCAPE) == 1 ||
		(!hover && (input->GetMouseLeftCount() == 1 || input->GetMouseRightCount() == 1))) {
		end_count_++;
	}

	int tab_index = 0;
	BOOST_FOREACH(const auto& tab, tabs_) {

		int x = base_rect_.x - 32;
		int y = base_rect_.y;

		int tab_x = x + 64 - 16;
		int tab_y = y + 80 + tab_index * TAB_BLOCK_HEIGHT + TAB_BLOCK_HEIGHT - 8;
		int tab_width = TAB_BLOCK_WIDTH;
		int tab_height = TAB_BLOCK_HEIGHT;

		bool tab_hover = (tab_x <= input->GetMouseX() && input->GetMouseX() <= tab_x + tab_width
			&& tab_y <= input->GetMouseY() && input->GetMouseY() <= tab_y + tab_height);

		if (tab_hover && input->GetMouseLeftCount() == 1) {
			selecting_tab_index = tab_index;
		}

		tab_index++;
	}
	
	tabs_[selecting_tab_index]->ProcessInput(input);

}

void Option::Draw()
{
	if (background_scene_) {
		background_scene_->Draw();
	}

    int screen_width, screen_height;
    GetScreenState(&screen_width, &screen_height, nullptr);
	
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, bg_alpha_);

	DrawBox(0, 0, screen_width, screen_height, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	int BASE_BLOCK_SIZE = 96;

	int x = base_rect_.x - 32;
	int y = base_rect_.y;
	int width = base_rect_.width + 64;
	int height = base_rect_.height + 32;

    DrawGraph(x, y, *base_image_handle_[0], TRUE);
    DrawGraph(x + width - BASE_BLOCK_SIZE, y, *base_image_handle_[1], TRUE);
    DrawGraph(x, y + height - BASE_BLOCK_SIZE, *base_image_handle_[2], TRUE);
    DrawGraph(x + width - BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE, *base_image_handle_[3], TRUE);

    DrawRectExtendGraphF(x + BASE_BLOCK_SIZE, y,
                         x + width - BASE_BLOCK_SIZE, y + BASE_BLOCK_SIZE,
                         0, 0, 1, BASE_BLOCK_SIZE, *base_image_handle_[1], TRUE);

    DrawRectExtendGraphF(x + BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE,
                         x + width - BASE_BLOCK_SIZE, y + height,
                         0, 0, 1, BASE_BLOCK_SIZE, *base_image_handle_[3], TRUE);

    DrawRectExtendGraphF(x, y + BASE_BLOCK_SIZE,
                         x + BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE,
                         0, 0, BASE_BLOCK_SIZE, 1, *base_image_handle_[2], TRUE);

    DrawRectExtendGraphF(x + width - BASE_BLOCK_SIZE, y + BASE_BLOCK_SIZE,
                         x + width, y + height - BASE_BLOCK_SIZE,
                         0, 0, BASE_BLOCK_SIZE, 1, *base_image_handle_[3], TRUE);

    DrawRectExtendGraphF(x + BASE_BLOCK_SIZE, y + BASE_BLOCK_SIZE,
                         x + width - BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE,
                         0, 0, 1, 1, *base_image_handle_[3], TRUE);

	
	DrawGraph(x + 64 - 16, y + 80 - 8, *tab_base_image_handle_[0], TRUE);
	DrawGraph(x + 64 - 16, y + height - 80 - 8, *tab_base_image_handle_[2], TRUE);

	int tab_index = 0;
	BOOST_FOREACH(const auto& tab, tabs_) {

		int tab_x = x + 64;
		int tab_y = y + 80 + tab_index * TAB_BLOCK_HEIGHT + TAB_BLOCK_HEIGHT;

		if (selecting_tab_index == tab_index) {

			DrawRectExtendGraphF(tab_x - 16, y + 80 - 8 + TAB_BLOCK_HEIGHT,
								 tab_x - 16 + TAB_BLOCK_WIDTH, tab_y - 8,
								 0, TAB_BLOCK_HEIGHT - 1, TAB_BLOCK_WIDTH, 1,
								 *tab_base_image_handle_[0], TRUE);

			DrawGraph(tab_x - 16, tab_y - 8, *tab_base_image_handle_[1], TRUE);

			DrawRectExtendGraphF(tab_x - 16, tab_y - 8 + TAB_BLOCK_HEIGHT,
								 tab_x - 16 + TAB_BLOCK_WIDTH, y + height - 80 - 8,
								 0, 0, TAB_BLOCK_WIDTH, 1, *tab_base_image_handle_[2], TRUE);

		} else {
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
		}

		DrawStringToHandle(tab_x, tab_y, tab->name().c_str(),
			GetColor(0, 0, 0), ResourceManager::default_font_handle());

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		tab_index++;
	}

	tabs_[selecting_tab_index]->Draw();
}

void Option::End()
{

}

OptionTabBase::OptionTabBase(const tstring name, const ManagerAccessorPtr& manager_accessor) :
	name_(name),
	manager_accessor_(manager_accessor)
{

}

tstring OptionTabBase::name() const
{
	return name_;
}

void OptionTabBase::set_base_rect(const Rect& rect)
{
	base_rect_ = rect;
}

void OptionTabBase::Update()
{
	int left = base_rect_.x + 128 + 64;
	int top = base_rect_.y + 80 + 32;

	BOOST_FOREACH(const auto& item, items_) {
		item->set_base_rect(Rect(left, top));
		top += item->height();
	}
}

void OptionTabBase::ProcessInput(InputManager* input)
{
	BOOST_FOREACH(const auto& item, items_) {
		item->ProcessInput(input);
	}
}

void OptionTabBase::Draw()
{
	BOOST_FOREACH(const auto& item, items_) {
		item->Draw();
	}
}

// ステータスタブ

StatusTab::StatusTab(const ManagerAccessorPtr& manager_accessor) :
	OptionTabBase(_LT("option.status.name"), manager_accessor)
{
	auto account_manager = manager_accessor_->account_manager().lock();
	auto command_manager = manager_accessor_->command_manager().lock();
	auto player_manager = manager_accessor_->player_manager().lock();

	items_.push_back(std::make_shared<TextItem>(_LT("option.status.nickname"),
		std::make_shared<std::function<tstring(void)>>(
		[account_manager](){
			return unicode::ToTString(account_manager->name());
		}), manager_accessor_));

	items_.push_back(std::make_shared<TextItem>(_LT("option.status.trip"),
		std::make_shared<std::function<tstring(void)>>(
		[player_manager](){
			return unicode::ToTString(player_manager->GetMyself()->trip());
		}), manager_accessor_));

	items_.push_back(std::make_shared<TextItem>(_LT("option.status.modelname"),
		std::make_shared<std::function<tstring(void)>>(
		[account_manager](){
			return unicode::ToTString(account_manager->model_name());
		}), manager_accessor_));

	items_.push_back(std::make_shared<TextItem>(_LT("option.status.channel"),
		std::make_shared<std::function<tstring(void)>>(
		[player_manager](){
			return (tformat(_T("%d")) % static_cast<int>(player_manager->GetMyself()->channel())).str();
		}), manager_accessor_));

	items_.push_back(std::make_shared<TextItem>(_LT("option.status.user_id"),
		std::make_shared<std::function<tstring(void)>>(
		[command_manager](){
			return (tformat(_T("%d")) % command_manager->user_id()).str();
		}), manager_accessor_));

	items_.push_back(std::make_shared<TextItem>(_LT("option.status.stage"),
		std::make_shared<std::function<tstring(void)>>(
		[command_manager](){
			return unicode::ToTString(command_manager->stage());
		}), manager_accessor_));

	items_.push_back(std::make_shared<TextItem>(_LT("option.status.position"),
		std::make_shared<std::function<tstring(void)>>(
		[player_manager]() -> tstring {
			const auto& pos = player_manager->GetMyself()->position();
			return (tformat(_T("X: %d     Y: %d     Z: %d")) % pos.x % pos.y % pos.z).str();
		}), manager_accessor_));

	items_.push_back(std::make_shared<TextItem>(_LT("option.status.receive_average"),
		std::make_shared<std::function<tstring(void)>>(
		[command_manager]() -> tstring {
			return (tformat(_T("%.1lf byte/s")) % command_manager->GetReadByteAverage()).str();
		}), manager_accessor_));

	items_.push_back(std::make_shared<TextItem>(_LT("option.status.send_average"),
		std::make_shared<std::function<tstring(void)>>(
		[command_manager]() -> tstring {
			return (tformat(_T("%.1lf byte/s")) % command_manager->GetWriteByteAverage()).str();
		}), manager_accessor_));
}


// 表示設定タブ

DisplayTab::DisplayTab(const ManagerAccessorPtr& manager_accessor) :
	OptionTabBase(_LT("option.display.name"), manager_accessor)
{
	items_.push_back(std::make_shared<RadioButtonItem>(
		_LT("option.display.show_nametag"),
		_LT("option.display.show_nametag_json"),
		std::make_shared<RadioButtonItemGetter>(
		[manager_accessor]() -> int{
			auto config_manager = 
				manager_accessor->config_manager().lock();
			return config_manager->show_nametag();
		}),
		std::make_shared<RadioButtonItemSetter>(
		[manager_accessor](int value){
			if (auto config_manager = 
				manager_accessor->config_manager().lock()) {
				return config_manager->set_show_nametag(value);
			}
		}),
		manager_accessor_));


	items_.push_back(std::make_shared<RadioButtonItem>(
		_LT("option.display.show_modelname"),
		_LT("option.display.show_modelname_json"),
		std::make_shared<RadioButtonItemGetter>(
		[manager_accessor]() -> int{
			auto config_manager = 
				manager_accessor->config_manager().lock();
			return config_manager->show_modelname();
		}),
		std::make_shared<RadioButtonItemSetter>(
		[manager_accessor](int value){
			if (auto config_manager = 
				manager_accessor->config_manager().lock()) {
				return config_manager->set_show_modelname(value);
			}
		}),
		manager_accessor_));
}

// 操作設定タブ

InputTab::InputTab(const ManagerAccessorPtr& manager_accessor) :
	OptionTabBase(_LT("option.input.name"), manager_accessor)
{
	items_.push_back(std::make_shared<RadioButtonItem>(
		_LT("option.input.gamepad_type"),
		_LT("option.input.gamepad_type_json"),
		std::make_shared<RadioButtonItemGetter>(
		[manager_accessor]() -> int{
			auto config_manager = 
				manager_accessor->config_manager().lock();
			return config_manager->gamepad_type();
		}),
		std::make_shared<RadioButtonItemSetter>(
		[manager_accessor](int value){
			if (auto config_manager = 
				manager_accessor->config_manager().lock()) {
				InputManager::SetGamepadType(value);
				config_manager->set_gamepad_type(value);
			}
		}),
		manager_accessor_));

	items_.push_back(std::make_shared<RadioButtonItem>(
		_LT("option.input.camera_direction"),
		_LT("option.input.camera_direction_json"),
		std::make_shared<RadioButtonItemGetter>(
		[manager_accessor]() -> int{
			auto config_manager = 
				manager_accessor->config_manager().lock();
			return config_manager->camera_direction();
		}),
		std::make_shared<RadioButtonItemSetter>(
		[manager_accessor](int value){
			if (auto config_manager = 
				manager_accessor->config_manager().lock()) {
				InputManager::SetGamepadType(value);
				config_manager->set_camera_direction(value);
			}
		}),
		manager_accessor_));
}

// その他設定タブ

OtherTab::OtherTab(const ManagerAccessorPtr& manager_accessor) :
	OptionTabBase(_LT("option.others.name"), manager_accessor)
{
	//items_.push_back(std::make_shared<RadioButtonItem>(
	//	_T("棒読みちゃんと連携"),
	//	_T("{\"オフ\":0,\"オン\":1}"),
	//	std::make_shared<RadioButtonItemGetter>(
	//	[manager_accessor]() -> int{
	//		auto config_manager = 
	//			manager_accessor->config_manager().lock();
	//		return config_manager->bouyomi_chan();
	//	}),
	//	std::make_shared<RadioButtonItemSetter>(
	//	[manager_accessor](int value){
	//		if (auto config_manager = 
	//			manager_accessor->config_manager().lock()) {
	//			return config_manager->set_bouyomi_chan(value);
	//		}
	//	}),
	//	manager_accessor_));
}

OptionItemBase::OptionItemBase(const ManagerAccessorPtr& manager_accessor) :
	manager_accessor_(manager_accessor)
{

}

void OptionItemBase::set_base_rect(const Rect& rect)
{
	base_rect_ = rect;
}

TextItem::TextItem(const tstring& name,
	const TextItemCallbackPtr& callback,
	const ManagerAccessorPtr& manager_accessor) :
		OptionItemBase(manager_accessor),
		name_(name),
		callback_(callback)
{

}

void TextItem::Update()
{

}

void TextItem::ProcessInput(InputManager* input)
{

}

void TextItem::Draw()
{
	DrawStringToHandle(base_rect_.x, base_rect_.y,
		name_.c_str(), GetColor(58, 133, 86), ResourceManager::default_font_handle());

	if (callback_) {
		DrawStringToHandle(base_rect_.x + 192, base_rect_.y,
			(*callback_)().c_str(),
			GetColor(0, 0, 0), ResourceManager::default_font_handle());
	}
}

int TextItem::height() const
{
	return 24;
}


DescriptionItem::DescriptionItem(const tstring& text,
	const ManagerAccessorPtr& manager_accessor) :
		OptionItemBase(manager_accessor),
		text_(text)
{

}

void DescriptionItem::Update()
{

}

void DescriptionItem::ProcessInput(InputManager* input)
{

}

void DescriptionItem::Draw()
{
	DrawStringToHandle(base_rect_.x + 192, base_rect_.y,
		text_.c_str(),
		GetColor(180, 180, 180), ResourceManager::default_font_handle());
}

int DescriptionItem::height() const
{
	return 24;
}


RadioButtonItem::RadioButtonItem(const tstring& name,
	const tstring& items,
	const RadioButtonItemGetterPtr& getter,
	const RadioButtonItemSetterPtr& setter,
	const ManagerAccessorPtr& manager_accessor) :
		OptionItemBase(manager_accessor),
		name_(name),
		getter_(getter),
		setter_(setter),
		selecting_index_(0)
{
    selecting_bg_image_handle_ = ResourceManager::LoadCachedDivGraph<3>(
            _T("system/images/gui/gui_option_selecting_bg.png"), 3, 1, 16, 24);

	ptree item_array;
	read_json(std::stringstream(unicode::ToString(items)), item_array);

	int index = 0;
	BOOST_FOREACH(const auto& item, item_array) {
		auto text = unicode::ToTString(item.first);
		int width = GetDrawStringWidthToHandle(
			text.c_str(), text.size(), ResourceManager::default_font_handle());
		items_.push_back(Item(text, item.second.get_value<int>(), width));

		index++;
	}
}

void RadioButtonItem::Update()
{

}

void RadioButtonItem::ProcessInput(InputManager* input)
{
	int item_left = base_rect_.x + 192;
	int index = 0;
	BOOST_FOREACH(auto& item, items_) {
		int left = item_left;
		int right = left + item.width;

		item.hover = (left <= input->GetMouseX() && input->GetMouseX() <= right
			&& base_rect_.y <= input->GetMouseY() && input->GetMouseY() <= base_rect_.y + height());

		if (item.hover && input->GetMouseLeftCount() == 1) {
			(*setter_)(item.value);
		}

		item_left += item.width + 20;
		index++;
	}
}

void RadioButtonItem::Draw()
{
	DrawStringToHandle(base_rect_.x, base_rect_.y,
		name_.c_str(), GetColor(58, 133, 86), ResourceManager::default_font_handle());

	int item_left = base_rect_.x + 192;
	int value = (*getter_)();
	int index = 0;
	BOOST_FOREACH(const auto& item, items_) {

		int left = item_left;
		int right = left + item.width;

		if (value != item.value) {
			if (item.hover) {
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
			} else {
				SetDrawBlendMode(DX_BLENDMODE_SUB, 40);
			}
		}

		DrawGraph(left - 8, base_rect_.y - 4, *selecting_bg_image_handle_[0], TRUE);
		DrawRectExtendGraphF(left - 8 + 16, base_rect_.y - 4,
								right - 16 + 8, base_rect_.y + 20,
								0, 0, 1, 24, *selecting_bg_image_handle_[2], TRUE);
		DrawGraph(right - 16 + 8, base_rect_.y - 4, *selecting_bg_image_handle_[2], TRUE);

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 4);

		DrawStringToHandle(left, base_rect_.y,
			item.name.c_str(),
			GetColor(0, 0, 0), ResourceManager::default_font_handle());
		

		item_left += item.width + 20;

		index++;
	}
}

int RadioButtonItem::height() const
{
	return 24;
}

}