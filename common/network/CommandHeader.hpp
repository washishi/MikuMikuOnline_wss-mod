//
// CommandHeader.hpp
//

#pragma once

namespace network {
namespace header {
    enum CommandHeader {
        FatalConnectionError =                      0xD0000003,
        ServerStartEncryptedSession =               0x00000006,
        ClientStartEncryptedSession =               0x80000007,
        ServerReceivePublicKey =                    0x00000008,
        ClientReceiveCommonKey =                    0x80000009,
        ClientUpdatePlayerPosition =                0x8000000E,
        ServerUpdatePlayerPosition =                0x0000000F,
        ServerReceiveClientInfo =                   0x00000010,
        ClientRequestedPublicKey =                  0xA0000011,
        ClientRequestedClientInfo =                 0xA0000012,
        ClientReceiveAccountRevisionPatch =         0x8000001B,
        ServerRequestedAccountRevisionPatch =       0x2000001C,
        ClientReceiveAccountRevisionUpdateNotify =  0x8000001D,
        ClientReceiveWriteAverageLimitUpdate =      0x8000001E,
        ClientReceiveServerCrowdedError =           0xC000001F,
        ClientReceiveUnsupportVersionError =        0xC0000023,
        ServerReceiveAccountInitializeData =        0x00000024,
        ClientReceiveServerInfo =                   0x80000026,
        ServerUpdateAccountProperty =               0x00000027,

        ServerReceiveJSON =                         0x00000040,
        ClientReceiveJSON =                         0x80000080
    };

}
}
