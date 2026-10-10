#pragma once

// The message controller's answer list (TU d_select.cpp, not decompiled). Class name and layout
// inferred from d_a_npc_nml's talk_c (8003226C / 800324DC); it is dDemo_c::mSelect (+0x5610).

#include <types.h>

class dMsgSelect_c { // controller+0x5610: the answer list (up to 5 answers)
public:
    // Sets answer idx to message code (a3..a6: always 0 from d_a_npc_nml).
    void fn_8015BF9C(int idx, u16 code, int a3, int a4, int a5, int a6); // 8015BF9C

    /* 0x000 */ u8 _000[4];
    /* 0x004 */ u8 mCursor;   // the chosen answer
    /* 0x005 */ u8 _005[0x930 - 0x5];
    /* 0x930 */ u32 mNum;     // number of answers
    /* 0x934 */ u8 _934[4];
    /* 0x938 */ int mCancel;  // answer taken on cancel (-1: none)
    /* 0x93C */ u8 _93C[0x9B4 - 0x93C];
}; // size 0x9B4
