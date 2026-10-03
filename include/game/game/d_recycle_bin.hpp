#pragma once

#include <types.h>
#include <game/game/d_fg_item.hpp>

class dTime_c;

#define RECYCLE_BIN_ITEM_NUM 12

// The town hall's recycle bin (dSaveData_c::mRecycleBin, town+0x72DF2).
// Defined in src/dol/game/d_recycle_bin.cpp (.text 80153E9C..80154080).
// The class name is inferred (no RTTI). The lost and found right before it
// (dPoliceBox_c, town+0x72DDA) is the same shape.
class dRecycleBin_c {
public:
    void clear(); // 80153E9C
    void update(dTime_c *now, int days); // 80153ED8: emptied on Mondays and Thursdays
    u16 get(int slot); // 80153FA0
    void set(int slot, u16 item); // 80153FAC
    BOOL add(u16 item); // 80153FB8: first free slot; not insects, fish or items without kind flag 4
    void fn_8015407C(); // 8015407C: empty

    /* 0x00 */ u16 mItems[RECYCLE_BIN_ITEM_NUM];
}; // size 0x18
