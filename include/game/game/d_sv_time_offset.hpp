#pragma once

#include <types.h>

// Saved game-clock offset (dSaveData_c::getTown()->mTimeOffset), see dTime_c::loadOffset / saveOffset.
// Source: src/dol/game/d_sv_time_offset.cpp (.text 8014D020..8014D0BC). Names are inferred.
enum {
    TIME_OFFSET_ADJUSTED = 1 << 0, // dTime_c's TIME_FLAG_ADJUSTED, saved
    TIME_OFFSET_CHANGED = 1 << 1,  // the clock was changed (isChanged)
    TIME_OFFSET_CHANGED2 = 1 << 2, // also tested by isChanged; never set in this TU
};

struct dSaveTimeOffset_c {
    void init();             // 8014D020: loads dTime_c's offset, clears the flags
    void clearFlags();       // 8014D054
    BOOL isAdjusted() const; // 8014D064
    BOOL isChanged() const;  // 8014D07C
    void setAdjusted();      // 8014D09C
    void setChanged();       // 8014D0AC

    /* 0x0 */ s64 mOffset; // dTime_c::sOffset
    /* 0x8 */ union {
        u8 mFlags; // TIME_OFFSET_*
        struct {   // MSB first
            u8 : 5;
            u8 mChanged2 : 1;
            u8 mChanged : 1;
            u8 mAdjusted : 1;
        };
    };
    /* 0x9 */ u8 _9[7];
}; // size 0x10
