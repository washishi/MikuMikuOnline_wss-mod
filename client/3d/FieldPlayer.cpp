//
// FieldPlayer.cpp
//

#include "FieldPlayer.hpp"
#include <random>
#include <DxLib.h>
#include "../../common/unicode.hpp"
#include "BasicMotion.hpp"
#include "Stage.hpp"

namespace
{

inline std::ostream& operator <<(std::ostream& os, const std::wstring& s)
{
    os << unicode::ToString(s);
    return os;
}

template <typename T, typename U>
inline bool NearlyEqualRelative(const T& lhs, const T& rhs, const U& ratio)
{
    return abs(lhs - rhs) <= ratio * abs(rhs);
}

} // namespace

FieldPlayer::FieldPlayer(CharacterDataProvider& data_provider, const StagePtr& stage, const TimerPtr& timer)
:       prev_stat_(),
        current_stat_(PlayerStatus( VGet(0, 0, 0), VGet(0, 0, 0), VGet(0, 0, 0),
                0.0f, 0.0f, 0, 0, 1.0f, false)),
        model_height_(0),
		flight_duration_ideal_(1.0f),
		jump_height_(1.0f),
		prev_mouse_pos_y_(0.0f),
        motion_player_(),
		additional_motion_(false,-1),
        timer_(timer),
        model_handle_(),
        stage_(stage),
        any_move_(),
        data_provider_(data_provider),
        camera_roty_(nullptr)
{
}

void FieldPlayer::Draw() const
{
    //std::cout << "roty = " << current_stat_.roty << std::endl;

    MV1SetPosition(model_handle_.handle(), current_stat_.pos);
    MV1SetRotationXYZ(model_handle_.handle(), VGet(0, current_stat_.roty, 0));
    MV1DrawModel(model_handle_.handle());

    // DrawLine3D(current_stat_.pos, current_stat_.pos + VGet(2 * stage_->map_scale(), 0, 0), GetColor(255, 0, 0));
    // DrawLine3D(current_stat_.pos, current_stat_.pos + VGet(0, 2 * stage_->map_scale(), 0), GetColor(0, 255, 0));
    // DrawLine3D(current_stat_.pos, current_stat_.pos + VGet(0, 0, 2 * stage_->map_scale()), GetColor(0, 0, 255));
}

void FieldPlayer::Init(tstring model_name)
{
    LoadModel(model_name);
    ResetPosition();
}

void FieldPlayer::ResetPosition()
{
    const auto& points = stage_->start_points();

    std::mt19937 engine(time(nullptr));
    std::uniform_int_distribution<int> distribution(0, points.size() - 1);

    current_stat_.pos = points[distribution(engine)];
    current_stat_.pos.y = stage_->GetFloorY(current_stat_.pos + VGet(0, 100, 0), current_stat_.pos - VGet(0, 100, 0));
}

void FieldPlayer::RescuePosition()
{
    const auto& points = stage_->start_points();

    const auto& new_pos =
            std::min_element(points.begin(), points.end(),
            [this](const VECTOR& a, const VECTOR& b){
                        return ((a.x - current_stat_.pos.x) * (a.x - current_stat_.pos.x) +
                                (a.y - current_stat_.pos.y) * (a.y - current_stat_.pos.y) +
                                (a.z - current_stat_.pos.z) * (a.z - current_stat_.pos.z)) <
                                ((b.x - current_stat_.pos.x) * (b.x - current_stat_.pos.x) +
                                (b.y - current_stat_.pos.y) * (b.y - current_stat_.pos.y) +
                                (b.z - current_stat_.pos.z) * (b.z - current_stat_.pos.z));
            });

    current_stat_.pos = *new_pos;
    current_stat_.pos.y = stage_->GetFloorY(current_stat_.pos + VGet(0, 100, 0), current_stat_.pos - VGet(0, 100, 0));
	current_stat_.acc.y = 0;
	current_stat_.vel.y = 0;
}

void FieldPlayer::LoadModel(const tstring& name)
{
    if (model_handle_) {
		loading_model_handle_ = ResourceManager::LoadModelFromName(name, true);
	} else {
		SetModel(ResourceManager::LoadModelFromName(name));
	}
	ResourceManager::SetModelEdgeSize(model_handle_.handle());
}

void FieldPlayer::SetModel(const ModelHandle& model)
{
    model_handle_ = model;
    model_height_ = model_handle_.property().get<float>("character.height", 1.58f);
	flight_duration_ideal_ = sqrt((model_height_*2.0f)/9.8f) + sqrt((model_height_*0.8f)/9.8);
	jump_height_ = sqrt(15.0f)*2.0f;
    motion.stand_ = MV1GetAnimIndex(model_handle_.handle(), _T("stand"));
    motion.walk_ = MV1GetAnimIndex(model_handle_.handle(), _T("walk"));
    motion.run_ = MV1GetAnimIndex(model_handle_.handle(), _T("run"));

    motion_player_.reset(new MotionPlayer(model_handle_.handle()));
    motion_player_->Play(motion.stand_, false, 0, -1, FALSE);
}

void FieldPlayer::Update()
{
	if (loading_model_handle_ && loading_model_handle_.CheckLoaded()) {
		SetModel(loading_model_handle_);
		loading_model_handle_ = ModelHandle();
	}

	// 落ちた時に強制復帰
	if (prev_stat_.pos.y < stage_->min_height()) {
		RescuePosition();
	}
    /*
    if (key_checker_.Check() == -1)
    {
        throw std::runtime_error("can't ckeck keyboard");
    }
    */

    prev_stat_ = current_stat_;
    Move();

    if (prev_stat_.motion != current_stat_.motion)
    {
        bool connect_prev = false;
        if ((current_stat_.motion == motion.walk_ ||
                    current_stat_.motion == motion.run_) &&
                (prev_stat_.motion == motion.walk_ ||
                    prev_stat_.motion == motion.run_))
        {
            connect_prev = true;
        }

        motion_player_->Play(current_stat_.motion, connect_prev, 200, -1, false);
	}else if(additional_motion_.first)
	{
		bool connect_prev = true;
		motion_player_->Play(additional_motion_.second, connect_prev, 400, -1, false);
		additional_motion_.first = false;
	}
    // モーション再生時刻更新
    motion_player_->Next(timer_->Delta());

    data_provider_.set_position(current_stat_.pos);
    data_provider_.set_theta(current_stat_.roty);
    data_provider_.set_motion(current_stat_.motion);
    data_provider_.set_vy(current_stat_.vel.y);

    stage_->UpdateSkymapPosition(GetCameraTarget());
}

void FieldPlayer::Move()
{
    // std::cout << "MovePlayer: " << timer.current_time() << std::endl;

    // myself_.prev_statを元にしてmyself_.current_statを計算する
    //current_stat_.pos = prev_stat_.pos + prev_stat_.vel * timer_->DeltaSec();
	current_stat_.pos = [&]()->VECTOR
	{
		VECTOR tmp_vel_ = prev_stat_.vel * timer_->DeltaSec();
		tmp_vel_.y *= (jump_height_ / 5.0f) ; // 5を基準として倍数を求める
		return prev_stat_.pos + tmp_vel_;
	}();
    //current_stat_.vel = prev_stat_.vel + prev_stat_.acc * timer_->DeltaSec();
	current_stat_.vel = [&]()->VECTOR
	{
		VECTOR tmp_acc_ = prev_stat_.acc * timer_->DeltaSec();
		tmp_acc_.y *= (1.0f / flight_duration_ideal_) * (jump_height_ / 5.0f) ; // 5を基準として倍数を求める
		return prev_stat_.vel + tmp_acc_;
	}();
    current_stat_.acc = prev_stat_.acc;
    current_stat_.roty = prev_stat_.roty + prev_stat_.roty_speed * timer_->DeltaSec();

    if (camera_roty_ != nullptr)
    {
        if (prev_stat_.roty_speed != 0)
        {
            *camera_roty_ = current_stat_.roty;
        }
        else
        {
            current_stat_.roty = *camera_roty_;
        }
    }


    // 移動方向に障害物があるか、または床がない場合は移動不可能
	auto front_collides = stage_->FrontCollides(
            0.4, current_stat_.pos, prev_stat_.pos,1.0 * stage_->map_scale(), (model_height_ - 0.1) * stage_->map_scale() ,128);

	if(front_collides.first && current_stat_.acc.y == 0)
	{
		current_stat_.pos = front_collides.second;
        current_stat_.vel.x = current_stat_.vel.z = 0;
	}else if(front_collides.first && current_stat_.acc.y != 0)
	{
		current_stat_.pos.x = front_collides.second.x;
		current_stat_.pos.z = front_collides.second.z;
        current_stat_.vel.x = current_stat_.vel.z = 0;
	}
    // 50mの深さまで床検出
    auto floor_exists = stage_->FloorExists(current_stat_.pos, model_height_, 50);
	if(!floor_exists.first)
	{
		current_stat_.pos.x = front_collides.second.x;
        current_stat_.pos.z = front_collides.second.z;
        current_stat_.vel.x = current_stat_.vel.z = 0;
	}

    // 足が地面にめり込んでいるか
    auto foot_floor_exists = stage_->FloorExists(current_stat_.pos, model_height_, 0);

    const auto pos_diff = current_stat_.pos - prev_stat_.pos;
    const auto pos_diff_length = VSize(pos_diff);

    if (pos_diff_length > 0)
    {

        // 前回キャラが接地していたなら、今回もキャラを地面に接地させる
        if (prev_stat_.acc.y == 0)
        {
            // 前回接地していた
            // std::cout << "  previous on the ground" << std::endl;

			// 登ったり下ったりできる段差の大きさの制限を求める
			static const float y_max_limit_factor = sin(45 * PHI_F / 180);
			static const float y_min_limit_factor = sin(-45 * PHI_F / 180);
			const float y_max_limit = y_max_limit_factor * pos_diff_length;
			const float y_min_limit = y_min_limit_factor * pos_diff_length;

			// 接地点計算
			//std::cout << "  ground collision check: current pos = " << current_stat_.pos << std::endl;

			auto coll_info = MV1CollCheck_Line(stage_->map_handle().handle(), -1,
				current_stat_.pos + VGet(0, y_max_limit, 0),
				current_stat_.pos + VGet(0, y_min_limit, 0));
			if (coll_info.HitFlag && NearlyEqualRelative(coll_info.HitPosition.y, floor_exists.second.y, 0.001))
			{
				// 今回も接地できる
				//std::cout << "    current on the ground" << std::endl;
				auto diff = coll_info.HitPosition - prev_stat_.pos;

				// 角度が急になるほどdiffの長さが大きくなるから、補正する
				if (VSize(diff) > 0)
				{
					current_stat_.pos = prev_stat_.pos + pos_diff_length * VNorm(diff);
				}
			}
			else if (floor_exists.first)
			{
				if (floor_exists.second.y < current_stat_.pos.y)
				{
					// 床はあるし、自分より低い位置なので落ちる
					current_stat_.acc.y = -9.8 * stage_->map_scale();
				}
				else if (floor_exists.second.y < current_stat_.pos.y + 0.6 * stage_->map_scale())
				{
					// 床があり、平らなので登る
					auto delta = prev_stat_.pos - current_stat_.pos;
					delta.y = floor_exists.second.y - current_stat_.pos.y;
					float xz_size = VSize(VGet(delta.x, 0, delta.z));

					if (xz_size > 0)
					{
						// 床の傾斜
						double angle = delta.y / xz_size;
						if (angle < 1.0)
						{
							current_stat_.pos.y = floor_exists.second.y;
							// current_stat_.pos = prev_stat_.pos + (current_stat_.pos - prev_stat_.pos) * (1.0 - angle);
						} else
						{
							current_stat_.pos = prev_stat_.pos;
						}
					}
				}
				else
				{
					// 床があるが、高すぎるので移動不可能
					current_stat_.pos = prev_stat_.pos;
				}
			}
			else
			{
				// 接地できない（移動可能範囲に地面が見つからない）
				current_stat_.pos = prev_stat_.pos;
			}
        }
        else if (prev_stat_.acc.y < 0)
        {
            //std::cout << "  falling now: current pos = " << current_stat_.pos << std::endl;
            // 空中にいる
            if (current_stat_.pos.y <= prev_stat_.pos.y)
            {
                // 落下している
                // std::cout << "  previous falling" << std::endl;

                // 地面に食い込むのを防止する

                if (foot_floor_exists.first)
                {
                    // 地面に到達した
                    // std::cout << "    current on the ground" << std::endl;

                    current_stat_.pos = foot_floor_exists.second;
                    current_stat_.acc.y = 0;
                    current_stat_.vel.y = 0;
                }
            }
            else
            {
                // 上昇している
                // std::cout << "  previous rising" << std::endl;

                const auto player_top = VGet(0, model_height_ * stage_->map_scale(), 0);
                auto coll_info = MV1CollCheck_Line(stage_->map_handle().handle(), -1,
                        prev_stat_.pos + player_top,
                        current_stat_.pos + player_top);
                if (coll_info.HitFlag)
                {
                    // 天井に到達した
                    // std::cout << "    current collided to ceiling" << std::endl;

                    current_stat_.pos = coll_info.HitPosition - player_top;
                    current_stat_.vel.y = -prev_stat_.vel.y * 1.0; // 反射
                }
            }
        }
        else
        {
            std::cerr << "Player's Y-acceleration is a positive number. Is this an error?" << std::endl;
        }

    }

    any_move_ = false; // キャラを動かすための入力があったかどうか

    if (current_stat_.acc.y == 0)
    {
        current_stat_.vel = VGet(0, 0, 0);
    }


    InputFromUser();
}

void FieldPlayer::InputFromUser()
{
    InputManager& input = input_;
    // myself_.current_statを更新する
    const auto roty = prev_stat_.roty;
    const auto move_speed = prev_stat_.motion == motion.walk_ ? 2.0f
        : prev_stat_.motion == motion.run_ ? 8.0f
        : 0;
    auto rot_speed = (prev_stat_.motion == motion.walk_ ? 90.0f
        : prev_stat_.motion == motion.run_ ? 180.0f
        : 90.0f) * PHI_F / 180;

	/*
	auto warp_chk = stage_->CheckWarpPoint(current_stat_.pos);
	if (warp_chk.first && input.GetKeyCount(InputManager::KEYBIND_ENTER) > 0 )
	{
		stage_->SetHostChangeFlag(warp_chk);
	}
	*/

    int move_dir = 0;
    if (input.GetKeyCount(InputManager::KEYBIND_FORWARD) > 0)
    {
        ++move_dir;
    }
    if (input.GetKeyCount(InputManager::KEYBIND_BACK) > 0)
    {
        --move_dir;
    }

    if(input.GetGamepadAnalogY() > 0) {
        --move_dir;
    } else if (input.GetGamepadAnalogY() < 0) {
        ++move_dir;
    }
	// 空中にいるとき、上下の移動で着地地点を操作できるようにする
	int chg_acc = 0;
	if (current_stat_.acc.y == 0)
	{
		prev_mouse_pos_y_ = input.GetMouseY();
	}
	if (current_stat_.acc.y != 0 && input.GetMouseLeft() && input.GetPrevMouseLeft())
	{
		if(prev_mouse_pos_y_ < input.GetMouseY())
		{
			--chg_acc;
		}else if(prev_mouse_pos_y_ > input.GetMouseY()){
			++chg_acc;
		}
	}

    // Shiftで歩きと走りの切り替え
//    if (input.GetKeyCount(InputManager::KEYBIND_CHANGE_SPEED) == 1 ||
//        input.GetKeyCount(InputManager::KEYBIND_CHANGE_SPEED2) == 1)
//    {
//        current_stat_.is_walking = !prev_stat_.is_walking;
//    }

    if (current_stat_.acc.y == 0 && move_dir != 0)
    {
        // 接地しており、かつ移動する
        any_move_ = true;
        current_stat_.vel = VGet(sin(roty), 0, cos(roty)) * (-move_dir * move_speed * stage_->map_scale());
        current_stat_.motion =
            current_stat_.is_walking ? motion.walk_ : motion.run_;
    }
    else if (current_stat_.acc.y != 0)
    {
        // 空中にいる
        any_move_ = true;
        auto acc = 5.0f;
		auto vel = current_stat_.vel + VGet(sin(roty), 0, cos(roty)) * (-move_dir * acc * stage_->map_scale());// * timer_->DeltaSec());// * ((6.0f-current_stat_.vel.y)/(stage_->map_scale()*12.0f))
        vel.y = 0;

        if (VSize(vel) > std::max(move_speed, 1.0f) * stage_->map_scale())
        {
            vel = vel * (std::max(move_speed, 1.0f) * stage_->map_scale() / VSize(vel));
        }
        vel.y = current_stat_.vel.y;
        current_stat_.vel = vel;
		current_stat_.acc.y = 
			(current_stat_.acc.y + (0.2f * chg_acc * stage_->map_scale())) > -6.5f * stage_->map_scale() ? -6.5f * stage_->map_scale() :
			current_stat_.acc.y + (0.2f * chg_acc * stage_->map_scale()) < -11.0f * stage_->map_scale() ? -11.0f * stage_->map_scale() :
			current_stat_.acc.y + (0.2f * chg_acc * stage_->map_scale());
    }
    else
    {
        // 接地しており、移動しない
        current_stat_.motion = motion.stand_;
    }

    int rot_dir = 0;
    if (input.GetKeyCount(InputManager::KEYBIND_RIGHT_TRUN) > 0)
    {
        ++rot_dir;
    }
    if (input.GetKeyCount(InputManager::KEYBIND_LEFT_TURN) > 0)
    {
        --rot_dir;
    }

    if(input.GetGamepadAnalogX() > 0) {
        ++rot_dir;
        rot_speed = fabs(input.GetGamepadAnalogX()) * 90.0f * PHI_F / 180;
    } else if (input.GetGamepadAnalogX() < 0) {
        --rot_dir;
        rot_speed = fabs(input.GetGamepadAnalogX()) * 90.0f * PHI_F / 180;
    }

    if (rot_dir != 0)
    {
        any_move_ = true;
        current_stat_.roty_speed = rot_dir * rot_speed;
    }
    else
    {
        current_stat_.roty_speed = 0;
    }

    if (current_stat_.acc.y == 0 &&
            (input.GetKeyCount(InputManager::KEYBIND_JUMP) > 0 ||
                    input.GetGamepadCount(InputManager::PADBIND_JUMP) > 0))
    {
        any_move_ = true;
        current_stat_.acc.y = -9.8 * stage_->map_scale();
        current_stat_.vel += VGet(0, jump_height_ * stage_->map_scale(), 0);
    }
}

void FieldPlayer::PlayMotion(const tstring& name)
{
	additional_motion_.second = MV1GetAnimIndex(model_handle_.handle(),name.c_str());
	additional_motion_.first = true;
}

const ModelHandle& FieldPlayer::model_handle() const
{
    return model_handle_;
}

const PlayerStatus& FieldPlayer::current_stat() const
{
    return current_stat_;
}

float FieldPlayer::model_height() const
{
    return model_height_;
}

bool FieldPlayer::any_move() const
{
    return any_move_;
}

void FieldPlayer::LinkToCamera(float* roty)
{
    camera_roty_ = roty;
}

void FieldPlayer::UnlinkToCamera()
{
    camera_roty_ = nullptr;
}

void FieldPlayer::UpdateInput(InputManager* input)
{
    input_ = *input;
}
