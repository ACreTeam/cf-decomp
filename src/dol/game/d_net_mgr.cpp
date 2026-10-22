// d_net_mgr: the network manager between d_net and the DWC thread (d_net_dwc).
// .text 800E48A0..800E6804, .ctors 804656CC, .data 804ECAE8..804ECB38, .bss
// 805B2CD8..805B68E0, .sbss 8074E538..8074E540. See
// include/game/game/d_net_mgr.hpp and notes/d_net_mgr.txt.
#include <cstring>
#include <game/game/d_heap.hpp>
#include <game/game/d_nand.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_net_mgr.hpp>
#include <game/game/d_obj_telop.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_voice_chat.hpp>
#include <game/game/d_wifi.hpp>
#include <lib/egg/core/eggHeap.h>
#include <revolution/sc.h>

dNetMgr_c *dNetMgr_c::sInstance;

dNetMgr_c::dNetMgr_c() {
    dNetFriend::Request_c *req = &mReq;
    OSInitMutex(&req->mMutex);
    req->mKind = dNetFriend::NET_FRIEND_REQ_NONE;
    req->mConnect = 0;
    req->mDisconnect = 0;

    mStatusCb = NULL;
    mJoinCb = NULL;
    mLeaveCb = NULL;
    mRecvCb = NULL;
    mEventCb = NULL;
    mStatusSize = 0;
    mFriends = NULL;
    mUserDataSrc = NULL;
    mFriendSrc = NULL;
    mSendBufs = NULL;
    mSendCounts = NULL;
    mSendCountMax = NULL;
    mRecvBufs = NULL;
    mRecvBufSize = 0;
    mRemovedFriends = 0;
    mState = NET_MGR_STATE_NONE;
    mMemberNum = 1;
    m3861 = 0;
    mFriendNum = 0;
    m3863 = 0;
    mStarted = 0;
    mTelopShown = 0;
    mVoiceOff = 0;
    mBusy = 0;
    sInstance = this;
}

dNetMgr_c::~dNetMgr_c() {
    cleanup();
    g_netDwc.cleanup();
    sInstance = NULL;
}

void dNetMgr_c::initDwc() {
    g_netDwc.init(dHeap::netSoHeap_p, dHeap::netDwcHeap_p, &sInstance->mData);
}

void dNetMgr_c::lock(int) {
    OSLockMutex(&mMutex.mMutex);
}

void dNetMgr_c::unlock(int) {
    OSUnlockMutex(&mMutex.mMutex);
}

void dNetMgr_c::setup(DWCUserData *userData, DWCFriendData *friends,
                      int friendNum, u8 memberNum, u8 *const *bufs,
                      u32 bufSize) {
    if (mState < NET_MGR_STATE_SETUP) {
        mFriendSrc = friends;
        mUserDataSrc = userData;
        mFriendNum = friendNum;
        mMemberNum = memberNum;
        mFriends = new (dHeap::netSoHeap_p, 4) DWCFriendData[friendNum];
        mSendBufs = new (dHeap::netSoHeap_p, 4) dNetFriend::SendBuf_c[memberNum];
        mRecvBufs = new (dHeap::netSoHeap_p, 4) u8 *[memberNum - 1];
        if (bufs != NULL) {
            for (int i = 0; i < memberNum - 1; i++) {
                mRecvBufs[i] = bufs[i];
            }
            mRecvBufSize = bufSize;
        }
        mSendCounts = new (dHeap::netSoHeap_p, 4) int[memberNum];
        mSendCountMax = new (dHeap::netSoHeap_p, 4) int[memberNum];
        for (int i = 0; i < memberNum; i++) {
            mSendCounts[i] = 0;
            mSendCountMax[i] = 0;
        }
        mState = NET_MGR_STATE_SETUP;
    }
}

void dNetMgr_c::cleanup() {
    if (mState > NET_MGR_STATE_NONE) {
        delete[] mSendCounts;
        mSendCounts = NULL;
        delete[] mSendCountMax;
        mSendCountMax = NULL;
        delete[] mRecvBufs;
        mRecvBufs = NULL;
        mRecvBufSize = 0;
        delete[] mSendBufs;
        mSendBufs = NULL;
        delete[] mFriends;
        mFriends = NULL;
        mState = NET_MGR_STATE_NONE;
        g_netDwc.cleanup();
        g_netDwc.init(dHeap::netSoHeap_p, dHeap::netDwcHeap_p, &mData);
    }
}

void dNetMgr_c::setCallbacks(dNetMgrRecvFunc recv, dNetMgrMemberFunc join,
                             dNetMgrEventFunc event, dNetMgrFunc status,
                             const void *statusData, u32 statusSize,
                             dNetMgrMemberFunc leave) {
    mJoinCb = join;
    mStatusCb = status;
    mRecvCb = recv;
    mEventCb = event;
    mLeaveCb = leave;
    memcpy(mStatus, statusData, statusSize);
    mStatusSize = statusSize;
}

void dNetMgr_c::reset() {
    g_netDwc.reset();
    mInfo.clear();
    mReq.reset();
    m3863 = 0;
    mVoiceOff = 0;
    mBusy = 0;
    mTelopShown = 0;
}

BOOL dNetMgr_c::start() {
    if (sInstance->mState < NET_MGR_STATE_SETUP) {
        return FALSE;
    }
    if (mState != NET_MGR_STATE_SETUP) {
        return FALSE;
    }
    if (sInstance->mInfo.mError.mState != 0) {
        return FALSE;
    }
    if (SCCheckPCMessageRestriction()) {
        return FALSE;
    }
    if (!isUserDataValid(mUserDataSrc)) {
        return FALSE;
    }
    memcpy(&mUserData, mUserDataSrc, sizeof(DWCUserData));
    memcpy(mFriends, mFriendSrc, mFriendNum * sizeof(DWCFriendData));
    mStarted = 1;
    setVolume(dSaveOption::getPlayerVoiceVolume());
    dVoiceChat::setMute(dSaveOption::getVoiceSound() ? 0 : 1);
    dVoiceChat::fn_80171C10(dSaveData_c::getRawExtra()->mVoiceLevel);
    if (!dSaveOption::getVoiceChat()) {
        mVoiceOff = 1;
    } else {
        mVoiceOff = 0;
    }
    mBusy = 1;
    g_netDwc.start(&mUserData, mFriends, mFriendNum, mSendBufs, mVoiceOff);
    mData.get(&mInfo, NULL, NULL);
    mState = NET_MGR_STATE_STARTED;
    return TRUE;
}

BOOL dNetMgr_c::open() {
    dNetMgr_c *mgr = sInstance;
    if (mgr->mInfo.mState >= 2 && !mgr->mInfo.mUpdatingFriends) {
        return TRUE;
    }
    if (mgr->mState <= NET_MGR_STATE_SETUP) {
        mgr->start();
    }
    return FALSE;
}

BOOL dNetMgr_c::close() {
    int state = sInstance->mState;
    if (state <= NET_MGR_STATE_SETUP) {
        sInstance->reset();
        return TRUE;
    }
    if (state > NET_MGR_STATE_2) {
        g_netDwc.requestStop();
        sInstance->mState = NET_MGR_STATE_2;
    }
    return FALSE;
}

void dNetMgr_c::execute() {
    if (sInstance->mState > NET_MGR_STATE_SETUP) {
        if (dNand::isWriting()) {
            mData.get(&mInfo, NULL, NULL);
        } else {
            mData.get(&mInfo,
                      &dPlayerMgr::Mgr_c::getCurrentPlayer()->mFriends.mUserData,
                      dPlayerMgr::Mgr_c::getCurrentPlayer()->mFriends.mKeys);
            dWifi::updateReq();
        }
        if (mState == NET_MGR_STATE_2 && mInfo.mState < 1) {
            mState = NET_MGR_STATE_SETUP;
        }
        if (dVoiceChat::isStarted() && !mBusy && !mTelopShown && mInfo.mMicUnavailable) {
            dObjTelop_c::request(TELOP_MIC_CHAT_UNAVAILABLE);
            mTelopShown = 1;
        }
    }
}

BOOL dNetMgr_c::fn_800E51C8(int arg) {
    if (mState == NET_MGR_STATE_5) {
        return TRUE;
    }
    if (!mReq.requestHostMatch(mMemberNum, arg)) {
        return FALSE;
    }
    mState = NET_MGR_STATE_5;
    return TRUE;
}

BOOL dNetMgr_c::fn_800E5230(int arg) {
    if (mState == NET_MGR_STATE_6) {
        return TRUE;
    }
    if (!mReq.requestFriendMatch(mMemberNum, arg)) {
        return FALSE;
    }
    mState = NET_MGR_STATE_6;
    return TRUE;
}

BOOL dNetMgr_c::fn_800E5298(int arg) {
    if (sInstance->mInfo.mState == 3) {
        return TRUE;
    }
    fn_800E5230(arg);
    return FALSE;
}

BOOL dNetMgr_c::setRecvBuf(u8 member, u8 *buf, u32 size) {
    if (sInstance->mInfo.mState != 3) {
        return FALSE;
    }
    lock(0);
    if (member != sInstance->mInfo.mMyNo) {
        if (member < sInstance->mInfo.mMyNo) {
            mRecvBufs[member] = buf;
        } else {
            mRecvBufs[member - 1] = buf;
        }
        mRecvBufSize = size;
    }
    unlock(0);
    return TRUE;
}

BOOL dNetMgr_c::send(u8 member, const void *data, u32 size) {
    if (size > mRecvBufSize) {
        return FALSE;
    }
    if (sInstance->mInfo.mState != 3) {
        return FALSE;
    }
    if (!sInstance->isPeer(member)) {
        return FALSE;
    }
    if (mSendBufs[member].isBusy()) {
        return FALSE;
    }
    mSendBufs[member].set(data, size);
    return TRUE;
}

BOOL dNetMgr_c::canSend(u8 member) {
    if (sInstance->mInfo.mState != 3) {
        return FALSE;
    }
    if (member >= mMemberNum || !sInstance->isPeer(member) ||
        mSendBufs[member].isBusy()) {
        return FALSE;
    }
    return TRUE;
}

int dNetMgr_c::getErrorKind() {
    dNetMgr_c *mgr = sInstance;
    if (!mgr->mInfo.mError.mState) {
        return ERROR_KIND_NONE;
    }
    switch (mgr->mInfo.mError.mKind) {
    case dNetDwc::ERROR_RESTART_REQUIRED:
        return ERROR_KIND_RESTART;
    case dNetDwc::ERROR_PERMANENT:
        return ERROR_KIND_PERMANENT;
    }
    if (mgr->mInfo.mError.getState() == 0xF) {
        return ERROR_KIND_GATE_CLOSED;
    }
    if (mgr->mInfo.mError.getState() == 0x10) {
        return ERROR_KIND_TOWN_FULL;
    }
    switch (mgr->mInfo.mError.mCode) {
    case 0xB:
        return ERROR_KIND_GATE_CLOSED;
    case 0xC:
        return ERROR_KIND_TOWN_FULL;
    case 0x17:
        return ERROR_KIND_TOWN_BUSY;
    }
    return ERROR_KIND_OTHER;
}

void dNetMgr_c::fn_800E55B0() {
    if (sInstance->mState > NET_MGR_STATE_SETUP) {
        sInstance->mReq.requestClearError();
    }
}

u8 dNetMgr_c::getFriendStatus(int idx, void *dst, u32 *size) {
    memcpy(dst, mInfo.mFriends[idx].mStatus, mStatusSize);
    *size = mStatusSize;
    if (mInfo.mFriends[idx].hasAid()) {
        return mInfo.mFriends[idx].mAid;
    }
    return 0;
}

BOOL dNetMgr_c::isFriendInSession(int idx) {
    dNetFriend::Status_c *status = &mInfo.mFriends[idx];
    if (status->hasAid() && status->mAid < mMemberNum) {
        return TRUE;
    }
    return FALSE;
}

BOOL dNetMgr_c::isStarted() {
    sInstance->lock(2);
    BOOL started = sInstance->mStarted;
    sInstance->unlock(2);
    return started;
}

void dNetMgr_c::setVoiceOff() {
    if (sInstance->mState > NET_MGR_STATE_SETUP) {
        sInstance->lock(3);
        sInstance->mVoiceOff = 1;
        sInstance->unlock(3);
    }
}

void dNetMgr_c::setVoiceOn() {
    if (sInstance->mState > NET_MGR_STATE_SETUP) {
        sInstance->lock(3);
        sInstance->mVoiceOff = 0;
        sInstance->unlock(3);
    }
}

void dNetMgr_c::connect(u8 aid) {
    if (sInstance->mState > NET_MGR_STATE_SETUP) {
        sInstance->mReq.connect(aid);
    }
}

void dNetMgr_c::disconnect(u8 aid) {
    if (sInstance->mState > NET_MGR_STATE_SETUP) {
        sInstance->mReq.disconnect(aid);
    }
}

void dNetMgr_c::setBusy() {
    sInstance->lock(3);
    sInstance->mBusy = 1;
    sInstance->unlock(3);
}

void dNetMgr_c::clearBusy() {
    sInstance->lock(3);
    sInstance->mBusy = 0;
    sInstance->unlock(3);
}

void dNetMgr_c::setVolume(int volume) {
    dVoiceChat::setVolume(volume);
}

void dNetMgr_c::mute() {
    if (sInstance->mState > NET_MGR_STATE_SETUP) {
        sInstance->mReq.requestMute();
    } else {
        dVoiceChat::setMute(1);
    }
}

void dNetMgr_c::unmute() {
    if (sInstance->mState > NET_MGR_STATE_SETUP) {
        sInstance->mReq.requestUnmute();
    } else {
        dVoiceChat::setMute(0);
    }
}

void dNetMgr_c::initFriendList(DWCUserData *userData, DWCFriendData *friends,
                               u32 num) {
    DWC_CreateUserData(userData);
    DWC_ClearDirtyFlag(userData);
    for (u32 i = 0; i < num; i++) {
        if (DWC_IsValidFriendData(&friends[i])) {
            DWC_ClearBuddyFlagFriendData(&friends[i]);
        }
    }
    DWC_ReportUserData(userData);
}

s64 dNetMgr_c::getInfo1A10() {
    return sInstance->mInfo.mMyFriendKey;
}

u32 dNetMgr_c::getInfo1A28() {
    return sInstance->mInfo.mMyProfileId;
}

s64 dNetMgr_c::getFriend00(int idx) {
    return sInstance->mInfo.mFriends[idx].mFriendKey;
}

u32 dNetMgr_c::getFriend08(int idx) {
    return sInstance->mInfo.mFriends[idx].mProfileId;
}

BOOL dNetMgr_c::getFriendFlag(int idx) {
    return sInstance->mInfo.mFriends[idx].mFlags & 1;
}

BOOL dNetMgr_c::isUserDataValid(const DWCUserData *userData) {
    if (DWC_CheckUserData(userData) && DWC_IsCompatibleUserData(userData)) {
        return TRUE;
    }
    return FALSE;
}

void dNetMgr_c::getMyFriendData(const DWCUserData *userData,
                                DWCFriendData *friendData) {
    DWC_CreateExchangeToken(userData, friendData);
}

u64 dNetMgr_c::getMyFriendKey(const DWCUserData *userData) {
    return DWC_CreateFriendKey(userData);
}

int dNetMgr_c::getMyProfileId(const DWCUserData *userData) {
    DWCFriendData friendData;
    DWC_CreateExchangeToken(userData, &friendData);
    return getProfileId(userData, &friendData);
}

u64 dNetMgr_c::getFriendKey(const DWCFriendData *friendData) {
    return DWC_GetFriendKey(friendData);
}

int dNetMgr_c::getProfileId(const DWCUserData *userData,
                            const DWCFriendData *friendData) {
    int id = DWC_GetGsProfileId(userData, friendData);
    return id > 0 ? id : 0;
}

void dNetMgr_c::makeFriendData(u64 key, DWCFriendData *friendData) {
    DWC_CreateFriendKeyToken(friendData, key);
}

BOOL dNetMgr_c::isFriendDataValid(const DWCFriendData *friendData) {
    return DWC_IsValidFriendData(friendData);
}

BOOL dNetMgr_c::isSameFriend(const DWCUserData *userData,
                             const DWCFriendData *a, const DWCFriendData *b) {
    if (!isFriendDataValid(a)) {
        return FALSE;
    }
    if (!isFriendDataValid(b)) {
        return FALSE;
    }
    if (DWC_IsEqualFriendData(a, b)) {
        return TRUE;
    }
    int kindA = DWC_GetFriendDataType(a);
    int kindB = DWC_GetFriendDataType(b);
    if ((kindA == 2 && kindB == 3) || (kindB == 2 && kindA == 3)) {
        int idA = DWC_GetGsProfileId(userData, a);
        int idB = DWC_GetGsProfileId(userData, b);
        if (idA > 0 && idB > 0 && idA == idB) {
            return TRUE;
        }
    }
    return FALSE;
}

int dNetMgr_c::checkFriendKey(const DWCUserData *userData, u64 key) {
    return DWC_CheckFriendKey(userData, key);
}

BOOL dNetMgr_c::reqMyData(const DWCUserData *userData) {
    return sInstance->mReq.requestMyFriendData(userData);
}

BOOL dNetMgr_c::reqKeyData(u64 key) {
    return sInstance->mReq.requestFriendKeyData(key);
}

BOOL dNetMgr_c::reqIsFriend(const DWCUserData *userData, u64 key) {
    return sInstance->mReq.requestIsFriend(userData, key);
}

BOOL dNetMgr_c::reqCheckKey(u64 key) {
    return sInstance->mReq.requestCheckFriendKey(key);
}

BOOL dNetMgr_c::reqRegister(int index, const DWCFriendData *data) {
    return sInstance->mReq.requestRegisterFriend(index, data);
}

BOOL dNetMgr_c::reqRemove(int index) {
    return sInstance->mReq.requestRemoveFriend(index);
}

BOOL dNetMgr_c::reqFindUser(const DWCUserData *userData) {
    return sInstance->mReq.requestFindUser(userData);
}

BOOL dNetMgr_c::reqFindData(const DWCFriendData *data) {
    return sInstance->mReq.requestFindFriendData(data);
}

void dNetMgr_c::onStatus() {
    dNetMgr_c *mgr = sInstance;
    if (mgr->mStatusSize != 0) {
        DWC_SetOwnStatusData(mgr->mStatus, mgr->mStatusSize);
    }
    if (mgr->mStatusCb != NULL) {
        mgr->mStatusCb();
    }
}

BOOL dNetMgr_c::isRequestIdle() {
    return dNet::isRequestIdle();
}

void dNetMgr_c::onJoin(u8 aid) {
    dNetMgr_c *mgr = sInstance;
    mgr->setDwcRecvBuf(aid);
    if (mgr->mJoinCb != NULL) {
        mgr->mJoinCb(aid);
    }
}

void dNetMgr_c::onLeave(u8 aid) {
    if (sInstance->mLeaveCb != NULL) {
        sInstance->mLeaveCb(aid);
    }
}

void dNetMgr_c::fn_800E5C7C() {
    dNetMgr_c *mgr = sInstance;
    if (mgr->mState < 7 && mgr->mState >= NET_MGR_STATE_4) {
        mgr->m3863 = 0;
        mgr->mState = NET_MGR_STATE_STARTED;
    }
}

void dNetMgr_c::onRecv(u8 aid, const void *data, u32 size) {
    sInstance->lock(0);
    dNetMgr_c *mgr = sInstance;
    if (mgr->mRecvCb != NULL) {
        mgr->mRecvCb(aid, data, size);
    }
    mgr->setDwcRecvBuf(aid);
    sInstance->unlock(0);
}

void dNetMgr_c::setDwcRecvBuf(u8 aid) {
    u8 myAid = DWC_GetMyAID();
    if (aid < myAid) {
        DWC_SetRecvBuffer(aid, mRecvBufs[aid], mRecvBufSize);
    } else if (aid > myAid) {
        DWC_SetRecvBuffer(aid, mRecvBufs[aid - 1], mRecvBufSize);
    }
}

void dNetMgr_c::onEvent(int arg, int arg2) {
    if (sInstance->mEventCb != NULL) {
        sInstance->mEventCb(arg, arg2);
    }
}

void dNetMgr_c::setFriendRemoved(int index) {
    sInstance->mRemovedFriends |= 1 << index;
}

void dNetMgr_c::procDwcRequest() {
    dNetFriend::Request_c *req = &sInstance->mReq;
    switch (req->getKind()) {
    case dNetFriend::NET_FRIEND_REQ_MATCH_ANYBODY:
        if (g_netDwc.matchAnybody(req->mArgs.mMatch.mMemberNum)) {
            req->finish();
        }
        break;
    case dNetFriend::NET_FRIEND_REQ_HOST_MATCH:
        if (g_netDwc.hostMatch(req->mArgs.mMatch.mMemberNum,
                                 req->mArgs.mMatch.mArg)) {
            req->finish();
        }
        break;
    case dNetFriend::NET_FRIEND_REQ_MATCH_FRIEND:
        if (g_netDwc.matchFriend(req->mArgs.mMatch.mMemberNum,
                                 req->mArgs.mMatch.mValue)) {
            req->finish();
        }
        break;
    case dNetFriend::NET_FRIEND_REQ_CANCEL_MATCH:
        g_netDwc.cancelMatch();
        req->finish();
        break;
    case dNetFriend::NET_FRIEND_REQ_CLOSE_CONNECTIONS:
        g_netDwc.closeConnections();
        req->finish();
        break;
    case dNetFriend::NET_FRIEND_REQ_CLEAR_ERROR:
        g_netDwc.clearError();
        req->finish();
        break;
    }

    sInstance->lock(1);
    if (sInstance->m3863) {
        g_netDwc.setBusy();
    } else {
        g_netDwc.clearBusy();
    }
    sInstance->unlock(1);

    sInstance->lock(3);
    if (!sInstance->mVoiceOff && dVoiceChat::isStarted()) {
        g_netDwc.setVoiceEnabled(FALSE);
    } else if (sInstance->mVoiceOff && !dVoiceChat::isStarted()) {
        g_netDwc.setVoiceEnabled(TRUE);
        for (int i = 0; i < sInstance->mMemberNum; i++) {
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayerRaw(i);
            if (player != NULL && player->mPID.isValid() &&
                !player->getOption()->mVoiceChat) {
                sInstance->mReq.connect(i);
            }
        }
    }
    sInstance->unlock(3);
    sInstance->mRemovedFriends = 0;
}

void dNetMgr_c::procRequest() {
    dNetMgr_c *mgr = sInstance;
    dNetFriend::Request_c *req = &mgr->mReq;
    DWCFriendData *friends = mgr->mFriends;
    DWCFriendData *reqData = &req->mArgs.mFriend.mData;

    if (DWC_CanChangeFriendList()) {
        int index = req->mArgs.mFriend.mIndex;
        switch (req->getKind()) {
        case dNetFriend::NET_FRIEND_REQ_REMOVE:
            DWC_DeleteBuddyFriendData(&friends[index]);
            mgr->mRemovedFriends |= 1 << index;
            g_netDwc.updateFriends();
            req->setResult(dNetFriend::NET_FRIEND_REQ_REMOVE, index);
            break;
        }
        u32 removed = mgr->mRemovedFriends;
        if (removed != 0) {
            u8 num = mgr->mFriendNum;
            for (u32 i = 0; i < num; i++) {
                if (removed & (1 << i)) {
                    u32 dst = i;
                    for (u32 j = i + 1; j < num; j++) {
                        if (!(mgr->mRemovedFriends & (1 << j))) {
                            memcpy(&friends[dst], &friends[j],
                                   sizeof(DWCFriendData));
                            memset(&friends[j], 0, sizeof(DWCFriendData));
                            dst++;
                        }
                    }
                    break;
                }
            }
        }
        switch (req->getKind()) {
        case dNetFriend::NET_FRIEND_REQ_REGISTER:
            if (isFriendDataValid(reqData)) {
                if (!isSameFriend(&mgr->mUserData, reqData, &friends[index])) {
                    memcpy(&friends[index], reqData, sizeof(DWCFriendData));
                    g_netDwc.updateFriends();
                }
                req->setResult(dNetFriend::NET_FRIEND_REQ_REGISTER, index);
            } else {
                req->setResult(dNetFriend::NET_FRIEND_REQ_REGISTER, -1);
            }
            break;
        }
    }

    DWCFriendData friendData;
    DWCFriendData friendData2;
    u32 i;
    switch (req->getKind()) {
    case dNetFriend::NET_FRIEND_REQ_VOICE_PARAM_ON:
        dVoiceChat::setParam(1);
        req->finish();
        break;
    case dNetFriend::NET_FRIEND_REQ_VOICE_PARAM_OFF:
        dVoiceChat::setParam(0);
        req->finish();
        break;
    case dNetFriend::NET_FRIEND_REQ_MUTE_ON:
        dVoiceChat::setMute(1);
        req->finish();
        break;
    case dNetFriend::NET_FRIEND_REQ_MUTE_OFF:
        dVoiceChat::setMute(0);
        req->finish();
        break;
    case dNetFriend::NET_FRIEND_REQ_VOICE_B_ON:
        if (dVoiceChat::fn_80171C08(1)) {
            dSaveData_c::getExtra()->setVoiceSetting(1);
        }
        req->finish();
        break;
    case dNetFriend::NET_FRIEND_REQ_VOICE_B_OFF:
        if (dVoiceChat::fn_80171C08(0)) {
            dSaveData_c::getExtra()->setVoiceSetting(0);
        }
        req->finish();
        break;
    case dNetFriend::NET_FRIEND_REQ_VOICE_C: {
        u8 level = req->mArgs.mParam;
        if (dVoiceChat::fn_80171C10(level)) {
            dSaveData_c::getExtra()->mVoiceLevel = level;
        }
        req->finish();
        break;
    }
    case dNetFriend::NET_FRIEND_REQ_MY_DATA:
        getMyFriendData(&req->mArgs.mUserData, &friendData);
        req->setResultData(dNetFriend::NET_FRIEND_REQ_MY_DATA, &friendData);
        break;
    case dNetFriend::NET_FRIEND_REQ_MY_KEY:
        req->setResultKey(dNetFriend::NET_FRIEND_REQ_MY_KEY,
                          getMyFriendKey(&req->mArgs.mUserData));
        break;
    case dNetFriend::NET_FRIEND_REQ_KEY:
        req->setResultKey(dNetFriend::NET_FRIEND_REQ_KEY, getFriendKey(reqData));
        break;
    case dNetFriend::NET_FRIEND_REQ_KEY_DATA:
        makeFriendData(req->mArgs.mFriendKey, &friendData);
        req->setResultData(dNetFriend::NET_FRIEND_REQ_KEY_DATA, &friendData);
        break;
    case dNetFriend::NET_FRIEND_REQ_CHECK_KEY:
        req->setResult(dNetFriend::NET_FRIEND_REQ_CHECK_KEY,
                       checkFriendKey(&mgr->mUserData, req->mArgs.mFriendKey));
        break;
    case dNetFriend::NET_FRIEND_REQ_FIND_USER:
        getMyFriendData(&req->mArgs.mUserData, &friendData);
        for (i = 0; i < mgr->mFriendNum; i++) {
            if (isSameFriend(&mgr->mUserData, friends, &friendData)) {
                break;
            }
            friends++;
        }
        if (i < mgr->mFriendNum) {
            req->setResult(dNetFriend::NET_FRIEND_REQ_FIND_USER, i);
        } else {
            req->setResult(dNetFriend::NET_FRIEND_REQ_FIND_USER, -1);
        }
        break;
    case dNetFriend::NET_FRIEND_REQ_FIND_DATA:
        for (i = 0; i < mgr->mFriendNum; i++) {
            if (isSameFriend(&mgr->mUserData, friends, reqData)) {
                break;
            }
            friends++;
        }
        if (i < mgr->mFriendNum) {
            req->setResult(dNetFriend::NET_FRIEND_REQ_FIND_DATA, i);
        } else {
            req->setResult(dNetFriend::NET_FRIEND_REQ_FIND_DATA, -1);
        }
        break;
    case dNetFriend::NET_FRIEND_REQ_FIND_KEY:
        makeFriendData(req->mArgs.mFriendKey, &friendData);
        for (i = 0; i < mgr->mFriendNum; i++) {
            if (isSameFriend(&mgr->mUserData, friends, &friendData)) {
                break;
            }
            friends++;
        }
        if (i < mgr->mFriendNum) {
            req->setResult(dNetFriend::NET_FRIEND_REQ_FIND_KEY, i);
        } else {
            req->setResult(dNetFriend::NET_FRIEND_REQ_FIND_KEY, -1);
        }
        break;
    case dNetFriend::NET_FRIEND_REQ_IS_FRIEND:
        getMyFriendData(&req->mArgs.mUserData, &friendData);
        makeFriendData(req->mArgs.mAccountKey.mFriendKey, &friendData2);
        if (isSameFriend(&req->mArgs.mUserData, &friendData, &friendData2)) {
            req->setResult(dNetFriend::NET_FRIEND_REQ_IS_FRIEND, TRUE);
        } else {
            req->setResult(dNetFriend::NET_FRIEND_REQ_IS_FRIEND, FALSE);
        }
        break;
    case dNetFriend::NET_FRIEND_REQ_FINISH:
        req->finish();
        break;
    }

    sInstance->lock(3);
    if (mgr->mBusy) {
        u16 mask = dVoiceChat::getClientMask();
        if (mask != 0) {
            req->addConnect(mask);
            for (int j = 0; j < sInstance->mMemberNum; j++) {
                if (mask & (1 << j)) {
                    g_netDwc.disconnectVoice(j);
                }
            }
        }
    } else {
        u16 mask = req->takeConnect();
        for (int j = 0; j < sInstance->mMemberNum; j++) {
            if (mask & (1 << j)) {
                g_netDwc.connectVoice(j);
            }
        }
    }
    u16 mask = req->takeDisconnect();
    for (int j = 0; j < sInstance->mMemberNum; j++) {
        if (mask & (1 << j)) {
            g_netDwc.disconnectVoice(j);
        }
    }
    sInstance->unlock(3);

    if (!isStarted() && g_netDwc.mKeepaliveEnabled) {
        g_netDwc.mKeepaliveEnabled = 0;
    } else if (isStarted() && !g_netDwc.mKeepaliveEnabled) {
        g_netDwc.mKeepaliveEnabled = 1;
    }
}

void dNetMgr_c::setSendCount(u8 member, int count) {
    sInstance->lock(2);
    sInstance->mSendCounts[member] = count;
    if (count > sInstance->mSendCountMax[member]) {
        sInstance->mSendCountMax[member] = count;
    }
    sInstance->unlock(2);
}

dNetMgr_c g_netMgr;
dNetDwc::Session_c g_netDwc;
