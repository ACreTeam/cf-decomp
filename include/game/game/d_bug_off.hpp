#pragma once

// Bug-Off (EVENT_BUG_OFF, the insect tournament): the standings saved in the town
// (dSaveData_c::mBugOff), the villagers' simulated entries every 3 game minutes, the score of an
// entry, and the result letters and bulletin board notice after the event. The host REL takes the
// players' entries (entryPlayer) and reads the standings. Source: src/dol/game/d_bug_off.cpp
// (.text 80113158..8011524C). The class, member and function names are inferred.

#include <types.h>
#include <game/game/d_time_stamp.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_animal_id.hpp>
#include <game/game/d_fg_item.hpp>

class dAnimal_c;


// Places in the standings: [0] the leader, [1] the runner-up. A villager entry has an empty mPlayer,
// a player entry an empty mAnimal.
#define BUG_OFF_RANK_NUM 2

class dBugOff_c {
public:
    void reset();                     // 80113158: no event, empty standings
    void checkResult();               // 8011322C: once the event ran, the letters and the notice
    void sendResultLetters();         // 80113270: to every entered player, the winner gets a prize
    void postResultNotice();          // 80113834: the results on the bulletin board (day after)
    void update();                    // 80113AEC: catchUp, checkResult
    void checkDay();                  // 80113B20: a new day resets and sets up the event again
    void setup();                     // 80113DB8: today's event times, a first villager entry
    BOOL entryPlayer(const dPersonalID_c *pid, const dItem::Item *item, int *score, int *size); // 80113F7C
    BOOL advance(BOOL force);         // 80114070: villager entries up to now (+30 minutes)
    BOOL catchUp();                   // 80114598: villager entries up to now
    BOOL tryNpcEntry(dAnimal_c *animal, dTime_c time, BOOL force); // 80114930
    static int getRank(int score);    // 80114BAC: 1..6 (<=20, <=40, <=60, <=80, <=99, 100)
    BOOL judge(BOOL isPlayer, const dItem::Item *item, int *score, int *size); // 80114C04: FALSE below the leader
    dPersonalID_c *getPlayer(int rank);    // 80114E84
    dAnmPersonalID_c *getAnimal(int rank); // 80114E9C
    int getScore(int rank);                // 80114EB4
    dItem::Item getItem(int rank);         // 80114ECC
    int pickAnimal(const dTime_c *time);   // 80114EEC: random available villager, -1 if none
    BOOL isAnimalAvailable(int idx, const dTime_c *time, BOOL anyPlace, BOOL allowSick, BOOL allowMoving,
                           BOOL allowSleeping); // 801150F8

    // The leader moves to second place.
    void pushDown() {
        mPlayer[1].copy(&mPlayer[0]);
        mAnimal[1].copy(&mAnimal[0]);
        mScore[1] = mScore[0];
        mItem[1] = mItem[0];
    }

    BOOL hasEnded(const dTime_c& now) const {
        dTime_c end = mEnd.get();

        return dTime_c::isSameOrAfter(now, end);
    }

    /* 0x000 */ dTimeStamp_c mCursor; // how far the villager entries are simulated; none: no event
    /* 0x008 */ dTimeStamp_c mStart;
    /* 0x010 */ dTimeStamp_c mEnd;
    /* 0x018 */ dPersonalID_c mPlayer[BUG_OFF_RANK_NUM];
    /* 0x070 */ dAnmPersonalID_c mAnimal[BUG_OFF_RANK_NUM];
    /* 0x1F0 */ int mScore[BUG_OFF_RANK_NUM];
    /* 0x1F8 */ dItem::Item mItem[BUG_OFF_RANK_NUM]; // the insect
    /* 0x1FC */ u8 mLettersSent;
    /* 0x1FD */ u8 mNoticePending;
    /* 0x1FE */ u8 _1FE[2];
}; // size 0x200
