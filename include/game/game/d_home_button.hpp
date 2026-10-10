#pragma once

// The HOME button menu (DOL TU d_home_button.cpp, not decompiled). Only what other TUs use; the
// class name and field names are inferred.

#include <types.h>

class dHomeButton_c {
public:
    enum State_e {
        STATE_OPEN = 2, // the HOME menu is shown
    };

    /* 0x000 */ u8 _0[0x1A4];
    /* 0x1A4 */ int mState; // State_e
    /* 0x1A8 */ int _1A8;
    /* 0x1AC */ u32 mFlags; // 0x200: HOME menu and reset disabled (saving)
};

extern "C" {
dHomeButton_c *fn_8017D8E8();                    // 8017D8E8: the instance
BOOL fn_8017E570(dHomeButton_c *hbm, int type);  // 8017E570: closes the menu for the reset (1) / power (2) button
}
