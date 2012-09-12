#include "model.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <stdexcept>
#include <cstring>
#include <cmath>
#include <boost/format.hpp>
#include "dx_vector.hpp"
#include "BasicMotion.hpp"
#include "PMDLoader.hpp"
#include "Stage.hpp"
#include "../../common/Logger.hpp"

int KeyChecker::Check()
{
    std::array<char, 256> key_state;
    int result = GetHitKeyStateAll(key_state.data());

    for (size_t i = 0; i < key_state.size(); ++i)
    {
        if (key_state[i] == 1)
        {
            ++key_count_[i];
        }
        else
        {
            key_count_[i] = 0;
        }
    }

    return result;
}

int KeyChecker::GetKeyCount(size_t key_code) const
{
    return key_count_[key_code];
}

const float GameLoop::CAMERA_MIN_RADIUS = 2.0f;
const float GameLoop::CAMERA_MAX_RADIUS = 20.0f;

GameLoop::GameLoop(const StagePtr& stage)
    : stage_(stage),
      camera_default_stat(CameraStatus(7.0f, 0.8f, 0.0f, 20 * PHI_F / 180, false)),
      camera(camera_default_stat)
{
    SetupCamera_Perspective(PHI_F * 60.0f / 180.0f); // 視野角60度
    SetCameraNearFar(1.0f * stage_->map_scale(), 700.0f * stage_->map_scale());
}

int GameLoop::Init(std::shared_ptr<CharacterManager> character_manager)
{
    charmgr_ = character_manager;

    myself_ = std::dynamic_pointer_cast<FieldPlayer>(charmgr_->Get(charmgr_->my_character_id()));

    return 0;
}

int GameLoop::Logic(InputManager* input)
{
    MoveCamera(input);
    myself_->UpdateInput(input);
    for (auto it = charmgr_->GetAll().begin(); it != charmgr_->GetAll().end(); ++it) {
        auto character = *it;
        character.second->Update();
    }
    return 0;
}

int GameLoop::Draw()
{
    //std::cout << "\nDraw" << std::endl;

    FixCameraPosition();

    stage_->Draw();
    //myself_->Draw();
    for (auto it = charmgr_->GetAll().begin(); it != charmgr_->GetAll().end(); ++it) {
        auto character = *it;
        character.second->Draw();
    }

    return 0;
}

FieldPlayerPtr GameLoop::myself() const
{
    return myself_;
}

void GameLoop::FixCameraPosition()
{
    if (!camera.manual_control/* && myself_->current_stat().motion != BasicMotion::STAND*/)
    {
        //　camera = camera_default_stat;
        camera.theta = myself_->current_stat().roty;
    }

    //std::cout << "camera.theta = " << camera.theta << ", roty = " << myself_->current_stat().roty << std::endl;

    const auto target_pos = myself_->current_stat().pos +
        VGet(0, myself_->model_height() * camera.target_height + 0.2f, 0) * stage_->map_scale();
    auto camera_pos = target_pos +
		VGet(cos(camera.phi) * sin(camera.theta),
			sin(camera.phi >= TORADIAN(180.0f) ? camera.phi + TORADIAN(90.0f) : camera.phi ),
			cos(camera.phi) * cos(camera.theta)) * (camera.radius * stage_->map_scale());	

    const auto coll_info = MV1CollCheck_Line(stage_->map_handle().handle(), -1, target_pos, camera_pos);
    static int wallcamera_cnt = 0;
	auto model_coll_size = (myself_->model_height() * (camera.target_height < 0.5f ? 1.0f - camera.target_height : camera.target_height) + 0.25f) * stage_->map_scale();

	if (coll_info.HitFlag &&
		VSize(camera_pos - myself_->current_stat().pos) > CAMERA_MIN_RADIUS + 2.0)
    {
        wallcamera_cnt++;
		if (wallcamera_cnt > 15 && VSize(camera_pos - myself_->current_stat().pos) < CAMERA_MIN_RADIUS + 4.0f + 1.0f) {
			camera_pos.x = coll_info.HitPosition.x;
			camera_pos.z = coll_info.HitPosition.z;
        }else{
			camera_pos = coll_info.HitPosition - VScale(coll_info.HitPosition - target_pos,0.2f);
		}
    } else {
        wallcamera_cnt = 0;
    }
	if(VSize(camera_pos - target_pos) <= model_coll_size)
	{
		camera_pos = target_pos + VScale(camera_pos - target_pos,(model_coll_size)/(VSize(camera_pos - target_pos) <= 0 ? 0.000001f : VSize(camera_pos - target_pos)));
	}

	auto camera_pos_delta = VScale(camera_pos - GetCameraPosition(),(float)0.3);
    if (VSize(camera_pos_delta) > 10) {
        camera_pos_delta = VNorm(camera_pos_delta) * 10;
    }

	SetCameraPositionAndTarget_UpVecY(
		GetCameraPosition() + camera_pos_delta, target_pos);
}

void GameLoop::ResetCameraPosition()
{
    camera = camera_default_stat;
}

void GameLoop::MoveCamera(InputManager* input)
{
	// 非アクティブ時はマウス操作無効
	if (GetActiveFlag() == 0) {
		return;
	}

    const bool prev_right = input->GetPrevMouseRight();
    const bool prev_left = input->GetPrevMouseLeft();
    const bool right = input->GetMouseRight();
    const bool left = input->GetMouseLeft();

    if ((right && !prev_right && !left) || (left && !prev_left && !right))
    {
        // クリックした瞬間
        if (!camera.manual_control)
        {
            camera.manual_control = true;
        }
        camera.manual_control_startpos = input->GetMousePos();
        SetMouseDispFlag(FALSE); // カーソル消去

        if (left)
        {
            myself_->LinkToCamera(&camera.theta);
        }
    }
    else if ((right && prev_right) || (left && prev_left))
    {
        // ドラッグしている
        // assert(camera.manual_control);

        int diff_x, diff_y;
        auto current_pos = input->GetMousePos();
        SetMousePoint(camera.manual_control_startpos.first, camera.manual_control_startpos.second);

        diff_x = current_pos.first - camera.manual_control_startpos.first;
        diff_y = current_pos.second - camera.manual_control_startpos.second;

		// ジャンプ時のみカメラ回転速度 x 3/5 y 2/5
		if(myself_->current_stat().acc.y != 0)
		{
			camera.theta += diff_x * 0.003f;
			camera.phi += diff_y * 0.0005f;
		}else{
			camera.theta += diff_x * 0.005f;
			camera.phi += diff_y * 0.005f;
		}
		if(camera.phi < (-40.0f * PHI_F)/180.0f)
		{
			camera.phi = (-39.9f * PHI_F)/180.0f;
		}else if(camera.phi > (220.0f * PHI_F)/180.0f)
		{
			camera.phi = (219.0f * PHI_F)/180.0f;
		}
    }
    else
    {
        // 左右ボタンを離した瞬間以降
        myself_->UnlinkToCamera();
        if (!GetMouseDispFlag())
        {
            SetMouseDispFlag(TRUE);
        }
        if (camera.manual_control && myself_->any_move())
        {
            camera.manual_control = false;
        }

    }

    if (!right && !left) {
        if (!camera.manual_control &&
                (input->GetGamepadPOVX() != 0 || input->GetGamepadPOVY() != 0)) {
            camera.manual_control = true;
        }

        if (input->GetGamepadPOVX() > 0) {
            camera.theta -= 0.02f;
        } else if (input->GetGamepadPOVX() < 0) {
            camera.theta += 0.02f;
        }
        if (input->GetGamepadPOVY() > 0) {
            camera.phi -= 0.02f;
        } else if (input->GetGamepadPOVY() < 0) {
            camera.phi += 0.02f;
        }
    }

    int wheel = input->GetMouseWheel();
    if (wheel > 0) {
        if (camera.radius > CAMERA_MIN_RADIUS) {
            camera.radius -= 0.5f;
            camera_default_stat.radius = camera.radius;
        }
    } else if (wheel < 0) {
        if (camera.radius < CAMERA_MAX_RADIUS) {
            camera.radius += 0.5f;
            camera_default_stat.radius = camera.radius;
        }
    }

    float pad_rz = input->GetGamepadSlider1();
    if (pad_rz > 0) {
        if (camera.radius > CAMERA_MIN_RADIUS) {
            camera.radius -= 0.5f * pad_rz;
            camera_default_stat.radius = camera.radius;
        }
    } else if (pad_rz < 0) {
        if (camera.radius < CAMERA_MAX_RADIUS) {
            camera.radius -= 0.5f * pad_rz;
            camera_default_stat.radius = camera.radius;
        }
    }
}

