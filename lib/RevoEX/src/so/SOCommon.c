#include <revolution/so.h>
#include <revolution/ncd.h>
#include <revolution/net/NETMem.h>
#include <revolution/os.h>
#include <revolution/nwc24/NWC24Err.h>

#include <private/ios.h>
#include <private/nwc24/NWC24Schedule.h>
#include <private/os/OSTime.h>

#include <string.h>

// Adapted from doldecomp/mkw (lib/rvl/so/soCommon.c, Dec 2007). This is the
// Jun 2007 build: SOInit is part of this file.

const char* __SO_VERSION = "<< RVL_SDK - SO \trelease build: Dec 10 2007 10:02:35 (0x4199_60831) >>";

static u8 soState = 0;
static SOSysWork soWork;
static s32 soError = 0;
static BOOL soRegistered = FALSE;
static BOOL soBufAddrCheck = TRUE;
static char NET_RM_SOCK[] = "/dev/net/ip/top";

static void SOiSetError(int result) {
    OSThread* cur = OSGetCurrentThread();
    if (cur) {
        cur->error = result;
    } else {
        soError = result;
    }
}

int SOInit(const SOLibraryConfig* config) {
    int result = SO_SUCCESS;
    int enabled = OSDisableInterrupts();

    if (!soRegistered) {
        OSRegisterVersion(__SO_VERSION);
        soRegistered = TRUE;
    }

    switch (soState) {
    case SO_INTERNAL_STATE_READY:
    case SO_INTERNAL_STATE_ACTIVE:
        result = SO_EALREADY;
        break;
    case SO_INTERNAL_STATE_TERMINATED:
    default:
        if (config == NULL || config->alloc == NULL || config->free == NULL) {
            result = SO_EINVAL;
            break;
        }

        NETMemSet(&soWork, 0, sizeof(SOSysWork));
        soWork.allocFunc = config->alloc;
        soWork.freeFunc = config->free;
        soWork.allocCount = 0;
        soWork.rmState = SO_INTERNAL_RM_STATE_CLOSED;
        soWork.rmFd = -1;
        soWork._unk10 = (u32)SOiAlloc(0x0B, 0x460);
        if (soWork._unk10 == 0) {
            result = SO_ENOMEM;
        }
        if (soWork._unk10 != 0) {
            soState = SO_INTERNAL_STATE_READY;
        }
        break;
    }

    SOiSetError(result);
    OSRestoreInterrupts(enabled);
    return result;
}

int SOFinish(void) {
    int result = 0;
    int enabled = OSDisableInterrupts();

    switch (soState) {
    case SO_INTERNAL_STATE_TERMINATED:
        result = SO_EALREADY;
        break;
    case SO_INTERNAL_STATE_READY:
        if (soWork.rmState > SO_INTERNAL_RM_STATE_CLOSED) {
            result = SO_EBUSY;
            break;
        } else if (soWork.allocCount > 1) {
            result = SO_EAGAIN;
            break;
        }
        soState = SO_INTERNAL_STATE_TERMINATED;
        if (soWork._unk10 != 0 && soWork.freeFunc) {
            soWork.allocCount--;
            soWork.freeFunc(0x0B, (void*)soWork._unk10, 0x460);
        }
        break;
    case SO_INTERNAL_STATE_ACTIVE:
        result = SO_EINPROGRESS;
        break;
    }

    SOiSetError(result);
    OSRestoreInterrupts(enabled);
    return result;
}

int SOStartup(void) {
    return SOiStartupCore(60000);
}

static inline int SOiConvertNWC24Error(s32 errNwc24, s32 exErr) {
    s32 result = SO_EINVAL;
    switch (errNwc24) {
    case NWC24_OK:
        result = SO_SUCCESS;
        break;
    case NWC24_ERR_MUTEX:
    case NWC24_ERR_NOT_FOUND:
        result = SO_ENOLINK;
        break;
    case NWC24_ERR_CONFIG_NETWORK:
    case NWC24_ERR_FAILED:
        result = exErr;
        break;
    case NWC24_ERR_FATAL:
        result = SO_EFATAL;
        break;
    case NWC24_ERR_INPROGRESS:
        result = SO_EINPROGRESS;
        break;
    }
    return result;
}

int SOiStartupCore(int timeout) {
    int result;
    int enabled;
    s32 exErr;
    s64 limitTime;
    s32 linkupRetryCount;

    limitTime = 0;
    if (timeout != 0) {
        limitTime = __OSGetSystemTime() + OSMillisecondsToTicks(timeout);
    }

    linkupRetryCount = 4;

begin_startup:
    result = SO_SUCCESS;

    enabled = OSDisableInterrupts();

    switch (soState) {
    case SO_INTERNAL_STATE_TERMINATED:
    default:
        result = SO_ENETRESET;
        break;
    case SO_INTERNAL_STATE_ACTIVE:
        result = SO_EALREADY;
        break;
    case SO_INTERNAL_STATE_READY:
        if (soWork.rmState > SO_INTERNAL_RM_STATE_CLOSED) {
            result = SO_EBUSY;
            break;
        } else if (!OSGetCurrentThread()) {
            result = SO_EFATAL;
            break;
        } else {
            int resultNCD;

            soWork.rmState = SO_INTERNAL_RM_STATE_WORKING;
            OSRestoreInterrupts(enabled);

            while (TRUE) {
                resultNCD = NCDGetLinkStatus();
                if (resultNCD != NCD_RESULT_INPROGRESS && resultNCD != NCD_LINKSTATUS_WORKING) {
                    break;
                }
                OSSleepTicks(OSMillisecondsToTicks((s64)100));
                if (limitTime != 0 && limitTime < __OSGetSystemTime()) {
                    if (resultNCD == NCD_LINKSTATUS_WORKING) {
                        result = SO_ERR_LINK_UP_TIMEOUT;
                    } else {
                        result = SO_EFATAL;
                    }
                    goto change_state;
                }
            }

            if (resultNCD < NCD_RESULT_SUCCESS) {
                result = SO_EFATAL;
                goto change_state;
            } else if (resultNCD == NCD_LINKSTATUS_NONE) {
                result = SO_ENOENT;
                goto change_state;
            }

            while (TRUE) {
                soWork.rmFd = IOS_Open(NET_RM_SOCK, 0);
                if (soWork.rmFd != -6) {
                    break;
                }
                OSSleepTicks(OSMillisecondsToTicks((s64)100));
                if (limitTime != 0 && limitTime < __OSGetSystemTime()) {
                    result = SO_EFATAL;
                    goto change_state;
                }
            }

            if (soWork.rmFd < 0) {
                result = SO_EFATAL;
                goto change_state;
            } else {
                s32 errNwc24;

                result = SO_SUCCESS;
                exErr = SO_SUCCESS;

                while (TRUE) {
                    errNwc24 = NWC24iStartupSocket(&exErr);
                    if (errNwc24 != NWC24_ERR_INPROGRESS) {
                        break;
                    }
                    OSSleepTicks(OSMillisecondsToTicks((s64)100));
                    if (limitTime != 0 && limitTime < __OSGetSystemTime()) {
                        result = SO_EFATAL;
                        break;
                    }
                }

                if (result == SO_SUCCESS) {
                    result = SOiConvertNWC24Error(errNwc24, exErr);
                }

                if (result != SO_SUCCESS) {
                    if (IOS_Close(soWork.rmFd) < 0) {
                        result = SO_EFATAL;
                        goto change_state;
                    } else {
                        soWork.rmFd = -1;
                    }
                }
            }

        change_state:
            enabled = OSDisableInterrupts();
            if (result == SO_SUCCESS) {
                soState = SO_INTERNAL_STATE_ACTIVE;
                soWork.rmState = SO_INTERNAL_RM_STATE_OPENED;
            } else {
                soState = SO_INTERNAL_STATE_READY;
                if (result != SO_EFATAL) {
                    soWork.rmState = SO_INTERNAL_RM_STATE_CLOSED;
                }
            }
        }
        break;
    }

    SOiSetError(result);
    OSRestoreInterrupts(enabled);

    if (result == SO_SUCCESS) {
        s64 dhcpTimeOutTicks;

        if (limitTime != 0) {
            dhcpTimeOutTicks = limitTime - __OSGetSystemTime();
        } else {
            dhcpTimeOutTicks = 0;
        }

        if (limitTime != 0 && dhcpTimeOutTicks <= 0) {
            result = SO_ETIMEDOUT;
        } else {
            result = SOiWaitForDHCPEx((int)(dhcpTimeOutTicks / OSMillisecondsToTicks(1)));
        }

        if (result != SO_SUCCESS) {
            SOCleanup();
        }
    }

    SOiSetError(result);

    if (result == -112) {
        linkupRetryCount--;
        if (linkupRetryCount >= 0) {
            goto begin_startup;
        }
    }

    return result;
}

int SOCleanup(void) {
    int result = SO_SUCCESS;
    int enabled = OSDisableInterrupts();
    s32 exErr = SO_SUCCESS;
    int errNwc24;

    switch (soState) {
    case SO_INTERNAL_STATE_TERMINATED:
    default:
        result = SO_ENETRESET;
        break;
    case SO_INTERNAL_STATE_READY:
        result = SO_EALREADY;
        break;
    case SO_INTERNAL_STATE_ACTIVE:
        if (soWork.rmState < SO_INTERNAL_RM_STATE_OPENED) {
            result = SO_EBUSY;
            break;
        } else if (!OSGetCurrentThread()) {
            result = SO_EFATAL;
            break;
        } else {
            soWork.rmState = SO_INTERNAL_RM_STATE_WORKING;
            OSRestoreInterrupts(enabled);

            errNwc24 = NWC24iCleanupSocket(&exErr);
            result = SOiConvertNWC24Error(errNwc24, exErr);
            if (result != SO_SUCCESS) {
            } else {
                if (IOS_Close(soWork.rmFd) < 0) {
                    result = SO_EFATAL;
                } else {
                    soWork.rmFd = -1;
                }
            }

            enabled = OSDisableInterrupts();
            if (result == SO_SUCCESS) {
                soState = SO_INTERNAL_STATE_READY;
                soWork.rmState = SO_INTERNAL_RM_STATE_CLOSED;
            } else {
                if (result != SO_EFATAL) {
                    soState = SO_INTERNAL_STATE_ACTIVE;
                    soWork.rmState = SO_INTERNAL_RM_STATE_OPENED;
                }
            }
        }
        break;
    }

    SOiSetError(result);
    OSRestoreInterrupts(enabled);
    return result;
}

SOSysWork* SOiGetSysWork(void) {
    return &soWork;
}

int SOiIsBufferAddrCheck(void) {
    return soBufAddrCheck;
}

int SOiIsInitialized(void) {
    int result = FALSE;
    int enabled = OSDisableInterrupts();
    switch (soState) {
    case SO_INTERNAL_STATE_READY:
    case SO_INTERNAL_STATE_ACTIVE:
        result = TRUE;
        break;
    }
    OSRestoreInterrupts(enabled);
    return result;
}

void* SOiAlloc(u32 name, s32 size) {
    if (size > 0 && soWork.allocFunc) {
        void* ptr = soWork.allocFunc(name, size);
        if (ptr) {
            soWork.allocCount++;
            if (SOiIsBufferAddrCheck() &&
                !(((u32)ptr & 0x1FFFFFFF) >= 0x10000000 && ((u32)ptr & 0x1FFFFFFF) < 0x18000000)) {
                SOiFree(name, ptr, size);
                ptr = NULL;
            }
        }
        return ptr;
    }
    return NULL;
}

void SOiFree(u32 name, void* ptr, s32 size) {
    if (ptr != NULL && soWork.freeFunc != NULL) {
        soWork.allocCount--;
        soWork.freeFunc(name, ptr, size);
    }
}

int SOiPrepare(const char* funcName, s32* pRmId) {
    int result = SO_SUCCESS;
    int enabled = OSDisableInterrupts();
#pragma unused(funcName)

    switch (soState) {
    case SO_INTERNAL_STATE_TERMINATED:
        result = SO_ENETRESET;
        break;
    case SO_INTERNAL_STATE_READY:
    default:
        result = SO_EINVAL;
        break;
    case SO_INTERNAL_STATE_ACTIVE:
        if (soWork.rmState < SO_INTERNAL_RM_STATE_OPENED) {
            result = SO_EBUSY;
            break;
        } else if (!OSGetCurrentThread()) {
            result = SO_EFATAL;
            break;
        }
        *pRmId = soWork.rmFd;
        break;
    }

    if (result != SO_SUCCESS) {
        SOiSetError(result);
    }
    OSRestoreInterrupts(enabled);
    return result;
}

int SOiConclude(const char* funcName, int result) {
    int enabled = OSDisableInterrupts();
#pragma unused(funcName)
    SOiSetError(result);
    OSRestoreInterrupts(enabled);
    return result;
}

int SOiPrepareTempRm(const char* funcName, s32* pRmId, int* pIsTempRm) {
    int result = SO_SUCCESS;
    int enabled = OSDisableInterrupts();
    int errNwc24;

    switch (soState) {
    case SO_INTERNAL_STATE_TERMINATED:
        result = SO_ENETRESET;
        break;
    case SO_INTERNAL_STATE_READY:
    default:
        if (soWork.rmState > SO_INTERNAL_RM_STATE_CLOSED) {
            result = SO_EBUSY;
            break;
        } else if (!OSGetCurrentThread()) {
            result = SO_EFATAL;
            break;
        }
        soWork.rmState = SO_INTERNAL_RM_STATE_WORKING;
        OSRestoreInterrupts(enabled);
        soWork.rmFd = IOS_Open(NET_RM_SOCK, 0);
        if (soWork.rmFd < 0) {
            enabled = OSDisableInterrupts();
            if (soWork.rmFd == -6) {
                result = SO_EINPROGRESS;
                soWork.rmState = SO_INTERNAL_RM_STATE_CLOSED;
            } else {
                result = SO_EFATAL;
            }
        } else {
            *pRmId = soWork.rmFd;
            if ((errNwc24 = NWC24iLockSocket()) == NWC24_OK) {
                s32 errNcd = NCDGetLinkStatus();
                switch (errNcd) {
                case 3:
                case 4:
                case 5:
                    result = (int)IOS_Ioctl(soWork.rmFd, 0x1F, NULL, 0, NULL, 0);
                    if (result == SO_SUCCESS) {
                        *pIsTempRm = TRUE;
                    } else {
                        result = SOiConcludeTempRm(funcName, result, TRUE);
                    }
                    break;
                case -8:
                    result = SOiConcludeTempRm(funcName, SO_EINPROGRESS, TRUE);
                    break;
                case -1:
                case -2:
                    result = SOiConcludeTempRm(funcName, SO_EFATAL, TRUE);
                    break;
                default:
                    result = SOiConcludeTempRm(funcName, SO_ENOLINK, TRUE);
                }
                enabled = OSDisableInterrupts();
            } else {
                switch (errNwc24) {
                case NWC24_ERR_INPROGRESS:
                    result = SO_EINPROGRESS;
                    break;
                case NWC24_ERR_FAILED:
                    result = SO_ENETRESET;
                    break;
                case NWC24_ERR_DONE:
                    result = SO_ENOLINK;
                    break;
                case NWC24_ERR_MUTEX:
                case NWC24_ERR_FATAL:
                default:
                    result = SO_EFATAL;
                    break;
                }
                if (IOS_Close(soWork.rmFd) < 0) {
                    result = SO_EFATAL;
                }
                enabled = OSDisableInterrupts();
                if (result != SO_EFATAL) {
                    soWork.rmFd = -1;
                    soWork.rmState = SO_INTERNAL_RM_STATE_CLOSED;
                }
            }
        }
        break;
    case SO_INTERNAL_STATE_ACTIVE:
        if (soWork.rmState < SO_INTERNAL_RM_STATE_OPENED) {
            result = SO_EBUSY;
            break;
        } else if (!OSGetCurrentThread()) {
            result = SO_EFATAL;
            break;
        }
        *pIsTempRm = FALSE;
        *pRmId = soWork.rmFd;
        break;
    }

    if (result != SO_SUCCESS) {
        SOiSetError(result);
    }
    OSRestoreInterrupts(enabled);
    return result;
}

int SOiConcludeTempRm(const char* funcName, int result, int isTempRm) {
    int enabled;
#pragma unused(funcName)

    if (isTempRm == TRUE) {
        switch (NWC24iUnlockSocket()) {
        case NWC24_OK:
            break;
        case NWC24_ERR_INPROGRESS:
            result = SO_EINPROGRESS;
            break;
        case NWC24_ERR_FATAL:
        default:
            result = SO_EFATAL;
        }
        if (IOS_Close(soWork.rmFd) < 0) {
            result = SO_EFATAL;
        }
        enabled = OSDisableInterrupts();
        if (result != SO_EFATAL) {
            soWork.rmFd = -1;
            soWork.rmState = SO_INTERNAL_RM_STATE_CLOSED;
        }
    } else {
        enabled = OSDisableInterrupts();
    }

    SOiSetError(result);
    OSRestoreInterrupts(enabled);
    return result;
}

int SOiWaitForDHCPEx(int timeout) {
    int result = SO_SUCCESS;
    int gioResult;
    int ifError;
    int ifErrorSize;
    s64 limitTime;

    limitTime = 0;
    if (timeout != 0) {
        limitTime = __OSGetSystemTime() + OSMillisecondsToTicks(timeout);
    }

    while (TRUE) {
        OSSleepTicks(OSMillisecondsToTicks((s64)10));

        ifErrorSize = sizeof(ifError);
        gioResult = SOGetInterfaceOpt(NULL, SO_SOL_CONFIG, SO_CONFIG_ERROR, &ifError, &ifErrorSize);
        if (gioResult != SO_SUCCESS) {
            result = gioResult;
            break;
        }
        if (gioResult == SO_SUCCESS && ifError != SO_SUCCESS) {
            result = ifError;
            break;
        }
        if (SOGetHostID() != 0) {
            break;
        }
        if (limitTime != 0 && limitTime < __OSGetSystemTime()) {
            result = SO_ETIMEDOUT;
            break;
        }
    }

    return result;
}
