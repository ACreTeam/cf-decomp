#ifndef GAME_D_NET_WC24_HPP
#define GAME_D_NET_WC24_HPP

// WiiConnect24 library, message transport and background downloads. Names inferred.

#include <types.h>
#include <revolution/nwc24.h>

struct dPostRequest_c;
struct dWifiMsgHeader_c;

namespace dNetWc24 {

enum ErrorKind_e { ERROR_OPEN, ERROR_CHECK, ERROR_SEND, ERROR_DOWNLOAD, ERROR_NONE };
enum TaskState_e { TASK_REGISTERED, TASK_NEW, TASK_UNCHECKED };
enum DownloadTask_e { TASK_DISTRIBUTION, TASK_NGWORD };
enum ReadResult_e { READ_ERROR = -1, READ_DONE, READ_MISSING };
enum {
    FRIEND_SELF = -1,
    BOARD_LETTER_TAG = 0x2019,
    MESSAGE_VERSION = 2,
    MESSAGE_REGION_MASK = 0xFFFFFF00,
    MESSAGE_GAME_CODE = 0x52555500,
    DOWNLOAD_FLAGS = 10, // NWC24SetDlFlags
    NAND_DELETE_ERROR = 5,
    HEAP_SIZE = 0x5C800,
    TASK_COUNT = 2,
    LIBRARY_WORK_SIZE = 0x4000,
    MESSAGE_CAPACITY = 0x25800,
    MESSAGE_WORK_SIZE = 2 * MESSAGE_CAPACITY,
    DOWNLOAD_RESOURCE_SIZE = 0x5A000,
    DOWNLOAD_AUX_SIZE = 0x800,
    MAX_RECEIVE_MESSAGES = 50,
    MAX_SUBJECT_LENGTH = 35
};

// The data blocks of a message (d_wifi; dPostRequest_c adds the sender): only [2] and [3] are sent
// (makeMessage), [2] first.
struct dNetWc24Data_c {
    dNetWc24Data_c() {
        mData[0] = NULL;
        mSize[0] = 0;
        mData[1] = NULL;
        mSize[1] = 0;
        mData[2] = NULL;
        mSize[2] = 0;
        mData[3] = NULL;
        mSize[3] = 0;
        mFriend = FRIEND_SELF;
    }

    /* 0x00 */ const void *mData[4];
    /* 0x10 */ u32 mSize[4];
    /* 0x20 */ int mFriend; // dPostRequest_c: the friend index of the sender
}; // size 0x24

// One entry of dNetWc24_c::getFriends (0x20).
struct dNetWc24Friend_c {
    /* 0x00 */ u32 mIdx;      // the NWC24 friend list index (= the dSaveWc24_c roster index)
    /* 0x04 */ u32 mType;     // NWC24FriendType
    /* 0x08 */ wchar_t mName[0xC]; // the friend's name
}; // size 0x20

// A friend record of the NWC24 friend list kept for a registered friend (registerFriendRec).
struct dNetWc24FriendRec_c {
    /* 0x00 */ u64 mKey;
    /* 0x08 */ int mProfileId;
    /* 0x0C */ u32 mGameCode; // 'RUUJ' / 'RUUE' / 'RUUP' / 'RUUK'
}; // size 0x10

// The u16 message tag of dNetWc24_c::sendMessage (stored in the message header; d_wifi fn_80178604 reads
// mKind of received ones).
enum MsgTag_e {
    MSG_TAG_UNCOMPRESSED_MASK = 0x7FFF,
    MSG_TAG_COMPRESSED = 0x8000
};

union dNetWc24MsgTag_c {
    u16 mRaw;
    struct {
        u16 mFlag : 1;
        u16 mKind : 7; // 1: a letter to a registered friend
        u16 mData : 8;
    } mBits;
};

// The WiiConnect24 state (805BE940; size 0x438).
class dNetWc24_c {
public:
    dNetWc24_c(); // 800E94C8
    ~dNetWc24_c(); // 800E953C
    BOOL getFriendKey(u32 idx, u64 *key); // 800E997C: entry idx of the NWC24 friend list, FALSE if unused

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
    u32 makeMessage(const void *data, u32 size, u16 tag) { return makeMessage(data, size, NULL, 0, tag); }
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

    static BOOL suspendScheduler();                  // 800E97B8: NWC24SuspendScheduler() >= 0
    static int trySuspendScheduler();               // 800E97E0: raw NWC24TrySuspendScheduler result
    static int resumeScheduler();                   // 800E97E4: NWC24ResumeScheduler

    NWC24DlTask *getTask(int task) { return &mTasks[task]; }
    u8 *getMessageBuffer() { return mMessageBuffer; }
    u8 *getCryptBuffer() { return mCryptBuffer; }

    BOOL readFriend(u32 idx, NWC24FriendInfo *info); // 800E98C0
    BOOL commitMessage(NWC24MsgObj *msg, u32 size, int profileId); // 800E9AC8
    BOOL decodeMessage(const u64 *key, const void *data, u32 size); // 800EA584
    BOOL setError(NWC24Err error, int kind); // 800EA760
    void clearError(); // 800EA7E0
    BOOL prepareTask(int task); // 800EA900
    BOOL configureTask(int task); // 800EA9F0
    int readDownload(int task, void *data, u32 capacity, u32 *size); // 800EAE38

    /* 0x000 */ NWC24DlTask mTasks[TASK_COUNT];
    /* 0x400 */ u64 mUserId; // this console's user id (sent by d_net)
    /* 0x408 */ void *mVfWork;
    /* 0x40C */ void *mLibraryWork;
    /* 0x410 */ u8 *mMessageBuffer;
    /* 0x414 */ u8 *mCryptBuffer;
    /* 0x418 */ NWC24Err mLastError; // of the last operation (d_wifi skips its updates on NWC24_ERR_NAND_CORRUPT)
    /* 0x41C */ int mErrorCode;      // NWC24GetErrorCode, negated (set with mLastError by 800EA760)
    /* 0x420 */ int mNandState;      // 5: a NAND operation failed (mNandResult; 800EAD18)
    /* 0x424 */ int mNandResult;     // NANDResult
    /* 0x428 */ int mTaskState[TASK_COUNT];
    /* 0x430 */ u8 mErrorKind;       // the kind given with mLastError (4 when cleared)
    /* 0x431 */ u8 mOpened;          // open() succeeded
};

extern dNetWc24_c g_wc24; // 805BE940

u32 getHeapSize(); // 800E94A8
u32 getDownloadResourceSize(); // 800E94B4
u32 getDownloadAuxSize(); // 800E94C0

} // namespace dNetWc24
#endif
