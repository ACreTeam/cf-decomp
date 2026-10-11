#pragma once

// The friend / connection data and the request queue of the network manager
// (DOL TU d_net_friend.cpp, .text 800E6804..800E7650, not decompiled). Only
// what d_net_mgr uses; all names are inferred.

#include <lib/dwc/dwc.h>
#include <revolution/OS.h>
#include <types.h>

// A mutex that initialises itself.
class dNetMutex_c {
  public:
    dNetMutex_c() {
        OSInitMutex(&mMutex);
    }

    /* 0x00 */ OSMutex mMutex;
}; // size 0x18

// One friend's connection status. 0xD0 bytes.
class dNetFriendStatus_c {
  public:
    dNetFriendStatus_c(); // 800E6804

    bool hasAid() const {
        return mFlags & 4;
    }

    /* 0x00 */ s64 m00;
    /* 0x08 */ u32 m08;
    // the friend's status data (dNetMgr_c::mStatus)
    /* 0x0C */ u8 mStatus[0xBD];
    /* 0xC9 */ u8 mAid;
    /* 0xCA */ u8 mFlags; // 1: ?, 4: mAid is valid
}; // size 0xD0

// The last error, kept by priority.
class dNetError_c {
  public:
    dNetError_c() {
        clear();
    }
    void clear(); // 800E6920
    // 800E6938: if kind > mKind
    void set(u8 state, u8 kind, int detail, int code);
    int getState() const {
        return mState;
    }

    /* 0x0 */ int mCode;
    /* 0x4 */ int mDetail;
    /* 0x8 */ u8 mKind;
    /* 0x9 */ u8 mState;
}; // size 0xC

#define NET_FRIEND_NUM 32

// The friends' status and the session state.
class dNetFriendInfo_c {
  public:
    dNetFriendInfo_c(); // 800E682C
    void clear();       // 800E689C

    /* 0x0000 */ dNetFriendStatus_c mFriends[NET_FRIEND_NUM];
    /* 0x1A00 */ dNetError_c mError;
    /* 0x1A10 */ s64 m1A10;
    /* 0x1A18 */ u8 m1A18[0xC];
    /* 0x1A24 */ int mState;
    /* 0x1A28 */ u32 m1A28;
    /* 0x1A2C */ u16 mMemberBits; // bit i: member i is connected
    /* 0x1A2E */ u8 mMyNo;        // this console's member number
    /* 0x1A2F */ u8 m1A2F;
    /* 0x1A30 */ u8 m1A30;
    /* 0x1A31 */ u8 m1A31; // d_fireworks: picks the alternative sounds
    /* 0x1A32 */ u8 m1A32;
    /* 0x1A33 */ u8 m1A33;
}; // size 0x1A38

// The account, the friend list and the status, as the DWC callbacks keep them
// (copied out under the mutex).
class dNetFriendData_c {
  public:
    // 800E6958
    void get(dNetFriendInfo_c *info, DWCUserData *userData,
             DWCFriendData *friends);

    /* 0x0000 */ DWCUserData mUserData;
    /* 0x0040 */ DWCFriendData mFriends[NET_FRIEND_NUM];
    /* 0x01C0 */ dNetFriendInfo_c mInfo;
    /* 0x1BF8 */ dNetMutex_c mMutex;
}; // size 0x1C10

// The send queue of one member.
class dNetSendBuf_c {
  public:
    dNetSendBuf_c() {
        mSize = 0;
    }
    ~dNetSendBuf_c() {
    }
    void set(const void *data, u32 size); // 800E6D38
    BOOL isBusy();                        // 800E6DA0

    /* 0x00 */ dNetMutex_c mMutex;
    /* 0x18 */ u8 _18[8];
    /* 0x20 */ u32 mSize;
}; // size 0x24

// Request kinds of dNetFriendReq_c (handled by dNetMgr_c::procRequest /
// procDwcRequest).
enum dNetFriendReqKind_e {
    NET_FRIEND_REQ_NONE = 0,
    NET_FRIEND_REQ_1 = 1,
    NET_FRIEND_REQ_2 = 2,
    NET_FRIEND_REQ_3 = 3,
    NET_FRIEND_REQ_4 = 4,
    NET_FRIEND_REQ_5 = 5,
    NET_FRIEND_REQ_6 = 6,
    NET_FRIEND_REQ_VOICE_PARAM_ON = 7,
    NET_FRIEND_REQ_VOICE_PARAM_OFF = 8,
    NET_FRIEND_REQ_MUTE_ON = 9,
    NET_FRIEND_REQ_MUTE_OFF = 10,
    NET_FRIEND_REQ_VOICE_B_ON = 11,
    NET_FRIEND_REQ_VOICE_B_OFF = 12,
    NET_FRIEND_REQ_VOICE_C = 13,
    NET_FRIEND_REQ_14 = 14,
    // a friend's data at an index of the friend list
    NET_FRIEND_REQ_REGISTER = 15,
    NET_FRIEND_REQ_REMOVE = 16,    // removes the friend at an index
    NET_FRIEND_REQ_MY_DATA = 17,   // own friend data
    NET_FRIEND_REQ_MY_KEY = 18,    // own friend key
    NET_FRIEND_REQ_KEY = 19,       // friend key of friend data
    NET_FRIEND_REQ_KEY_DATA = 20,  // friend data of a friend key
    NET_FRIEND_REQ_CHECK_KEY = 21, // checks a friend key
    NET_FRIEND_REQ_FIND_USER = 22, // friend list index of an account
    NET_FRIEND_REQ_FIND_DATA = 23, // friend list index of friend data
    NET_FRIEND_REQ_FIND_KEY = 24,  // friend list index of a friend key
    // whether an account and a friend key are the same friend
    NET_FRIEND_REQ_IS_FRIEND = 25,
};

// The arguments of a request.
struct dNetFriendReqArgs_c {
    union {
        DWCUserData mUserData;
        struct {
            DWCFriendData mData;
            int mIndex;
        } mFriend;
        u64 mFriendKey;
        struct {
            u32 mValue;
            u8 mAid;
            u8 mArg;
        } mMatch;
        u8 mParam;
    };
    /* 0x40 */ u64 mFriendKey2;
}; // size 0x48

// One request at a time from the game to the DWC thread, with its result. No
// constructor: dNetMgr_c's initialises it.
class dNetFriendReq_c {
  public:
    void reset(); // 800E6E48
    // 800E6E80: back to NET_FRIEND_REQ_NONE
    void finish();
    int getKind();                                           // 800E6EBC
    u16 takeConnect();                                       // 800E6F00
    u16 takeDisconnect();                                    // 800E6F4C
    void setResult(int kind, int result);                    // 800E6F98
    void setResultData(int kind, const DWCFriendData *data); // 800E7004
    void setResultKey(int kind, u64 key);                    // 800E705C
    BOOL fn_800E723C(u8 memberNum, int arg);                 // NET_FRIEND_REQ_2
    BOOL fn_800E726C(u8 memberNum, u32 arg);                 // NET_FRIEND_REQ_3
    BOOL fn_800E729C();                                      // NET_FRIEND_REQ_6
    // NET_FRIEND_REQ_REGISTER
    BOOL fn_800E72C4(int index, const DWCFriendData *data);
    // NET_FRIEND_REQ_REMOVE
    BOOL fn_800E731C(int index);
    // NET_FRIEND_REQ_MY_DATA
    BOOL fn_800E7348(const DWCUserData *userData);
    // NET_FRIEND_REQ_KEY_DATA
    BOOL fn_800E738C(u64 key);
    // NET_FRIEND_REQ_CHECK_KEY
    BOOL fn_800E73BC(u64 key);
    // NET_FRIEND_REQ_FIND_USER
    BOOL fn_800E73EC(const DWCUserData *userData);
    // NET_FRIEND_REQ_FIND_DATA
    BOOL fn_800E7430(const DWCFriendData *data);
    // NET_FRIEND_REQ_IS_FRIEND
    BOOL fn_800E7474(const DWCUserData *userData, u64 key);
    BOOL connect(u8 aid);      // 800E74D8
    BOOL disconnect(u8 aid);   // 800E753C
    void addConnect(u16 mask); // 800E75A0
    // NET_FRIEND_REQ_MUTE_ON
    BOOL fn_800E7600();
    // NET_FRIEND_REQ_MUTE_OFF
    BOOL fn_800E7628();

    /* 0x00 */ OSMutex mMutex;
    /* 0x18 */ dNetFriendReqArgs_c mArgs;
    /* 0x60 */ int mKind;
    /* 0x64 */ u16 mConnect;    // aids to connect
    /* 0x66 */ u16 mDisconnect; // aids to disconnect
    /* 0x68 */ int
        mResults[NET_FRIEND_REQ_IS_FRIEND - NET_FRIEND_REQ_REGISTER + 1];
    /* 0x94 */ DWCFriendData mResultData;
    /* 0xA0 */ u64 mResultKey;
}; // size 0xA8
