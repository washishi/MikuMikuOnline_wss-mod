//
// Stage.cpp
//

#include "Stage.hpp"
#include "../../common/Logger.hpp"
#include <string>

Stage::Stage(const tstring& model_name) :
    map_handle_(ResourceManager::LoadModelFromName(model_name)),
    map_scale_(map_handle_.property().get<float>("scale", 20.0))
{
    MV1SetScale(map_handle_.handle(), VGet(map_scale_, map_scale_, map_scale_));
    MV1SetupCollInfo(map_handle_.handle(), -1, 256, 256, 256);

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

    auto skymap_name = map_handle_.property().get<std::string>("stage.skydome", unicode::ToString(_T("skydome:入道雲のある風景")));
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
    MV1DrawModel(skymap_handle_.handle());
    MV1DrawModel(map_handle_.handle());
}

float Stage::GetFloorY(const VECTOR& v1, const VECTOR& v2) const
{
    auto coll_info = MV1CollCheck_Line(map_handle_.handle(), -1, v1, v2);
    return coll_info.HitFlag ? coll_info.HitPosition.y : 0;
}

bool Stage::GetFloorY(const VECTOR& v1, const VECTOR& v2, float* y) const
{
    auto coll_info = MV1CollCheck_Line(map_handle_.handle(), -1, v1, v2);
    if (y) {
        *y = coll_info.HitFlag ? coll_info.HitPosition.y : 0;
    }
    return coll_info.HitFlag;
}

std::pair<bool, VECTOR> Stage::FloorExists(const VECTOR& foot_pos, float model_height, float collision_depth_limit) const
{
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
					auto angle = (acos(VDot(slide_coll_info.Dim[j].Normal,current_pos - prev_pos) / (VSize(slide_coll_info.Dim[j].Normal) * VSize(current_pos - prev_pos)))*180.0f)/PHI_F;
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
    auto coll_info = MV1CollCheck_Line(map_handle_.handle(), -1, point, GetCameraPosition());
    auto screen_pos = ConvWorldPosToScreenPos(point);
    return (!coll_info.HitFlag && screen_pos.z > 0.0f && screen_pos.z < 1.0f);
}

void Stage::UpdateSkymapPosition(const VECTOR& pos)
{
    MV1SetPosition(skymap_handle_.handle(), VGet(pos.x, 800, pos.z));
}

const ModelHandle& Stage::map_handle() const
{
    return map_handle_;
}

const std::vector<VECTOR>& Stage::start_points() const
{
    return start_points_;
}

float Stage::map_scale() const
{
    return map_scale_;
}
