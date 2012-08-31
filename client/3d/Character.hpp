#pragma once

// 自キャラ、他人のキャラ、NPCなど、あらゆるキャラクタの基底クラス
class Character
{
public:
    virtual ~Character() {}

    virtual void Draw() const = 0;
    virtual void Update() = 0;
};
