#pragma once

// Quest save data. Source: src/dol/game/d_quest.cpp (.text 8013F458..80143E78).
// Class, method and field names are inferred unless they came with the symbols; the enums
// follow ac-decomp/afe-decomp m_quest.h, and names marked "?" are guesses.

#include <types.h>
#include <game/game/d_date.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_animal_id.hpp>
#include <game/game/d_quest_time.hpp>

class dMail_c;

// Quest category for a kind (dQuestBase_c::getKindType). Each type's kinds start at sKindBase[type].
enum dQuestType_e {
    QUEST_TYPE_REQUEST,       // kinds 0-6:   villager asks for something (dQuestVillager_c; AC "contest")
    QUEST_TYPE_ERRAND,        // kinds 7-16:  errands / Nook's part-time job (dQuestErrand_c, per player)
    QUEST_TYPE_APPOINTMENT,   // kinds 17-18: meet at hour:min (dQuestPlayerItem_c) ?
    QUEST_TYPE_STYLE,         // kind 19:     a villager remarks on another player's style (dQuestPlayerPair_c)
    QUEST_TYPE_HIDE_AND_SEEK, // kind 20:     3 villagers + a minute timer (dQuestPlayerAnimal_c) ?
    QUEST_TYPE_NONE,

    QUEST_TYPE_NUM = QUEST_TYPE_NONE,
};

enum dQuestKind_e {
    // QUEST_TYPE_REQUEST. dQuestWish_c kinds 0-4 map 1:1 onto these (wish 7 = random).
    QUEST_KIND_REQUEST_INSECT,                 // 0
    QUEST_KIND_REQUEST_FISH,                   // 1
    QUEST_KIND_REQUEST_FOSSIL,                 // 2
    QUEST_KIND_REQUEST_CLOTH,                  // 3
    QUEST_KIND_REQUEST_FTR,                    // 4
    QUEST_KIND_REQUEST_5,                      // 5  unknown
    QUEST_KIND_REQUEST_6,                      // 6  unknown

    // QUEST_TYPE_ERRAND. Errand type REQUEST = 7-8, FIRSTJOB = 9-16.
    QUEST_KIND_ERRAND_REQUEST,                 // 7  ?
    QUEST_KIND_ERRAND_REQUEST_FINAL,           // 8  ?
    QUEST_KIND_FIRSTJOB_CHANGE_CLOTH,          // 9  ? starts the job
    QUEST_KIND_FIRSTJOB_PLANT_FLOWER,          // 10 ?
    QUEST_KIND_FIRSTJOB_11,                    // 11 unknown, no AC counterpart
    QUEST_KIND_FIRSTJOB_DELIVER_FTR,           // 12 exotic bed (index 0x640, item 0xB710)
    QUEST_KIND_FIRSTJOB_SEND_LETTER,           // 13 letter to a villager
    QUEST_KIND_FIRSTJOB_DELIVER_CARPET,        // 14 exotic rug (index 0x319, item 0xA2C0)
    QUEST_KIND_FIRSTJOB_DELIVER_WATERING_CAN,  // 15 watering can (index 0x9A1, item 0xCEAC)
    QUEST_KIND_FIRSTJOB_POST_NOTICE,           // 16 ?

    // QUEST_TYPE_APPOINTMENT
    QUEST_KIND_APPOINTMENT_0,                  // 17 ?
    QUEST_KIND_APPOINTMENT_1,                  // 18 ?

    QUEST_KIND_STYLE,                          // 19
    QUEST_KIND_HIDE_AND_SEEK,                  // 20

    QUEST_KIND_NONE,                           // 21
    QUEST_KIND_NUM = QUEST_KIND_NONE,
};

// dQuestBase_c::getKindSubType (AC mQst_ERRAND_TYPE_*)
enum dQuestErrandType_e {
    QUEST_ERRAND_TYPE_REQUEST,  // 0: kinds 7-8
    QUEST_ERRAND_TYPE_FIRSTJOB, // 1: kinds 9-16 (required by dQuestErrandList_c::isActive)
    QUEST_ERRAND_TYPE_NONE,     // 2: anything else
};

// dQuestPlayerPair_c::mTopic, picked from STR_Q13 by dQuestPlayerPair_c::start
enum dQuestStyleTopic_e {
    QUEST_STYLE_TOPIC_CLOTHES,     // STR_Q13 1-3
    QUEST_STYLE_TOPIC_ACCESSORIES, // STR_Q13 4-6
    QUEST_STYLE_TOPIC_FURNITURE,   // STR_Q13 7-9
    QUEST_STYLE_TOPIC_WALLPAPER,   // STR_Q13 10-12
    QUEST_STYLE_TOPIC_CARPET,      // STR_Q13 13-15
    QUEST_STYLE_TOPIC_NONE,        // 5, set by clear

    QUEST_STYLE_TOPIC_NUM = QUEST_STYLE_TOPIC_NONE,
};

// dQuestBase_c::mDeadline: how getDeadline turns mTimeLimit into the expiry time
// (sDeadlines, lbl_8047638C). Values from QUEST_DEADLINE_NUM up use mTimeLimit as-is.
enum dQuestDeadline_e {
    QUEST_DEADLINE_NEXT_HOUR,   // 0 fn_8013F9E0: top of the next hour (the one after that from :50)
    QUEST_DEADLINE_NEXT_PERIOD, // 1 fn_8013FABC: next of noon / 6 PM / midnight
    QUEST_DEADLINE_MIDNIGHT,    // 2 fn_8013FBC0: midnight at the end of that day
    QUEST_DEADLINE_3_DAYS,      // 3 fn_8013FC68: 6 AM (day rollover), three days on
    QUEST_DEADLINE_12_HOURS,    // 4 fn_8013FD84: twelve hours on
    QUEST_DEADLINE_NOW,         // 5 NULL entry: the current time, so it never expires
    QUEST_DEADLINE_1_WEEK,      // 6 fn_8013FE80: seven days on
    QUEST_DEADLINE_LIMIT,       // 7 out of table: mTimeLimit itself (INT64_MAX = none when cleared)

    QUEST_DEADLINE_NUM = QUEST_DEADLINE_LIMIT,
};

#define QUEST_ERRAND_HANDLER_NUM 10 // lbl_805F2680, one per errand kind

// 0x02. What a villager currently wants (mKind < 8) and how much (0-100).
class dQuestWish_c {
public:
    dQuestWish_c();  // 8013F458
    ~dQuestWish_c(); // 8013F45C

    void clear();                                                // 8013F49C
    BOOL isValid();                                              // 8013F4B0: mKind < 8
    void set(int kind);                                          // 8013F4C0
    void setRandom();                                            // 8013F508
    void setValue(u8 value);                                     // 8013F544: clamped to 100
    u8 addValue(int delta);                                      // 8013F558
    int getQuestKind();                                           // 8013F5C8: toQuestKind(mKind)
    static BOOL isWishItem(const dItem::Item *item, int kind);   // 8013F5D0: item's BITM kind fits the wish
    BOOL isWishItem(const dItem::Item *item);                    // 8013F6C4
    static int toQuestKind(int kind);                             // 80143894: 7 picks one at random

    /* 0x0 */ u8 mKind;  // 8 when cleared
    /* 0x1 */ u8 mValue;
}; // size 0x2

// 0x0E. Shared head of every quest.
class dQuestBase_c {
public:
    dQuestBase_c();  // 8013F718
    ~dQuestBase_c(); // 8013F760

    void clear();                                                // 8013F7A0
    BOOL isActive() const;                                       // 8013F7E8: isValidKind(mKind)
    void set(int kind, const dItem::Item *item, dTime_c *limit, u8 deadline, u8 state); // 8013F7F0
    int getType() const;                                         // 8013F848
    int getSubType() const;                                      // 8013F850: dQuestErrandType_e
    static BOOL isDaytime(const dTime_c &time);                  // 8013F858: 5:00 <= hour < 22:00
    static BOOL isBeforeNight(const dTime_c &time);              // 8013F87C: hour < 22:00
    static int pickKind(const int *kinds, u32 num, dTime_c *time); // 8013F898
    void setTimeLimit(const dTime_c &time);                      // 8013F9D8
    dTime_c getTimeLimit() const;                                // 8013F9DC

    // Deadline calculators (sDeadlines, indexed by mDeadline).
    static dTime_c calcNextHour(const dTime_c &time);            // 8013F9E0: QUEST_DEADLINE_NEXT_HOUR
    static dTime_c calcNextPeriod(const dTime_c &time);          // 8013FABC: QUEST_DEADLINE_NEXT_PERIOD
    static dTime_c calcMidnight(const dTime_c &time);            // 8013FBC0: QUEST_DEADLINE_MIDNIGHT
    static dTime_c calc3Days(const dTime_c &time);               // 8013FC68: QUEST_DEADLINE_3_DAYS
    static dTime_c calc12Hours(const dTime_c &time);             // 8013FD84: QUEST_DEADLINE_12_HOURS
    static dTime_c calc1Week(const dTime_c &time);               // 8013FE80: QUEST_DEADLINE_1_WEEK

    dTime_c getDeadline(const dTime_c &now) const;               // 8013FF7C
    BOOL isPastDeadline(const dTime_c *now) const;               // 801400E4: NULL = now
    BOOL isExpired(const dTime_c *now) const;                    // 80140230: NULL = now

    static BOOL isValidKind(u32 kind);                           // 80143734
    static BOOL isValidType(u32 type);                           // 8014374C
    static int getKindType(int kind);                            // 80143764: dQuestType_e
    static int getKindSubType(int kind);                         // 801437BC: dQuestErrandType_e
    static BOOL getKindIndex(int *index, int kind);              // 8014381C: kind - first kind of its type
    static BOOL checkEventSchedule(int kind, const dTime_c *time); // 801438F8: NULL = now
    static BOOL checkTodayEvents(int kind);                      // 80143BA4

    /* 0x00 */ dQuestTime_c mTimeLimit;
    /* 0x08 */ dItem::Item mItem; // ITEM_ID_NONE when cleared
    /* 0x0A */ u8 mKind;          // dQuestKind_e; QUEST_KIND_NONE when cleared
    /* 0x0B */ u8 mState;
    /* 0x0C */ u8 mDeadline;      // dQuestDeadline_e; QUEST_DEADLINE_LIMIT when cleared
}; // size 0xE

// 0x190. One errand / part-time job step (QUEST_TYPE_ERRAND).
class dQuestErrand_c {
public:
    dQuestErrand_c();  // 80140650
    ~dQuestErrand_c(); // 80140680

    void clear(); // 801406D8
    BOOL start(int kind, const dAnmPersonalID_c *animal0, const dAnmPersonalID_c *animal1,
               const dItem::Item *item, dTime_c *limit, u8 deadline, u8 state); // 8014073C
    dAnmPersonalID_c *getAnimal(int i);             // 80140840
    const dAnmPersonalID_c *getAnimal(int i) const; // 80140850

    /* 0x000 */ dQuestBase_c mBase;
    /* 0x00E */ dAnmPersonalID_c mAnimals[2]; // recipient, sender
    /* 0x18E */ u8 _18E;
}; // size 0x190

// 0x3D0 at dPrivateData_c+0x7FEE. The player's errand plus the villagers already used by the job.
// The const overloads are the ones called from const contexts (dPrivateData_c::findErrand, isActive);
// they hand back non-const pointers.
class dQuestErrandList_c {
public:
    dQuestErrandList_c();  // 80140860
    ~dQuestErrandList_c(); // 801408A8

    void clearAnimals();                                         // 8014090C
    dAnmPersonalID_c *getAnimal(int i);                          // 80140958
    dAnmPersonalID_c *getAnimal(int i) const;                    // 80140968
    void clear();                                                // 80140978
    dQuestErrand_c *getErrand(int i);                            // 801409AC
    dQuestErrand_c *getErrand(int i) const;                      // 801409B8
    dQuestErrand_c *get(u32 i);                                  // 801409C4: NULL when out of range
    dQuestErrand_c *get(u32 i) const;                            // 801409E0
    BOOL isActive() const;                                       // 801409FC: an active part-time job step
    u8 getKind() const;                                          // 80140A54
    BOOL isState(u8 state) const;                                // 80140AA0
    BOOL setState(u8 state);                                     // 80140B04
    void resetState(u8 state);                                   // 80140B60
    dItem::Item *getItem() const;                                // 80140BC0
    const dAnmPersonalID_c *getSender() const;                   // 80140BE8
    void startChangeCloth();                                     // 80140C4C
    void startPlantFlower();                                     // 80140CB0
    void start11();                                              // 80140D00
    void startDeliverFtr();                                      // 80140D50
    void startDeliverFtrAgain();                                 // 80140E00: avoids the last sender
    void startSendLetter();                                      // 80140F28
    void startSendLetter2();                                     // 8014100C
    BOOL checkLetter(dMail_c *mail);                             // 80141010
    void startDeliverCarpet();                                   // 801411B8
    void startDeliverCarpetAgain();                              // 801412AC: avoids the last sender
    void startDeliverWateringCan();                              // 80141420
    void startPostNotice();                                      // 80141550
    void notifyNoticePosted();                                   // 801415A0
    void cancel();                                               // 801415FC
    BOOL isSender(const dAnmPersonalID_c *animal) const;         // 80141640
    BOOL isExpired(const dTime_c &now) const;                    // 80141748

    /* 0x000 */ dQuestErrand_c mErrands[1];
    /* 0x190 */ dAnmPersonalID_c mAnimals[3];
}; // size 0x3D0

// 0x80. A villager's request (QUEST_TYPE_REQUEST). Lives in dQuestVillagerWish_c.
class dQuestVillager_c {
public:
    dQuestVillager_c();  // 80141814
    ~dQuestVillager_c(); // 80141844

    void clear(); // 8014189C
    BOOL start(int kind, const dPlayerID_c *player, const dItem::Item *item, dTime_c *limit, u8 deadline,
               u8 state);                                        // 80141910
    BOOL isValidPlayerIndex(u32 i);                              // 801419D0
    void setPlayerFlag(int i);                                   // 801419E8
    void clearPlayerFlag(int i);                                 // 80141A3C
    BOOL getPlayerFlag(int i);                                   // 80141A90
    dPlayerID_c *getPlayer(int i);                               // 80141AE4
    int findPlayer(const dPlayerID_c *player);                   // 80141B38
    void removeMissingPlayers();                                 // 80141BC8: players no longer in the save
    int findEmptyPlayer();                                       // 80141C88
    BOOL addPlayer(const dPlayerID_c *player, BOOL flag);        // 80141CE8
    int countPlayers(BOOL all);                                  // 80141E18
    dPlayerID_c *pickOtherPlayer(const dPlayerID_c *exclude);    // 80141EA4
    void removePlayer(const dPlayerID_c *player);                // 80141FC0
    BOOL getPlayerFlag(const dPlayerID_c *player);               // 80142028
    void clearPlayerFlag(const dPlayerID_c *player);             // 801420A4
    void setRequester(const dPlayerID_c *player);                // 80142118
    void clearRequester();                                       // 80142120
    static int getInsectPriceRank(const dItem::Item *item);      // 80142128: 0-2, 3 if not an insect
    static int getFishPriceRank(const dItem::Item *item);        // 801421C8: 0-2, 3 if not a fish

    /* 0x00 */ dQuestBase_c mBase;
    /* 0x0E */ dPlayerID_c mRequester;
    /* 0x24 */ dPlayerID_c mPlayers[4];
    /* 0x7C */ u8 mPlayerFlags; // bit i for mPlayers[i]
    /* 0x7D */ u8 mMatchMode;
    /* 0x7E */ u8 mMatchParam;
}; // size 0x80

// 0x0A at dAnimalBlock_c+0x1E2CA. Lost-item quest; mKeyIdx is one of the 8 "key" items
// (index group 0xBF, BITM kind 0x35).
class dLostQuest_c {
public:
    dLostQuest_c(); // 80142268

    void clear();                                                // 801422A4
    void set(const dTime_c &time, int value, const dItem::Item *item); // 801422DC
    void setTime(const dTime_c &time);                           // 80142330
    dTime_c getTime();                                           // 80142334
    void fn_80142338(const dItem::Item *item);                   // 80142338: mKeyIdx from seeker_c::findLike
    dItem::Item fn_801423D8();                                   // 801423D8: random key other than mKeyIdx
    BOOL fn_801424BC(dTime_c *now);                              // 801424BC

    /* 0x0 */ dQuestTime_c mTime;
    /* 0x8 */ s8 _08;     // -1 when cleared
    /* 0x9 */ s8 mKeyIdx; // -1 when cleared
}; // size 0xA

// 0x46 at dAnimalBlock_c+0x1E284. A sick villager (cf. AFE mQst_CONTEST_KIND_SICK).
class dQuestSick_c {
public:
    dQuestSick_c(); // 801426DC

    void clear();                                                // 801426E0
    static dItem::Item getMedicine();                            // 80142748: item index 11
    static int getMaxSickness(u32 i);                            // 80142750
    void addSickness(int delta, u32 i);                          // 80142780: clamps to [-4, getMaxSickness(i)]
    void start(int animalIdx);                                    // 801427E4
    void addVisit();                                             // 80142844
    dPlayerID_c *getPlayer(u32 i);                               // 80142870
    void setPlayer(u32 i, const dPlayerID_c *player);            // 8014288C
    void clearPlayer(u32 i);                                     // 801428A8
    BOOL hasPlayer(const dPlayerID_c *player);                   // 801428C0
    int countPlayers();                                          // 80142958

    /* 0x00 */ dPlayerID_c mPlayers[3];
    /* 0x42 */ s8 mAnimalIdx; // sick villager, -1 when cleared
    /* 0x43 */ u8 _43;
    /* 0x44 */ s8 mSickness;
    /* 0x45 */ u8 _45;        // counts up to 3
}; // size 0x46

// 0x2A at dAnimalBlock_c+0x1E2D4. Meet a villager at mHour:mMinute (QUEST_TYPE_APPOINTMENT).
class dQuestPlayerItem_c {
public:
    dQuestPlayerItem_c(); // 801429C8

    void clear();                                                // 80142A08
    dTime_c getMeetTime();                                       // 80142A64
    BOOL isPastMeetTime(const dTime_c &now, int mins);           // 80142C2C
    void start(u8 kind, const dPlayerID_c *player, int animalIdx, u8 hour, u8 min, dTime_c *limit); // 80142DA0
    void setFlag(int bit);                                       // 80142E2C
    BOOL isFlag(u32 bit);                                        // 80142E48

    /* 0x00 */ dPlayerID_c mPlayer;
    /* 0x16 */ dQuestBase_c mBase;
    /* 0x24 */ dItem::Item mItem;
    /* 0x26 */ s8 mAnimalIdx; // -1 when cleared
    /* 0x27 */ u8 mHour;
    /* 0x28 */ u8 mMinute;
    /* 0x29 */ u8 mFlags;
}; // size 0x2A

// 0xEC at dAnimalBlock_c+0x1E2FE. Hide and seek with up to 3 villagers (QUEST_TYPE_HIDE_AND_SEEK).
class dQuestPlayerAnimal_c {
public:
    dQuestPlayerAnimal_c(); // 80142E68

    void clear();                                                // 80142EA8
    void clearInfo();                                            // 80142EDC
    int getHider(u32 i);                                         // 80142F5C
    void setHider(u32 i, int animalIdx);                         // 80142F7C
    void fn_80142F90(int delta);                                 // 80142F90: clamps _E9 to [-2, 10]
    int countHiders();                                           // 80142FC0
    void start(int hider0, int hider1, int hider2, const dPlayerID_c *player, dTime_c *limit); // 80143028
    dTime_c getEndTime(BOOL half);                               // 80143120
    BOOL isTimeUp(const dTime_c &now, BOOL half);                // 8014323C
    void setFlag(u32 i);                                         // 8014337C: hider i found
    BOOL isFlag(u32 i);                                          // 8014339C
    int countUnfound();                                          // 801433BC

    /* 0x00 */ dPlayerID_c mPlayer;
    /* 0x16 */ dAnmPersonalID_c mAnimal;
    /* 0xD6 */ dQuestBase_c mBase;
    /* 0xE4 */ dItem::Item mItem;
    /* 0xE6 */ s8 mHiders[3];  // villager indices, -1 when unused
    /* 0xE9 */ s8 _E9;
    /* 0xEA */ u8 mMinutes;    // time allowed
    /* 0xEB */ u8 mFoundFlags; // bit i for mHiders[i]
}; // size 0xEC

// 0x60 at dAnimalBlock_c+0x1E3EA. A villager remarks on another player's style (QUEST_TYPE_STYLE).
class dQuestPlayerPair_c {
public:
    dQuestPlayerPair_c(); // 80143438

    void clear();                                                // 8014346C
    void clearInfo();                                            // 801434A0
    void start(int animalIdx, const dPlayerID_c *player0, const dPlayerID_c *player1, dTime_c *limit); // 80143520
    dPlayerID_c *getPlayer(int i);                               // 801435DC
    void setTopicText(u16 index);                                // 801435E8: "sys_STRING/STR_Q13" entry

    /* 0x00 */ dPlayerID_c mPlayers[2];
    /* 0x2C */ dQuestBase_c mBase;
    /* 0x3A */ wchar_t mTopicText[17]; // memset by clearInfo
    /* 0x5C */ s8 mAnimalIdx;          // -1 when cleared
    /* 0x5D */ u8 mTopic;              // dQuestStyleTopic_e
    /* 0x5E */ u8 _5E;
}; // size 0x60

// 0x82 at dAnimal_c+0x2BE4: the villager's wish and its request.
class dQuestVillagerWish_c {
public:
    dQuestVillagerWish_c();  // 80143660
    ~dQuestVillagerWish_c(); // 80143698

    void clear(); // 80143700

    /* 0x00 */ dQuestWish_c mWish;
    /* 0x02 */ dQuestVillager_c mQuest;
}; // size 0x82
