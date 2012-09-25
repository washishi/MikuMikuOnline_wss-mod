//
// WindowManager.cpp
//

#include "WindowManager.hpp"
#include "PlayerManager.hpp"
#include "CommandManager.hpp"
#include "CardManager.hpp"
#include "AccountManager.hpp"
#include "ManagerAccessor.hpp"

WindowManager::WindowManager(const ManagerAccessorPtr& manager_accessor) :
manager_accessor_(manager_accessor)
{
	close_button_image_handle_ = 
		ResourceManager::LoadCachedGraph(_T("system/images/gui/gui_close_button.png"));
	icon_base_image_handle_ = 
		ResourceManager::LoadCachedGraph(_T("system/images/gui/gui_icon_base.png"));
}

void WindowManager::Init()
{

}

void WindowManager::ProcessInput(InputManager* input)
{
	closed_windows_.clear();

	BOOST_FOREACH(const auto& window, windows_) {
		if (auto ptr = window.lock()) {
			int x = ptr->absolute_x() - 12;
			int y = ptr->absolute_y() - 12;

			bool hover = (x <= input->GetMouseX() && input->GetMouseX() <= x + 30
				&& y <= input->GetMouseY() && input->GetMouseY() <= y + 30);

			if (hover && input->GetMouseLeftCount() == 1) {
				ptr->set_visible(false);
				input->CancelMouseLeft();
			}

			if (ptr->visible()) {
				ptr->ProcessInput(input);
			} else {
				closed_windows_.push_back(ptr);
			}
		}
	}

	auto card_manager = manager_accessor_->card_manager().lock();
	BOOST_FOREACH(const auto& card, card_manager->cards()) {
		if (auto ptr = card->GetWindow()) {
			int x = ptr->absolute_x() - 12;
			int y = ptr->absolute_y() - 12;

			bool hover = (x <= input->GetMouseX() && input->GetMouseX() <= x + 30
				&& y <= input->GetMouseY() && input->GetMouseY() <= y + 30);

			if (hover && input->GetMouseLeftCount() == 1) {
				ptr->set_visible(false);
				input->CancelMouseLeft();
			}

			if (ptr->visible()) {
				card->script().With([&](const Handle<Context>& context)
				{
					ptr->ProcessInput(input);
				});
			} else {
				closed_windows_.push_back(ptr);
			}
		}
	}
}

void WindowManager::Update()
{
	BOOST_FOREACH(const auto& window, windows_) {
		if (auto ptr = window.lock()) {
			ptr->Update();
		}
	}

	auto card_manager = manager_accessor_->card_manager().lock();
	BOOST_FOREACH(const auto& card, card_manager->cards()) {
		if (auto ptr = card->GetWindow()) {
			ptr->Update();
		}
	}
}

void WindowManager::Draw()
{
	BOOST_FOREACH(const auto& window, windows_) {
		if (auto ptr = window.lock()) {
			if (ptr->visible()) {
				ptr->Draw();
			}
		}
	}

	auto card_manager = manager_accessor_->card_manager().lock();
	BOOST_FOREACH(const auto& card, card_manager->cards()) {
		if (auto ptr = card->GetWindow()) {
			if (ptr->visible()) {
				ptr->Draw();
			}
		}
	}
}

void WindowManager::DrawButtons()
{
	BOOST_FOREACH(const auto& window, windows_) {
		if (auto ptr = window.lock()) {
			if (ptr->visible()) {
				int x = ptr->absolute_x() - 12;
				int y = ptr->absolute_y() - 12;

				DrawGraph(x, y, *close_button_image_handle_, TRUE);
			}
		}
	}

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
	int x = 32;
	BOOST_FOREACH(const auto& window, closed_windows_) {
		if (auto ptr = window.lock()) {
			ImageHandlePtr image_handle;
			if (auto custom_icon = ptr->icon_image_handle()) {
				image_handle = custom_icon;
			} else {
				image_handle = icon_base_image_handle_;
			}
			DrawGraph(rect.x + x, rect.y + 100 + 4, *image_handle, TRUE);
			x += 64;
		}
	}
}

void WindowManager::ProcessInputIcons(const Rect& rect, InputManager* input)
{
	int x = 32;
	BOOST_FOREACH(const auto& window, closed_windows_) {
		if (auto ptr = window.lock()) {

			int icon_x = rect.x + x;
			int icon_y = rect.y + 100 + 4;

			bool hover = (icon_x <= input->GetMouseX() && input->GetMouseX() <= icon_x + 48
				&& icon_y <= input->GetMouseY() && input->GetMouseY() <= icon_y + 48);

			if (hover && input->GetMouseLeftCount() == 1) {
				ptr->set_visible(true);
				input->CancelMouseLeft();
			}
			x += 64;
		}
	}
}


void WindowManager::AddWindow(const UISuperPtr& window)
{
	windows_.push_back(window);
}
