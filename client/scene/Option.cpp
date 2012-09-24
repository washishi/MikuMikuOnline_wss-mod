//
// Option.cpp
//

#include "Option.hpp"
#include "../ManagerAccessor.hpp"
#include "../AccountManager.hpp"
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
            _T("resources/images/gui/gui_option_base.png"), 2, 2, 96, 96);

    tab_base_image_handle_ = ResourceManager::LoadCachedDivGraph<3>(
            _T("resources/images/gui/gui_option_tab_base.png"), 1, 3, 128, 32);

}

Option::~Option()
{
}

void Option::Begin()
{
	tabs_.push_back(std::make_shared<StatusTab>(manager_accessor_));
	tabs_.push_back(std::make_shared<CameraTab>(manager_accessor_));
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

BasePtr Option::NextScene()
{
    if (end_count_ > 10) {
        return background_scene_;
    } else {
        return BasePtr();
    }
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

// ステータスタブ

StatusTab::StatusTab(const ManagerAccessorPtr& manager_accessor) :
	OptionTabBase(_T("ステータス"), manager_accessor)
{
	auto account_manager = manager_accessor_->account_manager().lock();
	auto command_manager = manager_accessor_->command_manager().lock();
	auto player_manager = manager_accessor_->player_manager().lock();

	items_.push_back(std::make_shared<TextItem>(_T("ニックネーム"),
		std::make_shared<std::function<tstring(void)>>(
		[account_manager](){
			return unicode::ToTString(account_manager->name());
		}), manager_accessor_));

	items_.push_back(std::make_shared<TextItem>(_T("モデル名"),
		std::make_shared<std::function<tstring(void)>>(
		[account_manager](){
			return unicode::ToTString(account_manager->model_name());
		}), manager_accessor_));

	items_.push_back(std::make_shared<TextItem>(_T("ユーザーID"),
		std::make_shared<std::function<tstring(void)>>(
		[command_manager](){
			return (tformat(_T("%d")) % command_manager->user_id()).str();
		}), manager_accessor_));

	items_.push_back(std::make_shared<TextItem>(_T("ステージ"),
		std::make_shared<std::function<tstring(void)>>(
		[command_manager](){
			return unicode::ToTString(command_manager->stage());
		}), manager_accessor_));

	items_.push_back(std::make_shared<TextItem>(_T("座標"),
		std::make_shared<std::function<tstring(void)>>(
		[player_manager]() -> tstring {
			const auto& pos = player_manager->GetMyself()->position();
			return (tformat(_T("X: %d     Y: %d     Z: %d")) % pos.x % pos.y % pos.z).str();
		}), manager_accessor_));

	items_.push_back(std::make_shared<TextItem>(_T("平均受信量"),
		std::make_shared<std::function<tstring(void)>>(
		[command_manager]() -> tstring {
			return (tformat(_T("%.1lf byte/s")) % command_manager->GetReadByteAverage()).str();
		}), manager_accessor_));

	items_.push_back(std::make_shared<TextItem>(_T("平均送信量"),
		std::make_shared<std::function<tstring(void)>>(
		[command_manager]() -> tstring {
			return (tformat(_T("%.1lf byte/s")) % command_manager->GetWriteByteAverage()).str();
		}), manager_accessor_));
}

void StatusTab::Update()
{
	int left = base_rect_.x + 128 + 64;
	int top = base_rect_.y + 80 + 32;

	BOOST_FOREACH(const auto& item, items_) {
		item->set_base_rect(Rect(left, top));
		top += item->height();
	}
}

void StatusTab::ProcessInput(InputManager* input)
{
	BOOST_FOREACH(const auto& item, items_) {
		item->ProcessInput(input);
	}
}

void StatusTab::Draw()
{
	BOOST_FOREACH(const auto& item, items_) {
		item->Draw();
	}
}


// カメラタブ

CameraTab::CameraTab(const ManagerAccessorPtr& manager_accessor) :
	OptionTabBase(_T("表示設定"), manager_accessor)
{
	items_.push_back(std::make_shared<RadioButtonItem>(_T("ネームタグと吹き出し"),
		"show_nametag", _T("{\"表示する\":1, \"表示しない\":0}"), manager_accessor_));
}

void CameraTab::Update()
{
	int left = base_rect_.x + 128 + 64;
	int top = base_rect_.y + 80 + 32;

	BOOST_FOREACH(const auto& item, items_) {
		item->set_base_rect(Rect(left, top));
		top += item->height();
	}
}

void CameraTab::ProcessInput(InputManager* input)
{
	BOOST_FOREACH(const auto& item, items_) {
		item->ProcessInput(input);
	}
}

void CameraTab::Draw()
{
	BOOST_FOREACH(const auto& item, items_) {
		item->Draw();
	}
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

RadioButtonItem::RadioButtonItem(const tstring& name,
	const std::string& path,
	const tstring& items,
	const ManagerAccessorPtr& manager_accessor) :
		OptionItemBase(manager_accessor),
		name_(name),
		path_(path),
		selecting_index_(0)
{
    selecting_bg_image_handle_ = ResourceManager::LoadCachedDivGraph<3>(
            _T("resources/images/gui/gui_option_selecting_bg.png"), 3, 1, 16, 20);

	ptree item_array;
	read_json(std::stringstream(unicode::ToString(items)), item_array);

	auto account_manager = manager_accessor_->account_manager().lock();
	auto value = account_manager->Get(path_);

	int index = 0;
	BOOST_FOREACH(const auto& item, item_array) {
		auto text = unicode::ToTString(item.first);
		int width = GetDrawStringWidthToHandle(
			text.c_str(), text.size(), ResourceManager::default_font_handle());
		items_.push_back(Item(text, item.second, width));

		if (value == item.second) {
			selecting_index_ = index;
		}

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
	BOOST_FOREACH(const auto& item, items_) {
		int left = item_left;
		int right = left + item.width;

		bool hover = (left <= input->GetMouseX() && input->GetMouseX() <= right
			&& base_rect_.y <= input->GetMouseY() && input->GetMouseY() <= base_rect_.y + height());

		if (hover && input->GetMouseLeftCount() == 1) {
			auto account_manager = manager_accessor_->account_manager().lock();
			account_manager->Set(path_, item.value);
			selecting_index_ = index;
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
	int index = 0;
	BOOST_FOREACH(const auto& item, items_) {

		int left = item_left;
		int right = left + item.width;

		if (index != selecting_index_) {
			SetDrawBlendMode(DX_BLENDMODE_SUB, 40);
		}

		DrawGraph(left - 8, base_rect_.y - 2, *selecting_bg_image_handle_[0], TRUE);
		DrawRectExtendGraphF(left - 8 + 16, base_rect_.y - 2,
								right - 16 + 8, base_rect_.y - 2 + 20,
								0, 0, 1, 20, *selecting_bg_image_handle_[2], TRUE);
		DrawGraph(right - 16 + 8, base_rect_.y - 2, *selecting_bg_image_handle_[2], TRUE);

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

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