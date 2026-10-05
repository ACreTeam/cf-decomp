// The player's registered friends (dFriendList_c at dPrivateData_c+0, 32 x dFriend_c).
// .text 8011A6C4..8011ACB4, no data. Notes: notes/d_friend.txt.
#include <game/game/d_friend.hpp>
#include <game/sLib/s_crc.hpp>
#include <game/cLib/c_lib.hpp>
#include <cstring>

// Dependencies whose owners are not recovered yet.
extern "C" {
void fn_800E5858(void *info, void *keys, int num);  // unsplit TU 800DE0E4..
BOOL fn_8017BA1C(int idx);                          // WiiConnect24 friend slot in use
u64 fn_8017BAB0(int idx);                           // WiiConnect24 friend key
void fn_8014EB20(u16 *value);                       // unsplit TU 8014BD88..80153818
}

// 8011A6C4
void dFriendList_c::updateChecksum() {
    mChecksum = calcChecksum();
}

// Inlined: isChecksumOK testing this result with an if keeps the compare branchy.
static inline BOOL checkCrc(const dFriendList_c *list) {
    u32 checksum = list->calcChecksum();
    if (checksum == list->mChecksum) {
        return TRUE;
    }
    return FALSE;
}

// 8011A6F4
BOOL dFriendList_c::isChecksumOK() const {
    if (checkCrc(this)) {
        return TRUE;
    }
    return FALSE;
}

// 8011A738
u32 dFriendList_c::calcChecksum() const {
    const u8 *base = (const u8 *)this;
    return sCrc::calcCRC32(base + sizeof(mChecksum),
                           sizeof(dFriendList_c) - sizeof(mChecksum) - ((const u8 *)&mChecksum - base), -1, -1);
}

// 8011A750
void dFriendList_c::clear() {
    clearFriends();
    clearInfo();
    memset(mName, 0, sizeof(mName));
}

// 8011A794
void dFriendList_c::clearFriends() {
    memset(mKeys, 0, sizeof(mKeys));
    dFriend_c *data = mFriends;
    for (int i = 0; i < FRIEND_NUM; i++, data++) {
        data->clear();
    }
}

// 8011A7F4
void dFriendList_c::clearInfo() {
    fn_800E5858(_0004, mKeys, FRIEND_NUM);
}

// 8011A804
void dFriendList_c::set(int idx, const void *key, const dFriend_c *data) {
    memcpy(mKeys[idx], key, sizeof(mKeys[idx]));
    if (data != NULL) {
        mFriends[idx].copy(data);
    } else {
        mFriends[idx].clear();
    }
}

// 8011A88C
void dFriendList_c::clearEntry(int idx) {
    memset(mKeys[idx], 0, sizeof(mKeys[idx]));
    mFriends[idx].clear();
}

// 8011A8E8
void dFriendList_c::set2(int idx, const void *key, const dFriend_c *data) {
    set(idx, key, data);
}

// 8011A8EC
void dFriendList_c::remove(u32 idx) {
    u8 *srcKey = mKeys[idx + 1];
    u8 *dstKey = mKeys[idx];
    dFriend_c *src = &mFriends[idx + 1];
    dFriend_c *dst = &mFriends[idx];
    for (u32 i = idx + 1; i < FRIEND_NUM; i++, dstKey += sizeof(mKeys[0]), dst++) {
        memcpy(dstKey, srcKey, sizeof(mKeys[0]));
        dst->copy(src);
        srcKey += sizeof(mKeys[0]);
        src++;
    }
    clearEntry(FRIEND_NUM - 1);
}

// 8011A998
void dFriendList_c::removeFriend(u32 idx) {
    dFriend_c *src = &mFriends[idx + 1];
    dFriend_c *dst = &mFriends[idx];
    for (u32 i = idx + 1; i < FRIEND_NUM; i++, dst++) {
        dst->copy(src);
        src++;
    }
    mFriends[FRIEND_NUM - 1].clear();
}

// 8011AA20
int dFriendList_c::getState(int idx) {
    if (!fn_8017BA1C(idx)) {
        return 0;
    }
    if (mFriends[idx].mPID.isValid()) {
        return 1;
    }
    if (fn_8017BAB0(idx) == 0) {
        return 2;
    }
    return 3;
}

// 8011AAA8
void dFriendList_c::setName(const wchar_t *name) {
    mName[0x20] = 0;
    memcpy(mName, name, 0x40);
}

// 8011AABC
void dFriendList_c::update(int idx, const dPersonalID_c *pid, const wchar_t *message) {
    if (*pid != mFriends[idx].mPID) {
        mFriends[idx].mPID = *pid;
        if (message[0] != 0 && mFriends[idx].mMessage[0] == 0) {
            cLib::memCpy(mFriends[idx].mMessage, message, 0x40);
        }
    }
}

// ---------------------------------------------------------------------------
// dFriend_c

// 8011AC58
void dFriend_c::clear() {
    mKey = 0;
    mPID.clear();
    fn_8014EB20(&_34);
    memset(mMessage, 0, sizeof(mMessage));
}

// 8011ACAC
void dFriend_c::copy(const dFriend_c *other) {
    memcpy(this, other, sizeof(dFriend_c));
}
