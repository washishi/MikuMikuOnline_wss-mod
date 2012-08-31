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
bool Stage::FrontCollides(float collision_length, const VECTOR& current_pos, const VECTOR& prev_pos,
        float height_begin, float height_end, size_t num_division) const
{
    const auto height_diff = (height_end - height_begin) / num_division;

    auto direction = current_pos - prev_pos;
    direction.y = 0; // 水平な進行方向を求める
    const auto collision_vector =
        //VNorm(direction) * (collision_length * map_scale_); // 体中心から15cm前方までの線分で当たり判定
        VAdjustLength(direction, collision_length * map_scale_);

    for (size_t i = 0; i <= num_division; ++i)
    {
        auto collision_vector_pos = prev_pos + VGet(0, height_begin + height_diff * i, 0);
        auto coll_info = MV1CollCheck_Line(map_handle_.handle(), -1,
                collision_vector_pos,
                collision_vector_pos + collision_vector);
        if (coll_info.HitFlag)
        {
            return true;
        }
    }

    return false;
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
