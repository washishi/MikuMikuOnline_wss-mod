//
// CommandHeader.hpp
//

#pragma once

namespace network {
namespace header {
    enum CommandHeader {
        ConnectionSucceeded =                       0x90000001,
        ConnectionFailed =                          0xD0000002,
        FatalConnectionError =                      0xD0000003,
        ClientReceiveChatMessage =                  0x80000004,
        ServerReceiveChatMessage =                  0x00000005,
        ServerStartEncryptedSession =               0x00000006,
        ClientStartEncryptedSession =               0x80000007,
        ServerReceivePublicKey =                    0x00000008,
        ClientReceiveCommonKey =                    0x80000009,
        ClientReceiveChatLog =                      0x8000000A,
        ServerRequestedChatLog =                    0x2000000B,
        ClientJoinPlayer =                          0x8000000C,
        ClientLeavePlayer =                         0x8000000D,
        ClientUpdatePlayerPosition =                0x8000000E,
        ServerUpdatePlayerPosition =                0x0000000F,
        ServerReceiveClientInfo =                   0x00000010,
        ClientRequestedPublicKey =                  0xA0000011,
        ClientRequestedClientInfo =                 0xA0000012,
        ClientUpdateChannelUserList =               0x80000013,
        ServerCloseSession =                        0x00000014,
        ServerReceiveNewCard =                      0x00000015,
        ClientReceiveSystemMessage =                0x80000016,
        ServerRequestedRemoveCard =                 0x20000017,
        ServerRequestedCardRevisionPatch =          0x20000018,
        ClientReceiveCardRevisionPatch =            0x80000019,
        ClientReceiveCardRevisionUpdateNotify =     0x8000001A,
        ClientReceiveAccountRevisionPatch =         0x8000001B,
        ServerRequestedAccountRevisionPatch =       0x2000001C,
        ClientReceiveAccountRevisionUpdateNotify =  0x8000001D,
        ClientReceiveWriteAverageLimitUpdate =      0x8000001E,
        ClientReceiveServerCrowdedError =           0xC000001F,
        PlayerLogoutNotify =                        0x10000020,
        ServerUpdatePlayerName =                    0x00000021,
        ServerUpdatePlayerTrip =                    0x00000022,
        ClientReceiveUnsupportVersionError =        0xC0000023,
        ServerReceiveAccountInitializeData =        0x00000024,
        ServerUpdatePlayerModelName =               0x00000025,
        ClientReceiveServerInfo =                   0x80000026,

        ServerReceiveJSON =                         0x00000040,
        ClientReceiveJSON =                         0x80000080
    };

}
}
