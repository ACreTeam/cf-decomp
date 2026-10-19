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
#include <game/game/d_animal_id.hpp>
#include <game/game/d_museum.hpp>
#include <game/game/d_sv_notice.hpp>
#include <game/game/d_save_check.hpp>
#include <game/game/d_save_dl_item.hpp>
#include <game/game/d_bug_off.hpp>
#include <game/game/d_ev_fishing.hpp>
#include <game/game/d_save_main_field.hpp>
#include <game/game/d_save_shops.hpp>
#include <game/game/d_save_visitor_npc.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_land.hpp>
#include <game/game/d_sv_auc.hpp>
#include <game/game/d_sv_town_info.hpp>
#include <game/game/d_save_building.hpp>

#include <game/game/d_sv_time_offset.hpp>
#include <game/game/d_sv_daily_rand.hpp>
#include <game/game/d_sv_player_flag.hpp>
#include <game/game/d_sv_daily.hpp>
#include <game/game/d_sv_town_list.hpp>
#include <game/game/d_sv_npc_pos.hpp>
#include <game/game/d_sv_week.hpp>
#include <game/game/d_sv_date_event.hpp>
#include <game/game/d_sv_star_sign.hpp>
#include <game/game/d_sv_villager_move.hpp>


// dSaveTown_c::getPublicWorks: the town hall's public works so far, in the order they're built: the extra
// bridge (dSaveMainField_c::buildBridge), the fountain, then the windmill or the lighthouse
// (BUILDING_FOUNTAIN / WINDMILL / LIGHTHOUSE, built by dSTR::buildPublicWorks from SAVE_FLAG_BUILD_*).
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
    SAVE_FLAG_ROVER_GREETED = 0, // Rover greeted a returning player at the player select (d_a_npc_sp_player_select)
    SAVE_FLAG_TURNIPS_SPOILED = 1, // set when the clock is changed or goes back within the week, cleared each new week (inferred)
    SAVE_FLAG_BUILD_WINDMILL = 7,    // dSTR::buildPublicWorks builds it (in place of the lighthouse)
    SAVE_FLAG_BUILD_LIGHTHOUSE = 8,  // dSTR::buildPublicWorks builds it (in place of the windmill)
    SAVE_FLAG_BUILD_FOUNTAIN = 9,    // dSTR::buildPublicWorks builds it
    SAVE_FLAG_PUBLIC_WORKS_FUNDED = 10, // the next public work is paid for; the vote starts on the next day
    SAVE_FLAG_PUBLIC_WORKS_VOTE = 11,   // the vote is on (BBS_office 1 / 4 / 7 / 8) until mPublicWorksDays runs out
    SAVE_FLAG_LIGHTHOUSE_BUILT = 12, // setNewConstruction(NEW_CONSTRUCTION_LIGHTHOUSE) only once
    SAVE_FLAG_WINDMILL_BUILT = 13,   // setNewConstruction(NEW_CONSTRUCTION_WINDMILL) only once
    SAVE_FLAG_RESET_OPEN = 0x0E,     // 1 day in 10 (dSTR::updateResetOpen): the city's reset building opens at 20:00
                                     // (daStrReset_c; inferred)
    SAVE_FLAG_UFO_SHOT = 0x12,       // the UFO was shot down today (d_a_ufo; becomes SAVE_FLAG_UFO_SHOT_BEFORE)
    SAVE_FLAG_UFO_SHOT_BEFORE = 0x13, // the UFO was shot down on an earlier day (d_field_assessment, inferred)
    SAVE_FLAG_UFO_FLOWN = 0x16,      // the UFO flew over the town today (d_a_ufo)
    SAVE_FLAG_UFO_STR_DRAW = 0x19,   // the crashed UFO (daStrUFO_c) is drawn this frame: read and cleared each frame
    SAVE_FLAG_FLAG6_BACKUP = 0x1A,   // backupFlag6; restored to flag 6 by startFgMngProcThread
    SAVE_FLAG_BUS_NET_EVENT_DONE = 0x1F, // set by the player-create NPC REL after its scene on the bus to the city
};

// A town message (d_wifi WIFI_MSG_TOWN, 0x2724 bytes): the sending town's players and its dSaveTown640C8_c
// (built by d_wifi fn_80179C14, taken over by dWifi::receiveTown).
struct dWifiTownMsg_c {
    // The first valid player (the sender), else mPlayers[0].
    dPersonalID_c *getSender() {
        for (int i = 0; i < PLAYER_NUM; i++) {
            if (mPlayers[i].isValid()) {
                return &mPlayers[i];
            }
        }
        return &mPlayers[0];
    }

    /* 0x0000 */ dPersonalID_c mPlayers[PLAYER_NUM];
    /* 0x00B0 */ dSvVillagerMove::Info_c mTown;
}; // size 0x2724

class dSaveTown_c {
public:
    struct RegionInfo_c {
        u8 language : 4;
        u8 region : 4;
    };
    dPersonalID_c *getFriendPlayer() { return &_072CC2; }
    dSvTownList::List_c *getTownList() { return &mTownList; }
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
    void clearPublicWorksBells() { mPublicWorksBells = 0; } // d_str dSTR::buildPublicWorks
    static BOOL isBusNetEventPending();   // 80116570: WiiConnect24 connected and SAVE_FLAG_BUS_NET_EVENT_DONE not set
    void backupFlag6();                   // 801165B8: SAVE_FLAG_FLAG6_BACKUP = flag 6
    dSvAuc_c *getAuction() { return &mAuction; }
    void setPublicWorksDays(u8 days) { mPublicWorksDays = days; }
    dLandID_c *getLandID() { return &mLandID; }

    /* 0x000000 */ dSaveCheck_c mHeader;
    /* 0x000020 */ dPrivateData_c mPlayers[PLAYER_NUM];
    /* 0x021B20 */ dAnimalSave_c mAnimals;
    /* 0x05E260 */ dSaveBuilding_c mBuilding;  // the town flag, the gate and the building positions
    /* 0x05EC60 */ u32 mTownChecksum ALIGN(32); // calcTownChecksum: +0x5EC64 up to mExtra (also a random seed)
    /* 0x05EC64 */ dSvActivity_c mActivity; // this town's jinx and the day's jinx results (d_sv_activity)
    /* 0x05EC7C */ u32 mPublicWorksBells; // inferred: donated so far; cleared when a public work is built
    /* 0x05EC80 */ dSaveShops_c mShops;
    /* 0x0632E0 */ dSaveTimeOffset_c mTimeOffset; // dTime_c::loadOffset / saveOffset
    /* 0x0632F0 */ dEvFishing_c mFishing;    // fishing tourney standings (d_ev_fishing)
    /* 0x0634F0 */ dBugOff_c mBugOff;          // Bug-Off standings (d_bug_off)
    /* 0x0636F0 */ dSvMdlRm_c mModelRoom;   // the HRA's model room of the month
    /* 0x063C3C */ u8 _063C3C[4];
    /* 0x063C40 */ dSvAuc_c mAuction;          // the auction house (d_sv_auc)
    /* 0x0640C8 */ dSvVillagerMove::Info_c mVillagerMove; // this town's news for its friends (d_sv_villager_move)
    /* 0x06673C */ dSvNpcPos::KindPos_c mNpcSpotA;
    /* 0x066741 */ dSvNpcPos::Pos_c mNpcSpotB;
    /* 0x066745 */ dSvNpcPos::SidePos_c mHarvestSpot; // the Harvest Festival spot
    /* 0x066749 */ u8 _066749;
    /* 0x06674A */ dSaveOtherTown_c mOtherTowns[4];
    /* 0x066B62 */ dNoticeBoard_c mNoticeBoard;
    /* 0x068372 */ dSaveTownInfo_c mTownInfo;
    /* 0x0683C8 */ dSaveVisitorNpc_c mVisitorNpc;
    /* 0x0683DF */ u8 _0683DF;
    /* 0x0683E0 */ dSvDateEvent::Event_c mSnowmen[dSvDateEvent::SNOWMAN_NUM]; // d_a_snowman
    /* 0x0683F8 */ dSvDaily::Daily_c mDaily;
    /* 0x0683FE */ dLandID_c mLandID;       // this town
    /* 0x068414 */ dSaveMainField_c mMainField; // the town field
    /* 0x06D5BC */ u8 _06D5BC[4];
    /* 0x06D5C0 */ dHomeList_c mHomes;
    /* 0x072CC0 */ dSvPlayerFlag::Flag_c mPlayerFlag;
    /* 0x072CC1 */ u8 _072CC1;
    /* 0x072CC2 */ dPersonalID_c _072CC2;   // a random player of a friend town (d_wifi receiveTown, TOWN_640C8_FLAG_PLAYER_BITS)
    /* 0x072CEE */ dPersonalID_c _072CEE;   // cleared by clearTown
    /* 0x072D1A */ dSvTownList::List_c mTownList; // other towns and their jinxes
    /* 0x072DDA */ dPoliceBox_c mPoliceBox;
    /* 0x072DF2 */ dRecycleBin_c mRecycleBin;
    /* 0x072E0A */ dSvStarSign::List_c mStarSigns; // the town's constellations (d_sv_star_sign)
    /* 0x0734F2 */ dPrivateHost_c mTownHost; // copied to/from dPrivateData_c::mHost
    /* 0x073520 */ u16 mItemVersion;        // dItem::BITM version, checked by isExtraGood
    /* 0x073522 */ dTimeStamp_c mSaveTime;  // now, when saving (dSaveData_c::prepareSave); later than now = the clock went back
    /* 0x07352A */ dMuseum_c mMuseum;
    /* 0x07359E */ dSaveMelody_c mVillageMelody; // the town tune
    /* 0x0735AE */ dSvDailyRand::Rand_c mDailyRand;
    /* 0x0735AF */ dItem::dSaveItemRarity_c mItemRarity;
    /* 0x0735B7 */ dSvWeek::Week_c mWeek;  // the game week (d_sv_week)
    /* 0x0735C2 */ union {
        u8 _0735C2;
        RegionInfo_c mRegionInfo;
    };
    /* 0x0735C3 */ u8 mFlags[8];           // town flags, bits 0..63 (SAVE_FLAG_*, isFlag / setFlag / clearFlag)
    /* 0x0735CB */ u8 mPublicWorksDays;     // days left in the public works vote; 0xFF = none
    /* 0x0735CC */ u8 mNewConstruction;     // NEW_CONSTRUCTION_*
    /* 0x0735CD */ s8 mNewConstructionBlockX;
    /* 0x0735CE */ s8 mNewConstructionBlockZ;
    /* 0x0735CF */ u8 _0735CF[0x11];
}; // size 0x735E0
