#pragma once

// The hand tool models (DOL TU d_hmn_tool_mng.cpp, "dHmnToolBank_c::m_heap_p", not decompiled).
// Only what d_a_npc uses; member names are provisional.

#include <types.h>
#include <game/game/d_fg_item.hpp>
#include <game/mLib/m_mtx.hpp>

// One tool model bank (non-polymorphic; starts with an mAllocator_c).
class dHmnToolBank_c {
public:
    dHmnToolBank_c();  // 800BA180
    ~dHmnToolBank_c(); // 800BA2C4
    BOOL fn_800BA474(const dItem::Item *item, int a, int b); // 800BA474: load the tool model of item
    void fn_800BAA00(const mMtx_c *mtx);                    // 800BAA00: calc at mtx
    void fn_800BAC9C();                                     // 800BAC9C: draw
    void fn_800BAD74();                                     // 800BAD74 (toolBase_c::change)
    void fn_800BB63C();                                     // 800BB63C (toolBase_c::putAway)
    int getType() const { return mType; } // int return: toolBase_c::isEnable's full signed compare

    /* 0x000 */ u8 _000[0x70E];
    /* 0x70E */ u8 mType; // 9 = none
    /* 0x70F */ u8 _70F[0x71C - 0x70F];
}; // size 0x71C

extern "C" {
int fn_800BA890(const dItem::Item *item); // 800BA890: tool type of an item (0 = none)
}
