//
// PlayerManager.hpp
//

#pragma once

#include <map>
#include <vector>
#include <array>
#include "Player.hpp"
#include "ManagerAccessor.hpp"

class CharacterManager;
class CharacterDataProvider;
class Timer;
typedef std::shared_ptr<Timer> TimerPtr;

class Stage;
typedef std::shared_ptr<Stage> StagePtr;

class InputManager;

class PlayerManager : public std::enable_shared_from_this<PlayerManager> {
    public:
        PlayerManager(const ManagerAccessorPtr& manager_accessor = ManagerAccessorPtr());
        ~PlayerManager();

        void Init();
        void ProcessInput(InputManager* input);
        void Update();
        void Draw();

        uint32_t GetCurrentUserRevision(uint32_t user_id);
        void ApplyRevisionPatch(const std::string& patch);

        PlayerPtr GetFromId(unsigned int user_id);
        PlayerPtr GetMyself();
        std::vector<PlayerPtr> GetAll();

        // プレイヤーの位置を更新
        void UpdatePlayerPosition(unsigned int user_id, const PlayerPosition& pos);

        std::shared_ptr<CharacterManager> charmgr() const;
        std::map<unsigned int, std::unique_ptr<CharacterDataProvider>>& char_data_providers();

        TimerPtr timer() const;

    private:
        TimerPtr timer_;

        ManagerAccessorPtr manager_accessor_;
        std::map<unsigned int, PlayerPtr> login_players_;

        std::shared_ptr<CharacterManager> charmgr_;
        std::map<unsigned int, std::unique_ptr<CharacterDataProvider>> char_data_providers_;

		const std::shared_ptr<StagePtr> stage_ptr_holder_;

        int font_handle_;
        std::array<int, 4> name_tip_image_handle_;

        template <typename CharacterType>
        void AddCharacter(unsigned int character_id, const tstring& model_name);
        void RemoveCharacter(unsigned int user_id);

};

typedef std::shared_ptr<PlayerManager> PlayerManagerPtr;
typedef std::weak_ptr<PlayerManager> PlayerManagerWeakPtr;
