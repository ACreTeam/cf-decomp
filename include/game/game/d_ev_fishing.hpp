#pragma once

// The fishing tourney (EVENT_FISHING_TOURNEY). DOL TU d_ev_fishing.cpp (.text 801511B4..80153818), not
// decompiled yet. Only the accessors of the tourney record used by d_npc_talk_fishing are declared here.
// The record is dSaveTown_c::_0632F0 (0x200 bytes; dSaveData_c::getRaw()->_0632F0). Rank 0 is the
// leader, 1 the runner-up. The functions keep their unmangled placeholder names until the TU is done.

#include <types.h>
#include <game/game/d_item.hpp>

class dPersonalID_c;
class dAnmPersonalID_c;

extern "C" {
dPersonalID_c *fn_801533C8(u8 *record, int rank);    // 801533C8 player of the rank (+0x18 / +0x44)
dAnmPersonalID_c *fn_801533E0(u8 *record, int rank); // 801533E0 villager of the rank (+0x70 / +0x130)
int fn_801533F8(u8 *record, int rank);               // 801533F8 fish size of the rank (+0x1F0 / +0x1F4)
dItem::Item fn_80153410(u8 *record);                 // 80153410 item at +0x1F8
dItem::Item fn_8015341C(u8 *record, int rank);       // 8015341C fish of the rank (+0x1FA / +0x1FC)
f32 fn_8015343C(f32 size);                           // 8015343C size in the display unit (by language)
}
