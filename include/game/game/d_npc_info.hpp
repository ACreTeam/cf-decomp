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

// Per special npc index (item 0x8000 | idx, idx < 0x61) tables.
extern "C" int fn_800F88E0(const dItem::Item *npc); // 800F88E0: sound id (0xFF = none)
extern "C" int fn_800F8914(const dItem::Item *npc); // 800F8914: voice type (2 = default)
