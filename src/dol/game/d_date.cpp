// Game calendar time (dTime_c). .text 8016D264..8016DE60.
// First pass: written for equivalence, matching has not started.
#include <game/game/d_date.hpp>
#include <game/game/d_save_data.hpp>
#include <revolution/OS/OSTime.h>

#define MONTHDAY(m, d) (((m) << 8) | (d))

// Dependencies whose owners are not recovered yet.
extern "C" {
BOOL fn_8014D054(dSaveTimeOffset_c *offset);
BOOL fn_8014D064(dSaveTimeOffset_c *offset);
void fn_8014D09C(dSaveTimeOffset_c *offset);
void fn_8013F354(void *obj);
}

// 8047B150
static const u8 sDaysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
// 8047B15C
static const u8 sDaysInMonthLeap[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// 8074E8B0
s64 dTime_c::sOffset;
// 8074E8B8
s64 dTime_c::sPendingOffset;
// 8074E8C0
u8 dTime_c::sFlags;
// 80600898
dTime_c dTime_c::sCurrent;

// 8016D264
void dTime_c::setFlag(u8 flag) {
    sFlags |= flag;
}

// 8016D274
void dTime_c::clearFlag(u8 flag) {
    sFlags &= ~flag;
}

// 8016D284
BOOL dTime_c::isFlag(u8 flag) {
    return (sFlags & flag) != 0;
}

// 8016D29C
BOOL dTime_c::isLeapYear(int year) {
    if ((year & 3) == 0) {
        if ((year % 100) == 0) {
            // shortcut for calculating % 400 when we know that
            // year % 4 = 0 and year % 100 = 0.
            // The only case where year % 100 == 0 AND year % 16 == 0 is also year % 400.
            return (year & 0xF) == 0;
        }
        return TRUE;
    }
    return FALSE;
}

// 8016D2E8
dTime_c *dTime_c::getCurrent() {
    return &sCurrent;
}

// 8016D2F4
s64 dTime_c::getCurrentTicks() {
    return OSCalendarTimeToTicks(&sCurrent);
}

// 8016D300
void dTime_c::setPendingTicks(s64 ticks) {
    sPendingOffset = ticks - OSGetTime();
}

// 8016D348
void dTime_c::applyPending() {
    sOffset = sPendingOffset;
    update();
}

// 8016D35C
void dTime_c::stop() {
    setFlag(TIME_FLAG_STOPPED);
}

// 8016D364
void dTime_c::start() {
    clearFlag(TIME_FLAG_STOPPED);
}

// 8016D36C
void dTime_c::update() {
    if (isFlag(TIME_FLAG_STOPPED)) {
        return;
    }

    s64 time = OSGetTime();
    time += sOffset;
    OSTicksToCalendarTime(time, &sCurrent);

    BOOL changed = FALSE;
    if (sCurrent.year >= TIME_YEAR_MAX) {
        changed = TRUE;
        while (sCurrent.year >= TIME_YEAR_MAX) {
            sCurrent.year -= 36;
            setFlag(TIME_FLAG_ADJUSTED);
        }
    }
    if (sCurrent.year < TIME_YEAR_MIN) {
        sCurrent.year = TIME_YEAR_MIN;
        changed = TRUE;
    }

    if (changed) {
        sCurrent.normalize();
        setOffset(sCurrent);
    }
}

// 8016D48C
void dTime_c::setOffset(dTime_c time) {
    s64 ticks = OSCalendarTimeToTicks(&time);
    sOffset = ticks - OSGetTime();
}

// 8016D4D8
void dTime_c::loadOffset() {
    sOffset = dSaveData_c::getTown()->mTimeOffset.mOffset;
    if (fn_8014D064(&dSaveData_c::getTown()->mTimeOffset)) {
        setFlag(TIME_FLAG_ADJUSTED);
    } else {
        clearFlag(TIME_FLAG_ADJUSTED);
    }
    update();
}

// 8016D53C
void dTime_c::saveOffset() {
    dSaveData_c::getTown()->mTimeOffset.mOffset = sOffset;
    if (isFlag(TIME_FLAG_ADJUSTED)) {
        fn_8014D09C(&dSaveData_c::getTown()->mTimeOffset);
    }
}

// 8016D5A0
void dTime_c::resetOffset() {
    if (isFlag(TIME_FLAG_ADJUSTED)) {
        fn_8013F354(dSaveData_c::getExtra()->_189C38);
    }
    fn_8014D054(&dSaveData_c::getTown()->mTimeOffset);
    clearFlag(TIME_FLAG_ADJUSTED);
}

// 8016D5F4
BOOL dTime_c::isSameOrAfter(dTime_c a, dTime_c b) {
    s64 ticks_a = OSCalendarTimeToTicks(&a);
    s64 ticks_b = OSCalendarTimeToTicks(&b);
    return ticks_a >= ticks_b;
}

// 8016D65C
BOOL dTime_c::isSameOrBeforeDay(dTime_c a, dTime_c b) {
    if (a.year == b.year) {
        if (a.month == b.month) {
            return a.mday >= b.mday;
        } else {
            return a.month > b.month;
        }
    } else {
        return a.year > b.year;
    }
}

// 8016D6C8
BOOL dTime_c::isSameDay(dTime_c a, dTime_c b) {
    BOOL same = FALSE;
    if (a.year == b.year && a.month == b.month && a.mday == b.mday) {
        same = TRUE;
    }
    return same;
}

// 8016D708
void dTime_c::init() {
    set(TIME_YEAR_MIN, 0, 1, 0, 0, 0);
    normalize();
}

// 8016D754
void dTime_c::set(int year, int month, int day, int hour, int min, int sec) {
    this->year = year;
    this->month = month;
    this->mday = day;
    this->hour = hour;
    this->min = min;
    this->sec = sec;
    msec = 0;
    usec = 0;
    yday = -1;
}

// 8016D784
void dTime_c::normalize() {
    OSTicksToCalendarTime(OSCalendarTimeToTicks(this), this);
}

// 8016D7B8
void dTime_c::add(int days, int hours, int mins, int secs) {
    mday += days;
    hour += hours;
    min += mins;
    sec += secs;
    normalize();
}

// 8016D7EC
int dTime_c::getWeekday(int year, int month, int day) {
    dTime_c time;
    time.set(year, month, day, 0, 0, 0);
    time.normalize();
    return time.wday;
}

// 8016D83C
int dTime_c::getNthWeekday(int year, int month, int wday, int nth) {
    dTime_c time;
    time.set(year, month, 1, 0, 0, 0);
    time.normalize();

    int diff = wday - time.wday;
    if (diff < 0) {
        diff += 7;
    }
    time.add(diff + (nth - 1) * 7, 0, 0, 0);

    if (time.month != month) {
        return -1;
    }
    return time.mday;
}

// 8016D8F0
u8 dTime_c::getDaysInMonth(int year, int month) {
    if (isLeapYear(year)) {
        return sDaysInMonthLeap[month];
    }
    return sDaysInMonth[month];
}

// 8016D940
int dTime_c::diffDays(const dTime_c* p_a, const dTime_c* p_b, BOOL adjust) {
    dTime_c a = *p_a;
    dTime_c b = *p_b;
    if (adjust && isFlag(TIME_FLAG_ADJUSTED)) {
        a.year += 36;
        a.normalize();
    }
    if (a.yday == -1) {
        a.normalize();
    }
    if (b.yday == -1) {
        b.normalize();
    }

    int days = 0;
    while (a.year > b.year) {
        int add_days;
        
        if (isLeapYear(b.year)) {
            days += 366;
        } else {
            days += 365;
        }

        b.year++;
    }
    while (a.year < b.year) {
        if (isLeapYear(a.year)) {
            days -= 366;
        } else {
            days -= 365;
        }
        a.year++;
    }

    return days + (a.yday - b.yday);
}

// 8016DAE4
s64 dTime_c::diffTicks(const dTime_c* p_a, const dTime_c* p_b, BOOL adjust) {
    dTime_c a = *p_a;
    dTime_c b = *p_b;
    if (adjust && isFlag(TIME_FLAG_ADJUSTED)) {
        a.year += 36;
        a.normalize();
    }
    return OSCalendarTimeToTicks(&a) - OSCalendarTimeToTicks(&b);
}

// 8016DBFC
int dTime_c::getTimeOfDay() {
    if (hour < 5) {
        return TIME_OF_DAY_NIGHT;
    }
    if (hour < 12) {
        return TIME_OF_DAY_MORNING;
    }

    return hour < 18 ? TIME_OF_DAY_AFTERNOON : TIME_OF_DAY_EVENING;
}

// 8047B168: period boundaries as (month << 8) | day, month 0-based.
static const u16 sTermDates[] = {
    MONTHDAY(MONTH_FEBRUARY, 3),
    MONTHDAY(MONTH_FEBRUARY, 17),
    MONTHDAY(MONTH_FEBRUARY, 24),
    MONTHDAY(MONTH_MARCH, 31),
    MONTHDAY(MONTH_APRIL, 3),
    MONTHDAY(MONTH_APRIL, 8),
    MONTHDAY(MONTH_JULY, 22),
    MONTHDAY(MONTH_SEPTEMBER, 15),
    MONTHDAY(MONTH_SEPTEMBER, 30),
    MONTHDAY(MONTH_OCTOBER, 4),
    MONTHDAY(MONTH_OCTOBER, 10),
    MONTHDAY(MONTH_OCTOBER, 16),
    MONTHDAY(MONTH_OCTOBER, 20),
    MONTHDAY(MONTH_OCTOBER, 25),
    MONTHDAY(MONTH_OCTOBER, 30),
    MONTHDAY(MONTH_NOVEMBER, 2),
    MONTHDAY(MONTH_NOVEMBER, 9),
    MONTHDAY(MONTH_NOVEMBER, 13),
    MONTHDAY(MONTH_NOVEMBER, 19),
    MONTHDAY(MONTH_NOVEMBER, 25),
    MONTHDAY(MONTH_DECEMBER, 1),
    MONTHDAY(MONTH_DECEMBER, 10),
    MONTHDAY(MONTH_DECEMBER, 31),
};


// 8016DC34
int dTime_c::getTerm() {
    u16 key = MONTHDAY(month, mday);
    int ret = 0;
    for (int i = 0; i < ARRAY_SIZE(sTermDates); i++) {
        if (key <= sTermDates[i]) {
            ret = i;
            break;
        }
    }

    return ret;
}

// 8016DC84
int dTime_c::getSeason() {
    if ((month == MONTH_FEBRUARY && mday >= 25) || (month == MONTH_MARCH || month == MONTH_APRIL || month == MONTH_MAY)) {
        return SEASON_SPRING;
    }
    if (month == MONTH_JUNE || month == MONTH_JULY || month == MONTH_AUGUST) {
        return SEASON_SUMMER;
    }
    if (month == MONTH_SEPTEMBER || month == MONTH_OCTOBER || (month == MONTH_NOVEMBER && mday <= 25)) {
        return SEASON_FALL;
    }
    return SEASON_WINTER;
}

// 8016DCF8
int dTime_c::getCurrentSeason() {
    return getCurrent()->getSeason();
}

// 8016DD1C
void dTime_c::setTermDate(const int *term, dTime_c *time) {
    u16 date = sTermDates[*term];
    time->month = date >> 8;
    time->mday = date & 0xFF;
    time->normalize();
}

// 8016DD48
u8 dTime_c::getStarSign(int month, int day) {
    // 8047B198: last (month, day) of each star sign, starting with Capricorn.
    static const dMD_c sStarSignEnds[] = {
        {MONTH_JANUARY, 19},
        {MONTH_FEBRUARY, 18},
        {MONTH_MARCH, 20},
        {MONTH_APRIL, 19},
        {MONTH_MAY, 20},
        {MONTH_JUNE, 21},
        {MONTH_JULY, 22},
        {MONTH_AUGUST, 22},
        {MONTH_SEPTEMBER, 22},
        {MONTH_OCTOBER, 23},
        {MONTH_NOVEMBER, 22},
        {MONTH_DECEMBER, 21},
    };

    const dMD_c* monthday_p = sStarSignEnds;
    int sign = 0;
    for (int i = 0; i < ARRAY_SIZE(sStarSignEnds); i++, monthday_p++) {
        if (month < monthday_p->month || (month == monthday_p->month && day <= monthday_p->day)) {
            sign = i;
            break;
        }
    }
    return sign;
}
