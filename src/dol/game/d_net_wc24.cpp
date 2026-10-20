#include <MSL/string.h>
#include <game/cLib/c_math.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_net_wc24.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_wifi.hpp>
#include <revolution/NAND.h>
#include <revolution/NWC24/NWC24Config.h>
#include <revolution/NWC24/NWC24Schedule.h>
#include <revolution/VF.h>
#include <stddef.h>


// Interfaces to SDK units whose public headers have not yet been reconstructed.
extern "C" {
int fn_80434A0C();
int fn_80434A20(void *, u32);
void fn_80434AE8();
NWC24Err fn_8040460C(void *);
NWC24Err fn_804047F8();
NWC24Err fn_80404954(u32);
int fn_80404B40();
NWC24Err NWC24IsFriendInfoThere(u32);
NWC24Err NWC24ReadFriendInfo(void *, u32);
NWC24Err NWC24CheckFriendInfo(const void *);
NWC24Err fn_8040DCA4(u32 *);
NWC24Err fn_8040F0F0(u64);
NWC24Err fn_8040538C(NWC24MsgObj *, const void *, u32);
NWC24Err fn_804054C4(NWC24MsgObj *, const void *, u32, NWC24Charset,
                     NWC24Encoding);
NWC24Err fn_804055FC(NWC24MsgObj *, const void *, u32, NWC24MIMEType);
NWC24Err fn_80405864(NWC24MsgObj *, const wchar_t *, u32);
NWC24Err fn_804058E8(NWC24MsgObj *, BOOL);
NWC24Err fn_80405940(NWC24MsgObj *, int);
NWC24Err fn_80405970(NWC24MsgObj *, u16);
NWC24Err fn_80405A3C(const NWC24MsgObj *, NWC24MsgType *);
NWC24Err fn_80405AB4(const NWC24MsgObj *, u32 *);
NWC24Err fn_80405AD0(const NWC24MsgObj *, u32, u32 *);
NWC24Err fn_80405B04(const NWC24MsgObj *, u32, NWC24MIMEType *);
NWC24Err fn_80405B80(const NWC24MsgObj *, u32 *);
NWC24Err fn_80405B90(const NWC24MsgObj *, u32 *);
NWC24Err fn_80405BA0(const NWC24MsgObj *, u64 *);
NWC24Err fn_80408D7C(NWC24MsgObj *);
NWC24Err fn_8040AFB4(NWC24MsgObj *, u32, void *, u32);
NWC24Err fn_8040B410(NWC24MsgObj *, const wchar_t *, u32, const wchar_t *, u32,
                     BOOL, u32, void *, u32);
NWC24Err fn_8040C250();
NWC24Err fn_8040C2C8(u32);
NWC24Err fn_8040C338(NWC24MsgObj *, u32, BOOL *, BOOL *);
NWC24Err fn_80406054(u32, u32);
NWC24Err fn_8040E2A4(const void *);
NWC24Err fn_8040E324(const void *);
NWC24Err fn_8040E388(void *, u32);
NWC24Err fn_8040E4FC(u32 *);
NWC24Err fn_8040FC7C(NWC24DlTask *, NWC24DlType);
NWC24Err fn_8040FE18(NWC24DlTask *, u32);
NWC24Err fn_8040FEB8(NWC24DlTask *, u16);
NWC24Err fn_80410000(NWC24DlTask *, const char *);
NWC24Err fn_804100FC(NWC24DlTask *, u8);
NWC24Err fn_804101B0(NWC24DlTask *, const char *);
NWC24Err fn_80410258(const NWC24DlTask *, char *, u32);
NWC24Err fn_80410300(NWC24DlTask *, s16);
NWC24Err fn_8041039C(NWC24DlTask *, u32);
NWC24Err fn_804103EC(NWC24DlTask *, int);
NWC24Err fn_80410904(NWC24DlTask *);
NWC24Err fn_80410AEC(NWC24DlTask *);
NWC24Err fn_80410BE8(NWC24DlTask *);
NWC24Err fn_8041100C(const NWC24DlTask *, u32 *, u32 *);
NWC24Err fn_8041109C(char *, u32);
NWC24Err fn_80411118(NWC24DlTask *, u32);
NWC24Err fn_80411238(const void *, const void *);
NWC24Err fn_8041135C();
u32 fn_8041399C();
void fn_80413D44(void *, u32, const void *, u32);
void fn_80413F80(void *, const void *, u32);
void fn_80413F90(void *, void *);
BOOL fn_80414948(void *, const void *, u32, const void *);
void fn_80414950(void *);
BOOL fn_80414954(void *, void *, const void *, u32);
BOOL fn_80414A0C(void *, void *, const void *, u32);
}

namespace dNetWc24 {

enum {
    AUTH_KEY_SIZE = 20,
    AUTH_DIGEST_SIZE = 20,
    AUTH_WORK_SIZE = 0xC8,
    AUTH_TRAILER_SIZE = sizeof(float) + AUTH_DIGEST_SIZE,
    CIPHER_KEY_SIZE = 16,
    CIPHER_IV_WORDS = 4,
    CIPHER_WORK_SIZE = 0x104,
    CIPHER_TRAILER_SIZE = CIPHER_KEY_SIZE + sizeof(float),
    MESSAGE_MAX_OVERHEAD = 0x4C,
    MESSAGE_ROUNDING_OVERHEAD = 0x47
};

// NWC24FriendInfo is a 0x40-byte attribute followed by its 0x100-byte address.
struct FriendInfo_c {
    /* 0x00 */ u32 mType;   // FriendType_e
    /* 0x04 */ u32 mStatus; // FriendStatus_e
    /* 0x08 */ u8 mName[0x18];
    /* 0x20 */ u8 _20[0x20];
    /* 0x40 */ union {
        u64 mKey;
        char mAddress[0x100];
    } mAddr;
};
struct Message_c {
    /* 0x00 */ u64 mSender;
    /* 0x08 */ char mMagic[8];
    /* 0x10 */ u32 mSize;
    /* 0x14 */ u32 mVersion;
    /* 0x18 */ union {
        dWifiMsgHeader_c mHeader;
        struct {
            u16 mParts;
            u16 mTag;
        } mHeaderWords;
    };
    /* 0x1C */ u32 _1C;
};
struct SecretFriend_c {
    /* 0x00 */ dNetWc24FriendRec_c mRec;
    /* 0x10 */ u8 _10[0x10];
};

static const char *sDownloadUrls[TASK_COUNT] = {
    "http://nwcs.wapp.wii.com/ruu/rvforestdl_us.enc",
    "http://nwcs.wapp.wii.com/nw/f/RUD/E.enc "};
static const char *sDownloadNames[TASK_COUNT] = {"game_data.bin",
                                                 "prc_list.bin"};
static const char sMessageMagic[] = "BU2mori";
static const u8 sDownloadFlags[TASK_COUNT] = {0xB8, 0xDC};
static const u16 sDownloadIntervals[TASK_COUNT] = {720, 1440};
static const s16 sDownloadCounts[TASK_COUNT] = {20, 1};

const u32 sDownloadPublicKey[64] = {
    0xD415C593, 0x59CC3886, 0x3B3CFE52, 0xC61F6A22, 0x9207A5B6, 0xE2D01F2E,
    0x5D9D2849, 0x33053436, 0xC04E15FC, 0x1CA1D966, 0xDE044F64, 0x37E6C4C9,
    0xE5CDD927, 0x08EF21C1, 0x2153CE78, 0x8C7C493B, 0x1C39420D, 0xBCEF456E,
    0x26AFC882, 0x6CCF8502, 0x9C32ED26, 0x2A5DF39A, 0x7582985F, 0x22265FDF,
    0x502DD028, 0x441F3187, 0xE47D900D, 0x6EB894AF, 0xEDA4CC14, 0xEE8DD7BB,
    0x324AE455, 0x57956878, 0x8C9EA5E3, 0xF62486FD, 0x8E40A97C, 0x0A2684B4,
    0x82688170, 0x579A75A8, 0x2346AC83, 0x53A541E3, 0xCDE21FA7, 0xADDA8D8F,
    0x7F224038, 0x2672CD32, 0xBC439D27, 0x835E185D, 0x950F047B, 0x42DFF3A2,
    0x4FA7563D, 0x5032DB70, 0x4D7928B6, 0x69236A41, 0xAA85D783, 0xA2B4D027,
    0x26E44982, 0x632A0C6C, 0x51808D77, 0x94360225, 0xB29D439D, 0x66811DD7,
    0xAE8FC672, 0x0A416E41, 0xF73623CB, 0x8D961B29,
};

const u32 sDownloadKey[4] = {0xD2D4865C, 0xB79FDD9D, 0xAAD6C9D3, 0xB4EDBC6C};

// 800E925C: errors that must be retained even when encountered while
// downloading.
BOOL isPersistentError(NWC24Err error) {
    switch (error) {
    case NWC24_ERR_FILE_BROKEN:
    case NWC24_ERR_INTERNAL_VF:
    case NWC24_ERR_INTERNAL_IPC:
    case NWC24_ERR_FILE_EXISTS:
    case NWC24_ERR_NAND_CORRUPT:
    case NWC24_ERR_FILE_OTHER:
    case NWC24_ERR_FILE_NOEXISTS:
    case NWC24_ERR_FILE_WRITE:
    case NWC24_ERR_FILE_READ:
    case NWC24_ERR_FILE_CLOSE:
    case NWC24_ERR_FILE_OPEN:
    case NWC24_ERR_BROKEN:
    case NWC24_ERR_NOMEM:
    case NWC24_ERR_FATAL:
        return TRUE;
    default:
        return FALSE;
    }
}

// 800E9290
BOOL isVfError(int error, int *deviceError) {
    switch (error) {
    case 5:
        *deviceError = VFGetLastDeviceError("C");
        return TRUE;
    case -1:
    case 8:
    case 12:
    case 13:
    case 17:
    case 21:
    case 22:
    case 23:
    case 24:
    case 27:
    case 28:
    case 90:
    case 0xB002:
    case 0xB003:
    case 0xB004:
    case 0xB005:
        return TRUE;
    case 2:
    case 9:
    case 0xB001:
    case 0xB006:
    default:
        return FALSE;
    }
}

// 800E9390: consume a downloaded file after its completion marker is clear.
BOOL readTaskFile(NWC24DlTask *task, void *data, u32 capacity, u32 *size,
                  int *error) {
    char path[0x208];
    path[0] = 0;
    fn_80410258(task, path, sizeof(path));
    VFFile file = VFOpenFile(path, "r", 0);
    if (file != NULL) {
        if (data != NULL) {
            int length = VFGetFileSizeByFd(file);
            if (length < 0) {
                *error = VFGetLastError();
            } else if ((u32)length <= capacity) {
                u32 lastUpdate;
                u32 lastError;
                lastError = 0;
                lastUpdate = 0;
                if (fn_8041100C(task, &lastUpdate, &lastError) == NWC24_OK &&
                    lastError == 0) {
                    *error = VFReadFile(file, data, length, size);
                }
            }
        }
        VFCloseFile(file);
        if (data != NULL) {
            VFDeleteFile(path);
        }
        return FALSE;
    }
    *error = VFGetLastError();
    return *error != VF_ERROR_0002;
}

// 800E94A8
u32 getHeapSize() {
    return HEAP_SIZE;
}
// 800E94B4
u32 getDownloadResourceSize() {
    return DOWNLOAD_RESOURCE_SIZE;
}
// 800E94C0
u32 getDownloadAuxSize() {
    return DOWNLOAD_AUX_SIZE;
}

// 800E94C8
dNetWc24_c::dNetWc24_c() {
    mUserId = 0;
    mVfWork = NULL;
    mLibraryWork = NULL;
    mMessageBuffer = NULL;
    mCryptBuffer = NULL;
    mLastError = NWC24_OK;
    mErrorCode = 0;
    mNandState = 0;
    mNandResult = 0;
    mErrorKind = ERROR_NONE;
    mOpened = FALSE;
    clearError();
    mTaskState[0] = TASK_UNCHECKED;
    mTaskState[1] = TASK_UNCHECKED;
}

// 800E953C
dNetWc24_c::~dNetWc24_c() {
}

// 800E957C
BOOL dNetWc24_c::open() {
    if (mOpened) {
        return TRUE;
    }
    if (!fn_80434A0C()) {
        mVfWork = dHeap::netWc24Heap_p->alloc(LIBRARY_WORK_SIZE, 32);
        fn_80434A20(mVfWork, LIBRARY_WORK_SIZE);
    }
    if (mLibraryWork == NULL) {
        mLibraryWork = dHeap::netWc24Heap_p->alloc(LIBRARY_WORK_SIZE, 32);
    }
    if (mMessageBuffer == NULL) {
        mMessageBuffer =
            (u8 *)dHeap::netWc24Heap_p->alloc(MESSAGE_WORK_SIZE, 32);
        mCryptBuffer = mMessageBuffer + MESSAGE_CAPACITY;
    }
    if (setError(fn_8040460C(mLibraryWork), ERROR_OPEN)) {
        dHeap::netWc24Heap_p->free(mMessageBuffer);
        mMessageBuffer = NULL;
        dHeap::netWc24Heap_p->free(mLibraryWork);
        mLibraryWork = NULL;
        if (mVfWork != NULL) {
            fn_80434AE8();
            dHeap::netWc24Heap_p->free(mVfWork);
            mVfWork = NULL;
        }
        return FALSE;
    }
    NWC24GetMyUserId(&mUserId);
    clearError();
    mOpened = TRUE;
    return TRUE;
}

// 800E9708
void dNetWc24_c::close() {
    if (mOpened) {
        fn_804047F8();
        dHeap::netWc24Heap_p->free(mMessageBuffer);
        mMessageBuffer = NULL;
        dHeap::netWc24Heap_p->free(mLibraryWork);
        mLibraryWork = NULL;
        if (mVfWork != NULL) {
            fn_80434AE8();
            dHeap::netWc24Heap_p->free(mVfWork);
            mVfWork = NULL;
        }
        mOpened = FALSE;
    }
}
// 800E97B8
BOOL dNetWc24_c::suspendScheduler() {
    return NWC24SuspendScheduler() >= 0;
}
// 800E97E0
int dNetWc24_c::trySuspendScheduler() {
    return NWC24TrySuspendScheduler();
}
// 800E97E4
int dNetWc24_c::resumeScheduler() {
    return NWC24ResumeScheduler();
}
// 800E97E8
BOOL dNetWc24_c::canUseFriends() {
    return !setError(fn_80404954(4), ERROR_CHECK);
}
// 800E9830
BOOL dNetWc24_c::canSendMessages() {
    return !setError(fn_80404954(1), ERROR_CHECK);
}
// 800E9878
BOOL dNetWc24_c::canDownload() {
    return !setError(fn_80404954(2), ERROR_CHECK);
}

// 800E98C0
BOOL dNetWc24_c::readFriend(u32 idx, FriendInfo_c *info) {
    NWC24Err error = NWC24IsFriendInfoThere(idx);
    if (error != 1) {
        setError(error, ERROR_DOWNLOAD);
        return FALSE;
    }
    if (setError(NWC24ReadFriendInfo(info, idx), ERROR_DOWNLOAD)) {
        return FALSE;
    }
    if (NWC24CheckFriendInfo(info) != NWC24_OK) {
        return FALSE;
    }
    return info->mStatus == FRIEND_CONFIRMED;
}
// 800E997C
BOOL dNetWc24_c::getFriendKey(u32 idx, u64 *key) {
    FriendInfo_c info ALIGN(32);
    if (readFriend(idx, &info) == FALSE) {
        return FALSE;
    }
    if (info.mType != FRIEND_WII) {
        return FALSE;
    }
    *key = info.mAddr.mKey;
    return TRUE;
}
// 800E99F4
u32 dNetWc24_c::getFriends(dNetWc24Friend_c *friends, u32 max) {
    u32 count ALIGN(32);
    FriendInfo_c info ALIGN(32);
    int num = 0;
    if (!setError(fn_8040DCA4(&count), ERROR_DOWNLOAD)) {
        for (u32 i = 0; i < count; i++) {
            if (readFriend(i, &info)) {
                friends[num].mIdx = i;
                friends[num].mType = info.mType;
                memcpy(friends[num].mName, info.mName, sizeof(info.mName));
                num++;
                if (num >= max) {
                    break;
                }
            }
        }
    }
    return num;
}

// 800E9AC8
BOOL dNetWc24_c::commitMessage(NWC24MsgObj *msg, u32 size, int profileId) {
    if (profileId != 0 && fn_80405940(msg, profileId) != NWC24_OK) {
        return FALSE;
    }
    if (fn_804055FC(msg, mMessageBuffer, size,
                    NWC24_APPLICATION_OCTET_STREAM) != NWC24_OK) {
        return FALSE;
    }
    return !setError(fn_80408D7C(msg), ERROR_SEND);
}
// 800E9B70
BOOL dNetWc24_c::sendMessage(u64 *keys, int num, const dNetWc24Data_c *data,
                             u16 tag, int profileId) {
    NWC24MsgObj msg;
    BOOL hasRecipient = FALSE;
    if (NWC24InitMsgObj(&msg, NWC24_MSGTYPE_RVL_HIDDEN) != NWC24_OK) {
        return FALSE;
    }
    for (int i = 0; i < num; i++) {
        NWC24Err error = fn_8040F0F0(*keys);
        if ((error == NWC24_OK || error == NWC24_ERR_ID_GENERATED) &&
            NWC24SetMsgToId(&msg, *keys) == NWC24_OK) {
            hasRecipient = TRUE;
        } else {
            *keys = 0;
        }
        keys++;
    }
    if (hasRecipient == FALSE) {
        return FALSE;
    }
    u32 size = makeMessage(data->mData[2], data->mSize[2], data->mData[3],
                           data->mSize[3], tag);
    return commitMessage(&msg, size, profileId);
}
// 800E9C74
BOOL dNetWc24_c::sendMessage(const u64 *key, u32 size, int profileId) {
    NWC24MsgObj msg;
    if (NWC24InitMsgObj(&msg, NWC24_MSGTYPE_RVL_HIDDEN) != NWC24_OK) {
        return FALSE;
    }
    NWC24Err error = fn_8040F0F0(*key);
    if (error != NWC24_OK && error != NWC24_ERR_ID_GENERATED) {
        return FALSE;
    }
    if (NWC24SetMsgToId(&msg, *key) != NWC24_OK) {
        return FALSE;
    }
    return commitMessage(&msg, size, profileId);
}

// 800E9D34
BOOL dNetWc24_c::sendBoardMessage(const dPostRequest_c *req) {
    u64 key;
    FriendInfo_c info ALIGN(32);
    u32 type;
    NWC24MsgObj msg;
    wchar_t subject[MAX_SUBJECT_LENGTH];
    if ((u32)req->mFriend != (u32)FRIEND_SELF) {
        if (readFriend(req->mFriend, &info) == FALSE) {
            return FALSE;
        }
        type = info.mType;
        if (type == FRIEND_WII) {
            key = info.mAddr.mKey;
        }
    } else {
        type = FRIEND_WII;
        if (NWC24GetMyUserId(&key) != NWC24_OK) {
            return FALSE;
        }
    }
    if (type == FRIEND_WII) {
        if (NWC24InitMsgObj(&msg, NWC24_MSGTYPE_RVL_MENU) != NWC24_OK) {
            return FALSE;
        }
        if (NWC24SetMsgToId(&msg, key) != NWC24_OK) {
            return FALSE;
        }
        if (fn_804054C4(&msg, req->mData[1], req->mSize[1], NWC24_UTF_16BE,
                        (u32)req->mFriend != (u32)FRIEND_SELF
                            ? NWC24_ENC_BASE64
                            : NWC24_ENC_8BIT) != NWC24_OK) {
            return FALSE;
        }
        fn_804058E8(&msg, TRUE);
        u32 length = req->mSize[0] / 2;
        if (length < MAX_SUBJECT_LENGTH) {
            for (u32 i = 0; i < length; i++) {
                subject[i] = ((const wchar_t *)req->mData[0])[i];
            }
            subject[length] = 0;
            if (fn_80405864(&msg, subject, length + 1) != NWC24_OK) {
                return FALSE;
            }
        } else {
            return FALSE;
        }
        if ((u32)req->mFriend != (u32)FRIEND_SELF) {
            fn_80405970(&msg, BOARD_LETTER_TAG);
        }
        if (req->mSize[2] != 0 && req->mData[2] != NULL &&
            fn_804055FC(&msg, req->mData[2], req->mSize[2],
                        NWC24_X_WII_MSGBOARD) != NWC24_OK) {
            return FALSE;
        }
        if (req->mSize[3] != 0 && req->mData[3] != NULL &&
            fn_804055FC(&msg, req->mData[3], req->mSize[3], NWC24_IMAGE_JPEG) !=
                NWC24_OK) {
            return FALSE;
        }
    } else {
        if (NWC24InitMsgObj(&msg, NWC24_MSGTYPE_PUBLIC) != NWC24_OK) {
            return FALSE;
        }
        if (fn_8040538C(&msg, info.mAddr.mAddress,
                        strlen(info.mAddr.mAddress)) != NWC24_OK) {
            return FALSE;
        }
        if (fn_8040B410(&msg, (const wchar_t *)req->mData[0], req->mSize[0] / 2,
                        (const wchar_t *)req->mData[1], req->mSize[1] / 2, TRUE,
                        0, mMessageBuffer, MESSAGE_CAPACITY) != NWC24_OK) {
            return FALSE;
        }
        if (req->mSize[3] != 0 && req->mData[3] != NULL &&
            fn_804055FC(&msg, req->mData[3], req->mSize[3], NWC24_IMAGE_JPEG) !=
                NWC24_OK) {
            return FALSE;
        }
    }
    if (setError(fn_80408D7C(&msg), ERROR_SEND)) {
        return FALSE;
    }
    return TRUE;
}

// 800EA0DC
void dNetWc24_c::receiveMessages(ReceiveCallback callback) {
    NWC24MsgObj msg;
    BOOL found, more;
    u32 attachmentCount;
    NWC24MsgType type;
    NWC24MIMEType mime;
    u32 size;
    u32 app;
    u32 id;
    u64 sender;
    if (fn_8040C250() != NWC24_OK) {
        return;
    }
    if (fn_8040C2C8(1) != NWC24_OK) {
        return;
    }
    int remaining = MAX_RECEIVE_MESSAGES;
    do {
        if (fn_8040C338(&msg, 1, &found, &more) != NWC24_OK) {
            break;
        }
        if (found && fn_80405B90(&msg, &app) == NWC24_OK &&
            (app & MESSAGE_REGION_MASK) == MESSAGE_GAME_CODE &&
            fn_80405A3C(&msg, &type) == NWC24_OK &&
            type != NWC24_MSGTYPE_RVL_MENU && type != NWC24_MSGTYPE_PUBLIC) {
            BOOL remove = TRUE;
            if (fn_80405AB4(&msg, &attachmentCount) == NWC24_OK) {
                fn_80405BA0(&msg, &sender);
                if (attachmentCount != 0) {
                    for (u32 i = 0; i < attachmentCount; i++) {
                        if (fn_80405B04(&msg, i, &mime) == NWC24_OK &&
                            mime == NWC24_APPLICATION_OCTET_STREAM &&
                            fn_80405AD0(&msg, i, &size) == NWC24_OK &&
                            fn_8040AFB4(&msg, i, mCryptBuffer,
                                        MESSAGE_CAPACITY) == NWC24_OK &&
                            decodeMessage(&sender, mCryptBuffer, size)) {
                            Message_c *message = (Message_c *)mMessageBuffer;
                            remove = callback(
                                &sender, mMessageBuffer + sizeof(Message_c),
                                message->mSize, &message->mHeader);
                        }
                    }
                }
                remaining--;
            }
            if (remove) {
                fn_80405B80(&msg, &id);
                fn_80406054(1, id);
            }
        }
    } while (more && remaining > 0);
}

// 800EA2C8
u32 dNetWc24_c::makeMessage(const void *data, u32 size, const void *data2,
                            u32 size2, u16 tag) {
    if (size + size2 + MESSAGE_MAX_OVERHEAD > MESSAGE_CAPACITY) {
        return 0;
    }
    u64 sender = 0;
    NWC24GetMyUserId(&sender);
    Message_c *msg = (Message_c *)mCryptBuffer;
    memcpy(msg->mMagic, sMessageMagic, sizeof(sMessageMagic));
    msg->mSender = sender;
    msg->mHeaderWords.mTag = tag;
    msg->mVersion = MESSAGE_VERSION;
    s16 parts = 0;
    if (size != 0 && data != NULL) {
        memcpy(mCryptBuffer + sizeof(Message_c), data, size);
        parts = 1;
    } else {
        size = 0;
    }
    if (size2 != 0 && data2 != NULL) {
        memcpy(mCryptBuffer + size + sizeof(Message_c), data2, size2);
        msg->mHeader.mOffset = size;
        size += size2;
        parts++;
    }
    msg->mSize = size;
    u32 encryptedSize = (size + MESSAGE_ROUNDING_OVERHEAD) & ~15;
    size = encryptedSize - AUTH_TRAILER_SIZE;
    msg->mHeader.mPartNum = parts;
    u8 authKey[AUTH_KEY_SIZE];
    memcpy(authKey, &sender, 8);
    float authNonce = cM::rnd();
    memcpy(authKey + 8, &authNonce, 4);
    authKey[12] = 0x45;
    authKey[13] = 0x30;
    authKey[14] = 0x39;
    authKey[15] = 0xCB;
    authKey[16] = 0xFE;
    authKey[17] = 0x19;
    authKey[18] = 0xDA;
    authKey[19] = 0x72;
    u8 digest[AUTH_DIGEST_SIZE];
    u8 authWork[AUTH_WORK_SIZE];
    u8 *cryptBuffer = getCryptBuffer();
    fn_80413D44(authWork, fn_8041399C(), authKey, sizeof(authKey));
    fn_80413F80(authWork, cryptBuffer, size);
    fn_80413F90(authWork, digest);
    memcpy(mCryptBuffer + size, &authNonce, 4);
    memcpy(mCryptBuffer + size + 4, digest, sizeof(digest));
    float iv[CIPHER_IV_WORDS];
    for (u32 i = 0; i < 4; i++) {
        iv[i] = cM::rnd();
    }
    float cryptNonce = cM::rnd();
    u8 cryptKey[CIPHER_KEY_SIZE];
    memcpy(cryptKey, &cryptNonce, 4);
    memcpy(cryptKey + 4, &sender, 8);
    cryptKey[12] = 0x71;
    cryptKey[13] = 0x71;
    cryptKey[14] = 0x71;
    cryptKey[15] = 0x71;
    u8 cryptWork[CIPHER_WORK_SIZE];
    fn_80414948(cryptWork, cryptKey, sizeof(cryptKey), iv);
    fn_80414954(cryptWork, mMessageBuffer, mCryptBuffer, encryptedSize);
    fn_80414950(cryptWork);
    memcpy(mMessageBuffer + encryptedSize, iv, sizeof(iv));
    memcpy(mMessageBuffer + encryptedSize + sizeof(iv), &cryptNonce, 4);
    return encryptedSize + CIPHER_TRAILER_SIZE;
}

// 800EA584
BOOL dNetWc24_c::decodeMessage(const u64 *key, const void *data, u32 size) {
    u32 encryptedSize = size - CIPHER_TRAILER_SIZE;
    const u8 *bytes = (const u8 *)data;
    u8 cryptKey[CIPHER_KEY_SIZE];
    u8 cryptWork[CIPHER_WORK_SIZE];
    u8 authKey[AUTH_KEY_SIZE];
    u8 authWork[AUTH_WORK_SIZE];
    u8 digest[AUTH_DIGEST_SIZE];
    memcpy(cryptKey, bytes + encryptedSize + 0x10, 4);
    memcpy(cryptKey + 4, key, 8);
    cryptKey[12] = 0x71;
    cryptKey[13] = 0x71;
    cryptKey[14] = 0x71;
    cryptKey[15] = 0x71;
    if (fn_80414948(cryptWork, cryptKey, sizeof(cryptKey),
                    bytes + encryptedSize) == FALSE) {
        return FALSE;
    }
    if (fn_80414A0C(cryptWork, mMessageBuffer, data, encryptedSize) == FALSE) {
        return FALSE;
    }
    fn_80414950(cryptWork);
    memcpy(authKey, key, 8);
    memcpy(authKey + 8, encryptedSize - AUTH_TRAILER_SIZE + mMessageBuffer, 4);
    authKey[12] = 0x45;
    authKey[13] = 0x30;
    authKey[14] = 0x39;
    authKey[15] = 0xCB;
    authKey[16] = 0xFE;
    authKey[17] = 0x19;
    authKey[18] = 0xDA;
    authKey[19] = 0x72;
    u8 *message = getMessageBuffer();
    fn_80413D44(authWork, fn_8041399C(), authKey, sizeof(authKey));
    u32 authenticatedSize = encryptedSize - AUTH_TRAILER_SIZE;
    fn_80413F80(authWork, message, authenticatedSize);
    fn_80413F90(authWork, digest);
    Message_c *msg = (Message_c *)mMessageBuffer;
    if (memcmp(digest, encryptedSize - 0x14 + (u8 *)msg, sizeof(digest)) != 0) {
        return FALSE;
    }
    if (msg->mVersion != MESSAGE_VERSION) {
        return FALSE;
    }
    if (msg->mSender != *key) {
        return FALSE;
    }
    return strcmp(msg->mMagic, sMessageMagic) == 0;
}

// 800EA760
BOOL dNetWc24_c::setError(NWC24Err error, int kind) {
    if (error != NWC24_OK) {
        if (kind != ERROR_DOWNLOAD || isPersistentError(error)) {
            mLastError = error;
            mErrorCode = -fn_80404B40();
            mErrorKind = kind;
        }
        return TRUE;
    }
    return FALSE;
}
// 800EA7E0
void dNetWc24_c::clearError() {
    mLastError = NWC24_OK;
    mErrorCode = 0;
    mErrorKind = ERROR_NONE;
    mNandState = 0;
    mNandResult = 0;
}
// 800EA800
BOOL dNetWc24_c::registerFriendRec(const u64 *key, int profileId,
                                   u32 gameCode) {
    dNetWc24FriendRec_c rec ALIGN(32);
    rec.mKey = *key;
    rec.mProfileId = profileId;
    rec.mGameCode = gameCode;
    if (fn_8040E2A4(&rec) == NWC24_OK) {
        return TRUE;
    }
    return FALSE;
}
// 800EA860
void dNetWc24_c::cleanFriendRecs(FriendRecCallback keep) {
    u32 count ALIGN(32);
    SecretFriend_c info ALIGN(32);
    if (fn_8040E4FC(&count) != NWC24_OK) {
        return;
    }
    for (u32 i = 0; i < count; i++) {
        if (fn_8040E388(&info, i) == NWC24_OK && keep(&info.mRec) == FALSE) {
            // The release build has no additional action for either removal
            // result.
            switch (fn_8040E324(&info)) {
            case NWC24_OK:
                break;
            default:
                break;
            }
        }
    }
}
// 800EA900
BOOL dNetWc24_c::prepareTask(int task) {
    NWC24DlTask *entry = &mTasks[task];
    if (fn_804103EC(entry, task) == NWC24_OK) {
        configureTask(task);
        mTaskState[task] = TASK_NEW;
        return TRUE;
    }
    if (mTaskState[task] == TASK_REGISTERED) {
        return TRUE;
    }
    if (setError(fn_8040FC7C(entry, NWC24_DLTYPE_OCTETSTREAM_V2),
                 ERROR_DOWNLOAD)) {
        return FALSE;
    }
    if (configureTask(task) == FALSE) {
        return FALSE;
    }
    mTaskState[task] = TASK_REGISTERED;
    return TRUE;
}
// 800EA9F0
BOOL dNetWc24_c::configureTask(int task) {
    NWC24DlTask *entry = getTask(task);
    if (setError(fn_804100FC(entry, DOWNLOAD_PRIORITY), ERROR_DOWNLOAD)) {
        return FALSE;
    }
    if (setError(fn_80410300(entry, sDownloadCounts[task]), ERROR_DOWNLOAD)) {
        return FALSE;
    }
    if (setError(fn_80410000(entry, sDownloadUrls[task]), ERROR_DOWNLOAD)) {
        return FALSE;
    }
    if (setError(fn_8040FEB8(entry, sDownloadIntervals[task]),
                 ERROR_DOWNLOAD)) {
        return FALSE;
    }
    if (setError(fn_8040FE18(entry, sDownloadFlags[task]), ERROR_DOWNLOAD)) {
        return FALSE;
    }
    if (setError(fn_804101B0(entry, sDownloadNames[task]), ERROR_DOWNLOAD)) {
        return FALSE;
    }
    return !setError(fn_8041039C(entry, task), ERROR_DOWNLOAD);
}
// 800EAB74
BOOL dNetWc24_c::needsDownloadSetup() {
    u8 type;
    return NANDGetType("wc24pubk.mod", &type) == NAND_RESULT_NOEXISTS;
}

// 800EABAC
BOOL dNetWc24_c::setupDownload() {
    // Install the download signature verification modulus and AES key.
    return !setError(fn_80411238(sDownloadPublicKey, sDownloadKey),
                     ERROR_DOWNLOAD);
}
// 800EAC00
int dNetWc24_c::checkTasks() {
    for (int i = 0; i < TASK_COUNT; i++) {
        if (prepareTask(i)) {
            return readDownload(i, NULL, 0, NULL);
        }
    }
    return READ_ERROR;
}
// 800EAC74
BOOL dNetWc24_c::registerTasks() {
    NWC24DlTask *task = NULL;
    for (int i = 0; i < TASK_COUNT; i++) {
        if (prepareTask(i)) {
            task = &mTasks[i];
            break;
        }
    }
    if (task == NULL) {
        return FALSE;
    }
    return !setError(fn_80411118(task, DOWNLOAD_RESOURCE_SIZE), ERROR_DOWNLOAD);
}
// 800EAD18
BOOL dNetWc24_c::deleteTasks() {
    BOOL ok = TRUE;
    for (int i = 0; i < TASK_COUNT; i++) {
        if (fn_804103EC(&mTasks[i], i) == NWC24_OK) {
            if (setError(fn_80410AEC(&mTasks[i]), ERROR_DOWNLOAD)) {
                ok = FALSE;
            }
        }
    }
    char path[0x40];
    u8 type;
    if (fn_8041109C(path, sizeof(path)) == NWC24_OK &&
        VFMountDriveNANDFlashEx("C", path) == VF_OK) {
        VFUnmountDrive("C");
        mNandResult = NANDDelete(path);
        if (mNandResult != NAND_RESULT_OK) {
            mNandState = NAND_DELETE_ERROR;
            ok = FALSE;
        }
    }
    if (NANDGetType("wc24pubk.mod", &type) != NAND_RESULT_NOEXISTS) {
        if (setError(fn_8041135C(), ERROR_DOWNLOAD)) {
            ok = FALSE;
        }
    }
    return ok;
}
// 800EAE38
int dNetWc24_c::readDownload(int task, void *data, u32 capacity, u32 *size) {
    if (size != NULL) {
        *size = 0;
    }
    int error = 0;
    char path[0x40];
    path[0] = 0;
    fn_8041109C(path, sizeof(path));
    error = VFMountDriveNANDFlashEx("C", path);
    BOOL missing;
    if (error == VF_OK) {
        missing = readTaskFile(&mTasks[task], data, capacity, size, &error);
        VFUnmountDrive("C");
    } else {
        missing = TRUE;
    }
    if (missing) {
        NANDDelete(path);
    }
    if (isVfError(error, &mNandResult)) {
        mNandState = error;
        return READ_ERROR;
    }
    return missing ? READ_MISSING : READ_DONE;
}
// 800EAF28
BOOL dNetWc24_c::download(int task, DownloadCallback callback) {
    if (prepareTask(task) == FALSE) {
        return FALSE;
    }
    u32 size = 0;
    if (readDownload(task, mMessageBuffer, MESSAGE_CAPACITY, &size) != 0) {
        return FALSE;
    }
    if (size != 0) {
        callback(mMessageBuffer, size, FALSE);
    }
    // A successful prepareTask leaves this entry either registered or new.
    NWC24Err error;
    if (mTaskState[task] == TASK_REGISTERED) {
        error = fn_80410BE8(&mTasks[task]);
    } else if (mTaskState[task] == TASK_NEW) {
        error = fn_80410904(&mTasks[task]);
    }
    return !setError(error, ERROR_DOWNLOAD);
}

// 800EB024: compiler-generated static initialization and destructor
// registration.
dNetWc24_c g_wc24;

typedef char CheckWc24Size[sizeof(dNetWc24_c) == 0x438 ? 1 : -1];
typedef char CheckWc24Id[offsetof(dNetWc24_c, mUserId) == 0x400 ? 1 : -1];
typedef char CheckMessageSize[sizeof(Message_c) == 0x20 ? 1 : -1];

} // namespace dNetWc24
