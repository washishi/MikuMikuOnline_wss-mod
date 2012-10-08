//
// Title.hpp
//

#pragma once

#include <memory>
#include "Base.hpp"
#include "../ManagerAccessor.hpp"
#include "../ui/Input.hpp"
#include "../ui/UILabel.hpp"
#include "../ui/UIButton.hpp"
#include "../Lobby.hpp"

namespace scene {

class Title : public Base {

    public:
        Title(const ManagerAccessorPtr&);
        ~Title();
        void Begin();
        void Update();
		void ProcessInput(InputManager*);
        void Draw();
        void End();

    private:
        void AsyncInitialize();

    private:
        ManagerAccessorPtr manager_accessor_;
        CardManagerPtr card_manager_;
        AccountManagerPtr account_manager_;
        ConfigManagerPtr config_manager_;

        Input input_host_;
        UILabel label_;
        UIButton button_;

		Lobby lobby_;

        bool connect_flag_;
        int screen_count_;

	private:
        std::array<ImageHandlePtr,4> base_image_handle_;
        const static int BASE_BLOCK_SIZE;

};

}
