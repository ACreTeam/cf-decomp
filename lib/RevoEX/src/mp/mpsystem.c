#include <private/mp.h>
#include <private/mpdl.h>
#include <private/wdp.h>
#include <private/wd.h>
#include <revolution/ncd.h>
#include <revolution/nwc24.h>
#include <revolution/net.h>
#include <revolution/vi.h>
#include <revolution/wpad.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

u32 mpFatalCommand[8];
const char *__MPVersion =
    "<< RVL_SDK - MP \trelease build: Aug  6 2008 15:31:26 (0x4199_60831) >>";
s32 mpWirelessLock = -1;
BOOL mpCheckMem2 = TRUE;
BOOL mpInitialized;
s32 mpFatalError;
BOOL mpVersionRegistered;
u32 mpWork;

static inline s32 LockWirelessDriver(void) {
    s32 result = NWC24SuspendScheduler();
    if (result < 0) {
        return -1;
    }
    if (mpWirelessLock >= 0) {
        return -250;
    }
    result = NCDLockWirelessDriver();
    if (result < 0 && result != -2) {
        return -250;
    }
    mpWirelessLock = result;
    return 0;
}

s32 MPStartup(void *config) {
    BOOL enabled;
    s32 result;
    s32 laterResult;
    result = LockWirelessDriver();
    if (result < 0) {
        return result;
    }
    enabled = OSDisableInterrupts();
    if (mpInitialized == TRUE) {
        OSRestoreInterrupts(enabled);
        MPiUnlockWirelessDriver();
        return -236;
    }
    mpInitialized = FALSE;
    if (!mpVersionRegistered) {
        mpVersionRegistered = TRUE;
        OSRegisterVersion(__MPVersion);
    }
    do {
        result = MPiStartupSystem(config);
        if (result < 0) {
            break;
        }
        result = MPiStartupConnection();
        if (result < 0) {
            break;
        }
        result = MPiStartupTxRx();
        if (result < 0) {
            break;
        }
        if (MPi_Work->config.mode == 1) {
            result = MPiStartupParent();
            if (result < 0) {
                break;
            }
        }
        result = MPiStartupNotification();
        if (result < 0) {
            break;
        }
        result = MPiStartupBeacon();
        if (result < 0) {
            break;
        }
        result = MPiStartupPort();
        if (result < 0) {
            break;
        }
        mpInitialized = TRUE;
    } while (FALSE);
    OSRestoreInterrupts(enabled);
    if (result < 0) {
        MPCleanup();
        return result;
    }
    do {
        laterResult = MPiStartupTxRxLater();
        if (laterResult < 0) {
            break;
        }
        laterResult = MPiStartupPortLater();
        if (laterResult < 0) {
            break;
        }
        laterResult = MPiStartupWD();
        if (laterResult < 0) {
            break;
        }
        if (MPi_Work->config.mode == 1) {
            laterResult = MPiStartupParentLater();
            if (laterResult < 0) {
                break;
            }
        }
        laterResult = MPiStartupNotificationLater();
    } while (FALSE);
    if (laterResult < 0) {
        MPCleanup();
        return laterResult;
    }
    return 0;
}

s32 MPCleanup(void) {
    BOOL enabled;
    MPiCleanupWD();
    MPiUnlockWirelessDriver();
    MPiCleanupNotificationEarlier();
    MPiCleanupParentEarlier();
    MPiCleanupTxRxEarlier();
    enabled = OSDisableInterrupts();
    mpInitialized = FALSE;
    MPiCleanupPort();
    MPiCleanupBeacon();
    MPiCleanupNotification();
    if (MPi_Work && MPi_Work->config.mode == 1) {
        MPiCleanupParent();
    }
    MPiCleanupTxRx();
    MPiCleanupConnection();
    if (MPi_Work) {
        void (*release)(void *);
        if (MPi_Work->callbackThread.state) {
            OSSendMessage(&MPi_Work->callbackQueue, NULL, OS_MESSAGE_BLOCK);
            OSJoinThread(&MPi_Work->callbackThread, NULL);
        }
        release = MPi_Work->config.free;
        memset(MPi_Work->commands, 0, sizeof(MPi_Work->commands));
        memset(MPi_Work, 0, sizeof(MPWork));
        release((u8 *)MPi_Work - ((u8 *)MPi_Work)[-1]);
    }
    mpWork = 0;
    OSRestoreInterrupts(enabled);
    return 0;
}

void *MPiCallbackThread(void *arg) {
    OSMessageQueue *callbackQueue;
    MPPortResult *result;
    OSMessageQueue *freeQueue;
    MPWork *work;
    s32 status;
    work = MPi_Work;
    callbackQueue = &work->callbackQueue;
    result = &work->callbackResult;
    freeQueue = &work->freeCommandQueue;
    for (;;) {
        MPCommand *command;
        MPCommand *received;
        u32 category;
        while (!OSReceiveMessage((OSMessageQueue *)(callbackQueue),
                                 (OSMessage *)&received,
                                 (s32)(OS_MESSAGE_BLOCK))) {
        }
        if (!(command = received) || !work) {
            break;
        }
        category = command->command & 0xffff0000;
        switch (category) {
        case 0:
            MPiHandlePortCallback((int *)((int *)command));
            break;
        case 0x10000: {
            status = command->result;
            if (command->synchronous == 1) {
                OSSignalSemaphore((OSSemaphore *)(&command->completion));
            } else {
                memset((void *)(result), (int)(0), (u32)(sizeof(MPPortResult)));
                result->header = command->header;
                switch (command->command) {
                case 0x10001:
                    result->aid = command->aidBitmap;
                    result->data.send.data = command->data;
                    result->data.send.size = command->size;
                    result->data.send.remainingBitmap =
                        command->remainingBitmap;
                    result->data.send.sequence = command->sequence;
                    break;
                case 0x10002:
                    break;
                }
                command->callback(status, result);
                OSSendMessage((OSMessageQueue *)(freeQueue),
                              (OSMessage)(command), (s32)(0));
            }
            break;
        }
        case 0x20000:
            if (work->config.callback) {
                work->config.callback(command->result, command);
            }
            break;
        }
    }
    return NULL;
}

u32 MPiUnlockWirelessDriver(void) {
    if (mpWirelessLock >= 0) {
        s32 id = mpWirelessLock;
        mpWirelessLock = -1;
        NCDUnlockWirelessDriver(id);
    }
    NWC24ResumeScheduler();
    return 0;
}

static inline BOOL IsFrameSizeValid(const MPConfig *config) {
    if (config->sendSize > 0x200) {
        return FALSE;
    }
    if (config->recvSize > 0x200) {
        return FALSE;
    }
    return TRUE;
}
static inline BOOL IsMemory2(const void *p) {
    u32 physical = (u32)p & 0x1fffffff;
    return physical >= 0x10000000 && physical < 0x18000000;
}
static inline void *AllocateAligned(const MPConfig *config, u32 size) {
    u8 *allocation = config->alloc(size + 0x40);
    u8 *aligned;
    if (allocation == NULL) {
        return NULL;
    }
    aligned = (u8 *)(((u32)allocation + 0x20) & ~31);
    aligned[-1] = aligned - allocation;
    return aligned;
}
s32 MPiStartupSystem(const MPConfig *config) {
    MPWork *work;
    s32 i;
    if (config->alloc == (void *)0 || config->free == (void *)0) {
        return -255;
    }
    if (!IsFrameSizeValid(config)) {
        return -255;
    }
    work = AllocateAligned(config, sizeof(MPWork));
    if (mpCheckMem2 && work) {
        if (!IsMemory2(work)) {
            config->free((u8 *)work - ((u8 *)work)[-1]);
            work = NULL;
        }
    }
    if (!work) {
        mpWork = 0;
        return -240;
    }
    mpWork = (u32)work;
    memset((void *)(work), (int)(0), (u32)(sizeof(MPWork)));
    memcpy((void *)(&work->config), (const void *)(config),
           (u32)(sizeof(MPConfig)));
    memset((void *)(work->commands), (int)(0), (u32)(sizeof(work->commands)));
    OSInitMessageQueue((OSMessageQueue *)(&work->freeCommandQueue),
                       (OSMessage *)(work->freeCommandMessages), (s32)(32));
    for (i = 0; i < 32; ++i) {
        OSSendMessage((OSMessageQueue *)(&work->freeCommandQueue),
                      (OSMessage)(&work->commands[i]), (s32)(0));
    }
    OSInitMessageQueue((OSMessageQueue *)(&work->callbackQueue),
                       (OSMessage *)(work->callbackMessages), (s32)(64));
    if (!OSCreateThread(
            (OSThread *)(&work->callbackThread),
            (void *(*)(void *))(MPiCallbackThread), (void *)(NULL),
            (void *)(work->callbackStack + sizeof(work->callbackStack)),
            (u32)(sizeof(work->callbackStack)),
            (OSPriority)(work->config.priority + 3), (u16)(0))) {
        return -239;
    }
    OSResumeThread((OSThread *)(&work->callbackThread));
    return 0;
}

s32 MPiAllocateCommandBlockFromQueue(OSMessageQueue *queue, MPCommand **out,
                                     s32 flags) {
    MPCommand *command;
    if (flags == 1) {
        OSReceiveMessage(queue, (OSMessage *)&command, OS_MESSAGE_BLOCK);
    } else if (!OSReceiveMessage(queue, (OSMessage *)&command, 0)) {
        *out = NULL;
        return -237;
    }
    *out = command;
    if (!command) {
        return -256;
    }
    return 0;
}

void MPiFreeCommandBlockToQueue(u32 param_1, u32 param_2) {
    OSSendMessage((OSMessageQueue *)(param_1), (OSMessage)(param_2), (s32)(0));
    return;
}

s32 MPiAllocateCommandBlock(MPCommand **out, s32 flags) {
    MPCommand *command;
    if (flags == 1) {
        OSReceiveMessage(&MPi_Work->freeCommandQueue, (OSMessage *)&command,
                         OS_MESSAGE_BLOCK);
    } else if (!OSReceiveMessage(&MPi_Work->freeCommandQueue,
                                 (OSMessage *)&command, 0)) {
        *out = NULL;
        return -237;
    }
    *out = command;
    return command == (void *)0 ? -256 : 0;
}

void MPiFreeCommandBlock(u32 param_1) {
    OSSendMessage((OSMessageQueue *)(mpWork + 0xcda0), (OSMessage)(param_1),
                  (s32)(0));
    return;
}

u32 MPiSendRequest(u32 param_1, int param_2, int param_3) {
    BOOL enable;
    int iVar1;
    if (param_3 == 1) {
        OSInitSemaphore((OSSemaphore *)(param_2 + 0x2c), (s32)(0));
        enable = OSDisableInterrupts();
        OSSendMessage((OSMessageQueue *)(param_1), (OSMessage)(param_2),
                      (s32)(1));
        OSWaitSemaphore((OSSemaphore *)(param_2 + 0x2c));
        OSRestoreInterrupts(enable);
    } else {
        iVar1 = OSSendMessage((OSMessageQueue *)(param_1), (OSMessage)(param_2),
                              (s32)(0));
        if (iVar1 == 0) {
            return 0xffffff13;
        }
    }
    return 0;
}

void MPiRespondToWaitingRequest(int param_1, int param_2) {
    if (param_2 == 1) {
        OSInitSemaphore((OSSemaphore *)(param_1 + 0x2c), (s32)(0));
        OSWaitSemaphore((OSSemaphore *)(param_1 + 0x2c));
    }
    return;
}

void MPiNotifyCallback(s32 result, MPCommand *command) {
    MPWork *work = (MPWork *)mpWork;
    s32 status;
    command->result = result;
    status = OSSendMessage(&work->callbackQueue, command, 0) ? 0 : -237;
    if (status) {
        OSSendMessage(&work->freeCommandQueue, command, 0);
    }
}

BOOL MPiIsEqualMacAddress(const u8 *a, const u8 *b) {
    int i;
    for (i = 0; i < 6; ++i) {
        if (a[i] != b[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

u32 MPiGetNewTgid(void) {
    return OSGetTime() & 0xffff;
}

u32 MPiGetNewBeaconPeriod(void) {
    u32 count = VIGetRetraceCount();
    OSTime time = OSGetTime();
    return ((time ^ count) & 0x1f) + 0xb8;
}

u32 MPiCountPopulation(u32 value) {
    value -= (value >> 1) & 0x55555555;
    value = (value & 0x33333333) + ((value >> 2) & 0x33333333);
    value += value >> 4;
    value &= 0x0f0f0f0f;
    value += value >> 8;
    value += value >> 16;
    return value & 0xff;
}

s32 MPiSendCallback(u32 message) {
    MPWork *work = (MPWork *)mpWork;
    return OSSendMessage(&work->callbackQueue, (OSMessage)message, 0) ? 0
                                                                      : -237;
}

void MPiSetFatalError(u32 param_1) {
    mpFatalError = param_1;
    if (mpInitialized != 0) {
        memset((void *)(&(*(u32 *)mpFatalCommand)), (int)(0), (u32)(0x10));
        (*(u32 *)((u8 *)mpFatalCommand + 0x8)) = param_1;
        if (mpWork != 0) {
            (*(u32 *)mpFatalCommand) = 0xffffffff;
            (*(u32 *)((u8 *)mpFatalCommand + 0x4)) = 0x2ff01;
            OSSendMessage((OSMessageQueue *)(mpWork + 0xcc80),
                          (OSMessage)(&(*(u32 *)mpFatalCommand)), (s32)(0));
        }
    }
    return;
}
