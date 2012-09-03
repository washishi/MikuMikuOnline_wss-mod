#include "PlayerCharacter.hpp"
#include <memory>
#include <cmath>
#include <iostream>
#include <DxLib.h>
#include "dx_vector.hpp"
#include "BasicMotion.hpp"
#include "MotionPlayer.hpp"
#include "../../common/Logger.hpp"

class PlayerCharacter::Impl
{
public:
    Impl(CharacterDataProvider& data_provider, const StagePtr& stage, const TimerPtr& timer)
        : data_provider_(data_provider),
          model_handle_(-1),
          current_target_pos_(VGet(0, 1000, 0)),
          prev_target_pos_(VGet(0, 1000, 0)),
          current_rot_(VGet(0, 0, 0)),
          prev_rot_(VGet(0, 0, 0)),
          current_motion_(-1),
          prev_motion_(-1),
          current_speed_(0),
          motion_player_(),
		  additional_motion_(false, -1),
          timer_(timer),
          stage_(stage)
    {
        model_handle_ = data_provider_.model().handle();

        motion.stand_ = MV1GetAnimIndex(model_handle_, _T("stand"));
        motion.walk_ = MV1GetAnimIndex(model_handle_, _T("walk"));
        motion.run_ = MV1GetAnimIndex(model_handle_, _T("run"));
        motion_player_.reset(new MotionPlayer(model_handle_));
    }

    void Impl::Draw() const
    {
        // TODO: 離れすぎている場合は描画しない
        MV1DrawModel(model_handle_);
        // DrawSphere3D( VGet(current_target_pos_.x, current_target_pos_.y + 2, current_target_pos_.z) , 1.0f, 32, GetColor( 255,0,0 ), GetColor( 255, 255, 255 ), TRUE ) ;
    }

    void Impl::Update()
    {
        const auto current_target_pos = data_provider_.target_position();
        // if (current_target_pos.y == 0) return;

        current_target_pos_ = current_target_pos;
        current_pos_ = MV1GetPosition(model_handle_);

        const auto distance_to_target = VSize(current_target_pos_ - current_pos_);

        if (distance_to_target < 200) {

            current_speed_ = std::min(80.0, distance_to_target * 3.0);
            // Logger::Debug("current_speed_: %d", current_speed_);

            /*
            if (distance_to_target > 1 * (20 / 1.58) * interval)
            {
                current_motion_ = BasicMotion::RUN;
            }
            else if (distance_to_target > 2.0)
            {
                current_motion_ = BasicMotion::WALK;
            }
            else
            {
                current_motion_ = BasicMotion::STAND;
            }
            */

            //Logger::Debug("distance_to_target: %d %d", distance_to_target, current_speed_ * timer_->DeltaSec());

            if (current_speed_ * timer_->DeltaSec() > 0.2)
            {
                current_motion_ = motion.run_;
            }
            else
            {
                current_motion_ = motion.stand_;
            }

            if (current_motion_ != prev_motion_ && current_motion_ != -1)
            {
                motion_player_->Play(current_motion_, false, 200, -1, false);
                prev_motion_ = current_motion_;
			}else if(additional_motion_.first)
			{
				bool connect_prev = true;
				motion_player_->Play(additional_motion_.second, connect_prev, 200, -1, false);
			}

            // TODO: Y下方向については重力加速度
            if (distance_to_target > 2.0){
                const auto direction = current_target_pos_ - current_pos_;
                const auto diff_pos = VAdjustLength(direction, current_speed_ * timer_->DeltaSec());
                const float roty = atan2(-direction.x, -direction.z);

                current_rot_ = VGet(0, roty, 0);
                //std::cout << "set pos to " << MV1GetPosition(model_handle_) + diff_pos << std::endl;

                // 床へのめり込みを防止
                VECTOR moved_pos = current_pos_ + diff_pos;
                float floor_y;
                for (int i = 1;
                        i < 10 && !(stage_->GetFloorY(moved_pos + VGet(0,10 * i,0), moved_pos + VGet(0,-10 * i,0), &floor_y));
                        i+=2);
                moved_pos.y = std::max(moved_pos.y, floor_y);

                data_provider_.set_position(moved_pos);
                data_provider_.set_theta(current_rot_.y);

                MV1SetPosition(model_handle_, moved_pos);
                MV1SetRotationXYZ(model_handle_, current_rot_);
            }

        } else {
            // 離れすぎている場合は瞬間移動

            data_provider_.set_position(current_target_pos_);
            data_provider_.set_theta(current_rot_.y);

            MV1SetPosition(model_handle_, current_target_pos_);
        }
        motion_player_->Next(timer_->Delta());
    }

	void Impl::PlayMotion(const tstring& name)
	{
		additional_motion_.second = MV1GetAnimIndex(model_handle_,name.c_str());
		additional_motion_.first = true;
	}

private:
    CharacterDataProvider& data_provider_;
    int model_handle_;
    VECTOR current_target_pos_;
    VECTOR prev_target_pos_;
    VECTOR current_pos_;
    VECTOR current_rot_, prev_rot_;
    int current_motion_;
    int prev_motion_;
    float current_speed_;
    std::unique_ptr<MotionPlayer> motion_player_;
	std::pair<bool, int> additional_motion_;
    TimerPtr timer_;
    StagePtr stage_;

    struct {
        int stand_, walk_, run_;
    } motion;
};

PlayerCharacter::PlayerCharacter(CharacterDataProvider& data_provider, const StagePtr& stage, const TimerPtr& timer)
    : impl_(new Impl(data_provider, stage, timer))
{}

PlayerCharacter::~PlayerCharacter()
{}

void PlayerCharacter::Draw() const
{
    impl_->Draw();
}

void PlayerCharacter::Update()
{
    impl_->Update();
}

void PlayerCharacter::PlayMotion(const tstring& name)
{
	impl_->PlayMotion(name);
}
