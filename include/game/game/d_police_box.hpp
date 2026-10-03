#pragma once

#include <types.h>
#include <game/game/d_fg_item.hpp>

#define POLICE_BOX_ITEM_NUM 12

// The police station's lost and found (dSaveData_c::mPoliceBox, town+0x72DDA).
// Defined in src/dol/game/d_police_box.cpp (.text 801538F8..80153E9C).
// The name follows GC's PoliceBox_c (no RTTI here).
class dPoliceBox_c {
public:
    void clear(); // 801538F8
    void init(); // 80153934: three random items
    void refill(int days); // 80153A04: up to one new random item per day, while fewer than 10 are kept
    void push(u16 item); // 80153C14: add(); when full, drops the oldest item
    int count(); // 80153CB0
    u16 get(int slot); // 80153D34
    void set(int slot, u16 item); // 80153D40
    BOOL add(u16 item); // 80153D4C: compact(), then the first free slot (same filter as dRecycleBin_c::add)
    void compact(); // 80153E14: moves the items to the front

    /* 0x00 */ u16 mItems[POLICE_BOX_ITEM_NUM];
}; // size 0x18
