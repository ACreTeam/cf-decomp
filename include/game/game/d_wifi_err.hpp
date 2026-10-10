#pragma once

// The WiiConnect24 / network error window (DOL TU d_wifi_err.cpp, not decompiled). Only what other
// TUs use; the class and field names are inferred.

#include <types.h>

class dWifiErr_c {
public:
    /* 0x0 */ bool mActive;        // the error window is shown
    /* 0x1 */ u8 _1[0x7 - 0x1];
    /* 0x7 */ bool mResetRequest;  // the reset button was pressed while it is shown (d_reset)
    /* 0x8 */ u8 _8[0x198 - 0x8];
}; // size 0x198

extern "C" {
extern dWifiErr_c lbl_80621670; // the instance (in an unsplit .bss)
}
