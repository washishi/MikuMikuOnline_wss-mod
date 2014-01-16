//
// WindowManager.cpp
//

#include "WindowManager.hpp"
#include "PlayerManager.hpp"
#include "CommandManager.hpp"
#include "CardManager.hpp"
#include "AccountManager.hpp"
#include "ManagerAccessor.hpp"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>

const char* WindowManager::LAYOUT_XML_PATH = "./user/layout.xml";

WindowManager::WindowManager(const ManagerAccessorPtr& manager_accessor) :
manager_accessor_(manager_accessor)
{
	close_button_image_handle_ = 
		ResourceManager::LoadCachedGraph(_T("system/images/gui/gui_close_button.png"));
	icon_base_image_handle_ = 
		ResourceManager::LoadCachedGraph(_T("system/images/gui/gui_icon_base.png"));
}

WindowManager::~WindowManager()
{
}

void WindowManager::Init()
{

}

void WindowManager::ProcessInput(InputManager* input)
{
	closed_windows_.clear();

	auto card_manager = manager_accessor_->card_manager().lock();
	BOOST_FOREACH(const auto& card, card_manager->cards()) {
		if (auto ptr = card->GetWindow()) {

			if (ptr->visible()) {
				card->script().With([&](const Handle<Context>& context)
				{
					ptr->ProcessInput(input);
				});
			}
		}
	}
}

void WindowManager::Update()
{
	auto card_manager = manager_accessor_->card_manager().lock();
	BOOST_FOREACH(const auto& card, card_manager->cards()) {
		if (auto ptr = card->GetWindow()) {
				card->script().With([&](const Handle<Context>& context)
				{
					ptr->Update();
				});
		}
	}
}

void WindowManager::Draw()
{
	auto card_manager = manager_accessor_->card_manager().lock();
	BOOST_FOREACH(const auto& card, card_manager->cards()) {
		if (auto ptr = card->GetWindow()) {
			if (ptr->visible()) {
				card->script().With([&](const Handle<Context>& context)
				{
					ptr->Draw();
				});
			}
		}
	}
}

void WindowManager::DrawButtons()
{
	auto card_manager = manager_accessor_->card_manager().lock();
	BOOST_FOREACH(const auto& card, card_manager->cards()) {
		if (auto ptr = card->GetWindow()) {
			if (ptr->visible()) {
				int x = ptr->absolute_x() - 12;
				int y = ptr->absolute_y() - 12;

				DrawGraph(x, y, *close_button_image_handle_, TRUE);
			}
		}
	}
}


void WindowManager::DrawIcons(const Rect& rect)
{
	int x = 16;
	auto card_manager = manager_accessor_->card_manager().lock();
	BOOST_FOREACH(const auto& card, card_manager->cards()) {

		if (auto ptr = card->GetWindow()) {
			ImageHandlePtr image_handle;
			if (auto custom_icon = ptr->icon_image_handle()) {
				image_handle = custom_icon;
			} else {
				image_handle = icon_base_image_handle_;
			}

			if (ptr->visible()) {
				SetDrawBlendMode(DX_BLENDMODE_SUB, 40);
			}
			DrawGraph(rect.x + x, rect.y + 100 + 8, *image_handle, TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

			x += 40;
		}
	}
}

void WindowManager::ProcessInputIcons(const Rect& rect, InputManager* input)
{
	int x = 16;
	auto card_manager = manager_accessor_->card_manager().lock();
	BOOST_FOREACH(const auto& card, card_manager->cards()) {
		if (auto ptr = card->GetWindow()) {

			{
				int x = ptr->absolute_x() - 12;
				int y = ptr->absolute_y() - 12;

				bool close_hover = (x <= input->GetMouseX() && input->GetMouseX() <= x + 30
					&& y <= input->GetMouseY() && input->GetMouseY() <= y + 30);

				if (close_hover && input->GetMouseLeftCount() == 1) {
					ptr->set_visible(false);
					input->CancelMouseLeft();
				}
			}

			int icon_x = rect.x + x;
			int icon_y = rect.y + 100 + 8;

			bool hover = (icon_x <= input->GetMouseX() && input->GetMouseX() <= icon_x + 32
				&& icon_y <= input->GetMouseY() && input->GetMouseY() <= icon_y + 32);

			if (hover && input->GetMouseLeftCount() == 1) {
				ptr->set_visible(!(ptr->visible()));
				// ※ ここからウィンドウが画面外の場合は初期位置に戻す処理を追加
			    int screen_width, screen_height;
			    GetScreenState(&screen_width, &screen_height, nullptr);

				if(ptr->absolute_x()+ptr->absolute_width()<10  ||
				   ptr->absolute_y()+ptr->absolute_height()<10 ||
				   ptr->absolute_x()>screen_width -10 ||
				   ptr->absolute_y() > screen_height -10){
					ptr->set_offset_x(0);
					ptr->set_offset_y(0);
					ptr->set_offset_width(0);
					ptr->set_offset_height(0);
				}
				// ここまで
				input->CancelMouseLeft();
			}
			x += 40;
		}
	}
}


void WindowManager::AddWindow(const UISuperPtr& window)
{
	windows_.push_back(window);
}

void WindowManager::RestorePosition()
{
	if (boost::filesystem::exists(LAYOUT_XML_PATH)) {
		ptree tree;
		read_xml(LAYOUT_XML_PATH, tree);

		auto card_manager = manager_accessor_->card_manager().lock();
		BOOST_FOREACH(const auto& card, card_manager->cards()) {
			if (auto ptr = card->GetWindow()) {
				auto child = tree.get_child_optional(card->name());
				if (child) {
					ptr->set_top			(child->get<int>("top", 0));
					ptr->set_left			(child->get<int>("left", 0));
					ptr->set_right			(child->get<int>("right", 0));
					ptr->set_bottom			(child->get<int>("bottom", 0));
					ptr->set_offset_x		(child->get<int>("offset_x", 0));
					ptr->set_offset_y		(child->get<int>("offset_y", 0));
					ptr->set_offset_width	(child->get<int>("offset_width", 0));
					ptr->set_offset_height	(child->get<int>("offset_height", 0));
					ptr->set_visible		(child->get<bool>("visible", true));
				}
			}
		}
	}
}

void WindowManager::SavePosition()
{
	ptree tree;
	auto card_manager = manager_accessor_->card_manager().lock();
	BOOST_FOREACH(const auto& card, card_manager->cards()) {
		if (auto ptr = card->GetWindow()) {
			ptree child_tree;
			
			child_tree.put("top",			ptr->top());
			child_tree.put("right",			ptr->right());
			child_tree.put("left",			ptr->left());
			child_tree.put("bottom",		ptr->bottom());
			child_tree.put("offset_x",		ptr->offset_x());
			child_tree.put("offset_y",		ptr->offset_y());
			child_tree.put("offset_width",	ptr->offset_width());
			child_tree.put("offset_height",	ptr->offset_height());
			child_tree.put("visible",		ptr->visible());

			tree.put_child(card->name(), child_tree);
		}
	}

	write_xml(LAYOUT_XML_PATH, tree);
}