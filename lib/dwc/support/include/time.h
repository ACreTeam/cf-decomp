#ifndef DWC_SUPPORT_TIME_H
#define DWC_SUPPORT_TIME_H

// time_t comes from the wii-ipl MSL stddef.h.
#include <stddef.h>

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

#endif
