// d_wifi_friend.inc: #included at the end of d_wifi.cpp (8017B894..8017C4EC). With -sym on its code lands after
// d_wifi's __sinit (8017B858) and its data after d_wifi's (.bss 806215E8, .sbss 8074E958): both as in the target.
// The current player's Wi-Fi Connection friend list; see include/game/game/d_wifi.hpp (namespace dWifi).
namespace dWifi {

static dFriend_c sRegisterFriend; // 806215E8: the friend data reqRegister's result is applied with
static int sReqState;             // 8074E958: ReqState_e
static u8 sChanged;               // 8074E95C: the friend list changed (saved by updateSave)
static u8 sSaving;                // 8074E95D

// 8017B894
DWCFriendData *getFriendData(int idx) {
    return &dPlayerMgr::Mgr_c::getCurrentPlayer()->mFriends.mKeys[idx];
}

// 8017B8CC
void onRegistered(int idx) {
    if (idx >= 0) {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->mFriends.mFriends[idx].copy(&sRegisterFriend);
        if (sRegisterFriend.mKey != 0) {
            sChanged = TRUE;
        }
    }
    sReqState = REQ_REGISTERED;
}

// 8017B93C
void onRemoved(int idx) {
    if (idx >= 0) {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->mFriends.removeFriend(idx);
    }
    sReqState = REQ_REMOVED;
}

// 8017B980
DWCUserData *getUserData() {
    return &dPlayerMgr::Mgr_c::getCurrentPlayer()->mFriends.mUserData;
}

// 8017B9A4
u64 getMyFriendKey() {
    if (dNetMgr_c::sInstance->mState > NET_MGR_STATE_SETUP) {
        return dNetMgr_c::getInfo1A10();
    }
    return dNetMgr_c::getMyFriendKey(getUserData());
}

// 8017B9E0
int getMyProfileId() {
    if (dNetMgr_c::sInstance->mState > NET_MGR_STATE_SETUP) {
        return dNetMgr_c::getInfo1A28();
    }
    return dNetMgr_c::getMyProfileId(getUserData());
}

// 8017BA1C
BOOL isFriendValid(int idx) {
    if (dNetMgr_c::sInstance->mState > NET_MGR_STATE_SETUP) {
        return dNetMgr_c::getFriendFlag(idx);
    }
    return dNetMgr_c::isFriendDataValid(getFriendData(idx));
}

// 8017BA58
int getFriendNum() {
    int num = 0;
    for (u32 i = 0; i < FRIEND_NUM; i++) {
        if (isFriendValid(i)) {
            num++;
        }
    }
    return num;
}

// 8017BAB0
u64 getFriendKey(int idx) {
    if (dNetMgr_c::sInstance->mState > NET_MGR_STATE_SETUP) {
        return dNetMgr_c::getFriend00(idx);
    }
    return dNetMgr_c::getFriendKey(getFriendData(idx));
}

// 8017BAEC
int getFriendProfileId(int idx) {
    if (dNetMgr_c::sInstance->mState > NET_MGR_STATE_SETUP) {
        return dNetMgr_c::getFriend08(idx);
    }
    return dNetMgr_c::getProfileId(getUserData(), getFriendData(idx));
}

// 8017BB3C
int findFreeFriend() {
    for (u32 i = 0; i < FRIEND_NUM; i++) {
        if (!isFriendValid(i)) {
            return i;
        }
    }
    return -1;
}

// 8017BB8C
int findMyself(const DWCUserData *userData) {
    DWCFriendData data;
    dNetMgr_c::getMyFriendData(userData, &data);
    return findFriendData(&data);
}

// 8017BBB8
int findUser(const DWCUserData *userData) {
    if (dNetMgr_c::sInstance->mState <= NET_MGR_STATE_SETUP) {
        return findMyself(userData);
    }
    if (sReqState == REQ_FIND_USER) {
        int result;
        if (dNetMgr_c::sInstance->mReq.takeResult(dNetFriend::NET_FRIEND_REQ_FIND_USER, &result)) {
            sReqState = REQ_NONE;
            return result;
        }
    } else if (sReqState == REQ_NONE && dNetMgr_c::reqFindUser(userData)) {
        sReqState = REQ_FIND_USER;
    }
    return WIFI_PENDING;
}

// 8017BC40
int findFriendData(const DWCFriendData *data) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    const DWCUserData *userData = &player->mFriends.mUserData;
    const DWCFriendData *keys = player->mFriends.mKeys;
    for (u32 i = 0; i < FRIEND_NUM; i++) {
        if (dNetMgr_c::isSameFriend(userData, &keys[i], data)) {
            return i;
        }
    }
    return -1;
}

// 8017BCC4
int findFriendDataReq(const DWCFriendData *data) {
    if (dNetMgr_c::sInstance->mState <= NET_MGR_STATE_SETUP) {
        return findFriendData(data);
    }
    if (sReqState == REQ_FIND_DATA) {
        int result;
        if (dNetMgr_c::sInstance->mReq.takeResult(dNetFriend::NET_FRIEND_REQ_FIND_DATA, &result)) {
            sReqState = REQ_NONE;
            return result;
        }
    } else if (sReqState == REQ_NONE && dNetMgr_c::reqFindData(data)) {
        sReqState = REQ_FIND_DATA;
    }
    return WIFI_PENDING;
}

// 8017BD4C
int findFriendKey(const u64 *key) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    for (int i = 0; i < FRIEND_NUM; i++) {
        if (isFriendValid(i) && *key == player->mFriends.getFriend(i)->mKey) {
            return i;
        }
    }
    return -1;
}

// 8017BDEC
int findFriendKey(const u64 *key, const dPrivateData_c *player) {
    for (int i = 0; i < FRIEND_NUM; i++) {
        if (*key == player->mFriends.getFriend(i)->mKey) {
            return i;
        }
    }
    return -1;
}

// 8017BEC0
int findFriend(const dPersonalID_c *pid) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    for (int i = 0; i < FRIEND_NUM; i++) {
        if (isFriendValid(i) && player->mFriends.getFriend(i)->mPID == *pid) {
            return i;
        }
    }
    return -1;
}

// 8017BF90
int findFriend(const dPersonalID_c *pid, const dPrivateData_c *player) {
    for (int i = 0; i < FRIEND_NUM; i++) {
        if (player->mFriends.getFriend(i)->mPID == *pid) {
            return i;
        }
    }
    return -1;
}

// 8017C04C
int findFriend(const dPlayerID_c *id, const dPrivateData_c *player) {
    for (int i = 0; i < FRIEND_NUM; i++) {
        if (player->mFriends.getFriend(i)->mPID.player.isSame(id)) {
            return i;
        }
    }
    return -1;
}

// 8017C0CC
int findFriend(const dLandID_c *land, const dPrivateData_c *player) {
    for (int i = 0; i < FRIEND_NUM; i++) {
        if (player->mFriends.getFriend(i)->mPID.land == *land) {
            return i;
        }
    }
    return -1;
}

// 8017C170
BOOL registerFriend(int idx, const DWCFriendData *data, const dFriend_c *fr) {
    if (dNetMgr_c::sInstance->mState > NET_MGR_STATE_SETUP) {
        return FALSE;
    }
    if (!dNetMgr_c::isFriendDataValid(data)) {
        return FALSE;
    }
    dPlayerMgr::Mgr_c::getCurrentPlayer()->mFriends.set2(idx, data, fr);
    return TRUE;
}

// 8017C1F8
BOOL registerFriendReq(int idx, const DWCFriendData *data, const dFriend_c *fr) {
    if (dNetMgr_c::sInstance->mState <= NET_MGR_STATE_SETUP) {
        return registerFriend(idx, data, fr);
    }
    if (sReqState == REQ_REGISTERED) {
        sReqState = REQ_NONE;
        return TRUE;
    }
    if (sReqState == REQ_NONE && dNetMgr_c::reqRegister(idx, data)) {
        sReqState = REQ_REGISTER;
        if (fr != NULL) {
            sRegisterFriend.copy(fr);
        } else {
            sRegisterFriend.clear();
        }
    }
    return FALSE;
}

// 8017C29C
BOOL removeFriend(int idx) {
    if (dNetMgr_c::sInstance->mState > NET_MGR_STATE_SETUP) {
        return FALSE;
    }
    dPlayerMgr::Mgr_c::getCurrentPlayer()->mFriends.remove(idx);
    return TRUE;
}

// 8017C2EC
BOOL removeFriendReq(int idx) {
    if (dNetMgr_c::sInstance->mState <= NET_MGR_STATE_SETUP) {
        return removeFriend(idx);
    }
    if (sReqState == REQ_REMOVED) {
        sReqState = REQ_NONE;
        return TRUE;
    }
    if (sReqState == REQ_NONE && dNetMgr_c::reqRemove(idx)) {
        sReqState = REQ_REMOVE;
    }
    return FALSE;
}

// 8017C35C
void updateReq() {
    int result;
    switch (sReqState) {
    case REQ_REGISTER:
        if (dNetMgr_c::sInstance->mReq.takeResult(dNetFriend::NET_FRIEND_REQ_REGISTER, &result)) {
            onRegistered(result);
        }
        break;
    case REQ_REMOVE:
        if (dNetMgr_c::sInstance->mReq.takeResult(dNetFriend::NET_FRIEND_REQ_REMOVE, &result)) {
            onRemoved(result);
        }
        break;
    }
}

// 8017C3EC
void updateFriend(u32 idx, const dPersonalID_c *pid, const wchar_t *message, const u64 *key) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (idx < FRIEND_NUM) {
        player->mFriends.mFriends[idx].mKey = *key;
    }
    sChanged = TRUE;
    dPlayerMgr::Mgr_c::getCurrentPlayer()->mFriends.update(idx, pid, message);
}

// 8017C478
BOOL updateSave() {
    if (sChanged) {
        if (!sSaving) {
            if (dWifi::startRegisterFriends()) {
                sSaving = TRUE;
            }
            return FALSE;
        }
        if (dWifi::getThreadResult() == -1) {
            return FALSE;
        }
        sChanged = FALSE;
        sSaving = FALSE;
    }
    return TRUE;
}

} // namespace dWifi
