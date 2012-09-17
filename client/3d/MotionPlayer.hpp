#pragma once

class MotionPlayer
{
public:
    MotionPlayer(int model_handle);
    void Play(int anim_index, bool connect_prev, int blend_time, int anim_src_model_handle, bool check_name, bool isloop = true);
    void Next(int diff_time);
	void Stop();

private:
    int model_handle_;
    int prev_attach_index_, current_attach_index_;
    bool connect_prev_;
    int prev_blend_rate_;
    int blend_time_;
	bool isloop_;
	int prev_anim_index_;

    void SetBlendRateToModel();
    void DetachPrevMotionIfExist();
    void AdvancePlayTime(int diff_time);
};
