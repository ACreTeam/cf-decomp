#include <private/nwc24.h>
#include <revolution/nwc24.h>

#define SECRET_FRIEND_LIST_MAGIC 'WcFs'
#define SECRET_FRIEND_LIST_VERSION 2

#define LIB_CHECK()                                               \
    if (!NWC24IsMsgLibOpened() && !NWC24IsMsgLibOpenedByTool()) { \
        return NWC24_ERR_LIB_NOT_OPENED;                          \
    }

static const char* FLFilePath = "/shared2/wc24/nwc24fls.bin";

static NWC24Err GetCachedSecretFLHeader(NWC24SecretFLHeader** header);

static inline NWC24Err ReadInfo(NWC24SecretFriendInfo* info, u32 index) {
    NWC24Err result = NWC24iReadSecretFriendInfoRaw(info, index);
    if (result != NWC24_OK) {
        return result;
    }
    return NWC24_OK;
}

NWC24Err NWC24AddSecretFriendInfo(const NWC24SecretFriendInfo* info) {
    NWC24Err result;

    result = NWC24CheckUserId(info->id);
    if (result != NWC24_OK && result != NWC24_ERR_ID_GENERATED) {
        return result;
    }

    if (info->appId != 0 && (info->appId & ~0xFF) != (NWC24GetAppId() & ~0xFF)) {
        return NWC24_ERR_INVALID_VALUE;
    }

    return NWC24iAddSecretFriendInfo(info, NULL);
}

NWC24Err NWC24DeleteSecretFriendInfo(const NWC24SecretFriendInfo* info) {
    NWC24Err result;
    u32 index;

    if ((info->appId & ~0xFF) != (NWC24GetAppId() & ~0xFF)) {
        return NWC24_ERR_PROTECTED;
    }

    result = NWC24SearchSecretFriendInfo(info, &index);
    if (result == NWC24_OK) {
        result = NWC24iDeleteSecretFriendInfo(index);
    }

    return result;
}

NWC24Err NWC24ReadSecretFriendInfo(NWC24SecretFriendInfo* info, u32 index) {
    NWC24Err result;

    NWC24iRegisterSecretFList();

    return ReadInfo(info, index);
}

NWC24Err NWC24SearchSecretFriendInfo(const NWC24SecretFriendInfo* info, u32* index) {
    NWC24Err result;
    u32 i;
    NWC24SecretFriendInfo* tmp;
    u32 found;

    LIB_CHECK();

    tmp = (NWC24SecretFriendInfo*)NWC24WorkP->mainWork;
    i = 0;
    do {
        result = NWC24iSearchSecretFriendKey(info->key, &found, i);
        if (result != NWC24_OK) {
            continue;
        }
        result = ReadInfo(tmp, found);
        if (result != NWC24_OK) {
            return result;
        }
        if ((tmp->appId & ~0xFF) != (info->appId & ~0xFF)) {
            i = found + 1;
        } else if (tmp->id != info->id) {
            i = found + 1;
        } else {
            *index = found;
            return NWC24_OK;
        }
    } while (result == NWC24_OK);

    return result;
}

NWC24Err NWC24GetNumSecretFriendInfos(u32* num) {
    NWC24Err result;
    NWC24SecretFLHeader* header;

    LIB_CHECK();

    result = GetCachedSecretFLHeader(&header);
    if (result != NWC24_OK) {
        return result;
    }

    *num = header->infoCount;
    return NWC24_OK;
}

NWC24Err NWC24iAddSecretFriendInfo(const NWC24SecretFriendInfo* info, u32* index) {
    NWC24Err result;
    NWC24SecretFriendInfo* tmp;
    u32 found;

    LIB_CHECK();

    if (((u32)info & 31) != 0) {
        return NWC24_ERR_ALIGNMENT;
    }

    tmp = (NWC24SecretFriendInfo*)NWC24WorkP;

    result = NWC24iSearchSecretFriendKey(info->key, &found, 0);
    while (result == NWC24_OK) {
        result = ReadInfo(tmp, found);
        if (result != NWC24_OK) {
            return result;
        }

        if ((tmp->appId & ~0xFF) != (NWC24GetAppId() & ~0xFF)) {
            result = NWC24iSearchSecretFriendKey(info->key, &found, found + 1);
        } else {
            if (tmp->id != info->id) {
                return NWC24_ERR_FORMAT;
            }

            result = NWC24iUpdateSecretFriendInfo(tmp, found);
            if (index != NULL) {
                *index = found;
            }
            return result;
        }
    }

    if (result != NWC24_OK && result != NWC24_ERR_NOT_FOUND) {
        return result;
    }

    result = NWC24iFindEmptySecretFriendSlot(&found);
    if (result == NWC24_OK) {
        result = NWC24iWriteSecretFriendInfo(info, found, FALSE);
        if (index != NULL) {
            *index = found;
        }
        return result;
    }

    result = NWC24iFindOldestSecretFriendSlot(&found);
    if (result != NWC24_OK) {
        return result;
    }

    result = NWC24iWriteSecretFriendInfo(info, found, TRUE);
    if (result != NWC24_OK) {
        return result;
    }

    if (index != NULL) {
        *index = found;
    }
    return result;
}

NWC24Err NWC24iFindEmptySecretFriendSlot(u32* index) {
    NWC24Err result;
    NWC24SecretFLHeader* header;
    u32 i;

    LIB_CHECK();

    result = GetCachedSecretFLHeader(&header);
    if (result != NWC24_OK) {
        return result;
    }

    if (header->registeredCount == header->infoCount) {
        return NWC24_ERR_NOT_FOUND;
    }

    for (i = 0; i < header->infoCount; i++) {
        if (header->keys[i] == 0) {
            *index = i;
            return NWC24_OK;
        }
    }

    return NWC24_ERR_NOT_FOUND;
}

NWC24Err NWC24iFindOldestSecretFriendSlot(u32* index) {
    NWC24Err result;
    NWC24SecretFLHeader* header;
    u32 i;
    u32 oldest;
    u32 oldestIdx;

    LIB_CHECK();

    result = GetCachedSecretFLHeader(&header);
    if (result != NWC24_OK) {
        return result;
    }

    if (header->infoCount == 0) {
        *index = 0;
        return NWC24_ERR_NULL;
    }

    oldest = 0xFFFF;
    oldestIdx = 0;
    for (i = 0; i < header->infoCount; i++) {
        if (header->times[i] < oldest) {
            oldest = header->times[i];
            oldestIdx = i;
        }
    }

    *index = oldestIdx;
    return NWC24_OK;
}

NWC24Err NWC24iSearchSecretFriendKey(s32 key, u32* index, u32 start) {
    NWC24Err result;
    NWC24SecretFLHeader* header;
    u32 i;

    LIB_CHECK();

    if (key == 0) {
        return NWC24_ERR_INVALID_VALUE;
    }

    result = GetCachedSecretFLHeader(&header);
    if (result != NWC24_OK) {
        return result;
    }

    for (i = start; i < header->infoCount; i++) {
        if (key == header->keys[i]) {
            *index = i;
            return NWC24_OK;
        }
    }

    return NWC24_ERR_NOT_FOUND;
}

NWC24Err NWC24iWriteSecretFriendInfo(const NWC24SecretFriendInfo* info, u32 index, BOOL replace) {
    NWC24Err result;
    NWC24Err resultFile;
    NWC24SecretFLHeader* header;
    NWC24SecretFriendInfo* tmp;
    NWC24File file;
    OSTime time;
    int newEntry = 1;

    LIB_CHECK();

    if (((u32)info & 31) != 0) {
        return NWC24_ERR_ALIGNMENT;
    }

    result = GetCachedSecretFLHeader(&header);
    if (result != NWC24_OK) {
        return result;
    }

    if (index >= header->infoCount) {
        return NWC24_ERR_INVALID_VALUE;
    }

    if (header->keys[index] != 0) {
        if (!replace) {
            return NWC24_ERR_FULL;
        }
        newEntry = 0;
    }

    tmp = (NWC24SecretFriendInfo*)NWC24WorkP->mainWork;
    Mail_memset(tmp, 0, sizeof(NWC24SecretFriendInfo));
    *tmp = *info;

    tmp->time = NWC24iGetUniversalTime(&time) == NWC24_OK ? (u32)time : 0x10000;

    if ((tmp->appId & ~0xFF) != (NWC24GetAppId() & ~0xFF)) {
        tmp->appId = NWC24GetAppId();
    }

    tmp->unk_0x14 = 0;

    result = NWC24FOpen(&file, FLFilePath, NWC24_OPEN_RW);
    if (result != NWC24_OK) {
        return result;
    }

    NWC24FSeek(&file, index * sizeof(NWC24SecretFriendInfo) + sizeof(NWC24SecretFLHeader), NWC24_SEEK_BEG);

    header->keys[index] = tmp->key;
    header->times[index] = tmp->time >> 16;

    resultFile = NWC24FWrite(tmp, sizeof(NWC24SecretFriendInfo), &file);
    if (resultFile == NWC24_OK) {
        header->registeredCount += newEntry;

        NWC24FSeek(&file, 0, NWC24_SEEK_BEG);
        resultFile = NWC24FWrite(header, sizeof(NWC24SecretFLHeader), &file);
    }

    result = NWC24FClose(&file);
    if (resultFile != NWC24_OK) {
        result = resultFile;
    }

    return result;
}

NWC24Err NWC24iUpdateSecretFriendInfo(const NWC24SecretFriendInfo* info, u32 index) {
    NWC24Err result;
    NWC24Err resultFile;
    NWC24SecretFLHeader* header;
    NWC24SecretFriendInfo* tmp;
    NWC24File file;
    OSTime time;

    LIB_CHECK();

    if (((u32)info & 31) != 0) {
        return NWC24_ERR_ALIGNMENT;
    }

    result = GetCachedSecretFLHeader(&header);
    if (result != NWC24_OK) {
        return result;
    }

    if (index >= header->infoCount) {
        return NWC24_ERR_INVALID_VALUE;
    }

    if (header->keys[index] == 0) {
        return NWC24_ERR_NULL;
    }

    tmp = (NWC24SecretFriendInfo*)NWC24WorkP->mainWork;
    Mail_memset(tmp, 0, sizeof(NWC24SecretFriendInfo));
    *tmp = *info;

    tmp->time = NWC24iGetUniversalTime(&time) == NWC24_OK ? (u32)time : 0x10000;

    result = NWC24FOpen(&file, FLFilePath, NWC24_OPEN_RW);
    if (result != NWC24_OK) {
        return result;
    }

    NWC24FSeek(&file, index * sizeof(NWC24SecretFriendInfo) + sizeof(NWC24SecretFLHeader), NWC24_SEEK_BEG);

    header->keys[index] = tmp->key;
    header->times[index] = tmp->time >> 16;

    resultFile = NWC24FWrite(tmp, sizeof(NWC24SecretFriendInfo), &file);
    if (resultFile == NWC24_OK) {
        NWC24FSeek(&file, 0, NWC24_SEEK_BEG);
        resultFile = NWC24FWrite(header, sizeof(NWC24SecretFLHeader), &file);
    }

    result = NWC24FClose(&file);
    if (resultFile != NWC24_OK) {
        result = resultFile;
    }

    return result;
}

NWC24Err NWC24iDeleteSecretFriendInfo(u32 index) {
    NWC24Err result;
    NWC24Err resultFile;
    NWC24SecretFLHeader* header;
    NWC24SecretFriendInfo* tmp;
    NWC24File file;

    LIB_CHECK();

    result = GetCachedSecretFLHeader(&header);
    if (result != NWC24_OK) {
        return result;
    }

    if (index >= header->infoCount) {
        return NWC24_ERR_INVALID_VALUE;
    }

    if (header->keys[index] == 0) {
        return NWC24_ERR_NULL;
    }

    tmp = (NWC24SecretFriendInfo*)NWC24WorkP->mainWork;
    Mail_memset(tmp, 0, sizeof(NWC24SecretFriendInfo));

    result = NWC24FOpen(&file, FLFilePath, NWC24_OPEN_RW);
    if (result != NWC24_OK) {
        return result;
    }

    NWC24FSeek(&file, index * sizeof(NWC24SecretFriendInfo) + sizeof(NWC24SecretFLHeader), NWC24_SEEK_BEG);

    resultFile = NWC24FWrite(tmp, sizeof(NWC24SecretFriendInfo), &file);
    if (resultFile == NWC24_OK) {
        header->keys[index] = 0;
        header->times[index] = 0;
        header->registeredCount--;

        NWC24FSeek(&file, 0, NWC24_SEEK_BEG);
        resultFile = NWC24FWrite(header, sizeof(NWC24SecretFLHeader), &file);
    }

    result = NWC24FClose(&file);
    if (resultFile != NWC24_OK) {
        result = resultFile;
    }

    return result;
}

NWC24Err NWC24iOpenSecretFriendList() {
    NWC24SecretFLHeader* header = (NWC24SecretFLHeader*)NWC24WorkP->secretFlHead;
    Mail_memset(header, 0, sizeof(NWC24SecretFLHeader));

    return GetCachedSecretFLHeader(&header);
}

static NWC24Err NWC24iInitSecretFriendList(BOOL force) {
    NWC24Err result;
    NWC24SecretFLHeader* header;

    result = NWC24iOpenSecretFriendList();

    if (result == NWC24_OK && !force) {
        return result;
    }

    if (result == NWC24_ERR_VER_MISMATCH) {
        result = GetCachedSecretFLHeader(&header);
        if (header->version > SECRET_FRIEND_LIST_VERSION) {
            return result;
        }
    }

    result = NWC24iCreateSecretFriendList();
    return result;
}

static NWC24Err NWC24iCreateSecretFriendList() {
    NWC24Err result;
    NWC24Err resultWrite;

    NWC24SecretFLHeader* header;
    NWC24SecretFriendInfo* tmpFriendInfo;

    NWC24File file;
    u32 i;

    header = (NWC24SecretFLHeader*)NWC24WorkP->secretFlHead;
    Mail_memset(header, 0, sizeof(NWC24SecretFLHeader));

    tmpFriendInfo = (NWC24SecretFriendInfo*)NWC24WorkP->mainWork;
    Mail_memset(tmpFriendInfo, 0, sizeof(NWC24SecretFriendInfo));

    header->magic = SECRET_FRIEND_LIST_MAGIC;
    header->version = SECRET_FRIEND_LIST_VERSION;
    header->infoCount = 336;
    header->registeredCount = 0;

    result = NWC24FOpen(&file, FLFilePath, NWC24_OPEN_WRITE);
    if (result != NWC24_OK) {
        return result;
    }

    resultWrite = NWC24FWrite(header, sizeof(NWC24SecretFLHeader), &file);
    if (resultWrite != NWC24_OK) {
        header->infoCount = 0;
    }

    for (i = 0; i < header->infoCount; i++) {
        resultWrite = NWC24FWrite(tmpFriendInfo, sizeof(NWC24SecretFriendInfo), &file);
        if (resultWrite != NWC24_OK) {
            break;
        }
    }

    result = NWC24FClose(&file);
    if (resultWrite != NWC24_OK) {
        return resultWrite;
    }

    return result;
}

NWC24Err NWC24iReadSecretFriendInfoRaw(NWC24SecretFriendInfo* info, u32 index) {
    NWC24Err result;
    NWC24Err resultRead;
    NWC24SecretFLHeader* header;
    NWC24File file;

    LIB_CHECK();

    if (((u32)info & 31) != 0) {
        return NWC24_ERR_ALIGNMENT;
    }

    result = GetCachedSecretFLHeader(&header);
    if (result != NWC24_OK) {
        return result;
    }

    if (index >= header->infoCount) {
        return NWC24_ERR_INVALID_VALUE;
    }

    if (header->keys[index] == 0) {
        return NWC24_ERR_NULL;
    }

    result = NWC24FOpen(&file, FLFilePath, NWC24_OPEN_READ | NWC24_OPEN_BUFF);
    if (result != NWC24_OK) {
        return result;
    }

    NWC24FSeek(&file, index * sizeof(NWC24SecretFriendInfo) + sizeof(NWC24SecretFLHeader), NWC24_SEEK_BEG);
    resultRead = NWC24FRead(info, sizeof(NWC24SecretFriendInfo), &file);

    result = NWC24FClose(&file);
    if (resultRead != NWC24_OK) {
        result = resultRead;
    }

    if (result != NWC24_OK) {
        return result;
    }

    return NWC24_OK;
}

static NWC24Err GetCachedSecretFLHeader(NWC24SecretFLHeader** header) {
    NWC24Err result;
    NWC24Err resultRead;
    NWC24Err resultClose;

    NWC24File file;

    *header = (NWC24SecretFLHeader*)NWC24WorkP->secretFlHead;

    if ((*header)->magic != SECRET_FRIEND_LIST_MAGIC) {
        result = NWC24FOpen(&file, FLFilePath, NWC24_OPEN_NAND_R);
        if (result != NWC24_OK) {
            return result;
        }

        NWC24FSeek(&file, 0, NWC24_SEEK_BEG);
        resultRead = NWC24FRead(*header, sizeof(NWC24SecretFLHeader), &file);
        resultClose = NWC24FClose(&file);

        if (resultRead != NWC24_OK) {
            result = resultRead;
        } else {
            result = resultClose;
        }

        if (result != NWC24_OK) {
            return result;
        }

        if ((*header)->magic != SECRET_FRIEND_LIST_MAGIC) {
            return NWC24_ERR_BROKEN;
        }

        if ((*header)->version != SECRET_FRIEND_LIST_VERSION) {
            return NWC24_ERR_VER_MISMATCH;
        }
    }

    return NWC24_OK;
}
