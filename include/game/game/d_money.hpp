#pragma once

// Money (d_money.cpp, .text from 8019BD78). Not decompiled yet; only what other TUs call is declared.
// The functions keep the target's C names until the TU is split.

#include <types.h>

extern "C" {
void fn_8019C34C(); // 8019C34C: sets bit 0x4 of the flags lbl_8074EAA8 (d_npc_talk_fmarket, d_npc_talk_quest_q09 trade)
void fn_8019C35C(); // 8019C35C: clears bit 0x4 of lbl_8074EAA8
}
