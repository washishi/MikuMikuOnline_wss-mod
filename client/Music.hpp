//
// Music.hpp
//
#include "../common/unicode.hpp"
#include <v8.h>
#include <map>
#include <vector>

//解析する信号の長さ(秒)
#define SIGNAL_LENGTH 0.5
//スペクトラムの数
#define SPECTRUM_NUM 256
//スペクトラムの拡大幅
#define SPECTRUM_WIDTH 2


class Music
{
public:
	Music();
	virtual ~Music(){};
	void Init();
	void Update();
	void Play(tstring name,bool crossfade);
	void Stop(bool fadeout);
	void PlaySE(tstring name);
	void PlayME(tstring name);
	bool CheckLoadedBGM(tstring name);
	std::vector<boost::filesystem::path>& GetMusicList();

private:
	bool crossfade_now_;
	int fade_count_;
	int present_bgm_;
	int prev_bgm_;
	int requested_bgm_;
	std::map<tstring,int> bgm_handle_;
	std::map<tstring,int> se_handle_;

	std::vector<boost::filesystem::path> music_paths_;
	std::vector<boost::filesystem::path> se_paths_;
};
