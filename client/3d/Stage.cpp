//
// Stage.cpp
//

#include "Stage.hpp"
#include "../../common/Logger.hpp"
#include "../Profiler.hpp"
#include "../CommandManager.hpp"
#include <string>

Stage::Stage(const ChannelPtr& channel,const ConfigManagerPtr &config_manager) :
    map_handle_(ResourceManager::LoadModelFromName(unicode::ToTString(channel->stage))),
    map_scale_(map_handle_.property().get<float>("scale", 20.0)),
	min_height_(map_handle_.property().get<float>("min_height", -200.0)),
	host_change_flag_(false),
	channel_(channel),
	config_manager_(config_manager)
	{
    MV1SetScale(map_handle_.handle(), VGet(map_scale_, map_scale_, map_scale_));
    MV1SetupCollInfo(map_handle_.handle(), -1, 128, 64, 128);// 元の数値は256,256,256

    auto start_points_array = map_handle_.property().get_child("stage.start_points", ptree());
    for (auto it = start_points_array.begin(); it != start_points_array.end(); ++it) {
        float x = it->second.get<float>("x", 0);
        float y = it->second.get<float>("y", 0);
        float z = it->second.get<float>("z", 0);
        start_points_.push_back(VGet(x, y, z));
    }
    if (start_points_.empty()) {
        start_points_.push_back(VGet(0, 0, 0));
    }

	int handle = map_handle_.handle();
	int frame_num = MV1GetMeshNum(handle);
	for (int i = 0; i < frame_num; i++) {
		if (MV1GetMeshSemiTransState(handle, i) == TRUE) {
			draw_after_meshes_.insert(i);
		}
	}

	BOOST_FOREACH(const auto& warp_point, channel_->warp_points) {
		auto handle = ResourceManager::LoadModelFromName(_T("warpobj:ワープオブジェクト"));
		float scale = handle.property().get<float>("scale", 80.0);
		MV1SetPosition(handle.handle(), VGet(warp_point.x, warp_point.y, warp_point.z));
		MV1SetScale(handle.handle(), VGet(scale, scale, scale));
		warpobj_handles_.push_back(handle);
	}

    auto skymap_name = unicode::ToString(_T("skydome:スカイドーム"));
    skymap_handle_ = ResourceManager::LoadModelFromName(unicode::ToTString(skymap_name));

    float skymap_scale = skymap_handle_.property().get<float>("scale", 80.0);
    MV1SetScale(skymap_handle_.handle(), VGet(skymap_scale, skymap_scale, skymap_scale));

}

Stage::~Stage()
{
    MV1TerminateCollInfo(map_handle_.handle());
}

void Stage::Draw()
{
	MMO_PROFILE_FUNCTION;

	int handle = map_handle_.handle();
	int frame_num = MV1GetMeshNum(handle);
	for (int i = 0; i < frame_num; i++) {
		if (draw_after_meshes_.find(i) == draw_after_meshes_.end()) {
			MV1DrawMesh(handle, i);
		}
	}

	auto skymep_rotate = MV1GetRotationXYZ(skymap_handle_.handle());
	skymep_rotate.y += 0.0001;
	MV1SetRotationXYZ(skymap_handle_.handle(), skymep_rotate);

    MV1DrawModel(skymap_handle_.handle());

	BOOST_FOREACH(const auto& warp_point_handle, warpobj_handles_) {
		MV1DrawModel(warp_point_handle.handle());
	}

}

void Stage::DrawAfter()
{
	MMO_PROFILE_FUNCTION;

	int handle = map_handle_.handle();
	int frame_num = MV1GetMeshNum(handle);
	for (int i = 0; i < frame_num; i++) {
		if (draw_after_meshes_.find(i) != draw_after_meshes_.end()) {
			MV1DrawMesh(handle, i);
		}
	}
}

float Stage::GetFloorY(const VECTOR& v1, const VECTOR& v2) const
{
	MMO_PROFILE_FUNCTION;

    auto coll_info = MV1CollCheck_Line(map_handle_.handle(), -1, v1, v2);
    return coll_info.HitFlag ? coll_info.HitPosition.y : 0;
}

bool Stage::GetFloorY(const VECTOR& v1, const VECTOR& v2, float* y) const
{
	MMO_PROFILE_FUNCTION;

    auto coll_info = MV1CollCheck_Line(map_handle_.handle(), -1, v1, v2);
    if (y) {
        *y = coll_info.HitFlag ? coll_info.HitPosition.y : 0;
    }
    return coll_info.HitFlag;
}

bool Stage::CheckWarpPoint(const VECTOR& v)
{
	if(
		warp_points_[0].x == 0 &&
		warp_points_[0].y == 0 &&
		warp_points_[0].z == 0)return false;
	auto it = warp_points_.begin();
	auto vec = VGet(0,200,0);
	VECTOR check_vec = VGet(0,0,0);

	for(it;it != warp_points_.end(); ++it)
	{
		check_vec = v - VGet(it->x,it->y-100,it->z);
		auto s = VSize(check_vec);
		auto t = VSize(vec*cos(VRad(check_vec,vec)));
		if((s * s) - (t * t) <= ( 8 * 8 ))
		{
			return true;
		}
	}
	return false;
}

void Stage::SetHostChangeFlag(bool flag)
{
	host_change_flag_ = flag;
}

std::pair<bool, VECTOR> Stage::FloorExists(const VECTOR& foot_pos, float model_height, float collision_depth_limit) const
{
	MMO_PROFILE_FUNCTION;

    // 床検出
    auto coll_info = MV1CollCheck_Line(map_handle_.handle(), -1,
            foot_pos + VGet(0, 0.5 * model_height * map_scale_, 0),
            foot_pos - VGet(0, collision_depth_limit * map_scale_, 0));
    if (coll_info.HitFlag)
    {
        return std::make_pair(true, coll_info.HitPosition);
    }

    return std::make_pair(false, VGet(0, 0, 0));
}

bool Stage::IsFlatFloor(const VECTOR& foot_pos, const VECTOR& direction) const
{
	MMO_PROFILE_FUNCTION;

    const float interval = 0.01 * map_scale_;
    const auto direction_ = VNorm(VGet(direction.x, 0, direction.z));
    const float threshold = 0.02 * map_scale_;
    const auto threshold_vector = VGet(0, threshold, 0);

    int flat_max = 0, flat_min = 0;
    for (int z = 1; z <= 20; ++z)
    {
        const auto check_pos1 = foot_pos + z * direction_ * interval;
        const auto check_pos2 = foot_pos - z * direction_ * interval;
        const auto coll_info1 = MV1CollCheck_Line(map_handle_.handle(), -1,
                check_pos1 + threshold_vector,
                check_pos1 - threshold_vector);
        const auto coll_info2 = MV1CollCheck_Line(map_handle_.handle(), -1,
                check_pos2 + threshold_vector,
                check_pos2 - threshold_vector);
        if (coll_info1.HitFlag && flat_max == z - 1)
        {
            ++flat_max;
        }
        if (coll_info2.HitFlag && flat_min == -(z - 1))
        {
            // 初めて衝突しなかった
            --flat_min;
        }
    }

    return flat_max - flat_min >= 10;
}

// 移動方向に障害物があるかどうか（当たり判定）
std::pair<bool,VECTOR> Stage::FrontCollides(float collision_length, const VECTOR& current_pos, const VECTOR& prev_pos,
        float height_begin, float height_end, size_t num_division) const
{
	MMO_PROFILE_FUNCTION;

	auto direction = current_pos - prev_pos;
	direction.y = 0; // 水平な進行方向を求める
	const auto collision_vector =
		//VNorm(direction) * (collision_length * map_scale_); // 体中心から15cm前方までの線分で当たり判定
		VAdjustLength(direction, collision_length * map_scale_);

	auto tmp_pos = prev_pos + VGet(0,height_begin,0);
	auto coll_info = MV1CollCheck_Capsule(map_handle_.handle(), -1,
		tmp_pos + collision_vector,
		prev_pos + collision_vector + VGet(0,height_end,0),VSize(collision_vector));

	auto NowPos = prev_pos;

	if (coll_info.HitNum)
	{
		int i = 0;
		for(i = 0;i < coll_info.HitNum; ++i)
		{
			if( coll_info.Dim[i].Normal.y > 0.000002f && coll_info.Dim[i].Normal.y < -0.000002f && 
				coll_info.Dim[i].Position[0].y < tmp_pos.y &&
				coll_info.Dim[i].Position[1].y < tmp_pos.y &&
				coll_info.Dim[i].Position[2].y < tmp_pos.y )continue;
			auto SlideVec = VCross( coll_info.Dim[i].Normal, VCross( current_pos - prev_pos ,coll_info.Dim[i].Normal));
			NowPos += SlideVec;

			const auto slide_collision_vector =
				//VNorm(direction) * (collision_length * map_scale_); // 体中心から15cm前方までの線分で当たり判定
				VAdjustLength(NowPos - prev_pos, collision_length * map_scale_);
			auto flag = false;
			auto slide_coll_info = MV1CollCheck_Capsule(map_handle_.handle(), -1,
				NowPos + VGet(0,height_begin,0),
				NowPos + VGet(0,height_end,0),VSize(collision_vector));
			if(slide_coll_info.HitNum)
			{
				for(int j = 0; j < slide_coll_info.HitNum; ++j)
				{
					auto angle = (acos(VDot(slide_coll_info.Dim[j].Normal,current_pos - prev_pos) / (VSize(slide_coll_info.Dim[j].Normal) * VSize(current_pos - prev_pos)))*180.0f)/DX_PI_F;
					if(angle < 90.0f || angle > 270.0f)
					{
						NowPos += VScale(slide_coll_info.Dim[j].Normal, 0.1f * map_scale_);
						flag = true;
						break;
					}else{
						continue;
					}
				}
				if(!flag)NowPos = prev_pos;
			}
			// 当たり判定情報の後始末
			MV1CollResultPolyDimTerminate(slide_coll_info);
			break;
		}
		// 当たり判定情報の後始末
		MV1CollResultPolyDimTerminate(coll_info);
		return std::make_pair(true,NowPos);;
	}
	// 当たり判定情報の後始末
	MV1CollResultPolyDimTerminate(coll_info);
	return std::make_pair(false,prev_pos);
}

bool Stage::IsVisiblePoint(const VECTOR& point) const
{
	MMO_PROFILE_FUNCTION;

	const auto& camera = GetCameraPosition();

	auto distance = (point.x - camera.x) * (point.x - camera.x) +
		(point.y - camera.y) * (point.y - camera.y) +
		(point.z - camera.z) * (point.z - camera.z);

    auto screen_pos = ConvWorldPosToScreenPos(point);
    return (distance < 500000 && screen_pos.z > 0.0f && screen_pos.z < 1.0f);
}

bool Stage::IsNearPoint(const VECTOR& point) const
{
	MMO_PROFILE_FUNCTION;

	const auto& camera = GetCameraPosition();

	auto distance = (point.x - camera.x) * (point.x - camera.x) +
		(point.y - camera.y) * (point.y - camera.y) +
		(point.z - camera.z) * (point.z - camera.z);

    auto screen_pos = ConvWorldPosToScreenPos(point);
    return (distance < 10000 && screen_pos.z > 0.0f && screen_pos.z < 1.0f);
}

void Stage::UpdateSkymapPosition(const VECTOR& pos)
{
    MV1SetPosition(skymap_handle_.handle(), VGet(pos.x, 800, pos.z));
}

const ModelHandle& Stage::map_handle() const
{
    return map_handle_;
}

const ModelHandle& Stage::skymap_handle() const
{
    return skymap_handle_;
}

const std::vector<VECTOR>& Stage::start_points() const
{
    return start_points_;
}

float Stage::map_scale() const
{
    return map_scale_;
}

float Stage::min_height() const
{
    return min_height_;
}

const bool& Stage::host_change_flag() const
{
	return host_change_flag_;
}

ConfigManagerPtr Stage::config_manager() const
{
	return config_manager_;
}