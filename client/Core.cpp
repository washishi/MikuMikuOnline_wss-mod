//
// Core.cpp
//

#include "Scene/Include.hpp"
#include "Core.hpp"
#include "ServerLauncher.hpp"
#include "ConfigManager.hpp"
#include "../common/Logger.hpp"

const TCHAR* Core::CONFIG_PATH = _T("config.json");
const TCHAR* Core::WINDOW_TITILE = _T("Miku Miku Online");
const TCHAR* Core::FONT_FILE_PATH = _T("resources/fonts/umeplus-p-gothic.ttf");

Core::Core()
{
}

int Core::Run()
{
    if (SetUpDxLib() == -1) {
        return -1;
    }

    ServerLauncher server;

    LoadFont();
    SceneInit();
    MainLoop();

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
    const TCHAR* title = WINDOW_TITILE;
    SetMainWindowText(title);

    SetWindowIconID(100);

    ConfigManager config;
    config.Load("config.json");

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
