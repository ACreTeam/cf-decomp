#ifndef _TIME_H_
#define _TIME_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Components of time (C89 4.12.1). */
#define CLOCKS_PER_SEC 1000

typedef unsigned long clock_t;
typedef signed long long time_t;

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

/* Time manipulation (C89 4.12.2). */
clock_t clock(void);
double difftime(time_t time1, time_t time0);
time_t mktime(struct tm* timeptr);
time_t time(time_t* timer);

/* Time conversion (C89 4.12.3). */
char* asctime(const struct tm* timeptr);
char* ctime(const time_t* timer);
struct tm* gmtime(const time_t* timer);
struct tm* localtime(const time_t* timer);
size_t strftime(char* s, size_t maxsize, const char* format, const struct tm* timeptr);

#ifdef __cplusplus
}
#endif

#endif
