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

}

void WindowManager::Init()
{

}

void WindowManager::ProcessInput(InputManager* input)
{
	BOOST_FOREACH(const auto& window, windows_) {
		if (auto ptr = window.lock()) {
			ptr->ProcessInput(input);
		}
	}

	auto card_manager = manager_accessor_->card_manager().lock();
	BOOST_FOREACH(const auto& card, card_manager->cards()) {
		if (auto ptr = card->GetWindow()) {
			// ptr->ProcessInput(input);
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
			// ptr->Update();
		}
	}
}

void WindowManager::Draw()
{
	BOOST_FOREACH(const auto& window, windows_) {
		if (auto ptr = window.lock()) {
			ptr->Draw();
		}
	}

	auto card_manager = manager_accessor_->card_manager().lock();
	BOOST_FOREACH(const auto& card, card_manager->cards()) {
		if (auto ptr = card->GetWindow()) {
			// ptr->Draw();
		}
	}
}

void WindowManager::AddWindow(const UISuperPtr& window)
{
	windows_.push_back(window);
}
