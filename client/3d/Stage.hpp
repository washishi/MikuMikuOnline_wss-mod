//
// Stage.hpp
//

#pragma once

#include <memory>
#include <vector>
#include <DxLib.h>
#include "dx_vector.hpp"
#include "../ResourceManager.hpp"

class Stage {
    public:
        Stage(const tstring& model_path);
        ~Stage();
        void Draw();

        float GetFloorY(const VECTOR& v1, const VECTOR& v2) const;
        bool GetFloorY(const VECTOR& v1, const VECTOR& v2, float* y) const;

        std::pair<bool, VECTOR> FloorExists(const VECTOR& foot_pos, float model_height, float collision_depth_limit) const;
        bool IsFlatFloor(const VECTOR& foot_pos, const VECTOR& direction) const;
        bool FrontCollides(float collision_length, const VECTOR& current_pos, const VECTOR& prev_pos,
                float height_begin, float height_end, size_t num_division) const;

        bool IsVisiblePoint(const VECTOR& point) const;

        const ModelHandle& map_handle() const;
        float map_scale() const;

        const std::vector<VECTOR>& start_points() const;

        void UpdateSkymapPosition(const VECTOR& pos);

    private:
        ModelHandle map_handle_;
        float map_scale_;
        ModelHandle skymap_handle_;

        std::vector<VECTOR> start_points_;
};

typedef std::shared_ptr<Stage> StagePtr;

