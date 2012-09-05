//
// ServerLauncher.hpp
//

#pragma once
#include <windows.h>
#include <boost/interprocess/windows_shared_memory.hpp>

class ServerLauncher {
    public:
        ServerLauncher();
        ~ServerLauncher();

    private:
        PROCESS_INFORMATION pi_;
		boost::interprocess::windows_shared_memory shm_;
};
