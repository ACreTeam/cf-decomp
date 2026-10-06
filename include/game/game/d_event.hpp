#pragma once

// Town event schedule and state: the City Folk counterpart of the GameCube Animal Crossing
// m_event.c (mEv_*). Source: src/dol/game/d_event.cpp (.text 80088BCC..8008AEF0).
// File name and function meanings are inferred. See notes/d_event.txt.

#include <types.h>
#include <game/game/d_date.hpp>

// Event id: index into the event info table (sEventInfo, EVENT_NUM entries).
// Taken by const reference everywhere, so literal ids become anonymous .sdata temporaries.
// Names follow the NA release; region-only events carry the region (see EVENT_REGIONS_*).
enum dQuestEvent_e {
    EVENT_ANIMAL_BIRTHDAY_0, // villager 0..9 (dEventInfo_c::mMonth is the villager index)
    EVENT_ANIMAL_BIRTHDAY_1,
    EVENT_ANIMAL_BIRTHDAY_2,
    EVENT_ANIMAL_BIRTHDAY_3,
    EVENT_ANIMAL_BIRTHDAY_4,
    EVENT_ANIMAL_BIRTHDAY_5,
    EVENT_ANIMAL_BIRTHDAY_6,
    EVENT_ANIMAL_BIRTHDAY_7,
    EVENT_ANIMAL_BIRTHDAY_8,
    EVENT_ANIMAL_BIRTHDAY_9,
    EVENT_PLAYER_BIRTHDAY_0, // player 0..3
    EVENT_PLAYER_BIRTHDAY_1,
    EVENT_PLAYER_BIRTHDAY_2,
    EVENT_PLAYER_BIRTHDAY_3,
    EVENT_FISHING_TOURNEY,   // 14: one Saturday a month, Oct..May, 9:00-18:00
    EVENT_BUG_OFF,           // 15: 3rd Saturday, Jun..Sep, 9:00-18:00
    EVENT_FIREWORKS,         // 16: Sundays in August, from 19:00
    EVENT_FLEA_MARKET,       // 17: 4th Sunday, not Aug/Dec, all day
    EVENT_VALENTINES_DAY,    // 18: Feb 14
    EVENT_COUNTDOWN,         // 19: Dec 31, 23:00-2:00
    EVENT_NEW_YEARS_DAY,     // 20: Jan 1
    EVENT_HALLOWEEN,         // 21: Oct 31, from 18:00
    EVENT_HARVEST_FESTIVAL,  // 22: 4th Thursday of November, 15:00-21:00
    EVENT_TOY_DAY,           // 23: Dec 24, from 20:00
    EVENT_FESTIVALE,         // 24: carnival (table), all day
    EVENT_BUNNY_DAY,         // 25: Easter (table), all day
    EVENT_JP_SETSUBUN,       // 26: Feb 3
    EVENT_JP_GIRLS_DAY,      // 27: Mar 3
    EVENT_JP_CHILDRENS_DAY,  // 28: May 5
    EVENT_JP_AUTUMN_MOON,    // 29: Tsukimi (table)
    EVENT_JP_TANABATA,       // 30: Jul 7
    EVENT_NA_GROUNDHOG_DAY,  // 31: Feb 2
    EVENT_NA_NATURE_DAY,     // 32: Apr 22
    EVENT_NA_LABOR_DAY,      // 33: 1st Monday of September
    EVENT_NA_EXPLORERS_DAY,  // 34: 2nd Monday of October
    EVENT_NA_AUTUMN_MOON,    // 35: harvest moon (table)
    EVENT_EU_MIDSUMMERS_DAY, // 36: Jun 21
    EVENT_EU_NAUGHTY_OR_NICE_DAY, // 37: Dec 6
    EVENT_EU_MIDWINTERS_DAY, // 38: Dec 21
    EVENT_EU_AUTUMN_MOON,    // 39: harvest moon (table)
    EVENT_KR_LUNAR_NEW_YEAR, // 40: Seollal (table)
    EVENT_KR_ARBOR_DAY,      // 41: Apr 5 (name from the date)
    EVENT_KR_TEACHERS_DAY,   // 42: May 15 (name from the date)
    EVENT_KR_DAEBOREUM,      // 43: lunar new year + 14 days
    EVENT_APRIL_FOOLS_DAY,   // 44: Apr 1
    EVENT_MOTHERS_DAY,       // 45: by region (isMothersDay)
    EVENT_FATHERS_DAY,       // 46: by region (isFathersDay)

    EVENT_NUM
};

// Ids past EVENT_NUM in the town's event list (procEvents adds them on Wisp's days; endEvents
// removes the lamps for both).
enum {
    EVENT_LAMP_BURIED = EVENT_NUM, // buryLamp() succeeded today
    EVENT_LAMP_SAME_DAY,           // the saved lamp date is today
};

// dEvent::getTodayVisitor() (only the one the field manager uses is named).
enum dVisitor_e {
    VISITOR_WISP = 5, // isVisitorHere: 20:00..6:00; procEvents buries a lamp on his day
};

#define EVENT_ACTIVE_NUM 10

// dEventInfo_c::mKind
enum {
    EVENT_KIND_TOWN,     // events held in town (and the Mother's / Father's Day letters)
    EVENT_KIND_HOLIDAY,  // Tortimer's gift days
    EVENT_KIND_BIRTHDAY,
};

// dEventInfo_c::mRegions
#define EVENT_REGIONS_JP  ((1 << LANGUAGE_JP))
#define EVENT_REGIONS_NA  ((1 << LANGUAGE_US) | (1 << LANGUAGE_MX) | (1 << LANGUAGE_QC))
#define EVENT_REGIONS_EU  ((1 << LANGUAGE_EN) | (1 << LANGUAGE_ES) | (1 << LANGUAGE_FR) | (1 << LANGUAGE_IT) | (1 << LANGUAGE_DE))
#define EVENT_REGIONS_KR  ((1 << LANGUAGE_KR))
#define EVENT_REGIONS_ALL (EVENT_REGIONS_JP | EVENT_REGIONS_NA | EVENT_REGIONS_EU | EVENT_REGIONS_KR)

// "Is it this event on date t". a/b are dEventInfo_c::mMonth/mDay (for birthdays, mMonth is
// the villager or player index).
typedef BOOL (*dEventCheckFunc)(const dTime_c *t, const u8 *a, const u8 *b);

// One entry of the event info table (8046EC70).
struct dEventInfo_c {
    /* 0x0 */ dEventCheckFunc mCheck;
    /* 0x4 */ u16 mRegions;   // EVENT_REGIONS_*: bit per save region
    /* 0x6 */ u8 mKind;       // EVENT_KIND_*
    /* 0x7 */ u8 mStartHour;  // the day starts at 6:00; hours < 6 belong to the next day
    /* 0x8 */ u8 mEndHour;    // 24 with mStartHour 0: all day
    /* 0x9 */ u8 mMonth;      // 1-based (isFixedDate), or the birthday index
    /* 0xA */ u8 mDay;
}; // size 0xC

// Events active today (80585368), filled by buildSchedule.
struct dEventActive_c {
    /* 0x0 */ dQuestEvent_e mEvent;
    /* 0x4 */ BOOL mActive;
}; // size 0x8

// Per-year date tables: MONTH_*, day pairs for 2000..2035. A flat int array: the code indexes
// [i * 2] and [i * 2 + 1], which a struct or [36][2] array does not reproduce.
typedef int dEventYearDates_t[(TIME_YEAR_MAX - TIME_YEAR_MIN) * 2];

namespace dEvent {

// Date rules: "is it that event on t", and the helpers computing its date.
BOOL isPlayerBirthday(const dTime_c *t, const u8 *player, const u8 *b); // 80088BCC
BOOL isAnimalBirthday(const dTime_c *t, const u8 *animal, const u8 *b); // 80088C34
int getFishingTourneyDay(const dTime_c *t);                    // 80088CC8: 0 if none this month
BOOL isFishingTourney(const dTime_c *t, const u8 *a, const u8 *b);      // 80088CFC
int getBugOffDay(const dTime_c *t);                            // 80088D38
BOOL isBugOff(const dTime_c *t, const u8 *a, const u8 *b);              // 80088D64
BOOL isFireworks(const dTime_c *t, const u8 *a, const u8 *b);           // 80088DA0
int getFleaMarketDay(const dTime_c *t);                        // 80088DC4
BOOL isFleaMarket(const dTime_c *t, const u8 *a, const u8 *b);          // 80088DF4
BOOL isFixedDate(const dTime_c *t, const u8 *month, const u8 *day);     // 80088E4C
int getHarvestFestivalDay(const dTime_c *t);                   // 80088E80
BOOL isHarvestFestival(const dTime_c *t, const u8 *a, const u8 *b);     // 80088EA8
BOOL getFestivaleDate(dTime_c *out, const int &year);          // 80088EE4
BOOL isFestivale(const dTime_c *t, const u8 *a, const u8 *b);           // 80088F68
BOOL getBunnyDayDate(dTime_c *out, const int &year);           // 80088FD4
BOOL isBunnyDay(const dTime_c *t, const u8 *a, const u8 *b);            // 80089058
BOOL getJpAutumnMoonDate(dTime_c *out, const int &year);       // 800890C4
BOOL isJpAutumnMoon(const dTime_c *t, const u8 *a, const u8 *b);        // 80089148
int getLaborDayDay(const dTime_c *t);                          // 800891B4
BOOL isLaborDay(const dTime_c *t, const u8 *a, const u8 *b);            // 800891DC
int getExplorersDayDay(const dTime_c *t);                      // 80089218
BOOL isExplorersDay(const dTime_c *t, const u8 *a, const u8 *b);        // 80089240
BOOL getAutumnMoonDate(dTime_c *out, const int &year);         // 8008927C
BOOL isAutumnMoon(const dTime_c *t, const u8 *a, const u8 *b);          // 80089300
BOOL getLunarNewYearDate(dTime_c *out, const int &year, int days); // 8008936C: + days
BOOL isLunarNewYear(const dTime_c *t, const u8 *a, const u8 *b);        // 800893F8
BOOL isDaeboreum(const dTime_c *t, const u8 *a, const u8 *b);           // 80089468
BOOL getMotheringSundayDate(dTime_c *out, const int &year);    // 800894D8: UK Mother's Day
BOOL getFrMothersDayDate(dTime_c *out, const int &year);       // 8008955C
BOOL isMothersDay(const dTime_c *t, const u8 *a, const u8 *b);          // 800895E0
BOOL getAscensionDayDate(dTime_c *out, const int &year);       // 8008972C: DE Father's Day
BOOL isFathersDay(const dTime_c *t, const u8 *a, const u8 *b);          // 800897B0

// Event checks.
BOOL isEventOn(const dQuestEvent_e &event, const dTime_c *t);  // 80089890
BOOL isEventOnDate(const dQuestEvent_e &event, const int &year, const int &month, const int &day); // 8008990C
BOOL isEventWithin(const dQuestEvent_e &event, const dTime_c &time, const int &from, const int &to); // 80089968: on any of days from..to after time
int getFireworksState(const dTime_c &time);                    // 80089A58
BOOL isEventInRegion(const dQuestEvent_e &event);              // 80089BD4
int getTownEventOn(const dTime_c *time);                       // 80089C40: EVENT_KIND_TOWN event, or -1
void getTodayTime(dTime_c *out, int hour, int min);            // 80089CBC: hour:min of the current day
void buildSchedule(const dTime_c &now, BOOL reset);            // 80089D3C: today's active events
void updateSchedule(const dTime_c &now, BOOL reset);           // 80089ED0
BOOL isActive(const dQuestEvent_e &event);                     // 80089F0C: active today
BOOL isOngoing(const dQuestEvent_e &event);                    // 80089FE8: active and within its hours
BOOL isNotStarted(const dQuestEvent_e &event);                 // 8008A058
BOOL isOver(const dQuestEvent_e &event);                       // 8008A27C
BOOL isOverWithin(const dQuestEvent_e &event, const int &hours, const int &mins); // 8008A4A0: over, at most hours:mins ago
int getActiveOfKind(int kind);                                 // 8008A6A0: or EVENT_NUM
BOOL getStartTime(const dQuestEvent_e &event, dTime_c *out);   // 8008A72C
BOOL getEndTime(const dQuestEvent_e &event, dTime_c *out);     // 8008A794
dTime_c *getToday();                                           // 8008A7FC
BOOL isNowBetween(const int &startHour, const int &startMin, const int &endHour, const int &endMin); // 8008A808
BOOL isNowBefore(const int &hour, const int &min);             // 8008AA44

// Today's visitor from the weekly schedule (dSaveData_c::mVisitorNpc, VISITOR_NPC_*). The kinds are inferred from
// their hours (2: 6:00-12:00, Joan?); 7..9 are picked at random.
BOOL isRandomVisitor(int kind);                                // 8008AB3C: 7..9
int pickRandomVisitor(int a, int b);                           // 8008AB5C: one of 7..9 other than a and b
u8 getTodayVisitor();                                          // 8008AC40
BOOL isVisitorHere(int kind);                                  // 8008AC84: today's and within its hours
BOOL isSavedDateToday();                                       // 8008AD7C

// Reset to 0xFFFF when the schedule is rebuilt with reset.
int getDailyId();                                              // 8008AEB0
void setDailyId(int value);                                    // 8008AEB8
void resetDailyId();                                           // 8008AEC0

const dEventInfo_c *getInfo(const dQuestEvent_e &event);       // 8008AED0
int fn_8008AEE8();                                             // 8008AEE8: returns 0

} // namespace dEvent

// Other TUs used here, not split yet (C linkage keeps the target names).
struct dTimeStamp_c;
extern "C" {
int fn_800DCF90();                               // 800DCF90
}
