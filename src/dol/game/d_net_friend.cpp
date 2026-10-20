#include <game/game/d_net_dwc.hpp>
#include <game/game/d_net_friend.hpp>
#include <string.h>

namespace dNetFriend {

// 800E6804
void Status_c::clear() {
    mFriendKey = 0;
    mProfileId = 0;
    mAid = 0;
    mFlags = 0;
    memset(mStatus, 0, sizeof(mStatus));
}

// 800E682C
Info_c::Info_c() {
    clear();
    mError.clear();
}

// 800E689C
void Info_c::clear() {
    mState = INFO_OFFLINE;
    mMemberBits = 0;
    mMyNo = 0;
    mMemberCount = 0;
    mError.clear();
    memset(mFriends, 0, sizeof(mFriends));
    mMyFriendKey = 0;
    memset(mMyFriendData, 0, sizeof(mMyFriendData));
    mMyProfileId = 0;
    mUpdatingFriends = 0;
    mMicActive = 0;
    mMatchReady = 0;
}

// 800E6920
void Error_c::clear() {
    mCode = 0;
    mDetail = 0;
    mKind = 0;
    mState = 0;
}

// 800E6938
void Error_c::set(u8 state, u8 kind, int detail, int code) {
    if (kind > mKind) {
        mCode = code;
        mDetail = detail;
        mKind = kind;
        mState = state;
    }
}

// 800E6958
void Data_c::get(Info_c *info, DWCUserData *userData, DWCFriendData *friends) {
    OSLockMutex(&mMutex.mMutex);
    if (userData) {
        memcpy(userData, &mUserData, sizeof(mUserData));
    }
    if (friends) {
        memcpy(friends, mFriends, sizeof(mFriends));
    }
    memcpy(info, &mInfo, sizeof(mInfo));
    OSUnlockMutex(&mMutex.mMutex);
}

// 800E69F4
void Data_c::updateSession(int state, Error_c *error, DWCUserData *user,
                           DWCFriendData *friends, u16 members, u8 updating,
                           BOOL micActive, bool micFailed) {
    OSLockMutex(&mMutex.mMutex);
    if (state == dNetDwc::STATE_CONNECTED) {
        mInfo.mState = INFO_CONNECTED;
    } else if (state >= dNetDwc::STATE_ONLINE) {
        mInfo.mState = INFO_ONLINE;
    } else if (state > dNetDwc::STATE_INITIALIZED) {
        mInfo.mState = INFO_STARTING;
    } else {
        mInfo.mState = INFO_OFFLINE;
    }
    memcpy(&mInfo.mError, error, sizeof(mInfo.mError));
    mInfo.mMyNo = DWC_GetMyAID();
    mInfo.mMemberBits = members;
    u8 count = 0;
    for (int i = 0; i < dNetDwc::MEMBER_COUNT; i++) {
        if (members & (1 << i)) {
            count++;
        }
    }
    mInfo.mMemberCount = count;
    mInfo.mUpdatingFriends = updating;
    mInfo.mMicActive = micActive && !micFailed;
    mInfo.mMicUnavailable = micFailed;
    mInfo.mMatchReady = DWC_GetSuspendMatch() != 0;
    if (user) {
        memcpy(&mUserData, user, sizeof(mUserData));
    }
    if (friends) {
        memcpy(mFriends, friends, sizeof(mFriends));
    }
    mInfo.mMyFriendKey = DWC_CreateFriendKey(user);
    DWC_CreateExchangeToken(user, (DWCFriendData *)mInfo.mMyFriendData);
    int profile =
        DWC_GetGsProfileId(user, (DWCFriendData *)mInfo.mMyFriendData);
    if (profile > 0) {
        mInfo.mMyProfileId = profile;
    } else {
        mInfo.mMyProfileId = 0;
    }
    for (int i = 0; i < NET_FRIEND_NUM; i++) {
        DWCFriendData *friendData = &mFriends[i];
        if (DWC_IsValidFriendData(friendData)) {
            mInfo.mFriends[i].mFlags |= STATUS_VALID;
            mInfo.mFriends[i].mFriendKey = DWC_GetFriendKey(friendData);
            int profile = DWC_GetGsProfileId(user, friendData);
            if (profile > 0) {
                mInfo.mFriends[i].mProfileId = profile;
            } else {
                mInfo.mFriends[i].mProfileId = 0;
            }
            if (DWC_IsBuddyFriendData(friendData)) {
                mInfo.mFriends[i].mFlags |= STATUS_BUDDY;
            } else {
                mInfo.mFriends[i].mFlags &= ~STATUS_BUDDY;
            }
            u8 maxEntry, aid;
            int size;
            if (DWC_GetFriendStatusDataSC(friendData, &maxEntry, &aid,
                                          mInfo.mFriends[i].getStatus(),
                                          &size) == FRIEND_STATUS_CONNECTED) {
                mInfo.mFriends[i].mFlags |= STATUS_HAS_AID;
                mInfo.mFriends[i].mAid = aid;
            } else {
                mInfo.mFriends[i].mFlags &= ~STATUS_HAS_AID;
                mInfo.mFriends[i].mAid = 0;
            }
        } else {
            mInfo.mFriends[i].mFlags &= ~STATUS_VALID;
        }
    }
    OSUnlockMutex(&mMutex.mMutex);
}

// 800E6CC0
void SendBuf_c::reset() {
    OSLockMutex(&mMutex.mMutex);
    mSendState = SEND_IDLE;
    OSUnlockMutex(&mMutex.mMutex);
}
// 800E6CFC
void SendBuf_c::onSent() {
    OSLockMutex(&mMutex.mMutex);
    mSendState = SEND_IDLE;
    OSUnlockMutex(&mMutex.mMutex);
}
// 800E6D38
void SendBuf_c::set(const void *data, u32 size) {
    OSLockMutex(&mMutex.mMutex);
    mData = data;
    mLength = size;
    mSendState = SEND_BUSY;
    OSUnlockMutex(&mMutex.mMutex);
}
// 800E6D94
void SendBuf_c::clearPending() {
    mData = NULL;
}
// 800E6DA0
BOOL SendBuf_c::isBusy() {
    OSLockMutex(&mMutex.mMutex);
    BOOL busy = mSendState != SEND_IDLE;
    OSUnlockMutex(&mMutex.mMutex);
    return busy;
}
// 800E6DE8
BOOL SendBuf_c::hasPending() {
    OSLockMutex(&mMutex.mMutex);
    BOOL pending = FALSE;
    if (mSendState == SEND_BUSY && mData) {
        pending = TRUE;
    }
    OSUnlockMutex(&mMutex.mMutex);
    return pending;
}
// 800E6E48
void Request_c::reset() {
    finish();
    mConnect = 0;
    mDisconnect = 0;
}
// 800E6E80
void Request_c::finish() {
    OSLockMutex(&mMutex);
    mKind = NET_FRIEND_REQ_NONE;
    OSUnlockMutex(&mMutex);
}
// 800E6EBC
int Request_c::getKind() {
    OSLockMutex(&mMutex);
    int kind = mKind;
    OSUnlockMutex(&mMutex);
    return kind;
}
// 800E6F00
u16 Request_c::takeConnect() {
    OSLockMutex(&mMutex);
    u16 mask = mConnect;
    mConnect = 0;
    OSUnlockMutex(&mMutex);
    return mask;
}
// 800E6F4C
u16 Request_c::takeDisconnect() {
    OSLockMutex(&mMutex);
    u16 mask = mDisconnect;
    mDisconnect = 0;
    OSUnlockMutex(&mMutex);
    return mask;
}
// 800E6F98
void Request_c::setResult(int kind, int result) {
    if (kind >= NET_FRIEND_REQ_REGISTER) {
        OSLockMutex(&mMutex);
        mResults[kind - NET_FRIEND_REQ_REGISTER] = result;
        OSUnlockMutex(&mMutex);
    }
    finish();
}
// 800E7004
void Request_c::setResultData(int kind, const DWCFriendData *data) {
    OSLockMutex(&mMutex);
    memcpy(&mResultData, data, sizeof(mResultData));
    OSUnlockMutex(&mMutex);
    finish();
}
// 800E705C
void Request_c::setResultKey(int kind, u64 key) {
    OSLockMutex(&mMutex);
    mResultKey = key;
    OSUnlockMutex(&mMutex);
    finish();
}
// 800E70B8
BOOL Request_c::takeResult(int kind, int *result) {
    if (kind != getKind()) {
        OSLockMutex(&mMutex);
        *result = mResults[kind - NET_FRIEND_REQ_REGISTER];
        OSUnlockMutex(&mMutex);
        return TRUE;
    }
    return FALSE;
}
// 800E7134
BOOL Request_c::takeResultData(int kind, DWCFriendData *data) {
    if (kind != getKind()) {
        OSLockMutex(&mMutex);
        memcpy(data, &mResultData, sizeof(mResultData));
        OSUnlockMutex(&mMutex);
        return TRUE;
    }
    return FALSE;
}
// 800E71AC
BOOL Request_c::submit(int kind, const RequestArgs_c *args) {
    OSLockMutex(&mMutex);
    BOOL accepted;
    if (mKind == NET_FRIEND_REQ_NONE) {
        mArgs = *args;
        mKind = kind;
        accepted = TRUE;
    } else {
        accepted = FALSE;
    }
    OSUnlockMutex(&mMutex);
    return accepted;
}

// 800E723C
BOOL Request_c::requestHostMatch(u8 memberNum, int arg) {
    RequestArgs_c args;
    args.mMatch.mMemberNum = memberNum;
    args.mMatch.mArg = arg;
    return submit(NET_FRIEND_REQ_HOST_MATCH, &args);
}

// 800E726C
BOOL Request_c::requestFriendMatch(u8 memberNum, u32 arg) {
    RequestArgs_c args;
    args.mMatch.mValue = arg;
    args.mMatch.mMemberNum = memberNum;
    return submit(NET_FRIEND_REQ_MATCH_FRIEND, &args);
}

// 800E729C
BOOL Request_c::requestClearError() {
    RequestArgs_c args;

    return submit(NET_FRIEND_REQ_CLEAR_ERROR, &args);
}

// 800E72C4
BOOL Request_c::requestRegisterFriend(int index, const DWCFriendData *data) {
    RequestArgs_c args;
    memcpy(&args.mFriend.mData, data, sizeof(args.mFriend.mData));
    args.mFriend.mIndex = index;
    return submit(NET_FRIEND_REQ_REGISTER, &args);
}

// 800E731C
BOOL Request_c::requestRemoveFriend(int index) {
    RequestArgs_c args;
    args.mFriend.mIndex = index;
    return submit(NET_FRIEND_REQ_REMOVE, &args);
}

// 800E7348
BOOL Request_c::requestMyFriendData(const DWCUserData *userData) {
    RequestArgs_c args;
    memcpy(&args.mUserData, userData, sizeof(args.mUserData));
    return submit(NET_FRIEND_REQ_MY_DATA, &args);
}

// 800E738C
BOOL Request_c::requestFriendKeyData(u64 key) {
    RequestArgs_c args;
    args.mFriendKey = key;
    return submit(NET_FRIEND_REQ_KEY_DATA, &args);
}

// 800E73BC
BOOL Request_c::requestCheckFriendKey(u64 key) {
    RequestArgs_c args;
    args.mFriendKey = key;
    return submit(NET_FRIEND_REQ_CHECK_KEY, &args);
}

// 800E73EC
BOOL Request_c::requestFindUser(const DWCUserData *userData) {
    RequestArgs_c args;
    memcpy(&args.mUserData, userData, sizeof(args.mUserData));
    return submit(NET_FRIEND_REQ_FIND_USER, &args);
}

// 800E7430
BOOL Request_c::requestFindFriendData(const DWCFriendData *data) {
    RequestArgs_c args;
    memcpy(&args.mFriend.mData, data, sizeof(args.mFriend.mData));
    return submit(NET_FRIEND_REQ_FIND_DATA, &args);
}

// 800E7474
BOOL Request_c::requestIsFriend(const DWCUserData *userData, u64 key) {
    RequestArgs_c args;
    memcpy(&args.mAccountKey.mUserData, userData,
           sizeof(args.mAccountKey.mUserData));
    args.mAccountKey.mFriendKey = key;
    return submit(NET_FRIEND_REQ_IS_FRIEND, &args);
}

// 800E74D8
BOOL Request_c::connect(u8 aid) {
    OSLockMutex(&mMutex);
    mConnect |= (1 << aid);
    mDisconnect &= ~(1 << aid);
    OSUnlockMutex(&mMutex);
    return TRUE;
}

// 800E753C
BOOL Request_c::disconnect(u8 aid) {
    OSLockMutex(&mMutex);
    mConnect &= ~(1 << aid);
    mDisconnect |= (1 << aid);
    OSUnlockMutex(&mMutex);
    return TRUE;
}

// 800E75A0
void Request_c::addConnect(u16 mask) {
    OSLockMutex(&mMutex);
    mConnect |= mask;
    mConnect &= ~mDisconnect;
    mDisconnect = 0;
    OSUnlockMutex(&mMutex);
}

// 800E7600
BOOL Request_c::requestMute() {
    RequestArgs_c args;
    return submit(NET_FRIEND_REQ_MUTE_ON, &args);
}

// 800E7628
BOOL Request_c::requestUnmute() {
    RequestArgs_c args;
    return submit(NET_FRIEND_REQ_MUTE_OFF, &args);
}

} // namespace dNetFriend
