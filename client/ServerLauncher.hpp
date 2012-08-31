//
// ServerLauncher.hpp
//

#pragma once
#include <DxLib.h>

class ServerLauncher {
    public:
        ServerLauncher();
        ~ServerLauncher();

    private:
        PROCESS_INFORMATION pi_;
};
