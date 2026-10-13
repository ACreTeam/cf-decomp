#pragma once

// WiiConnect24 (DOL TU d_net_wc24.cpp, not decompiled). Only what other TUs use.

#include <types.h>
#include <revolution/NWC24/NWC24Types.h>

struct dPostRequest_c;
struct dWifiMsgHeader_c;

// The data blocks of a message (d_wifi; dPostRequest_c adds the sender): only [2] and [3] are sent
// (makeMessage), [2] first.
struct dNetWc24Data_c {
    dNetWc24Data_c() {
        for (int i = 0; i < 4; i++) {
            mData[i] = NULL;
            mSize[i] = 0;
        }
        mFriend = -1;
    }

    /* 0x00 */ const void *mData[4];
    /* 0x10 */ u32 mSize[4];
    /* 0x20 */ int mFriend; // dPostRequest_c: the friend index of the sender
}; // size 0x24

// One entry of dNetWc24_c::getFriends (0x20).
struct dNetWc24Friend_c {
    /* 0x00 */ u32 mIdx;      // the NWC24 friend list index (= the dSaveWc24_c roster index)
    /* 0x04 */ u32 mType;     // 1: a friend registered with a key
    /* 0x08 */ u8 mInfo[0x18];
}; // size 0x20

// A friend record of the NWC24 friend list kept for a registered friend (registerFriendRec).
struct dNetWc24FriendRec_c {
    /* 0x00 */ u64 mKey;
    /* 0x08 */ int mProfileId;
    /* 0x0C */ u32 mGameCode; // 'RUUJ' / 'RUUE' / 'RUUP' / 'RUUK'
}; // size 0x10

// The u16 message tag of dNetWc24_c::sendMessage (stored in the message header; d_wifi fn_80178604 reads
// mKind of received ones).
union dNetWc24MsgTag_c {
    u16 mRaw;
    struct {
        u16 mFlag : 1;
        u16 mKind : 7; // 1: a letter to a registered friend
        u16 mData : 8;
    } mBits;
};

// The WiiConnect24 state (805BE940; size unknown).
class dNetWc24_c {
public:
    BOOL getFriendKey(u32 idx, u64 *key); // 800E997C: entry idx of the NWC24 friend list, FALSE if unused

    // ---- region D of d_wifi (8017A708..8017B894); names inferred ----
    typedef BOOL (*FriendRecCallback)(const dNetWc24FriendRec_c *rec);
    typedef void (*DownloadCallback)(void *data, u32 size, BOOL arg);
    // A received message: (message object, its data, size, header); the result is kept per message.
    // dWifi::onReceive: the sender's key, the message data and its header.
    typedef BOOL (*ReceiveCallback)(const u64 *key, const void *data, u32 size, const dWifiMsgHeader_c *header);
    BOOL open();                                     // 800E957C: opens the NWC24 library (allocates its buffers)
    void close();                                    // 800E9708
    BOOL canUseFriends();                            // 800E97E8: NWC24 check 4 (before registering friends)
    BOOL canSendMessages();                          // 800E9830: NWC24 check 1 (before a send)
    BOOL canDownload();                              // 800E9878: NWC24 check 2 (before the download tasks)
    u32 getFriends(dNetWc24Friend_c *friends, u32 max); // 800E99F4: the used NWC24 friend list entries
    BOOL sendMessage(u64 *keys, int num, const dNetWc24Data_c *data, u16 tag, int profileId); // 800E9B70
    BOOL sendMessage(const u64 *key, u32 size, int profileId); // 800E9C74: the message makeMessage built
    // 800EA2C8: builds the message to send from two data blocks; returns its size (0: too big).
    u32 makeMessage(const void *data, u32 size, const void *data2, u32 size2, u16 tag);
    BOOL sendBoardMessage(const dPostRequest_c *req); // 800E9D34: mFriend = roster entry, -1 = this console
    void receiveMessages(ReceiveCallback callback);  // 800EA0DC: (with dWifi::onReceive)
    static BOOL registerFriendRec(const u64 *key, int profileId, u32 gameCode); // 800EA800
    static void cleanFriendRecs(FriendRecCallback keep);  // 800EA860: deletes the records keep() rejects
    static BOOL needsDownloadSetup();                // 800EAB74: the download file does not exist
    BOOL setupDownload();                            // 800EABAC
    int checkTasks();                                // 800EAC00: -1 none, 1 needs registerTasks
    BOOL registerTasks();                            // 800EAC74
    BOOL deleteTasks();                              // 800EAD18
    BOOL download(int task, DownloadCallback callback); // 800EAF28: callback(data, size, FALSE) on new data

    // ---- region A of d_wifi (80177A7C..80178604); names inferred ----
    static BOOL suspendScheduler();                  // 800E97B8: NWC24SuspendScheduler() >= 0
    static int trySuspendScheduler();               // 800E97E0: raw NWC24SuspendScheduler result
    static int resumeScheduler();                   // 800E97E4: NWC24ResumeScheduler

    /* 0x000 */ u8 _000[0x400];
    /* 0x400 */ u64 mUserId; // this console's user id (sent by d_net)
    /* 0x408 */ u8 _408[0x10];
    /* 0x418 */ NWC24Err mLastError; // of the last operation (d_wifi skips its updates on NWC24_ERR_NAND_CORRUPT)
    /* 0x41C */ int mErrorCode;      // NWC24GetErrorCode, negated (set with mLastError by 800EA760)
    /* 0x420 */ int mNandState;      // 5: a NAND operation failed (mNandResult; 800EAD18)
    /* 0x424 */ int mNandResult;     // NANDResult
    /* 0x428 */ u8 _428[8];
    /* 0x430 */ u8 mErrorKind;       // the kind given with mLastError (4 when cleared)
    /* 0x431 */ u8 mOpened;          // open() succeeded
};

extern dNetWc24_c lbl_805BE940; // 805BE940

extern "C" {
u32 fn_800E94A8(); // 800E94A8: size of the NWC24 library heap (0x5C800)
u32 fn_800E94B4(); // 800E94B4: 0x5A000 (d_wifi fn_8017B18C; d_sv_mgr: NAND resource size)
u32 fn_800E94C0(); // 800E94C0: 0x800 (d_wifi fn_8017B190; d_sv_mgr: NAND auxiliary size)
}
