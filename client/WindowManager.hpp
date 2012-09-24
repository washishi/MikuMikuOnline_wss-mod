//
// WindowManager.hpp
//

#pragma once

#include "ManagerAccessor.hpp"
#include "ResourceManager.hpp"
#include "ui/UISuper.hpp"
#include <vector>

class InputManager;

class WindowManager {
    public:
        WindowManager(const ManagerAccessorPtr& manager_accessor = ManagerAccessorPtr());
        void Init();
        void ProcessInput(InputManager* input);
        void Update();
        void Draw();
		
        void DrawButtons();
        void DrawIcons(const Rect& rect);
		void ProcessInputIcons(const Rect& rect, InputManager* input);

		void AddWindow(const UISuperPtr& window);

    private:
        ManagerAccessorPtr manager_accessor_;

		ImageHandlePtr close_button_image_handle_;
		ImageHandlePtr icon_base_image_handle_;
		std::vector<UISuperWeakPtr> windows_;
		std::vector<UISuperWeakPtr> closed_windows_;
};

typedef std::shared_ptr<WindowManager> WindowManagerPtr;
typedef std::weak_ptr<WindowManager> WindowManagerWeakPtr;