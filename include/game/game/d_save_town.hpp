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
#include <game/game/d_sv_auc.hpp>
#include <game/game/d_sv_town_info.hpp>
#include <game/game/d_save_building.hpp>

#include <game/game/d_sv_time_offset.hpp>

// Skeleton members whose classes live in unsplit TUs. Their constructors keep the target's C names
// until those TUs are split; the inline ctors reproduce the calls dSaveData_c::create makes.
extern "C" {
void fn_80150140(void *obj); // 80150140
void fn_80150B94(void *obj); // 80150B94
}
// A town's jinx (called a "charm" in the English game): a superstition villagers pass on about a town,
// "If you <condition> in <town>... <outcome>!" (message labels FreeA_Jinx for another town, FreeG_JinxV for
// this one; condition text = message code 11 + condition, outcome text = 31 + outcome). Names below follow
// the English message text.
enum dJinxCondition_e {
    JINX_COND_HEART_LETTER,     // 0 send a heart-filled letter from the town hall
    JINX_COND_CUT_TREE,         // 1 cut down a single tree
    JINX_COND_PULL_WEEDS,       // 2 go on a weed-plucking spree (mValue weeds)
    JINX_COND_PLANT_TREES,      // 3 plant exactly mValue trees
    JINX_COND_PLANT_FLOWERS,    // 4 plant exactly mValue flowers
    JINX_COND_FOUR_LEAF_CLOVER, // 5 find a four-leaf clover
    JINX_COND_6,                // 6 no message text (unused)
    JINX_COND_RED_TURNIP,       // 7 plant a red turnip seed
    JINX_COND_CATCH_BUG,        // 8 catch a bug (mValue: index from ITEM_IDX_COMMON_BUTTERFLY)
    JINX_COND_CATCH_FISH,       // 9 catch a fish (mValue: index from ITEM_IDX_BITTERLING)
    JINX_COND_MONEY_TREE,       // 10 plant a money tree
    JINX_COND_NUM
};
enum dJinxOutcome_e {
    JINX_OUTCOME_NO_BEES,       // 0 all the bees will disappear from your own town
    JINX_OUTCOME_TOWN_IMPROVES, // 1 the condition of your own town will improve
    JINX_OUTCOME_GYROIDS,       // 2 you'll find a lot of gyroids
    JINX_OUTCOME_FOSSILS,       // 3 it will be easier to find fossils
    JINX_OUTCOME_TREE_LUCK,     // 4 happiness will fall from the trees
    JINX_OUTCOME_RARE_ITEM,     // 5 a chance to get a rare item
    JINX_OUTCOME_FRIENDSHIP,    // 6 your friendships will become stronger
    JINX_OUTCOME_FLOWERS,       // 7 flowers will become less likely to wilt
    JINX_OUTCOME_MONEY,         // 8 your financial outlook will improve
    JINX_OUTCOME_DROPPED_ITEMS, // 9 more things will be dropped in your town
    JINX_OUTCOME_NUM
};
// 2 bytes. Rolled by d_sv_activity; the town's own is dSaveTown_c::mJinx, other towns' are in mTownList.
struct dSaveJinx_c {
    /* 0x0 */ u8 mCondition : 4; // dJinxCondition_e
    /* 0x0 */ u8 mOutcome : 4;   // dJinxOutcome_e
    /* 0x1 */ u8 mValue;         // count (JINX_COND_PULL_WEEDS..PLANT_FLOWERS) or bug / fish (CATCH_BUG / CATCH_FISH)
};
// An entry of dSaveTownList_c: another town (by land) and its jinx.
struct dSaveTownListEntry_c {
    /* 0x00 */ dLandID_c mLand;
    /* 0x16 */ dSaveJinx_c mJinx;
}; // size 0x18
struct dSaveTownList_c;
extern "C" {
dSaveTownListEntry_c *fn_80150308(dSaveTownList_c *list, int idx); // 80150308 (d_sv_town_list): entry idx, NULL if its land is invalid
const dSaveTownListEntry_c *fn_80150350(const dSaveTownList_c *list, int idx); // 80150350 (d_sv_town_list): same body (const)
}
#define SAVE_TOWN_LIST_NUM 8

// d_sv_town_list: up to SAVE_TOWN_LIST_NUM other towns the player knows about, with their jinxes (talk topic FreeA_Jinx).
struct dSaveTownList_c {
    dSaveTownList_c() { fn_80150140(this); }
    dSaveTownListEntry_c *getTown(int idx) { return fn_80150308(this, idx); }
    const dSaveTownListEntry_c *getTown(int idx) const { return fn_80150350(this, idx); }
    /* 0x00 */ dSaveTownListEntry_c mTowns[SAVE_TOWN_LIST_NUM];
}; // size 0xC0
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
    /* 0x102 */ u8 mImpression;         // < 0x31: shown as STR_Impress + 1
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

// Town block at 0x640C8 (methods in the unsplit code at 8014EB3C..: fn_8014EB3C init, fn_8014EFC0,
// fn_8014F030, fn_8014F0A4, fn_8014F248). Holds the auction's featured downloaded item: one object,
// since the target reaches the flag byte and mDLItem from one base.
struct dSaveTown640C8_c {
    /* 0x000 */ u8 _000;
    /* 0x001 */ u8 mFlags;                // 0x20: dSvAuc_c::pickDLItem set mDLItem
    /* 0x002 */ dOutfit_c _002;
    /* 0x00E */ u8 _00E[0x116];
    /* 0x124 */ u8 _124;                  // bitfield byte, cleared by the ctor
    /* 0x125 */ u8 _125;
    /* 0x126 */ u8 _126[2];
    /* 0x128 */ dModelRoom_c _128;
    /* 0x674 */ dSaveDLItem_c mDLItem;
}; // size 0x2674

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
    dSvAuc_c *getAuction() { return (dSvAuc_c *)mAuctionItems; }

    /* 0x000000 */ dSaveCheck_c mHeader;
    /* 0x000020 */ dPrivateData_c mPlayers[PLAYER_NUM];
    /* 0x021B20 */ dAnimalSave_c mAnimals;
    /* 0x05E260 */ dSaveBuilding_c mBuilding;  // the town flag, the gate and the building positions
    /* 0x05EC60 */ u32 mTownChecksum;     // calcTownChecksum: +0x5EC64 up to mExtra (also a random seed)
    /* 0x05EC64 */ u16 _05EC64;
    /* 0x05EC66 */ u8 _05EC66;
    /* 0x05EC67 */ u8 _05EC67;
    /* 0x05EC68 */ u8 _05EC68[0xE];
    /* 0x05EC76 */ dSaveJinx_c mJinx;       // this town's jinx (talk topic FreeG_JinxV)
    /* 0x05EC78 */ u8 _05EC78[8];
    /* 0x05EC80 */ dSaveShops_c mShops;
    /* 0x0632E0 */ dSaveTimeOffset_c mTimeOffset; // dTime_c::loadOffset / saveOffset
    /* 0x0632F0 */ u8 _0632F0[0x200];       // 3 dTimeStamp_c, Items at +0x1F8..; fn_80152428, fn_80151CBC
    /* 0x0634F0 */ dBugOff_c mBugOff;          // Bug-Off standings (d_bug_off)
    /* 0x0636F0 */ dModelRoom_c _0636F0;
    /* 0x063C3C */ u8 _063C3C[4];
    // The auction (d_sv_auc): items, their senders, day and day type; dSvAuc_c describes the whole
    // block (getAuction). Kept as plain members: a wrapper member adds a destructor to our d_save_data.
    // The target's town constructor builds the items then calls dSvAuc_c::clear here, so the original
    // may well have had a dSvAuc_c member (see notes/d_sv_auc.txt).
    /* 0x063C40 */ dSvAucItem_c mAuctionItems[9];
    /* 0x064078 */ u8 mAuctionData[0x50];      // dSvAuc_c::mTags, mDay, mDayType
    /* 0x0640C8 */ dSaveTown640C8_c _0640C8;
    /* 0x06673C */ u8 _06673C[3];
    /* 0x06673F */ u8 _06673F;
    /* 0x066740 */ u8 _066740;
    /* 0x066741 */ u8 _066741[4];          // record of fn_801505E4
    /* 0x066745 */ u8 mHarvestSpot[5];     // d_sv_npc_pos record: the Harvest Festival spot
    /* 0x06674A */ dSaveOtherTown_c mOtherTowns[4];
    /* 0x066B62 */ dNoticeBoard_c mNoticeBoard;
    /* 0x068372 */ dSaveTownInfo_c mTownInfo;
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
    /* 0x072D1A */ dSaveTownList_c mTownList; // other towns and their jinxes
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
    /* 0x0735C2 */ u8 _0735C2;              // low nibble: region (getRegion() value, dSaveData_c::getTownRegion);
                                            // high nibble: language (LANGUAGE_*, dSaveData_c::getTownLanguage)
    /* 0x0735C3 */ u8 mFlags[8];           // town flags, bits 0..63 (SAVE_FLAG_*, isFlag / setFlag / clearFlag)
    /* 0x0735CB */ u8 mPublicWorksDays;     // days left in the public works vote; 0xFF = none
    /* 0x0735CC */ u8 mNewConstruction;     // NEW_CONSTRUCTION_*
    /* 0x0735CD */ s8 mNewConstructionBlockX;
    /* 0x0735CE */ s8 mNewConstructionBlockZ;
    /* 0x0735CF */ u8 _0735CF[0x11];
}; // size 0x735E0
