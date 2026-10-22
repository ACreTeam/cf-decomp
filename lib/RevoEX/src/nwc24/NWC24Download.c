#include <private/nwc24.h>
#include <revolution/nwc24.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* DLFilePath = "/shared2/wc24/nwc24dl.bin";

static s32 NWC24iLaxParameterCheck;

static BOOL IsPrivateId(u16 id);
static BOOL IsMyApp(u32 appId);
static BOOL IsGroupWritable(u16 groupId, u32 flags);
static NWC24iDlEntry* GetDlTaskEntryHeader(u16 id);
static NWC24Err WriteDlHeader(NWC24File* pFile);
static NWC24Err SeekDlTaskEntry(u16 id, NWC24File* pFile);
static NWC24Err CheckDlEntryAvailable(u16 id);
static NWC24Err WriteDlTaskEntry(NWC24iDlTask* pTask, NWC24File* pFile);
static NWC24Err ReadDlTaskEntry(NWC24iDlTask* pTask, u16 id, NWC24File* pFile);
static NWC24Err ClearDlTaskEntry(u16 id, NWC24File* pFile);
static NWC24Err ReadDlHeader(NWC24File* pFile);
static void InitTaskEntryHeader(u16 id);
static NWC24Err LoadDlTask(NWC24iDlTask* pTask, u16 id);
static NWC24Err StoreDlTask(NWC24iDlTask* pTask) NO_INLINE;
static NWC24Err AddTaskInternal(NWC24iDlTask* pTask, u16 minId, u16 maxId) NO_INLINE;
static NWC24Err DeleteDlTask(NWC24iDlTask* pTask) NO_INLINE;
static NWC24Err SeekAndWriteFile(const char* pPath, u32 offset, const void* pData, u32 size, u32 padSize) NO_INLINE;
static NWC24Err SearchDlTaskId(u16* pId, u32 appId, u32 userParam, BOOL matchParam) NO_INLINE;
static NWC24Err CheckHeader(NWC24iDlHeader* pHeader);
static s32 IterationPredicatorLastAccess(u16 id);
static s32 IterationPredicatorNextTime(u16 id);
static s32 IterationPredicatorPriority(u16 id);

NWC24Err NWC24CheckDlTask(const NWC24DlTask* pTask, BOOL wantWrite) {
    NWC24iDlTask* pTaskImpl;
    NWC24iDlHeader* pHeader;

    pTaskImpl = (NWC24iDlTask*)pTask;
    pHeader = NWC24iGetCachedDlHeader();

    if (pTaskImpl == 0) {
        return NWC24_ERR_INVALID_VALUE;
    }

    if (pHeader == 0) {
        return NWC24_ERR_LIB_NOT_OPENED;
    }

    if (pTaskImpl->appId == 0 || pTaskImpl->titleIdLo == 0) {
        return NWC24_ERR_INVALID_VALUE;
    }
    if (wantWrite && !NWC24IsMsgLibOpenedByTool() && !IsMyApp(pTaskImpl->appId)) {
        if (!IsGroupWritable(pTaskImpl->groupId, pTaskImpl->flags)) {
            return NWC24_ERR_PROTECTED;
        }
    }
    if (pTaskImpl->id != 0xFFFF && pTaskImpl->id >= pHeader->maxTasks) {
        return NWC24_ERR_INVALID_VALUE;
    }

    return NWC24_OK;
}

static BOOL IsOctetStream(NWC24DLType type) {
    if (type == NWC24_DLTYPE_OCTETSTREAM_V1 || type == NWC24_DLTYPE_OCTETSTREAM_V2) {
        return TRUE;
    }
    return FALSE;
}

static NWC24Err CheckDlUrl(const char* pUrl) {
    NWC24Err result;

    result = NWC24iCheckStringLength(pUrl, 7, 256);
    if (result < 0) {
        return result;
    }

    if (strncmp(pUrl, "http://", 7) != 0 && strncmp(pUrl, "https://", 8) != 0) {
        return NWC24_ERR_FORMAT;
    }

    return NWC24_OK;
}

static NWC24Err SetDlNextTime(const NWC24iDlTask* pTask, s64 time) {
    NWC24Err result;
    u16 id;

    result = NWC24CheckDlTask((const NWC24DlTask*)pTask, FALSE);
    if (result != NWC24_OK) {
        return result;
    }

    id = pTask->id;
    if (id == 0xFFFF) {
        return NWC24_ERR_FAILED;
    }

    GetDlTaskEntryHeader(id)->nextTime = time / 60;
    return NWC24_OK;
}

static NWC24Err SetDlLastAccess(const NWC24iDlTask* pTask, s64 time) {
    NWC24Err result;
    u16 id;

    result = NWC24CheckDlTask((const NWC24DlTask*)pTask, FALSE);
    if (result != NWC24_OK) {
        return result;
    }

    id = pTask->id;
    if (id == 0xFFFF) {
        return NWC24_ERR_FAILED;
    }

    GetDlTaskEntryHeader(id)->lastAccess = time / 60;
    return NWC24_OK;
}

static NWC24Err UpdateDlLastAccess(const NWC24iDlTask* pTask) {
    s64 now;
    NWC24Err result;

    result = NWC24iGetUniversalTime(&now);
    if (result >= 0) {
        result = SetDlLastAccess(pTask, now);
        if (result < 0) {
            return result;
        }
    }

    return result;
}

static NWC24Err ClearDlTaskError(NWC24iDlTask* pTask) {
    NWC24Err result;

    result = NWC24CheckDlTask((const NWC24DlTask*)pTask, TRUE);
    if (result != NWC24_OK) {
        return result;
    }

    pTask->lastError = 0;
    pTask->errorCount = 0;
    return NWC24_OK;
}

static NWC24Err CheckSubTaskEnabled(const NWC24iDlTask* pTask, u8 index) {
    NWC24Err result;

    result = NWC24CheckDlTask((const NWC24DlTask*)pTask, FALSE);
    if (result != NWC24_OK) {
        return result;
    }

    if (pTask->subTaskType == NWC24_DL_STTYPE_NONE) {
        return NWC24_ERR_INVALID_OPERATION;
    }

    if (pTask->subTaskMask == 0) {
        return NWC24_ERR_FATAL;
    }

    if (index > 31) {
        return NWC24_ERR_INVALID_VALUE;
    }

    return ((1 << index) & pTask->subTaskMask) == 0 ? NWC24_ERR_DISABLED : NWC24_OK;
}

NWC24Err NWC24InitDlTask(NWC24DlTask* pTask, NWC24DLType type) {
    NWC24iDlTask* pTaskImpl = (NWC24iDlTask*)pTask;
    char homeDir[NAND_MAX_PATH] = {0};
    u32 titleIdHi;
    u32 titleIdLo;

    NANDGetHomeDir(homeDir);
    homeDir[15] = '\0';
    titleIdHi = strtoul(homeDir + 7, NULL, 16);
    homeDir[24] = '\0';
    titleIdLo = strtoul(homeDir + 16, NULL, 16);

    if (NWC24iGetCachedDlHeader() == NULL) {
        return NWC24_ERR_LIB_NOT_OPENED;
    }

    if (pTaskImpl == NULL) {
        return NWC24_ERR_INVALID_VALUE;
    }

    if (type >= 4) {
        return NWC24_ERR_INVALID_VALUE;
    }

    memset(pTaskImpl, 0, sizeof(NWC24iDlTask));
    pTaskImpl->type = type;
    pTaskImpl->priority = 0xC0;
    pTaskImpl->appId = NWC24GetAppId();
    pTaskImpl->groupId = NWC24GetGroupId();
    pTaskImpl->titleIdHi = titleIdHi;
    pTaskImpl->titleIdLo = titleIdLo;
    pTaskImpl->id = 0xFFFF;
    pTaskImpl->count = 1;
    pTaskImpl->interval = 2880;
    pTaskImpl->margin = 1440;

    if (IsOctetStream(type)) {
        strcpy(pTaskImpl->fileName, "content.bin");
    }

    if (NWC24CheckDlTask(pTask, TRUE) < 0) {
        return NWC24_ERR_FATAL;
    }

    return NWC24_OK;
}

NWC24Err NWC24SetDlPriority(NWC24DlTask* pTask, u8 priority) {
    NWC24iDlTask* pTaskImpl = (NWC24iDlTask*)pTask;
    NWC24Err result;

    result = NWC24CheckDlTask(pTask, TRUE);
    if (result != NWC24_OK) {
        return result;
    }

    if (!NWC24iLaxParameterCheck && !IsPrivateId(pTaskImpl->id) && !(pTaskImpl->flags & NWC24_DL_FLAG_NO_LIMIT) &&
        priority < 0x80) {
        return NWC24_ERR_INVALID_VALUE;
    }

    pTaskImpl->priority = priority;
    return NWC24_OK;
}

NWC24Err NWC24SetDlInterval(NWC24DlTask* pTask, u16 interval) {
    NWC24iDlTask* pTaskImpl = (NWC24iDlTask*)pTask;
    NWC24Err result;

    result = NWC24CheckDlTask(pTask, TRUE);
    if (result != NWC24_OK) {
        return result;
    }

    if (!NWC24iLaxParameterCheck && !IsPrivateId(pTaskImpl->id) && !(pTaskImpl->flags & NWC24_DL_FLAG_NO_LIMIT) &&
        (interval < NWC24_DL_INTERVAL_MIN || interval > NWC24_DL_INTERVAL_MAX)) {
        return NWC24_ERR_INVALID_VALUE;
    }

    if (pTaskImpl->interval != interval) {
        pTaskImpl->interval = interval;

        if (pTaskImpl->id != 0xFFFF) {
            s64 now = 0;

            if (NWC24iGetUniversalTime(&now) >= 0) {
                SetDlNextTime(pTaskImpl, now + pTaskImpl->interval * 60);
            }
        }
    }

    return NWC24_OK;
}

NWC24Err NWC24SetDlUrl(NWC24DlTask* pTask, const char* pUrl) {
    NWC24iDlTask* pTaskImpl = (NWC24iDlTask*)pTask;
    NWC24Err result;

    result = NWC24CheckDlTask(pTask, TRUE);
    if (result != NWC24_OK) {
        return result;
    }

    result = CheckDlUrl(pUrl);
    if (result < 0) {
        return result;
    }

    if (!NWC24iLaxParameterCheck && (pTaskImpl->flags & NWC24_DL_FLAG_HTTPS_ONLY) && strncmp(pUrl, "http://", 7) == 0) {
        return NWC24_ERR_INVALID_VALUE;
    }

    NWC24iStrLCpy(pTaskImpl->url, pUrl, sizeof(pTaskImpl->url));
    return NWC24_OK;
}

NWC24Err NWC24SetDlFlags(NWC24DlTask* pTask, u32 flags) {
    NWC24iDlTask* pTaskImpl = (NWC24iDlTask*)pTask;
    NWC24Err result;

    result = NWC24CheckDlTask(pTask, TRUE);
    if (result != NWC24_OK) {
        return result;
    }

    if (!NWC24iLaxParameterCheck && (flags & NWC24_DL_FLAG_HTTPS_ONLY) && strncmp(pTaskImpl->url, "http://", 7) == 0) {
        return NWC24_ERR_INVALID_VALUE;
    }

    switch (pTaskImpl->type) {
    case NWC24_DLTYPE_MULTIPART_V1:
    case NWC24_DLTYPE_OCTETSTREAM_V1: {
        if (flags & 0x80000038) {
            return NWC24_ERR_INVALID_OPERATION;
        }
        break;
    }
    case NWC24_DLTYPE_MULTIPART_V2:
    case NWC24_DLTYPE_OCTETSTREAM_V2:
    default: {
        break;
    }
    }

    pTaskImpl->flags = flags;
    return NWC24_OK;
}

NWC24Err NWC24SetDlFilename(NWC24DlTask* pTask, const char* pFileName) {
    NWC24iDlTask* pTaskImpl = (NWC24iDlTask*)pTask;
    NWC24Err result;

    result = NWC24CheckDlTask(pTask, TRUE);
    if (result != NWC24_OK) {
        return result;
    }

    result = NWC24iCheckStringLength(pFileName, 1, NAND_MAX_PATH);
    if (result < 0) {
        return result;
    }

    if (!IsOctetStream(pTaskImpl->type)) {
        return NWC24_ERR_INVALID_OPERATION;
    }

    NWC24iStrLCpy(pTaskImpl->fileName, pFileName, NAND_MAX_PATH);
    return NWC24_OK;
}

NWC24Err NWC24GetDlFilename(const NWC24DlTask* pTask, char* pBuf, u32 size) {
    const NWC24iDlTask* pTaskImpl = (const NWC24iDlTask*)pTask;
    NWC24Err result;
    u32 len;

    result = NWC24CheckDlTask(pTask, FALSE);
    if (result != NWC24_OK) {
        return result;
    }

    len = strlen(pTaskImpl->fileName);
    if (len == 0) {
        return NWC24_ERR_NOT_FOUND;
    }

    if (len + 1 > size) {
        return NWC24_ERR_NOMEM;
    }

    strncpy(pBuf, pTaskImpl->fileName, len);
    pBuf[len] = '\0';
    return NWC24_OK;
}

NWC24Err NWC24SetDlCount(NWC24DlTask* pTask, s16 count) {
    NWC24iDlTask* pTaskImpl = (NWC24iDlTask*)pTask;
    NWC24Err result;

    result = NWC24CheckDlTask(pTask, TRUE);
    if (result != NWC24_OK) {
        return result;
    }

    if (count < 1 || (!IsPrivateId(pTaskImpl->id) && !(pTaskImpl->flags & NWC24_DL_FLAG_NO_LIMIT) && count > NWC24_DL_COUNT_MAX)) {
        return NWC24_ERR_INVALID_VALUE;
    }

    pTaskImpl->count = count;
    return NWC24_OK;
}

NWC24Err NWC24SetDlUserParam(NWC24DlTask* pTask, u32 param) {
    NWC24iDlTask* pTaskImpl = (NWC24iDlTask*)pTask;
    NWC24Err result;

    result = NWC24CheckDlTask(pTask, TRUE);
    if (result != NWC24_OK) {
        return result;
    }

    pTaskImpl->userParam = param;
    return NWC24_OK;
}

NWC24Err NWC24GetDlTask(NWC24DlTask* pTask, u16 id) {
    NWC24iDlTask* pTaskImpl;
    NWC24Err result;

    pTaskImpl = (NWC24iDlTask*)pTask;

    if (NWC24iGetCachedDlHeader() == NULL) {
        return NWC24_ERR_LIB_NOT_OPENED;
    }
    result = CheckDlEntryAvailable(id);
    if (result < 0) {
        return result;
    }

    result = LoadDlTask(pTaskImpl, id);
    if (result < 0) {
        return result;
    }

    return result;
}

NWC24Err NWC24GetDlTaskMine(NWC24DlTask* pTask, u32 param) {
    NWC24Err result;
    u16 id;

    result = SearchDlTaskId(&id, NWC24GetAppId(), param, TRUE);
    if (result < 0) {
        return result;
    }

    return NWC24GetDlTask(pTask, id);
}

NWC24Err NWC24IterateDlTask(u16* pId, BOOL first) {
    u16 id;

    if (NWC24iGetCachedDlHeader() == NULL) {
        return NWC24_ERR_LIB_NOT_OPENED;
    }

    if (pId == NULL) {
        return NWC24_ERR_INVALID_VALUE;
    }

    if (first) {
        *pId = 0;
    } else {
        (*pId)++;
    }

    if (*pId >= NWC24iGetCachedDlHeader()->maxTasks) {
        return NWC24_ERR_DONE;
    }

    id = *pId;
    do {
        if (CheckDlEntryAvailable(id) >= 0) {
            *pId = id;
            return NWC24_OK;
        }
        id++;
    } while (id < NWC24iGetCachedDlHeader()->maxTasks);

    return NWC24_ERR_DONE;
}

static BOOL IsKeyBefore(s32 lhs, s32 rhs, BOOL desc) {
    return desc ? rhs < lhs : lhs < rhs;
}

NWC24Err NWC24IterateDlTaskEx(NWC24DlIterateWork* pWork, u16* pId) {
    BOOL desc;
    BOOL found = FALSE;
    s32 (*getKey)(u16 id);
    NWC24Err result;
    u16 id;
    s32 key;

    if (!pWork->valid) {
        return NWC24_ERR_INVALID_VALUE;
    }

    desc = pWork->mode >> 31;

    switch (pWork->mode & 0xFFFF) {
    case 0: {
        getKey = IterationPredicatorLastAccess;
        break;
    }
    case 1: {
        getKey = IterationPredicatorNextTime;
        break;
    }
    case 2: {
        getKey = IterationPredicatorPriority;
        break;
    }
    default: {
        return NWC24_ERR_INVALID_VALUE;
    }
    }

    if (!pWork->first) {
        for (result = NWC24IterateDlTask(&id, TRUE); result >= 0; result = NWC24IterateDlTask(&id, FALSE)) {
            key = getKey(id);
            if (key == pWork->cur && pWork->lastId < id) {
                *pId = id;
                pWork->cur = key;
                pWork->lastId = id;
                pWork->prev = key;
                return NWC24_OK;
            }
        }
    } else {
        pWork->first = FALSE;
    }

    if (!(pWork->mode & 0x80000000)) {
        pWork->cur = 0x7FFFFFFF;
    } else {
        pWork->cur = 0x80000001;
    }

    for (result = NWC24IterateDlTask(&id, TRUE); result >= 0; result = NWC24IterateDlTask(&id, FALSE)) {
        key = getKey(id);

        if (IsKeyBefore(pWork->prev, key, desc) && IsKeyBefore(key, pWork->cur, desc)) {
            found = TRUE;
            *pId = id;
            pWork->cur = key;
            pWork->lastId = id;
        }
    }

    if (found) {
        pWork->prev = pWork->cur;
        return NWC24_OK;
    }

    pWork->valid = FALSE;
    return NWC24_ERR_DONE;
}

#pragma push
#pragma inline_max_auto_size(1000)
NWC24Err NWC24UpdateDlTask(NWC24DlTask* pTask) {
    NWC24iDlTask* pTaskImpl = (NWC24iDlTask*)pTask;
    NWC24Err result;

    result = NWC24CheckDlTask(pTask, TRUE);
    if (result != NWC24_OK) {
        return result;
    }

    if (pTaskImpl->id == 0xFFFF || pTaskImpl->id >= NWC24iGetCachedDlHeader()->maxTasks) {
        return NWC24_ERR_INVALID_VALUE;
    }

    result = UpdateDlLastAccess(pTaskImpl);
    if (result < 0) {
        return result;
    }

    ClearDlTaskError(pTaskImpl);

    if (pTaskImpl->type == 0xFF) {
        return NWC24_ERR_INVALID_VALUE;
    }

    if (pTaskImpl->subTaskType == NWC24_DL_STTYPE_INCREMENT) {
        while ((result = CheckSubTaskEnabled(pTaskImpl, pTaskImpl->subTaskCounter)) == NWC24_ERR_DISABLED) {
            pTaskImpl->subTaskCounter = (u32)(pTaskImpl->subTaskCounter + 1) % NWC24_DL_SUBTASK_MAX;
        }

        if (result < 0) {
            return result;
        }
    }

    return StoreDlTask(pTaskImpl);
}
#pragma pop

NWC24Err NWC24DeleteDlTaskForced(NWC24DlTask* pTask) {
    NWC24iDlTask* pTaskImpl;
    NWC24Err result;

    pTaskImpl = (NWC24iDlTask*)pTask;

    result = NWC24CheckDlTask(pTask, FALSE);
    if (result != NWC24_OK) {
        return result;
    }

    result = CheckDlEntryAvailable(pTaskImpl->id);
    if (result < 0) {
        return result;
    }
    result = DeleteDlTask(pTaskImpl);
    if (result < 0) {
        return result;
    }

    pTaskImpl->id = 0xFFFF;
    return result;
}

NWC24Err NWC24DeleteDlTask(NWC24DlTask* pTask) {
    NWC24Err result;

    if (NWC24GetAppId() != NWC24i_APP_ID_IPL) {
        result = NWC24CheckDlTask(pTask, TRUE);
        if (result != NWC24_OK) {
            return result;
        }
    }

    return NWC24DeleteDlTaskForced(pTask);
}

NWC24Err NWC24AddDlTask(NWC24DlTask* pTask) {
    NWC24iDlTask* pTaskImpl = (NWC24iDlTask*)pTask;
    NWC24Err result;

    result = CheckHeader(NWC24iGetCachedDlHeader());
    if (result < 0) {
        return result;
    }

    return AddTaskInternal(pTaskImpl, NWC24iGetCachedDlHeader()->privateTasks, NWC24iGetCachedDlHeader()->maxTasks);
}

static NWC24Err InitDlIterateWork(NWC24DlIterateWork* pWork, u32 key, BOOL desc) {
    memset(pWork, 0, sizeof(NWC24DlIterateWork));

    pWork->cur = desc ? 0x80000001 : 0x7FFFFFFF;
    pWork->prev = desc ? 0x7FFFFFFF : 0x80000001;
    pWork->mode = key | (desc ? 0x80000000 : 0);
    pWork->lastId = -1;
    pWork->first = TRUE;
    pWork->valid = TRUE;
    return NWC24_OK;
}

NWC24Err NWC24PurgeOldestDlTask(void) {
    NWC24DlIterateWork iter;
    NWC24DlTask task;
    NWC24Err result;
    u16 id;

    result = InitDlIterateWork(&iter, 0, FALSE);
    if (result < 0) {
        return result;
    }

    while ((result = NWC24IterateDlTaskEx(&iter, &id)) == NWC24_OK) {
        if (!IsPrivateId(id)) {
            break;
        }
    }

    if (result >= 0) {
        result = NWC24GetDlTask(&task, id);
        if (result < 0) {
            return result;
        }

        result = NWC24DeleteDlTaskForced(&task);
        if (result < 0) {
            return result;
        }
    } else if (result == NWC24_ERR_DONE) {
        result = NWC24_ERR_FAILED;
    }

    return result;
}

NWC24Err NWC24GetDlError(const NWC24DlTask* pTask, u32* pLastError, s32* pErrorCount) {
    const NWC24iDlTask* pTaskImpl = (const NWC24iDlTask*)pTask;
    NWC24Err result;

    result = NWC24CheckDlTask(pTask, FALSE);
    if (result != NWC24_OK) {
        return result;
    }

    if (pLastError == NULL && pErrorCount == NULL) {
        return NWC24_ERR_INVALID_VALUE;
    }

    if (pLastError != NULL) {
        *pLastError = pTaskImpl->lastError;
    }

    if (pErrorCount != NULL) {
        *pErrorCount = pTaskImpl->errorCount;
    }

    return NWC24_OK;
}

NWC24Err NWC24iGetDlTitleDir(const NWC24DlTask* pTask, char* pBuf, u32 size) {
    const NWC24iDlTask* pTaskImpl = (const NWC24iDlTask*)pTask;
    NWC24Err result;

    result = NWC24CheckDlTask(pTask, FALSE);
    if (result != NWC24_OK) {
        return result;
    }

    if (pTaskImpl->appId == 0) {
        return NWC24_ERR_INVALID_VALUE;
    }

    if (size < 30) {
        return NWC24_ERR_NOMEM;
    }

    snprintf(pBuf, size, "/title/%08x/%08x/data", pTaskImpl->titleIdHi, pTaskImpl->titleIdLo);
    return NWC24_OK;
}

NWC24Err NWC24GetDlVfPathByTask(const NWC24DlTask* pTask, char* pBuf, u32 size) {
    const NWC24iDlTask* pTaskImpl = (const NWC24iDlTask*)pTask;
    const char* pFileName = "wc24dl.vff";
    NWC24Err result;
    u32 len;

    result = NWC24CheckDlTask(pTask, FALSE);
    if (result != NWC24_OK) {
        return result;
    }

    if (pTaskImpl->appId == 0) {
        return NWC24_ERR_INVALID_VALUE;
    }

    result = NWC24iGetDlTitleDir(pTask, pBuf, size);
    if (result < 0) {
        return result;
    }

    len = strlen(pBuf);
    snprintf(pBuf + len, size - len, "/%s", pFileName);
    return NWC24_OK;
}

NWC24Err NWC24GetDlVfPath(char* pBuf, u32 size) {
    const char* pFileName = "wc24dl.vff";

    if (size < 0x2B) {
        return NWC24_ERR_NOMEM;
    }

    if (NANDGetHomeDir(pBuf) != 0) {
        return -0x15;
    }

    strcat(pBuf, "/");
    strncat(pBuf, pFileName, 12);
    return NWC24_OK;
}

NWC24Err NWC24CreateDlVf(const NWC24DlTask* pTask, u32 size) {
    static char path[128];
    NWC24Err result;

    result = NWC24CheckDlTask(pTask, TRUE);
    if (result != NWC24_OK) {
        return result;
    }

    if (size < 0x2800) {
        return NWC24_ERR_INVALID_VALUE;
    }

    NWC24GetDlVfPathByTask(pTask, path, sizeof(path));
    return NWC24CreateVF(path, size);
}

static NWC24Err GetDlKeyPath(char* pPath) {
    const char* pFileName = "wc24pubk.mod";

    if (NANDGetHomeDir(pPath) != 0) {
        return -0x15;
    }

    strcat(pPath, "/");
    strncat(pPath, pFileName, 12);
    return NWC24_OK;
}

NWC24Err NWC24SetDlKeys(const void* pKey, const void* pIv) {
    char path[NAND_MAX_PATH];
    NWC24Err result;
    s32 nandResult;

    result = GetDlKeyPath(path);
    if (result < 0) {
        return result;
    }

    nandResult = NANDCreate(path, 0x3D, 0);
    if (nandResult != 0 && nandResult != -6) {
        return -0x15;
    }

    if (pKey != NULL) {
        result = SeekAndWriteFile(path, 0, pKey, 0x100, 0);
    }

    if (pIv != NULL) {
        result = SeekAndWriteFile(path, 0x100, NULL, 0, 0x100);
        if (result >= 0) {
            result = SeekAndWriteFile(path, 0x200, pIv, 0x10, 0);
        }
    }

    return result;
}

NWC24Err NWC24ClearDlKeys(void) {
    char path[NAND_MAX_PATH];
    NWC24Err result;
    s32 nandResult;

    result = GetDlKeyPath(path);
    if (result < 0) {
        return result;
    }

    nandResult = NANDDelete(path);
    if (nandResult != 0 && nandResult != -6) {
        return -0x15;
    }

    return NWC24_OK;
}

static BOOL IsGroupWritable(u16 groupId, u32 flags) {
    return (flags & NWC24_DL_FLAG_GROUP_WRITABLE) && groupId == NWC24GetGroupId();
}

static BOOL IsMyApp(u32 appId) {
    return (appId & 0xFFFFFF00) == (NWC24GetAppId() & 0xFFFFFF00);
}

static BOOL IsPrivateId(u16 id) {
    return id < NWC24iGetCachedDlHeader()->privateTasks;
}

NWC24Err NWC24iOpenDlTaskList(void) {
    NWC24Err result;

    result = NWC24iLoadDlHeader();

    if (result >= 0) {
        NWC24iSynchronizeRtcCounter(FALSE);
        result = NWC24iCheckDlHeaderConsistency(NWC24iGetCachedDlHeader(), FALSE);

        if (result >= 0) {
            return NWC24_OK;
        }
    }

    return result;
}

NWC24Err NWC24iCloseDlTaskList(void) {
    return NWC24_OK;
}

NWC24iDlHeader* NWC24iGetCachedDlHeader(void) {
    if (NWC24WorkP != 0)
        return (NWC24iDlHeader*)NWC24WorkP->dlHead;
    else
        return 0;
}

NWC24Err NWC24iCheckDlHeaderConsistency(NWC24iDlHeader* pHeader, BOOL clear) {
    NWC24iDlTask* pTaskImpl;
    NWC24DlTask task;
    u16 i;

    pTaskImpl = (NWC24iDlTask*)&task;

    for (i = 0; i < pHeader->maxTasks; i++) {
        if (CheckDlEntryAvailable(i) == NWC24_OK && clear) {
            if (NWC24GetDlTask(&task, i) < 0) {
                NWC24DeleteDlTaskForced(&task);
            } else if (!IsPrivateId(i) && pTaskImpl->count == 0) {
                NWC24DeleteDlTaskForced(&task);
            }
        }
    }

    return NWC24_OK;
}

NWC24Err NWC24iLoadDlHeader(void) {
    NWC24File file;
    NWC24Err result;
    NWC24Err close;
    NWC24Err ret;
    u32 length;

    length = 0;

    result = NWC24FOpen(&file, DLFilePath, NWC24_OPEN_NAND_R);
    if (result < 0) {
        return result;
    }

    result = ReadDlHeader(&file);
    if (result < 0) {
        return result;
    }

    result = NWC24FGetLength(&file, &length);
    if (result >= 0) {
        result = CheckHeader(NWC24iGetCachedDlHeader());
    }

    close = NWC24FClose(&file);

    if (result != NWC24_OK) {
        ret = result;
    } else {
        ret = close;
    }

    return ret;
}

static NWC24iDlEntry* GetDlTaskEntryHeader(u16 id) {
    return &NWC24iGetCachedDlHeader()->entries[id];
}

static NWC24Err WriteDlHeader(NWC24File* pFile) {
    NWC24Err result;

    result = NWC24FSeek(pFile, 0, NWC24_SEEK_BEG);
    if (result < 0) {
        return result;
    }

    result = NWC24FWrite(NWC24iGetCachedDlHeader(), sizeof(NWC24iDlHeader), pFile);
    if (result < 0) {
        return result;
    }

    return NWC24_OK;
}

static NWC24Err SeekDlTaskEntry(u16 id, NWC24File* pFile) {
    u16 maxTasks = NWC24iGetCachedDlHeader()->maxTasks;
    if (maxTasks > NWC24_DL_TASK_MAX || id >= maxTasks) {
        return NWC24_ERR_INVALID_VALUE;
    }
    return NWC24FSeek(pFile, sizeof(NWC24iDlHeader) + id * sizeof(NWC24iDlTask), NWC24_SEEK_BEG);
}

static NWC24Err CheckDlEntryAvailable(u16 id) {
    if (id >= NWC24iGetCachedDlHeader()->maxTasks || id == 0xFFFF) {
        return NWC24_ERR_INVALID_VALUE;
    }

    if (GetDlTaskEntryHeader(id)->app == 0) {
        return NWC24_ERR_NOT_FOUND;
    }

    return NWC24_OK;
}

static NWC24Err WriteDlTaskEntry(NWC24iDlTask* pTask, NWC24File* pFile) {
    NWC24Err result;

    result = SeekDlTaskEntry(pTask->id, pFile);
    if (result < 0) {
        return result;
    }

    memcpy(NWC24WorkP->dlTask, pTask, sizeof(NWC24iDlTask));
    result = NWC24FWrite(NWC24WorkP->dlTask, sizeof(NWC24iDlTask), pFile);
    if (result < 0) {
        return result;
    }

    return NWC24_OK;
}

static NWC24Err ReadDlTaskEntry(NWC24iDlTask* pTask, u16 id, NWC24File* pFile) {
    NWC24Err result;

    result = SeekDlTaskEntry(id, pFile);
    if (result < 0) {
        return result;
    }

    result = NWC24FRead(pTask, sizeof(NWC24iDlTask), pFile);
    if (result < 0) {
        return result;
    }

    return NWC24_OK;
}

static NWC24Err ClearDlTaskEntry(u16 id, NWC24File* pFile) {
    NWC24iDlTask* pTask = (NWC24iDlTask*)NWC24WorkP->dlTask;

    memset(pTask, 0, sizeof(NWC24iDlTask));
    pTask->type = 0xFF;
    pTask->id = id;

    InitTaskEntryHeader(id);
    return WriteDlTaskEntry(pTask, pFile);
}

static NWC24Err ReadDlHeader(NWC24File* pFile) {
    NWC24Err result;

    result = NWC24FSeek(pFile, 0, NWC24_SEEK_BEG);
    if (result < 0) {
        return result;
    }

    result = NWC24FRead(&NWC24WorkP->dlHead, sizeof(NWC24iDlHeader), pFile);
    if (result < 0) {
        return result;
    }

    return NWC24_OK;
}

static void InitTaskEntryHeader(u16 id) {
    NWC24iDlHeader* pHeader = (NWC24iDlHeader*)NWC24WorkP->dlHead;
    memset(&pHeader->entries[id], 0, sizeof(NWC24iDlEntry));
}

static NWC24Err LoadDlTask(NWC24iDlTask* pTask, u16 id) {
    NWC24File file;
    NWC24Err result;
    NWC24Err close;
    NWC24Err ret;

    result = NWC24FOpen(&file, DLFilePath, NWC24_OPEN_NAND_RBUFF);
    if (result < 0) {
        return result;
    }

    result = ReadDlTaskEntry(pTask, id, &file);
    close = NWC24FClose(&file);

    if (result != NWC24_OK) {
        ret = result;
    } else {
        ret = close;
    }

    return ret;
}

static NWC24Err SetDlEntryHeader(const NWC24iDlTask* pTask) {
    NWC24iDlEntry* pEntry = GetDlTaskEntryHeader(pTask->id);

    pEntry->app = pTask->appId;
    pEntry->flags = pTask->priority;
    return NWC24_OK;
}

static NWC24Err StoreDlTask(NWC24iDlTask* pTask) {
    NWC24File file;
    NWC24Err result;
    NWC24Err close;
    NWC24Err ret;

    result = NWC24FOpen(&file, DLFilePath, NWC24_OPEN_NAND_RW);
    if (result < 0) {
        return result;
    }

    result = WriteDlTaskEntry(pTask, &file);
    if (result >= 0) {
        result = SetDlEntryHeader(pTask);
        if (result >= 0) {
            result = WriteDlHeader(&file);
        }
    }

    close = NWC24FClose(&file);

    if (result != NWC24_OK) {
        ret = result;
    } else {
        ret = close;
    }

    return ret;
}

static NWC24Err CheckDlTaskParams(const NWC24iDlTask* pTask) {
    NWC24Err result;

    if (pTask->id >= NWC24iGetCachedDlHeader()->maxTasks && pTask->id != 0xFFFF) {
        return NWC24_ERR_INVALID_VALUE;
    }

    result = CheckDlUrl(pTask->url);
    if (result < 0) {
        return result;
    }

    return NWC24_OK;
}

static NWC24Err AssignDlTaskId(NWC24iDlTask* pTask, u32 minId, u32 maxId) {
    NWC24iDlHeader* pHeader = NWC24iGetCachedDlHeader();
    u16 id;

    if (pTask == NULL || minId > maxId || minId >= pHeader->maxTasks || maxId > pHeader->maxTasks) {
        return NWC24_ERR_INVALID_VALUE;
    }

    for (id = minId; id < maxId; id++) {
        if (GetDlTaskEntryHeader(id)->app == 0) {
            pTask->id = id;
            return NWC24_OK;
        }
    }

    return NWC24_ERR_FULL;
}

static NWC24Err UpdateNextTime(NWC24iDlTask* pTask) {
    s64 now = 0;
    NWC24Err result;

    result = NWC24iGetUniversalTime(&now);
    if (result >= 0) {
        result = SetDlNextTime(pTask, now + pTask->interval * 60);
    }
    return result;
}

static NWC24Err AddTaskInternal(NWC24iDlTask* pTask, u16 minId, u16 maxId) {
    NWC24Err result;

    result = NWC24CheckDlTask((const NWC24DlTask*)pTask, TRUE);
    if (result != NWC24_OK) {
        return result;
    }

    result = CheckDlTaskParams(pTask);
    if (result < 0) {
        return result;
    }

    while (TRUE) {
        if (pTask->id != 0xFFFF) {
            return NWC24UpdateDlTask((NWC24DlTask*)pTask);
        }

        result = AssignDlTaskId(pTask, minId, maxId);
        if (result >= 0) {
            result = UpdateNextTime(pTask);
        } else if (result == NWC24_ERR_FULL) {
            result = NWC24PurgeOldestDlTask();
            if (result < 0) {
                return result;
            }
        } else if (result < 0) {
            return result;
        }
    }
}

static NWC24Err DeleteDlTask(NWC24iDlTask* pTask) {
    NWC24File file;
    NWC24Err result;
    NWC24Err close;
    NWC24Err ret;

    result = NWC24FOpen(&file, DLFilePath, NWC24_OPEN_NAND_RW);
    if (result < 0) {
        return result;
    }

    result = ClearDlTaskEntry(pTask->id, &file);
    if (result >= 0) {
        InitTaskEntryHeader(pTask->id);
        result = WriteDlHeader(&file);
    }

    close = NWC24FClose(&file);

    if (result != NWC24_OK) {
        ret = result;
    } else {
        ret = close;
    }

    return ret;
}

static NWC24Err SeekAndWriteFile(const char* pPath, u32 offset, const void* pData, u32 size, u32 padSize) {
    NWC24File file;
    NWC24Err result;
    u32 i;
    u8 zero;

    result = NWC24FOpen(&file, pPath, NWC24_OPEN_NAND_WBUFF);
    if (result < 0) {
        return result;
    }

    result = NWC24FSeek(&file, offset, NWC24_SEEK_BEG);
    if (result < 0) {
        return result;
    }

    if (pData != NULL && size != 0) {
        result = NWC24FWrite(pData, size, &file);
    }

    for (i = 0; i < padSize; i++) {
        zero = 0;
        result = NWC24FWrite(&zero, 1, &file);
        if (result < 0) {
            break;
        }
    }

    {
        NWC24Err close = NWC24FClose(&file);
        if (result != NWC24_OK) {
            return result;
        }
        return close;
    }
}

static NWC24Err SearchDlTaskId(u16* pId, u32 appId, u32 userParam, BOOL matchParam) {
    NWC24iDlTask task;
    NWC24Err result;

    if (NWC24iGetCachedDlHeader() == NULL) {
        return NWC24_ERR_LIB_NOT_OPENED;
    }

    if (pId == NULL || appId == 0) {
        return NWC24_ERR_INVALID_VALUE;
    }

    for (result = NWC24IterateDlTask(pId, TRUE); result >= 0; result = NWC24IterateDlTask(pId, FALSE)) {
        if (GetDlTaskEntryHeader(*pId)->app == appId) {
            if (!matchParam || (NWC24GetDlTask((NWC24DlTask*)&task, *pId) >= 0 && task.userParam == userParam)) {
                return NWC24_OK;
            }
        }
    }

    return NWC24_ERR_NOT_FOUND;
}

static NWC24Err CheckHeader(NWC24iDlHeader* pHeader) {
    NWC24File file;
    if (pHeader->maxTasks == 0 && pHeader->maxSubTasks != 0) {
        pHeader->maxTasks = pHeader->maxSubTasks;
        if (pHeader->maxSubTasks > 32) {
            pHeader->maxSubTasks = 32;
        }
        if (NWC24FOpen(&file, DLFilePath, NWC24_OPEN_NAND_RW) >= 0) {
            WriteDlHeader(&file);
            NWC24FClose(&file);
        }
    }

    if (pHeader->maxTasks < 1 || pHeader->privateTasks < 1 || pHeader->maxTasks < pHeader->privateTasks) {
        return NWC24_ERR_BROKEN;
    }

    return NWC24_OK;
}

static s32 IterationPredicatorLastAccess(u16 id) {
    return GetDlTaskEntryHeader(id)->lastAccess;
}

static s32 IterationPredicatorNextTime(u16 id) {
    return GetDlTaskEntryHeader(id)->nextTime;
}

static s32 IterationPredicatorPriority(u16 id) {
    return GetDlTaskEntryHeader(id)->flags;
}
