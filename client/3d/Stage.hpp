//
// Stage.hpp
//

#pragma once

#include <memory>
#include <vector>
#include <DxLib.h>
#include "dx_vector.hpp"
#include "../ResourceManager.hpp"
#include "../ui/InputBox.hpp"
#include "../ConfigManager.hpp"

struct Channel;
typedef std::shared_ptr<Channel> ChannelPtr;

class Stage {
    public:
        Stage(const ChannelPtr& channel,const ConfigManagerPtr &config_manager);
        ~Stage();
        void Draw();
        void DrawAfter();

        float GetFloorY(const VECTOR& v1, const VECTOR& v2) const;
        bool GetFloorY(const VECTOR& v1, const VECTOR& v2, float* y) const;
		bool CheckWarpPoint(const VECTOR& v);

        std::pair<bool, VECTOR> FloorExists(const VECTOR& foot_pos, float model_height, float collision_depth_limit) const;
        bool IsFlatFloor(const VECTOR& foot_pos, const VECTOR& direction) const;
        std::pair<bool,VECTOR> FrontCollides(float collision_length, const VECTOR& current_pos, const VECTOR& prev_pos,
        float height_begin, float height_end, size_t num_division) const;

        bool IsVisiblePoint(const VECTOR& point) const;
        bool IsNearPoint(const VECTOR& point) const;

        const ModelHandle& map_handle() const;
        float map_scale() const;
        float min_height() const;

        const std::vector<VECTOR>& start_points() const;
		const std::vector<VECTOR>& warp_points() const;
		const bool& host_change_flag() const;

		void SetHostChangeFlag(bool flag);

        void UpdateSkymapPosition(const VECTOR& pos);

		ConfigManagerPtr config_manager() const;

    private:
        ModelHandle map_handle_;
        float map_scale_;
		float min_height_;

		std::unordered_set<int> draw_after_meshes_;

        ModelHandle skymap_handle_;
		std::vector<ModelHandle> warpobj_handles_;

        std::vector<VECTOR> start_points_;
		std::vector<VECTOR> warp_points_;

		bool host_change_flag_;
		ChannelPtr channel_;

		ConfigManagerPtr config_manager_;
};

typedef std::shared_ptr<Stage> StagePtr;

