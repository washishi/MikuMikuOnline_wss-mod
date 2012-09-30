//
// Core.cpp
//

#include "Scene/Include.hpp"
#include "Core.hpp"
#include "ServerLauncher.hpp"
#include "ConfigManager.hpp"
#include "../common/Logger.hpp"
#include "version.hpp"
#include "resource.h"

const TCHAR* Core::CONFIG_PATH = _T("config.json");
const TCHAR* Core::FONT_FILE_PATH = _T("system/fonts/umeplus-p-gothic.ttf");

#ifdef _DEBUG
#define EXCEPTION_LOG(e) (e)
#else
#define EXCEPTION_LOG(e) try { \
			(e); \
		} catch (std::exception& e) { \
			Logger::Error(_T("%s"), unicode::ToTString(e.what())); \
		}
#endif

Core::Core()
{
}

int Core::Run()
{
	Logger::Info(_T("%s"), unicode::ToTString(MMO_VERSION_TEXT));

    if (SetUpDxLib() == -1) {
        return -1;
    }

    ServerLauncher server;

    LoadFont();
    SceneInit();

	EXCEPTION_LOG(MainLoop());

    if (current_scene_) {
        current_scene_ = scene::BasePtr();
    }

    DxLib_End();

    return 0;
}

void Core::MainLoop()
{
    while(ProcessMessage() == 0 &&
            ScreenFlip() == 0 &&
            ClearDrawScreen() == 0 &&
            current_scene_) {

        InputManager::Update();
		InputManager input;
		
        current_scene_->ProcessInput(&input);
        current_scene_->Update();
        current_scene_->Draw();

        if (auto next = current_scene_->NextScene()) {
            SetScene(next);
        }
    }

    if (current_scene_)
    {
        current_scene_->End();
    }
}

int Core::SetUpDxLib()
{
    //TCHAR title[] = WINDOW_TITILE;
    SetMainWindowText(unicode::ToTString(MMO_VERSION_TEXT).c_str());

    SetWindowIconID(100);

    ConfigManager config;
	
	Language::Initialize(config.language());

    if (config.fullscreen() == 0) {
        ChangeWindowMode(TRUE);
    }

    SetAlwaysRunFlag(TRUE);
    SetMultiThreadFlag(TRUE);
    SetDoubleStartValidFlag(FALSE);

    SetGraphMode(config.screen_width(), config.screen_height(), 32);

    if (config.antialias() == 1) {
        SetFullSceneAntiAliasingMode(4, 2);
    }

    if( DxLib_Init() == -1 )
        return -1;

    SetDrawScreen(DX_SCREEN_BACK);
    SetDragFileValidFlag(TRUE);
    DragFileInfoClear();

    MV1SetLoadModelPhysicsWorldGravity(-100);
    MV1SetLoadModelUsePhysicsMode(DX_LOADMODEL_PHYSICS_LOADCALC);

    return 0;
}

void Core::SceneInit()
{
    SetScene(scene::BasePtr(new scene::Init()));
}

void Core::SetScene(const scene::BasePtr& scene)
{
    if (current_scene_)
    {
        current_scene_->End();
    }

    current_scene_ = scene;

    if (current_scene_)
    {
        current_scene_->Begin();
    }
}

void Core::LoadFont()
{
    const TCHAR* font_path = FONT_FILE_PATH;
    if (AddFontResourceEx(font_path, FR_PRIVATE, nullptr) == 0) {
        Logger::Error(_T("Cannot load font: %s"), font_path);
    }
}