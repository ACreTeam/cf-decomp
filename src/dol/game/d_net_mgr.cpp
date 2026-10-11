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
#include <revolution/SC.h>

dNetMgr_c *dNetMgr_c::sInstance;

dNetMgr_c::dNetMgr_c() {
    dNetFriendReq_c *req = &mReq;
    OSInitMutex(&req->mMutex);
    req->mKind = NET_FRIEND_REQ_NONE;
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
        mSendBufs = new (dHeap::netSoHeap_p, 4) dNetSendBuf_c[memberNum];
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
    if (fn_803AD87C()) {
        return FALSE;
    }
    if (!isUserDataValid(mUserDataSrc)) {
        return FALSE;
    }
    memcpy(&mUserData, mUserDataSrc, sizeof(DWCUserData));
    memcpy(mFriends, mFriendSrc, mFriendNum * sizeof(DWCFriendData));
    mStarted = 1;
    setVolume(fn_8010DA54());
    dVoiceChat::setMute(fn_8010DA28() ? 0 : 1);
    dVoiceChat::fn_80171C10(dSaveData_c::getRawExtra()->mVoiceLevel);
    if (!fn_8010D9FC()) {
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
    if (mgr->mInfo.mState >= 2 && !mgr->mInfo.m1A30) {
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
        g_netDwc.fn_800E7BE4();
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
                      &dPlayerMgr_c::getCurrentPlayer()->mFriends.mUserData,
                      dPlayerMgr_c::getCurrentPlayer()->mFriends.mKeys);
            dWifi::updateReq();
        }
        if (mState == NET_MGR_STATE_2 && mInfo.mState < 1) {
            mState = NET_MGR_STATE_SETUP;
        }
        if (dVoiceChat::isStarted() && !mBusy && !mTelopShown && mInfo.m1A32) {
            fn_801B66F4(0x11);
            mTelopShown = 1;
        }
    }
}

BOOL dNetMgr_c::fn_800E51C8(int arg) {
    if (mState == NET_MGR_STATE_5) {
        return TRUE;
    }
    if (!mReq.fn_800E723C(mMemberNum, arg)) {
        return FALSE;
    }
    mState = NET_MGR_STATE_5;
    return TRUE;
}

BOOL dNetMgr_c::fn_800E5230(int arg) {
    if (mState == NET_MGR_STATE_6) {
        return TRUE;
    }
    if (!mReq.fn_800E726C(mMemberNum, arg)) {
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
        return 0;
    }
    switch (mgr->mInfo.mError.mKind) {
    case 2:
        return 5;
    case 3:
        return 6;
    }
    if (mgr->mInfo.mError.getState() == 0xF) {
        return 1;
    }
    if (mgr->mInfo.mError.getState() == 0x10) {
        return 2;
    }
    switch (mgr->mInfo.mError.mCode) {
    case 0xB:
        return 1;
    case 0xC:
        return 2;
    case 0x17:
        return 3;
    }
    return 4;
}

void dNetMgr_c::fn_800E55B0() {
    if (sInstance->mState > NET_MGR_STATE_SETUP) {
        sInstance->mReq.fn_800E729C();
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
    dNetFriendStatus_c *status = &mInfo.mFriends[idx];
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
        sInstance->mReq.fn_800E7600();
    } else {
        dVoiceChat::setMute(1);
    }
}

void dNetMgr_c::unmute() {
    if (sInstance->mState > NET_MGR_STATE_SETUP) {
        sInstance->mReq.fn_800E7628();
    } else {
        dVoiceChat::setMute(0);
    }
}

void dNetMgr_c::initFriendList(DWCUserData *userData, DWCFriendData *friends,
                               u32 num) {
    fn_8033C2BC(userData, friends, num);
    fn_8033C63C(userData);
    for (u32 i = 0; i < num; i++) {
        if (fn_8033C28C(&friends[i])) {
            fn_8033BC84(&friends[i]);
        }
    }
    fn_8033CFE8(userData);
}

s64 dNetMgr_c::getInfo1A10() {
    return sInstance->mInfo.m1A10;
}

u32 dNetMgr_c::getInfo1A28() {
    return sInstance->mInfo.m1A28;
}

s64 dNetMgr_c::getFriend00(int idx) {
    return sInstance->mInfo.mFriends[idx].m00;
}

u32 dNetMgr_c::getFriend08(int idx) {
    return sInstance->mInfo.mFriends[idx].m08;
}

BOOL dNetMgr_c::getFriendFlag(int idx) {
    return sInstance->mInfo.mFriends[idx].mFlags & 1;
}

BOOL dNetMgr_c::isUserDataValid(const DWCUserData *userData) {
    if (fn_8033C2F4(userData) && fn_8033C600(userData)) {
        return TRUE;
    }
    return FALSE;
}

void dNetMgr_c::getMyFriendData(const DWCUserData *userData,
                                DWCFriendData *friendData) {
    fn_8033CA50(userData, friendData);
}

u64 dNetMgr_c::getMyFriendKey(const DWCUserData *userData) {
    return fn_8033C944(userData);
}

int dNetMgr_c::getMyProfileId(const DWCUserData *userData) {
    DWCFriendData friendData;
    fn_8033CA50(userData, &friendData);
    return getProfileId(userData, &friendData);
}

u64 dNetMgr_c::getFriendKey(const DWCFriendData *friendData) {
    return fn_8033C7B4(friendData);
}

int dNetMgr_c::getProfileId(const DWCUserData *userData,
                            const DWCFriendData *friendData) {
    int id = fn_8033C7DC(userData, friendData);
    return id > 0 ? id : 0;
}

void dNetMgr_c::makeFriendData(u64 key, DWCFriendData *friendData) {
    fn_8033C9D8(friendData, key);
}

BOOL dNetMgr_c::isFriendDataValid(const DWCFriendData *friendData) {
    return fn_8033C28C(friendData);
}

BOOL dNetMgr_c::isSameFriend(const DWCUserData *userData,
                             const DWCFriendData *a, const DWCFriendData *b) {
    if (!isFriendDataValid(a)) {
        return FALSE;
    }
    if (!isFriendDataValid(b)) {
        return FALSE;
    }
    if (fn_8033CCDC(a, b)) {
        return TRUE;
    }
    int kindA = fn_8033BC40(a);
    int kindB = fn_8033BC40(b);
    if ((kindA == 2 && kindB == 3) || (kindB == 2 && kindA == 3)) {
        int idA = fn_8033C7DC(userData, a);
        int idB = fn_8033C7DC(userData, b);
        if (idA > 0 && idB > 0 && idA == idB) {
            return TRUE;
        }
    }
    return FALSE;
}

int dNetMgr_c::checkFriendKey(const DWCUserData *userData, u64 key) {
    return fn_8033BCB8(userData, key);
}

BOOL dNetMgr_c::reqMyData(const DWCUserData *userData) {
    return sInstance->mReq.fn_800E7348(userData);
}

BOOL dNetMgr_c::reqKeyData(u64 key) {
    return sInstance->mReq.fn_800E738C(key);
}

BOOL dNetMgr_c::reqIsFriend(const DWCUserData *userData, u64 key) {
    return sInstance->mReq.fn_800E7474(userData, key);
}

BOOL dNetMgr_c::reqCheckKey(u64 key) {
    return sInstance->mReq.fn_800E73BC(key);
}

BOOL dNetMgr_c::reqRegister(int index, const DWCFriendData *data) {
    return sInstance->mReq.fn_800E72C4(index, data);
}

BOOL dNetMgr_c::reqRemove(int index) {
    return sInstance->mReq.fn_800E731C(index);
}

BOOL dNetMgr_c::reqFindUser(const DWCUserData *userData) {
    return sInstance->mReq.fn_800E73EC(userData);
}

BOOL dNetMgr_c::reqFindData(const DWCFriendData *data) {
    return sInstance->mReq.fn_800E7430(data);
}

void dNetMgr_c::onStatus() {
    dNetMgr_c *mgr = sInstance;
    if (mgr->mStatusSize != 0) {
        fn_8031A724(mgr->mStatus, mgr->mStatusSize);
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
    u8 myAid = fn_8031E9C4();
    if (aid < myAid) {
        fn_80339AF0(aid, mRecvBufs[aid], mRecvBufSize);
    } else if (aid > myAid) {
        fn_80339AF0(aid, mRecvBufs[aid - 1], mRecvBufSize);
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
    dNetFriendReq_c *req = &sInstance->mReq;
    switch (req->getKind()) {
    case NET_FRIEND_REQ_1:
        if (g_netDwc.fn_800E84C0(req->mArgs.mMatch.mAid)) {
            req->finish();
        }
        break;
    case NET_FRIEND_REQ_2:
        if (g_netDwc.fn_800E85CC(req->mArgs.mMatch.mAid,
                                 req->mArgs.mMatch.mArg)) {
            req->finish();
        }
        break;
    case NET_FRIEND_REQ_3:
        if (g_netDwc.fn_800E86BC(req->mArgs.mMatch.mAid,
                                 req->mArgs.mMatch.mValue)) {
            req->finish();
        }
        break;
    case NET_FRIEND_REQ_4:
        g_netDwc.fn_800E8820();
        req->finish();
        break;
    case NET_FRIEND_REQ_5:
        g_netDwc.fn_800E8864();
        req->finish();
        break;
    case NET_FRIEND_REQ_6:
        g_netDwc.fn_800E8B5C();
        req->finish();
        break;
    }

    sInstance->lock(1);
    if (sInstance->m3863) {
        g_netDwc.fn_800E8910();
    } else {
        g_netDwc.fn_800E8918();
    }
    sInstance->unlock(1);

    sInstance->lock(3);
    if (!sInstance->mVoiceOff && dVoiceChat::isStarted()) {
        g_netDwc.fn_800E8DF8(FALSE);
    } else if (sInstance->mVoiceOff && !dVoiceChat::isStarted()) {
        g_netDwc.fn_800E8DF8(TRUE);
        for (int i = 0; i < sInstance->mMemberNum; i++) {
            dPrivateData_c *player = dPlayerMgr_c::getNetPlayerRaw(i);
            if (player != NULL && player->mPID.isValid() &&
                !player->getOption()->mBit1) {
                sInstance->mReq.connect(i);
            }
        }
    }
    sInstance->unlock(3);
    sInstance->mRemovedFriends = 0;
}

void dNetMgr_c::procRequest() {
    dNetMgr_c *mgr = sInstance;
    dNetFriendReq_c *req = &mgr->mReq;
    DWCFriendData *friends = mgr->mFriends;
    DWCFriendData *reqData = &req->mArgs.mFriend.mData;

    if (fn_8031A7BC()) {
        int index = req->mArgs.mFriend.mIndex;
        switch (req->getKind()) {
        case NET_FRIEND_REQ_REMOVE:
            fn_8031A7EC(&friends[index]);
            mgr->mRemovedFriends |= 1 << index;
            g_netDwc.fn_800E88A8();
            req->setResult(NET_FRIEND_REQ_REMOVE, index);
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
        case NET_FRIEND_REQ_REGISTER:
            if (isFriendDataValid(reqData)) {
                if (!isSameFriend(&mgr->mUserData, reqData, &friends[index])) {
                    memcpy(&friends[index], reqData, sizeof(DWCFriendData));
                    g_netDwc.fn_800E88A8();
                }
                req->setResult(NET_FRIEND_REQ_REGISTER, index);
            } else {
                req->setResult(NET_FRIEND_REQ_REGISTER, -1);
            }
            break;
        }
    }

    DWCFriendData friendData;
    DWCFriendData friendData2;
    u32 i;
    switch (req->getKind()) {
    case NET_FRIEND_REQ_VOICE_PARAM_ON:
        dVoiceChat::setParam(1);
        req->finish();
        break;
    case NET_FRIEND_REQ_VOICE_PARAM_OFF:
        dVoiceChat::setParam(0);
        req->finish();
        break;
    case NET_FRIEND_REQ_MUTE_ON:
        dVoiceChat::setMute(1);
        req->finish();
        break;
    case NET_FRIEND_REQ_MUTE_OFF:
        dVoiceChat::setMute(0);
        req->finish();
        break;
    case NET_FRIEND_REQ_VOICE_B_ON:
        if (dVoiceChat::fn_80171C08(1)) {
            dSaveData_c::getExtra()->setVoiceSetting(1);
        }
        req->finish();
        break;
    case NET_FRIEND_REQ_VOICE_B_OFF:
        if (dVoiceChat::fn_80171C08(0)) {
            dSaveData_c::getExtra()->setVoiceSetting(0);
        }
        req->finish();
        break;
    case NET_FRIEND_REQ_VOICE_C: {
        u8 level = req->mArgs.mParam;
        if (dVoiceChat::fn_80171C10(level)) {
            dSaveData_c::getExtra()->mVoiceLevel = level;
        }
        req->finish();
        break;
    }
    case NET_FRIEND_REQ_MY_DATA:
        getMyFriendData(&req->mArgs.mUserData, &friendData);
        req->setResultData(NET_FRIEND_REQ_MY_DATA, &friendData);
        break;
    case NET_FRIEND_REQ_MY_KEY:
        req->setResultKey(NET_FRIEND_REQ_MY_KEY,
                          getMyFriendKey(&req->mArgs.mUserData));
        break;
    case NET_FRIEND_REQ_KEY:
        req->setResultKey(NET_FRIEND_REQ_KEY, getFriendKey(reqData));
        break;
    case NET_FRIEND_REQ_KEY_DATA:
        makeFriendData(req->mArgs.mFriendKey, &friendData);
        req->setResultData(NET_FRIEND_REQ_KEY_DATA, &friendData);
        break;
    case NET_FRIEND_REQ_CHECK_KEY:
        req->setResult(NET_FRIEND_REQ_CHECK_KEY,
                       checkFriendKey(&mgr->mUserData, req->mArgs.mFriendKey));
        break;
    case NET_FRIEND_REQ_FIND_USER:
        getMyFriendData(&req->mArgs.mUserData, &friendData);
        for (i = 0; i < mgr->mFriendNum; i++) {
            if (isSameFriend(&mgr->mUserData, friends, &friendData)) {
                break;
            }
            friends++;
        }
        if (i < mgr->mFriendNum) {
            req->setResult(NET_FRIEND_REQ_FIND_USER, i);
        } else {
            req->setResult(NET_FRIEND_REQ_FIND_USER, -1);
        }
        break;
    case NET_FRIEND_REQ_FIND_DATA:
        for (i = 0; i < mgr->mFriendNum; i++) {
            if (isSameFriend(&mgr->mUserData, friends, reqData)) {
                break;
            }
            friends++;
        }
        if (i < mgr->mFriendNum) {
            req->setResult(NET_FRIEND_REQ_FIND_DATA, i);
        } else {
            req->setResult(NET_FRIEND_REQ_FIND_DATA, -1);
        }
        break;
    case NET_FRIEND_REQ_FIND_KEY:
        makeFriendData(req->mArgs.mFriendKey, &friendData);
        for (i = 0; i < mgr->mFriendNum; i++) {
            if (isSameFriend(&mgr->mUserData, friends, &friendData)) {
                break;
            }
            friends++;
        }
        if (i < mgr->mFriendNum) {
            req->setResult(NET_FRIEND_REQ_FIND_KEY, i);
        } else {
            req->setResult(NET_FRIEND_REQ_FIND_KEY, -1);
        }
        break;
    case NET_FRIEND_REQ_IS_FRIEND:
        getMyFriendData(&req->mArgs.mUserData, &friendData);
        makeFriendData(req->mArgs.mFriendKey2, &friendData2);
        if (isSameFriend(&req->mArgs.mUserData, &friendData, &friendData2)) {
            req->setResult(NET_FRIEND_REQ_IS_FRIEND, TRUE);
        } else {
            req->setResult(NET_FRIEND_REQ_IS_FRIEND, FALSE);
        }
        break;
    case NET_FRIEND_REQ_14:
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
                    g_netDwc.fn_800E8DF0(j);
                }
            }
        }
    } else {
        u16 mask = req->takeConnect();
        for (int j = 0; j < sInstance->mMemberNum; j++) {
            if (mask & (1 << j)) {
                g_netDwc.fn_800E8DA4(j);
            }
        }
    }
    u16 mask = req->takeDisconnect();
    for (int j = 0; j < sInstance->mMemberNum; j++) {
        if (mask & (1 << j)) {
            g_netDwc.fn_800E8DF0(j);
        }
    }
    sInstance->unlock(3);

    if (!isStarted() && g_netDwc.mVoiceOn) {
        g_netDwc.mVoiceOn = 0;
    } else if (isStarted() && !g_netDwc.mVoiceOn) {
        g_netDwc.mVoiceOn = 1;
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
dNetDwc_c g_netDwc;
