#pragma once

#include <types.h>
#include <game/game/d_date.hpp>
#include <game/game/d_save_mother_mail.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_friend.hpp>
#include <game/game/d_animal_id.hpp>
#include <game/game/d_dsn.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_catalog.hpp>

#define PLAYER_POCKETS_COUNT 15
#define PLAYER_MAIL_COUNT 10
#define PLAYER_ORG_DESIGN_COUNT 8
#define PLAYER_OUTFIT_COUNT 8
#define PLAYER_NUM 4

#define PRIVATE_FLAGS0_NUM 160
#define PRIVATE_FLAGS1_NUM 128
#define PRIVATE_FLAGS2_NUM 128
#define PRIVATE_FLAGS3_NUM 128
#define PRIVATE_SAVINGS_MAX 999999999
#define PRIVATE_BELLS_MAX 99999
#define PRIVATE_NOOK_POINTS_MAX 50000

// Unrecovered member objects. Their constructors and destructors live in other TUs.
class dUnk5560_c { // 0x6E
public:
    dUnk5560_c(); // 80150A5C
    ~dUnk5560_c(); // 80150A60

    u8 _00[0x6E];
};


// 0x0E at dPrivateData_c+0x1124. Play dates (dYMD_c).
struct dPrivateDates_c {
    void init(); // 80139A14
    BOOL fn_80139A6C(); // 80139A6C; uses mDate0
    BOOL fn_80139C30(dYMD_c *out); // 80139C30; uses mDate1

    /* 0x00 */ dYMD_c mDate0;
    /* 0x04 */ dYMD_c mDate1;
    /* 0x08 */ u32 _08;
    /* 0x0C */ u8 _0C;
    /* 0x0D */ u8 _0D;
};

// 8 bytes at +0x4400 of dDesignList_c: display order of the designs.
class dDesignOrder_c {
public:
    dDesignOrder_c(); // 8013A418
    ~dDesignOrder_c(); // 8013A41C

    void init(); // 8013A45C
    void swap(u32 a, u32 b); // 8013A4A0
    u8 get(u32 i); // 8013A4BC

    u8 mOrder[PLAYER_ORG_DESIGN_COUNT];
};

// 0x4420 at dPrivateData_c+0x1140.
class dDesignList_c {
public:
    void init(dPersonalID_c *creator); // 8013A4CC
    dDesign_c *getDesign(u32 i); // 8013A5F4
    void setDesign(u32 i, dDesign_c *design); // 8013A630
    u8 getOrder(u32 i); // 8013A880
    u16 fn_8013A888(u32 i, int kind); // 8013A888

    /* 0x0000 */ dDesign_c mDesigns[PLAYER_ORG_DESIGN_COUNT];
    /* 0x4400 */ dDesignOrder_c mOrder;
};

// 10 bytes (80 bits) at dPrivateData_c+0x85FA, indexed by seeker_c::findLike.
class dPrivateBits85FA_c {
public:
    void clear(); // 8013A2B8
    BOOL isSet(const dItem::Item &item); // 8013A2E8
    void set(const dItem::Item &item); // 8013A330
    void reset(const dItem::Item &item); // 8013A378
    BOOL isSetIdx(u32 idx); // 8013A3C0
    void setIdx(u32 idx); // 8013A3D8
    void resetIdx(u32 idx); // 8013A3F8

    u8 mBits[10];
};

// 16 bytes (128 bits) at dPrivateData_c+0x8604, indexed by seeker_c::findLike.
class dPrivateBits8604_c {
public:
    void clear(); // 8013C53C
    BOOL isSet(const dItem::Item &item); // 8013C584
    void set(const dItem::Item &item); // 8013C5CC
    void reset(const dItem::Item &item); // 8013C614
    BOOL isSetIdx(u32 idx) const; // 8013C65C
    void setIdx(u32 idx); // 8013C674
    void resetIdx(u32 idx); // 8013C694
    int count() const; // 8013C6B4

    u8 mBits[16];
};

class dEquip_c {
public:
    dEquip_c(); // 8013A940
    ~dEquip_c(); // 8013A988

    BOOL fn_8013A9C8(); // 8013A9C8
    void setFromPlayer(); // 8013AA28
    void clear(); // 8013AA90

    dItem::Item mHeld;
    dItem::Item mShirt;
    dItem::Item mHat;
    dItem::Item mAcc;
};

// 4 bytes at dPrivateData_c+0x8614, 0xFF when empty.
class dPrivateSlots_c {
public:
    BOOL isValid(int i); // 8013AAAC
    u8 get(int i); // 8013AACC
    void set(int i, u8 value); // 8013AAD4
    void clear(); // 8013AADC
    int findEmpty(); // 8013AAF4

    u8 mSlots[4];
};

// 8 bytes at dPrivateData_c+0x83ED.
class dUnk83ED_c {
public:
    dUnk83ED_c(); // 8013AB54
    ~dUnk83ED_c(); // 8013AB58

    u8 _00[8];
};

// 0xC; 8 of these at dPrivateData_c+0x7F40.
class dOutfit_c {
public:
    dOutfit_c(); // 8013AB98
    ~dOutfit_c(); // 8013ABD0

    void clear(); // 8013AC28
    void setDefault(); // 8013AC78
    void setFromPlayer(); // 8013ACE8
    void copy(const dOutfit_c *other); // 8013ADA4
    BOOL isSame(const dOutfit_c *other) const; // 8013ADAC
    BOOL hasShoes() const {
        if (isValid()) {
            if (mFlags.noShoes) {
                return FALSE;
            }
            return TRUE;
        }
        return TRUE;
    }

    BOOL isValid() const {
        return mFlags.valid;
    }

    BOOL isMale() const {
        return mFlags.male;
    }

    BOOL noShoes() const {
        return mFlags.noShoes;
    }

    /* 0x0 */ dEquip_c mEquip;
    /* 0x8 */ u8 mHairColor;
    /* 0x9 */ u8 mShoeColor;
    /* 0xA */ u8 mHair;

    union {
        struct {
            u8 unused : 5;
            u8 noShoes : 1;
            u8 male : 1;
            u8 valid : 1;
        } mFlags;
        u8 mFlagByte;
    };
    // /* 0xB */ u8 _B7 : 5;
    // u8 mNoShoes : 1; // set when 0x83F7 is 0
    // u8 mMale : 1;
    // u8 mValid : 1;
};

// 0x24 at dPrivateData_c+0x55CE: a ring of 8 item pairs.
class dItemPairRing_c {
public:
    void clear(); // 8013C054
    void fn_8013C0C0(); // 8013C0C0
    void push(const dItem::Item *a, const dItem::Item *b); // 8013C0D0
    void fn_8013C1A0(); // 8013C1A0
    BOOL contains(const dItem::Item *a, const dItem::Item *b); // 8013C1C0
    void fn_8013C25C(); // 8013C25C
    void fn_8013C27C(); // 8013C27C

    /* 0x00 */ dItem::Item mA[8];
    /* 0x10 */ dItem::Item mB[8];
    /* 0x20 */ s8 mIndex;
    /* 0x21 */ u8 mCount : 2;
    u8 _21_5 : 1;
    u8 _21_4 : 1;
    u8 _21_3 : 1;
    u8 _21_2 : 1;
    u8 _21_1 : 1;
    u8 _21_0 : 1;
    /* 0x22 */ u8 _22_7 : 1;
    u8 _22_6 : 1;
    u8 _22_5 : 1;
    u8 _22_4 : 1;
    u8 _22_0 : 4;
};

// 0xC at dPrivateData_c+0x55FC.
class dUnk55FC_c {
public:
    dUnk55FC_c(); // 8013C3A0
    ~dUnk55FC_c(); // 8013C3A4

    void clear(); // 8013C3E4

    s32 _00;
    s32 _04;
    s8 _08;
};

// 0xC2 at dPrivateData_c+0x5608.
class dAnimalItem_c {
public:
    dAnimalItem_c(); // 8013C3F8
    ~dAnimalItem_c(); // 8013C408

    void clear(); // 8013C448
    BOOL isValid(); // 8013C480
    void set(const dAnmPersonalID_c *animal, const dItem::Item *item); // 8013C4AC

    dAnmPersonalID_c mAnimal;
    dItem::Item mItem;
};

// 4 bytes at dPrivateData_c+0x8634 and 0x8638, -1 when cleared.
class dUnk8634_c {
public:
    dUnk8634_c(); // 8013C4EC
    ~dUnk8634_c(); // 8013C4F0

    void clear(); // 8013C530

    s32 mValue;
};

// 0x2E at dPrivateData_c+0x7FA8 (also copied to/from town data +0x734F2).
struct dPrivateHost_c {
    void decrease(int n); // 801363E0

    int getCount(void) const { return mCount; }
    void setCount(int count) {
        mCount = count;
    }

    dPersonalID_c mPID;
    u8 mCount : 4;
    u8 mFlagA : 1;
    u8 mFlagB : 1;
    u8 mFlagC : 1;
    u8 mFlagD : 1;
};

// 0x886: a design plus state bytes (owner not recovered; its methods are in this TU).
class dUnkDesignBoard_c {
public:
    void clear(); // 80136638
    void decrease(int n); // 8013675C
    void fn_8013677C(); // 8013677C
    void fn_80136868(); // 80136868
    BOOL fn_801368F4(); // 801368F4

    u8 get885(void) const { return _885; }

    /* 0x000 */ dDesign_c mDesign;
    /* 0x880 */ u8 _880;
    /* 0x881 */ u8 _881;
    /* 0x882 */ u8 _882;
    /* 0x883 */ u8 _883;
    /* 0x884 */ u8 _884;
    /* 0x885 */ u8 _885;
};

struct dBirthday_c {
    dBirthday_c() : mMonth(0), mDay(1) {}
    BOOL isSame(u8 month, u8 day) const { return mMonth == month && mDay == day; }

    u8 mMonth;
    u8 mDay;
};

class dPrivateData_c {
public:
    // Static helpers on the current player (fn_80101770).
    static BOOL fn_80136244();                                    // 80136244
    static BOOL fn_80136310();                                    // 80136310
    static void saveHostToTown();                                 // 80136460
    static void loadHostFromTown();                               // 8013654C
    static BOOL fn_80136694();                                    // 80136694

    dPrivateData_c();                                             // 80136914
    ~dPrivateData_c();                                            // 80136B08
    void copy(const dPrivateData_c *other);                       // 80136C7C; memcpy 0x86C0

    // Checksum over 0x1124..0x86C0 (fn_802A98FC, seed -1), stored in mChecksum.
    void updateChecksum();                                        // 80136C88; updates mFriends first
    BOOL isChecksumValid(int arg);                                // 80136CC0; checks mFriends first
    u32 calcChecksum() const;                                           // 80136D1C
    static void updateChecksumAll(dPrivateData_c *players);       // 80136D40; 4 players, stride 0x86C0
    static BOOL isChecksumValidAll(dPrivateData_c *players, int arg); // 80136D90

    // Run over all 4 save players with a valid mPID.
    static BOOL fn_80136E10(dPrivateData_c *players);             // 80136E10
    static void setFlag0All(dPrivateData_c *players, u32 flag);   // 80136E90
    static void clearFlag0All(dPrivateData_c *players, u32 flag); // 80136F00
    static BOOL isFlag0Any(dPrivateData_c *players, u32 flag);    // 80136F70

    void setup(const wchar_t *name, u16 id, u8 gender);           // 80137000
    int findInSave() const;                                             // 80137110

    // Daily update, and the letters it generates.
    void fn_801371DC();                                           // 801371DC
    void fn_8013760C();                                           // 8013760C
    static BOOL sendLetter(u16 kind, const dItem::Item *present, dPrivateData_c *player, int amount); // 80137788
    void fn_80137898(int days);                                   // 80137898
    void updateLooks(int days);                                   // 80137BCC
    void updateTan(int days);                                     // 80137C10
    void fn_80137CD0(int days);                                   // 80137CD0
    void updateHair(int days);                                    // 80137D34
    void dailyUpdate(int days);                                   // 80137DA4

    // Items.
    void setPocket(const dItem::Item *item, int idx, BOOL flag);  // 80137F58
    // Empties a pocket (d_a_npc_nml talk_c 800315D4 / 800319F4: the inlined local takes the stack
    // slot below the caller's own Item locals).
    void clearPocket(int idx) {
        dItem::Item none;
        setPocket(&none, idx, FALSE);
    }
    int findEmptyPocket(int start);                               // 801380EC
    BOOL pickUp(const dItem::Item *item, BOOL flag);              // 8013812C
    int countPockets(BOOL (*fn)(const dItem::Item *), u16 *mask); // 801381B4
    int countPocketsFlag(BOOL (*fn)(const dItem::Item *, int), u16 *mask); // 8013828C
    void setPocketFlag(int idx, int flag);                         // 8013834C
    void fn_8013835C(void *arg, int idx);                         // 8013835C
    void clearLetter(int idx);                                    // 8013836C
    int findLetter();                                             // 8013837C
    int getPocketFlag(int idx) const;                                    // 801383DC
    void clear();                                                 // 801383EC

    // Flag bits.
    BOOL isFlag0(u32 flag) const;                                       // 80138580
    void setFlag0(u32 flag);                                      // 801385C0
    void clearFlag0(u32 flag);                                    // 801385F0
    BOOL isFlag1(u32 flag);                                       // 80138620
    void setFlag1(u32 flag);                                      // 80138660
    void clearFlag1(u32 flag);                                    // 80138690
    void setFlag2(u32 flag);                                      // 801386C0
    BOOL isFlag3(u32 flag);                                       // 801386F0
    void setFlag3(u32 flag);                                      // 80138730

    BOOL fn_80138760();                                           // 80138760
    int fn_80138890() const;                                            // 80138890

    // Money.
    void setSavings(int amount);                                  // 801389A4
    void addSavings(int amount);                                  // 801389C4
    void subSavings(int amount);                                  // 801389EC
    void payDebt(int amount);                                     // 80138A0C
    void setDebtFromHouse();                                      // 80138A38
    int getPocketMoney() const;                                         // 80138AC8
    int getMoneyRoom(int slots);                                  // 80138B58
    int findMoneyPocketMostRoom();                                // 80138C24
    int findMoneyPocketSmallest();                                // 80138CE0
    BOOL addMoney(int amount);                                    // 80138D84
    BOOL payMoney(int amount, BOOL allowItems);                   // 80138E78

    u8 get_86A5();                                                // 80138FFC
    void set_86A5(u8 value);                                      // 80139018
    void fn_8013902C();                                           // 8013902C

    void addNookPoints(int points);                               // 80139090
    void subNookPoints(int points);                               // 801390E8
    void addNookPointsForShop();                                  // 801390F8

    void fn_801391A0();                                           // 801391A0
    BOOL fn_801391E0();                                           // 801391E0

    void inc_8694();                                              // 801392B8
    void set_8694(u8 value);                                      // 801392F4
    void fn_80139314();                                           // 80139314
    u8 get_8694();                                                // 80139364
    u8 get_8695();                                                // 80139370
    BOOL fn_8013937C();                                           // 8013937C
    int get_8693();                                                // 80139384
    void inc_8693();                                              // 80139390
    BOOL fn_801393AC();                                           // 801393AC

    // Per-player blocks in other save data, found by mPID.
    void *fn_80139408();                                          // 80139408
    void *fn_80139468();                                          // 80139468
    void *fn_801394D0();                                          // 801394D0
    void *fn_80139530();                                          // 80139530

    // Letters.
    dMail_c *fn_80139594();                                       // 80139594
    dMail_c *fn_8013967C();                                       // 8013967C
    BOOL fn_801396F4();                                           // 801396F4

    void inc_8696();                                              // 8013974C

    // Errand.
    dQuestErrand_c *findErrand(dAnmPersonalID_c *animal, int which, u32 idx); // 80139770
    static BOOL isPocketErrandItem(const dItem::Item *item, int flag); // 80139914
    int countErrandPockets(u16 *mask);                            // 80139938
    BOOL fn_80139948(int arg);                                    // 80139948

    // Static helpers over the 4 save players.
    static void clearAll(dPrivateData_c *players);                // 8013AEC0
    static void clearPlayer(dPrivateData_c *players, int idx);    // 8013AF0C
    static int find(dPrivateData_c *players, const dPersonalID_c *pid); // 8013AFB8
    static int findByPlayerID(dPrivateData_c *players, const dPlayerID_c *id); // 8013B080
    static int findEmpty(dPrivateData_c *players);                // 8013B104
    static int count(dPrivateData_c *players);                    // 8013B174
    static dPrivateData_c *get(dPrivateData_c *players, const dPersonalID_c *pid); // 8013B1E0
    static dPrivateData_c *getChecked(dPrivateData_c *players, int idx); // 8013B218
    static dPrivateData_c *getRaw(dPrivateData_c *players, int idx); // 8013B28C
    static int getNthValid(dPrivateData_c *players, u32 n);       // 8013B2C8
    static BOOL contains(dPrivateData_c *players, const dPersonalID_c *pid); // 8013B344
    static dQuestErrand_c *findErrandAll(dPrivateData_c *players, dAnmPersonalID_c *animal, int which, u32 idx); // 8013B3D4
    static BOOL fn_8013B474(dPrivateData_c *players, int arg);    // 8013B474
    static int fn_8013B4F8();                                     // 8013B4F8
    static int fn_8013B5CC();                                     // 8013B5CC
    static void fn_8013B7A4();                                    // 8013B7A4
    static void fn_8013B848();                                    // 8013B848
    static void fn_8013B9F8();                                    // 8013B9F8
    static void fn_8013BA50();                                    // 8013BA50
    static void fn_8013BBCC();                                    // 8013BBCC
    static void fn_8013BC38(int days);                            // 8013BC38
    BOOL fn_8013BDA0(int chance, int count, int flag);            // 8013BDA0

    u8 get_869D(int i);                                           // 8013C2A0
    void inc_869D(int i);                                         // 8013C2B0
    BOOL isBirthday(const dTime_c& cal) const;                          // 8013C2D0

    BOOL fn_8013C71C() const;                                           // 8013C71C
    u8 get_86A3();                                                // 8013C7C0
    BOOL fn_8013C7CC() const;                                           // 8013C7CC
    void fn_8013C878();                                           // 8013C878
    dOutfit_c *getOutfit(int i);                                  // 8013CAD8


    BOOL isChecksumOK() const {
        u32 checksum = calcChecksum();
        if (checksum == mChecksum) {
            return TRUE;
        }

        return FALSE;
    }

    /* 0x0000 */ dFriendList_c mFriends;
    /* 0x1120 */ u32 mChecksum;
    /* 0x1124 */ dPrivateDates_c mDates;
    /* 0x1134 */ s32 mBells;
    /* 0x1138 */ s32 mDebt;
    /* 0x113C */ s32 mSavings;
    /* 0x1140 */ dDesignList_c mOrgDesigns;
    /* 0x5560 */ dUnk5560_c _5560;
    /* 0x55CE */ dItemPairRing_c _55CE;
    /* 0x55F4 */ u32 _55F4;
    /* 0x55F8 */ u32 _55F8;
    /* 0x55FC */ dUnk55FC_c mBirthdayHost;
    /* 0x5608 */ dAnimalItem_c mVisitorLetter;
    /* 0x56CA */ dMail_c mLetters[PLAYER_MAIL_COUNT];
    /* 0x7A6A */ dMail_c mFutureSelfLetter;
    /* 0x7DFA */ dLetterStyle_c mLetterStyle;
    /* 0x7EC2 */ dPersonalID_c mPID;
    /* 0x7EEE */ dPersonalID_c _7EEE;
    /* 0x7F1A */ dEquip_c mEquipment;
    /* 0x7F22 */ dItem::Item mPockets[PLAYER_POCKETS_COUNT];
    /* 0x7F40 */ dOutfit_c mOutfits[PLAYER_OUTFIT_COUNT];
    /* 0x7FA0 */ u16 mNookPoints; // current points
    /* 0x7FA2 */ u16 mNookPointsLifetime; // total lifetime points
    /* 0x7FA4 */ u16 mNookPointsMax; // max balance reached
    /* 0x7FA6 */ u16 _7FA6;
    /* 0x7FA8 */ dPrivateHost_c mHost;
    /* 0x7FD6 */ dSaveMotherMail_c mMotherMail;
    /* 0x7FEE */ dQuestErrandList_c mErrand;
    /* 0x83BE */ dYMD_c _83BE;
    /* 0x83C2 */ dYMD_c _83C2;
    /* 0x83C6 */ dYMD_c _83C6;
    /* 0x83CA */ dItem::Item _83CA;
    /* 0x83CC */ u16 _83CC[13]; // Bunny Day egg counts (12 kinds + the fake egg)
    /* 0x83E6 */ dBirthday_c mBirthday;
    /* 0x83E8 */ u8 _83E8;
    /* 0x83E9 */ u8 _83E9;
    /* 0x83EA */ u8 mFace;
    /* 0x83EB */ u8 mHair;
    /* 0x83EC */ u8 mHairColor;
    /* 0x83ED */ dUnk83ED_c _83ED;
    /* 0x83F5 */ u8 _83F5;
    /* 0x83F6 */ u8 mShoeColor;
    /* 0x83F7 */ u8 _83F7;
    /* 0x83F8 */ u8 mTan;
    /* 0x83F9 */ u8 _83F9;
    /* 0x83FA */ dCatalog_c mCatalog;
    /* 0x85FA */ dPrivateBits85FA_c _85FA;
    /* 0x8604 */ dPrivateBits8604_c _8604;
    /* 0x8614 */ dPrivateSlots_c _8614;
    /* 0x8618 */ dTimeStamp_c _8618;
    /* 0x8620 */ dTimeStamp_c _8620;
    /* 0x8628 */ s32 _8628;
    /* 0x862C */ s32 _862C;
    /* 0x8630 */ s32 _8630;
    /* 0x8634 */ dUnk8634_c mValentineYear;
    /* 0x8638 */ dUnk8634_c mNewYearYear;
    /* 0x863C */ u8 mPocketFlags[PLAYER_POCKETS_COUNT]; // one per pocket; getPocketMoney skips nonzero
    /* 0x864B */ u8 mFlags0[PRIVATE_FLAGS0_NUM / 8]; // bit 41 = reset flag
    /* 0x865F */ u8 mFlags1[PRIVATE_FLAGS1_NUM / 8];
    /* 0x866F */ u8 mFlags2[PRIVATE_FLAGS2_NUM / 8];
    /* 0x867F */ u8 mFlags3[PRIVATE_FLAGS3_NUM / 8];
    /* 0x868F */ u8 _868F;
    /* 0x8690 */ u8 _8690;
    /* 0x8691 */ u8 _8691;
    /* 0x8692 */ u8 _8692;
    /* 0x8693 */ u8 _8693;
    /* 0x8694 */ u8 _8694;
    /* 0x8695 */ u8 _8695;
    /* 0x8696 */ u8 _8696;
    /* 0x8697 */ u8 _8697;
    /* 0x8698 */ u8 _8698;
    /* 0x8699 */ u8 _8699;
    /* 0x869A */ s8 _869A;
    /* 0x869B */ s8 _869B;
    /* 0x869C */ u8 _869C;
    /* 0x869D */ u8 _869D[5];
    /* 0x86A2 */ u8 _86A2;
    /* 0x86A3 */ u8 _86A3;
    /* 0x86A4 */ u8 _86A4;
    /* 0x86A5 */ u8 _86A5;
    /* 0x86A6 */ u8 _86A6[0x1A];
}; // size 0x86C0
