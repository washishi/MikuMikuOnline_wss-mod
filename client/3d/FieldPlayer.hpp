//
// FieldPlayer.hpp
//

#pragma once
#include "Character.hpp"
#include "CharacterManager.hpp"
#include "CharacterDataProvider.hpp"
#include "MotionPlayer.hpp"
#include "Timer.hpp"
#include "../InputManager.hpp"
#include "../ResourceManager.hpp"

class Stage;
typedef std::shared_ptr<Stage> StagePtr;

struct PlayerStatus
{
    PlayerStatus(const VECTOR& pos_ = VGet(0, 0, 0), const VECTOR& vel_ = VGet(0, 0, 0), const VECTOR& acc_ = VGet(0, 0, 0),
        float roty_ = 0, float roty_speed_ = 0, int motion_ = 0, int prev_motion_ = 0,
        float blend_ratio_ = 0, bool is_walking_ = false) :
    pos(pos_), vel(vel_), acc(acc_), roty(roty_),
        roty_speed(roty_speed_), motion(motion_), prev_motion(prev_motion_),
        blend_ratio(blend_ratio_), is_walking(is_walking_)
    {}

    VECTOR pos, vel, acc; // プレイヤーのマップ上での位置、速度、加速度
    float roty, roty_speed; // プレイヤーの鉛直軸周りの回転角
    int motion, prev_motion; // モーションの種類(BasicMotion::hoge)
    float blend_ratio; // モーションのブレンド比
    bool is_walking; // true:歩き, false:走り
};

// フィールド上のキャラクタ
class FieldPlayer : public Character
{
	//struct AdditionalMotion
	//{
	//	AdditionalMotion() : flag_(false), handle_(-1), isloop_(true), nextanim_handle_(-1),loopcheck_(false){}
	//	bool flag_;
	//	int handle_;
	//	bool isloop_;
	//	int nextanim_handle_;
	//	bool loopcheck_;
	//};
	struct AdditionalMotion
	{
		std::vector<MotionPlayer::ChainData> chain_data;
		bool flag_;
	};
public:
    FieldPlayer(CharacterDataProvider& data_provider, const std::shared_ptr<const StagePtr>& stage, const TimerPtr& timer);

    void Draw() const;
    void Update();
    void Init(tstring model_path, const std::shared_ptr<VECTOR>& init_position);
    void ResetPosition(const std::shared_ptr<VECTOR>& init_position);
    void RescuePosition();

//	void LoadModel(const tstring& name);
    void LoadModel(const tstring& name,const bool async = false); // ※　非同期読み込みの指定ができるように修正
    void SetModel(const ModelHandle& model);
	void PlayMotion(const tstring& name,bool isloop);
	void FieldPlayer::ResetMotion();

public:
    const ModelHandle& model_handle() const;
    const PlayerStatus& current_stat() const;
    float model_height() const;
    bool any_move() const;

    void LinkToCamera(float* roty);
    void UnlinkToCamera();
    void UpdateInput(InputManager* input);
	ModelHandle loading_model_handle_; // ※非同期読み込みを復活させるため追加

private:
    void Move();
    void InputFromUser();
	void Chara_ShadowRender() const;

private:
    PlayerStatus prev_stat_, current_stat_;
    float model_height_;
	float flight_duration_ideal_;
	float jump_height_;
	int prev_mouse_pos_y_;
    std::unique_ptr<MotionPlayer> motion_player_;
	AdditionalMotion additional_motion_;
    TimerPtr timer_;

private:
    ModelHandle model_handle_;
    std::shared_ptr<const StagePtr> stage_;
    bool any_move_;
    InputManager input_;

	int shadow_handle_;
	float shadow_size_;

    struct {
        int
			stand_,
			walk_,
			run_,
			pre_jmp_,
			jmp_,
			end_jmp_;
    } motion;

	int dummy_move_count_;
	bool jump_wait_;

    CharacterDataProvider& data_provider_;

    float* camera_roty_;

	std::unordered_map<std::string,std::string> allocated_motion_;
};
typedef std::shared_ptr<FieldPlayer> FieldPlayerPtr;
