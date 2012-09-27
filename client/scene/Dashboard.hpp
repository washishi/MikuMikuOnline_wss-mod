//
// Dashboard.hpp
//

#pragma once

#include <boost/property_tree/json_parser.hpp>
#include "Base.hpp"
#include "../ManagerHeader.hpp"
#include "../ManagerAccessor.hpp"
#include "../ResourceManager.hpp"
#include "../ui/UISuper.hpp"

using namespace boost::property_tree;

namespace scene {

class Dashboard : public Base {
    public:
        Dashboard(const ManagerAccessorPtr& manager_accessor,
			const BasePtr& background_scene);
        ~Dashboard();
        void Begin();
        void Update();
		void ProcessInput(InputManager*);
        void Draw();
        void End();

    private:
        ManagerAccessorPtr manager_accessor_;
        ConfigManagerPtr config_manager_;
        CardManagerPtr card_manager_;
        AccountManagerPtr account_manager_;
        WindowManagerPtr window_manager_;

		BasePtr background_scene_;

        int start_count_;
        int end_count_;

		int bg_alpha_;
		Rect base_rect_;
		std::array<ImageHandlePtr,4> base_image_handle_;
		std::array<ImageHandlePtr,3> tab_base_image_handle_;

	private:
		static const int TAB_BLOCK_HEIGHT;
		static const int TAB_BLOCK_WIDTH;
};

}