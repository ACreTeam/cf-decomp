// The WiiConnect24 friend block of the save (dSaveWc24_c, dSaveData_c+0x89CE0). .text 8014D900..8014EB3C.
// See include/game/game/d_sv_wc24.hpp.
#include <game/game/d_sv_wc24.hpp>
#include <game/game/d_friend.hpp>
#include <game/game/d_net_wc24.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_wifi.hpp>
#include <revolution/OS.h>
#include <string.h>

// The send index of a player's first registered friend, before the roster offset (WC24_FRIEND_NUM).
static inline u32 getFriendBase(int player) {
    return player * FRIEND_NUM;
}

// 8014D900
BOOL dSaveWc24_c::isNewKey(const u64 *keys, const u64 *key, int num) {
    for (int i = 0; i < num; i++) {
        if (*key == keys[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

// 8014D944
int dSaveWc24_c::getDaysSince(const dYMD_c *date) {
    dTime_c now;
    OSTicksToCalendarTime(OSGetTime(), &now);
    if (date->isNone()) {
        return -1;
    }
    return date->diffDays(&now, FALSE);
}

// 8014D9B8
void dSaveWc24_c::setToday(dYMD_c *date) {
    dTime_c now;
    OSTicksToCalendarTime(OSGetTime(), &now);
    date->set(&now);
}

// 8014D9F8
void dSaveWc24_c::init() {
    memset(this, 0, sizeof(*this));
    _C5C.clear();
    _C60.clear();
    mBoardSendDate.clear();
    mLetterSendDate.clear();
    _C6C.clear();
    _C70.clear();
    mBoardSendLimit = 1;
    mLetterSendLimit = 1;
    for (u32 i = 0; i < WC24_FRIEND_NUM; i++) {
        mLands[i].clear();
    }
}

// 8014DAB4
void dSaveWc24_c::requestClearSend() {
    mClearSendRequest = 1;
}

// 8014DAC0
void dSaveWc24_c::clearSend() {
    for (u32 i = 0; i < WC24_FRIEND_NUM; i++) {
        mFlags[i].mSend = 0;
    }
    for (int p = 0; p < PLAYER_NUM; p++) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getPlayer(p);
        for (int f = 0; f < FRIEND_NUM; f++) {
            player->mFriends.getFriend(f)->mWc24Flag.mSend = 0;
        }
    }
    mSendCleared = 1;
    mClearSendRequest = 0;
}

// 8014DD14
void dSaveWc24_c::updateClearSend(BOOL busy) {
    if (mClearSendRequest) {
        if (!busy) {
            clearSend();
        }
    } else if (busy) {
        requestClearSend();
    }
}

// 8014DD3C
void dSaveWc24_c::setBoardSendLimit(u32 limit) {
    if (limit == 0) {
        return;
    }
    if (limit > WC24_SEND_LIMIT_MAX) {
        return;
    }
    mBoardSendLimit = limit;
}

// 8014DD54
void dSaveWc24_c::setLetterSendLimit(u32 limit) {
    if (limit == 0) {
        return;
    }
    if (limit > WC24_SEND_LIMIT_MAX) {
        return;
    }
    mLetterSendLimit = limit;
}

// 8014DD6C
u8 dSaveWc24_c::startBoardSendDay() {
    if (mBoardSendLimit > WC24_SEND_LIMIT_MAX) {
        mBoardSendLimit = WC24_SEND_LIMIT_MAX;
    }
    if (getDaysSince(&mBoardSendDate) != 0) {
        mBoardSendCount = 0;
    } else {
        return 0;
    }
    return mBoardSendLimit > mBoardSendCount ? mBoardSendLimit - mBoardSendCount : 0;
}

// 8014DDE0
void dSaveWc24_c::addBoardSendCount(u8 n) {
    mBoardSendCount += n;
    setToday(&mBoardSendDate);
}

// 8014DDF4
BOOL dSaveWc24_c::canSendLetter() {
    if (mLetterSendLimit > WC24_SEND_LIMIT_MAX) {
        mLetterSendLimit = WC24_SEND_LIMIT_MAX;
    }
    if (getDaysSince(&mLetterSendDate) != 0) {
        mLetterSendCount = 0;
    }
    if (mLetterSendCount < mLetterSendLimit) {
        return TRUE;
    }
    return FALSE;
}

// 8014DE64
void dSaveWc24_c::addLetterSendCount() {
    mLetterSendCount++;
    setToday(&mLetterSendDate);
}

// 8014DE78
void dSaveWc24_c::updateRoster() {
    for (u32 i = 0; i < WC24_FRIEND_NUM; i++) {
        u64 key;
        if (dNetWc24::g_wc24.getFriendKey(i, &key)) {
            if (key != mKeys[i]) {
                mKeys[i] = key;
                mLands[i].clear();
                mFlags[i].clearSend();
            }
            mFlags[i].mRegistered = 1;
        } else {
            mKeys[i] = 0;
            mLands[i].clear();
            mFlags[i].clearSend();
            mFlags[i].mRegistered = 0;
        }
    }
}

// 8014DF64
BOOL dSaveWc24_c::isRegistered(u32 idx) const {
    return mFlags[idx].mRegistered;
}

// 8014DF74
int dSaveWc24_c::getKeys(u64 *keys, int max) const {
    int num = 0;
    for (u32 i = 0; i < WC24_FRIEND_NUM; i++) {
        if (isRegistered(i)) {
            keys[num] = mKeys[i];
            num++;
            if (num >= max) {
                return num;
            }
        }
    }
    for (int p = 0; p < PLAYER_NUM; p++) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getPlayer(p);
        for (int f = 0; f < FRIEND_NUM; f++) {
            u64 key = player->mFriends.getFriend(f)->mKey;
            if (key != 0 && isNewKey(keys, &key, num)) {
                keys[num] = key;
                num++;
                if (num >= max) {
                    return num;
                }
            }
        }
    }
    return num;
}

// 8014E0A4
void dSaveWc24_c::setSend(const u64 *key, BOOL send) {
    for (int i = 0; i < WC24_FRIEND_NUM; i++) {
        if (*key == mKeys[i]) {
            if (send) {
                mFlags[i].mSend = 1;
            } else {
                mFlags[i].mSend = 0;
            }
            break;
        }
    }
    dPrivateData_c *player;
    for (int p = 0; p < PLAYER_NUM; p++) {
        player = dPlayerMgr::Mgr_c::getPlayer(p);
        int idx = dWifi::findFriendKey(key, player);
        if (idx >= 0) {
            dFriend_c *fr = player->mFriends.getFriend(idx);
            if (send) {
                fr->mWc24Flag.mSend = 1;
            } else {
                fr->mWc24Flag.mSend = 0;
            }
        }
    }
}

// 8014E1B0
int dSaveWc24_c::getNextSend(u32 *indices, u64 *keys, int max) const {
    int num = 0;
    u32 idx = mNextSend;
    int p;
    int f;
    for (u32 n = 0; n < WC24_SEND_INDEX_NUM; n++) {
        if (idx < WC24_FRIEND_NUM) {
            if (isRegistered(idx) && mFlags[idx].mSend && isNewKey(keys, &mKeys[idx], num)) {
                indices[num] = idx;
                keys[num] = mKeys[idx];
                num++;
                if (num >= max) {
                    return num;
                }
            }
        } else {
            p = idx - WC24_FRIEND_NUM;
            f = p % FRIEND_NUM;
            p /= FRIEND_NUM;
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getPlayer(p);
            u64 key = player->mFriends.getFriend(f)->mKey;
            if (key != 0 && player->mFriends.getFriend(f)->mWc24Flag.mSend && isNewKey(keys, &key, num)) {
                indices[num] = getFriendBase(p) + WC24_FRIEND_NUM + f;
                keys[num] = key;
                num++;
                if (num >= max) {
                    return num;
                }
            }
        }
        if (++idx >= WC24_SEND_INDEX_NUM) {
            idx = 0;
        }
    }
    return num;
}

// 8014E350
void dSaveWc24_c::setNextSend(u32 idx) {
    mNextSend = idx + 1;
    if (mNextSend >= WC24_SEND_INDEX_NUM) {
        mNextSend = 0;
    }
}

// 8014E36C
int dSaveWc24_c::getSendKeys(u32 *indices, u64 *keys, int max) const {
    int num = 0;
    for (u32 i = 0; i < WC24_FRIEND_NUM; i++) {
        if (isRegistered(i) && mFlags[i].mSend) {
            indices[num] = i;
            keys[num] = mKeys[i];
            num++;
            if (num >= max) {
                return num;
            }
        }
    }
    for (int p = 0; p < PLAYER_NUM; p++) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getPlayer(p);
        for (int f = 0; f < FRIEND_NUM; f++) {
            u64 key = player->mFriends.getFriend(f)->mKey;
            if (key != 0 && player->mFriends.getFriend(f)->mWc24Flag.mSend && isNewKey(keys, &key, num)) {
                indices[num] = getFriendBase(p) + WC24_FRIEND_NUM + f;
                keys[num] = key;
                num++;
                if (num >= max) {
                    return num;
                }
            }
        }
    }
    return num;
}

// 8014E4E8
void dSaveWc24_c::setTargets(const u64 *keys, int num) {
    int i = 0;
    for (; i < num; i++) {
        mTargets[i] = keys[i];
    }
    for (; i < WC24_TARGET_NUM; i++) {
        mTargets[i] = 0;
    }
    mTargetSent[0] = 0;
}

// 8014E6A0
void dSaveWc24_c::setTargetSent(const u64 *key) {
    for (u32 i = 0; i < WC24_TARGET_NUM; i++) {
        if (*key == mTargets[i]) {
            mTargetSent[i >> 3] |= 1 << (i & 7);
            return;
        }
    }
}

// 8014E704
int dSaveWc24_c::getSentTargets(u64 *keys, int max) const {
    int num = 0;
    for (u32 i = 0; i < WC24_TARGET_NUM; i++) {
        if (mTargets[i] != 0 && (mTargetSent[i >> 3] & (1 << (i & 7)))) {
            keys[num] = mTargets[i];
            num++;
            if (num >= max) {
                break;
            }
        }
    }
    return num;
}

// 8014E7CC
int dSaveWc24_c::getSendKeys2(u32 *indices, u64 *keys, int max) const {
    return getSendKeys(indices, keys, max);
}

// 8014E7D0
BOOL dSaveWc24_c::hasSend() const {
    for (u32 i = 0; i < WC24_FRIEND_NUM; i++) {
        if (isRegistered(i) && mFlags[i].mSend) {
            return TRUE;
        }
    }
    for (int p = 0; p < PLAYER_NUM; p++) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getPlayer(p);
        for (int f = 0; f < FRIEND_NUM; f++) {
            if (player->mFriends.getFriend(f)->mKey != 0 && player->mFriends.getFriend(f)->mWc24Flag.mSend) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 8014E920
int dSaveWc24_c::findKey(const u64 *key) const {
    for (int i = 0; i < WC24_FRIEND_NUM; i++) {
        if (*key == mKeys[i]) {
            return i;
        }
    }
    for (int p = 0; p < PLAYER_NUM; p++) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getPlayer(p);
        for (int f = 0; f < FRIEND_NUM; f++) {
            if (*key == player->mFriends.getFriend(f)->mKey) {
                return getFriendBase(p) + f;
            }
        }
    }
    return -1;
}

// 8014EB20
void dSaveWc24Flag_c::clear() {
    memset(this, 0, sizeof(*this));
}

// 8014EB2C
void dSaveWc24Flag_c::clearSend() {
    mSend = 0;
}
