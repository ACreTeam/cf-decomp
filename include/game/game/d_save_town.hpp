#pragma once

// The town part of the save file (dSaveData_c's base, 0x735E0 bytes: everything before
// dSaveExtra_c), with its own CRC (mTownChecksum). Source: src/dol/game/d_save_town.cpp.
// dSaveData_c::getTown() returns it once the town has been transferred.

#include <types.h>
#include <game/game/d_item.hpp>
#include <game/game/d_dsn.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_model_room.hpp>
#include <game/game/d_police_box.hpp>
#include <game/game/d_recycle_bin.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_museum.hpp>
#include <game/game/d_notice.hpp>
#include <game/game/d_save_check.hpp>
#include <game/game/d_save_dl_item.hpp>
#include <game/game/d_bug_off.hpp>
#include <game/game/d_save_main_field.hpp>
#include <game/game/d_save_shops.hpp>
#include <game/game/d_save_visitor_npc.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_land.hpp>
#include <game/game/d_save_building.hpp>

// Saved game-clock offset (fn_8014D054 / fn_8014D064 / fn_8014D07C / fn_8014D09C).
struct dSaveTimeOffset_c {
    /* 0x0 */ s64 mOffset; // dTime_c::sOffset
    /* 0x8 */ u8 _8[8];
}; // size 0x10

// 0x78 record (ctor fn_8010EB50: Item = none, then fn_8010A758).
struct dSaveRecord78_c {
    /* 0x00 */ dItem::Item mItem;
    /* 0x02 */ u8 _02[0x76];
}; // size 0x78

// Skeleton members whose classes live in unsplit TUs. Their constructors keep the target's C names
// until those TUs are split; the inline ctors reproduce the calls dSaveData_c::create makes.
extern "C" {
void fn_80150140(void *obj); // 80150140
void fn_80150B94(void *obj); // 80150B94
}
struct dSaveUnk72D1A_c {
    dSaveUnk72D1A_c() { fn_80150140(this); }
    u8 _00[0xC0];
};
struct dSaveUnk72E0A_c {
    dSaveUnk72E0A_c() { fn_80150B94(this); }
    u8 _00[0x6E8];
};

// Another town the residents know about (4 slots, dSaveTown_c::mOtherTowns). The villager dialog in d_npc
// (8003B3B4) talks about it: its land, one of its players, a villager, and its shop size and public works
// (the same things getPublicWorks reads for this town).
struct dSaveOtherTown_c {
    /* 0x000 */ dLandID_c mLand;
    /* 0x016 */ dPersonalID_c mPlayer;
    /* 0x042 */ u8 mVillager[0xC0];    // the villager's core data
    /* 0x102 */ u8 _102;                // < 0x31: shown as STR_Impress + 1
    /* 0x103 */ u8 _103;
    /* 0x104 */ u8 mValid : 1;          // getRandomOtherTown
    /* 0x104 */ u8 mShopStage : 2;      // compared with dSaveShop_c's stage
    /* 0x104 */ u8 mHasBridge : 1;      // PUBLIC_WORKS_BRIDGE
    /* 0x104 */ u8 mHasFountain : 1;
    /* 0x104 */ u8 mHasWindmill : 1;
    /* 0x104 */ u8 mHasLighthouse : 1;
    /* 0x104 */ u8 _104_0 : 1;
    /* 0x105 */ u8 _105;
}; // size 0x106

// dSaveTown_c::getPublicWorks: the town hall's public works so far, in the order they're built: the extra
// bridge (dSaveMainField_c::buildBridge), the fountain, then the windmill or the lighthouse
// (BUILDING_FOUNTAIN / WINDMILL / LIGHTHOUSE, built by fn_80169C48 from SAVE_FLAG_BUILD_*).
enum {
    PUBLIC_WORKS_NONE = 0,
    PUBLIC_WORKS_BRIDGE = 1, // the extra bridge is built (dSaveMainField_c::mBridgeBlockX/Z == -1)
    PUBLIC_WORKS_FOUNTAIN = 2,
    PUBLIC_WORKS_TOWER = 3, // the windmill or the lighthouse
};

// dSaveTown_c::mNewConstruction: something new in the town today (cleared on each new day by
// fgMngProc_procDayChange).
enum {
    NEW_CONSTRUCTION_NONE = 0,
    NEW_CONSTRUCTION_BRIDGE = 1,       // dSaveMainField_c::buildBridge; with the bridge's block
    NEW_CONSTRUCTION_FOUNTAIN = 2,
    NEW_CONSTRUCTION_WINDMILL = 3,     // only the first time (SAVE_FLAG_WINDMILL_BUILT)
    NEW_CONSTRUCTION_LIGHTHOUSE = 4,   // only the first time (SAVE_FLAG_LIGHTHOUSE_BUILT)
};

// Town flags: 64 bits at dSaveTown_c::mFlags, indexed 0..63. Most indices are not named yet.
enum {
    SAVE_FLAG_TURNIPS_SPOILED = 1, // set when the clock is changed or goes back within the week, cleared each new week (inferred)
    SAVE_FLAG_BUILD_WINDMILL = 7,    // fn_80169C48 builds it (in place of the lighthouse)
    SAVE_FLAG_BUILD_LIGHTHOUSE = 8,  // fn_80169C48 builds it (in place of the windmill)
    SAVE_FLAG_BUILD_FOUNTAIN = 9,    // fn_80169C48 builds it
    SAVE_FLAG_PUBLIC_WORKS_FUNDED = 10, // the next public work is paid for; the vote starts on the next day
    SAVE_FLAG_PUBLIC_WORKS_VOTE = 11,   // the vote is on (BBS_office 1 / 4 / 7 / 8) until mPublicWorksDays runs out
    SAVE_FLAG_LIGHTHOUSE_BUILT = 12, // setNewConstruction(NEW_CONSTRUCTION_LIGHTHOUSE) only once
    SAVE_FLAG_WINDMILL_BUILT = 13,   // setNewConstruction(NEW_CONSTRUCTION_WINDMILL) only once
    SAVE_FLAG_FLAG6_BACKUP = 0x1A,   // backupFlag6; restored to flag 6 by startFgMngProcThread
    SAVE_FLAG_BUS_NET_EVENT_DONE = 0x1F, // set by the player-create NPC REL after its scene on the bus to the city
};

class dSaveTown_c {
public:
    BOOL isHeaderVersionOK();             // 80115CDC
    BOOL isHeaderState2();                // 80115CE0
    void updateTownChecksum();            // 80115CE4: header, players, animals, mTownChecksum
    BOOL isTownChecksumOK(int arg);       // 80115D34
    u32 calcTownChecksum() const;         // 80115DDC: everything after mTownChecksum
    void clearTown();                     // 80115E04: new town
    void deletePlayer(int player);        // 80115F44: also removes their designs from the town field
    void resetSaveTime();                 // 80116180: mSaveTime = an hour before 2000-01-01
    static int getPublicWorks();          // 801161F0: PUBLIC_WORKS_*; dPrivateData_c's "BBS_office" notices
    int getRandomOtherTown() const;       // 80116310: a random valid mOtherTowns slot, or -1
    void setNewConstruction(int kind, int blockX, int blockZ); // 8011641C: NEW_CONSTRUCTION_*
    BOOL isFlag(int idx) const;           // 801164D0: out-of-range indices are FALSE
    void setFlag(int idx);                // 80116510
    void clearFlag(int idx);              // 80116540
    static BOOL isBusNetEventPending();   // 80116570: WiiConnect24 connected and SAVE_FLAG_BUS_NET_EVENT_DONE not set
    void backupFlag6();                   // 801165B8: SAVE_FLAG_FLAG6_BACKUP = flag 6

    /* 0x000000 */ dSaveCheck_c mHeader;
    /* 0x000020 */ dPrivateData_c mPlayers[PLAYER_NUM];
    /* 0x021B20 */ dAnimalSave_c mAnimals;
    /* 0x05E260 */ dDesign_c _05E260;
    /* 0x05EAE0 */ u8 _05EAE0[0x24];
    /* 0x05EB04 */ u32 _05EB04;             // object, ctor 80149E38; checked against fn_8014B3CC
    /* 0x05EB08 */ u8 _05EB08[0x158];
    /* 0x05EC60 */ u32 mTownChecksum;     // calcTownChecksum: +0x5EC64 up to mExtra (also a random seed)
    /* 0x05EC64 */ u16 _05EC64;
    /* 0x05EC66 */ u8 _05EC66;
    /* 0x05EC67 */ u8 _05EC67;
    /* 0x05EC68 */ u8 _05EC68[0xE];
    /* 0x05EC76 */ u8 _05EC76;              // bitfield byte, cleared by the ctor
    /* 0x05EC77 */ u8 _05EC77;
    /* 0x05EC78 */ u8 _05EC78[8];
    /* 0x05EC80 */ dSaveShops_c mShops;
    /* 0x0632E0 */ dSaveTimeOffset_c mTimeOffset; // dTime_c::loadOffset / saveOffset
    /* 0x0632F0 */ u8 _0632F0[0x200];       // 3 dTimeStamp_c, Items at +0x1F8..; fn_80152428, fn_80151CBC
    /* 0x0634F0 */ dBugOff_c mBugOff;          // Bug-Off standings (d_bug_off)
    /* 0x0636F0 */ dModelRoom_c _0636F0;
    /* 0x063C3C */ u8 _063C3C[4];
    /* 0x063C40 */ dSaveRecord78_c _063C40[9]; // then fn_8010C0A4 on the array
    /* 0x064078 */ u8 _064078[0x52];
    /* 0x0640CA */ dOutfit_c _0640CA;
    /* 0x0640D6 */ u8 _0640D6[0x116];
    /* 0x0641EC */ u8 _0641EC;              // bitfield byte, cleared by the ctor
    /* 0x0641ED */ u8 _0641ED;
    /* 0x0641EE */ u8 _0641EE[2];
    /* 0x0641F0 */ dModelRoom_c _0641F0;
    /* 0x06473C */ dSaveDLItem_c _06473C;
    /* 0x06673C */ u8 _06673C[3];
    /* 0x06673F */ u8 _06673F;
    /* 0x066740 */ u8 _066740[0xA];
    /* 0x06674A */ dSaveOtherTown_c mOtherTowns[4];
    /* 0x066B62 */ dNoticeBoard_c mNoticeBoard;
    /* 0x068372 */ u8 _068372[0x50];        // ctor 8014D0BC
    /* 0x0683C2 */ u16 _0683C2;             // an item id (d_fg_item)
    /* 0x0683C4 */ u8 _0683C4[4];
    /* 0x0683C8 */ dSaveVisitorNpc_c mVisitorNpc;
    /* 0x0683DF */ u8 _0683DF;
    /* 0x0683E0 */ u16 _0683E0;
    /* 0x0683E2 */ u8 _0683E2;
    /* 0x0683E3 */ u8 _0683E3;
    /* 0x0683E4 */ u8 _0683E4[4];
    /* 0x0683E8 */ u8 _0683E8[2][8];        // {u16, u8, u8, pad}, zeroed by the ctor
    /* 0x0683F8 */ u16 _0683F8;
    /* 0x0683FA */ u8 _0683FA;
    /* 0x0683FB */ u8 _0683FB;
    /* 0x0683FC */ u8 _0683FC[2];
    /* 0x0683FE */ dLandID_c mLandID;       // this town
    /* 0x068414 */ dSaveMainField_c mMainField; // the town field
    /* 0x06D5BC */ u8 _06D5BC[4];
    /* 0x06D5C0 */ dHomeList_c mHomes;
    /* 0x072CC0 */ u8 _072CC0[0x2E];
    /* 0x072CEE */ dPersonalID_c _072CEE;   // cleared by clearTown
    /* 0x072D1A */ dSaveUnk72D1A_c _072D1A; // 8 x 0x18 entries
    /* 0x072DDA */ dPoliceBox_c mPoliceBox;
    /* 0x072DF2 */ dRecycleBin_c mRecycleBin;
    /* 0x072E0A */ dSaveUnk72E0A_c _072E0A;
    /* 0x0734F2 */ dPrivateHost_c mTownHost; // copied to/from dPrivateData_c::mHost
    /* 0x073520 */ u16 mItemVersion;        // dItem::BITM version, checked by isExtraGood
    /* 0x073522 */ dTimeStamp_c mSaveTime;  // now, when saving (fn_8010DCF0); later than now = the clock went back
    /* 0x07352A */ dMuseum_c mMuseum;
    /* 0x07359E */ dSaveMelody_c mVillageMelody; // the town tune
    /* 0x0735AE */ u8 _0735AE;               // fn_8015384C's object starts here
    /* 0x0735AF */ dItem::dSaveItemRarity_c mItemRarity;
    /* 0x0735B7 */ u8 _0735B7[0xB];         // fn_801541D8
    /* 0x0735C2 */ u8 _0735C2;              // low nibble read by d_item
    /* 0x0735C3 */ u8 mFlags[8];           // town flags, bits 0..63 (SAVE_FLAG_*, isFlag / setFlag / clearFlag)
    /* 0x0735CB */ u8 mPublicWorksDays;     // days left in the public works vote; 0xFF = none
    /* 0x0735CC */ u8 mNewConstruction;     // NEW_CONSTRUCTION_*
    /* 0x0735CD */ s8 mNewConstructionBlockX;
    /* 0x0735CE */ s8 mNewConstructionBlockZ;
    /* 0x0735CF */ u8 _0735CF[0x11];
}; // size 0x735E0
