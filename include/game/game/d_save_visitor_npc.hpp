#pragma once

#include <types.h>
#include <game/game/d_time_stamp.hpp>

// Days in dSaveVisitorNpc_c::mDays (one dVisitor_e per weekday, d_event.hpp).
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
    u8 getDay(int wday); // 801100B8: dVisitor_e
    int getFreeDay(int exclude); // 801100C4: a random unscheduled weekday (1..5) other than exclude, or -1
    BOOL isNewWeek(int *weeks); // 8011026C: weeks since mWeekStart

    /* 0x00 */ dTimeStamp_c mWeekStart; // 6:00 on this week's Monday
    /* 0x08 */ dTimeStamp_c mEventDay; // a weekend day picked at random (needs town flag 0x15), or none
    /* 0x10 */ u8 mDays[VISITOR_NPC_DAY_NUM]; // dVisitor_e by weekday, Sunday first
};
