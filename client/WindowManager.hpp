//
// WindowManager.hpp
//

#pragma once

#include "ManagerAccessor.hpp"
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

		void AddWindow(const UISuperPtr& window);

    private:
        ManagerAccessorPtr manager_accessor_;

		std::vector<UISuperWeakPtr> windows_;
};

typedef std::shared_ptr<WindowManager> WindowManagerPtr;
typedef std::weak_ptr<WindowManager> WindowManagerWeakPtr;