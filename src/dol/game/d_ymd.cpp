// Calendar dates (dYMD_c, d_date.hpp). .text 8014CBFC..8014D020, right after d_time_stamp.cpp.
// See notes/d_ymd.txt. Function names are inferred.
#include <game/game/d_date.hpp>
#include <game/game/d_time_stamp.hpp>

// 8014CBFC
dTime_c dYMD_c::get() const {
    dTime_c t;
    t.set(year, month, day, 0, 0, 0);
    return t;
}

// 8014CC98
void dYMD_c::set(const dTime_c *time) {
    set(time->year, time->month, time->mday);
}

// 8014CCB4
void dYMD_c::set(int year, u8 month, u8 day) {
    this->year = year;
    this->month = month;
    this->day = day;
}

// 8014CCC4
void dYMD_c::setToday(BOOL dayStart) {
    dTime_c t = *dTime_c::getCurrent();
    if (dayStart) {
        t.add(0, -TIME_DAY_START_HOUR, 0, 0);
    }
    set(t.year, t.month, t.mday);
}

// 8014CD88
BOOL dYMD_c::isToday(BOOL dayStart) const {
    if (isNone()) {
        return FALSE;
    }
    if (dayStart) {
        dTimeStamp_c stamp(dTime_c::getCurrent());
        stamp.toDayStart();
        dTime_c t = stamp.get();
        return isDate(t.year, t.month, t.mday);
    } else {
        dTime_c t = *dTime_c::getCurrent();
        return isDate(t.year, t.month, t.mday);
    }
}

// 8014CF38
int dYMD_c::compare(int year, u8 month, u8 day) const {
    if (this->year > year) {
        return 1;
    }
    if (this->year == year) {
        u32 a = MONTHDAY(this->month, this->day);
        u32 b = MONTHDAY(month, day);
        if (a == b) {
            return 0;
        }
        return a > b ? 1 : -1;
    }
    return -1;
}

// 8014CF88
int dYMD_c::compare(const dYMD_c *other) const {
    return compare(other->year, other->month, other->day);
}

// 8014CF9C
int dYMD_c::compare(const dTime_c *time) const {
    return compare(time->year, time->month, time->mday);
}

// 8014CFB8
int dYMD_c::diffDays(const dTime_c *time, BOOL adjust) const {
    dTime_c t;
    t.set(year, month, day, 0, 0, 0);
    return dTime_c::diffDays(time, &t, adjust);
}
