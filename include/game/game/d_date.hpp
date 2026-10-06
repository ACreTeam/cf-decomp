#pragma once

#include <types.h>
#include <revolution/OS/OSTime.h>

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

union dMD_c {
    struct {
        u8 month;
        u8 day;
    };
    u16 date;
};

// Game calendar time. Same layout as OSCalendarTime (month is 0-based).
// The game clock is the console clock plus a saved tick offset; the current
// value is cached in sCurrent and refreshed by update().
// Source: src/dol/game/d_date.cpp (.text 8016D264..8016DE60). Function names are inferred.
enum {
    TIME_FLAG_ADJUSTED = 1 << 0, // year was wrapped into 2000..2035, or the saved offset says so
    TIME_FLAG_STOPPED = 1 << 1,  // update() leaves sCurrent alone
};

enum {
    TIME_OF_DAY_MORNING,   // 5:00-11:59
    TIME_OF_DAY_AFTERNOON, // 12:00-17:59
    TIME_OF_DAY_EVENING,   // 18:00-23:59
    TIME_OF_DAY_NIGHT,     // 0:00-4:59
};

enum {
    SEASON_SPRING, // Feb 25 - May
    SEASON_SUMMER, // Jun - Aug
    SEASON_FALL,   // Sep - Nov 25
    SEASON_WINTER,
};

#define TIME_YEAR_MIN 2000
#define TIME_YEAR_MAX 2036

enum dTime_Month_e {
    MONTH_JANUARY,
    MONTH_FEBRUARY,
    MONTH_MARCH,
    MONTH_APRIL,
    MONTH_MAY,
    MONTH_JUNE,
    MONTH_JULY,
    MONTH_AUGUST,
    MONTH_SEPTEMBER,
    MONTH_OCTOBER,
    MONTH_NOVEMBER,
    MONTH_DECEMBER,

    MONTH_NUM
};

#define MONTHDAY(m, d) (((m) << 8) | (d)) // month 0-based, as dTime_c::month

#define TIME_DAY_START_HOUR 6 // the game day starts at 6:00 (the day change)
#define TIME_DAYS_PER_WEEK 7

// dTime_c::getTerm(): the first i with MONTHDAY(month, mday) <= sTermDates[i] (d_date.cpp).
enum dTime_Term_e {
    TERM_JAN_01_FEB_03,
    TERM_FEB_04_FEB_17,
    TERM_FEB_18_FEB_24,
    TERM_FEB_25_MAR_31,
    TERM_APR_01_APR_03,
    TERM_APR_04_APR_08,
    TERM_APR_09_JUL_22,
    TERM_JUL_23_SEP_15,
    TERM_SEP_16_SEP_30,
    TERM_OCT_01_OCT_04,
    TERM_OCT_05_OCT_10,
    TERM_OCT_11_OCT_16,
    TERM_OCT_17_OCT_20,
    TERM_OCT_21_OCT_25,
    TERM_OCT_26_OCT_30,
    TERM_OCT_31_NOV_02,
    TERM_NOV_03_NOV_09,
    TERM_NOV_10_NOV_13,
    TERM_NOV_14_NOV_19,
    TERM_NOV_20_NOV_25,
    TERM_NOV_26_DEC_01,
    TERM_DEC_02_DEC_10,
    TERM_DEC_11_DEC_31,

    TERM_NUM
};

class dTime_c : public OSCalendarTime {
public:
    static void setFlag(u8 flag);                  // 8016D264
    static void clearFlag(u8 flag);                // 8016D274
    static BOOL isFlag(u8 flag);                   // 8016D284
    static BOOL isLeapYear(int year);              // 8016D29C
    static dTime_c *getCurrent();                  // 8016D2E8
    static s64 getCurrentTicks();                  // 8016D2F4
    static void setPendingTicks(s64 ticks);        // 8016D300: offset so that "now" becomes ticks
    static void applyPending();                    // 8016D348
    static void stop();                            // 8016D35C
    static void start();                           // 8016D364
    static void update();                          // 8016D36C
    static void setOffset(dTime_c time);           // 8016D48C: offset so that "now" becomes time
    static void loadOffset();                      // 8016D4D8: from dSaveData_c::mTimeOffset
    static void saveOffset();                      // 8016D53C: to dSaveData_c::mTimeOffset
    static void resetOffset();                     // 8016D5A0
    static BOOL isSameOrAfter(dTime_c a, dTime_c b);     // 8016D5F4: ticks(a) >= ticks(b)
    static BOOL isSameOrBeforeDay(dTime_c a, dTime_c b); // 8016D65C: date part only
    static BOOL isSameDay(dTime_c a, dTime_c b);         // 8016D6C8

    void init();                                   // 8016D708: 2000-01-01 00:00:00
    void set(int year, int month, int day, int hour, int min, int sec); // 8016D754
    void normalize();                              // 8016D784: recomputes wday/yday, carries overflow
    void add(int days, int hours, int mins, int secs); // 8016D7B8

    static int getWeekday(int year, int month, int day);            // 8016D7EC
    static int getNthWeekday(int year, int month, int wday, int nth); // 8016D83C: day of month or -1
    static u8 getDaysInMonth(int year, int month);                  // 8016D8F0
    static int diffDays(const dTime_c* a, const dTime_c* b, BOOL adjust);         // 8016D940: a - b in days
    static s64 diffTicks(const dTime_c* p_a, const dTime_c* p_b, BOOL adjust);        // 8016DAE4: a - b in ticks

    int getTimeOfDay();                            // 8016DBFC
    int getTerm();                                 // 8016DC34: index into the 24-term table
    int getSeason();                               // 8016DC84
    static int getCurrentSeason();                 // 8016DCF8
    static void setTermDate(const int *term, dTime_c *time); // 8016DD1C
    static u8 getStarSign(int month, int day);     // 8016DD48

    static s64 sOffset;        // 8074E8B0: game time - console time, in ticks
    static s64 sPendingOffset; // 8074E8B8
    static u8 sFlags;          // 8074E8C0: TIME_FLAG_*
    static dTime_c sCurrent;   // 80600898
}; // size 0x28
