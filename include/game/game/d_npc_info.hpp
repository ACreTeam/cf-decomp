#pragma once

// d_npc_info.cpp (not decompiled). Only what the talk TUs and d_a_npc_sp use is declared.

#include <types.h>

namespace dItem {
struct Item;
}

// 800F8948: both items are valid and have the same nonzero BITM fashion theme (m_fashion < 0x39).
extern "C" BOOL fn_800F8948(const dItem::Item *a, const dItem::Item *b);

// Fields of the npc info global 805CE800 (d_a_npc_sp's wander parameters).
extern "C" int fn_800F59EC(); // 800F59EC: +0x18, walk chance (percent) of a wandering special npc
extern "C" int fn_800F59FC(); // 800F59FC: +0x1C, wander wait (frames, base)
extern "C" int fn_800F5A0C(); // 800F5A0C: +0x20, wander wait (frames, random part)

class dItemPairRing_c;

// The pair ring object 805CE854 of the npc info global (d_net).
extern "C" dItemPairRing_c *fn_800F5C88();         // 800F5C88: &805CE854
extern "C" void fn_800F6098(dItemPairRing_c *ring); // 800F6098: (the current player's own when from this town)
extern "C" void fn_800F60E4(dItemPairRing_c *ring); // 800F60E4

// Per special npc index (item 0x8000 | idx, idx < 0x61) tables.
extern "C" int fn_800F88E0(const dItem::Item *npc); // 800F88E0: sound id (0xFF = none)
extern "C" int fn_800F8914(const dItem::Item *npc); // 800F8914: voice type (2 = default)

// 805CE928 (+0x128) of the npc info global: a visit state (dNpcMng_c's letter visits). fn_800F7D54 is its
// first word while lbl_8074AEE8 is set, else 0.
extern "C" void *fn_800F7D2C();          // 800F7D2C: &805CE928
extern "C" int fn_800F7D54(void *state); // 800F7D54

// Work objects of the npc info global 805CE800 (d_net packets 0x45..0x50).
extern "C" void *fn_800F6514();          // 800F6514: &805CE884
extern "C" void fn_800F6634(void *work); // 800F6634

// 805CE964 (fn_800F800C): an npc event state synchronised over the net (d_net packets 0x45..0x4A).
struct dNpcInfoNet_c {
    /* 0x00 */ u8 _00[2];
    /* 0x02 */ u8 _02;
    /* 0x03 */ u8 _03;
    /* 0x04 */ u8 _04[4];
    /* 0x08 */ u8 _08;
    /* 0x09 */ u8 _09;
    /* 0x0A */ u8 _0A[0x30];
    /* 0x3A */ u8 _3A;
    /* 0x3B */ u8 _3B;
};
extern "C" dNpcInfoNet_c *fn_800F800C();                            // 800F800C: &805CE964
extern "C" void fn_800F8700(dNpcInfoNet_c *info, BOOL flag);        // 800F8700
extern "C" void fn_800F87B8(dNpcInfoNet_c *info, const void *data); // 800F87B8: net receive (0x28 bytes)
