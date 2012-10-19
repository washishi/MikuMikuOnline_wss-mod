#pragma once

#include <vector>

class MotionPlayer
{
public:
	struct ChainData{
		ChainData() : anim_index(-1), blend_time(200), anim_src_model_handle(-1),connect_prev(true),check_name(false),isloop(false){}
		int anim_index;
		int blend_time;
		int anim_src_model_handle;
		bool connect_prev;
		bool isloop;
		bool check_name;
	};
    MotionPlayer(int model_handle);
    void Play(int anim_index, bool connect_prev, int blend_time, int anim_src_model_handle, bool check_name, bool isloop = true, int nextanim_handle = -1,bool isloopcheck = false);
    void ChainPlay(const std::vector<ChainData> chain_data);
    void Next(int diff_time);
	void Stop();
	bool GetPlayEnd();

private:
	void _ChainPlay(const std::vector<ChainData>::const_iterator &it);

    int model_handle_;
    int prev_attach_index_, current_attach_index_;
    bool connect_prev_;
    int prev_blend_rate_;
    int blend_time_;
	bool isloop_;
	int prev_anim_index_;
	bool isplayend_;
	bool isloopcheck_;
	std::vector<ChainData> chain_data_;
	std::vector<ChainData>::const_iterator chain_data_it_;

    void SetBlendRateToModel();
    void DetachPrevMotionIfExist();
    void AdvancePlayTime(int diff_time);
};
