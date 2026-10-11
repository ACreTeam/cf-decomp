#pragma once

// WiiConnect24 (DOL TU d_net_wc24.cpp, not decompiled). Only what other TUs use.

#include <types.h>

// The WiiConnect24 state (805BE940; size unknown).
class dNetWc24_c {
public:
    BOOL getFriendKey(u32 idx, u64 *key); // 800E997C: entry idx of the NWC24 friend list, FALSE if unused

    /* 0x000 */ u8 _000[0x400];
    /* 0x400 */ u64 mUserId; // this console's user id (sent by d_net)
};

extern dNetWc24_c lbl_805BE940; // 805BE940

extern "C" {
u32 fn_800E94A8(); // 800E94A8: size of the NWC24 library heap (0x5C800)
}
