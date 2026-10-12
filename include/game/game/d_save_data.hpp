#pragma once

// The whole save file (rvforest.dat, 0x40F340 bytes) as it sits in memory: the town
// (dSaveTown_c, d_save_town.hpp), then dSaveExtra_c and the downloaded items.
// Skeleton: known member types are filled in, everything else is a u8 array
// named by offset. Class and member names are inferred. See
// notes/save_file_layout.txt for where each unknown member's code lives.

#include <types.h>
#include <game/game/d_sv_wc24.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_theater.hpp>
#include <game/game/d_save_box.hpp>
#include <game/game/d_post_box.hpp>
#include <game/game/d_sv_extra_header.hpp>
#include <game/game/d_sv_lang_flag.hpp>

#define SAVE_DATA_SIZE 0x40F340
#define SAVE_VERSION 0x5A

// A player's options, 3 bytes: the option bits, the voice chat volume and a changed flag per bit. Each player's
// are at dPrivateData_c+0x83E8 (dPrivateData_c::getOption); the current ones are worked on in a static copy
// (dSaveData_c::sOption) and written back to the player where the changed flags are set (d_save_option).
struct dSaveOption_c {
    BOOL isKana() const { return mKana; }
    BOOL isVoiceChat() const { return mVoiceChat; } // d_obj_voice

    /* 0x0 */ u8 mKana : 1;       // Japanese text shows the kana readings (dScript::Word_c::procRubyJP)
    /* 0x0 */ u8 mVoiceMode : 2;  // the talk voices: 0 the speaker's own mode, 1 / 2 a fixed one (d_msg_seq)
    /* 0x0 */ u8 mKeySe : 1;      // lower case keys use the upper case key SE (d_letter, d_kbd)
    /* 0x0 */ u8 mNoTutorial : 1; // dObjTutorial_c::isEnabled when clear
    /* 0x0 */ u8 mBit2 : 1;       // read by nothing (its getter fn_8010D9F4 returns 0)
    /* 0x0 */ u8 mVoiceChat : 1;  // voice chat on (d_net_mgr, d_net_sync, d_obj_voice)
    /* 0x0 */ u8 mVoiceSound : 1; // voice chat sound on (dVoiceChat::setMute when clear)
    /* 0x1 */ u8 mVoiceVolume;    // voice chat volume (dNetMgr_c::setVolume), 6 by default
    // One changed flag per bit above.
    /* 0x2 */ u8 mKanaChanged : 1;
    /* 0x2 */ u8 mVoiceModeChanged : 1;
    /* 0x2 */ u8 mKeySeChanged : 1;
    /* 0x2 */ u8 mNoTutorialChanged : 1;
    /* 0x2 */ u8 mBit2Changed : 1;
    /* 0x2 */ u8 mVoiceChatChanged : 1;
    /* 0x2 */ u8 mVoiceSoundChanged : 1;
    /* 0x2 */ u8 _2_0 : 1;
};

// The save file's name and size (d_save_data .sdata / .sdata2; used by d_nand and d_sv_mgr).
extern "C" {
extern const char *lbl_8074B010; // 8074B010: the save file's name (804EE330)
extern const int lbl_80750AD0;   // 80750AD0: the save file's size (0x40F340, SAVE_DATA_SIZE)
}

// The options (src/dol/game/d_save_option.cpp). The get / set functions work on the static copy and mark it changed.
namespace dSaveOption {
void setDefaults(dSaveOption_c *opt);                  // 8010D7E8
void setVoiceVolume(dSaveOption_c *opt, u8 volume);    // 8010D808
void getBits(const dSaveOption_c *opt, void *dst);     // 8010D810: the option bits (byte 0)
void setBits(dSaveOption_c *opt, const void *src);     // 8010D824
void clearChanged(dSaveOption_c *opt);                 // 8010D82C
void reset(dSaveOption_c *opt);                        // 8010D83C: defaults, nothing changed
void setNewPlayerDefaults(dSaveOption_c *opt);         // 8010D870: kana off unless a player of the town has it on
int getKana();                                         // 8010D944
void setKana(u8 on);                                   // 8010D950
int getVoiceMode();                                    // 8010D970
void setVoiceMode(u8 mode);                            // 8010D97C
BOOL getKeySe();                                       // 8010D99C
void setKeySe(u8 on);                                  // 8010D9A8
BOOL isTutorialOff();                                  // 8010D9C8
void setTutorialOff(u8 off);                           // 8010D9D4
BOOL getVoiceChat();                                   // 8010D9FC
void setVoiceChat(u8 on);                              // 8010DA08
BOOL getVoiceSound();                                  // 8010DA28
void setVoiceSound(u8 on);                             // 8010DA34
u8 getPlayerVoiceVolume();                             // 8010DA54: the current player's (0 without one)
void setPlayerVoiceVolume(u8 volume);                  // 8010DA8C
void storeToCurrentPlayer();                           // 8010DAC8
void store(dSaveOption_c *opt);                        // 8010DAFC: the changed bits into opt
} // namespace dSaveOption

extern "C" {
int fn_8010D9F4();  // 8010D9F4: returns 0 (probably mBit2's getter, stubbed; no callers)
int fn_8010DBFC();  // 8010DBFC: returns 0 (d_s_boot_static)
}

// The check block at the start of the file is dSaveCheck_c (d_save_check.hpp).

// Rooms and houses (dHomeRoom_c / dHome_c / dHomeList_c) are in d_home.hpp.
// The house ctor/dtor used here are weak copies (8010EC64 / 8010ECC0; rooms: ctor 8010EBCC,
// dtor 800B4C7C; layers: ctor 800B4BD0 / dtor 800B4C18).

// The per-player storage boxes (dSaveDesignBox_c, dSaveMailBox_c, dSaveItemBox_c) are in d_save_box.hpp.

// Downloaded pattern with its version (dSaveExtra_c+0x88B00).
struct dSaveDistPattern_c {
    void clear();                                         // 8010F008
    void set(const void *data, u32 size, u32 version);   // 8010F018: from a downloaded design.bin of 0x2000 bytes

    /* 0x00 */ u32 mVersion;
    /* 0x04 */ u8 _04[0x1C];
    /* 0x20 */ dDesign_c mDesign;
}; // size 0x8A0

// Downloaded letter with its version (dSaveExtra_c+0x893A0), from an ltr_<lang>.bmg message.
struct dSaveDistMail_c {
    void clear();                                                      // 8010F060
    void set(const void *bmg, dItem::Item present, u32 version);       // 8010F070: present unless ITEM_ID_NONE
    void setReceived(int player);                                      // 8010F114
    void clearReceived(int player);                                    // 8010F11C

    /* 0x0 */ u32 mVersion;
    /* 0x4 */ dMail_c mMail;
}; // size 0x394

// Downloaded NG-word patterns, up to 0xC800 bytes (dSaveExtra_c::mDistNgWords). dPrcMng_c prefers them to the
// disc's /PRC/<region>.bin.
struct dSaveDistBlock_c {
    void clear();                        // 8010EEB4: back to dPrcMng_c's patterns
    void set(const void *data, u32 size); // 8010EF10: size < 0xC800

    /* 0x0000 */ s32 mSize; // 0 = none
    /* 0x0004 */ u8 mData[0xC800];
}; // size 0xC804

// One downloaded per-language file.
struct dSaveDistContent_c {
    void clear();                        // 8010EFD8
    void set(const void *data, u32 size); // 8010EFF0: size < 0x1800

    /* 0x0000 */ u32 _0000;
    /* 0x0004 */ u8 mData[0x1800];
}; // size 0x1804

// Downloaded files: 10 languages x 6 slots (loaded by fn_80177AE0).
struct dSaveDistContentList_c {
    void clear(); // 8010EF58

    /* 0x00000 */ u32 _00000;
    /* 0x00004 */ dSaveDistContent_c mContents[10][6];
}; // size 0x5A0F4

// Two flags of the post office's letters to other towns (d_wifi updateDaily, fn_8017A484).
struct dSavePostSend_c {
    void set(const dPersonalID_c *to); // 8014F8FC (d_sv_activity): to, from the current player, pending

    /* 0x00 */ u8 mSending; // 1 while fn_8017A484 runs
    /* 0x01 */ u8 mPending; // fn_8017A484 is called on new days until it succeeds
    /* 0x02 */ dPersonalID_c mTo;     // a registered friend of a player of this town
    /* 0x2E */ dPersonalID_c mSender; // goes to that player's dPrivateData_c::mLastWc24Sender (receivePostPid)
}; // size 0x5A: the whole block is what dWifi::sendPostPid sends

// Second half of the file (base+0x735E0), returned by dSaveData_c::getExtra().
// Has its own CRC at +0x20 over +0x24..end (updateChecksum updates it, isChecksumOK
// checks it, init initializes the block). Holds per-player storage
// (saved patterns, saved letters) and WiiConnect24 / distribution data, mostly
// used by the code at 801769B4..8017B858. The name is inferred.
struct dSaveExtra_c;

#define RUMOR_NUM 8 // dSaveExtra_c::mRumorPlayers

struct dSaveExtra_c {
    void init();                                  // 80116900: a new save
    void clearDistMailReceived(int player);       // 80116A68
    // 80116A74: a random rumor (a valid player with a kind) other than exclude; FALSE when there is none
    BOOL pickRumor(dPersonalID_c *out, u32 *kind, const dPersonalID_c *exclude) const;
    void addRumor(dPersonalID_c player, int kind); // 80116D08: on dSaveData_c::getExtra(), not this
    void setupNew();                              // 80117044: the header and the theater schedule
    void updateChecksum();                        // 80117078: the header's and mChecksum
    BOOL isChecksumOK(int);                       // 801170B0
    u32 calcChecksum() const;                     // 801170F4: everything after mChecksum

    dSvAucList_c *getAucListings() { return (dSvAucList_c *)_089734; }
    dSvAucList_c *getAucBids() { return (dSvAucList_c *)_089918; }
    dSvAucList_c *getAucSales() { return (dSvAucList_c *)_089AFC; }
    // An inline result: keeps the block's address in a register (d_wifi updateRoster, the sends).
    dSaveWc24_c *getWc24() { return &mWc24; }
    void setVoiceSetting(u16 value) { mVoiceSetting = value; }

    /* 0x000000 */ dSvExtraHeader::Header_c mHeader; // its own CRC; mNetEnabled gates all distribution handling
    /* 0x000020 */ u32 mChecksum;           // CRC32 of everything after it (calcChecksum)
    /* 0x000024 */ dLandID_c mOutfitLand;  // the town of mOutfit (d_wifi receiveTown)
    /* 0x00003A */ dOutfit_c mOutfit;      // an outfit received from a friend town (d_wifi receiveTown)
    // Players of friend towns the villagers talk about (d_npc_talk_town "Town_Rumor"), newest first, with what
    // they are known for (dSvVillagerMove::Info_c::mPlayerValue, 1..9; 0 = none).
    /* 0x000046 */ dPersonalID_c mRumorPlayers[RUMOR_NUM];
    /* 0x0001A6 */ u8 _0001A6[2];
    /* 0x0001A8 */ u32 mRumorKinds[RUMOR_NUM];
    /* 0x0001C8 */ dTheater::dSchedule_c mTheater; // the theater's weekly programs
    /* 0x0001CE */ dSvLangFlag::Flag_c mLangFlags; // the downloaded villager messages still to be told
    /* 0x0001D2 */ u8 _0001D2[0xE];
    /* 0x0001E0 */ dRelayDesign_c mDesignBoard; // validated by dSaveData_c::updateOnSave
    /* 0x000A80 */ dSaveDesignBox_c mSavedPatterns[PLAYER_NUM];
    /* 0x088B00 */ dSaveDistPattern_c mDistPattern;
    /* 0x0893A0 */ dSaveDistMail_c mDistMail;
    // Auction records for other towns (d_sv_auc): each header + items is a dSvAucList_c (getAucListings
    // / getAucBids / getAucSales). Kept as plain members.
    /* 0x089734 */ u8 _089734[4];
    /* 0x089738 */ dSvAucItem_c mAucListings[4];
    /* 0x089918 */ u8 _089918[4];
    /* 0x08991C */ dSvAucItem_c mAucBids[4];
    /* 0x089AFC */ u8 _089AFC[4];
    /* 0x089B00 */ dSvAucItem_c mAucSales[4];
    /* 0x089CE0 */ dSaveWc24_c mWc24;          // the WiiConnect24 friend roster and send state (d_sv_wc24)
    /* 0x08A960 */ dSaveDistBlock_c mDistNgWords;
    /* 0x097164 */ dSaveDistContentList_c mDistContent;
    /* 0x0F1258 */ dSaveMailBox_c mSavedLetters[PLAYER_NUM];
    /* 0x17FA58 */ dSaveItemBox_c mSavedLetterItems[PLAYER_NUM];
    /* 0x17FF58 */ dPlayerPostBox_c mPlayerPostBoxes[PLAYER_NUM]; // the players' house mailboxes (d_post_box)
    /* 0x189C38 */ dPostBox_c mPostBox;     // the letters held at the post office (d_post_box)
    // dPostBox_c+0x120A2 (inside its 0x120FC; the target reaches it from the extra block's base).
    /* 0x19BCDA */ dSavePostSend_c mPostSend;
    /* 0x19BD34 */ u16 mVoiceSetting;        // voice chat: dVoiceChat::fn_80171C08 accepted it (d_net_mgr)
    /* 0x19BD36 */ u16 mVoiceLevel;          // voice chat: given to dVoiceChat::fn_80171C10 (d_net_mgr)
    /* 0x19BD38 */ u8 _19BD38[8];
}; // size 0x19BD40


class dSaveData_c : public dSaveTown_c {
public:
    static u32 getDLDataOffset();         // 8010DC04: offsetof mDLItems
    static u16 getVersion();              // 8010DC10
    static dSaveData_c *get();            // 8010DC18 (after dSvMgr_c::isFullTransferComplete)
    static dSaveData_c *getRaw();         // 8010DC3C
    BOOL isGood();                        // 8010DC44 (Save_IsGood)
    void initialize();                    // 8010DCB0: initial fill after construction
    void prepareSave();                   // 8010DCF0: the saving player, the option bits and mSaveTime
    void updateOnSave();                  // 8010DDE0: catches up the tourneys, the design board and the player
    static void refreshTown();            // 8010DEF8: the field, jinx, events and clock of the town (no callers)
    void setupAfterLoad();                // 8010DFC0: the field, events and records of the loaded town
    void updateChecksum();                // 8010E0A8
    BOOL isExtraGood(int arg);            // 8010E0F8: dSaveExtra_c CRC, buildings CRC, item version
    static dSaveDLItemList_c *getDLData(); // 8010E1B8: mDLItems, after isDLDataTransferComplete
    static dSaveTown_c *getTown();        // 8010E1E4 (after isTownTransferComplete)
    // Room `room` of player house `home`.
    static dHomeRoom_c *getHomeRoom(const int &home, int room) {
        dSaveTown_c *town = getTown();
        return town->mHomes.getHome(home & 3)->getRoom(room & 3);
    }
    static dSaveExtra_c *getExtra();      // 8010E208
    static dSaveData_c *getRaw2();        // 8010E234
    static u32 getSize();                 // 8010E23C
    static void create();                 // 8010E248
    void clear();                         // 8010EA88: memset the whole file
    // 8010EA98: player idx becomes a new player, with an id no other player has (no callers)
    static dPrivateData_c *setupPlayer(const wchar_t *name, u8 gender, u32 idx);

    static void *operator new(size_t, void *p) { return p; }

    // Takes its own copy of the item; dHomeRoom_c::recycleItems only matches with the extra copy.
    static BOOL addToRecycleBin(const dItem::Item &item) {
        dItem::Item copy = item;
        return getTown()->mRecycleBin.add(copy.mId);
    }

    static dSaveExtra_c *getRawExtra() { return &getRaw()->mExtra; }
    // Distribution / WiiConnect24 data handling is on (dSaveExtra_c::mHeader.mNetEnabled). Returns bool: d_wifi
    // onReceive tests the converted value.
    static bool isNetEnabled() { return getRaw()->mExtra.mHeader.mNetEnabled; }
    // The town's id and its villagers. Inline results (not plain expressions): their temps get the
    // registers d_npc_talk_reaction msgFirstMeeting needs.
    static dLandID_c *getTownLand() { return &getTown()->mLandID; }
    // Inline results: a constant stored through them is loaded after the call (d_wifi updateDaily).
    static dSvVillagerMove::Info_c *getVillagerMove() { return &getTown()->mVillagerMove; }
    static dRelayDesign_c *getDesignBoard() { return &getExtra()->mDesignBoard; }
    static dSavePostSend_c *getPostSend() { return &getExtra()->mPostSend; }
    static dSaveWc24_c *getWc24() { return &getExtra()->mWc24; }
    static dAnimalBlock_c *getAnimalBlock() { return &getRaw()->mAnimals.mTown; }

    // The region the town was made in (getRegion() value); dPrcMng_c ignores downloaded patterns otherwise.
    static int getTownRegion() { return dSaveData_c::getRaw()->_0735C2 & 0xF; }

    // The town's language (LANGUAGE_*); events and holidays follow it.
    static inline int getTownLanguage() {
        return (dSaveData_c::getRaw()->_0735C2 >> 4) & 0xF;
    }

    static dSaveOption_c sOption;         // 8074E6D8: cached option bits + changed flags
    static dSaveData_c *sSaveData;        // 8074E6E0: the whole save file

    // 0x000000..0x0735E0: dSaveTown_c
    /* 0x0735E0 */ dSaveExtra_c mExtra;
    /* 0x20F320 */ dSaveDLItemList_c mDLItems; // getDLData()
}; // size 0x40F340

inline dPostBox_c *dPostBox_c::get() {
    return &dSaveData_c::getExtra()->mPostBox;
}

