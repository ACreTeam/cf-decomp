#include <egg/core/eggHeap.h>
#include <game/game/d_ds_dl.hpp>
#include <game/game/d_net_dwc.hpp>
#include <game/game/d_net_mgr.hpp>
#include <game/game/d_net_wc24.hpp>
#include <game/game/d_thunder.hpp>
#include <game/game/d_voice_chat.hpp>
#include <game/mLib/m_perf.hpp>
#include <revolution/OS/OSTime.h>
#include <revolution/PMIC/pmic.h>
#include <revolution/SO.h>
#include <revolution/VI.h>
#include <stddef.h>
#include <string.h>

// Narrow interfaces to the unscaffolded VCT and socket libraries.
// Signatures are inferred from the calls; unused callback arguments remain
// unnamed.
extern "C" {
BOOL VCT_HandleData(u8, const void *, u32);
int NETGetStartupErrorCode();
}

namespace dNetDwc {
static dDsDl::Mutex_c sSocketMutex;
static dDsDl::Mutex_c sDwcMutex;
static u8 sThreadStack[THREAD_STACK_SIZE];
static EGG::Heap *sSocketHeap;
static EGG::Heap *sDwcHeap;
static int sMicResult;
Session_c *Session_c::sInstance;

// 800E7650
void *allocSocket(int, u32 size) {
    OSLockMutex(&sSocketMutex.mMutex);
    void *memory = sSocketHeap->alloc(size, 32);
    OSUnlockMutex(&sSocketMutex.mMutex);
    return memory;
}
// 800E76B8
void freeSocket(int, void *memory) {
    if (memory) {
        OSLockMutex(&sSocketMutex.mMutex);
        sSocketHeap->free(memory);
        OSUnlockMutex(&sSocketMutex.mMutex);
    }
}
// 800E771C
void *allocDwc(int, u32 size, int align) {
    OSLockMutex(&sDwcMutex.mMutex);
    void *memory = sDwcHeap->alloc(size, align);
    OSUnlockMutex(&sDwcMutex.mMutex);
    return memory;
}
// 800E7790
void freeDwc(int, void *memory, u32) {
    if (memory) {
        OSLockMutex(&sDwcMutex.mMutex);
        sDwcHeap->free(memory);
        OSUnlockMutex(&sDwcMutex.mMutex);
    }
}
// 800E77F4
Session_c::Session_c()
    : mMatchStart(0), mUserData(0), mFriends(0), mData(0), mSendBufs(0),
      mSendHead(0) {
    mStopRequested = 0;
    mInVoiceCalc = 0;
    mState = STATE_NONE;
    _36C = 0;
    mKeepaliveTimer = 0;
    mMemberBits = 0;
    mFriendNum = 0;
    mUpdatingFriends = 0;
    mKeepaliveEnabled = 0;
    mVoiceEnabled = 0;
    mMatchClosed = 0;
    mChangingMatchClosed = 0;
    sInstance = this;
}
// 800E799C
Session_c::~Session_c() {
    sInstance = 0;
}
// 800E79E4
void Session_c::init(EGG::Heap *soHeap, EGG::Heap *dwcHeap,
                     dNetFriend::Data_c *data) {
    if (mState < STATE_INITIALIZED) {
        sSocketHeap = soHeap;
        sDwcHeap = dwcHeap;
        DWC_Init(1, "acrossingwii", GAME_CODE, allocDwc, freeDwc);
        DWC_SetReportLevel(0);
        mData = data;
        setState(STATE_INITIALIZED);
    }
}
// 800E7A6C
void Session_c::cleanup() {
    if (mState > STATE_NONE) {
        DWC_Shutdown();
        sSocketHeap = 0;
        sDwcHeap = 0;
        setState(STATE_NONE);
    }
}
// 800E7ABC
void Session_c::setAccount(DWCUserData *data, DWCFriendData *friends, u8 num,
                           dNetFriend::SendBuf_c *bufs) {
    mUserData = data;
    mFriends = friends;
    mFriendNum = num;
    mSendBufs = bufs;
    mUpdatingFriends = 0;
}
// 800E7AD8
void Session_c::reset() {
    mUserData = 0;
    mFriends = 0;
    mFriendNum = 0;
    mSendBufs = 0;
    mStopRequested = 0;
    mError.clear();
}
// 800E7AF8
void Session_c::start(DWCUserData *data, DWCFriendData *friends, u8 num,
                      dNetFriend::SendBuf_c *bufs, u8 voice) {
    if (OSIsThreadTerminated(&mThread)) {
        setAccount(data, friends, num, bufs);
        mStopRequested = 0;
        mMemberBits = 0;
        _36C = 0;
        mMatchClosed = 0;
        mChangingMatchClosed = 0;
        mInVoiceCalc = 0;
        mKeepaliveTimer = 0;
        mKeepaliveEnabled = 1;
        mVoiceEnabled = voice;
        publishStatus();
        OSCreateThread(&mThread, threadMain, 0,
                       sThreadStack + THREAD_STACK_SIZE, THREAD_STACK_SIZE,
                       THREAD_PRIORITY, OS_THREAD_DETACHED);
        mPerf::setThreadColor(&mThread, dPerf::getThreadColor(4));
        OSResumeThread(&mThread);
    }
}
// 800E7BE4
void Session_c::requestStop() {
    if (!OSIsThreadTerminated(&mThread)) {
        mStopRequested = 1;
    }
}
// 800E7C20
void *Session_c::threadMain(void *) {
    OSCalendarTime startTime;
    OSTicksToCalendarTime(OSGetTime(), &startTime);
    Session_c *session = sInstance;
    session->setState(STATE_SCHEDULER_SUSPENDED);
    session->publishStatus();
    if (session->suspendScheduler()) {
        session->setState(STATE_MIC_OPEN);
        session->publishStatus();
        void *micWork = sSocketHeap->alloc(MIC_WORK_SIZE, 4);
        if (session->openMic(micWork)) {
            session->setState(STATE_SOCKET_INITIALIZED);
            session->publishStatus();
            if (session->startSockets() && !session->mStopRequested) {
                DWC_InitFriendsMatch(
                    0, session->mUserData, GAME_PRODUCT_ID, "acrossingwii",
                    "Z7Fm9K", TRANSPORT_BUFFER_SIZE, TRANSPORT_BUFFER_SIZE,
                    session->mFriends, session->mFriendNum);
                DWC_SetSendSplitMax(TRANSPORT_PACKET_SIZE);
                // Login uses a blank four-code-unit nickname.
                if (DWC_LoginAsync(L"\0\0\0", 0, onLogin, 0)) {
                    session->setState(STATE_LOGIN);
                    for (;;) {
                        session->execute();
                        session->publishStatus();
                        if (!sInstance->mUpdatingFriends &&
                            session->mStopRequested) {
                            OSCalendarTime endTime;
                            OSTicksToCalendarTime(OSGetTime(), &endTime);
                            session->closeConnections();
                            session->cancelMatch();
                            session->clearConnections();
                            break;
                        }
                        VIWaitForRetrace();
                    }
                    DWC_ClearDirtyFlag(session->mUserData);
                } else {
                    session->setError(ERROR_LOGIN, ERROR_RESTART_REQUIRED, 0,
                                      0);
                }
                DWC_ShutdownFriendsMatch();
                session->setState(STATE_DWC_READY);
            }
            session->stopSockets();
            session->closeMic();
            session->setState(STATE_MIC_OPEN);
        }
        sSocketHeap->free(micWork);
        session->resumeScheduler();
        session->setState(STATE_SCHEDULER_SUSPENDED);
    }
    mPerf::resetThreadColor(&session->mThread);
    session->setState(STATE_INITIALIZED);
    session->publishStatus();
    return 0;
}
// 800E7E54
void Session_c::setState(int state) {
    if (state != mState) {
        mState = state;
        if (state == STATE_ONLINE) {
            clearConnections();
            dNetMgr_c::fn_800E5C7C();
        }
    }
}
// 800E7E90
BOOL Session_c::openMic(void *work) {
    if (work) {
        sMicResult = fn_80225BC0(work);
        if (!sMicResult) {
            return TRUE;
        }
    }
    sMicResult = MIC_UNAVAILABLE;
    return TRUE;
}
// 800E7EDC
BOOL Session_c::closeMic() {
    int retries = MIC_SHUTDOWN_RETRIES;
    do {
        dVoiceChat::update();
        if (dVoiceChat::isMicShutDown()) {
            break;
        }
        OSSleepTicks((u64)(OS_TIME_SPEED / 1000) * MIC_RETRY_DELAY_MS);
    } while (--retries > 0);
    retries = MIC_SHUTDOWN_RETRIES;
    do {
        if (fn_80225EBC() != PMIC_ERR_BUSY) {
            break;
        }
        OSSleepTicks((u64)(OS_TIME_SPEED / 1000) * MIC_RETRY_DELAY_MS);
    } while (--retries > 0);
    sMicResult = 0;
    return TRUE;
}
// 800E7FA8
BOOL Session_c::suspendScheduler() {
    int result;
    int retries = SCHEDULER_RETRIES;
    do {
        result = dNetWc24::dNetWc24_c::trySuspendScheduler();
        if (result != NWC24_ERR_BUSY) {
            break;
        }
        OSSleepTicks((s64)(OS_TIME_SPEED / 1000) * SCHEDULER_RETRY_DELAY_MS);
    } while (--retries > 0);
    if (result >= 0) {
        return TRUE;
    }
    setError(ERROR_SCHEDULER_SUSPEND, ERROR_RESTART_REQUIRED, 0, 0);
    return FALSE;
}
// 800E8058
BOOL Session_c::resumeScheduler() {
    if (dNetWc24::dNetWc24_c::resumeScheduler() >= 0) {
        return TRUE;
    }
    setError(ERROR_SCHEDULER_RESUME, ERROR_RESTART_REQUIRED, 0, 0);
    return FALSE;
}
// 800E80B0
BOOL Session_c::startSockets() {
    SOLibraryConfig allocators;
    memset(&allocators, 0, sizeof(allocators));
    allocators.alloc = allocSocket;
    allocators.free = freeSocket;
    int result = SOInit(&allocators);
    if (result != 0 && result != -7) {
        setError(ERROR_SOCKET_INIT, ERROR_RESTART_REQUIRED, 0, result);
        return FALSE;
    }
    setState(STATE_SOCKET_STARTED);
    result = SOStartup();
    if (result != 0 && result != -7) {
        setError(ERROR_SOCKET_START, ERROR_RESTART_REQUIRED, NETGetStartupErrorCode(),
                 result);
        return FALSE;
    }
    setState(STATE_DWC_READY);
    return TRUE;
}
// 800E8198
BOOL Session_c::stopSockets() {
    if (mState > STATE_SOCKET_STARTED) {
        int result = SOCleanup();
        if (result == 0 || result == -7) {
            setState(STATE_SOCKET_STARTED);
        } else {
            setError(ERROR_SOCKET_STOP, ERROR_PERMANENT, 0, result);
            return FALSE;
        }
    }
    if (mState > STATE_SOCKET_INITIALIZED) {
        int result = SOFinish();
        if (result == 0 || result == -7) {
            setState(STATE_SOCKET_INITIALIZED);
        } else {
            setError(ERROR_SOCKET_CLEANUP, ERROR_PERMANENT, 0, result);
            return FALSE;
        }
    }
    return TRUE;
}
// 800E8264
void Session_c::execute() {
    if (mState == STATE_MATCHING && DWC_GetLastMatchingType() == 3 &&
        (u64)(OSGetTime() - mMatchStart) / OS_MSEC_TO_TICKS(1) >
            MATCH_TIMEOUT_MS) {
        cancelMatch();
        setError(ERROR_MATCH_TIMEOUT, ERROR_RECOVERABLE, 0, 0);
    }
    dNetMgr_c::procDwcRequest();
    sendPending();
    DWC_ProcessFriendsMatch();
    if (sMicResult != MIC_UNAVAILABLE) {
        sMicResult = dVoiceChat::update();
    }
    if (mVoiceEnabled) {
        mInVoiceCalc = 1;
        dVoiceChat::calc();
        mInVoiceCalc = 0;
    }
    dNetMgr_c::procRequest();
    int detail, kind;
    int error = DWC_GetLastErrorEx(&detail, &kind);
    if (error) {
        if (kind >= 1) {
            u8 priority;
            if (kind == 7) {
                priority = ERROR_PERMANENT;
            } else if (kind <= 2) {
                priority = ERROR_RECOVERABLE;
            } else {
                priority = ERROR_RESTART_REQUIRED;
            }
            setError(ERROR_DWC_PROCESS, priority, detail, error);
            if (mState == STATE_LOGIN) {
                DWC_ClearDirtyFlag(mUserData);
            }
        }
        DWC_ClearError();
    }
    if (mKeepaliveEnabled && ++mKeepaliveTimer >= KEEPALIVE_FRAMES) {
        mKeepaliveTimer = 0;
        if (DWC_IsServerMyself()) {
            for (int aid = 0; aid < MEMBER_COUNT; aid++) {
                if (DWC_IsValidAID(aid)) {
                    if (mKeepaliveCount[aid] >= KEEPALIVE_LIMIT) {
                        DWC_CloseConnectionHard(aid);
                    } else {
                        DWC_Ping(aid);
                        mKeepaliveCount[aid]++;
                    }
                }
            }
        } else {
            u8 aid = DWC_GetServerAID();
            if (aid != 0xFF && aid != DWC_GetMyAID()) {
                if (mKeepaliveCount[aid] >= KEEPALIVE_LIMIT) {
                    DWC_CloseConnectionHard(aid);
                } else {
                    DWC_Ping(aid);
                    mKeepaliveCount[aid]++;
                }
            }
        }
    }
}
// 800E84C0
BOOL Session_c::matchAnybody(u8) {
    if (mError.mState) {
        dNetMgr_c::fn_800E5C7C();
        return TRUE;
    }
    if (mState == STATE_LOGIN) {
        return FALSE;
    }
    if (mState == STATE_ONLINE) {
        if (mUpdatingFriends) {
            return FALSE;
        }
        DWC_AddMatchKeyString(0, "str_key", "rvl_forest");
        if (DWC_ConnectToAnybodyAsync(2, 4, "str_key='rvl_forest'", onMatched,
                                      0, onMatchProgress, 0, 0, 0, canMatch, 0,
                                      0)) {
            setState(STATE_MATCHING);
            return TRUE;
        }
    }
    setError(ERROR_ANYBODY_MATCH, ERROR_RECOVERABLE, 0, 0);
    dNetMgr_c::fn_800E5C7C();
    return TRUE;
}
// 800E85CC
BOOL Session_c::hostMatch(u8, u8) {
    if (mError.mState) {
        dNetMgr_c::fn_800E5C7C();
        return TRUE;
    }
    if (mState == STATE_LOGIN) {
        return FALSE;
    }
    if (mState == STATE_ONLINE) {
        if (mUpdatingFriends) {
            return FALSE;
        }
        if (DWC_SetupGameServer(2, 4, onMatched, 0, onMatchProgress, 0,
                                canMatch, 0, 0)) {
            mMatchClosed = 0;
            setState(STATE_MATCHING);
            return TRUE;
        }
    }
    setError(ERROR_HOST_MATCH, ERROR_RECOVERABLE, 0, 0);
    dNetMgr_c::fn_800E5C7C();
    return TRUE;
}
// 800E86BC
BOOL Session_c::matchFriend(u8, u32 index) {
    if (mError.mState) {
        dNetMgr_c::fn_800E5C7C();
        return TRUE;
    }
    if (mState == STATE_LOGIN) {
        return FALSE;
    }
    if (mState == STATE_ONLINE) {
        if (mUpdatingFriends) {
            return FALSE;
        }
        switch (checkFriend(index)) {
        case FRIEND_OFFLINE:
            setError(ERROR_FRIEND_OFFLINE, ERROR_RECOVERABLE, 0, 0);
            dNetMgr_c::fn_800E5C7C();
            return TRUE;
        case FRIEND_FULL:
            setError(ERROR_FRIEND_FULL, ERROR_RECOVERABLE, 0, 0);
            dNetMgr_c::fn_800E5C7C();
            return TRUE;
        }
        if (DWC_ConnectToGameServerAsync(2, index, onMatched, 0,
                                         onMatchProgress, 0, canMatch, 0, 0)) {
            mMatchStart = OSGetTime();
            setState(STATE_MATCHING);
            return TRUE;
        }
    }
    setError(ERROR_FRIEND_MATCH, ERROR_RECOVERABLE, 0, 0);
    dNetMgr_c::fn_800E5C7C();
    return TRUE;
}
// 800E8820
void Session_c::cancelMatch() {
    if (mState == STATE_MATCHING) {
        DWC_CancelMatching();
        setState(STATE_ONLINE);
    }
}
// 800E8864
void Session_c::closeConnections() {
    if (mState >= STATE_CONNECTED) {
        DWC_CloseAllConnectionsHard();
        setState(STATE_ONLINE);
    }
}
// 800E88A8
void Session_c::updateFriends() {
    if (mState >= STATE_ONLINE) {
        DWC_UpdateServersAsync(0, onFriendsUpdated, 0, onFriendDeleted, 0,
                               onBuddyFriend, 0);
        mUpdatingFriends = 1;
    }
}
// 800E8910
void Session_c::setBusy() {
    setMatchClosed(TRUE);
}
// 800E8918
void Session_c::clearBusy() {
    setMatchClosed(FALSE);
}
// 800E8920
BOOL Session_c::setMatchClosed(BOOL closed) {
    if ((u32)closed != mMatchClosed && !mChangingMatchClosed &&
        (mState >= STATE_CONNECTED ||
         (mState == STATE_MATCHING && DWC_IsServerMyself()))) {
        if (DWC_RequestSuspendMatchAsync(closed, onMatchClosed, 0)) {
            mMatchClosed = closed;
            mChangingMatchClosed = 1;
            return TRUE;
        }
    }
    return FALSE;
}
// 800E89C0
void Session_c::sendPending() {
    if (mState >= STATE_CONNECTED) {
        for (SendNode_c *node = mSendHead; node; node = node->mNext) {
            if (mSendBufs[node->mAid].hasPending() &&
                DWC_IsSendableReliable(node->mAid) &&
                DWC_SendReliable(node->mAid, mSendBufs[node->mAid].mData,
                                 mSendBufs[node->mAid].mLength)) {
                mSendBufs[node->mAid].clearPending();
                rotateSendOrder(node);
                break;
            }
        }
    }
}
// 800E8A84
void Session_c::initSendOrder(u8 ownAid) {
    u8 aid = 0;
    // Link the interior nodes, then terminate the tail separately.
    for (int i = 0; i < MEMBER_COUNT - 2; i++) {
        if (aid == ownAid) {
            aid++;
        }
        mSendOrder[i].mAid = aid++;
        mSendOrder[i].mNext = &mSendOrder[i + 1];
    }
    if (aid == ownAid) {
        aid++;
    }
    mSendOrder[MEMBER_COUNT - 2].mAid = aid;
    mSendOrder[MEMBER_COUNT - 2].mNext = 0;
    mSendHead = mSendOrder;
}
// 800E8AF4
void Session_c::rotateSendOrder(SendNode_c *node) {
    SendNode_c *prev = mSendHead;
    if (node == prev) {
        mSendHead = node->mNext;
        prev = node->mNext;
    } else {
        while (prev->mNext != node) {
            prev = prev->mNext;
        }
        prev->mNext = node->mNext;
    }
    while (prev->mNext) {
        prev = prev->mNext;
    }
    prev->mNext = node;
    node->mNext = 0;
}
// 800E8B50
void Session_c::setError(u8 state, u8 kind, int detail, int code) {
    mError.set(state, kind, -detail, code);
}
// 800E8B5C
BOOL Session_c::clearError() {
    if (mError.mKind >= ERROR_PERMANENT) {
        return FALSE;
    }
    if (mError.mKind >= ERROR_RESTART_REQUIRED && mState > STATE_INITIALIZED) {
        return FALSE;
    }
    mError.clear();
    return TRUE;
}
// 800E8BB4
void Session_c::clearConnections() {
    DWC_SetUserRecvTimeoutCallback(0, 0);
    mMemberBits = 0;
    for (int aid = 0; aid < MEMBER_COUNT; aid++) {
        mSendBufs[aid].reset();
        mKeepaliveCount[aid] = 0;
    }
    mSendHead = 0;
    cleanupVoice();
}
// 800E8C44
void Session_c::publishStatus() {
    bool micFailed = sMicResult == MIC_UNAVAILABLE;
    BOOL micActive = dVoiceChat::isMicActive();
    mData->updateSession(mState, &mError, mUserData, mFriends, mMemberBits,
                         mUpdatingFriends, micActive, micFailed);
}
// 800E8CB0
int Session_c::checkFriend(u32 index) {
    u8 maximum, current;
    int statusSize;
    if (DWC_GetFriendStatusDataSC(&mFriends[index], &maximum, &current, 0,
                                  &statusSize) != 6) {
        return FRIEND_OFFLINE;
    }
    // Signed choices preserve the arithmetic selection used by MWCC.
    return current >= maximum ? (int)FRIEND_FULL : (int)FRIEND_AVAILABLE;
}
// 800E8D18
void Session_c::initVoice(u8 aid) {
    dVoiceChat::init(aid);
    if (mVoiceEnabled) {
        startVoice();
    }
}
// 800E8D5C
void Session_c::cleanupVoice() {
    dVoiceChat::cleanup();
}
// 800E8D60
void Session_c::startVoice() {
    dVoiceChat::start();
}
// 800E8D64
void Session_c::stopVoice() {
    for (u32 aid = 0; aid < MEMBER_COUNT; aid++) {
        dVoiceChat::disconnect(aid);
    }
    dVoiceChat::stop();
}
// 800E8DA4
void Session_c::connectVoice(u8 aid) {
    if (mVoiceEnabled && DWC_IsValidAID(aid)) {
        dVoiceChat::connect(aid);
    }
}
// 800E8DF0
void Session_c::disconnectVoice(u8 aid) {
    dVoiceChat::disconnect(aid);
}
// 800E8DF8
void Session_c::setVoiceEnabled(BOOL enabled) {
    if (mVoiceEnabled != (u32)enabled) {
        mVoiceEnabled = enabled;
        if (mVoiceEnabled) {
            startVoice();
        } else {
            stopVoice();
        }
    }
}
// 800E8E1C
void Session_c::onLogin(int error, int, void *) {
    Session_c *session = sInstance;
    if (!error) {
        DWC_SetConnectionClosedCallback(onClosed, 0);
        DWC_SetUserSendCallback(onSend, 0);
        DWC_SetUserRecvCallback(onReceive, 0);
        DWC_SetUserPingCallback(onPing, 0);
        DWC_SetConnectionKeepAliveTime(MATCH_TIMEOUT_MS);
        DWC_SetBuddyFriendCallback(onBuddyFriendOnline, 0);
        session->setState(STATE_ONLINE);
        dNetMgr_c::onStatus();
        DWC_ClearDirtyFlag(session->mUserData);
        if (!session->mStopRequested) {
            session->updateFriends();
        }
    }
}
// 800E8ED4
void Session_c::onFriendsUpdated(int, BOOL, void *) {
    sInstance->mUpdatingFriends = 0;
}
// 800E8EE4
void Session_c::onBuddyFriend(int index, void *) {
    dNetMgr_c::setFriendRemoved(index);
}
// 800E8EE8
void Session_c::onBuddyFriendOnline(int, void *) {
}
// 800E8EEC
void Session_c::onFriendDeleted(int deletedIndex, int sourceIndex, void *) {
    dNetMgr_c::onEvent(deletedIndex, sourceIndex);
}
// 800E8EF0
int Session_c::canMatch(int, void *) {
    return dNetMgr_c::isRequestIdle() != 0;
}
// 800E8F1C
void Session_c::onMatchClosed(int, void *) {
    sInstance->mChangingMatchClosed = 0;
}
// 800E8F2C
void Session_c::onMatchProgress(int, int, void *) {
}
// 800E8F30
void Session_c::onMatched(int error, BOOL cancelled, BOOL self, BOOL isServer,
                          int, int, void *) {
    Session_c *session = sInstance;
    if (!error) {
        if (!cancelled) {
            u8 own = DWC_GetMyAID();
            if (!(session->mMemberBits & (1 << own))) {
                session->mMemberBits |= 1 << own;
                session->initSendOrder(own);
                session->initVoice(own);
            }
            for (u32 aid = 0; aid < MEMBER_COUNT; aid++) {
                if (DWC_IsValidAID(aid) &&
                    !(session->mMemberBits & (1 << (u8)aid))) {
                    session->mMemberBits |= 1 << (u8)aid;
                    session->mSendBufs[(u8)aid].reset();
                    dNetMgr_c::onJoin(aid);
                }
            }
            session->setState(STATE_CONNECTED);
        } else if (self) {
            session->setState(STATE_ONLINE);
        } else if (isServer) {
            session->setError(ERROR_MATCH_TIMEOUT, ERROR_RECOVERABLE, 0, 0);
            session->setState(STATE_ONLINE);
        }
    } else if (session->mState < STATE_CONNECTED) {
        session->setState(STATE_ONLINE);
    }
}
// 800E907C
void Session_c::onClosed(int, BOOL, BOOL, u8 aid, int, void *) {
    Session_c *session = sInstance;
    session->mMemberBits &= ~(1 << aid);
    session->mSendBufs[aid].reset();
    session->mKeepaliveCount[aid] = 0;
    session->_35C[(u8)aid] = 0;
    session->disconnectVoice(aid);
    dNetMgr_c::onLeave(aid);
    if (DWC_GetNumConnectionHost() < 2) {
        if (DWC_GetLastMatchingType() == 2) {
            session->clearConnections();
            session->setState(STATE_MATCHING);
        } else {
            DWC_CloseAllConnectionsHard();
            session->setState(STATE_ONLINE);
        }
    }
}
// 800E9140
void Session_c::onSend(u32, u8 aid, void *) {
    if (!sInstance->mInVoiceCalc) {
        sInstance->mSendBufs[aid].onSent();
    }
}
// 800E9164
void Session_c::onReceive(u8 aid, const void *data, u32 size, void *) {
    sInstance->mKeepaliveCount[aid] = 0;
    if (!VCT_HandleData(aid, data, size)) {
        dNetMgr_c::onRecv(aid, data, size);
    }
}
// 800E91D4
void Session_c::onPing(int latency, u8 aid, void *) {
    sInstance->mKeepaliveCount[aid] = 0;
    // This manager slot records the ping latency in milliseconds.
    dNetMgr_c::setSendCount(aid, latency);
}

typedef char SessionSizeCheck[sizeof(Session_c) == 0x380 ? 1 : -1];
typedef char SessionErrorCheck[offsetof(Session_c, mError) == 0x32C ? 1 : -1];
typedef char
    SessionKeepaliveCheck[offsetof(Session_c, mKeepaliveEnabled) == 0x379 ? 1
                                                                          : -1];
} // namespace dNetDwc
