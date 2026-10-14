#pragma once

#include <types.h>
#include <game/game/d_date.hpp>
#include <game/game/d_save_mother_mail.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_sv_star_sign.hpp>
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

// Bit indices of the per-player flag tables (dPrivateData_c::isFlag0 / setFlag0 / clearFlag0 on mFlags0, and
// the Flag1 functions on mFlags1). Shared by many systems; most indices are still literals, named here as
// their meanings are found.
enum dPrivateFlag0_e {
    PRIVATE_FLAG0_BEE_STUNG = 0x02,             // stung by bees (state 0x62 after dDemo::request9): villagers react
                                                // (d_npc_talk_reaction, d_msg, d_animal); cleared by the
                                                // APPEARANCE_CLEAR_BEE_STING appearance change and on moving out
    PRIVATE_FLAG0_JOAN_MET = 0x03,              // dNpcInfo::kaburiba_c
    PRIVATE_FLAG0_PART_TIME_JOB = 0x0D,         // Nook's part-time job is under way: set for a new player
                                                // (dSvProc_c::initializePlayer); villagers talk about the
                                                // newcomer (d_npc_talk_arbeit), no event announcements,
                                                // villager memories / letters, quests (inferred)
    PRIVATE_FLAG0_HRA_RATED = 0x1C,             // the house takes part in the HRA's weekly ratings and model room choice (inferred)
    PRIVATE_FLAG0_HRA_NO_LETTER = 0x1D,         // no weekly HRA rating letter (inferred)
    PRIVATE_FLAG0_HRA_HAS_RESULT = 0x1E,        // mHRAInfo holds the last rating (score, remark, series)
    PRIVATE_FLAG0_HRA_JOINED = 0x27,            // the HRA's welcome letter was delivered
    PRIVATE_FLAG0_HRA_HOUSE_MODEL = 0x2A,       // the HRA's house model present was sent (70000 points)
    PRIVATE_FLAG0_HRA_WIDE_HOUSE_MODEL = 0x2B,  // the HRA's wide house model present was sent (100000 points)
    PRIVATE_FLAG0_HRA_TWO_STORY_MODEL = 0x2C,   // the HRA's two-story model present was sent (150000 points)
    PRIVATE_FLAG0_FISHING_ENTERED = 0x2D,       // the player entered this fishing tourney (d_ev_fishing)
    PRIVATE_FLAG0_BUG_OFF_ENTERED = 0x6E,       // the player entered this Bug-Off (cleared once the letter is sent)
    PRIVATE_FLAG0_EVENT_LETTER_HELD = 0x72,     // a Bug-Off / fishing tourney result letter waits in mFutureSelfLetter
};
enum dPrivateFlag1_e {
    PRIVATE_FLAG1_FISHING_JOINED = 0x11, // 0x11..0x13: dNpcInfo::uomasa_c
    PRIVATE_FLAG1_FISHING_TALKED_BEFORE_START = 0x12,
    PRIVATE_FLAG1_FISHING_RESULT_HEARD = 0x13,
    PRIVATE_FLAG1_BUG_OFF_JOINED = 0x35, // 0x35..0x38: dNpcInfo::bug_c
    PRIVATE_FLAG1_BUG_OFF_TALKED_BEFORE_START = 0x36,
    PRIVATE_FLAG1_BUG_OFF_RESULT_HEARD = 0x37,
    PRIVATE_FLAG1_BUG_OFF_JUDGED = 0x38,
};
#define PRIVATE_SAVINGS_MAX 999999999
#define PRIVATE_BELLS_MAX 99999
#define PRIVATE_NOOK_POINTS_MAX 50000

// The Happy Room Academy's per-player state (dPrivateData_c::mHRAInfo, 0xE at +0x1124).
struct dPrivateHRAInfo_c {
    void init(); // 80139A14
    BOOL isNewRatingWeek(dYMD_c *out); // 80139A6C: this game week differs from mRatingDate's (out is unused; the HRA passes NULL)
    BOOL isNewModelRoomMonth(dYMD_c *out); // 80139C30: this month's model room date (*out) differs from mModelRoomDate

    /* 0x00 */ dYMD_c mRatingDate;    // the last HRA rating (game day)
    /* 0x04 */ dYMD_c mModelRoomDate; // the last model room month handled
    /* 0x08 */ u32 mLastScore;        // the last rating's points
    /* 0x0C */ u8 mLastRemark;         // the last rating's remark (message - 12)
    /* 0x0D */ u8 mLastSeries;         // the last rating's series
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
    u16 getDesignItem(u32 i, int kind); // 8013A888

    /* 0x0000 */ dDesign_c mDesigns[PLAYER_ORG_DESIGN_COUNT];
    /* 0x4400 */ dDesignOrder_c mOrder;
};

// The K.K. songs the player has (dPrivateData_c::mSongs, 80 bits indexed by seeker_c::findLike; d_menu_music,
// d_menu_item02).
class dPrivateSongs_c {
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

// The gyroids the player has collected (dPrivateData_c::mGyroids, 128 bits indexed by seeker_c::findLike;
// d_menu_haniwa, d_a_npc_sp_cafe).
class dPrivateGyroids_c {
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
    enum Part {
        PART_SHIRT = 0,
        PART_HAT = 1,
        PART_ACCESSORY = 2,
        PART_HELD = 3,
    };

    dEquip_c(); // 8013A940
    ~dEquip_c(); // 8013A988

    BOOL isAccessoryVisible(); // 8013A9C8
    void setFromPlayer(); // 8013AA28
    void clear(); // 8013AA90

    dItem::Item mHeld;
    dItem::Item mShirt;
    dItem::Item mHat;
    dItem::Item mAcc;
};

// The player's 4 emotion slots (dPrivateData_c::mEmotions), 0xFF when empty: learnt from Dr. Shrunk
// (d_a_npc_sp_sisyo), used in chat (d_menu_chat, d_obj_keyboard, d_a_c_uitag).
class dPrivateEmotions_c {
public:
    BOOL isValid(int i); // 8013AAAC
    u8 get(int i); // 8013AACC
    void set(int i, u8 value); // 8013AAD4
    void clear(); // 8013AADC
    int findEmpty(); // 8013AAF4

    u8 mSlots[4];
};

// The player's Mii (an RFL create ID, 8 bytes) at dPrivateData_c+0x83ED (d_player_mgr fills it from the Mii data).
class dMiiCreateID_c {
public:
    dMiiCreateID_c(); // 8013AB54
    ~dMiiCreateID_c(); // 8013AB58

    u8 mData[8];
};

// A saved outfit (0xC); 8 of these at dPrivateData_c+0x7F40 (mOutfits).
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
            u8 noShoes : 1; // set from the player when dPrivateData_c::mShoeKind is 0
            u8 male : 1;
            u8 valid : 1;
        } mFlags;
        u8 mFlagByte;
    };
};

// 0x24 at dPrivateData_c+0x55CE: Jingle's Toy Day state (dNpcInfo::jingle_c), the last 8 looks (hat and
// accessory) he gave a present to.
class dItemPairRing_c {
public:
    void clear();                                    // 8013C054
    void meet();                                     // 8013C0C0
    void push(dItem::Item hat, dItem::Item acc);     // 8013C0D0
    void incCount();                                 // 8013C1A0
    BOOL contains(dItem::Item hat, dItem::Item acc); // 8013C1C0
    void move();                                     // 8013C25C
    void resetEncounter();                           // 8013C27C

    /* 0x00 */ dItem::Item mHats[8];
    /* 0x10 */ dItem::Item mAccs[8];
    /* 0x20 */ s8 mIndex;
    /* 0x21 */ u8 mCount : 2;
    u8 mMet : 1;
    u8 mGiven : 1;
    u8 mMoved : 1;
    u8 mTalked : 1;
    u8 mGivenHere : 1;
    u8 mRefusedNewLook : 1;
    /* 0x22 */ u8 mFirstGift : 1;
    u8 mFirstGiftDone : 1;
    u8 mGreetedNewLook : 1;
    u8 _22_4 : 1;
    u8 _22_0 : 4;
};

// 0xC at dPrivateData_c+0x55FC (mBirthdayHost): who hosts the player's birthday party this year.
class dBirthdayHost_c {
public:
    dBirthdayHost_c(); // 8013C3A0
    ~dBirthdayHost_c(); // 8013C3A4

    void clear(); // 8013C3E4

    /* 0x0 */ s32 mHostYear;   // the year the host was picked (d_animal pickBirthdayHost)
    /* 0x4 */ s32 mLetterYear; // the year the party letter was sent
    /* 0x8 */ s8 mHostIdx;     // the villager (dAnimalBlock_c index) hosting this year's party
};

// 0xC2 at dPrivateData_c+0x5608 (mVisitorLetter): a villager and an item.
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

// A year marker (mValentineYear / mNewYearYear at dPrivateData_c+0x8634 / 0x8638), -1 when cleared.
class dPrivateYear_c {
public:
    dPrivateYear_c(); // 8013C4EC
    ~dPrivateYear_c(); // 8013C4F0

    void clear(); // 8013C530

    s32 mYear; // the year it last happened (-1: never)
};

// A visit with another player (0x2E, dPrivateData_c::mHost at +0x7FA8; copied to/from dSaveTown_c::mTownHost).
struct dPrivateHost_c {
    void decrease(int n); // 801363E0

    int getCount(void) const { return mCount; }
    void clearFlagB() { mFlagB = 0; }
    int getFlagC() const { return mFlagC; }
    dPersonalID_c& getPID() { return mPID; }
    void setCount(int count) {
        mCount = count;
    }
    void clear() {
        mPID.clear();
        mCount = 0;
        mIsGuest = 0;
        mFlagB = 0;
    }

    dPersonalID_c mPID; // the other player
    u8 mCount : 4;      // days left (1..7 when due)
    u8 mIsGuest : 1; // this side is the guest (the town copy has the opposite value)
    u8 mFlagB : 1;
    u8 mFlagC : 1;
    u8 mFlagD : 1;
};

// 0x886 (dSaveExtra_c::mDesignBoard): a design relayed between friends over WiiConnect24 (d_wifi sendDesign /
// receiveDesign). Names inferred.
class dRelayDesign_c {
public:
    void clear(); // 80136638
    void decrease(int n); // 8013675C
    void update(); // 8013677C
    void validate(); // 80136868
    BOOL isHopLimit(); // 801368F4

    u8 getHops(void) const { return mHops; }

    /* 0x000 */ dDesign_c mDesign;
    /* 0x880 */ u8 mKind;        // 2 when sent (d_wifi receiveDesign checks it)
    /* 0x881 */ u8 mSendPending; // d_wifi sends it once a day while set
    /* 0x882 */ u8 mDays;        // days until the next step (update)
    /* 0x883 */ u8 mActive;      // a design is being relayed
    /* 0x884 */ u8 _884;
    /* 0x885 */ u8 mHops;        // friends it passed through; stops at 8 (isHopLimit)
};

struct dBirthday_c {
    dBirthday_c() : mMonth(0), mDay(1) {}
    BOOL isSame(u8 month, u8 day) const { return mMonth == month && mDay == day; }

    u8 mMonth;
    u8 mDay;
};

// dPrivateData_c::mFortune (the fortune teller's reading; villagers comment on it, d_npc_talk_approach).
enum dFortune_e {
    FORTUNE_NONE,
    FORTUNE_LOVE,       // 1 approach: villagers of the other gender
    FORTUNE_FRIENDSHIP, // 2 approach: villagers of the same gender
    FORTUNE_MONEY,      // 3 field assessment doubles the money-tree chance
    FORTUNE_ITEM,       // 4 approach / rollan: item luck
};

struct dSaveOption_c;
struct dSaveMailBox_c;
struct dSaveItemBox_c;
struct dSaveDesignBox_c;
class dPlayerPostBox_c;

class dPrivateData_c {
public:
    // The player's option bits (_83E8, the first byte of a dSaveOption_c; d_save_data getOption).
    dSaveOption_c *getOption() { return (dSaveOption_c *)&_83E8; }
    const dItem::Item *getHeldItem() const { return &mEquipment.mHeld; }

    // Checks on the current player (dPlayerMgr::Mgr_c::getCurrentPlayer) for the special npcs' appearances (d_npc_mng):
    // not resetting, from this town, offline.
    static BOOL isHostVisitReady();        // 80136244: mHost set up (1..7) with this player as the host
    static BOOL isGuestVisitReady();       // 80136310: mHost set up (1..7) with this player as the guest
    static void saveHostToTown();          // 80136460
    static void loadHostFromTown();        // 8013654C
    static BOOL isRelayDesignVisitReady(); // 80136694: a relay design is active and due (WC24 on)

    dPrivateData_c();                       // 80136914
    ~dPrivateData_c();                      // 80136B08
    void copy(const dPrivateData_c *other); // 80136C7C: memcpy 0x86C0

    // Checksum over 0x1124..0x86C0 (sCrc::calcCRC32, seed -1), stored in mChecksum.
    void updateChecksum();                                            // 80136C88: updates mFriends first
    BOOL isChecksumValid(int arg);                                    // 80136CC0: checks mFriends first
    u32 calcChecksum() const;                                         // 80136D1C
    static void updateChecksumAll(dPrivateData_c *players);           // 80136D40: 4 players, stride 0x86C0
    static BOOL isChecksumValidAll(dPrivateData_c *players, int arg); // 80136D90

    // Run over all 4 save players with a valid mPID.
    static BOOL isUserDataValidAll(dPrivateData_c *players);      // 80136E10: dNetMgr_c::isUserDataValid for each
    static void setFlag0All(dPrivateData_c *players, u32 flag);   // 80136E90
    static void clearFlag0All(dPrivateData_c *players, u32 flag); // 80136F00
    static BOOL isFlag0Any(dPrivateData_c *players, u32 flag);    // 80136F70

    void setup(const wchar_t *name, u16 id, u8 gender); // 80137000
    int findInSave() const;                             // 80137110

    // Daily update, and the letters it sends.
    void sendBankAnniversaryLetter(); // 801371DC: "MAIL_ETC_ATM" with a town hall model, a year after mBankStamp
    void sendLostChildThanks();       // 8013760C: "MAIL_NPC_maigo" with a present (flag0 0x4A)
    static BOOL sendLetter(u16 kind, const dItem::Item *present, dPrivateData_c *player, int amount); // 80137788
    void updateBank(int days);        // 80137898: interest, the savings milestone letters, a new shopping card
    void updateLooks(int days);       // 80137BCC
    void updateTan(int days);         // 80137C10
    void updateTanDaily(int days);    // 80137CD0: updateTan once per session (d_field_assessment)
    void updateHair(int days);        // 80137D34: bed head after 15 days away, once per session
    void dailyUpdate(int days);       // 80137DA4

    // Items.
    void setPocket(const dItem::Item *item, int idx, BOOL flag); // 80137F58
    // Empties a pocket (d_a_npc_nml talk_c 800315D4 / 800319F4: the inlined local takes the stack
    // slot below the caller's own Item locals).
    void clearPocket(int idx) {
        dItem::Item none;
        setPocket(&none, idx, FALSE);
    }
    int findEmptyPocket(int start);                                        // 801380EC
    BOOL pickUp(const dItem::Item *item, BOOL flag);                       // 8013812C
    int countPockets(BOOL (*fn)(const dItem::Item *), u16 *mask);          // 801381B4
    int countPocketsFlag(BOOL (*fn)(const dItem::Item *, int), u16 *mask); // 8013828C
    void setPocketFlag(int idx, int flag);                                 // 8013834C
    void setLetter(const dMail_c *mail, int idx);                          // 8013835C: d_post_office
    void clearLetter(int idx);                                             // 8013836C
    int findLetter();                                                      // 8013837C
    int getPocketFlag(int idx) const;                                      // 801383DC
    void clear();                                                          // 801383EC

    // Flag bits.
    BOOL isFlag0(u32 flag) const; // 80138580
    void setFlag0(u32 flag);      // 801385C0
    void clearFlag0(u32 flag);    // 801385F0
    BOOL isFlag1(u32 flag);       // 80138620
    void setFlag1(u32 flag);      // 80138660
    void clearFlag1(u32 flag);    // 80138690
    void setFlag2(u32 flag);      // 801386C0
    BOOL isFlag3(u32 flag);       // 801386F0
    void setFlag3(u32 flag);      // 80138730

    // The bank card (shopping / gold card).
    BOOL isCardMissing();       // 80138760: the account is open but no card is carried (and none is on its way)
    int getCardSavings() const; // 80138890: mSavings when a card is carried, else 0

    // Money.
    void setSavings(int amount);                // 801389A4
    void addSavings(int amount);                // 801389C4
    void subSavings(int amount);                // 801389EC
    void payDebt(int amount);                   // 80138A0C
    void setDebtFromHouse();                    // 80138A38
    int getPocketMoney() const;                 // 80138AC8
    int getMoneyRoom(int slots);                // 80138B58
    int findMoneyPocketMostRoom();              // 80138C24
    int findMoneyPocketSmallest();              // 80138CE0
    BOOL addMoney(int amount);                  // 80138D84
    BOOL payMoney(int amount, BOOL allowItems); // 80138E78

    u8 getHouseSize();           // 80138FFC
    void setHouseSize(u8 value); // 80139018
    void syncHouseSize();        // 8013902C: from the player's dHome_c (d_s_stage)

    void addNookPoints(int points); // 80139090
    void subNookPoints(int points); // 801390E8
    void addNookPointsForShop();    // 801390F8

    void setFortuneWeek();   // 801391A0: Katrina's reading this week (d_a_npc_sp_fortune)
    BOOL isNewFortuneWeek(); // 801391E0

    void incResetCount();          // 801392B8: d_a_npc_sp_resetsan
    void setResetCount(u8 value);  // 801392F4: 9 when over 10
    void syncResetCount();         // 80139314: flag0 0x58 when the count went down; mLastResetCount = mResetCount
    u8 getResetCount();            // 80139364
    u8 getLastResetCount();        // 80139370
    BOOL isDonAtCafe();            // 8013937C: flag0 0x61 (d_save_cafe_guest: Don can be the noon guest)
    int getNookPurchases();        // 80139384
    void incNookPurchases();       // 80139390: up to 50 (d_a_npc_sp_tanukichi)
    BOOL isNookLoyaltyLetterDue(); // 801393AC: 50 purchases and its "MAIL_ETC_TPS" letter not sent (d_save_shop)

    // This player's blocks in the extra save data (by its index in the save, found by mPID).
    dPlayerPostBox_c *getPostBox();   // 80139408: its house mailbox (dSaveExtra_c::mPlayerPostBoxes)
    dSaveMailBox_c *getMailBox();     // 80139468: its saved letters
    dSaveItemBox_c *getItemBox();     // 801394D0: its saved letter presents
    dSaveDesignBox_c *getDesignBox(); // 80139530: its saved designs

    // Invitation letters (d_a_npc_sp_tunekichi, d_a_str_town).
    dMail_c *findAnsweredInvite();   // 80139594: read and flagged, from a player first, then from a villager
    dMail_c *findUnansweredInvite(); // 8013967C: read but not flagged
    BOOL hasInvite();                // 801396F4

    void incNewcomerTip(); // 8013974C: next newcomer tip, wraps after 9

    // Errand.
    dQuestErrand_c *findErrand(dAnmPersonalID_c *animal, int which, u32 idx); // 80139770
    static BOOL isPocketErrandItem(const dItem::Item *item, int flag);         // 80139914
    int countErrandPockets(u16 *mask);                                         // 80139938
    BOOL expireErrand(const dTime_c *now); // 80139948: marks the errand items (pocket flag 5) once it expired

    // Static helpers over the 4 save players.
    static void clearAll(dPrivateData_c *players);                                 // 8013AEC0
    static void clearPlayer(dPrivateData_c *players, int idx);                     // 8013AF0C
    static int find(dPrivateData_c *players, const dPersonalID_c *pid);            // 8013AFB8
    static int findByPlayerID(dPrivateData_c *players, const dPlayerID_c *id);     // 8013B080
    static int findEmpty(dPrivateData_c *players);                                 // 8013B104
    static int count(dPrivateData_c *players);                                     // 8013B174
    static dPrivateData_c *get(dPrivateData_c *players, const dPersonalID_c *pid); // 8013B1E0
    static dPrivateData_c *getChecked(dPrivateData_c *players, int idx);           // 8013B218
    static dPrivateData_c *getRaw(dPrivateData_c *players, int idx);               // 8013B28C
    static int getNthValid(dPrivateData_c *players, u32 n);                        // 8013B2C8
    static BOOL contains(dPrivateData_c *players, const dPersonalID_c *pid);       // 8013B344
    static dQuestErrand_c *findErrandAll(dPrivateData_c *players, dAnmPersonalID_c *animal, int which, u32 idx); // 8013B3D4
    static BOOL expireErrandAll(dPrivateData_c *players, const dTime_c *now);      // 8013B474

    // Public works votes (the town hall's BBS notices "BBS_office").
    static int tallyBuildingVotes();         // 8013B4F8: 1 windmill / 2 lighthouse by mBuildingVote (ties: random)
    static int tallyBridgeVotes();           // 8013B5CC: the most voted block (x * 7 + y), -1 when no votes
    static void clearPublicWorksVotes();     // 8013B7A4
    static void buildVotedBridge();          // 8013B848: the voted block, else a random bridge block
    static void buildFountain();             // 8013B9F8
    static void buildVotedBuilding();        // 8013BA50: the windmill or the lighthouse
    static void finishPublicWorksVote();     // 8013BBCC
    static void updatePublicWorks(int days); // 8013BC38: d_field_assessment

    // Starts a visit with another player (mHost and dSaveTown_c::mTownHost) with a 1 in `chance` chance; flag 1:
    // with the current player (d_net), else the last valid save player (d_travel_data).
    BOOL startHostVisit(int chance, int count, int flag); // 8013BDA0

    u8 getCafeCount(int i);                    // 8013C2A0
    void incCafeCount(int i);                  // 8013C2B0: up to 0xFF
    BOOL isBirthday(const dTime_c &cal) const; // 8013C2D0

    // GracieGrace.
    BOOL isBedHeadVisible() const;   // 8013C71C: bed head hair, no Mii mask, a hat that doesn't hide the hair
    u8 getGracieStep();              // 8013C7C0
    BOOL isGracieStoryReady() const; // 8013C7CC
    void updateGracieStep();         // 8013C878: advances mGracieStep as its flags are set (dailyUpdate)
    dOutfit_c *getOutfit(int i);     // 8013CAD8


    BOOL isChecksumOK() const {
        u32 checksum = calcChecksum();
        if (checksum == mChecksum) {
            return TRUE;
        }

        return FALSE;
    }

    /* 0x0000 */ dFriendList_c mFriends;
    /* 0x1120 */ u32 mChecksum;
    /* 0x1124 */ dPrivateHRAInfo_c mHRAInfo;
    /* 0x1134 */ s32 mBells;
    /* 0x1138 */ s32 mDebt;
    /* 0x113C */ s32 mSavings;
    /* 0x1140 */ dDesignList_c mOrgDesigns;
    /* 0x5560 */ dSvStarSign::Sign_c mStarSign; // the constellation being drawn (d_sv_star_sign)
    /* 0x55CE */ dItemPairRing_c mItemPairRing; // d_npc_info / d_net (NET_PACKET_NPC_SPOT_A_FLAG)
    /* 0x55F4 */ u32 _55F4;
    /* 0x55F8 */ u32 mDistPatternVersion; // the downloaded pattern version last seen (d_field_assessment)
    /* 0x55FC */ dBirthdayHost_c mBirthdayHost;
    /* 0x5608 */ dAnimalItem_c mVisitorLetter;
    /* 0x56CA */ dMail_c mLetters[PLAYER_MAIL_COUNT];
    /* 0x7A6A */ dMail_c mFutureSelfLetter;
    /* 0x7DFA */ dLetterStyle_c mLetterStyle;
    /* 0x7EC2 */ dPersonalID_c mPID;
    /* 0x7EEE */ dPersonalID_c mLastWc24Sender; // the friend who last sent this player a WC24 message (d_wifi)
    /* 0x7F1A */ dEquip_c mEquipment;
    /* 0x7F22 */ dItem::Item mPockets[PLAYER_POCKETS_COUNT];
    /* 0x7F40 */ dOutfit_c mOutfits[PLAYER_OUTFIT_COUNT];
    /* 0x7FA0 */ u16 mNookPoints; // current points
    /* 0x7FA2 */ u16 mNookPointsLifetime; // total lifetime points
    /* 0x7FA4 */ u16 mNookPointsMax; // max balance reached
    /* 0x7FA6 */ u16 mVillagerDailyBits; // one bit per villager, cleared daily (dSvPlayerFlag)
    /* 0x7FA8 */ dPrivateHost_c mHost;         // a visit with another player (startHostVisit; mirrored in dSaveTown_c::mTownHost)
    /* 0x7FD6 */ dSaveMotherMail_c mMotherMail;
    /* 0x7FEE */ dQuestErrandList_c mErrand;
    /* 0x83BE */ dYMD_c _83BE;
    /* 0x83C2 */ dYMD_c _83C2;
    /* 0x83C6 */ dYMD_c mWc24LetterDate; // the day WC24 letters were last sent (d_wifi isLetterDayOver)
    /* 0x83CA */ dItem::Item mSonchoPresent;
    /* 0x83CC */ u16 mBunnyEggs[13]; // Bunny Day egg counts (12 kinds + the fake egg)
    /* 0x83E6 */ dBirthday_c mBirthday;
    /* 0x83E8 */ u8 _83E8;
    /* 0x83E9 */ u8 _83E9;
    /* 0x83EA */ u8 mFace;
    /* 0x83EB */ u8 mHair;
    /* 0x83EC */ u8 mHairColor;
    /* 0x83ED */ dMiiCreateID_c mMiiId;
    /* 0x83F5 */ u8 mMiiMask;     // a Mii mask is worn (d_msg / d_animal / d_player_mgr, inferred)
    /* 0x83F6 */ u8 mShoeColor;
    /* 0x83F7 */ u8 mShoeKind;    // 0: no shoes (dOutfit_c::setFromPlayer); d_player_mgr sets it with mShoeColor
    /* 0x83F8 */ u8 mTan;
    /* 0x83F9 */ u8 mFortune;     // dFortune_e: today's fortune (reset daily by d_field_assessment)
    /* 0x83FA */ dCatalog_c mCatalog;
    /* 0x85FA */ dPrivateSongs_c mSongs;
    /* 0x8604 */ dPrivateGyroids_c mGyroids;
    /* 0x8614 */ dPrivateEmotions_c mEmotions;
    /* 0x8618 */ dTimeStamp_c mBankStamp;   // the bank account's anniversary (sendBankAnniversaryLetter)
    /* 0x8620 */ dTimeStamp_c mFortuneWeek; // the fortune teller's last week (setFortuneWeek / isNewFortuneWeek, Katrina)
    /* 0x8628 */ s32 mDaysAway;         // days since the player last played (dailyUpdate; 0 once played)
    /* 0x862C */ s32 mPendingInterest;  // interest not yet paid out (updateBank)
    /* 0x8630 */ s32 mDaysPassed;       // days counted in total (dailyUpdate)
    /* 0x8634 */ dPrivateYear_c mValentineYear;
    /* 0x8638 */ dPrivateYear_c mNewYearYear;
    /* 0x863C */ u8 mPocketFlags[PLAYER_POCKETS_COUNT]; // one per pocket; getPocketMoney skips nonzero
    /* 0x864B */ u8 mFlags0[PRIVATE_FLAGS0_NUM / 8]; // bit 41 = reset flag
    /* 0x865F */ u8 mFlags1[PRIVATE_FLAGS1_NUM / 8];
    /* 0x866F */ u8 mFlags2[PRIVATE_FLAGS2_NUM / 8];
    /* 0x867F */ u8 mFlags3[PRIVATE_FLAGS3_NUM / 8];
    /* 0x868F */ u8 mShopVote;       // the Nookington's vote (d_save_shop)
    /* 0x8690 */ u8 _8690;
    /* 0x8691 */ u8 _8691;
    /* 0x8692 */ u8 mCityVisitDays;  // days with flag0 0x17 (city visit, inferred), up to 11 (isGracieStoryReady)
    /* 0x8693 */ u8 mNookPurchases;  // up to 50 (d_a_npc_sp_tanukichi; isNookLoyaltyLetterDue)
    /* 0x8694 */ u8 mResetCount;     // Mr. Resetti visits (d_a_npc_sp_resetsan), 9 when over 10
    /* 0x8695 */ u8 mLastResetCount; // mResetCount when last synced (syncResetCount)
    /* 0x8696 */ u8 mNewcomerTip; // next newcomer tip (Ev_Arbeit message 6 + mNewcomerTip, 0..8)
    /* 0x8697 */ u8 _8697;
    /* 0x8698 */ u8 _8698;
    /* 0x8699 */ u8 _8699;
    /* 0x869A */ s8 mBridgeVoteX;    // the block voted for the new bridge (-1: none; tallyBridgeVotes)
    /* 0x869B */ s8 mBridgeVoteY;
    /* 0x869C */ u8 mBuildingVote;   // 1 windmill, 2 lighthouse (tallyBuildingVotes)
    /* 0x869D */ u8 mCafeCounts[5]; // per cafe guest (d_a_npc_sp_cafe_guest)
    /* 0x86A2 */ u8 _86A2;
    /* 0x86A3 */ u8 mGracieStep;    // GracieGrace story step 0..10 (updateGracieStep; Gracie, the Able sisters, ...)
    /* 0x86A4 */ u8 _86A4;
    /* 0x86A5 */ u8 mHouseSize;     // the house size (syncHouseSize; d_msg house size jump)
    /* 0x86A6 */ u8 _86A6[0x1A];
}; // size 0x86C0
