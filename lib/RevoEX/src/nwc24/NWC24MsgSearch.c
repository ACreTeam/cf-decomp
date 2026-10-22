#include <private/nwc24.h>
#include <revolution/nwc24.h>
#include <revolution/os.h>

#define SEARCH_BY_MENU (1 << 0)
#define SEARCH_BY_PUBLIC (1 << 1)
#define SEARCH_BY_FROM (1 << 4)
#define SEARCH_BY_APP_ID (1 << 5)
#define SEARCH_BY_TAG (1 << 8)

typedef struct NWC24iSearchConds {
    u32 initialized;
    u32 pos;
    u32 numFound;
    u32 numRemaining;
    u32 flags;
    NWC24MBoxType msgBox;
    NWC24UserId from;
    u32 appId;
    u32 tagCond;
} NWC24iSearchConds;

static NWC24iSearchConds sConds;
static NWC24iMBCEntry sEntry;

static BOOL NWC24iSearchMatching(const NWC24iSearchConds* pConds, NWC24iMBCEntry* pEntry);
static NWC24Err NWC24iSearchMain(NWC24iSearchConds* pConds, u32* pIds);

NWC24Err NWC24InitSearchConds(void) {
    if (!NWC24IsMsgLibOpened() && !NWC24IsMsgLibOpenedByTool()) {
        return NWC24_ERR_LIB_NOT_OPENED;
    }

    sConds.initialized = 0;
    sConds.pos = 0;
    sConds.numFound = 0;
    sConds.numRemaining = 0;
    sConds.flags = 0;
    sConds.msgBox = NWC24_MBOX_TYPE_RECV;
    sConds.from = 0;
    sConds.appId = 0;
    sConds.tagCond = 0;
    return NWC24_OK;
}

NWC24Err NWC24SetSearchCondMsgBox(NWC24MBoxType id) {
    if (!NWC24IsMsgLibOpened() && !NWC24IsMsgLibOpenedByTool()) {
        return NWC24_ERR_LIB_NOT_OPENED;
    }

    if (id != NWC24_MBOX_TYPE_SEND && id != NWC24_MBOX_TYPE_RECV) {
        return NWC24_ERR_INVALID_VALUE;
    }

    sConds.msgBox = id;
    return NWC24_OK;
}

NWC24Err NWC24SearchMsgs(NWC24MsgObj* pMsgs, u32 max, u32* pNumRead, u32* pNumRemaining) {
    NWC24Err result;
    u32 i;
    u32* pIds;
    NWC24iSearchConds* pConds = &sConds;

    if (!NWC24IsMsgLibOpened() && !NWC24IsMsgLibOpenedByTool()) {
        return NWC24_ERR_LIB_NOT_OPENED;
    }

    *pNumRead = 0;
    *pNumRemaining = 0;
    pIds = (u32*)((char*)NWC24WorkP + 0x1a00);

    if (pMsgs == NULL && max != 0) {
        return NWC24_ERR_NULL;
    }

    if (!pConds->initialized) {
        Mail_memset(pIds, 0, 0x1000);
        result = NWC24iSearchMain(pConds, pIds);
        if (result != NWC24_OK) {
            return result;
        }
        pConds->initialized = 1;
    }

    if (pConds->numRemaining == 0) {
        return NWC24_OK;
    }

    for (i = 0; i < pConds->numRemaining; pMsgs++, i++) {
        if (i >= max) {
            break;
        }

        result = NWC24GetMsgObj(pMsgs, pConds->msgBox, pIds[pConds->pos++]);
        if (result != NWC24_OK) {
            return result;
        }

        (*pNumRead)++;
    }

    pConds->numRemaining -= *pNumRead;
    *pNumRemaining = pConds->numRemaining;
    return NWC24_OK;
}

static NWC24Err NWC24iSearchMain(NWC24iSearchConds* pConds, u32* pIds) {
    NWC24Err result;
    NWC24File file;
    u32 numMsgs;
    u32 numSeen;
    u32 offset;
    NWC24iMBCHeader* pCtrl;
    char* pPath;

    numSeen = 0;
    pConds->numFound = 0;
    pConds->numRemaining = 0;

    result = NWC24GetNumMsgs(pConds->msgBox, &numMsgs);
    if (result != NWC24_OK) {
        return result;
    }

    if (numMsgs == 0) {
        return NWC24_OK;
    }

    if (pConds->msgBox == NWC24_MBOX_TYPE_SEND) {
        pCtrl = &NWC24WorkP->sendCtrl;
    } else if (pConds->msgBox == NWC24_MBOX_TYPE_RECV) {
        pCtrl = &NWC24WorkP->recvCtrl;
    }

    pPath = NWC24WorkP->pathWork;
    result = NWC24iMBoxGetCtrlFilePath(pConds->msgBox, pPath, NWC24i_PATH_WORK_SIZE);
    if (result != NWC24_OK) {
        return result;
    }

    result = NWC24FOpen(&file, pPath, NWC24_OPEN_READ);
    if (result != NWC24_OK) {
        return result;
    }

    result = NWC24_ERR_BROKEN;
    for (offset = sizeof(NWC24iMBCEntry); offset < pCtrl->fileSize; offset += sizeof(NWC24iMBCEntry)) {
        NWC24FSeek(&file, offset, NWC24_SEEK_BEG);
        result = NWC24FRead(&sEntry, sizeof(NWC24iMBCEntry), &file);
        if (result != NWC24_OK) {
            break;
        }

        if (sEntry.id != 0 && sEntry.id <= NWC24i_MSG_ID_MAX) {
            if (NWC24iSearchMatching(pConds, &sEntry)) {
                pIds[pConds->numFound++] = sEntry.id;
            }

            numSeen++;
            if (numSeen == numMsgs) {
                result = NWC24_OK;
                break;
            }
        }
    }

    NWC24FClose(&file);
    if (result != NWC24_OK) {
    }
    pConds->numRemaining = pConds->numFound;
    return NWC24_OK;
}

static BOOL NWC24iSearchMatching(const NWC24iSearchConds* pConds, NWC24iMBCEntry* pEntry) {
    u32 flags;

    if (!NWC24iIsMsgObjReadable(pEntry)) {
        return FALSE;
    }

    flags = pConds->flags;
    if ((flags & SEARCH_BY_MENU) && !(pEntry->flags & NWC24_MSGOBJ_FOR_MENU)) {
        return FALSE;
    }

    if (!(flags & SEARCH_BY_PUBLIC) && (pEntry->flags & NWC24_MSGOBJ_FOR_PUBLIC)) {
        return FALSE;
    }

    if ((flags & SEARCH_BY_APP_ID) && pConds->appId != pEntry->appId) {
        return FALSE;
    }

    if (flags & SEARCH_BY_FROM) {
        if (!(pEntry->flags & NWC24_MSGOBJ_FOR_RECIPIENT)) {
            return FALSE;
        }

        if (pConds->from != pEntry->from.id) {
            return FALSE;
        }
    }

    if (flags & SEARCH_BY_TAG) {
        u32 op = pConds->tagCond >> 16;
        u32 tag = pEntry->tag & 0xFFFF;
        u32 val = pConds->tagCond & 0xFFFF;
        BOOL ok;

        switch (op) {
        case 0:
            ok = tag != val;
            break;
        case 1:
            ok = tag == val;
            break;
        case 2:
            ok = val >= tag;
            break;
        case 3:
            ok = val <= tag;
            break;
        case 4:
            ok = (tag & val) != 0;
            break;
        default:
            ok = FALSE;
            break;
        }

        if (!ok) {
            return FALSE;
        }
    }

    return TRUE;
}
