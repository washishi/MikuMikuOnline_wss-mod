#pragma once

#include <memory>
#include "Character.hpp"
#include "CharacterDataProvider.hpp"
#include "Timer.hpp"
#include "Stage.hpp"

// 他人のキャラ
class PlayerCharacter : public Character
{
public:
    PlayerCharacter(CharacterDataProvider& data_provider, const std::shared_ptr<const StagePtr>& stage, const TimerPtr& timer);
    virtual ~PlayerCharacter();

    void Draw() const;
    void Update();

    void SetModel();
	void PlayMotion(const tstring& name);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;

};
