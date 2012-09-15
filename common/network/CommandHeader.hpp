//
// CommandHeader.hpp
//

#pragma once

namespace network {
namespace header {
    enum CommandHeader {
        FatalConnectionError =                      0x01,
        ServerStartEncryptedSession =               0x02,
        ClientStartEncryptedSession =               0x03,
        ServerReceivePublicKey =                    0x04,
        ClientReceiveCommonKey =                    0x05,
        ClientUpdatePlayerPosition =                0x06,
        ServerUpdatePlayerPosition =                0x07,
        ServerReceiveClientInfo =                   0x08,
        ClientRequestedPublicKey =                  0x09,
        ClientRequestedClientInfo =                 0x0A,
        ClientReceiveAccountRevisionPatch =         0x0B,
        ServerRequestedAccountRevisionPatch =       0x0C,
        ClientReceiveAccountRevisionUpdateNotify =  0x0D,
        ClientReceiveWriteAverageLimitUpdate =      0x0E,
        ClientReceiveServerCrowdedError =           0x0F,
        ClientReceiveUnsupportVersionError =        0x10,
        ServerReceiveAccountInitializeData =        0x11,
        ClientReceiveServerInfo =                   0x12,
        ServerUpdateAccountProperty =               0x13,
        ServerReceiveJSON =                         0x14,
        ClientReceiveJSON =                         0x15,

		ServerRequstedStatus =						0xE0,

        LZ4_COMPRESS_HEADER =                       0xF0,
        ENCRYPT_HEADER =                            0xF1
    };

}
}
