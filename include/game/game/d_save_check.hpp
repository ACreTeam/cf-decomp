#pragma once

#include <types.h>

// The check block at the start of the save file (dSaveData_c::mHeader, save + 0): a CRC32 over
// +0x04..+0x20, the save version and two state bytes. Source: src/dol/game/d_save_check.cpp
// (.text 8011524C..80115380). Names are inferred.
struct dSaveCheck_c {
    void updateChecksum();      // 8011524C
    BOOL isChecksumOK();        // 8011527C
    u32 calcChecksum();         // 801152C0
    void init();                // 801152D8: current version, state 0
    BOOL isVersionOK();         // 8011532C: mVersion is dSaveData_c::getVersion()
    BOOL isState2();            // 8011536C

    /* 0x00 */ u32 mChecksum;
    /* 0x04 */ u16 mVersion; // SAVE_VERSION
    /* 0x06 */ u8 mState;
    /* 0x07 */ u8 mFlags;    // init: 0 when lbl_80750334 is 1, else 1
    /* 0x08 */ u8 _08[0x18];
}; // size 0x20
