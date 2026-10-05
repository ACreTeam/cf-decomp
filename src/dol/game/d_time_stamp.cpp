// OSTime timestamp stored as bytes (formerly dQuestTime_c). See notes/d_time_stamp.txt.
// .text 8014C2C4..8014CBFC; no data.
#include <game/game/d_time_stamp.hpp>
#include <string.h>

// dTime_c::diffDays on temporaries: the results of get() go straight into the call.
static inline int diffCalDays(const dTime_c &a, const dTime_c &b, BOOL adjust) {
    return dTime_c::diffDays(&a, &b, adjust);
}

// 8014C2C4
dTime_c dTimeStamp_c::get() const {
    dTime_c cal;
    OSTicksToCalendarTime(getRaw(), &cal);
    return cal;
}

// 8014C34C
s64 dTimeStamp_c::getRaw() const {
    s64 ticks;
    memcpy(&ticks, mData, sizeof(ticks));
    return ticks;
}

// 8014C380
s64 dTimeStamp_c::getTicks() {
    return getRaw();
}

// 8014C384
BOOL dTimeStamp_c::isNone() {
    return getRaw() == TIME_STAMP_NONE;
}

// 8014C3C4
void dTimeStamp_c::setTimeOfDay(int hour, int min, int sec) {
    dTime_c cal = get();
    cal.hour = hour;
    cal.min = min;
    cal.sec = sec;
    cal.usec = 0;
    cal.msec = 0;
    set(&cal);
}

// 8014C468
void dTimeStamp_c::reset() {
    set(TIME_STAMP_NONE);
}

// 8014C478
BOOL dTimeStamp_c::isToday(BOOL dayStart) {
    if (dayStart) {
        dTimeStamp_c now(dTime_c::getCurrent());
        dTimeStamp_c self(getTicks());
        now.toDayStart();
        self.toDayStart();
        return diffCalDays(now.get(), self.get(), FALSE) == 0;
    }
    return diffCalDays(*dTime_c::getCurrent(), get(), FALSE) == 0;
}

// 8014C538
void dTimeStamp_c::set(const dTime_c *cal) {
    dTime_c copy = *cal;
    set(OSCalendarTimeToTicks(&copy));
}

// 8014C5D0
void dTimeStamp_c::set(s64 ticks) {
    memset(mData, 0, sizeof(mData));
    memcpy(mData, &ticks, sizeof(mData));
}

// 8014C61C
void dTimeStamp_c::setNow() {
    dTime_c cal = *dTime_c::getCurrent();
    set(&cal);
}

// 8014C6A4
void dTimeStamp_c::addDays(int days) {
    dTime_c cal = get();
    cal.add(days, 0, 0, 0);
    set(&cal);
}

// 8014C758
void dTimeStamp_c::add(int days, int hours, int mins, int secs) {
    dTime_c cal = get();
    cal.add(days, hours, mins, secs);
    set(&cal);
}

// 8014C818
void dTimeStamp_c::toDayStart() {
    dTime_c cal = get();
    cal.add(0, -6, 0, 0);
    cal.hour = 6;
    cal.usec = 0;
    cal.msec = 0;
    cal.sec = 0;
    cal.min = 0;
    set(&cal);
}

// 8014C8E4
int dTimeStamp_c::getWeekday() {
    dTime_c cal = get();
    return cal.wday;
}

// 8014C95C
int dTimeStamp_c::diffDays(const dTime_c *cal, BOOL dayStart, BOOL adjust) {
    dTimeStamp_c other(cal);
    return diffDays(other.getTicks(), dayStart, adjust);
}

// 8014C9C4
int dTimeStamp_c::diffDays(s64 ticks, BOOL dayStart, BOOL adjust) {
    if (dayStart) {
        dTimeStamp_c self(getTicks());
        dTimeStamp_c other(ticks);
        self.toDayStart();
        other.toDayStart();
        return diffCalDays(other.get(), self.get(), adjust);
    }
    dTime_c cal;
    OSTicksToCalendarTime(ticks, &cal);
    return diffCalDays(cal, get(), adjust);
}

// 8014CAA4
int dTimeStamp_c::diffDaysFromNow(BOOL dayStart, BOOL adjust) {
    dTime_c cal = *dTime_c::getCurrent();
    dTimeStamp_c now(&cal);
    if (dayStart) {
        now.toDayStart();
    }
    return diffDays(now.getTicks(), dayStart, adjust);
}

// 8014CB74
int dTimeStamp_c::compare(const dTime_c *cal) {
    s64 other = OSCalendarTimeToTicks(cal);
    s64 ticks = getRaw();
    if (ticks > other) {
        return -1;
    }
    return other != ticks;
}
