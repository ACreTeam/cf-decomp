// Town event schedule and state (AC m_event.c counterpart). See notes/d_event.txt.
// .text 80088BCC..8008AEF0, .rodata 8046E340..8046EEA8, .data 804DF968..804DF990 (switch table),
// .bss 80585368..805853E0, .sdata 80749ED0..80749EE0 (literal event-id temporaries),
// .sbss 8074E2E0..8074E2E8.
#include <game/game/d_event.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_time_stamp.hpp>
#include <game/cLib/c_math.hpp>
#include <revolution/SC/scapi.h>

// 8046E340: which Saturday of each month the fishing tourney is on (-1: none).
static const int sFishingTourneySaturday[MONTH_NUM] = {3, 2, 3, 2, 3, -1, -1, -1, -1, 2, 3, 2};

// 8046E370: Festivale (carnival)
static const dEventYearDates_t sFestivaleDates = {
    MONTH_MARCH,     6, // 2000
    MONTH_FEBRUARY, 26, // 2001
    MONTH_FEBRUARY, 11, // 2002
    MONTH_MARCH,     3, // 2003
    MONTH_FEBRUARY, 23, // 2004
    MONTH_FEBRUARY,  7, // 2005
    MONTH_FEBRUARY, 27, // 2006
    MONTH_FEBRUARY, 19, // 2007
    MONTH_FEBRUARY,  4, // 2008
    MONTH_FEBRUARY, 23, // 2009
    MONTH_FEBRUARY, 15, // 2010
    MONTH_MARCH,     7, // 2011
    MONTH_FEBRUARY, 20, // 2012
    MONTH_FEBRUARY, 11, // 2013
    MONTH_MARCH,     3, // 2014
    MONTH_FEBRUARY, 16, // 2015
    MONTH_FEBRUARY,  8, // 2016
    MONTH_FEBRUARY, 27, // 2017
    MONTH_FEBRUARY, 12, // 2018
    MONTH_MARCH,     4, // 2019
    MONTH_FEBRUARY, 24, // 2020
    MONTH_FEBRUARY, 15, // 2021
    MONTH_FEBRUARY, 28, // 2022
    MONTH_FEBRUARY, 20, // 2023
    MONTH_FEBRUARY, 12, // 2024
    MONTH_MARCH,     3, // 2025
    MONTH_FEBRUARY, 16, // 2026
    MONTH_FEBRUARY,  8, // 2027
    MONTH_FEBRUARY, 28, // 2028
    MONTH_FEBRUARY, 12, // 2029
    MONTH_MARCH,     4, // 2030
    MONTH_FEBRUARY, 24, // 2031
    MONTH_FEBRUARY,  9, // 2032
    MONTH_FEBRUARY, 28, // 2033
    MONTH_FEBRUARY, 20, // 2034
    MONTH_FEBRUARY,  5, // 2035
};

// 8046E490: Bunny Day (Easter)
static const dEventYearDates_t sBunnyDayDates = {
    MONTH_APRIL, 23, // 2000
    MONTH_APRIL, 15, // 2001
    MONTH_MARCH, 31, // 2002
    MONTH_APRIL, 20, // 2003
    MONTH_APRIL, 11, // 2004
    MONTH_MARCH, 27, // 2005
    MONTH_APRIL, 16, // 2006
    MONTH_APRIL,  8, // 2007
    MONTH_MARCH, 23, // 2008
    MONTH_APRIL, 12, // 2009
    MONTH_APRIL,  4, // 2010
    MONTH_APRIL, 24, // 2011
    MONTH_APRIL,  8, // 2012
    MONTH_MARCH, 31, // 2013
    MONTH_APRIL, 20, // 2014
    MONTH_APRIL,  5, // 2015
    MONTH_MARCH, 27, // 2016
    MONTH_APRIL, 16, // 2017
    MONTH_APRIL,  1, // 2018
    MONTH_APRIL, 21, // 2019
    MONTH_APRIL, 12, // 2020
    MONTH_APRIL,  4, // 2021
    MONTH_APRIL, 17, // 2022
    MONTH_APRIL,  9, // 2023
    MONTH_MARCH, 31, // 2024
    MONTH_APRIL, 20, // 2025
    MONTH_APRIL,  5, // 2026
    MONTH_MARCH, 28, // 2027
    MONTH_APRIL, 16, // 2028
    MONTH_APRIL,  1, // 2029
    MONTH_APRIL, 21, // 2030
    MONTH_APRIL, 13, // 2031
    MONTH_MARCH, 28, // 2032
    MONTH_APRIL, 17, // 2033
    MONTH_APRIL,  9, // 2034
    MONTH_MARCH, 25, // 2035
};

// 8046E5B0: Tsukimi
static const dEventYearDates_t sJpAutumnMoonDates = {
    MONTH_SEPTEMBER, 12, // 2000
    MONTH_OCTOBER,    1, // 2001
    MONTH_SEPTEMBER, 21, // 2002
    MONTH_SEPTEMBER, 11, // 2003
    MONTH_SEPTEMBER, 28, // 2004
    MONTH_SEPTEMBER, 18, // 2005
    MONTH_OCTOBER,    6, // 2006
    MONTH_SEPTEMBER, 25, // 2007
    MONTH_SEPTEMBER, 14, // 2008
    MONTH_OCTOBER,    3, // 2009
    MONTH_SEPTEMBER, 22, // 2010
    MONTH_SEPTEMBER, 12, // 2011
    MONTH_SEPTEMBER, 30, // 2012
    MONTH_SEPTEMBER, 19, // 2013
    MONTH_SEPTEMBER,  8, // 2014
    MONTH_SEPTEMBER, 27, // 2015
    MONTH_SEPTEMBER, 15, // 2016
    MONTH_OCTOBER,    4, // 2017
    MONTH_SEPTEMBER, 24, // 2018
    MONTH_SEPTEMBER, 13, // 2019
    MONTH_OCTOBER,    1, // 2020
    MONTH_SEPTEMBER, 21, // 2021
    MONTH_SEPTEMBER, 10, // 2022
    MONTH_SEPTEMBER, 29, // 2023
    MONTH_SEPTEMBER, 17, // 2024
    MONTH_OCTOBER,    6, // 2025
    MONTH_SEPTEMBER, 25, // 2026
    MONTH_SEPTEMBER, 15, // 2027
    MONTH_OCTOBER,    3, // 2028
    MONTH_SEPTEMBER, 22, // 2029
    MONTH_SEPTEMBER, 12, // 2030
    MONTH_OCTOBER,    1, // 2031
    MONTH_SEPTEMBER, 19, // 2032
    MONTH_SEPTEMBER,  6, // 2033
    MONTH_SEPTEMBER, 27, // 2034
    MONTH_SEPTEMBER, 16, // 2035
};

// 8046E6D0: harvest moon
static const dEventYearDates_t sHarvestMoonDates = {
    MONTH_SEPTEMBER, 13, // 2000
    MONTH_OCTOBER,    2, // 2001
    MONTH_SEPTEMBER, 21, // 2002
    MONTH_SEPTEMBER, 10, // 2003
    MONTH_SEPTEMBER, 28, // 2004
    MONTH_SEPTEMBER, 18, // 2005
    MONTH_OCTOBER,    7, // 2006
    MONTH_SEPTEMBER, 26, // 2007
    MONTH_SEPTEMBER, 15, // 2008
    MONTH_OCTOBER,    4, // 2009
    MONTH_SEPTEMBER, 23, // 2010
    MONTH_SEPTEMBER, 12, // 2011
    MONTH_SEPTEMBER, 30, // 2012
    MONTH_SEPTEMBER, 19, // 2013
    MONTH_SEPTEMBER,  9, // 2014
    MONTH_SEPTEMBER, 28, // 2015
    MONTH_SEPTEMBER, 16, // 2016
    MONTH_OCTOBER,    5, // 2017
    MONTH_SEPTEMBER, 25, // 2018
    MONTH_SEPTEMBER, 14, // 2019
    MONTH_OCTOBER,    1, // 2020
    MONTH_SEPTEMBER, 20, // 2021
    MONTH_SEPTEMBER, 10, // 2022
    MONTH_SEPTEMBER, 29, // 2023
    MONTH_SEPTEMBER, 18, // 2024
    MONTH_OCTOBER,    7, // 2025
    MONTH_SEPTEMBER, 26, // 2026
    MONTH_SEPTEMBER, 15, // 2027
    MONTH_OCTOBER,    3, // 2028
    MONTH_SEPTEMBER, 22, // 2029
    MONTH_SEPTEMBER, 11, // 2030
    MONTH_SEPTEMBER, 30, // 2031
    MONTH_SEPTEMBER, 19, // 2032
    MONTH_SEPTEMBER,  9, // 2033
    MONTH_SEPTEMBER, 28, // 2034
    MONTH_SEPTEMBER, 17, // 2035
};

// 8046E7F0: Korean lunar new year
static const dEventYearDates_t sLunarNewYearDates = {
    MONTH_FEBRUARY,  5, // 2000
    MONTH_JANUARY,  24, // 2001
    MONTH_FEBRUARY, 12, // 2002
    MONTH_FEBRUARY,  1, // 2003
    MONTH_JANUARY,  22, // 2004
    MONTH_FEBRUARY,  9, // 2005
    MONTH_JANUARY,  29, // 2006
    MONTH_FEBRUARY, 18, // 2007
    MONTH_FEBRUARY,  7, // 2008
    MONTH_JANUARY,  26, // 2009
    MONTH_FEBRUARY, 14, // 2010
    MONTH_FEBRUARY,  3, // 2011
    MONTH_JANUARY,  23, // 2012
    MONTH_FEBRUARY, 10, // 2013
    MONTH_JANUARY,  31, // 2014
    MONTH_FEBRUARY, 19, // 2015
    MONTH_FEBRUARY,  8, // 2016
    MONTH_JANUARY,  28, // 2017
    MONTH_FEBRUARY, 16, // 2018
    MONTH_FEBRUARY,  5, // 2019
    MONTH_JANUARY,  25, // 2020
    MONTH_FEBRUARY, 12, // 2021
    MONTH_FEBRUARY,  1, // 2022
    MONTH_JANUARY,  22, // 2023
    MONTH_FEBRUARY, 10, // 2024
    MONTH_JANUARY,  29, // 2025
    MONTH_FEBRUARY, 17, // 2026
    MONTH_FEBRUARY,  7, // 2027
    MONTH_JANUARY,  27, // 2028
    MONTH_FEBRUARY, 13, // 2029
    MONTH_FEBRUARY,  3, // 2030
    MONTH_JANUARY,  23, // 2031
    MONTH_FEBRUARY, 11, // 2032
    MONTH_JANUARY,  31, // 2033
    MONTH_FEBRUARY, 19, // 2034
    MONTH_FEBRUARY,  8, // 2035
};

// 8046E910: UK Mothering Sunday
static const dEventYearDates_t sMotheringSundayDates = {
    MONTH_APRIL,  5, // 2000
    MONTH_MARCH, 25, // 2001
    MONTH_MARCH, 10, // 2002
    MONTH_MARCH, 30, // 2003
    MONTH_MARCH, 21, // 2004
    MONTH_MARCH,  5, // 2005
    MONTH_MARCH, 26, // 2006
    MONTH_MARCH, 18, // 2007
    MONTH_MARCH,  2, // 2008
    MONTH_MARCH, 22, // 2009
    MONTH_MARCH, 14, // 2010
    MONTH_APRIL,  3, // 2011
    MONTH_MARCH, 18, // 2012
    MONTH_MARCH, 10, // 2013
    MONTH_MARCH, 30, // 2014
    MONTH_MARCH, 15, // 2015
    MONTH_MARCH,  6, // 2016
    MONTH_MARCH, 26, // 2017
    MONTH_MARCH, 11, // 2018
    MONTH_MARCH, 31, // 2019
    MONTH_MARCH, 22, // 2020
    MONTH_MARCH, 14, // 2021
    MONTH_MARCH, 27, // 2022
    MONTH_MARCH, 19, // 2023
    MONTH_MARCH, 10, // 2024
    MONTH_MARCH, 30, // 2025
    MONTH_MARCH, 22, // 2026
    MONTH_MARCH,  7, // 2027
    MONTH_MARCH, 26, // 2028
    MONTH_MARCH, 11, // 2029
    MONTH_MARCH, 31, // 2030
    MONTH_MARCH, 23, // 2031
    MONTH_MARCH,  7, // 2032
    MONTH_MARCH, 27, // 2033
    MONTH_MARCH, 19, // 2034
    MONTH_MARCH,  4, // 2035
};

// 8046EA30: French Mother's Day
static const dEventYearDates_t sFrMothersDayDates = {
    MONTH_MAY,  28, // 2000
    MONTH_MAY,  27, // 2001
    MONTH_MAY,  26, // 2002
    MONTH_MAY,  25, // 2003
    MONTH_JUNE,  6, // 2004
    MONTH_MAY,  29, // 2005
    MONTH_MAY,  28, // 2006
    MONTH_JUNE,  3, // 2007
    MONTH_MAY,  25, // 2008
    MONTH_JUNE,  7, // 2009
    MONTH_MAY,  30, // 2010
    MONTH_MAY,  29, // 2011
    MONTH_JUNE,  3, // 2012
    MONTH_MAY,  26, // 2013
    MONTH_MAY,  25, // 2014
    MONTH_MAY,  31, // 2015
    MONTH_MAY,  29, // 2016
    MONTH_MAY,  28, // 2017
    MONTH_MAY,  27, // 2018
    MONTH_MAY,  26, // 2019
    MONTH_JUNE,  7, // 2020
    MONTH_MAY,  30, // 2021
    MONTH_MAY,  29, // 2022
    MONTH_JUNE,  4, // 2023
    MONTH_MAY,  26, // 2024
    MONTH_MAY,  25, // 2025
    MONTH_MAY,  31, // 2026
    MONTH_MAY,  30, // 2027
    MONTH_MAY,  28, // 2028
    MONTH_MAY,  27, // 2029
    MONTH_MAY,  26, // 2030
    MONTH_MAY,  25, // 2031
    MONTH_MAY,  30, // 2032
    MONTH_MAY,  29, // 2033
    MONTH_JUNE,  4, // 2034
    MONTH_MAY,  27, // 2035
};

// 8046EB50: Ascension Day (German Father's Day)
static const dEventYearDates_t sAscensionDayDates = {
    MONTH_JUNE,  1, // 2000
    MONTH_MAY,  24, // 2001
    MONTH_MAY,   9, // 2002
    MONTH_MAY,  29, // 2003
    MONTH_MAY,  20, // 2004
    MONTH_MAY,   5, // 2005
    MONTH_MAY,  25, // 2006
    MONTH_MAY,  17, // 2007
    MONTH_MAY,   1, // 2008
    MONTH_MAY,  21, // 2009
    MONTH_MAY,  13, // 2010
    MONTH_JUNE,  2, // 2011
    MONTH_MAY,  17, // 2012
    MONTH_MAY,   9, // 2013
    MONTH_MAY,  29, // 2014
    MONTH_MAY,  14, // 2015
    MONTH_MAY,   5, // 2016
    MONTH_MAY,  25, // 2017
    MONTH_MAY,  10, // 2018
    MONTH_MAY,  30, // 2019
    MONTH_MAY,  21, // 2020
    MONTH_MAY,  13, // 2021
    MONTH_MAY,  26, // 2022
    MONTH_MAY,  18, // 2023
    MONTH_MAY,   9, // 2024
    MONTH_MAY,  29, // 2025
    MONTH_MAY,  14, // 2026
    MONTH_MAY,   6, // 2027
    MONTH_MAY,  25, // 2028
    MONTH_MAY,  10, // 2029
    MONTH_MAY,  30, // 2030
    MONTH_MAY,  22, // 2031
    MONTH_MAY,   6, // 2032
    MONTH_MAY,  26, // 2033
    MONTH_MAY,  18, // 2034
    MONTH_MAY,   3, // 2035
};

// 8046EC70
static const dEventInfo_c sEventInfo[EVENT_NUM] = {
    {dEvent::isAnimalBirthday, EVENT_REGIONS_ALL, EVENT_KIND_BIRTHDAY, 6, 0, 0, 0}, // EVENT_ANIMAL_BIRTHDAY_0
    {dEvent::isAnimalBirthday, EVENT_REGIONS_ALL, EVENT_KIND_BIRTHDAY, 6, 0, 1, 0}, // EVENT_ANIMAL_BIRTHDAY_1
    {dEvent::isAnimalBirthday, EVENT_REGIONS_ALL, EVENT_KIND_BIRTHDAY, 6, 0, 2, 0}, // EVENT_ANIMAL_BIRTHDAY_2
    {dEvent::isAnimalBirthday, EVENT_REGIONS_ALL, EVENT_KIND_BIRTHDAY, 6, 0, 3, 0}, // EVENT_ANIMAL_BIRTHDAY_3
    {dEvent::isAnimalBirthday, EVENT_REGIONS_ALL, EVENT_KIND_BIRTHDAY, 6, 0, 4, 0}, // EVENT_ANIMAL_BIRTHDAY_4
    {dEvent::isAnimalBirthday, EVENT_REGIONS_ALL, EVENT_KIND_BIRTHDAY, 6, 0, 5, 0}, // EVENT_ANIMAL_BIRTHDAY_5
    {dEvent::isAnimalBirthday, EVENT_REGIONS_ALL, EVENT_KIND_BIRTHDAY, 6, 0, 6, 0}, // EVENT_ANIMAL_BIRTHDAY_6
    {dEvent::isAnimalBirthday, EVENT_REGIONS_ALL, EVENT_KIND_BIRTHDAY, 6, 0, 7, 0}, // EVENT_ANIMAL_BIRTHDAY_7
    {dEvent::isAnimalBirthday, EVENT_REGIONS_ALL, EVENT_KIND_BIRTHDAY, 6, 0, 8, 0}, // EVENT_ANIMAL_BIRTHDAY_8
    {dEvent::isAnimalBirthday, EVENT_REGIONS_ALL, EVENT_KIND_BIRTHDAY, 6, 0, 9, 0}, // EVENT_ANIMAL_BIRTHDAY_9
    {dEvent::isPlayerBirthday, EVENT_REGIONS_ALL, EVENT_KIND_BIRTHDAY, 6, 0, 0, 0}, // EVENT_PLAYER_BIRTHDAY_0
    {dEvent::isPlayerBirthday, EVENT_REGIONS_ALL, EVENT_KIND_BIRTHDAY, 6, 0, 1, 0}, // EVENT_PLAYER_BIRTHDAY_1
    {dEvent::isPlayerBirthday, EVENT_REGIONS_ALL, EVENT_KIND_BIRTHDAY, 6, 0, 2, 0}, // EVENT_PLAYER_BIRTHDAY_2
    {dEvent::isPlayerBirthday, EVENT_REGIONS_ALL, EVENT_KIND_BIRTHDAY, 6, 0, 3, 0}, // EVENT_PLAYER_BIRTHDAY_3
    {dEvent::isFishingTourney, EVENT_REGIONS_ALL, EVENT_KIND_TOWN, 9, 18, 0, 0}, // EVENT_FISHING_TOURNEY
    {dEvent::isBugOff, EVENT_REGIONS_ALL, EVENT_KIND_TOWN, 9, 18, 0, 0}, // EVENT_BUG_OFF
    {dEvent::isFireworks, EVENT_REGIONS_ALL, EVENT_KIND_TOWN, 19, 0, 0, 0}, // EVENT_FIREWORKS
    {dEvent::isFleaMarket, EVENT_REGIONS_ALL, EVENT_KIND_TOWN, 0, 24, 0, 0}, // EVENT_FLEA_MARKET
    {dEvent::isFixedDate, EVENT_REGIONS_ALL, EVENT_KIND_TOWN, 6, 0, 2, 14}, // EVENT_VALENTINES_DAY
    {dEvent::isFixedDate, EVENT_REGIONS_ALL, EVENT_KIND_TOWN, 23, 2, 12, 31}, // EVENT_COUNTDOWN
    {dEvent::isFixedDate, EVENT_REGIONS_ALL, EVENT_KIND_TOWN, 6, 0, 1, 1}, // EVENT_NEW_YEARS_DAY
    {dEvent::isFixedDate, EVENT_REGIONS_ALL, EVENT_KIND_TOWN, 18, 0, 10, 31}, // EVENT_HALLOWEEN
    {dEvent::isHarvestFestival, EVENT_REGIONS_ALL, EVENT_KIND_TOWN, 15, 21, 0, 0}, // EVENT_HARVEST_FESTIVAL
    {dEvent::isFixedDate, EVENT_REGIONS_ALL, EVENT_KIND_TOWN, 20, 0, 12, 24}, // EVENT_TOY_DAY
    {dEvent::isFestivale, EVENT_REGIONS_ALL, EVENT_KIND_TOWN, 0, 24, 0, 0}, // EVENT_FESTIVALE
    {dEvent::isBunnyDay, EVENT_REGIONS_ALL, EVENT_KIND_TOWN, 0, 24, 0, 0}, // EVENT_BUNNY_DAY
    {dEvent::isFixedDate, EVENT_REGIONS_JP, EVENT_KIND_HOLIDAY, 6, 0, 2, 3}, // EVENT_JP_SETSUBUN
    {dEvent::isFixedDate, EVENT_REGIONS_JP, EVENT_KIND_HOLIDAY, 6, 0, 3, 3}, // EVENT_JP_GIRLS_DAY
    {dEvent::isFixedDate, EVENT_REGIONS_JP, EVENT_KIND_HOLIDAY, 6, 0, 5, 5}, // EVENT_JP_CHILDRENS_DAY
    {dEvent::isJpAutumnMoon, EVENT_REGIONS_JP, EVENT_KIND_HOLIDAY, 6, 0, 0, 0}, // EVENT_JP_AUTUMN_MOON
    {dEvent::isFixedDate, EVENT_REGIONS_JP, EVENT_KIND_HOLIDAY, 6, 0, 7, 7}, // EVENT_JP_TANABATA
    {dEvent::isFixedDate, EVENT_REGIONS_NA, EVENT_KIND_HOLIDAY, 6, 0, 2, 2}, // EVENT_NA_GROUNDHOG_DAY
    {dEvent::isFixedDate, EVENT_REGIONS_NA, EVENT_KIND_HOLIDAY, 6, 0, 4, 22}, // EVENT_NA_NATURE_DAY
    {dEvent::isLaborDay, EVENT_REGIONS_NA, EVENT_KIND_HOLIDAY, 6, 0, 0, 0}, // EVENT_NA_LABOR_DAY
    {dEvent::isExplorersDay, EVENT_REGIONS_NA, EVENT_KIND_HOLIDAY, 6, 0, 0, 0}, // EVENT_NA_EXPLORERS_DAY
    {dEvent::isAutumnMoon, EVENT_REGIONS_NA, EVENT_KIND_HOLIDAY, 6, 0, 0, 0}, // EVENT_NA_AUTUMN_MOON
    {dEvent::isFixedDate, EVENT_REGIONS_EU, EVENT_KIND_HOLIDAY, 6, 0, 6, 21}, // EVENT_EU_MIDSUMMERS_DAY
    {dEvent::isFixedDate, EVENT_REGIONS_EU, EVENT_KIND_HOLIDAY, 6, 0, 12, 6}, // EVENT_EU_NAUGHTY_OR_NICE_DAY
    {dEvent::isFixedDate, EVENT_REGIONS_EU, EVENT_KIND_HOLIDAY, 6, 0, 12, 21}, // EVENT_EU_MIDWINTERS_DAY
    {dEvent::isAutumnMoon, EVENT_REGIONS_EU, EVENT_KIND_HOLIDAY, 6, 0, 0, 0}, // EVENT_EU_AUTUMN_MOON
    {dEvent::isLunarNewYear, EVENT_REGIONS_KR, EVENT_KIND_HOLIDAY, 6, 0, 0, 0}, // EVENT_KR_LUNAR_NEW_YEAR
    {dEvent::isFixedDate, EVENT_REGIONS_KR, EVENT_KIND_HOLIDAY, 6, 0, 4, 5}, // EVENT_KR_ARBOR_DAY
    {dEvent::isFixedDate, EVENT_REGIONS_KR, EVENT_KIND_HOLIDAY, 6, 0, 5, 15}, // EVENT_KR_TEACHERS_DAY
    {dEvent::isDaeboreum, EVENT_REGIONS_KR, EVENT_KIND_HOLIDAY, 6, 0, 0, 0}, // EVENT_KR_DAEBOREUM
    {dEvent::isFixedDate, EVENT_REGIONS_ALL, EVENT_KIND_HOLIDAY, 6, 0, 4, 1}, // EVENT_APRIL_FOOLS_DAY
    {dEvent::isMothersDay, EVENT_REGIONS_ALL, EVENT_KIND_TOWN, 6, 0, 0, 0}, // EVENT_MOTHERS_DAY
    {dEvent::isFathersDay, EVENT_REGIONS_ALL, EVENT_KIND_TOWN, 6, 0, 0, 0}, // EVENT_FATHERS_DAY
};

static dEventActive_c sActive[EVENT_ACTIVE_NUM]; // 80585368
static dTime_c sDay;                             // 805853B8: the current game day (6:00 based)
static int sDailyId;                           // 8074E2E0

// The fields are s32 (long); the original's were int, so the reference binds directly.
static inline const int &getYear(const dTime_c *t) {
    return reinterpret_cast<const int &>(t->year);
}

namespace dEvent {

// 80088BCC
BOOL isPlayerBirthday(const dTime_c *t, const u8 *player, const u8 *b) {
    dPrivateData_c *p = dPlayerMgr_c::getPlayer(*player);
    if (p == NULL || !p->mPID.isValid()) {
        return FALSE;
    }
    return p->isBirthday(*t);
}

// 80088C34
BOOL isAnimalBirthday(const dTime_c *t, const u8 *animal, const u8 *b) {
    dAnimal_c *a = dSaveData_c::getRaw()->mAnimals.mTown.getAnimalConst(*animal);
    if (a == NULL) {
        return FALSE;
    }
    if (t->month == (u8)a->getBirthMonth() && t->mday == (u8)a->getBirthDay()) {
        return TRUE;
    }
    return FALSE;
}

// 80088CC8
int getFishingTourneyDay(const dTime_c *t) {
    int nth = sFishingTourneySaturday[t->month];
    if (nth == -1) {
        return 0;
    }
    return dTime_c::getNthWeekday(t->year, t->month, 6, nth);
}

// 80088CFC
BOOL isFishingTourney(const dTime_c *t, const u8 *a, const u8 *b) {
    int day = getFishingTourneyDay(t);
    return day == t->mday;
}

// 80088D38
int getBugOffDay(const dTime_c *t) {
    if (t->month >= MONTH_JUNE && t->month <= MONTH_SEPTEMBER) {
        return dTime_c::getNthWeekday(t->year, t->month, 6, 3);
    }
    return 0;
}

// 80088D64
BOOL isBugOff(const dTime_c *t, const u8 *a, const u8 *b) {
    int day = getBugOffDay(t);
    return day == t->mday;
}

// 80088DA0
BOOL isFireworks(const dTime_c *t, const u8 *a, const u8 *b) {
    if (t->month != MONTH_AUGUST) {
        return FALSE;
    }
    return t->wday == 0;
}

// 80088DC4
int getFleaMarketDay(const dTime_c *t) {
    if (t->month == MONTH_AUGUST || t->month == MONTH_DECEMBER) {
        return 0;
    }
    return dTime_c::getNthWeekday(t->year, t->month, 0, 4);
}

// 80088DF4
BOOL isFleaMarket(const dTime_c *t, const u8 *a, const u8 *b) {
    if (t->month == MONTH_AUGUST || t->month == MONTH_DECEMBER) {
        return FALSE;
    }
    int day = getFleaMarketDay(t);
    return day == t->mday;
}

// 80088E4C
BOOL isFixedDate(const dTime_c *t, const u8 *month, const u8 *day) {
    BOOL ret = FALSE;
    if (t->month == *month - 1 && t->mday == *day) {
        ret = TRUE;
    }
    return ret;
}

// 80088E80
int getHarvestFestivalDay(const dTime_c *t) {
    if (t->month != MONTH_NOVEMBER) {
        return 0;
    }
    return dTime_c::getNthWeekday(t->year, t->month, 4, 4);
}

// 80088EA8
BOOL isHarvestFestival(const dTime_c *t, const u8 *a, const u8 *b) {
    int day = getHarvestFestivalDay(t);
    return day == t->mday;
}

// 80088EE4
BOOL getFestivaleDate(dTime_c *out, const int &year) {
    if (year < TIME_YEAR_MIN || year > TIME_YEAR_MAX - 1) {
        return FALSE;
    }
    int idx = year - TIME_YEAR_MIN;
    out->set(year, sFestivaleDates[idx * 2], sFestivaleDates[idx * 2 + 1], 0, 0, 0);
    out->normalize();
    return TRUE;
}

// 80088F68
BOOL isFestivale(const dTime_c *t, const u8 *a, const u8 *b) {
    dTime_c date;
    BOOL ret;
    if (!getFestivaleDate(&date, getYear(t))) {
        ret = FALSE;
    } else {
        ret = FALSE;
        if (t->month == date.month && t->mday == date.mday) {
            ret = TRUE;
        }
    }
    return ret;
}

// 80088FD4
BOOL getBunnyDayDate(dTime_c *out, const int &year) {
    if (year < TIME_YEAR_MIN || year > TIME_YEAR_MAX - 1) {
        return FALSE;
    }
    int idx = year - TIME_YEAR_MIN;
    out->set(year, sBunnyDayDates[idx * 2], sBunnyDayDates[idx * 2 + 1], 0, 0, 0);
    out->normalize();
    return TRUE;
}

// 80089058
BOOL isBunnyDay(const dTime_c *t, const u8 *a, const u8 *b) {
    dTime_c date;
    BOOL ret;
    if (!getBunnyDayDate(&date, getYear(t))) {
        ret = FALSE;
    } else {
        ret = FALSE;
        if (t->month == date.month && t->mday == date.mday) {
            ret = TRUE;
        }
    }
    return ret;
}

// 800890C4
BOOL getJpAutumnMoonDate(dTime_c *out, const int &year) {
    if (year < TIME_YEAR_MIN || year > TIME_YEAR_MAX - 1) {
        return FALSE;
    }
    int idx = year - TIME_YEAR_MIN;
    out->set(year, sJpAutumnMoonDates[idx * 2], sJpAutumnMoonDates[idx * 2 + 1], 0, 0, 0);
    out->normalize();
    return TRUE;
}

// 80089148
BOOL isJpAutumnMoon(const dTime_c *t, const u8 *a, const u8 *b) {
    dTime_c date;
    BOOL ret;
    if (!getJpAutumnMoonDate(&date, getYear(t))) {
        ret = FALSE;
    } else {
        ret = FALSE;
        if (t->month == date.month && t->mday == date.mday) {
            ret = TRUE;
        }
    }
    return ret;
}


// 800891B4
int getLaborDayDay(const dTime_c *t) {
    if (t->month != MONTH_SEPTEMBER) {
        return 0;
    }
    return dTime_c::getNthWeekday(t->year, t->month, 1, 1);
}

// 800891DC
BOOL isLaborDay(const dTime_c *t, const u8 *a, const u8 *b) {
    int day = getLaborDayDay(t);
    return day == t->mday;
}

// 80089218
int getExplorersDayDay(const dTime_c *t) {
    if (t->month != MONTH_OCTOBER) {
        return 0;
    }
    return dTime_c::getNthWeekday(t->year, t->month, 1, 2);
}

// 80089240
BOOL isExplorersDay(const dTime_c *t, const u8 *a, const u8 *b) {
    int day = getExplorersDayDay(t);
    return day == t->mday;
}

// 8008927C
BOOL getAutumnMoonDate(dTime_c *out, const int &year) {
    if (year < TIME_YEAR_MIN || year > TIME_YEAR_MAX - 1) {
        return FALSE;
    }
    int idx = year - TIME_YEAR_MIN;
    out->set(year, sHarvestMoonDates[idx * 2], sHarvestMoonDates[idx * 2 + 1], 0, 0, 0);
    out->normalize();
    return TRUE;
}

// 80089300
BOOL isAutumnMoon(const dTime_c *t, const u8 *a, const u8 *b) {
    dTime_c date;
    BOOL ret;
    if (!getAutumnMoonDate(&date, getYear(t))) {
        ret = FALSE;
    } else {
        ret = FALSE;
        if (t->month == date.month && t->mday == date.mday) {
            ret = TRUE;
        }
    }
    return ret;
}


// 8008936C
BOOL getLunarNewYearDate(dTime_c *out, const int &year, int days) {
    if (year < TIME_YEAR_MIN || year > TIME_YEAR_MAX - 1) {
        return FALSE;
    }
    out->set(year, sLunarNewYearDates[(year - TIME_YEAR_MIN) * 2],
             sLunarNewYearDates[(year - TIME_YEAR_MIN) * 2 + 1] + days, 0, 0, 0);
    out->normalize();
    return TRUE;
}

// 800893F8
BOOL isLunarNewYear(const dTime_c *t, const u8 *a, const u8 *b) {
    dTime_c date;
    BOOL ret;
    if (!getLunarNewYearDate(&date, getYear(t), 0)) {
        ret = FALSE;
    } else {
        ret = FALSE;
        if (t->month == date.month && t->mday == date.mday) {
            ret = TRUE;
        }
    }
    return ret;
}

// 80089468
BOOL isDaeboreum(const dTime_c *t, const u8 *a, const u8 *b) {
    dTime_c date;
    BOOL ret;
    if (!getLunarNewYearDate(&date, getYear(t), 14)) {
        ret = FALSE;
    } else {
        ret = FALSE;
        if (t->month == date.month && t->mday == date.mday) {
            ret = TRUE;
        }
    }
    return ret;
}

// 800894D8
BOOL getMotheringSundayDate(dTime_c *out, const int &year) {
    if (year < TIME_YEAR_MIN || year > TIME_YEAR_MAX - 1) {
        return FALSE;
    }
    int idx = year - TIME_YEAR_MIN;
    out->set(year, sMotheringSundayDates[idx * 2], sMotheringSundayDates[idx * 2 + 1], 0, 0, 0);
    out->normalize();
    return TRUE;
}

// 8008955C
BOOL getFrMothersDayDate(dTime_c *out, const int &year) {
    if (year < TIME_YEAR_MIN || year > TIME_YEAR_MAX - 1) {
        return FALSE;
    }
    int idx = year - TIME_YEAR_MIN;
    out->set(year, sFrMothersDayDates[idx * 2], sFrMothersDayDates[idx * 2 + 1], 0, 0, 0);
    out->normalize();
    return TRUE;
}


// 800895E0
BOOL isMothersDay(const dTime_c *t, const u8 *a, const u8 *b) {
    dTime_c date;
    switch (dSaveData_c::getSaveRegion()) {
    case LANGUAGE_ES:
        date.month = MONTH_MAY;
        date.mday = dTime_c::getNthWeekday(t->year, MONTH_MAY, 0, 1);
        break;
    case LANGUAGE_EN:
        if (SCGetLanguage() == 6) {
            date.month = MONTH_MAY;
            date.mday = dTime_c::getNthWeekday(t->year, MONTH_MAY, 0, 2);
        } else if (!getMotheringSundayDate(&date, getYear(t))) {
            return FALSE;
        }
        break;
    case LANGUAGE_FR:
        if (!getFrMothersDayDate(&date, getYear(t))) {
            return FALSE;
        }
        break;
    case LANGUAGE_KR:
        date.month = MONTH_MAY;
        date.mday = 8;
        break;
    default:
        date.month = MONTH_MAY;
        date.mday = dTime_c::getNthWeekday(t->year, MONTH_MAY, 0, 2);
        break;
    }
    BOOL ret = FALSE;
    if (t->month == date.month && t->mday == date.mday) {
        ret = TRUE;
    }
    return ret;
}

// 8008972C
BOOL getAscensionDayDate(dTime_c *out, const int &year) {
    if (year < TIME_YEAR_MIN || year > TIME_YEAR_MAX - 1) {
        return FALSE;
    }
    int idx = year - TIME_YEAR_MIN;
    out->set(year, sAscensionDayDates[idx * 2], sAscensionDayDates[idx * 2 + 1], 0, 0, 0);
    out->normalize();
    return TRUE;
}


// 800897B0
BOOL isFathersDay(const dTime_c *t, const u8 *a, const u8 *b) {
    dTime_c date;
    switch (dSaveData_c::getSaveRegion()) {
    case LANGUAGE_ES:
    case LANGUAGE_IT:
        date.month = MONTH_MARCH;
        date.mday = 19;
        break;
    case LANGUAGE_DE:
        if (!getAscensionDayDate(&date, getYear(t))) {
            return FALSE;
        }
        break;
    case LANGUAGE_KR:
        return FALSE;
    default:
        date.month = MONTH_JUNE;
        date.mday = dTime_c::getNthWeekday(t->year, MONTH_JUNE, 0, 3);
        break;
    }
    BOOL ret = FALSE;
    if (t->month == date.month && t->mday == date.mday) {
        ret = TRUE;
    }
    return ret;
}

// 80089890
BOOL isEventOn(const dQuestEvent_e &event, const dTime_c *t) {
    if (!isEventInRegion(event)) {
        return FALSE;
    }
    const dEventInfo_c *info = getInfo(event);
    if (info->mCheck == NULL) {
        return FALSE;
    }
    return info->mCheck(t, &info->mMonth, &info->mDay);
}

// 8008990C
BOOL isEventOnDate(const dQuestEvent_e &event, const int &year, const int &month, const int &day) {
    dTime_c t;
    t.set(year, month, day, 0, 0, 0);
    t.normalize();
    return isEventOn(event, &t);
}

// 80089968
BOOL isEventWithin(const dQuestEvent_e &event, const dTime_c &time, const int &a, const int &b) {
    dTime_c t = time;
    t.add(a, 0, 0, 0);
    for (int i = a; i <= b; i++) {
        if (isEventOn(event, &t)) {
            return TRUE;
        }
        t.add(1, 0, 0, 0);
    }
    return FALSE;
}

// 80089A58
int getFireworksState(const dTime_c &time) {
    dTime_c t = time;
    t.add(1, 0, 0, 0);
    if (isEventOn(EVENT_FIREWORKS, &t)) {
        int week = (t.mday - 1) / 7;
        if (week == 0) {
            return 2;
        }
        if (week == 4) {
            return 4;
        }
        if (week == 3 && dTime_c::getNthWeekday(t.year, MONTH_AUGUST, 0, 5) == -1) {
            return 4;
        }
        return 3;
    }
    if (isEventWithin(EVENT_FIREWORKS, time, 2, 6)) {
        if (time.month != MONTH_AUGUST) {
            return 1;
        }
        if (dTime_c::getNthWeekday(time.year, MONTH_AUGUST, 0, 1) > time.mday) {
            return 1;
        }
    }
    return 0;
}

// FAKE (decomp-permuter): the volatile return value is what puts the region byte in r5 and the
// 1 in r4; every non-volatile form swaps them. The real source is unknown.
static inline volatile int fakeVolatile(int x) { return x; }

// 80089BD4
BOOL isEventInRegion(const dQuestEvent_e &event) {
    dSaveData_c * const save = dSaveData_c::getRaw();
    dEventInfo_c * const info = const_cast<dEventInfo_c*>(getInfo(event));
    const u16 mask = info->mRegions;
    const u8 region = fakeVolatile((save->_0735C2 >> 4) & 0xF);
    u16 regionMask = 1 << (region & 0xF);
    regionMask = regionMask & mask;

    return regionMask != 0;
}

// 80089C40
int getTownEventOn(const dTime_c *time) {
    for (int i = 0; i < EVENT_NUM; i++) {
        if (getInfo(reinterpret_cast<const dQuestEvent_e &>(i))->mKind == EVENT_KIND_TOWN &&
            isEventOn(reinterpret_cast<const dQuestEvent_e &>(i), time)) {
            return i;
        }
    }
    return -1;
}

// 80089CBC
void getTodayTime(dTime_c *out, int hour, int min) {
    out->set(sDay.year, sDay.month, sDay.mday, hour, min, 0);
    out->normalize();
    if (hour < 6) {
        out->add(1, 0, 0, 0);
    }
}

// 80089D3C
void buildSchedule(const dTime_c &now, BOOL reset) {
    dTime_c t = now;
    dEventActive_c *active;
    t.add(0, -6, 0, 0);
    int i;
    for (i = 0; i < EVENT_ACTIVE_NUM; i++) {
        sActive[i].mActive = FALSE;
    }
    int num = 0;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();

    active = sActive;
    for (i = 0; i < EVENT_NUM && num < EVENT_ACTIVE_NUM; i++) {
        if (i == EVENT_BUNNY_DAY && player != NULL && player->isFlag0(0x28)) {
            continue;
        }
        if (isEventOn(reinterpret_cast<const dQuestEvent_e &>(i), &t)) {
            active->mActive = TRUE;
            active->mEvent = (dQuestEvent_e)i;
            active++;
            num++;
        }
    }
    sDay.set(t.year, t.month, t.mday, 0, 0, 0);
    sDay.normalize();
    if (reset) {
        sDailyId = 0xFFFF;
    }
}

// 80089ED0
void updateSchedule(const dTime_c &now, BOOL reset) {
    buildSchedule(now, reset);
    if (fn_800DCF90()) {
        dSaveData_c::getTown()->mVisitorNpc.update();
    }
}

// 80089F0C
BOOL isActive(const dQuestEvent_e &event) {
    for (int i = 0; i < EVENT_ACTIVE_NUM; i++) {
        if (sActive[i].mActive && event == sActive[i].mEvent) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80089FE8
BOOL isOngoing(const dQuestEvent_e &event) {
    BOOL ret;
    if (!isActive(event)) {
        ret = FALSE;
    } else {
        ret = FALSE;
        if (!isNotStarted(event) && !isOver(event)) {
            ret = TRUE;
        }
    }
    return ret;
}

// 8008A058
BOOL isNotStarted(const dQuestEvent_e &event) {
    if (!isActive(event)) {
        return FALSE;
    }
    const dEventInfo_c *info = getInfo(event);
    if (info->mStartHour == 0 && info->mEndHour == 24) {
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    if (!dTime_c::isSameOrAfter(now, sDay)) {
        now.year += TIME_YEAR_MAX - TIME_YEAR_MIN;
        now.normalize();
    }
    dTime_c start;
    getStartTime(event, &start);
    return !dTime_c::isSameOrAfter(now, start);
}

// 8008A27C
BOOL isOver(const dQuestEvent_e &event) {
    if (!isActive(event)) {
        return FALSE;
    }
    const dEventInfo_c *info = getInfo(event);
    if (info->mStartHour == 0 && info->mEndHour == 24) {
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    if (!dTime_c::isSameOrAfter(now, sDay)) {
        now.year += TIME_YEAR_MAX - TIME_YEAR_MIN;
        now.normalize();
    }
    dTime_c end;
    getEndTime(event, &end);
    return !dTime_c::isSameOrAfter(end, now);
}

// 8008A4A0
BOOL isOverWithin(const dQuestEvent_e &event, const int &hours, const int &mins) {
    if (!isActive(event)) {
        return FALSE;
    }
    dTime_c end;
    getEndTime(event, &end);
    dTime_c now = *dTime_c::getCurrent();
    if (dTime_c::isSameOrAfter(end, now)) {
        return FALSE;
    }
    end.add(0, hours, mins, 0);
    return dTime_c::isSameOrAfter(end, now);
}

// 8008A6A0
int getActiveOfKind(int kind) {
    for (int i = 0; i < EVENT_ACTIVE_NUM; i++) {
        if (sActive[i].mActive) {
            int k = getInfo(sActive[i].mEvent)->mKind;
            if (k == kind) {
                return sActive[i].mEvent;
            }
        }
    }
    return EVENT_NUM;
}

// 8008A72C
BOOL getStartTime(const dQuestEvent_e &event, dTime_c *out) {
    if (!isActive(event)) {
        return FALSE;
    }
    const dEventInfo_c *info = getInfo(event);
    getTodayTime(out, info->mStartHour, 0);
    return TRUE;
}

// 8008A794
BOOL getEndTime(const dQuestEvent_e &event, dTime_c *out) {
    if (!isActive(event)) {
        return FALSE;
    }
    const dEventInfo_c *info = getInfo(event);
    getTodayTime(out, info->mEndHour, 0);
    return TRUE;
}

// 8008A7FC
dTime_c *getToday() {
    return &sDay;
}

// 8008A808
BOOL isNowBetween(const int &startHour, const int &startMin, const int &endHour, const int &endMin) {
    dTime_c now = *dTime_c::getCurrent();
    dTime_c start;
    getTodayTime(&start, startHour, startMin);
    if (!dTime_c::isSameOrAfter(now, start)) {
        return FALSE;
    }
    dTime_c end;
    getTodayTime(&end, endHour, endMin);
    if (endHour == 6) {
        end.add(1, 0, 0, 0);
    }
    return dTime_c::isSameOrAfter(end, now);
}

// 8008AA44
BOOL isNowBefore(const int &hour, const int &min) {
    dTime_c t;
    getTodayTime(&t, hour, min);
    return dTime_c::isSameOrAfter(t, *dTime_c::getCurrent());
}

// 8008AB3C
BOOL isRandomVisitor(int x) {
    BOOL ret = FALSE;
    if (x >= 7 && x <= 9) {
        ret = TRUE;
    }
    return ret;
}

// 8008AB5C
int pickRandomVisitor(int a, int b) {
    int num = 3;
    if (isRandomVisitor(a)) {
        num = 2;
    }
    if (isRandomVisitor(b)) {
        num--;
    }
    int r = cM::rndInt(num);
    for (int i = 7; i <= 9; i++) {
        if (i != a && i != b) {
            if (r <= 0) {
                return i;
            }
            r--;
        }
    }
    return -1;
}

// 8008AC40
u8 getTodayVisitor() {
    dSaveTown_c *town = dSaveData_c::getTown();
    return town->mVisitorNpc.getDay(getToday()->wday);
}

// 8008AC84
BOOL isVisitorHere(int kind) {
    if (kind != getTodayVisitor()) {
        return FALSE;
    }
    int startHour, startMin, endHour, endMin;
    switch (kind) {
    case 2:
        startHour = 6;
        startMin = 0;
        endHour = 12;
        endMin = 0;
        break;
    case 3:
        startHour = 19;
        startMin = 30;
        endHour = 0;
        endMin = 0;
        break;
    case 5:
        startHour = 20;
        startMin = 0;
        endHour = 6;
        endMin = 0;
        break;
    case 7:
    case 9:
        startHour = 6;
        startMin = 0;
        endHour = 0;
        endMin = 0;
        break;
    default:
        return TRUE;
    }
    return isNowBetween(startHour, startMin, endHour, endMin);
}

// 8008AD7C
BOOL isSavedDateToday() {
    dTimeStamp_c *time = &dSaveData_c::getRaw()->mVisitorNpc.mEventDay;
    if (time->isNone()) {
        return FALSE;
    }
    dTime_c date = time->get();
    return dTime_c::isSameDay(sDay, date);
}

// 8008AEB0
int getDailyId() {
    return sDailyId;
}

// 8008AEB8
void setDailyId(int value) {
    sDailyId = value;
}

// 8008AEC0
void resetDailyId() {
    sDailyId = 0xFFFF;
}

// 8008AED0
const dEventInfo_c *getInfo(const dQuestEvent_e &event) {
    return &sEventInfo[event];
}

// 8008AEE8
int fn_8008AEE8() {
    return 0;
}

} // namespace dEvent
