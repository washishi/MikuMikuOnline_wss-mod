//
// main.cpp
//

#include <Windows.h>
#include <tchar.h>
#include "Core.hpp"
#include <crtdbg.h>

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    //_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_ALLOC_MEM_DF);

    Core core;
    return core.Run();
}
