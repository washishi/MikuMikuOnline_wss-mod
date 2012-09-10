#include "PlayerCharacter.hpp"
#include <memory>
#include <cmath>
#include <iostream>
#include <DxLib.h>
#include "dx_vector.hpp"
#include "BasicMotion.hpp"
#include "MotionPlayer.hpp"
#include "../../common/Logger.hpp"


namespace
{

template <typename T, typename U>
inline bool NearlyEqualRelative(const T& lhs, const T& rhs, const U& ratio)
{
    return abs(lhs - rhs) <= ratio * abs(rhs);
}

}

class PlayerCharacter::Impl
{
public:
    Impl(CharacterDataProvider& data_provider, const StagePtr& stage, const TimerPtr& timer)
        : data_provider_(data_provider),
          model_handle_(-1),
          current_target_pos_(VGet(0, 1000, 0)),
          prev_target_pos_(VGet(0, 1000, 0)),
		  current_target_vec_y_(0.0f),
          current_rot_(VGet(0, 0, 0)),
          prev_rot_(VGet(0, 0, 0)),
		  move_vec_y_(0.0f),
		  jump_flag_(false),
		  flight_duration_ideal_(1.0f),
		  model_height_(1.58f),
		  jump_height_(sqrt(15.0f)*2.0f),
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

		model_height_ = data_provider_.model().property().get<float>("character.height",1.58f);
		flight_duration_ideal_ = sqrt((model_height_*2.0f)/9.8f) + sqrt((model_height_*0.8f)/9.8);	
	}

    void Impl::Draw() const
    {
        // TODO: 離れすぎている場合は描画しない
        MV1DrawModel(model_handle_);
        // DrawSphere3D( VGet(current_target_pos_.x, current_target_pos_.y + 2, current_target_pos_.z) , 1.0f, 32, GetColor( 255,0,0 ), GetColor( 255, 255, 255 ), TRUE ) ;
    }

    void Impl::Update()
    {
		static auto time_now = 0.0f;
		prev_target_pos_ = current_target_pos_;
        const auto current_target_pos = data_provider_.target_position();
		const auto current_target_vec_y_ = data_provider_.vy();
        // if (current_target_pos.y == 0) return;
		if(current_target_pos != prev_target_pos_ )
		{
			time_now = 0.0f;
		}
		

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

            // TODO: Y下方向については重力加速度
            if (distance_to_target > 2.0){
                auto direction = current_target_pos_ - current_pos_;
                const auto diff_pos = VAdjustLength(direction, current_speed_ * timer_->DeltaSec());
                const float roty = atan2(-direction.x, -direction.z);
				const auto time_entire = VSize(direction) / (current_speed_ * (float)timer_->DeltaSec());	// ターゲットまでの移動完了時間
				time_now += timer_->DeltaSec();	// タイマーのカウントアップ

                current_rot_ = VGet(0, roty, 0);
                //std::cout << "set pos to " << MV1GetPosition(model_handle_) + diff_pos << std::endl;

				/*
				const auto jump_result_y =
					jump_height_ * stage_->map_scale() * (time_entire - time_now) * (jump_height_ / 5.0f) * 1.019952f + 
					0.5f * -9.8f * (time_entire - time_now) * (time_entire - time_now) * (1.0f / flight_duration_ideal_) * (jump_height_ / 5.0f) * (jump_height_ / 5.0f);	// 今からジャンプした際の最終的な位置
				*/
				static auto st_acc = -9.8f;
				auto acc = -6.5f;
				auto prediction_vector = 0.0f;
				
				for(acc;acc < -11.0f;acc-=0.1f){
					prediction_vector = jump_height_ * stage_->map_scale() + (acc) * (time_entire - time_now) * (jump_height_ / 5.0f) * (1.0f / flight_duration_ideal_ );
					if( current_target_vec_y_ != 0 && jump_flag_ == false )
					{
						// ターゲット座標ではジャンプを始めている
						if( prediction_vector > current_target_vec_y_ - 1 && prediction_vector < current_target_vec_y_ + 1)
						{
							// 目標座標でベクトルが上向きなら一致、ジャンプを始める
							move_vec_y_ = jump_height_ * stage_->map_scale();
							jump_flag_ = true;
						}//	それ以外はそのまま
					}
				}
				st_acc = acc;

                VECTOR moved_pos;
				moved_pos = current_pos_ + diff_pos;
				if( jump_flag_ == true )
				{
					move_vec_y_ += (st_acc * stage_->map_scale() * timer_->DeltaSec() * (1.0f/flight_duration_ideal_) * (jump_height_ / 5.0f));
					moved_pos.y = current_pos_.y + move_vec_y_ * timer_->DeltaSec() * (jump_height_ / 5.0f);
				}
				Logger::Debug(_T("move_vec_y %.3f"), move_vec_y_);

                // 床へのめり込みを防止
                float floor_y;
				std::pair<bool, VECTOR> floor_coll;
				if(!jump_flag_)
				{
					// 延長線上で接地検査
					floor_coll = stage_->FloorExists(moved_pos,model_height_,8.0f);
				}else if(jump_flag_ && current_pos_.y >= moved_pos.y)
				{
					// 足で接地検査
					floor_coll = stage_->FloorExists(moved_pos,model_height_,0);
				}
                //moved_pos.y = std::max(moved_pos.y, floor_y); // 床にあたっているときは床の方がyが高くなる
				if(!jump_flag_){
					// 登ったり下ったりできる段差の大きさの制限を求める
					static const float y_max_limit_factor = sin(45 * PHI_F / 180);
					static const float y_min_limit_factor = sin(-45 * PHI_F / 180);
					const float y_max_limit = y_max_limit_factor * VSize(diff_pos);
					const float y_min_limit = y_min_limit_factor * VSize(diff_pos);

					// 接地点計算
					//std::cout << "  ground collision check: current pos = " << current_stat_.pos << std::endl;

					auto coll_info = MV1CollCheck_Line(stage_->map_handle().handle(), -1,
						moved_pos + VGet(0, y_max_limit, 0),
						moved_pos + VGet(0, y_min_limit, 0));
					if(coll_info.HitFlag && NearlyEqualRelative(coll_info.HitPosition.y, floor_coll.second.y, 0.001)){
						// 接地可能
						auto diff = coll_info.HitPosition - current_pos_;

						// 角度が急になるほどdiffの長さが大きくなるから、補正する
						if (VSize(diff) > 0)
						{
							moved_pos = current_pos_ + VSize(diff_pos) * VNorm(diff);
						}

					}
					if( floor_coll.first )
					{
						if( floor_coll.second.y < moved_pos.y )
						{
							jump_flag_ = true;
						}
					}
					//move_vec_y_ = 0;
				}else{
					if( moved_pos.y <= current_pos_.y )
					{
						if( floor_coll.first )
						{
							moved_pos = floor_coll.second;
							move_vec_y_ = 0;
						}
					}else{
						// 上昇している

						const auto player_top = VGet(0, model_height_ * stage_->map_scale(), 0);
						auto coll_info = MV1CollCheck_Line(stage_->map_handle().handle(), -1,
							current_pos_ + player_top,
							moved_pos + player_top);
						if (coll_info.HitFlag)
						{
							// 天井に到達した
							// std::cout << "    current collided to ceiling" << std::endl;

							moved_pos = coll_info.HitPosition - player_top;
							move_vec_y_ = -(move_vec_y_ - (st_acc * stage_->map_scale() * timer_->DeltaSec() * (1.0f/flight_duration_ideal_) * (jump_height_ / 5.0f))) * 1.0; // 反射
						}
					}
				}

                data_provider_.set_position(moved_pos);
                data_provider_.set_theta(current_rot_.y);

                MV1SetPosition(model_handle_, moved_pos);
				MV1SetRotationXYZ(model_handle_, current_rot_);

				if (current_speed_ * timer_->DeltaSec() > 0.205f)
				{
					current_motion_ = motion.run_;
				}
				else
				{
					current_motion_ = motion.walk_;
				}
            }else{
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
				additional_motion_.first = false;
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
    VECTOR prev_target_pos_;	// ひとつ前のターゲット
    VECTOR current_pos_;
    VECTOR current_rot_, prev_rot_;
	float current_target_vec_y_;
	float move_vec_y_;
	bool jump_flag_;
	float flight_duration_ideal_;
	float model_height_;
	float jump_height_;
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
