// Saved game-clock offset (dSaveTimeOffset_c, d_sv_time_offset.hpp). .text 8014D020..8014D0BC.
// See notes/d_ymd.txt. Function names are inferred.
#include <game/game/d_sv_time_offset.hpp>
#include <game/game/d_date.hpp>

// 8014D020
void dSaveTimeOffset_c::init() {
    dTime_c::loadOffset();
    mFlags = 0;
}

// 8014D054
void dSaveTimeOffset_c::clearFlags() {
    mFlags &= ~(TIME_OFFSET_ADJUSTED | TIME_OFFSET_CHANGED | TIME_OFFSET_CHANGED2);
}

// 8014D064
BOOL dSaveTimeOffset_c::isAdjusted() const {
    return mAdjusted != 0;
}

// 8014D07C
BOOL dSaveTimeOffset_c::isChanged() const {
    return (mFlags & TIME_OFFSET_CHANGED) || (mFlags & TIME_OFFSET_CHANGED2);
}

// 8014D09C
void dSaveTimeOffset_c::setAdjusted() {
    mAdjusted = 1;
}

// 8014D0AC
void dSaveTimeOffset_c::setChanged() {
    mChanged = 1;
}
