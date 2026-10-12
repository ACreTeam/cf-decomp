#pragma once

// d_post_box (DOL TU d_post_box.cpp, .text 8013EBCC.., not decompiled): a player's post box in the extra save
// data (dSaveExtra_c::_17FF58, 0x2738 per player; ctor 8013ED24, dtor 8013ED88). Only what other TUs use.

#include <types.h>

extern "C" {
void fn_8013EE54(void *box); // 8013EE54: clears it (its 10 letters, then the dMail_c at +0x23A0)
}
