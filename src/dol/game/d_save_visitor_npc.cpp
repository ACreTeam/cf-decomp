// The town's weekly visitor schedule (dSaveVisitorNpc_c). .text 8010FB6C..8011041C.
#include <game/game/d_save_visitor_npc.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_event.hpp>
#include <game/cLib/c_math.hpp>

// 8010FB6C
dSaveVisitorNpc_c::dSaveVisitorNpc_c() {}

// 8010FBC0
void dSaveVisitorNpc_c::clear() {
    mWeekStart.reset();
    mEventDay.reset();
    for (int i = 0; i < VISITOR_NPC_DAY_NUM; i++) {
        mDays[i] = VISITOR_NPC_NOT_SET;
    }
}

// 8010FC14
void dSaveVisitorNpc_c::buildTodayEvents() {
    dEvent::buildSchedule(*dTime_c::getCurrent(), FALSE);
}

// 8010FC3C
void dSaveVisitorNpc_c::update() {
    int weeks;
    if (isNewWeek(&weeks)) {
        mWeekStart.setNow();
        mWeekStart.toDayStart();
        int wday = mWeekStart.getWeekday();
        mWeekStart.addDays(-wday);
        mWeekStart.addDays(1);

        int i;
        int last = 0;
        if (mDays[0] != VISITOR_NPC_NOT_SET) {
            for (i = 5; i >= 1; i--) {
                if (dEvent::isRandomVisitor(mDays[i])) {
                    last = mDays[i];
                    break;
                }
            }
        }

        for (i = 1; i <= 5; i++) {
            mDays[i] = VISITOR_NPC_NOT_SET;
        }
        mDays[0] = VISITOR_NPC_SUNDAY;
        mDays[6] = VISITOR_NPC_SATURDAY;

        int free = 5;
        dTime_c t = mWeekStart.get();
        for (i = 1; i <= 5; i++) {
            if (dEvent::getTownEventOn(&t) != -1) {
                mDays[i] = VISITOR_NPC_TOWN_EVENT;
                free--;
            }
            t.add(1, 0, 0, 0);
        }

        int num = 0;
        if (free > 0) {
            int chance;
            if (weeks <= 1) {
                chance = 10;
            } else {
                chance = (1 - weeks) * -10;
            }
            if (cM::rndInt(100) < chance) {
                int day = getFreeDay(-1);
                mDays[day] = VISITOR_NPC_WISP;
                num = 1;
                free--;
            }
        }

        if (free > 0 && !dSaveData_c::getRaw()->isFlag(0x12) && !dSaveData_c::getRaw()->isFlag(0x13) &&
            cM::rndInt(100) < 30) {
            int day = getFreeDay(5);
            if (day != -1) {
                mDays[day] = VISITOR_NPC_6;
                free--;
                num++;
            }
        }

        int first;
        if (free > 0 && num < 2) {
            first = dEvent::pickRandomVisitor(last, 0);
            int day = getFreeDay(-1);
            num++;
            mDays[day] = first;
            free--;
        }
        if (free > 0 && num < 2) {
            int second = dEvent::pickRandomVisitor(last, first);
            int day = getFreeDay(-1);
            mDays[day] = second;
        }

        for (i = 1; i <= 5; i++) {
            if (mDays[i] == VISITOR_NPC_NOT_SET) {
                mDays[i] = VISITOR_NPC_NONE;
            }
        }

        mEventDay.reset();
        BOOL ok = TRUE;
        t = mWeekStart.get();
        t.add(5, 0, 0, 0);
        if (dEvent::getTownEventOn(&t) != -1) {
            ok = FALSE;
        } else {
            t.add(1, 0, 0, 0);
            if (dEvent::getTownEventOn(&t) != -1) {
                ok = FALSE;
            }
        }
        if (!dSaveData_c::getRaw()->isFlag(0x15)) {
            ok = FALSE;
        }
        if (ok && cM::rndInt(100) < 50) {
            mEventDay = mWeekStart;
            if (cM::rndInt(100) < 50) {
                mEventDay.addDays(5);
            } else {
                mEventDay.addDays(6);
            }
        }

        // Wisp's lamp date (month, day): none.
        dMD_c *lampDate = &dSaveData_c::getTown()->mTownInfo.mLampDate;
        lampDate->month = 12;
        lampDate->day = 0;
    }
}

// 801100B8
u8 dSaveVisitorNpc_c::getDay(int wday) {
    return mDays[wday];
}

// 801100C4
int dSaveVisitorNpc_c::getFreeDay(int exclude) {
    int num = 0;
    for (int i = 1; i <= 5; i++) {
        if (mDays[i] == VISITOR_NPC_NOT_SET && exclude != i) {
            num++;
        }
    }
    int r = cM::rndInt(num);
    for (int i = 1; i <= 5; i++) {
        if (mDays[i] == VISITOR_NPC_NOT_SET && exclude != i) {
            if (r == 0) {
                return i;
            }
            r--;
        }
    }
    return -1;
}

// 8011026C
BOOL dSaveVisitorNpc_c::isNewWeek(int *weeks) {
    *weeks = 1;
    if (mWeekStart.isNone()) {
        return TRUE;
    }
    dTime_c now = *dTime_c::getCurrent();
    dTime_c cur = now;
    if (mWeekStart.compare(&cur) == -1) {
        return TRUE;
    }
    now.add(-7, 0, 0, 0);
    dTime_c weekAgo = now;
    if (mWeekStart.compare(&weekAgo) == 1) {
        *weeks = mWeekStart.diffDays(&now, TRUE, FALSE) / 7 + 1;
        return TRUE;
    }
    return FALSE;
}
