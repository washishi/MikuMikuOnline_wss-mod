#include "MotionPlayer.hpp"
#include <stdexcept>
#include <DxLib.h>


MotionPlayer::MotionPlayer(int model_handle)
    : model_handle_(model_handle), prev_attach_index_(-1), current_attach_index_(-1),
      connect_prev_(false), prev_blend_rate_(0), blend_time_(0),prev_anim_index_(-1),isloop_(true),isloopcheck_(false),nextconnect_(true)
{}

void MotionPlayer::Play(int anim_index, bool connect_prev, int blend_time, int anim_src_model_handle, bool check_name, bool isloop, int nextanim_handle, bool isloopcheck, bool nextconnect)
{
 	if(nextanim_handle != -1)
	{
		prev_anim_index_ = nextanim_handle;
	}else if(!isloop){
		prev_anim_index_ = MV1GetAttachAnim(model_handle_,current_attach_index_);
	}

	// まだ前回の移行期間の最中なら、移行を中止する
    DetachPrevMotionIfExist();

    connect_prev_ = connect_prev;
    prev_blend_rate_ = blend_time;
    blend_time_ = blend_time;
	isloop_ = isloop;
	isloopcheck_ = isloopcheck;
	nextconnect_ = nextconnect;
	if(!isloopcheck_)isplayend_ = false;

    prev_attach_index_ = current_attach_index_;
    if (blend_time_ <= 0)
    {
        DetachPrevMotionIfExist();
    }
	
    current_attach_index_ = MV1AttachAnim(model_handle_, anim_index,
            anim_src_model_handle, check_name ? TRUE : FALSE);
    MV1SetAttachAnimTime(model_handle_, current_attach_index_, 0);

    SetBlendRateToModel();
}

void MotionPlayer::Stop()
{
    if (current_attach_index_ != -1)
    {
        if (MV1DetachAnim(model_handle_, current_attach_index_) == -1)
        {
            throw std::logic_error("can't detach anim");
        }
        current_attach_index_ = -1;
    }
}

// diff_time [ms]だけ時刻をすすめる
void MotionPlayer::Next(int diff_time)
{
    if (current_attach_index_ != -1)
    {
        AdvancePlayTime(diff_time);

        prev_blend_rate_ -= diff_time;
        if (prev_blend_rate_ < 0)
        {
            DetachPrevMotionIfExist();
            prev_blend_rate_ = 0;
        }

        SetBlendRateToModel();
    }
}

void MotionPlayer::SetBlendRateToModel()
{
    float prev_blend_rate = static_cast<float>(prev_blend_rate_) / blend_time_;

    if (prev_attach_index_ != -1)
    {
        MV1SetAttachAnimBlendRate(model_handle_, prev_attach_index_, prev_blend_rate);
    }
    if (current_attach_index_ != -1)
    {
        MV1SetAttachAnimBlendRate(model_handle_, current_attach_index_, 1.0f - prev_blend_rate);
    }
}

void MotionPlayer::DetachPrevMotionIfExist()
{
    if (prev_attach_index_ != -1)
    {
        if (MV1DetachAnim(model_handle_, prev_attach_index_) == -1)
        {
            throw std::logic_error("can't detach anim");
        }
        prev_attach_index_ = -1;
    }
}

void MotionPlayer::AdvancePlayTime(int diff_time)
{
    auto anim_total_time = MV1GetAttachAnimTotalTime(model_handle_, current_attach_index_);

    auto anim_time = MV1GetAttachAnimTime(model_handle_, current_attach_index_);
    anim_time += (diff_time * 30) / 1000.0f;
    if (anim_time > anim_total_time)
    {
		if(!isloop_)
		{
			Stop();
			isplayend_ = true;
			Play(prev_anim_index_,nextconnect_,200,-1,false,true,-1,isloopcheck_);
			return;
		}
        anim_time -= anim_total_time;
    }
    if (MV1SetAttachAnimTime(model_handle_, current_attach_index_, anim_time) == -1)
    {
        throw std::logic_error("can't set anim time to current_attach_index_");
    }

    if (connect_prev_ && prev_attach_index_ != -1)
    {
        auto prev_anim_total_time = MV1GetAttachAnimTotalTime(model_handle_, prev_attach_index_);

        auto prev_anim_time = MV1GetAttachAnimTotalTime(model_handle_, prev_attach_index_);
        prev_anim_time = anim_time * prev_anim_total_time / anim_total_time;
        if (MV1SetAttachAnimTime(model_handle_, prev_attach_index_, prev_anim_time) == -1)
        {
            throw std::logic_error("can't set anim time to prev_attach_index_");
        }
    }
}

bool MotionPlayer::GetPlayEnd()
{
	if(isplayend_)
	{
		isplayend_ = false;
		return true;
	}
	return false;
}
