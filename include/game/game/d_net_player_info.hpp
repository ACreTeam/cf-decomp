#pragma once

// d_net_player_info (DOL TU d_net_player_info.cpp, .text 800AC154..800AC1CC; split from the old d_sys_heap range): this console's
// player / town / friend-list names, sent to the other members (dNet_c::mPlayerInfo). No RTTI: names inferred.

#include <types.h>

namespace dNetPlayerInfo {

struct Info_c {
    void clear(); // 800AC154
    void set();   // 800AC160: from dPlayerMgr_c::getCurrentPlayer()

    /* 0x00 */ wchar_t mPlayerName[9];    // dPrivateData_c::mPID.player.mName
    /* 0x12 */ wchar_t mTownName[9];      // dPrivateData_c::mPID.land.mName
    /* 0x24 */ wchar_t mFriendName[0x21]; // dFriendList_c::mName
}; // size 0x66

} // namespace dNetPlayerInfo
