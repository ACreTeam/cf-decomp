#ifndef GAME_D_NET_FRIEND_HPP
#define GAME_D_NET_FRIEND_HPP

// The friend / connection data and the request queue of the network manager
// (DOL TU d_net_friend.cpp, .text 800E6804..800E7650). Names are inferred
// from the network manager and DWC worker behavior.

#include <lib/dwc/dwc.h>
#include <revolution/os.h>
#include <types.h>

namespace dNetFriend {

enum InfoState_e { INFO_OFFLINE, INFO_STARTING, INFO_ONLINE, INFO_CONNECTED };
enum StatusFlags_e { STATUS_VALID = 1, STATUS_BUDDY = 2, STATUS_HAS_AID = 4 };
enum SendState_e { SEND_IDLE, SEND_BUSY };
enum { FRIEND_STATUS_CONNECTED = 6 };

// A mutex that initialises itself.
class Mutex_c {
  public:
    Mutex_c() {
        OSInitMutex(&mMutex);
    }

    /* 0x00 */ OSMutex mMutex;
}; // size 0x18

// One friend's connection status. 0xD0 bytes.
class Status_c {
  public:
    Status_c() {
        clear();
    }
    void clear(); // 800E6804

    void *getStatus() {
        return mStatus;
    }

    bool hasAid() const {
        return mFlags & STATUS_HAS_AID;
    }

    /* 0x00 */ s64 mFriendKey;
    /* 0x08 */ u32 mProfileId;
    // the friend's status data (dNetMgr_c::mStatus)
    /* 0x0C */ u8 mStatus[0xBD];
    /* 0xC9 */ u8 mAid;
    /* 0xCA */ u8 mFlags; // STATUS_* bit flags
}; // size 0xD0

// The last error, kept by priority.
class Error_c {
  public:
    Error_c() {
        clear();
    }
    void clear(); // 800E6920
    // 800E6938: if kind > mKind
    void set(u8 state, u8 kind, int detail, int code);
    int getState() const {
        return mState;
    }
    int getKind() const {
        return mKind;
    }

    /* 0x0 */ int mCode;
    /* 0x4 */ int mDetail;
    /* 0x8 */ u8 mKind;
    /* 0x9 */ u8 mState;
}; // size 0xC

#define NET_FRIEND_NUM 32

// The friends' status and the session state.
class Info_c {
  public:
    Info_c();     // 800E682C
    void clear(); // 800E689C

    /* 0x0000 */ Status_c mFriends[NET_FRIEND_NUM];
    /* 0x1A00 */ Error_c mError;
    /* 0x1A10 */ s64 mMyFriendKey;
    /* 0x1A18 */ u8 mMyFriendData[0xC];
    /* 0x1A24 */ int mState;
    /* 0x1A28 */ u32 mMyProfileId;
    /* 0x1A2C */ u16 mMemberBits; // bit i: member i is connected
    /* 0x1A2E */ u8 mMyNo;        // this console's member number
    /* 0x1A2F */ u8 mMemberCount;
    /* 0x1A30 */ u8 mUpdatingFriends;
    /* 0x1A31 */ u8
        mMicActive; // usable microphone; also selects fireworks sounds
    /* 0x1A32 */ u8 mMicUnavailable;
    /* 0x1A33 */ u8 mMatchReady; // SDK match synchronization predicate
}; // size 0x1A38

// The account, the friend list and the status, as the DWC callbacks keep them
// (copied out under the mutex).
class Data_c {
  public:
    // 800E6958
    void get(Info_c *info, DWCUserData *userData, DWCFriendData *friends);

    // 800E69F4: publish the worker session under the friend-data mutex.
    void updateSession(int state, Error_c *error, DWCUserData *user,
                       DWCFriendData *friends, u16 members, u8 updating,
                       BOOL micActive, bool micFailed);

    /* 0x0000 */ DWCUserData mUserData;
    /* 0x0040 */ DWCFriendData mFriends[NET_FRIEND_NUM];
    /* 0x01C0 */ Info_c mInfo;
    /* 0x1BF8 */ Mutex_c mMutex;
}; // size 0x1C10

// The send queue of one member.
class SendBuf_c {
  public:
    SendBuf_c() {
        mSendState = SEND_IDLE;
    }
    ~SendBuf_c() {
    }
    void set(const void *data, u32 size); // 800E6D38
    BOOL isBusy();                        // 800E6DA0

    void reset();        // 800E6CC0
    void onSent();       // 800E6CFC
    void clearPending(); // 800E6D94
    BOOL hasPending();   // 800E6DE8

    /* 0x00 */ Mutex_c mMutex;
    /* 0x18 */ const void *mData;
    /* 0x1C */ u32 mLength;
    /* 0x20 */ int mSendState;
}; // size 0x24

// Request kinds of Request_c (handled by dNetMgr_c::procRequest /
// procDwcRequest).
enum RequestKind_e {
    NET_FRIEND_REQ_NONE = 0,
    NET_FRIEND_REQ_MATCH_ANYBODY = 1,
    NET_FRIEND_REQ_HOST_MATCH = 2,
    NET_FRIEND_REQ_MATCH_FRIEND = 3,
    NET_FRIEND_REQ_CANCEL_MATCH = 4,
    NET_FRIEND_REQ_CLOSE_CONNECTIONS = 5,
    NET_FRIEND_REQ_CLEAR_ERROR = 6,
    NET_FRIEND_REQ_VOICE_PARAM_ON = 7,
    NET_FRIEND_REQ_VOICE_PARAM_OFF = 8,
    NET_FRIEND_REQ_MUTE_ON = 9,
    NET_FRIEND_REQ_MUTE_OFF = 10,
    NET_FRIEND_REQ_VOICE_B_ON = 11,
    NET_FRIEND_REQ_VOICE_B_OFF = 12,
    NET_FRIEND_REQ_VOICE_C = 13,
    NET_FRIEND_REQ_FINISH = 14,
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
union RequestArgs_c {
    DWCUserData mUserData;
    struct {
        DWCFriendData mData;
        int mIndex;
    } mFriend;
    u64 mFriendKey;
    struct {
        u32 mValue;
        u8 mMemberNum;
        u8 mArg;
    } mMatch;
    u8 mParam;
    struct {
        /* 0x00 */ DWCUserData mUserData;
        /* 0x40 */ u64 mFriendKey;
    } mAccountKey;
}; // size 0x48

// One request at a time from the game to the DWC thread, with its result. No
// constructor: dNetMgr_c's initialises it.
class Request_c {
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
    // 800E70B8: TRUE once the current request is no longer `kind` (done);
    // *result = its result
    BOOL takeResult(int kind, int *result);
    BOOL takeResultData(int kind, DWCFriendData *data); // 800E7134
    BOOL submit(int kind, const RequestArgs_c *args);   // 800E71AC
    BOOL requestHostMatch(u8 memberNum, int arg);       // 800E723C
    BOOL requestFriendMatch(u8 memberNum,
                            u32 arg); // 800E726C
    BOOL requestClearError();         // 800E729C
    // NET_FRIEND_REQ_REGISTER
    BOOL requestRegisterFriend(int index,
                               const DWCFriendData *data); // 800E72C4
    // NET_FRIEND_REQ_REMOVE
    BOOL requestRemoveFriend(int index); // 800E731C
    // NET_FRIEND_REQ_MY_DATA
    BOOL requestMyFriendData(const DWCUserData *userData); // 800E7348
    // NET_FRIEND_REQ_KEY_DATA
    BOOL requestFriendKeyData(u64 key); // 800E738C
    // NET_FRIEND_REQ_CHECK_KEY
    BOOL requestCheckFriendKey(u64 key); // 800E73BC
    // NET_FRIEND_REQ_FIND_USER
    BOOL requestFindUser(const DWCUserData *userData); // 800E73EC
    // NET_FRIEND_REQ_FIND_DATA
    BOOL requestFindFriendData(const DWCFriendData *data); // 800E7430
    // NET_FRIEND_REQ_IS_FRIEND
    BOOL requestIsFriend(const DWCUserData *userData, u64 key); // 800E7474
    BOOL connect(u8 aid);                                       // 800E74D8
    BOOL disconnect(u8 aid);                                    // 800E753C
    void addConnect(u16 mask);                                  // 800E75A0
    // NET_FRIEND_REQ_MUTE_ON
    BOOL requestMute(); // 800E7600
    // NET_FRIEND_REQ_MUTE_OFF
    BOOL requestUnmute(); // 800E7628

    /* 0x00 */ OSMutex mMutex;
    /* 0x18 */ RequestArgs_c mArgs;
    /* 0x60 */ int mKind;
    /* 0x64 */ u16 mConnect;    // aids to connect
    /* 0x66 */ u16 mDisconnect; // aids to disconnect
    /* 0x68 */ int
        mResults[NET_FRIEND_REQ_IS_FRIEND - NET_FRIEND_REQ_REGISTER + 1];
    /* 0x94 */ DWCFriendData mResultData;
    /* 0xA0 */ u64 mResultKey;
}; // size 0xA8

} // namespace dNetFriend
#endif
