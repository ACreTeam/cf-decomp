#pragma once

// The whole save file (rvforest.dat, 0x40F340 bytes) as it sits in memory.
// Skeleton: known member types are filled in, everything else is a u8 array
// named by offset. Class and member names are inferred. See
// notes/save_file_layout.txt for where each unknown member's code lives.

#include <types.h>
#include <game/game/d_item.hpp>
#include <game/game/d_dsn.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_model_room.hpp>
#include <game/game/d_police_box.hpp>
#include <game/game/d_recycle_bin.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_museum.hpp>
#include <game/game/d_theater.hpp>
#include <game/game/d_notice.hpp>
#include <game/game/d_save_check.hpp>
#include <game/game/d_save_dl_item.hpp>
#include <game/game/d_save_box.hpp>
#include <game/game/d_bug_off.hpp>
#include <game/game/d_save_stalk_market.hpp>
#include <game/game/d_save_main_field.hpp>
#include <game/game/d_save_visitor_npc.hpp>

#define SAVE_DATA_SIZE 0x40F340
#define SAVE_VERSION 0x5A

// Player option bits, 3 bytes. Lives at dPrivateData_c+0x83E8; the current
// values are cached in a static copy (lbl_8074E6D8) whose third byte marks
// which bits were changed.
struct dSaveOption_c {
    /* 0x0 */ u8 mBit7 : 1;  // fn_8010D944 / fn_8010D950
    /* 0x0 */ u8 mBit65 : 2; // fn_8010D970 / fn_8010D97C
    /* 0x0 */ u8 mBit4 : 1;  // fn_8010D99C / fn_8010D9A8
    /* 0x0 */ u8 mBit3 : 1;  // fn_8010D9C8 / fn_8010D9D4
    /* 0x0 */ u8 mBit2 : 1;
    /* 0x0 */ u8 mBit1 : 1;  // fn_8010D9FC / fn_8010DA08
    /* 0x0 */ u8 mBit0 : 1;  // fn_8010DA28 / fn_8010DA34
    /* 0x1 */ u8 _1;         // 6 after fn_8010D7E8
    // One changed-flag per field above.
    /* 0x2 */ u8 mDirty7 : 1;
    /* 0x2 */ u8 mDirty65 : 1;
    /* 0x2 */ u8 mDirty4 : 1;
    /* 0x2 */ u8 mDirty3 : 1;
    /* 0x2 */ u8 mDirty2 : 1;
    /* 0x2 */ u8 mDirty1 : 1;
    /* 0x2 */ u8 mDirty0 : 1;
    /* 0x2 */ u8 _2_0 : 1;
};

// Option helpers (free functions in the target).
extern "C" {
void fn_8010D7E8(dSaveOption_c *opt);                 // set defaults
void fn_8010D808(dSaveOption_c *opt, u8 value);       // set _1
void fn_8010D810(const dSaveOption_c *opt, void *dst); // copy byte 0 out
void fn_8010D824(dSaveOption_c *opt, const void *src); // copy byte 0 in
void fn_8010D82C(dSaveOption_c *opt);                 // clear the changed flags
void fn_8010D83C(dSaveOption_c *opt);                 // defaults + clear flags
void fn_8010D870(dSaveOption_c *opt);
int fn_8010D944();       // static copy getters/setters
void fn_8010D950(u8 v);
u8 fn_8010D970();
void fn_8010D97C(u8 v);
u8 fn_8010D99C();
void fn_8010D9A8(u8 v);
u8 fn_8010D9C8();
void fn_8010D9D4(u8 v);
int fn_8010D9F4();
u8 fn_8010D9FC();
void fn_8010DA08(u8 v);
u8 fn_8010DA28();
void fn_8010DA34(u8 v);
u8 fn_8010DA54();        // current player's option _1
void fn_8010DA8C(u8 v);
void fn_8010DAC8();      // write changed static bits to the current player
void fn_8010DAFC(dSaveOption_c *opt);
}

// Saved game-clock offset (fn_8014D054 / fn_8014D064 / fn_8014D07C / fn_8014D09C).
struct dSaveTimeOffset_c {
    /* 0x0 */ s64 mOffset; // dTime_c::sOffset
    /* 0x8 */ u8 _8[8];
}; // size 0x10

// The check block at the start of the file is dSaveCheck_c (d_save_check.hpp).

// Rooms and houses (dHomeRoom_c / dHome_c / dHomeList_c) are in d_home.hpp.
// The house ctor/dtor used here are fn_8010EC64 / fn_8010ECC0 (rooms: ctor inlined
// from fn_8010EBCC, dtor 800B4C7C; layers: ctor 800B4BD0 / dtor 800B4C18).

// 0x78 record (ctor fn_8010EB50: Item = none, then fn_8010A758).
struct dSaveRecord78_c {
    /* 0x00 */ dItem::Item mItem;
    /* 0x02 */ u8 _02[0x76];
}; // size 0x78

// The per-player storage boxes (dSaveDesignBox_c, dSaveMailBox_c, dSaveItemBox_c) are in d_save_box.hpp.

// Downloaded pattern with its version (dSaveExtra_c+0x88B00; clear fn_8010F008, set fn_8010F018).
struct dSaveDistPattern_c {
    /* 0x00 */ u32 mVersion;
    /* 0x04 */ u8 _04[0x1C];
    /* 0x20 */ dDesign_c mDesign;
}; // size 0x8A0

// Downloaded letter with its version (dSaveExtra_c+0x893A0; clear fn_8010F060,
// set fn_8010F070 from an ltr_<lang>.bmg message).
struct dSaveDistMail_c {
    /* 0x0 */ u32 mVersion;
    /* 0x4 */ dMail_c mMail;
}; // size 0x394

// 0xC800-byte downloaded blob (dSaveExtra_c+0x8A960; clear fn_8010EEB4, set fn_8010EF10).
struct dSaveDistBlock_c {
    /* 0x0000 */ u32 _0000;
    /* 0x0004 */ u8 mData[0xC800];
}; // size 0xC804

// One downloaded per-language file (clear fn_8010EFD8, set fn_8010EFF0).
struct dSaveDistContent_c {
    /* 0x0000 */ u32 _0000;
    /* 0x0004 */ u8 mData[0x1800];
}; // size 0x1804

// Downloaded files: 10 languages x 6 slots (clear fn_8010EF58; loaded by fn_80177AE0).
struct dSaveDistContentList_c {
    /* 0x00000 */ u32 _00000;
    /* 0x00004 */ dSaveDistContent_c mContents[10][6];
}; // size 0x5A0F4

// Skeleton members whose classes live in unsplit TUs. Their constructors keep the target's C names
// until those TUs are split; the inline ctors reproduce the calls dSaveData_c::create makes.
extern "C" {
void fn_80150140(void *obj); // 80150140
void fn_80150B94(void *obj); // 80150B94
void fn_80117118(void *obj); // 80117118
}
struct dSaveUnk72D1A_c {
    dSaveUnk72D1A_c() { fn_80150140(this); }
    u8 _00[0xC0];
};
struct dSaveUnk72E0A_c {
    dSaveUnk72E0A_c() { fn_80150B94(this); }
    u8 _00[0x6E8];
};
struct dSaveUnk1CE_c {
    dSaveUnk1CE_c() { fn_80117118(this); }
    u8 _00[0x12];
};

// Second half of the file (base+0x735E0), returned by dSaveData_c::getExtra().
// Has its own CRC at +0x20 over +0x24..end (fn_80117078 updates it, fn_801170B0
// checks it, fn_80116900 initializes the block). Holds per-player storage
// (saved patterns, saved letters) and WiiConnect24 / distribution data, mostly
// used by the code at 801769B4..8017B858. The name is inferred.
struct dSaveExtra_c {
    /* 0x000000 */ u8 _000000[4];
    /* 0x000004 */ u8 mNetEnabled;          // gates all distribution handling
    /* 0x000005 */ u8 _000005[0x1B];
    /* 0x000020 */ u32 mChecksum;
    /* 0x000024 */ u8 _000024[0x16];
    /* 0x00003A */ dOutfit_c mOutfit;
    /* 0x000046 */ u8 _000046[0x182];
    /* 0x0001C8 */ dTheater::dSchedule_c mTheater; // the theater's weekly programs
    /* 0x0001CE */ dSaveUnk1CE_c _0001CE;
    /* 0x0001E0 */ dUnkDesignBoard_c mDesignBoard; // fn_80136868 from fn_8010DDE0
    /* 0x000A80 */ dSaveDesignBox_c mSavedPatterns[PLAYER_NUM];
    /* 0x088B00 */ dSaveDistPattern_c mDistPattern;
    /* 0x0893A0 */ dSaveDistMail_c mDistMail;
    /* 0x089734 */ u8 _089734[4];
    /* 0x089738 */ dSaveRecord78_c _089738[4];
    /* 0x089918 */ u8 _089918[4];
    /* 0x08991C */ dSaveRecord78_c _08991C[4];
    /* 0x089AFC */ u8 _089AFC[4];
    /* 0x089B00 */ dSaveRecord78_c _089B00[4];
    /* 0x089CE0 */ u8 _089CE0[0x360];       // received-message manager (fn_8014DD14, fn_8014E920, ...)
    /* 0x08A040 */ dLandID_c mSenderLands[100]; // fn_801795C4
    /* 0x08A8D8 */ u8 _08A8D8[0x64];
    /* 0x08A93C */ u8 _08A93C[6][4];        // {u16, u8, u8}, zeroed by the ctor
    /* 0x08A954 */ u8 _08A954[0xC];
    /* 0x08A960 */ dSaveDistBlock_c _08A960;
    /* 0x097164 */ dSaveDistContentList_c mDistContent;
    /* 0x0F1258 */ dSaveMailBox_c mSavedLetters[PLAYER_NUM];
    /* 0x17FA58 */ dSaveItemBox_c mSavedLetterItems[PLAYER_NUM];
    /* 0x17FF58 */ u8 _17FF58[PLAYER_NUM][0x2738]; // ctor 8013ED24, dtor 8013ED88
    /* 0x189C38 */ u8 _189C38[0x12108];     // ctor 8013F098; used by the mail code at 8010263C
}; // size 0x19BD40

class dSaveData_c {
public:
    static u32 getDLDataOffset();         // 8010DC04: offsetof mDLItems
    static u16 getVersion();              // 8010DC10
    static dSaveData_c *get();            // 8010DC18 (after dSvMgr_c::isFullTransferComplete)
    static dSaveData_c *getRaw();         // 8010DC3C
    BOOL isGood();                        // 8010DC44 (Save_IsGood)
    void initialize();                    // 8010DCB0: initial fill after construction
    void updateChecksum();                // 8010E0A8
    BOOL isExtraGood(int arg);            // 8010E0F8: dSaveExtra_c CRC, buildings CRC, item version
    static dSaveDLItemList_c *getDLData(); // 8010E1B8: mDLItems, after isDLDataTransferComplete
    static dSaveData_c *getTown();        // 8010E1E4 (after isTownTransferComplete)
    static dSaveExtra_c *getExtra();      // 8010E208
    static dSaveData_c *getRaw2();        // 8010E234
    static u32 getSize();                 // 8010E23C
    static void create();                 // 8010E248
    void clear();                         // 8010EA88: memset the whole file
    static void *operator new(size_t, void *p) { return p; }

    // Takes its own copy of the item; dHomeRoom_c::recycleItems only matches with the extra copy.
    static BOOL addToRecycleBin(const dItem::Item &item) {
        dItem::Item copy = item;
        return getTown()->mRecycleBin.add(copy.mId);
    }

    static inline int getSaveRegion() {
        return (dSaveData_c::getRaw()->_0735C2 >> 4) & 0xF;
    }

    static dSaveOption_c sOption;         // 8074E6D8: cached option bits + changed flags
    static dSaveData_c *sSaveData;        // 8074E6E0: the whole save file

    /* 0x000000 */ dSaveCheck_c mHeader;
    /* 0x000020 */ dPrivateData_c mPlayers[PLAYER_NUM];
    /* 0x021B20 */ dAnimalSave_c mAnimals;
    /* 0x05E260 */ dDesign_c _05E260;
    /* 0x05EAE0 */ u8 _05EAE0[0x24];
    /* 0x05EB04 */ u32 _05EB04;             // object, ctor 80149E38; checked against fn_8014B3CC
    /* 0x05EB08 */ u8 _05EB08[0x15C];
    /* 0x05EC64 */ u16 _05EC64;
    /* 0x05EC66 */ u8 _05EC66;
    /* 0x05EC67 */ u8 _05EC67;
    /* 0x05EC68 */ u8 _05EC68[0xE];
    /* 0x05EC76 */ u8 _05EC76;              // bitfield byte, cleared by the ctor
    /* 0x05EC77 */ u8 _05EC77;
    /* 0x05EC78 */ u8 _05EC78[8];
    /* 0x05EC80 */ dDesign_c _05EC80[8];
    /* 0x063080 */ u8 _063080[0x40];
    /* 0x0630C0 */ u8 _0630C0[0x98];        // passed to fn_80146AA4
    /* 0x063158 */ u32 _063158;
    /* 0x06315C */ dTimeStamp_c _06315C[4];
    /* 0x06317C */ u8 _06317C[3];
    /* 0x06317F */ s8 _06317F;              // 0x7F = none
    /* 0x063180 */ u8 _063180[4];
    /* 0x063184 */ dTimeStamp_c _063184;
    /* 0x06318C */ u8 _06318C[0xC];
    /* 0x063198 */ dTimeStamp_c _063198;
    /* 0x0631A0 */ u8 _0631A0[0x5C];
    /* 0x0631FC */ u16 _0631FC;
    /* 0x0631FE */ u8 _0631FE;
    /* 0x0631FF */ u8 _0631FF;
    /* 0x063200 */ dSaveStalkMarket_c mStalkMarket;
    /* 0x063248 */ u8 _063248[0x98];
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
    /* 0x066740 */ u8 _066740[0x422];
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
    /* 0x072CC0 */ u8 _072CC0[0x5A];
    /* 0x072D1A */ dSaveUnk72D1A_c _072D1A; // 8 x 0x18 entries
    /* 0x072DDA */ dPoliceBox_c mPoliceBox;
    /* 0x072DF2 */ dRecycleBin_c mRecycleBin;
    /* 0x072E0A */ dSaveUnk72E0A_c _072E0A;
    /* 0x0734F2 */ dPrivateHost_c mTownHost; // copied to/from dPrivateData_c::mHost
    /* 0x073520 */ u16 mItemVersion;        // dItem::BITM version, checked by isExtraGood
    /* 0x073522 */ dTimeStamp_c _073522;    // set by fn_8010DCF0
    /* 0x07352A */ dMuseum_c mMuseum;
    /* 0x07359E */ dSaveMelody_c mVillageMelody; // the town tune
    /* 0x0735AE */ u8 _0735AE;               // fn_8015384C's object starts here
    /* 0x0735AF */ dItem::dSaveItemRarity_c mItemRarity;
    /* 0x0735B7 */ u8 _0735B7[0xB];         // fn_801541D8
    /* 0x0735C2 */ u8 _0735C2;              // low nibble read by d_item
    /* 0x0735C3 */ u8 _0735C3[8];          // town flags, bits 0..63 (SAVE_FLAG_*, fn_801164D0 / fn_80116510 / fn_80116540)
    /* 0x0735CB */ u8 _0735CB;              // 0xFF = none; days counter
    /* 0x0735CC */ u8 _0735CC[0x14];
    /* 0x0735E0 */ dSaveExtra_c mExtra;
    /* 0x20F320 */ dSaveDLItemList_c mDLItems; // getDLData()
}; // size 0x40F340

// Town flags: 64 bits at dSaveData_c::_0735C3, indexed 0..63. Most indices are not named yet.
enum {
    SAVE_FLAG_TURNIPS_SPOILED = 1, // set when the clock is changed or goes back within the week, cleared each new week (inferred)
};

// Town flag accessors (not split yet; C linkage keeps the target names). Out-of-range indices are ignored.
extern "C" {
BOOL fn_801164D0(dSaveData_c *save, int idx); // 801164D0: is the flag set
void fn_80116510(dSaveData_c *save, int idx); // 80116510: set the flag
void fn_80116540(dSaveData_c *save, int idx); // 80116540: clear the flag
}
