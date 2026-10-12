#pragma once

// d_sv_villager_move (DOL TU d_sv_villager_move.cpp, .text 8014EB3C..8014F284): this town's news as its
// friends' towns get it over WiiConnect24 (dSaveTown_c::mVillagerMove, +0x640C8; d_wifi sends it inside a
// dWifiTownMsg_c and dWifi::receiveTown takes over the parts mFlags names). It carries what a villager moving
// to (or visiting from) this town would talk about: this town as another town sees it (mOtherTown, set when a
// villager moves away, d_animal), its shop, public works, jinx, Catherine and outfit picks, the HRA's next
// model room and the auction's downloaded item. No RTTI: names inferred (the TU name is the split's).

#include <types.h>
#include <game/game/d_land.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_animal_id.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_model_room.hpp>
#include <game/game/d_save_dl_item.hpp>
#include <game/game/d_sv_activity.hpp>
#include <game/game/d_save_catherine.hpp>

// Another town the residents know about (4 slots, dSaveTown_c::mOtherTowns). The villager dialog in d_npc
// (8003B3B4) talks about it: its land, one of its players, a villager, and its shop size and public works
// (the same things getPublicWorks reads for this town).
struct dSaveOtherTown_c {
    /* 0x000 */ dLandID_c mLand;
    /* 0x016 */ dPersonalID_c mPlayer;
    /* 0x042 */ dAnmPersonalID_c mVillager; // one of its villagers
    /* 0x102 */ u8 mImpression;         // < 0x31: shown as STR_Impress + 1
    /* 0x103 */ u8 _103;                // the town assessment (fgMngProc_assessLiveTown; 4 after init)
    /* 0x104 */ u8 mValid : 1;          // getRandomOtherTown
    /* 0x104 */ u8 mShopStage : 2;      // compared with dSaveShop_c's stage
    // The newest public work (at most one is set; dSvVillagerMove::Info_c::decPublicWorksDays clears them).
    /* 0x104 */ u8 mHasBridge : 1;      // PUBLIC_WORKS_BRIDGE
    /* 0x104 */ u8 mHasFountain : 1;
    /* 0x104 */ u8 mHasWindmill : 1;
    /* 0x104 */ u8 mHasLighthouse : 1;
    /* 0x104 */ u8 _104_0 : 1;
}; // size 0x106 (struct copies skip the padding byte at 0x105)

namespace dSvVillagerMove {

#define VILLAGER_MOVE_VERSION 10
enum {
    VILLAGER_MOVE_FLAG_CATHERINE = 0x01,    // mCatherine* / mPlayerCategory / mPlayerSub
    VILLAGER_MOVE_FLAG_OUTFIT = 0x02,       // mOutfit
    VILLAGER_MOVE_FLAG_MODEL_ROOM = 0x04,   // mModelRoomCandidate (setModelRoom)
    VILLAGER_MOVE_FLAG_JINX = 0x08,         // mJinx
    VILLAGER_MOVE_FLAG_PLAYER_BITS = 0x10,  // mPlayerBits
    VILLAGER_MOVE_FLAG_DL_ITEM = 0x20,      // mDLItem (dSvAuc_c::pickDLItem)
    VILLAGER_MOVE_FLAG_PLAYER_VALUE = 0x40, // mPlayerValue
    VILLAGER_MOVE_FLAG_OTHER_TOWN = 0x80,   // mOtherTown
};

enum {
    CATHERINE_PLAYER_NONE = 7,  // mCatherinePlayer after init
    PLAYER_VALUE_NUM = 10,      // mPlayerValue is < this
    PUBLIC_WORKS_NEWS_DAYS = 10, // mPublicWorksDays after a public work is built
    ASSESSMENT_INITIAL = 4,     // mOtherTown._103 after init
    IMPRESSION_INITIAL = 0x31,  // mOtherTown.mImpression after init
};

// Holds the auction's featured downloaded item: one object, since the target reaches the flag byte and
// mDLItem from one base.
struct Info_c {
    void init();                                        // 8014EB3C
    void clearOutfitFlag();                             // 8014EBE8: (d_wifi, after sending it)
    void setCatherine(int player, u32 category, u32 sub); // 8014EBF8
    void setPlayerBit(int player, BOOL on);             // 8014EC40
    BOOL isPlayerBit(int player) const;                 // 8014EC84
    void setOutfit(const dOutfit_c *outfit);            // 8014ECA4
    BOOL getOutfit(dOutfit_c *outfit) const;            // 8014ECE0: FALSE if mOutfit is not valid
    void setPlayerValue(int player, u8 value);          // 8014ED24: also records the player in dSaveExtra_c
    int getPlayerValue(int player) const;               // 8014EE28: 0 if not < PLAYER_VALUE_NUM
    // A villager moved away from this town (d_animal): this town, the player and the villager.
    void setOtherTown(const dPersonalID_c *player, const dAnmPersonalID_c *villager, u8 impression); // 8014EE50
    void setAssessment(u8 assessment);                  // 8014EFC0
    void setShopStage(u8 stage);                        // 8014F030
    void setBridgeBuilt();                              // 8014F0A4
    void setFountainBuilt();                            // 8014F10C
    void setWindmillBuilt();                            // 8014F178
    void setLighthouseBuilt();                          // 8014F1E4
    void decPublicWorksDays(int days);                  // 8014F248: the public work news lapses at 0

    // The house rating's model room candidate (d_hr fn_800B0ABC).
    void setModelRoom(const dSvMdlRm_c &room) {
        mModelRoomCandidate = room;
        mFlags |= VILLAGER_MOVE_FLAG_MODEL_ROOM;
    }

    /* 0x000 */ u8 mVersion;              // VILLAGER_MOVE_VERSION; a received one must match
    /* 0x001 */ u8 mFlags;                // VILLAGER_MOVE_FLAG_*: the parts the receiver takes over
    /* 0x002 */ dOutfit_c mOutfit;
    /* 0x00E */ u8 mPlayerBits;           // one bit per player
    /* 0x00F */ s8 mCatherinePlayer;      // the player, category and sub-category for
    /* 0x010 */ s8 mCatherineCategory;    //   dSaveCatherine_c::set (-1 = none)
    /* 0x011 */ s8 mCatherineSub;
    /* 0x012 */ s8 mPlayerCategory[4];    // per player, for dSaveCatherine_c::count (-1 = none)
    /* 0x016 */ s8 mPlayerSub[4];
    /* 0x01A */ u8 mPlayerValue[4];       // per player, < PLAYER_VALUE_NUM
    /* 0x01E */ dSaveOtherTown_c mOtherTown; // this town as another town sees it
    /* 0x124 */ dSaveJinx_c mJinx;
    /* 0x126 */ u8 mPublicWorksDays;      // days left to tell about the newest public work
    /* 0x127 */ u8 _127;
    /* 0x128 */ dSvMdlRm_c mModelRoomCandidate; // the HRA's next model room (dHR::selectModelRooms)
    /* 0x674 */ dSaveDLItem_c mDLItem;
}; // size 0x2674

} // namespace dSvVillagerMove
