#pragma once

#include <types.h>

struct dYMD_c {
    dYMD_c() : year(0), month(0), day(0) {}
    void clear() {
        year = 0;
        month = 0;
        day = 0;
    }

    u16 year;
    u8 month;
    u8 day;
};

struct dTime_c {
    s32 sec;
    s32 min;
    s32 hour;
    s32 day;
    s32 month;
    s32 year;
    s32 wday;
    s32 yday;
    s32 msec;
    s32 usec;
};
