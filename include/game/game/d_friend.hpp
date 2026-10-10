#pragma once

// The player's registered friends: the CRC'd block at the start of dPrivateData_c (0x1120 bytes).
// Source: src/dol/game/d_friend.cpp (.text 8011A6C4..8011ACB4). All names are inferred.

#include <types.h>
#include <game/game/d_personal_id.hpp>

#define FRIEND_NUM 32

// One registered friend. 0x78 bytes.
class dFriend_c {
public:
    void clear();                                                    // 8011AC58
    void copy(const dFriend_c *other);                               // 8011ACAC

    /* 0x00 */ u64 mKey;            // WiiConnect24 friend key (0 = unregistered)
    /* 0x08 */ dPersonalID_c mPID;  // the friend's player and town
    /* 0x34 */ u16 _34;             // cleared by fn_8014EB20
    /* 0x36 */ wchar_t mMessage[0x21];
}; // size 0x78

// The whole list, at dPrivateData_c+0.
class dFriendList_c {
public:
    const dFriend_c *getFriend(u32 idx) const { return &mFriends[idx]; }
    void updateChecksum();                                           // 8011A6C4
    BOOL isChecksumOK() const;                                        // 8011A6F4
    u32 calcChecksum() const;                                         // 8011A738
    void clear();                                                    // 8011A750
    void clearFriends();                                             // 8011A794
    void clearInfo();                                                // 8011A7F4: fn_800E5858(_04, mKeys, FRIEND_NUM)
    void set(int idx, const void *key, const dFriend_c *data);       // 8011A804: NULL data clears the entry
    void clearEntry(int idx);                                        // 8011A88C
    void set2(int idx, const void *key, const dFriend_c *data);      // 8011A8E8: same as set
    void remove(u32 idx);                                            // 8011A8EC: shifts the later keys and friends down
    void removeFriend(u32 idx);                                      // 8011A998: shifts the later friends down
    int getState(int idx);                                           // 8011AA20: 0 none, 1 known, 2 no key, 3 key only
    void setName(const wchar_t *name);                               // 8011AAA8
    void update(int idx, const dPersonalID_c *pid, const wchar_t *message); // 8011AABC

    /* 0x0000 */ u32 mChecksum; // CRC32 over 0x0004..0x1120
    /* 0x0004 */ u8 _0004[0x40];
    /* 0x0044 */ u8 mKeys[FRIEND_NUM][0xC];
    /* 0x01C8 */ dFriend_c mFriends[FRIEND_NUM];
    /* 0x10C8 */ wchar_t mName[0x21];
    /* 0x110A */ u8 _110A[0x16];
}; // size 0x1120
