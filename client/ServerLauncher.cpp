//
// ServerLauncher.cpp
//

#include "ServerLauncher.hpp"
#include "../common/Logger.hpp"
#include <boost/interprocess/managed_shared_memory.hpp>

ServerLauncher::ServerLauncher()
{
    // サーバーを起動
    STARTUPINFO si;
    ZeroMemory(&si,sizeof(si));
    si.cb=sizeof(si);

    // カレントディレクトリを取得
    TCHAR crDir[MAX_PATH + 1];
    GetCurrentDirectory(MAX_PATH + 1 , crDir);
    _tcscat(crDir, _T("\\server"));
    
    Logger::Info(_T("Starting Server..."));

    using namespace boost::interprocess;
    managed_shared_memory shm(open_or_create, "MMO_SERVER_WITH_CLIENT", 256);
    CreateProcess(_T("./server/Server.exe"), nullptr, nullptr, nullptr, FALSE,
            NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, nullptr, crDir, &si, &pi_);
}

ServerLauncher::~ServerLauncher()
{
    // サーバーを停止
    Logger::Info(_T("Closing Server..."));
    using namespace boost::interprocess;
    shared_memory_object::remove("MMO_SERVER_WITH_CLIENT");
    WaitForSingleObject(pi_.hThread, INFINITE);
    CloseHandle(pi_.hThread);
    CloseHandle(pi_.hProcess);
}
