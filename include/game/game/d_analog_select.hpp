#pragma once

// The message controller's two-way npc select (TU d_analog_select.cpp, not decompiled). Class name and
// layout inferred from d_a_npc_nml's talk_c (800323F0 / 800324DC); it is dDemo_c::mAnalogSelect (+0x5FC4).

#include <types.h>

class dMsgAnalogSelect_c { // controller+0x5FC4: the two-way npc select ("sys_SELECT/SYS_SelectNPC")
public:
    // Shows label (a message label) / code on side idx (0, 1).
    void fn_8000D340(int idx, u16 code, const char *label); // 8000D340

    /* 0x000 */ u8 _000[4];
    /* 0x004 */ u8 mCursor;   // the chosen side
    /* 0x005 */ u8 _005[3];
}; // size >= 8 (end not checked)
