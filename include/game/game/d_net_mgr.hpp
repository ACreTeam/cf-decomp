#pragma once

// d_net_mgr: the network manager (DOL TU src/dol/game/d_net_mgr.cpp, .text
// 800E48A0..800E6804, -sym on). It sits between d_net (the session protocol)
// and the DWC thread (d_net_dwc): receive / send buffers per member, the friend
// list, voice chat settings and one request at a time to the DWC thread
// (dNetFriend::Request_c). The instance is g_netMgr, reached through
// dNetMgr_c::sInstance. All names are inferred; see notes/d_net_mgr.txt.

#include <game/game/d_net_dwc.hpp>
#include <game/game/d_net_friend.hpp>
#include <lib/dwc/dwc.h>
#include <types.h>

// Nintendo Wi-Fi Connection error codes (the number shown to the player, dNetFriend::Error_c::mCode / mDetail)
// that pick a sys_MESS/SYS_NetworkError message (d_a_npc_sp_gatekeeper_a getNetErrorMsg / checkNetError).
// Named after those messages; Nintendo's own names are unknown.
#define NET_ERROR_CODE_NO_INTERNET 20100    // NET_ERROR_MSG_NO_INTERNET: "Unable to connect to the Internet."
#define NET_ERROR_CODE_SERVER_BUSY 20101    // NET_ERROR_MSG_WFC_BUSY: high traffic, or the service is down
#define NET_ERROR_CODE_LOGIN_FIRST 20102    // 20102..20999 (but SERVICE_ENDED): NET_ERROR_MSG_WFC_ERROR
#define NET_ERROR_CODE_SERVICE_ENDED 20110  // NET_ERROR_MSG_WFC_DISCONTINUED: the service has been discontinued
#define NET_ERROR_CODE_LOGIN_LAST 20999
#define NET_ERROR_CODE_BUSY_FIRST 23000     // 23000..23999: NET_ERROR_MSG_WFC_BUSY
#define NET_ERROR_CODE_BUSY_LAST 23999
#define NET_ERROR_CODE_NAND_FULL 29000      // NET_ERROR_MSG_NAND_FULL: not enough Wii system memory
#define NET_ERROR_CODE_NAND_CORRUPTED 29001 // NET_ERROR_MSG_NAND_CORRUPTED
#define NET_ERROR_CODE_INTERNET_FIRST 50000 // 50000..59999: NET_ERROR_MSG_NO_INTERNET
#define NET_ERROR_CODE_INTERNET_LAST 59999
#define NET_ERROR_CODE_NO_RESPONSE 80430    // NET_ERROR_MSG_NO_RESPONSE / MSG_TOWN_NO_RESPONSE: "There is no response."

// Manager states (mState).
enum dNetMgrState_e {
    NET_MGR_STATE_NONE = 0,
    NET_MGR_STATE_SETUP = 1, // buffers allocated (setup)
    NET_MGR_STATE_2 = 2,
    NET_MGR_STATE_STARTED = 3, // start
    NET_MGR_STATE_4 = 4,
    NET_MGR_STATE_5 = 5,
    NET_MGR_STATE_6 = 6,
};

// Callbacks given to setCallbacks (functions of d_net.cpp).
typedef void (*dNetMgrRecvFunc)(u8 member, const void *data, u32 size);
typedef void (*dNetMgrMemberFunc)(u32 member);
typedef void (*dNetMgrEventFunc)(int arg, int arg2);
typedef void (*dNetMgrFunc)();

class dNetMgr_c {
  public:
    dNetMgr_c();
    ~dNetMgr_c();

    // Member `no` is connected (inlined in d_net).
    bool isConnected(u8 no) const {
        return mInfo.mMemberBits & (1 << no);
    }
    u8 getMyNo() const {
        return mInfo.mMyNo;
    }
    // Another connected member.
    bool isPeer(u8 no) const {
        return no != mInfo.mMyNo && isConnected(no);
    }

    static void initDwc(); // d_sys
    void lock(int);        // the index is not used
    void unlock(int);
    void setup(DWCUserData *userData, DWCFriendData *friends, int friendNum,
               u8 memberNum, u8 *const *bufs, u32 bufSize);
    void cleanup();
    void setCallbacks(dNetMgrRecvFunc recv, dNetMgrMemberFunc join,
                      dNetMgrEventFunc event, dNetMgrFunc status,
                      const void *statusData, u32 statusSize,
                      dNetMgrMemberFunc leave);
    void reset();
    BOOL start();
    static BOOL open();
    static BOOL close();
    void execute();
    BOOL fn_800E51C8(int arg);
    BOOL fn_800E5230(int arg);
    BOOL fn_800E5298(int arg);
    BOOL setRecvBuf(u8 member, u8 *buf, u32 size);
    BOOL send(u8 member, const void *data, u32 size);
    BOOL canSend(u8 member);
    // getErrorKind: what went wrong, by the error state / kind / code.
    enum {
        ERROR_KIND_NONE,
        ERROR_KIND_GATE_CLOSED,  // state 0xF or code 0xB: the town's gate is closed
        ERROR_KIND_TOWN_FULL,    // state 0x10 or code 0xC: the town has no room
        ERROR_KIND_TOWN_BUSY,    // code 0x17: someone else is joining
        ERROR_KIND_OTHER,        // any other recoverable error (see the detail code)
        ERROR_KIND_RESTART,      // dNetDwc::ERROR_RESTART_REQUIRED
        ERROR_KIND_PERMANENT,    // dNetDwc::ERROR_PERMANENT
    };
    static int getErrorKind();
    static void fn_800E55B0();
    u8 getFriendStatus(int idx, void *dst, u32 *size);
    BOOL isFriendInSession(int idx);
    static BOOL isStarted();
    static void setVoiceOff();
    static void setVoiceOn();
    static void connect(u8 aid);
    static void disconnect(u8 aid);
    static void setBusy();
    static void clearBusy();
    static void setVolume(int volume);
    static void mute();
    static void unmute();
    static void initFriendList(DWCUserData *userData, DWCFriendData *friends,
                               u32 num);
    static s64 getInfo1A10();
    static u32 getInfo1A28();
    static s64 getFriend00(int idx);
    static u32 getFriend08(int idx);
    static BOOL getFriendFlag(int idx);
    static BOOL isUserDataValid(const DWCUserData *userData);
    static void getMyFriendData(const DWCUserData *userData,
                                DWCFriendData *friendData);
    static u64 getMyFriendKey(const DWCUserData *userData);
    static int getMyProfileId(const DWCUserData *userData);
    static u64 getFriendKey(const DWCFriendData *friendData);
    static int getProfileId(const DWCUserData *userData,
                            const DWCFriendData *friendData);
    static void makeFriendData(u64 key, DWCFriendData *friendData);
    static BOOL isFriendDataValid(const DWCFriendData *friendData);
    static BOOL isSameFriend(const DWCUserData *userData,
                             const DWCFriendData *a, const DWCFriendData *b);
    static int checkFriendKey(const DWCUserData *userData, u64 key);
    static BOOL reqRemove(int index);
    static BOOL reqKeyData(u64 key);
    static BOOL reqIsFriend(const DWCUserData *userData, u64 key);
    static BOOL reqCheckKey(u64 key);
    static BOOL reqRegister(int index, const DWCFriendData *data);
    static BOOL reqMyData(const DWCUserData *userData);
    static BOOL reqFindUser(const DWCUserData *userData);
    static BOOL reqFindData(const DWCFriendData *data);
    static void onStatus();
    static BOOL isRequestIdle();
    static void onJoin(u8 aid);
    static void onLeave(u8 aid);
    static void fn_800E5C7C();
    static void onRecv(u8 aid, const void *data, u32 size);
    void setDwcRecvBuf(u8 aid);
    static void onEvent(int arg, int arg2);
    static void setFriendRemoved(int index);
    static void procDwcRequest();
    static void procRequest();
    static void setSendCount(u8 member, int count);

    static dNetMgr_c *sInstance;

    /* 0x0000 */ dNetFriend::Mutex_c mMutex;
    /* 0x0018 */ dNetFriend::Mutex_c mMutex2;
    /* 0x0030 */ dNetFriend::Data_c mData; // written by the DWC thread
    // mData.mInfo, copied by execute / start
    /* 0x1C40 */ dNetFriend::Info_c mInfo;
    /* 0x3678 */ dNetFriend::Request_c mReq;
    /* 0x3720 */ dNetMgrFunc mStatusCb; // after the status data was sent
    /* 0x3724 */ dNetMgrMemberFunc mJoinCb;
    /* 0x3728 */ dNetMgrMemberFunc mLeaveCb;
    /* 0x372C */ dNetMgrRecvFunc mRecvCb;
    /* 0x3730 */ dNetMgrEventFunc mEventCb;
    /* 0x3734 */ u8 mStatus[0xC0]; // the status data the friends see
    /* 0x37F4 */ u32 mStatusSize;
    /* 0x37F8 */ DWCUserData mUserData; // copy of *mUserDataSrc (start)
    // copy of mFriendSrc (start), mFriendNum entries
    /* 0x3838 */ DWCFriendData *mFriends;
    // the current player's dFriendList_c::mUserData
    /* 0x383C */ DWCUserData *mUserDataSrc;
    // the current player's dFriendList_c::mKeys
    /* 0x3840 */ DWCFriendData *mFriendSrc;
    /* 0x3844 */ dNetFriend::SendBuf_c *mSendBufs; // mMemberNum
    /* 0x3848 */ int *mSendCounts;         // mMemberNum (setSendCount)
    /* 0x384C */ int *mSendCountMax;       // mMemberNum
                                           // mMemberNum - 1 (the other members)
    /* 0x3850 */ u8 **mRecvBufs;
    /* 0x3854 */ u32 mRecvBufSize;
    // bit i: friend i was removed (setFriendRemoved)
    /* 0x3858 */ u32 mRemovedFriends;
    /* 0x385C */ int mState; // dNetMgrState_e
    /* 0x3860 */ u8 mMemberNum;
    /* 0x3861 */ u8 m3861;
    /* 0x3862 */ u8 mFriendNum;
    /* 0x3863 */ u8 m3863;
    /* 0x3864 */ u8 mStarted;
    /* 0x3865 */ u8 mTelopShown;
    /* 0x3866 */ u8 mVoiceOff;
    /* 0x3867 */ u8 mBusy; // set while a scene change is under way (d_net)
}; // size 0x3868

extern dNetDwc::Session_c g_netDwc; // 805B6560
