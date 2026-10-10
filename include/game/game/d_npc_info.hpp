#pragma once

// d_npc_info.cpp (not decompiled). Only what the talk TUs use is declared.

#include <types.h>

namespace dItem {
struct Item;
}

// 800F8948: both items are valid and have the same nonzero BITM fashion theme (m_fashion < 0x39).
extern "C" BOOL fn_800F8948(const dItem::Item *a, const dItem::Item *b);
