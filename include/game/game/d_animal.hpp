#pragma once

// Villager (animal) save data. Draft; see notes/d_animal.txt.
// Class names except dGreetingWord_c / dHabitWord_c (RTTI) are inferred.

#include <types.h>
#include <game/game/d_item.hpp>
#include <game/game/d_land.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_animal_id.hpp>
#include <game/game/d_dsn.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_script.hpp>
#include <game/game/d_save_melody.hpp>

class dAnimal_c;
class dAnimalBlock_c;
class dPrivateData_c;
class mVec3_c;

// Filter for the memory searches (getReplaceMemoryIdx).
typedef BOOL (*dAnimalMemoryFilter)(const dPersonalID_c *pid, const dLandID_c *land, dPrivateData_c *players);

#define ANIMAL_NUM 10        // villagers per town (fn_80129A3C)
#define ANIMAL_MEMORY_NUM 16 // fn_8011E2C0
#define ANIMAL_GREETING_LEN 16
#define ANIMAL_NICKNAME_LEN 8
#define ANIMAL_HABIT_LEN 10

// Script words for a villager's greeting and catchphrase. Names from RTTI.
// The vtables also hold many inherited dScript::Word_c entries not declared in d_script.hpp.
class dGreetingWord_c : public dScript::Word_c { // vtable 804EF0C8
public:
    dGreetingWord_c(); // 8011B400
    virtual ~dGreetingWord_c(); // 8011B444
    virtual u32 getBufferSize(); // 8011B49C
    virtual wchar_t *getBuffer(); // 8011B4A4

    /* 0x24 */ wchar_t mBuffer[ANIMAL_GREETING_LEN + 1];
}; // size 0x48

class dHabitWord_c : public dScript::Word_c { // vtable 804EEFD0
public:
    dHabitWord_c(); // 8011B4AC
    virtual ~dHabitWord_c(); // 8011B4F0
    virtual u32 getBufferSize(); // 8011B548
    virtual wchar_t *getBuffer(); // 8011B550

    /* 0x24 */ wchar_t mBuffer[ANIMAL_HABIT_LEN + 1];
}; // size 0x3C

// Two capped counters (max 5) at dAnimalMemory_c+0x4E. The kind argument goes
// through fn_80162548 / fn_80162594 to pick which counter applies.
class dAnimalTalkCount_c {
public:
    dAnimalTalkCount_c(); // 8011C618
    ~dAnimalTalkCount_c(); // 8011C61C
    void clear(); // 8011C65C
    void inc(u32 kind); // 8011C66C
    u8 get(u32 kind); // 8011C6E4

    /* 0x0 */ u8 mCount;
    /* 0x1 */ u8 mCountNoAttr5;
}; // size 0x2

// Flag word at dAnimalMemory_c+0x00.
// Bits 24, 23, 21, 20 and 10-8 are set and tested by the talk code (not decompiled yet).
struct dAnimalMemoryFlags_c {
    u32 mGotLetter : 1;           // this player's letter was taken into dAnimal_c::mLetter
    u32 mTalkDays : 3;            // consecutive-day talk streak, max 4 (setTalkDays)
    u32 mHasGreeting : 1;         // mGreeting is set (setGreeting)
    u32 mHasNickname : 1;         // mNickname is set (setNickname)
    u32 mSameTown : 1;            // player is from this town
    u32 _24 : 1;                  // set by event talks; cleared at event start/end
    u32 _23 : 1;                  // set after a once-only talk; cleared on move-out
    u32 mTunekichiInvite : 1;     // pending Tunekichi invite letter
    u32 _21 : 1;                  // cleared daily
    u32 _20 : 1;                  // cleared daily
    u32 mNicknameWait : 4;        // days before asking for a nickname again (decNicknameWait)
    u32 mGreetingWait : 3;        // days before asking for a greeting again (decGreetingWait)
    u32 mBirthdayLetterSent : 1;  // birthday letter sent
    u32 mBirthdayDone : 1;        // birthday present given
    u32 _10 : 1;                  // set with _86 by the talk code; cleared daily
    u32 _9 : 1;
    u32 _8 : 1;                   // cleared on move-out
    u32 _lo : 8;
}; // size 0x4

// A villager's memory of one player (AC's Anmmem_c equivalent). 0x8C.
class dAnimalMemory_c {
public:
    dAnimalMemory_c(); // 8011C73C
    ~dAnimalMemory_c(); // 8011C79C
    void clear(); // 8011C7F8

    void setEmptyItem() {
        dItem::Item none;
        setPresent(&none);
    }

    BOOL init(const dPersonalID_c *pid, const dLandID_c *land, const dTime_c *time); // 8011C8A8
    BOOL init(const dPersonalID_c *pid, s8 a, u16 count, const dLandID_c *land, const dTime_c *time); // 8011C93C
    void updateTalk(const dPersonalID_c *pid, const dLandID_c *land, const dTime_c *time); // 8011C9D0
    void set(s8 a, u16 count, const dPersonalID_c *pid, const dLandID_c *land, const dTime_c *time); // 8011CB4C
    void setTalkDays(u16 count); // 8011CCCC
    BOOL calcTalkDays(const dTime_c *now); // 8011CCE8
    s8 addTalkFriendship(); // 8011CF38
    s8 getFriendship(); // 8011CF58
    void setFriendship(s8 value); // 8011CF64
    s8 addFriendship(s8 delta); // 8011CF6C
    int getFriendshipLevel(s8 value); // 8011CFE0
    int getFriendshipLevel(); // 8011D008
    u32 hasLetter(); // 8011D044
    void setNickname(const wchar_t *name, u32 len); // 8011D050
    void setNickname(dScript::Word_c *word); // 8011D0C8
    void getNickname(dScript::Word_c *word); // 8011D134
    void decNicknameWait(int num); // 8011D148
    void copyNickname(dAnimalMemory_c *other); // 8011D184
    u32 hasGreeting(); // 8011D1F4
    void setGreeting(const wchar_t *str, u32 len); // 8011D200
    void getGreeting(dScript::Word_c *word); // 8011D278
    void decGreetingWait(int num); // 8011D28C
    BOOL setPresent(const dItem::Item *item); // 8011D2C8
    void updateLetterCond(int value); // 8011D3B4
    void onEventFlag(u32 bit); // 8011D3F0
    void offEventFlag(u32 bit); // 8011D410
    BOOL isEventFlag(u32 bit); // 8011D430


    /* 0x00 */ dAnimalMemoryFlags_c mFlags;
    /* 0x04 */ dTimeStamp_c mLastTalkTime;
    /* 0x0C */ dPersonalID_c mPlayer;
    /* 0x38 */ dLandID_c mLand;           // cleared by dLandID_c::clear
    /* 0x4E */ dAnimalTalkCount_c mTalkCount;
    /* 0x50 */ wchar_t mNickname[PLAYER_NAME_LEN + 1]; // player name
    /* 0x62 */ wchar_t mGreeting[17];          // 0x22 bytes, memset by clear
    /* 0x84 */ dItem::Item mPresent;
    /* 0x86 */ dItem::Item _86;
    /* 0x88 */ u8 mFriendship;
    /* 0x89 */ u8 mImpression;                  // '1' when cleared
    /* 0x8A */ u8 mEventFlags;
}; // size 0x8C

// 3 bytes at dAnimal_c+0x2FF6.
class dAnimalEventState_c {
public:
    dAnimalEventState_c(); // 8011D450
    ~dAnimalEventState_c(); // 8011D454
    void clear(); // 8011D494
    void setPlace(u8 state); // 8011D4AC
    BOOL isInEvent(); // 8011D4E8
    void setFlag(u32 bit); // 8011D4F8
    void clearFlag(u32 bit); // 8011D518
    BOOL isFlag(u32 bit); // 8011D538


    /* 0x0 */ u8 mPlace; // 3 when cleared
    /* 0x1 */ u8 _1;
    /* 0x2 */ u8 mFlags;
};

// 0x14 at dAnimal_c+0x301C. setD013Spot fills _08/_0C/_10/_11.
class dAnimalSpot_c {
public:
    dAnimalSpot_c(); // 8011D558
    ~dAnimalSpot_c(); // 8011D594
    void clear(); // 8011D5D4
    BOOL isValid(); // 8011D61C
    dTime_c getTime(); // 8011D650
    s64 getTimeRaw(); // 8011D654
    void setTime(const dTime_c *time, int mins); // 8011D658
    void setTimeRaw(const s64 *value); // 8011D6FC
    BOOL pickWishSpot(dAnimal_c *animal, dAnimalBlock_c *block); // 8011DDE8
    BOOL setHomeSpot1(dAnimal_c *animal, dAnimalBlock_c *block); // 8011DFC4
    BOOL fn_8011E080(dAnimal_c *animal, dAnimalBlock_c *block); // 8011E080
    BOOL fn_8011E140(dAnimal_c *animal, dAnimalBlock_c *block); // 8011E140
    BOOL setD013Spot(dAnimal_c *animal, dAnimalBlock_c *block); // 8011E200


    /* 0x00 */ dTimeStamp_c mTime;
    /* 0x08 */ s32 mX; // -1 when cleared
    /* 0x0C */ s32 mZ; // -1 when cleared
    /* 0x10 */ u8 mType;  // 4 when cleared
    /* 0x11 */ u8 mGroup;
}; // size 0x14

// Unrecovered member objects. Their constructors and destructors live in other TUs.
// dQuestVillagerWish_c (wish + villager request) is in d_quest.hpp.

// Per-species template record, copied whole into dAnimal_c+0x1824 by fn_8011E688.
struct dAnimalTemplate_c {
    s16 getUmbrella() const { return mUmbrella; }

    /* 0x000 */ s16 mNpcIdx; // npc index
    /* 0x002 */ s16 mCloth; // -> dAnimal_c::_2FFA
    /* 0x004 */ s16 mCarpet; // setCarpet
    /* 0x006 */ s16 mWall; // setWall
    /* 0x008 */ s16 mUmbrella;      // getUmbrella / setUmbrella
    /* 0x00A */ s16 mFurniture[10]; // house furniture slots (getTemplateFtr)
    /* 0x01E */ s16 mMusic;           // music (getMusic / setMusic)
    /* 0x020 */ u8 _020[2];
    /* 0x022 */ wchar_t mNames[REGION_NUM][ANIMAL_NAME_LEN + 1];
    /* 0x0B2 */ wchar_t mHabits[LANGUAGE_NUM][ANIMAL_HABIT_LEN + 1]; // getHabit / setHabit
    /* 0x18E */ u8 mSpecies;
    /* 0x18F */ u8 mBirthMonth;
    /* 0x190 */ u8 mBirthDay;
    /* 0x191 */ s8 mRoomLayout; // getRoomLayout
    /* 0x192 */ s8 mLikedStyle; // clothing / design style (getStyleMatch, pickClothRequestItemLikedStyle, pickDesignToWear)
    /* 0x193 */ s8 mDislikedStyle; // second clothing style (pickClothRequestItemNotDisliked)
    /* 0x194 */ u8 mFavFtrColor; // pickFtrRequestColor
    /* 0x195 */ s8 mFavFtrSeries; // furniture series (pickOwnItem, pickFtrRequestSeries)
    /* 0x196 */ u8 mLooks : 4;
    /* 0x196 */ u8 mLikesNewFtr : 1; // pickFtrRequestTaste
    /* 0x196 */ u8 mLikesOldFtr : 1;
    /* 0x196 */ u8 mLikesAdultFtr : 1;
    /* 0x196 */ u8 mLikesKiddyFtr : 1;
    /* 0x197 */ u8 mIsStarter : 1; // pickTemplateByLooks
    /* 0x197 */ u8 _197_lo : 7;
}; // size 0x198

// One villager's save record. 0x3040, alignment 32 (dDesign_c).
class dAnimal_c {
public:
    dAnimal_c(); // 8011E2C0
    ~dAnimal_c(); // 8011E39C
    void clear(); // 8011E458
    void copy(const dAnimal_c *other); // 8011E544
    BOOL isChecksumValid(); // 8011E54C
    u32 getChecksum(); // 8011E5C0
    u32 calcChecksum(); // 8011E61C
    void init(u16 npcIdx, u8 arg, const dLandID_c *land, const dAnimalTemplate_c *tmpl); // 8011E688
    void setSetupData(const void *src); // copies the 0x1824-byte head and sets mSetupLoaded
    BOOL isMoving(); // 80122350: used by dQuestErrandList_c::startDeliverWateringCan
    BOOL usesNickname(const dPersonalID_c *pid); // 8011F090

    // Request item pickers (pointer-to-member tables in pickRequestItem, pickClothRequest, pickFtrRequest).
    typedef u32 (dAnimal_c::*RequestPickFunc)(dItem::Item *item, u8 *arg);
    u32 pickInsectRequest(dItem::Item *item, u8 *arg); // 80124AF0: insect
    u32 pickFishRequest(dItem::Item *item, u8 *arg); // 80124B5C: fish
    u32 pickFossilRequest(dItem::Item *item, u8 *arg); // 80124BC8: fossil
    u32 pickClothRequestNotDisliked(dItem::Item *item, u8 *arg); // 80124C44
    u32 pickClothRequestLikedStyle(dItem::Item *item, u8 *arg); // 80124C4C
    u32 pickClothRequestItemNotDisliked(dItem::Item *item, u8 *arg); // 80124C54: clothing
    u32 pickClothRequestItemLikedStyle(dItem::Item *item, u8 *arg); // 80124CEC: clothing
    u32 pickClothRequest(dItem::Item *item, u8 *arg); // 80124D84: clothing; returns the isClothRequestMatch mode
    u32 pickFtrRequestCategory(dItem::Item *item, u8 *arg); // 80124E10: furniture conditions return the checkFtrRequest mode (3-6)
    u32 pickFtrRequestColor(dItem::Item *item, u8 *arg); // 80124E4C
    u32 pickFtrRequestTaste(dItem::Item *item, u8 *arg); // 80124ED0
    u32 pickFtrRequestSeries(dItem::Item *item, u8 *arg); // 80125044
    u32 pickFtrRequest(dItem::Item *item, u8 *arg); // 801250C8: furniture
    int getMemoryIdx(const dPersonalID_c *pid); // 8011E7C8
    dAnimalMemory_c *getMemory(u32 idx); // 8011E888
    dAnimalMemory_c *getMemory2(u32 idx); // 8011E8A8
    dAnimalMemory_c *findMemory(const dPersonalID_c *pid); // 8011E8C8
    dAnimalMemory_c *findMemory2(const dPersonalID_c *pid); // 8011E900
    int getFreeMemoryIdx(); // 8011E938
    int getReplaceMemoryIdx(dAnimalMemoryFilter filter); // 8011E9A4
    int getMovedOutPlayerMemoryIdx(); // 8011EC68
    int getOtherTownMemoryIdx(); // 8011ECE0
    u32 getNewMemoryIdx(); // 8011ECEC
    int getRandomMemoryIdx(const dPersonalID_c **exclude, u32 num, BOOL checkTown); // 8011ED38
    dAnimalMemory_c *getRandomMemory(const dPersonalID_c **exclude, u32 num, BOOL checkTown); // 8011EF48
    int getMemoryNum(); // 8011EF80
    void clearInvalidPresents(); // 8011EFF0
    void getPlayerCallName(dScript::Word_c *word, const dPersonalID_c *pid); // 8011F110
    BOOL getGreeting(dScript::Word_c *word, const dPersonalID_c *pid); // 8011F334
    dItem::Item getFtr(u32 idx); // 8011F468
    void clearFtr(u32 idx); // 8011F478
    BOOL setFtr(u32 idx, const dItem::Item *item); // 8011F518
    int countFtr(); // 8011F638
    int findFtr(const dItem::Item *item); // 8011F6B8
    u32 countFossilSetFtr(int fossil, int start, int end); // 8011FB14
    BOOL placeFtr(const dItem::Item *item, int idx, BOOL notify, dItem::Item *out); // 8011FB94
    BOOL throwAwayFtr(); // 8011FFE0
    void applyRoomFtr(dItem::Item *room); // 80120238
    void validateHouse(); // 80120388
    void clearFossilRoomFtr(); // 80120548
    void clearNewItems(); // 80120584
    dItem::Item *fn_801205A0(u32 idx); // 801205A0
    dItem::Item *fn_801205C0(u32 idx); // 801205C0
    void packNewItems(); // 801205E0
    int findNewItem(const dItem::Item *item); // 80120664
    BOOL addNewItem(const dItem::Item *item); // 80120794
    BOOL removeNewItem(const dItem::Item *item); // 80120844
    void validateNewItems(); // 801208B4
    dItem::Item *pickNewItem(const dItem::Item *exclude, u32 num, BOOL all); // 80120944
    dItem::Item *pickPricedNewItem(const dItem::Item *exclude, u32 num, BOOL all); // 80120A50
    dItem::Item pickNewItemOfKind(int kind, const dItem::Item *exclude); // 80120BC0
    void applyNewItems(u8 idx, BOOL notify); // 80120D00
    u32 countNewItems(); // 80121234
    dItem::Item pickOwnItem(); // 801212A0
    BOOL updateOwnItems(); // 80121738
    int getRoomLayout(int *type); // 8012188C
    void setCloth(const dItem::Item *item); // 8012194C
    BOOL wearTailorDesign(u32 idx); // 80121A10
    BOOL isWearingOrgCloth(); // 80121AC8
    BOOL isWearingCloth(); // 80121AD0
    BOOL setDesignFromTailor(u32 idx); // 80121B3C
    BOOL wearDesign(const dDesign_c *design); // 80121D94
    int getStyleMatch(const dItem::Item *item); // 80121FEC
    void setWall(const dItem::Item *item); // 8012209C
    dItem::Item *getWall(); // 80122130
    void setCarpet(const dItem::Item *item); // 80122138
    dItem::Item *getCarpet(); // 801221CC
    dItem::Item getMusic(); // 801221D4
    void setMusic(const dItem::Item *item); // 801221E0
    void setMovingIn(); // 80122274
    BOOL isMovingIn(); // 80122284
    void setMovingOut(); // 801222D8
    BOOL isMovingOut(); // 801222EC
    void clearMoving(); // 80122340
    void setQuestStarted(); // 80122368
    void clearQuestStarted(); // 80122378
    BOOL isQuestStarted(); // 80122388
    void clearPlaceChangeTime(); // 801223A0
    dTime_c getPlaceChangeTime(); // 801223A8
    BOOL isPlaceChangeTimeSet(); // 801223B0
    void setPlaceChangeTime(int mins, const dTime_c *time); // 801223DC
    BOOL isPlaceChangeTimeReached(const dTime_c *now); // 80122498
    BOOL isVersionValid(); // 801225F4
    s32 getVersion(); // 80122620
    void initVersion(); // 80122628
    dItem::Item getKey(); // 80122634
    void setHabitFromWord(dScript::Word_c *word, int arg); // 80122684
    void getHabitWord(dScript::Word_c *word, int arg, BOOL itchy); // 801226DC
    void decHabitCooldown(int num); // 80122770
    BOOL receiveLetter(dMail_c *mail, int slot); // 8012279C
    BOOL hasAnyLetter(); // 801228B4
    BOOL hasLetterFrom(const dPersonalID_c *pid); // 8012294C
    BOOL writeReplyLetter(dMail_c *mail, dPrivateData_c *player); // 80122A60
    int getDayPlace(BOOL checkPlayer, BOOL arg2, BOOL useDefault); // 80123240
    void clearTalkCounts(); // 80123374
    void clearMemoryFlag24(); // 801233C8
    void clearEventFlags(); // 801234AC
    void clearMemoryFlag23(); // 80123514
    void fn_801235F8(); // 801235F8
    void fn_801236DC(); // 801236DC
    void fn_801237C0(); // 801237C0
    void fn_801238A4(); // 801238A4
    void fn_80123988(); // 80123988
    void fn_80123A6C(); // 80123A6C
    void decNicknameWait(int n); // 80123B50
    void decGreetingWait(int n); // 80123BBC
    void growWish(); // 80123C28
    u32 pickErrandReward(dItem::Item *item, int *price, dPrivateData_c *player); // 80123C6C
    BOOL sendErrandThanksLetter(const dPersonalID_c *to, dAnmPersonalID_c *other, const dItem::Item *present, BOOL tryDirect); // 80123E5C
    BOOL completeErrandRequest(u32 idx, dPrivateData_c *player, BOOL tryDirect); // 80124084
    u32 pickErrandFinalReward(dItem::Item *item, int *price, dPrivateData_c *player, u32 a, u32 b); // 801242C8
    BOOL completeErrandRequestFinal(u32 idx, dPrivateData_c *player, BOOL tryDirect); // 8012453C
    BOOL expireErrandRequest(dPrivateData_c *player, BOOL enable); // 801247A0
    void updateLostItemRequest(); // 801249B4
    BOOL clearExpiredRequest(dLostQuest_c *lost, BOOL enable); // 80124A34
    u32 pickRequestItem(dItem::Item *item, u8 *arg, int kind); // 8012514C
    u32 pickInsectFishReward(dItem::Item *item, int *price, dPrivateData_c *player, int mode, const dItem::Item *exclude); // 8012528C
    u32 pickFossilReward(dItem::Item *item, int *price, dPrivateData_c *player, int mode, const dItem::Item *exclude); // 801255B8
    BOOL isClothRequestMatch(const dItem::Item *item, int mode, const dItem::Item *other); // 80125878
    u32 pickClothReward(dItem::Item *item, int *price, dPrivateData_c *player, int mode, const dItem::Item *exclude); // 801259DC
    u32 checkFtrRequest(const dItem::Item *item, int mode, int value); // 80125C38
    u32 pickFtrReward(dItem::Item *item, int *price, dPrivateData_c *player, int mode, const dItem::Item *exclude); // 80125ED8
    u32 pickLostItemReward(dItem::Item *item, int *price, dPrivateData_c *player); // 801260B0
    int pickSickReward(dItem::Item *item, dPrivateData_c *player); // 80126238
    BOOL sendSickThanksLetter(const dPersonalID_c *to, const dItem::Item *present, BOOL tryDirect); // 80126354
    BOOL sendSickReward(dItem::Item *item, dPrivateData_c *player, int arg); // 80126544
    BOOL isRequestMatch(const dItem::Item *item, int kind, u8 a, u8 b, const dItem::Item *questItem); // 80126698
    BOOL isRequestedItem(const dItem::Item *item); // 80126764
    BOOL updateQuests(dLostQuest_c *lost, dPrivateData_c *player, int arg2); // 8012689C
    BOOL sendTunekichiLetter(const dPersonalID_c *to, BOOL invite, BOOL flag); // 80126950
    BOOL sendBirthdayLetter(const dPersonalID_c *to, const dItem::Item *present, int hi); // 80126AD8
    BOOL sendValentineLetter(const dPersonalID_c *to, const dItem::Item *present, int hi); // 80126CAC
    BOOL sendNewYearLetter(const dPersonalID_c *to, int year); // 80126EAC
    BOOL sendMoveLetter(const dPersonalID_c *to); // 8012706C
    BOOL sendMoveLetters(); // 80127290
    wchar_t *getHabit(int language); // 80127330
    void setHabit(const wchar_t *habit, int language); // 801273D4
    u32 getSpecies(); // 8012808C
    u32 getBirthMonth(); // 80128094
    u32 getBirthDay(); // 801280A4
    dItem::Item getUmbrella(); // 801280AC
    void setUmbrella(const dItem::Item *item); // 801280B8
    u8 fn_8012813C(); // 8012813C
    u8 fn_801281AC(); // 801281AC
    BOOL isSleepTime(const dTime_c *now); // 8012820C
    BOOL canMoveOut(dAnimalBlock_c *block); // 80128270
    BOOL isLostItemRequestDone(dPrivateData_c *player, BOOL checkFlag); // 80128374
    BOOL fn_80128440(); // 80128440
    s8 getMaxFriendship(); // 801284D4
    int getDaysSinceLastTalk(const dTime_c *now); // 8012855C
    void validateItems(); // 8012865C
    void validateUmbrella(); // 80128698
    BOOL isFtrBoxed(u32 idx); // 80128728
    void setFtrBoxed(u32 idx); // 80128748
    void clearFtrBoxed(u32 idx); // 80128768
    void pickBoxedFtr(); // 80128788
    void pickBoxedFtrMoveOut(); // 801289B0
    int countBoxedFtr(); // 80128B28
    BOOL setHeldItem(const dItem::Item *item); // 80128BBC
    BOOL isHeldItemChangeMinute(u32 minute); // 80128C20
    BOOL isHeldItemChangeDue(u32 minute); // 80128C4C
    BOOL wantsParasol(); // 80128D1C
    dItem::Item getWishTool(); // 80128DF0
    BOOL recordImpression(const dPersonalID_c *pid); // 80128F34
    BOOL setVisitorLetter(dPrivateData_c *player); // 80129100
    BOOL isItemInMyBlock(const dItem::Item *item); // 80129238
    BOOL pickDesignToWear(); // 801295D0
    BOOL pickUmbrella(); // 801296C4


    /* 0x0000 */ u8 mSetupData[0x1820];     // checksummed area
    /* 0x1820 */ u32 mChecksum;        // fn_802A98FC over 0x0000..0x1820
    /* 0x1824 */ dAnimalTemplate_c mTemplate;
    /* 0x19BC */ u8 _19BC[4];
    /* 0x19C0 */ dDesign_c mClothDesign;
    /* 0x2240 */ dTimeStamp_c mPlaceChangeTime;
    /* 0x2248 */ s32 mVersion; // getVersion / initVersion
    /* 0x224C */ dAnmPersonalID_c mID;
    /* 0x230C */ dLandID_c mPrevLand;
    /* 0x2322 */ u8 _2322[2];
    /* 0x2324 */ dAnimalMemory_c mMemories[ANIMAL_MEMORY_NUM];
    /* 0x2BE4 */ dQuestVillagerWish_c mQuest;
    /* 0x2C66 */ dMail_c mLetter;
    /* 0x2FF6 */ dAnimalEventState_c mEvent;
    /* 0x2FFA */ dItem::Item mCloth;
    /* 0x2FFC */ dItem::Item mNewItems[4];
    /* 0x3004 */ dItem::Item mWall;
    /* 0x3006 */ dItem::Item mCarpet;
    /* 0x3008 */ dItem::Item mHeldItem;
    /* 0x300A */ u16 mBoxedFtrMask;
    /* 0x300C */ dSaveMelody_c mMelody; // the town tune, carried when moving
    /* 0x301C */ dAnimalSpot_c mSpot;
    /* 0x3030 */ u16 mIsMoving : 1; // isMoving
    /* 0x3030 */ u16 mIsMovingIn : 1;
    /* 0x3030 */ u16 mQuestStarted : 1;
    /* 0x3030 */ u16 _3030_lo : 13;
    /* 0x3032 */ u8 mJoinType; // 4 when cleared; set by init
    /* 0x3033 */ u8 mDayPlace; // 3 when cleared
    /* 0x3034 */ u8 mPlace; // 4 when cleared
    /* 0x3035 */ u8 mSetupLoaded; // set by setSetupData; gates the checksum check
    /* 0x3036 */ u8 mHeldItemChangeMinute; // a minute (0-59); 60 or more = unset (isHeldItemChangeMinute)
    /* 0x3037 */ u8 mHabitCooldown; // decHabitCooldown
    /* 0x3038 */ u8 _3038[8];
}; // size 0x3040

// Town-level members of dAnimalBlock_c owned by other TUs.
// dQuestSick_c and dLostQuest_c are in d_quest.hpp.

// {index, ?} at dAnimalBlock_c+0x1E452.
class dAnimalHomeStay_c {
public:
    dAnimalHomeStay_c(); // 80129854
    void clear(); // 80129858
    BOOL isValid(); // 8012986C: _0 < ANIMAL_NUM && _1 < 4
    void set(u32 idx, u8 looks); // 80129898
    BOOL isPeriod(const dTime_c *time); // 80129918
    BOOL isAnimal(int idx); // 801299E0


    /* 0x0 */ s8 mAnimalIdx; // -1 when cleared
    /* 0x1 */ u8 mPeriod;
};

// All villagers of a town plus town-level quests. 0x1E4A0.
class dAnimalBlock_c {
public:
    dAnimalBlock_c(); // 80129A3C
    void clear(); // 80129AE4
    dItem::Item getAnimalKey(const dAnmPersonalID_c *animal); // 80129D24
    dAnimal_c *getAnimalByKey(dItem::Item *key); // 80129DA8
    dAnimal_c *pickRandomAnimalNotMoving(const dAnmPersonalID_c **exclude, int num); // 8012A6E4: random villager not in exclude
    void initNewTown(void *a, u32 b, const dLandID_c *land); // 80129BC4
    u32 getAnimalNum(); // 80129D1C
    u32 getAnimalIdx(const dAnmPersonalID_c *animal); // 80129D7C
    dAnimal_c *getAnimal(int idx); // 80129D90
    dAnimal_c *getAnimalConst(int idx); // 80129D9C
    dAnimal_c *getAnimalByKeyConst(const dItem::Item *key); // 80129E24
    dAnimal_c *findAnimalByNpcIdx(u16 npcIdx); // 80129EA0
    int getFreeIdx(); // 80129EB4
    dAnimal_c *pickRandomAnimal(const dAnmPersonalID_c **exclude, u32 num, BOOL flag); // 8012A088
    dAnimal_c *pickRandomAnimalConst(const dAnmPersonalID_c **exclude, u32 num, BOOL flag); // 8012A0D4
    int getSickAnimalIdx(); // 8012A120
    BOOL isSickAnimal(const dAnmPersonalID_c *animal); // 8012A1B0
    int getStyleAnimalIdx(); // 8012A218
    BOOL isStyleAnimal(const dAnmPersonalID_c *id); // 8012A280
    int getAppointmentAnimalIdx(); // 8012A390
    BOOL isAppointmentAnimal(const dAnmPersonalID_c *id); // 8012A3FC
    BOOL isAppointmentSoon(int idx); // 8012A50C
    int getHiderIdx(u32 i); // 8012A668
    dAnimal_c *pickRandomAvailableAnimal(const dAnmPersonalID_c **exclude, u32 num); // 8012A818
    void initOutdoorQueue(); // 8012AA84
    void compactOutdoorQueue(); // 8012AB04
    BOOL addToOutdoorQueue(u32 idx); // 8012ABE8
    u32 getOutdoorNum(); // 8012AD80
    void decideOutdoorAnimals(BOOL flag); // 8012ADA4
    BOOL shouldDecideOutdoor(BOOL flag); // 8012B35C
    void updateOutdoorAnimals(BOOL flag); // 8012B620
    void syncHouses(); // 8012B688
    BOOL removeHouse(u32 idx); // 8012B73C
    BOOL getHouseBlockPos(int *x, int *z, int idx); // 8012B7B4
    BOOL getHouseBlockPosById(int *x, int *z, const dAnmPersonalID_c *animal); // 8012B86C
    BOOL getHousePos(mVec3_c *out, int idx); // 8012B8D0
    BOOL getHousePosById(mVec3_c *out, const dAnmPersonalID_c *id); // 8012B938
    BOOL getHouseFrontPos(mVec3_c *out, int idx); // 8012B98C
    BOOL isHomeStayTime(const dAnmPersonalID_c *id, dPrivateData_c *player); // 8012B9DC
    void updateAnimalPlaces(); // 8012BB4C
    void stampTalkCountTime(); // 8012BCBC
    void resetDailyTalkCounts(); // 8012BCC8
    void updateEvent(BOOL force); // 8012C0C4
    void updateQuestsDaily(int arg); // 8012C330
    int pickWishOfBestMatch(dAnimal_c *animal); // 8012C420
    int pickWishUnlikeWorstMatch(dAnimal_c *animal); // 8012C59C
    BOOL changeWish(dAnimal_c *animal); // 8012C818
    void updateWishes(BOOL arg); // 8012C948
    void expireLostItemQuests(); // 8012CA50
    void sendTunekichiInvites(); // 8012CAA4
    void updateDaily(int arg); // 8012CBAC
    int receiveLetter(dMail_c *mail); // 8012CCD0
    int getBusyQuestKind(const dAnmPersonalID_c *id, int which, BOOL current); // 8012CE0C
    void assignQuestCandidates(); // 8012CF3C
    void updateQuestCandidates(); // 8012D508
    int pickEvent16Animal(); // 8012D6E8
    void setEvent16Done(int idx); // 8012D80C
    int getLostItemAnimalIdx(); // 8012D864
    BOOL isLostItemAnimal(const dAnmPersonalID_c *id); // 8012D8F4
    BOOL tryStartLostItem(int arg); // 8012D95C
    int getLostItemLikeIdx(dPrivateData_c *player); // 8012DB6C
    void setLostItemState1(); // 8012DC70
    BOOL tryStartSick(int days, dTime_c *now); // 8012DCC4
    BOOL updateSickAnimal(); // 8012DF50
    void updateSick(int days); // 8012E1CC
    dPlayerID_c *pickStyleOtherPlayer(dAnimal_c *animal, dPrivateData_c *players, dPrivateData_c *self); // 8012E278
    BOOL canStartStyle(dAnimal_c *animal, dPrivateData_c *players, dPrivateData_c *self); // 8012E444
    BOOL startStyle(const dAnmPersonalID_c *animal, const dPlayerID_c *player0, const dPlayerID_c *player1, dTime_c *limit); // 8012E558
    dItem::Item pickStylePresent(dPrivateData_c *player); // 8012E618
    BOOL sendStyleLetter(const dPersonalID_c *pid0, const dPersonalID_c *pid1, dAnimal_c *animal, const wchar_t *topic, const dItem::Item *present, u32 which, BOOL flag); // 8012E87C
    dPrivateData_c *getStylePlayer(dPrivateData_c *players, u32 i); // 8012EC00
    BOOL sendStyleLetterTo(u32 which, BOOL flag); // 8012EC54
    void checkStylePlayers(); // 8012ED90
    BOOL updateStyleDaily(BOOL arg); // 8012EE30
    BOOL canStartAppointment(dPrivateData_c *player, dTime_c *time, int kind); // 8012EF00
    BOOL startAppointment(u8 kind, const dPlayerID_c *player, const dAnmPersonalID_c *animal, u8 hour, u8 min, dTime_c *limit); // 8012F010
    dPrivateData_c *getAppointmentPlayer(dPrivateData_c *players); // 8012F6EC
    BOOL sendAppointment1Letter(); // 8012F9AC
    BOOL sendAppointment0Letter(); // 8012FC8C
    void updateAppointmentFlags(); // 8012FD44
    BOOL updateAppointmentDaily(BOOL arg); // 8012FE44
    BOOL canAskHideAndSeek(u32 idx); // 80130160
    BOOL canAskHideAndSeekById(const dAnmPersonalID_c *id); // 80130204
    BOOL canHide(int idx, dPrivateData_c *player); // 80130248
    int pickHider(const int *exclude, u32 num); // 80130378
    BOOL canStartHideAndSeek(dPrivateData_c *player); // 8013048C
    int countHiders(const dAnmPersonalID_c *exclude); // 8013055C
    BOOL startHideAndSeek(const dPlayerID_c *player, const dAnmPersonalID_c *id, dTime_c *limit); // 801305E0
    int getHiderSlot(const dAnmPersonalID_c *id); // 801306D0
    int pickHideAndSeekPresent(dItem::Item *out, dAnimal_c *animal); // 80130778
    dPrivateData_c *getHideAndSeekPlayer(dPrivateData_c *players); // 801308DC
    BOOL sendHideAndSeekLetter(BOOL flag); // 80130AE4
    void checkHideAndSeek(); // 80130BA0
    BOOL updateHideAndSeekDaily(int delta); // 80130C3C
    BOOL isHideAndSeekRunning(); // 80130CE4
    BOOL isErrandSenderResident(dPrivateData_c *player); // 80130D7C
    BOOL hasErrandFromResident(dPrivateData_c *player); // 80130E18
    void setAppeared(u16 npcIdx); // 80130EEC
    void resetAppeared(); // 80130F1C
    BOOL shouldMoveIn(); // 80130FB4
    int moveInNewAnimal(); // 8013104C
    int moveInAnimal(const dAnimal_c *src); // 80131268
    void addMoveDays(int delta); // 8013135C
    void finishMovingIn(); // 80131380
    int getMovingOutIdx(); // 801313F4
    BOOL shouldPickMoveOut(); // 8013146C
    void pickMoveOutAnimal(); // 801314E0
    BOOL isMoveOutAnimal(const dAnmPersonalID_c *id); // 801315F4
    void clearMovedOut(); // 80131704
    int findMovedOut(u16 item); // 80131714
    void packMovedOut(); // 801317F8
    void removeMovedOut(u16 item); // 80131904
    void pushMovedOut(u16 item); // 80131968
    BOOL updateHeldItem(int idx); // 80131A10
    BOOL updateEvent15Flag(int idx); // 80132268
    int pickEvent11Animal(BOOL any, const dPersonalID_c *pid, const dLandID_c *land); // 801323A4
    int pickBirthdayHost(dPrivateData_c *player); // 80132614
    BOOL decideBirthdayHost(int playerNo); // 801327A4
    void decideBirthdayHosts(); // 80132890
    void clearBirthdayHost(int playerNo); // 801328E0
    void clearBirthdayHosts(); // 801329BC
    void sendBirthdayLetters(); // 80132A0C
    BOOL hasBirthdayHost(); // 80132C38
    void sendBirthdayHostPresent(); // 80132D44
    int getBirthdayHostIdx(int playerNo, BOOL checkEvent, BOOL checkFlag, BOOL checkPocket); // 80133044
    BOOL isBirthdayHost(const dAnmPersonalID_c *id, int playerNo, BOOL checkEvent, BOOL checkFlag, BOOL checkPocket); // 80133158
    void sendValentineLetter(dPrivateData_c *player); // 801331C0
    void sendValentineLetters(); // 80133418
    void sendNewYearLetters(dPrivateData_c *player); // 801334A8
    void sendNewYearLettersCurrent(); // 801335CC
    void clearSpots(); // 8013365C
    void fn_801336B0(); // 801336B0
    void fn_80133704(); // 80133704
    void clearMemoryFlag200(); // 80133758
    void updateTalkFlags(dPrivateData_c *player); // 801337AC
    void halveTalkFlags(int skip); // 80133840
    int countAnimalsNearItem(const dItem::Item *item, int group); // 801338D4
    int countByOutdoorState(int value); // 80133988
    void updateClothes(BOOL flag); // 80133A20


    /* 0x00000 */ dAnimal_c mAnimals[ANIMAL_NUM];
    /* 0x1E280 */ u8 _1E280[4];
    /* 0x1E284 */ dQuestSick_c mSick;      // clear()
    /* 0x1E2CA */ dLostQuest_c mLostItem;     // clear()
    /* 0x1E2D4 */ dQuestPlayerItem_c mAppointment;     // clear()
    /* 0x1E2FE */ dQuestPlayerAnimal_c mHideAndSeek; // clear()
    /* 0x1E3EA */ dQuestPlayerPair_c mStyle;     // clear()
    /* 0x1E44A */ dTimeStamp_c mTalkCountTime;
    /* 0x1E452 */ dAnimalHomeStay_c mHomeStay;
    /* 0x1E454 */ u16 mMovedOutNpcIdx[ANIMAL_NUM]; // npc indices of recently moved-out villagers (can't move back), 0xFFFF = empty
    /* 0x1E468 */ s8 mOutdoorQueue[ANIMAL_NUM]; // filled with -1 by clearIdxList
    /* 0x1E472 */ u8 mEventId;             // dQuestEvent_e; EVENT_NUM when cleared
    /* 0x1E473 */ u8 mAppearedFlags[0x1B];       // npc index bitset (0xD2 bits); passed to pickTemplate
    /* 0x1E48E */ u8 mMoveDays;             // day counter capped at 0xFF (addMoveDays, updateMoves)
    /* 0x1E48F */ s8 mMoveOutIdx;             // -1 when cleared
    /* 0x1E490 */ s8 mMoveInIdx;             // -1 when cleared
    /* 0x1E491 */ u8 _1E491[0xF];
}; // size 0x1E4A0

// Second villager array in the save, after dAnimalBlock_c. 0x1E280.
class dMovedAnimalList_c {
public:
    dMovedAnimalList_c(); // 80133AC4
    void clear(); // 80133B0C
    dItem::Item getAnimalKey(const dAnmPersonalID_c *id); // 80133B68
    int getAnimalIdx(const dAnmPersonalID_c *id); // 80133BC4
    dAnimal_c *getAnimalByKey(const dItem::Item *key); // 80133BD8
    dAnimal_c *getAnimal(int idx); // 80133C14
    dAnimal_c *getAnimalConst(int idx); // 80133C20
    u32 getAnimalNum(); // 80133C2C
    int pickMoveInIdx(dAnimalBlock_c *block, const dLandID_c *land); // 80133C34
    int tryPickMoveInIdx(dAnimalBlock_c *block, const dLandID_c *land); // 80133E98
    u32 getReplaceIdx(dAnimal_c *animal); // 80133F60
    u32 addAnimal(dAnimal_c *animal, const dLandID_c *land, const dSaveMelody_c *arg); // 801342E0
    u32 pickGiveAwayIdx(const dLandID_c *land); // 8013443C


    /* 0x00000 */ dAnimal_c mAnimals[ANIMAL_NUM];
}; // size 0x1E280

// The villager part of the save file (dSaveData_c+0x21B20). 0x3C740.
class dAnimalSave_c {
public:
    dAnimalSave_c(); // 801345D8
    dItem::Item getAnimalKey(const dAnmPersonalID_c *animal); // 80134704: key of a villager
    dAnimal_c *getAnimalByKey(const dItem::Item *key); // 801347FC: villager by key
    void updateChecksum(); // 80134614
    BOOL isChecksumValid(); // 80134648
    u32 calcChecksum() const; // 80134690
    void clear(); // 801346B8
    BOOL updateMoves(int days); // 801348A0
    BOOL addMovedAnimal(dAnimal_c *animal); // 80134B28
    dAnimal_c *pickMovedAnimal(); // 80134BC8
    BOOL takeMovedAnimal(dAnimal_c *out, BOOL remove); // 80134C40


    /* 0x00000 */ dAnimalBlock_c mTown;
    /* 0x1E4A0 */ dMovedAnimalList_c mMoved;
    /* 0x3C720 */ u32 mChecksum; // CRC of the rest (calcChecksum)
    /* 0x3C724 */ u8 _3C724[0x1C];
}; // size 0x3C740

// Random set bit of mask (count bits set, num bits wide), or -1. 80134CD4
u32 pickRandomBit(u32 mask, int count, u32 num);
