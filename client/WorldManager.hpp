//
// WorldManager.hpp
//

#pragma once

#include "ManagerAccessor.hpp"
#include "3d/gameloop.hpp"

class InputManager;

class WorldManager {
    public:
        WorldManager(const StagePtr& stage, const ManagerAccessorPtr& manager_accessor = ManagerAccessorPtr());
        void Init();
        void ProcessInput(InputManager* input);
        void Update();
        void Draw();

        bool IsVisiblePoint(const VECTOR& point);
        bool IsNearPoint(const VECTOR& point);
        void ResetCameraPosition();

    public:
        StagePtr stage();
        FieldPlayerPtr myself();

    private:
        ManagerAccessorPtr manager_accessor_;

        StagePtr stage_;
        GameLoop game_loop_;

};

typedef std::shared_ptr<WorldManager> WorldManagerPtr;
typedef std::weak_ptr<WorldManager> WorldManagerWeakPtr;
