#pragma once

#include <types.h>

#define SAVE_MELODY_NOTE_NUM 16

// The town tune: 16 notes. Kept in the save data (dSaveData_c::mVillageMelody) and by each villager
// (dAnimal_c::mMelody). Source: src/dol/game/d_save_melody.cpp (.text 8011927C..80119444).
class dSaveMelody_c { // 0x10
public:
    dSaveMelody_c(); // 8011927C
    ~dSaveMelody_c(); // 80119280
    dSaveMelody_c *copy(const dSaveMelody_c *other); // 801192C0
    void clear(); // 801192F4: the default tune
    void set(const u8 *notes); // 80119338
    void get(u8 *notes); // 801193BC
    u8 *getNotes(); // 80119440

    /* 0x00 */ u8 mNotes[SAVE_MELODY_NOTE_NUM];
};
