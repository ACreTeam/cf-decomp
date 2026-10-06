#pragma once

#include <types.h>
#include <game/game/d_time_stamp.hpp>

// What comes to town on a day of the week (dSaveVisitorNpc_c::mDays, dEvent::getTodayVisitor).
// Kinds are inferred from the visiting hours in dEvent::isVisitorHere and from who sets them.
enum {
    VISITOR_NPC_NOT_SET,      // 0: not decided yet
    VISITOR_NPC_TOWN_EVENT,   // 1: a weekday with a town event, no visitor
    VISITOR_NPC_SUNDAY,       // 2: every Sunday, 6:00-12:00 (Joan?)
    VISITOR_NPC_SATURDAY,     // 3: every Saturday, 19:30-0:00
    VISITOR_NPC_NONE,         // 4: nobody
    VISITOR_NPC_WISP,         // 5: 20:00-6:00, chance grows with the weeks skipped (VISITOR_WISP)
    VISITOR_NPC_6,            // 6: 30% a week, not twice in a row (town flags 0x12/0x13)
    VISITOR_NPC_RANDOM_FIRST, // 7..9: two a week (dEvent::pickRandomVisitor)
    VISITOR_NPC_RANDOM_LAST = VISITOR_NPC_RANDOM_FIRST + 2,
};

#define VISITOR_NPC_DAY_NUM 7

// The town's weekly visitor schedule (NH ::Game::SaveVisitorNpc), at dSaveData_c::_0683C8.
// Rebuilt by update() (from dEvent::updateSchedule) when a new week starts.
// Source: src/dol/game/d_save_visitor_npc.cpp (.text 8010FB6C..8011041C).
class dSaveVisitorNpc_c { // 0x17
public:
    dSaveVisitorNpc_c(); // 8010FB6C
    void clear(); // 8010FBC0
    void buildTodayEvents(); // 8010FC14: dEvent::buildSchedule for now
    void update(); // 8010FC3C
    u8 getDay(int wday); // 801100B8: VISITOR_NPC_*
    int getFreeDay(int exclude); // 801100C4: a random unscheduled weekday (1..5) other than exclude, or -1
    BOOL isNewWeek(int *weeks); // 8011026C: weeks since mWeekStart

    /* 0x00 */ dTimeStamp_c mWeekStart; // 6:00 on this week's Monday
    /* 0x08 */ dTimeStamp_c mEventDay; // a weekend day picked at random (needs town flag 0x15), or none
    /* 0x10 */ u8 mDays[VISITOR_NPC_DAY_NUM]; // VISITOR_NPC_* by weekday, Sunday first
};
