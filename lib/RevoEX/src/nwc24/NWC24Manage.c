#include <private/nwc24.h>
#include <revolution/nwc24.h>
#include <revolution/ncd.h>
#include <revolution/os.h>
#include <revolution/sc.h>
#include <revolution/vf.h>

typedef enum { NWC24_LIB_CLOSED, NWC24_LIB_OPENED, NWC24_LIB_OPENED_BY_TOOL, NWC24_LIB_BLOCKED } NWC24LibState;

typedef enum { NWC24_FAIL_SFL = 1 << 0, NWC24_FAIL_DL_TASK = 1 << 1, NWC24_FAIL_FATAL = 1 << 2 } NWC24FailFlag;

const char* __NWC24VersionSecretFList = "<< RVL_SDK - NWC24 	release build: Aug  6 2008 15:31:46 (0x4199_60831) UNOFFICIAL >>";
const char* __NWC24Version = "<< RVL_SDK - NWC24 	release build: Aug  6 2008 15:31:46 (0x4199_60831) >>";

NWC24Work* NWC24WorkP = NULL;

static NWC24LibState Opened = NWC24_LIB_CLOSED;
static u32 YouGotMail = 0;
static u32 GlobalErrorCode = 0;
static BOOL Registered = FALSE;
static BOOL RegisteredSecretFList = FALSE;

static NWC24Err NWC24OpenLibInternal(NWC24Work* pWork, NWC24LibState state);
static NWC24Err AnalyzeScdErrors(s32* pErrorCode, u32 usage);
static NWC24Err AnalyzeErrorCode(s32 errorCode, u32 usage, u32* pScore);
static void ReadSavedErrorCode(s32* pErrorCode) NO_INLINE;

void NWC24iRegisterSecretFList(void) {
    BOOL enabled = OSDisableInterrupts();

    if (!RegisteredSecretFList) {
        OSRegisterVersion(__NWC24VersionSecretFList);
        RegisteredSecretFList = TRUE;
    }

    OSRestoreInterrupts(enabled);
}

void NWC24iRegister(void) {
    if (Registered) {
        return;
    }

    OSRegisterVersion(__NWC24Version);
    Registered = TRUE;
}

NWC24Err NWC24OpenLib(void* pWork) {
    NWC24Work* pWorkImpl = (NWC24Work*)pWork;

    if (Opened == NWC24_LIB_OPENED_BY_TOOL) {
        return NWC24_ERR_BUSY;
    }

    return NWC24OpenLibInternal(pWorkImpl, NWC24_LIB_OPENED);
}

static NWC24Err NWC24OpenLibInternal(NWC24Work* pWork, NWC24LibState state) {
    NWC24Err result;
    NWC24Err failErr;
    u32 failFlag;

    NWC24iSetErrorCode(NWC24_OK);

    if (!VFIsAvailable()) {
        return NWC24_ERR_FATAL;
    }

    if (NWC24IsMsgLibOpened()) {
        return NWC24_ERR_LIB_OPENED;
    }

    if (NWC24IsMsgLibOpenBlocking()) {
        return NWC24_ERR_BUSY;
    }

    if (pWork == NULL) {
        return NWC24_ERR_NULL;
    }

    if ((u32)pWork % 32 != 0) {
        return NWC24_ERR_ALIGNMENT;
    }

    result = NWC24iTrySuspendForOpenLib();
    if (result == NWC24_OK) {
        NWC24iRegister();

        YouGotMail &= ~NWC24_MSG_ARRIVED;
        NWC24WorkP = pWork;

        NWC24InitBase64Table(NWC24WorkP->base64Work);

        failFlag = 0;
        failErr = NWC24_OK;

        result = NWC24iConfigOpen();
        if (result != NWC24_OK) {
            failErr = result;
            failFlag |= NWC24_FAIL_FATAL;
        }

        result = NWC24iOpenMBox();
        if (result != NWC24_OK) {
            failErr = result;
            failFlag |= NWC24_FAIL_FATAL;
        }

        result = NWC24iOpenFriendList();
        if (result != NWC24_OK) {
            failErr = result;
            failFlag |= NWC24_FAIL_FATAL;
        }

        result = NWC24iOpenSecretFriendList();
        if (result != NWC24_OK) {
            failErr = result;

            if (result != NWC24_ERR_FILE_NOEXISTS) {
                failFlag |= NWC24_FAIL_FATAL;
            } else {
                failFlag |= NWC24_FAIL_SFL;
            }
        }

        result = NWC24iOpenDlTaskList();
        if (result < 0) {
            failErr = result;

            if (result != NWC24_ERR_FILE_NOEXISTS) {
                failFlag |= NWC24_FAIL_FATAL;
            } else {
                failFlag |= NWC24_FAIL_DL_TASK;
            }
        }

        if (failFlag == (NWC24_FAIL_SFL | NWC24_FAIL_DL_TASK)) {
            failErr = NWC24_ERR_OLD_SYSTEM;
        }

        if (failFlag != 0) {
            NWC24WorkP = NULL;
            NWC24iResumeForCloseLib();
            result = failErr;
        } else {
            Opened = state;
            return NWC24_OK;
        }
    }

    switch (result) {
    case NWC24_ERR_OLD_SYSTEM:
    case NWC24_ERR_FILE_BROKEN:
    case NWC24_ERR_INTERNAL_VF:
    case NWC24_ERR_INTERNAL_IPC:
    case NWC24_ERR_NAND_CORRUPT:
    case NWC24_ERR_INPROGRESS:
    case NWC24_ERR_BUSY:
    case NWC24_ERR_MUTEX:
    case NWC24_ERR_FILE_OTHER:
    case NWC24_ERR_FILE_NOEXISTS:
    case NWC24_ERR_FILE_WRITE:
    case NWC24_ERR_FILE_READ:
    case NWC24_ERR_FILE_CLOSE:
    case NWC24_ERR_FILE_OPEN:
    case NWC24_ERR_BROKEN:
    case NWC24_ERR_FATAL: {
        NWC24iSetErrorCode(NWC24iMakeCode(NWC24_ERRCODE_1090XX, result));
        break;
    }

    default: {
        break;
    }
    }

    return result;
}

NWC24Err NWC24CloseLib(void) {
    s32 result;

    if (Opened != NWC24_LIB_OPENED) {
        return NWC24_ERR_LIB_NOT_OPENED;
    }

    result = NWC24iConfigFlush();
    if (result != NWC24_OK) {
        return result;
    }

    result = NWC24iCloseDlTaskList();
    if (result < 0) {
        return result;
    }

    result = NWC24iResumeForCloseLib();
    if (result != NWC24_OK) {
        return result;
    }

    NWC24WorkP = NULL;
    Opened = NWC24_LIB_CLOSED;
    return result;
}

BOOL NWC24IsMsgLibOpened(void) {
    return Opened == NWC24_LIB_OPENED;
}

BOOL NWC24IsMsgLibOpenedByTool(void) {
    return Opened == NWC24_LIB_OPENED_BY_TOOL;
}

BOOL NWC24IsMsgLibOpenBlocking(void) {
    return Opened == NWC24_LIB_BLOCKED;
}

NWC24Err NWC24BlockOpenMsgLib(BOOL block) {
    s32 result;
    BOOL enabled;

    result = NWC24_OK;
    enabled = OSDisableInterrupts();

    if (block) {
        if (Opened == NWC24_LIB_CLOSED) {
            Opened = NWC24_LIB_BLOCKED;
        } else if (Opened == NWC24_LIB_OPENED) {
            result = NWC24_ERR_LIB_OPENED;
        } else {
            result = NWC24_ERR_BUSY;
        }
    } else {
        if (Opened == NWC24_LIB_BLOCKED) {
            Opened = NWC24_LIB_CLOSED;
        } else {
            result = NWC24_ERR_LIB_NOT_OPENED;
        }
    }

    OSRestoreInterrupts(enabled);
    return result;
}

NWC24Err NWC24iSetNewMsgArrived(u32 flags) {
    YouGotMail |= flags;
    return NWC24_OK;
}

s32 NWC24GetErrorCode(void) {
    return GlobalErrorCode;
}

void NWC24iSetErrorCode(s32 code) {
    GlobalErrorCode = code;
}

NWC24Err NWC24Check(u32 usage) {
    NWC24Err result;
    NWC24Err err;
    SCParentalControlsInfo pcInfo;
    u32 wiredList;
    u32 wirelessList;
    u32 aossList;
    NWC24IdCreationStage stage;
    s32 errorCode;
    u32 status;
    u32 wcFlags;
    BOOL eula;
    u32 restrictions;

    GlobalErrorCode = 0;

    if (Opened != NWC24_LIB_OPENED && Opened != NWC24_LIB_OPENED_BY_TOOL) {
        result = NWC24_ERR_LIB_NOT_OPENED;
    } else {
        result = NWC24_ERR_DISABLED;

        do {
            do {
                status = SCCheckStatus();
            } while (status == SC_STATUS_BUSY);

            if (status == SC_STATUS_FATAL) {
                GlobalErrorCode = -109112;
                break;
            }

            wcFlags = SCGetWCFlags() & SC_WC_FLAGS_ENABLED;
            eula = SCGetEULA();
            restrictions = SCGetNetContentRestrictions() & SC_NET_RESTRICTIONS_MSG_BOARD;
            SCGetParentalControl(&pcInfo);

            if (!wcFlags) {
                GlobalErrorCode = -109139;
                break;
            }

            if (!eula) {
                GlobalErrorCode = -109107;
                break;
            }

            if ((pcInfo.enable & SC_PARENTAL_FLAG_ENABLED) && restrictions && !(usage & 2)) {
                GlobalErrorCode = -109107;
                result = NWC24_ERR_PROTECTED;
                break;
            }

            result = NWC24_ERR_NETWORK;
            if (NCDiGetEnabledConfigList(&wiredList, &wirelessList, &aossList) < 0) {
                GlobalErrorCode = -109133;
                break;
            }

            if (wiredList == 0 && wirelessList == 0 && aossList == 0) {
                GlobalErrorCode = -50299;
                break;
            }

            if (usage & 1) {
                NWC24GetIdCreationStage(&stage);
                if (stage != NWC24_IDCS_REGISTERED) {
                    GlobalErrorCode = -109144;
                    break;
                }
            }

            errorCode = 0;
            err = AnalyzeScdErrors(&errorCode, usage);
            if (err < 0) {
                result = err;
                GlobalErrorCode = errorCode;
                break;
            }

            if (usage & 1) {
                result = NWC24iMBoxCheck(NWC24_MBOX_TYPE_SEND, 0);
                if (result == NWC24_ERR_FULL) {
                    GlobalErrorCode = -109106;
                    break;
                }
            }

            result = NWC24_OK;
        } while (0);
    }

    return result;
}

static NWC24Err AnalyzeScdErrors(s32* pErrorCode, u32 usage) {
    NWC24ScdStat* stat;
    NWC24Err result;
    u32 total;
    u32 count;
    u32 i;
    s32 idx;
    u32 score;
    s32 savedError;

    stat = (NWC24ScdStat*)NWC24WorkP->mainWork;
    total = 0;
    *pErrorCode = 0;

    if (usage & 4) {
        return NWC24_OK;
    }

    result = NWC24iGetSchedulerStat(stat, sizeof(NWC24ScdStat));
    if (result == NWC24_ERR_FATAL || result == NWC24_ERR_NOMEM || result == NWC24_ERR_INTERNAL_IPC) {
        *pErrorCode = result - 109100;
        return NWC24_ERR_FATAL;
    }

    if (result != NWC24_OK) {
        return NWC24_OK;
    }

    if (usage & 1) {
        if ((stat->mailTaskTrace & 3) == 3) {
            return NWC24_OK;
        }
    } else if (usage & 2) {
        if ((stat->dlTaskTrace & 1) == 1) {
            return NWC24_OK;
        }
    }

    count = stat->numErrors < NWC24_SCD_ERROR_LOG_MAX ? stat->numErrors : NWC24_SCD_ERROR_LOG_MAX;
    idx = (s32)(stat->numErrors + NWC24_SCD_ERROR_LOG_MAX - 1) % NWC24_SCD_ERROR_LOG_MAX;

    for (i = 0; i < count; i++) {
        result = AnalyzeErrorCode(stat->errorLog[idx], usage, &score);
        if (result != NWC24_OK) {
            total += score;
            if (total >= 60) {
                *pErrorCode = stat->errorLog[idx];
                return result;
            }
        }

        if (--idx < 0) {
            idx = NWC24_SCD_ERROR_LOG_MAX - 1;
        }
    }

    if (count == 0) {
        ReadSavedErrorCode(&savedError);
        if (savedError < 0) {
            result = AnalyzeErrorCode(savedError, usage, &score);
            *pErrorCode = savedError;
            return result;
        }
    }

    *pErrorCode = 0;
    return NWC24_OK;
}

static NWC24Err AnalyzeErrorCode(s32 errorCode, u32 usage, u32* pScore) {
    char digit[6];
    int i;
    s32 tmp = -errorCode;

    *pScore = 0;

    for (i = 0; i < 6; i++) {
        digit[i] = tmp % 10;
        tmp = tmp / 10;
    }

    // 5XXXX: NCD error
    if (digit[5] == 0 && digit[4] == 5) {
        *pScore = 60;
        return NWC24_ERR_NETWORK;
    }

    if (digit[3] == 7) {
        if ((usage & 2) == 0) {
            return NWC24_OK;
        }
    } else {
        if ((usage & 1) == 0) {
            return NWC24_OK;
        }
    }

    // 11XXXX: CGI error
    if (digit[5] == 1 && digit[4] == 1) {
        if (digit[2] == 1) {
            return NWC24_OK;
        }

        if (digit[2] == 4 && digit[1] == 0 && digit[0] == 7) {
            *pScore = 60;
            return NWC24_ERR_NETWORK;
        }

        if ((usage & 1) == 0) {
            return NWC24_OK;
        }

        *pScore = 30;
        return NWC24_ERR_SERVER;
    }

    // 10XXXX: NWC24 library error
    if (digit[5] == 1 && digit[4] == 0) {
        if ((usage & 1) == 0) {
            return NWC24_OK;
        }

        tmp = digit[0] + (digit[1] * 10);
        switch (digit[2]) {
        case 0: {
            if (tmp >= 70) {
                return NWC24_OK;
            }
            *pScore = 60;
            switch (tmp) {
            case 0:
            case 1:
            case 2:
            case 3:
                return NWC24_ERR_FATAL;
            case 30:
            case 31:
                return NWC24_ERR_NETWORK;
            case 32:
                return NWC24_ERR_SERVER;
            default:
                return NWC24_OK;
            }
        }
        case 2: {
            switch (-tmp) {
            case -1:
                *pScore = 60;
                return NWC24_ERR_FATAL;
            case -11:
            case -45:
                return NWC24_OK;
            case -31:
            case -44:
            case -33:
                *pScore = 60;
                return NWC24_ERR_NETWORK;
            case -32:
                *pScore = 20;
                return NWC24_ERR_SERVER;
            default:
                return NWC24_OK;
            }
        }
        case 3: {
            switch (tmp) {
            case 1:
            case 2:
            case 3:
            case 4:
            case 9:
            case 10:
            case 11:
            case 12:
                *pScore = 30;
                return NWC24_ERR_NETWORK;
            case 5:
            case 13:
                *pScore = 30;
                return NWC24_ERR_SERVER;
            case 6:
            case 7:
            case 8:
                *pScore = 20;
                return NWC24_ERR_SERVER;
            default:
                return NWC24_OK;
            }
        }
        case 4: {
            switch (tmp) {
            case 9:
            case 10:
            case 11:
            case 12:
                *pScore = 60;
                return NWC24_ERR_SERVER;
            default:
                *pScore = 20;
                return NWC24_ERR_NETWORK;
            }
        }
        }
    }

    return NWC24_OK;
}

static void ReadSavedErrorCode(s32* pErrorCode) {
    NWC24File file;
    s32* pWork;

    *pErrorCode = 0;
    pWork = (s32*)NWC24WorkP;

    if (NWC24FOpen(&file, "/shared2/wc24/misc.bin", 2) >= 0) {
        if (NWC24FSeek(&file, 0x40, NWC24_SEEK_BEG) >= 0) {
            if (NWC24FRead(pWork, 0x20, &file) >= 0) {
                if ((*pErrorCode = pWork[0]) >= 0) {
                    *pErrorCode = pWork[1];
                }
            }
        }

        NWC24FClose(&file);
    }
}
