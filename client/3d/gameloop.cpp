#include "gameloop.hpp"

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
#include "../ConfigManager.hpp"
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
const float GameLoop::CAMERA_MAX_RADIUS = 40.0f;

GameLoop::GameLoop(const ManagerAccessorPtr& manager_accessor, const StagePtr& stage)
    : stage_(stage),
	  manager_accessor_(manager_accessor),
      camera_default_stat(CameraStatus(7.0f, 0.8f, 0.0f, 20 * DX_PI_F / 180, false)),
      camera(camera_default_stat),
	  light( 34.4100f, 135.2900f)
{
    SetupCamera_Perspective(DX_PI_F * 60.0f / 180.0f); // 視野角60度
    SetCameraNearFar(1.0f * stage_->map_scale(), 700.0f * stage_->map_scale());
}

int GameLoop::Init(std::shared_ptr<CharacterManager> character_manager)
{
    charmgr_ = character_manager;

    myself_ = std::dynamic_pointer_cast<FieldPlayer>(charmgr_->Get(charmgr_->my_character_id()));

	light.Init();

    return 0;
}

int GameLoop::ProcessInput(InputManager* input)
{
    MoveCamera(input);
    myself_->UpdateInput(input);
    return 0;
}

int GameLoop::Update()
{
    for (auto it = charmgr_->GetAll().begin(); it != charmgr_->GetAll().end(); ++it) {
        auto character = *it;
        character.second->Update();
    }

	light.Update();

    return 0;
}

int GameLoop::Draw()
{
    //std::cout << "\nDraw" << std::endl;

    FixCameraPosition();

    stage_->Draw();

    for (auto it = charmgr_->GetAll().begin(); it != charmgr_->GetAll().end(); ++it) {
        auto character = *it;
        character.second->Draw();
	}

    stage_->DrawAfter();

    return 0;
}

FieldPlayerPtr GameLoop::myself() const
{
    return myself_;
}

void GameLoop::FixCameraPosition()
{
	auto config_manager = manager_accessor_->config_manager().lock();

    if (config_manager->camera_direction() == 0 &&
		!camera.manual_control/* && myself_->current_stat().motion != BasicMotion::STAND*/)
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
	// if (VSize(camera_pos_delta) > 10) {
	//    camera_pos_delta = VNorm(camera_pos_delta) * 10;
	//}

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
		if(camera.phi < (-40.0f * DX_PI_F)/180.0f)
		{
			camera.phi = (-39.9f * DX_PI_F)/180.0f;
		}else if(camera.phi > (220.0f * DX_PI_F)/180.0f)
		{
			camera.phi = (219.0f * DX_PI_F)/180.0f;
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

    float pad_rz = input->GetGamepadManagedAnalogRy();
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

void LightStatus::Init()
{
	time_ = boost::posix_time::second_clock::local_time();
	//time_ = boost::posix_time::ptime(time_.date(),boost::posix_time::time_duration(6,10,0));
	time_differ_ = time_ - boost::posix_time::second_clock::universal_time();

	auto time_differ = time_differ_.hours();
	auto date = time_.date();
	auto yy = date.year();
	auto mm = date.month();
	auto dd = date.day();
	//auto h = time_.time_of_day().hours();
	//auto m = time_.time_of_day().minutes();
	//auto s = time_.time_of_day().seconds();

	// boost::gregorian::date::modjulian_day()は負の値になるときにバグるので使用しない

	auto julius_year		=	JuliusYear(yy,mm,dd - 1,23,59,0,time_differ);
	auto sidereal_hour	=	SiderealHour(julius_year,23,59,0,calc_location_.x,time_differ);
	auto solar_position1	=	SolarPosition1(julius_year);
	auto solar_position2	=	SolarPosition2(julius_year);
	auto solar_position3	=	SolarPosition3(julius_year);
	auto solar_position4	=	SolarPosition4(julius_year);
	auto altitude	=	SolarAltitude(calc_location_.y,sidereal_hour,solar_position3,solar_position4);
	auto direction	=	SolarDirection(calc_location_.y,sidereal_hour,solar_position3,solar_position4);

	float calc_altitude,calc_direction,tt,t4;

	bool flag = true;

	for( int hh = 0; hh < 24; ++hh ){
		if( !flag )
			break;
		for( int m = 0; m < 60; ++m ){
			julius_year		=	JuliusYear(yy,mm,dd,hh,m,0,time_differ);
			sidereal_hour	=	SiderealHour(julius_year,hh,m,0,calc_location_.x,time_differ);
			solar_position1	=	SolarPosition1(julius_year);
			solar_position2	=	SolarPosition2(julius_year);
			solar_position3	=	SolarPosition3(julius_year);
			solar_position4	=	SolarPosition4(julius_year);
			calc_altitude	=	SolarAltitude(calc_location_.y,sidereal_hour,solar_position3,solar_position4);
			calc_direction	=	SolarDirection(calc_location_.y,sidereal_hour,solar_position3,solar_position4);
			//tt = SolarApparentAltitude1(elevation_,solar_position2);
			t4 = SolarApparentAltitude2(elevation_,solar_position2);
			if( ( altitude < t4 ) && ( calc_altitude > t4 ) ){
				moning_glow_ = boost::posix_time::ptime(time_.date(),boost::posix_time::time_duration(hh,m,0));
				//hh += 8;	// 冬至の際の日出～日没間時間は８時間程度なのでその分スキップする
			}
			if( ( altitude > t4 ) && ( calc_altitude < t4 ) ){
				sunset_ = boost::posix_time::ptime(time_.date(),boost::posix_time::time_duration(hh,m,0));
				flag = false;
				break;
			}
			altitude = calc_altitude;
			direction = calc_direction;
		}	
	}

	//light_handle_ = CreateDirLightHandle(VSub(VGet(0,0,0),light_pos_));

	SetGlobalAmbientLight(GetColorF(0,0,0,0));
	//SetLightDifColorHandle( light_handle_ ,GetColorF(0.603f,0.603f,0.603f,0));
	//SetLightSpcColorHandle( light_handle_ ,GetColorF(0.603f,0.603f,0.603f,0));
	//SetLightAmbColorHandle( light_handle_ ,GetColorF(1,1,1,0));
	SetLightDifColor( GetColorF(0.603f,0.603f,0.603f,0));
	SetLightSpcColor( GetColorF(0.603f,0.603f,0.603f,0));
	SetLightAmbColor( GetColorF(1,1,1,0));

	Calc();

}


void LightStatus::Calc()
{
	time_ = boost::posix_time::second_clock::local_time();
	time_differ_ = time_ - boost::posix_time::second_clock::universal_time();

	auto time_differ = time_differ_.hours();
	auto date = time_.date();
	auto yy = date.year();
	auto mm = date.month();
	auto dd = date.day();
	auto h = time_.time_of_day().hours();
	auto m = time_.time_of_day().minutes();
	auto s = time_.time_of_day().seconds();

	// boost::gregorian::date::modjulian_day()は負の値になるときにバグるので使用しない

	auto julius_year		=	JuliusYear(yy,mm,dd,h,m,s,time_differ);
	auto sidereal_hour	=	SiderealHour(julius_year,h,m,s,calc_location_.x,time_differ);
	auto solar_position1	=	SolarPosition1(julius_year);
	auto solar_position2	=	SolarPosition2(julius_year);
	auto solar_position3	=	SolarPosition3(julius_year);
	auto solar_position4	=	SolarPosition4(julius_year);
	auto altitude	=	SolarAltitude(calc_location_.y,sidereal_hour,solar_position3,solar_position4);
	auto direction	=	SolarDirection(calc_location_.y,sidereal_hour,solar_position3,solar_position4);
	//auto tt = SolarApparentAltitude1(elevation_,solar_position2);
	//auto t4 = SolarApparentAltitude2(elevation_,solar_position2);

	light_distance_ = 100;

	VectorRotationX(&light_pos_,&VGet(0,0,-light_distance_),altitude);
	VectorRotationY(&light_pos_,&light_pos_,-direction);

	SetGrobalAmbientColorMatchToTime();
}

void LightStatus::Update()
{
	if(frame_count_++ % ( 60 * 60 ) == 0){
		Calc();
		frame_count_ = 1;	// オーバーフロー対策
		//SetLightDirectionHandle(light_handle_,VSub(VGet(0,0,0),light_pos_));
		SetLightPosition(light_pos_);
		SetLightDirection(VSub(VGet(0,0,0),light_pos_));
	}
}

float LightStatus::JuliusYear(int yy,int mm,int dd,int h,int m,int s,int time_differ)
{
	yy -= 2000;
	if( mm <= 2 ) {
		m += 12;
		--yy;
	}
	auto k = 365 * yy + 30 * mm + dd - 33.5f - time_differ / 24.0f + floor(3 * (mm + 1) / 5.0f)
		+ floor(yy / 4.0f) - floor(yy / 100.0f) + floor(yy / 400.0f);
	k += ((s / 60.0f + m) / 60.0f + h) / 24.0f;
	k += (65 + yy) / 86400.0f;
	return k / 365.25f;
}

float LightStatus::SiderealHour(float julius_year,int h,int m,int s,float longitude,int time_differ)
{
    auto d = ((s / 60.0f + m) / 60.0f + h) / 24.0f; // elapsed hour (from 0:00 a.m.)
    auto th = 100.4606 + 360.007700536 * julius_year + 0.00000003879 * julius_year * julius_year - 15 * time_differ;
    th += longitude + 360 * d;
    while (th >= 360) { th -= 360; }
    while (th < 0) { th += 360; }
    return static_cast<float>(th);
}


float LightStatus::SolarPosition1(float julius_year)
{
	auto l = 280.4603 + 360.00769 * julius_year
		+ (1.9146 - 0.00005 * julius_year) * sin(TORADIAN(357.538 + 359.991 * julius_year))
		+ 0.0200 * sin(TORADIAN(355.05 + 719.981 * julius_year))
		+ 0.0048 * sin(TORADIAN(234.95 + 19.341 * julius_year))
		+ 0.0020 * sin(TORADIAN(247.1 + 329.640 * julius_year))
		+ 0.0018 * sin(TORADIAN(297.8 + 4452.67 * julius_year))
		+ 0.0018 * sin(TORADIAN(251.3 + 0.20 * julius_year))
		+ 0.0015 * sin(TORADIAN(343.2 + 450.37 * julius_year))
		+ 0.0013 * sin(TORADIAN(81.4 + 225.18 * julius_year))
		+ 0.0008 * sin(TORADIAN(132.5 + 659.29 * julius_year))
		+ 0.0007 * sin(TORADIAN(153.3 + 90.38 * julius_year))
		+ 0.0007 * sin(TORADIAN(206.8 + 30.35 * julius_year))
		+ 0.0006 * sin(TORADIAN(29.8 + 337.18 * julius_year))
		+ 0.0005 * sin(TORADIAN(207.4 + 1.50 * julius_year))
		+ 0.0005 * sin(TORADIAN(291.2 + 22.81 * julius_year))
		+ 0.0004 * sin(TORADIAN(234.9 + 315.56 * julius_year))
		+ 0.0004 * sin(TORADIAN(157.3 + 299.30 * julius_year))
		+ 0.0004 * sin(TORADIAN(21.1 + 720.02 * julius_year))
		+ 0.0003 * sin(TORADIAN(352.5 + 1079.97 * julius_year))
		+ 0.0003 * sin(TORADIAN(329.7 + 44.43 * julius_year));
	while (l >= 360) { l -= 360; }
	while (l < 0) { l += 360; }
	return l;
}
float LightStatus::SolarPosition2(float julius_year)
{
	auto r = (0.007256 - 0.0000002 * julius_year) * sin(TORADIAN(267.54 + 359.991 * julius_year))
		+ 0.000091 * sin(TORADIAN(265.1 + 719.98 * julius_year))
		+ 0.000030 * sin(TORADIAN(90.0))
		+ 0.000013 * sin(TORADIAN(27.8 + 4452.67 * julius_year))
		+ 0.000007 * sin(TORADIAN(254 + 450.4 * julius_year))
		+ 0.000007 * sin(TORADIAN(156 + 329.6 * julius_year));
	r = pow(10, r);
	return r;
}

float LightStatus::SolarPosition3(float julius_year)
{
	auto ls = SolarPosition1(julius_year);
	auto ep = 23.439291 - 0.000130042 * julius_year;
	auto al = atan(tan(TORADIAN(ls)) * cos(TORADIAN(ep))) * 180 / DX_PI_F;
	if ((ls >= 0) && (ls < 180)) {
		while (al < 0) { al += 180; }
		while (al >= 180) { al -= 180; } 
	}
	else {
		while (al < 180) { al += 180; }
		while (al >= 360) { al -= 180; } 
	}
	return al;
}

float LightStatus::SolarPosition4(float julius_year)
{
	auto ls = SolarPosition1(julius_year);
	auto ep = 23.439291 - 0.000130042 * julius_year;
	auto dl = asin(sin(TORADIAN(ls)) * sin(TORADIAN(ep))) * 180 / DX_PI_F;
	return dl;
}

float LightStatus::SolarAltitude(float latitude,float sidereal_hour,float solar_declination,float right_ascension)
{
	long float a = (sidereal_hour - solar_declination) * (DX_PI_F / 180.0L);
	long float h = sin(TORADIAN(right_ascension)) * sin(TORADIAN(latitude)) + cos(TORADIAN(right_ascension)) * cos(TORADIAN(latitude)) * cos(a);
	h = asin(h) * 180.0 / DX_PI_F;
	return h;
}

float LightStatus::SolarDirection(float latitude,float sidereal_hour,float solar_declination,float right_ascension)
{
	auto t = sidereal_hour - solar_declination;
	auto dc = -cos(TORADIAN(right_ascension)) * sin(TORADIAN(t));
	auto dm = sin(TORADIAN(right_ascension)) * sin(TORADIAN(latitude)) - cos(TORADIAN(right_ascension)) * cos(TORADIAN(latitude)) * cos(TORADIAN(t));
	float dr = 0;
	if (dm == 0) {
		auto st = sin(TORADIAN(t));
		if (st > 0) dr = -90;
		if (st == 0) dr = 9999;
		if (st < 0) dr = 90;
	}
	else {
		dr = atan(dc / dm) * 180 / DX_PI_F;
		if (dm < 0) dr += 180;
	}
	if (dr < 0) dr += 360;
	return dr;
}

float LightStatus::SolarApparentAltitude1(float altitude,float distance)
{
	auto e = 0.035333333 * sqrt(altitude);
	auto p = 0.002442818 / distance;
	return p - e;
}

float LightStatus::SolarApparentAltitude2(float altitude,float distance)
{
	auto s = 0.266994444 / distance;
	auto r = 0.585555555;
	auto k = SolarApparentAltitude1(altitude, distance) - s - r;
	return k;
}

void LightStatus::SetGrobalAmbientColorMatchToTime()
{
	auto h = time_.time_of_day().hours();
	auto m = time_.time_of_day().minutes();
	auto s = time_.time_of_day().seconds();

	auto mh = moning_glow_.time_of_day().hours();
	auto mm = moning_glow_.time_of_day().minutes();
	auto ms = moning_glow_.time_of_day().seconds();

	auto sh = sunset_.time_of_day().hours();
	auto sm = sunset_.time_of_day().minutes();
	auto ss = sunset_.time_of_day().seconds();

	auto CumulativeSeconds = [](int hours, int minutes, int seconds)->int{
		return seconds + 60 * minutes + 3600 * hours;
	};

	if( CumulativeSeconds( mh, mm, ms ) - ( 60 * 60 ) <= CumulativeSeconds(h,m,s) &&
		CumulativeSeconds( mh, mm, ms ) + ( 60 * 60 ) >= CumulativeSeconds(h,m,s) ){
			int ColorCoefficient =  CumulativeSeconds(h,m,s) - (CumulativeSeconds( mh, mm, ms ) - ( 60 * 60 ));
			auto r = 0.9f/2.55f * sin(0.001f * (ColorCoefficient - 1000) / 2.0f);
			r = (r <= 0 ? 0 : r) + ((2201 - ColorCoefficient)/2200.f < 0 ? 0 : ((2201 - ColorCoefficient)/2200.f)*0.603f);
			auto g = 0.3f/2.55f * sin(0.001f * (ColorCoefficient - 1000) / 1.3f);
			g = (g <= 0 ? 0 : g) + ((2201 - ColorCoefficient)/2200.f < 0 ? 0 : ((2201 - ColorCoefficient)/2200.f)*0.603f);
			auto b = 0.5f/2.55f * sin(0.001f * (ColorCoefficient) / 1.5f);
			b = (b <= 0 ? 0 : b) + ((2201 - ColorCoefficient)/2200.f < 0 ? 0 : ((2201 - ColorCoefficient)/2200.f)*0.603f);
			SetLightDifColor(GetColorF(r,g,b,0));
			SetLightSpcColor(GetColorF(r,g,b,0));
	}else if( CumulativeSeconds( sh, sm, ss ) - ( 60 * 60 ) <= CumulativeSeconds(h,m,s) &&
		CumulativeSeconds( sh, sm, ss ) + ( 60 * 60 ) >= CumulativeSeconds(h,m,s) ){
			int ColorCoefficient =  CumulativeSeconds(h,m,s) - (CumulativeSeconds( sh, sm, ss ) - ( 60 * 60 ));
			auto r = 1.2f/2.55f * sin(0.001f * (ColorCoefficient) / 1.8f);
			r = (r <= 0 ? 0 : r) + ((2201 - ColorCoefficient)/2200.f < 0 ? 0 : ((2201 - ColorCoefficient)/2200.f)*0.603f);
			auto g = 0.225f/2.55f * sin(0.001f * (ColorCoefficient - 1400) / 1.3f);
			g = (g <= 0 ? 0 : g) + ((2201 - ColorCoefficient)/2200.f < 0 ? 0 : ((2201 - ColorCoefficient)/2200.f)*0.603f);
			auto b = 0.5f/2.55f * sin(0.001f * (ColorCoefficient - 3400) / 1.2f);
			b = (b <= 0 ? 0 : b) + ((2201 - ColorCoefficient)/2200.f < 0 ? 0 : ((2201 - ColorCoefficient)/2200.f)*0.603f);
			SetLightDifColor(GetColorF(r,g,b,0));
			SetLightSpcColor(GetColorF(r,g,b,0));
	}else if(CumulativeSeconds( sh, sm, ss ) + ( 60 * 60 ) < CumulativeSeconds(h,m,s) ||
		CumulativeSeconds( mh, mm, ms ) - ( 60 * 60 ) > CumulativeSeconds(h,m,s) ){
			SetLightDifColor(GetColorF(0,0,0,0));
			SetLightSpcColor(GetColorF(0,0,0,0));
	}else{
			SetLightDifColor(GetColorF(0.603f,0.603f,0.603f,0));
			SetLightSpcColor(GetColorF(0.603f,0.603f,0.603f,0));
	}
}