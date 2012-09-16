//
// Core.hpp
//

#pragma once
#include <DxLib.h>
#include <memory>

namespace scene {
class Base;
typedef std::shared_ptr<Base> BasePtr;
}

class Core {

    public:
        Core();
        int Run();

    private:
        void MainLoop();
        int SetUpDxLib();
        void SceneInit();
        void SetScene(const scene::BasePtr&);

        void LoadFont();

    private:
        scene::BasePtr current_scene_;

    private:
        const static TCHAR* CONFIG_PATH;
        const static TCHAR* FONT_FILE_PATH;

};
