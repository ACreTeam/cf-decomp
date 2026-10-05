#pragma once

// A point in time as OSTime ticks, stored as bytes so it can sit at 2-byte-aligned save offsets.
// TIME_STAMP_NONE (INT64_MAX) means "not set" / "no limit". Used for quest deadlines, villager
// memories, notices, the museum and the save. Source: src/dol/game/d_time_stamp.cpp
// (.text 8014C2C4..8014CBFC). Formerly dQuestTime_c; the class and function names are inferred.
// See notes/d_time_stamp.txt.

#include <types.h>
#include <game/game/d_date.hpp>

#define TIME_STAMP_NONE 0x7FFFFFFFFFFFFFFFLL

class dTimeStamp_c {
public:
    dTimeStamp_c(s64 ticks = TIME_STAMP_NONE) { set(ticks); }
    dTimeStamp_c(const dTime_c *cal) { set(cal); }

    dTime_c get() const;                                         // 8014C2C4
    s64 getRaw() const;                                          // 8014C34C
    s64 getTicks();                                              // 8014C380
    BOOL isNone();                                               // 8014C384: == TIME_STAMP_NONE
    void setTimeOfDay(int hour, int min, int sec);               // 8014C3C4: same day, msec/usec 0
    void reset();                                                // 8014C468: to TIME_STAMP_NONE
    BOOL isToday(BOOL dayStart);                                 // 8014C478: dayStart: days begin at 6:00
    void set(const dTime_c *cal);                                // 8014C538
    void set(s64 ticks);                                         // 8014C5D0
    void setNow();                                               // 8014C61C
    void addDays(int days);                                      // 8014C6A4
    void add(int days, int hours, int mins, int secs);           // 8014C758
    void toDayStart();                                           // 8014C818: 6:00 of its game day
    int getWeekday();                                            // 8014C8E4
    int diffDays(const dTime_c *cal, BOOL dayStart, BOOL adjust); // 8014C95C: cal - this
    int diffDays(s64 ticks, BOOL dayStart, BOOL adjust);         // 8014C9C4: ticks - this
    int diffDaysFromNow(BOOL dayStart, BOOL adjust);             // 8014CAA4: now - this
    int compare(const dTime_c *cal);                             // 8014CB74: -1 after cal, 0 same, 1 before

    u8 mData[8];
};
