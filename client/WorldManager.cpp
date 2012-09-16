//
// WorldManager.cpp
//

#include "WorldManager.hpp"
#include "PlayerManager.hpp"
#include "3d/Stage.hpp"
#include "Profiler.hpp"
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>

WorldManager::WorldManager(const ManagerAccessorPtr& manager_accessor) :
manager_accessor_(manager_accessor),
stage_(std::make_shared<Stage>(
[]()->tstring{
	FILE *fp;
	size_t file_size;

	fopen_s(&fp,".\\resources\\models\\default_stage.cfg", "r, ccs=UTF-8");
	fseek(fp, 0, SEEK_SET);
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *buf = (char*)malloc(file_size);
	ZeroMemory(buf,file_size);
	fread(buf,file_size,1,fp);
	fclose(fp);
	tstring str = _T("stage:");
	str += reinterpret_cast<TCHAR*>(buf);
	free(buf);
	return str;
}().c_str())),
game_loop_(stage_)
{

}

void WorldManager::Init()
{
    auto player_manager = manager_accessor_->player_manager().lock();
    game_loop_.Init(player_manager->charmgr());

}

void WorldManager::ProcessInput(InputManager* input)
{
	MMO_PROFILE_FUNCTION;

    game_loop_.Logic(input);
}

void WorldManager::Update()
{
	MMO_PROFILE_FUNCTION;
}

void WorldManager::Draw()
{
	MMO_PROFILE_FUNCTION;

    game_loop_.Draw();
}

bool WorldManager::IsVisiblePoint(const VECTOR& point)
{
    return stage_->IsVisiblePoint(point);
}

StagePtr WorldManager::stage()
{
    return stage_;
}

FieldPlayerPtr WorldManager::myself()
{
    return game_loop_.myself();
}

void WorldManager::ResetCameraPosition()
{
    game_loop_.ResetCameraPosition();
}