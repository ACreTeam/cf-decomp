#pragma once

// The DWC session (DOL TU d_net_dwc.cpp, .text 800E7650..800E925C, not
// decompiled): the Wi-Fi Connection login, matchmaking and voice chat on the
// DWC thread. Only what d_net_mgr uses; all names are inferred. The instance is
// d_net_mgr's g_netDwc.

#include <lib/dwc/dwc.h>
#include <types.h>

namespace EGG {
class Heap;
}
class dNetFriendData_c;
class dNetSendBuf_c;

class dNetDwc_c {
  public:
    dNetDwc_c();  // 800E77F4
    ~dNetDwc_c(); // 800E799C

    // 800E79E4
    void init(EGG::Heap *soHeap, EGG::Heap *dwcHeap, dNetFriendData_c *data);
    // 800E7A6C
    void cleanup();
    // 800E7AD8
    void reset();
    void start(DWCUserData *userData, DWCFriendData *friends, u8 friendNum,
               dNetSendBuf_c *sendBufs,
               // 800E7AF8
               u8 voice);
    void fn_800E7BE4();
    BOOL fn_800E84C0(u8 arg);
    BOOL fn_800E85CC(u8 arg, u8 arg2);
    BOOL fn_800E86BC(u8 arg, u32 arg2);
    void fn_800E8820();
    void fn_800E8864();
    void fn_800E88A8(); // after the friend list changed
    void fn_800E8910();
    void fn_800E8918();
    void fn_800E8B5C();
    void fn_800E8DA4(u8 aid);
    void fn_800E8DF0(u8 aid);
    void fn_800E8DF8(BOOL arg);

    /* 0x000 */ u8 _000[0x379];
    /* 0x379 */ u8 mVoiceOn;
    /* 0x37A */ u8 _37A[6];
}; // size 0x380
