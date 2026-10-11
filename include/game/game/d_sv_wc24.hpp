#pragma once

// The WiiConnect24 friend block of the save (dSaveData_c+0x89CE0, 0xC80 bytes): this console's Wii friend
// roster (message board address book, refreshed from the NWC24 friend list), which of them (and of the
// players' registered friends, dFriendList_c) are marked to be sent to, and the daily send limits from the
// downloaded info.bin. Used by d_wifi. Source: src/dol/game/d_sv_wc24.cpp (.text 8014D900..8014EB3C).
// No RTTI: the class and all names are inferred.

#include <types.h>
#include <game/game/d_date.hpp>
#include <game/game/d_land.hpp>

#define WC24_FRIEND_NUM 100 // Wii friend roster entries
#define WC24_TARGET_NUM 8   // keys of one send
#define WC24_SEND_LIMIT_MAX 8
// Send indices: [0, WC24_FRIEND_NUM) for the roster, then WC24_FRIEND_NUM + player * FRIEND_NUM + friend for
// every player's registered friends (228). Needs PLAYER_NUM (d_private_data.hpp) and FRIEND_NUM (d_friend.hpp)
// where it is used: d_friend.hpp includes this header.
#define WC24_SEND_INDEX_NUM (WC24_FRIEND_NUM + PLAYER_NUM * FRIEND_NUM)

// Per-key flags (the roster's mFlags, dFriend_c::mWc24Flag). Bit fields: MWCC allocates them from the top
// bit, so mRegistered is bit 0.
class dSaveWc24Flag_c {
public:
    void clear();                                                   // 8014EB20
    void clearSend();                                               // 8014EB2C

    /* 0x0 */ u8 _bits : 6;
    /* 0x0 */ u8 mSend : 1;                          // marked to be sent to
    /* 0x0 */ u8 mRegistered : 1;                    // in the console's friend roster (roster entries only)
}; // size 0x1

class dSaveWc24_c {
public:
    static BOOL isNewKey(const u64 *keys, const u64 *key, int num); // 8014D900: key is not in keys[0, num)
    // On the real-time clock (OSGetTime, not dTime_c::getCurrent).
    static int getDaysSince(const dYMD_c *date);                    // 8014D944: -1 if unset
    static void setToday(dYMD_c *date);                             // 8014D9B8

    void init();                                                    // 8014D9F8
    void requestClearSend();                                        // 8014DAB4
    void clearSend();                                               // 8014DAC0: unmarks every key
    void updateClearSend(BOOL busy);                                // 8014DD14: clears once a request meets !busy
    void setBoardSendLimit(u32 limit);                              // 8014DD3C: info.bin; 1..WC24_SEND_LIMIT_MAX
    void setLetterSendLimit(u32 limit);                             // 8014DD54: info.bin; 1..WC24_SEND_LIMIT_MAX
    u8 startBoardSendDay();                                         // 8014DD6C: on a new day, resets the count and returns what is left; else 0
    void addBoardSendCount(u8 n);                                   // 8014DDE0
    BOOL canSendLetter();                                           // 8014DDF4
    void addLetterSendCount();                                      // 8014DE64
    void updateRoster();                                            // 8014DE78: from the NWC24 friend list
    BOOL isRegistered(u32 idx) const;                               // 8014DF64
    int getKeys(u64 *keys, int max) const;                          // 8014DF74: roster, then the players' friends
    void setSend(const u64 *key, BOOL send);                        // 8014E0A4
    int getNextSend(u32 *indices, u64 *keys, int max) const;        // 8014E1B0: round robin from mNextSend
    void setNextSend(u32 idx);                                      // 8014E350
    int getSendKeys(u32 *indices, u64 *keys, int max) const;        // 8014E36C
    void setTargets(const u64 *keys, int num);                      // 8014E4E8
    void setTargetSent(const u64 *key);                             // 8014E6A0
    int getSentTargets(u64 *keys, int max) const;                   // 8014E704
    int getSendKeys2(u32 *indices, u64 *keys, int max) const;       // 8014E7CC: same as getSendKeys
    BOOL hasSend() const;                                           // 8014E7D0
    int findKey(const u64 *key) const;                              // 8014E920: roster index, else player * FRIEND_NUM + friend; -1

    /* 0x000 */ u64 mTargets[WC24_TARGET_NUM];       // the keys of the current send (setTargets)
    /* 0x040 */ u64 mKeys[WC24_FRIEND_NUM];          // the roster (0 = free)
    /* 0x360 */ dLandID_c mLands[WC24_FRIEND_NUM];   // their towns (fn_801795C4)
    /* 0xBF8 */ dSaveWc24Flag_c mFlags[WC24_FRIEND_NUM];
    /* 0xC5C */ dYMD_c _C5C;                         // only cleared by init: nothing in the DOL reads it
    /* 0xC60 */ dYMD_c _C60;                         // only cleared by init: nothing in the DOL reads it
    /* 0xC64 */ dYMD_c mBoardSendDate;
    /* 0xC68 */ dYMD_c mLetterSendDate;
    /* 0xC6C */ dYMD_c _C6C;                         // only cleared by init: nothing in the DOL reads it
    /* 0xC70 */ dYMD_c _C70;                         // only cleared by init: nothing in the DOL reads it
    /* 0xC74 */ u32 mNextSend;                       // getNextSend's first send index
    /* 0xC78 */ u8 mBoardSendLimit;                  // sends to the marked keys per day (default 1)
    /* 0xC79 */ u8 mLetterSendLimit;                 // single letters to a registered friend per day (default 1)
    /* 0xC7A */ u8 mBoardSendCount;
    /* 0xC7B */ u8 mLetterSendCount;
    /* 0xC7C */ u8 mTargetSent[WC24_TARGET_NUM / 8]; // bit i: mTargets[i] was sent
    /* 0xC7D */ u8 mClearSendRequest;
    /* 0xC7E */ u8 mSendCleared;
    /* 0xC7F */ u8 _C7F;
}; // size 0xC80
