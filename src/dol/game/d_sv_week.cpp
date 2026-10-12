// The game week and clock offset of the last update (dSvWeek::Week_c). .text 80154080..80154358.
// See include/game/game/d_sv_week.hpp.
#include <game/game/d_sv_week.hpp>
#include <game/game/d_save_data.hpp>
#include <string.h>

namespace dSvWeek {

// 80154080
void Week_c::init() {
    memset(mOffset, 0, sizeof(mOffset));
    memset(mWeek, WEEK_NONE, sizeof(mWeek));
    mTimeChanged = 0;
}

// 801540CC
s64 Week_c::getOffset() const {
    s64 offset;
    memcpy(&offset, mOffset, sizeof(offset));
    return offset;
}

// 80154100
s16 Week_c::getWeek() const {
    s16 week;
    memcpy(&week, mWeek, sizeof(week));
    return week;
}

// 80154134
void Week_c::setOffset(s64 offset) {
    memset(mOffset, 0, sizeof(mOffset));
    memcpy(mOffset, &offset, sizeof(offset));
}

// 80154180
void Week_c::setWeek(int week) {
    memset(mWeek, 0, sizeof(mWeek));
    s16 value = week;
    memcpy(mWeek, &value, sizeof(value));
}

// 801541D8
void Week_c::update() {
    s64 offset;
    int week;
    dTime_c base;
    base.set(2000, MONTH_JANUARY, 1, 0, 0, 0);
    base.normalize();
    base.add(-(base.wday - WEEKDAY_MONDAY), 0, 0, 0);
    dTime_c now = *dTime_c::getCurrent();
    week = dTime_c::diffDays(&now, &base, FALSE) / TIME_DAYS_PER_WEEK;
    offset = dSaveData_c::getTown()->mTimeOffset.mOffset;
    if (getWeek() >= 0) {
        if (week != getWeek()) {
            mTimeChanged = 0;
        }
        if (getOffset() - offset != 0) {
            mTimeChanged = 1;
        }
    }
    setWeek(week);
    setOffset(offset);
}

} // namespace dSvWeek
