#pragma once

// The fishing tourney (EVENT_FISHING_TOURNEY): the standings saved in the town
// (dSaveTown_c::mFishing), the villagers' simulated entries every 3 game minutes, the size of an
// entry, and the result letters and bulletin board notice after the event. The counterpart of the
// Bug-Off (d_bug_off), with fish sizes instead of scores. Source: src/dol/game/d_ev_fishing.cpp
// (.text 801511B4..80153818). The class, member and function names are inferred.
//
// Sizes are kept in tenths of an inch (length in cm * rate * 3.93707); toDisplaySize gives inches in
// the US / UK, centimetres elsewhere.

#include <types.h>
#include <game/game/d_time_stamp.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_animal_id.hpp>
#include <game/game/d_fg_item.hpp>

class dAnimal_c;

// Places in the standings: [0] the leader, [1] the runner-up. A villager entry has an empty mPlayer,
// a player entry an empty mAnimal.
#define FISHING_RANK_NUM 2

class dEvFishing_c {
public:
    void reset();                     // 801511B4: no event, empty standings
    void checkResult();               // 8015128C: once the event ran, the letters and the notice
    void sendResultLetters();         // 801512D0: to every entered player, the winner gets a prize
    void postResultNotice();          // 80151960: the results on the bulletin board (day after)
    void update();                    // 80151CBC: catchUp, checkResult
    void checkDay();                  // 80151CF0: a new day resets and sets up the event again
    void setup();                     // 80151F88: today's event times and fish, a first villager entry
    BOOL entryPlayer(const dPersonalID_c *pid, const dItem::Item *item, int *size); // 801521C4
    BOOL advance(BOOL force);         // 80152428: villager entries up to now (+30 minutes)
    BOOL catchUp();                   // 80152950: villager entries up to now
    BOOL tryNpcEntry(dAnimal_c *animal, dTime_c time, BOOL force); // 80152D10
    BOOL isNewRecord(int size);       // 8015339C: bigger than the leader's
    dPersonalID_c *getPlayer(int rank);    // 801533C8
    dAnmPersonalID_c *getAnimal(int rank); // 801533E0
    int getSize(int rank);                 // 801533F8
    dItem::Item getFish();                 // 80153410: the tourney's fish (none: any fish)
    dItem::Item getItem(int rank);         // 8015341C: the fish caught
    static f32 toDisplaySize(f32 size);    // 8015343C: inches (US / UK) or centimetres
    int pickAnimal(const dTime_c *time);   // 801534AC: random available villager, -1 if none
    BOOL isAnimalAvailable(int idx, const dTime_c *time, BOOL anyPlace, BOOL allowSick, BOOL allowMoving,
                           BOOL allowSleeping); // 801536B8

    // The leader moves to second place.
    void pushDown() {
        mPlayer[1].copy(&mPlayer[0]);
        mAnimal[1].copy(&mAnimal[0]);
        mSize[1] = mSize[0];
        mItem[1] = mItem[0];
    }

    /* 0x000 */ dTimeStamp_c mCursor; // how far the villager entries are simulated; none: no event
    /* 0x008 */ dTimeStamp_c mStart;
    /* 0x010 */ dTimeStamp_c mEnd;
    /* 0x018 */ dPersonalID_c mPlayer[FISHING_RANK_NUM];
    /* 0x070 */ dAnmPersonalID_c mAnimal[FISHING_RANK_NUM];
    /* 0x1F0 */ int mSize[FISHING_RANK_NUM];
    /* 0x1F8 */ dItem::Item mFish; // the tourney's fish (Jan, Mar, May, Nov), none otherwise
    /* 0x1FA */ dItem::Item mItem[FISHING_RANK_NUM]; // the fish caught
    /* 0x1FE */ u8 mLettersSent;
    /* 0x1FF */ u8 mNoticePending;
}; // size 0x200
