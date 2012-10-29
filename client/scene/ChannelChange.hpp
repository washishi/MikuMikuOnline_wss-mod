//
// ChannelChange.h
//

#pragma once

#include <memory>
#include "Base.hpp"
#include "../ManagerAccessor.hpp"

namespace scene {

class ChannelChange : public Base{

	public:
		ChannelChange(
			unsigned char channel,
			const std::shared_ptr<VECTOR>& destination,
			const ManagerAccessorPtr&);
		~ChannelChange();
        void Begin();
        void Update();
		void ProcessInput(InputManager*);
        void Draw();
        void End();

    private:
        ManagerAccessorPtr manager_accessor_;
        CardManagerPtr card_manager_;
        CommandManagerPtr command_manager_;
        AccountManagerPtr account_manager_;
        ConfigManagerPtr config_manager_;
        WorldManagerPtr world_manager_;
		PlayerManagerPtr player_manager_;

		unsigned char channel_;
		int fade_counter_;
		std::shared_ptr<VECTOR> init_position_;
};

}