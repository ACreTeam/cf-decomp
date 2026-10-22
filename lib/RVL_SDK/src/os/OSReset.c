#include <decomp.h>
#include <string.h>
#include "private/dvd.h"
#include "private/hollywood.h"
#include "private/os.h"
#include "private/pad.h"
#include "private/vi.h"
#include "revolution/dvd.h"
#include "revolution/os.h"
#include "revolution/pad.h"
#include "revolution/sc.h"

static OSShutdownFunctionQueue ShutdownFunctionQueue;
static u32 bootThisDol = 0;
volatile BOOL __OSIsReturnToIdle = FALSE;

#define EnqueueTail(queue, info)                                                                                                                     \
    do {                                                                                                                                             \
        OSShutdownFunctionInfo* __prev;                                                                                                              \
                                                                                                                                                     \
        __prev = (queue)->tail;                                                                                                                      \
        if (__prev == NULL)                                                                                                                          \
            (queue)->head = (info);                                                                                                                  \
        else                                                                                                                                         \
            __prev->next = (info);                                                                                                                   \
        (info)->prev = __prev;                                                                                                                       \
        (info)->next = NULL;                                                                                                                         \
        (queue)->tail = (info);                                                                                                                      \
    } while (0)

#define EnqueuePrio(queue, info)                                                                                                                     \
    do {                                                                                                                                             \
        OSShutdownFunctionInfo* __prev;                                                                                                              \
        OSShutdownFunctionInfo* __next;                                                                                                              \
                                                                                                                                                     \
        for (__next = (queue)->head; __next && __next->priority <= info->priority; __next = __next->next)                                            \
            ;                                                                                                                                        \
        if (__next == NULL)                                                                                                                          \
            EnqueueTail(queue, info);                                                                                                                \
        else {                                                                                                                                       \
            (info)->next = __next;                                                                                                                   \
            __prev = __next->prev;                                                                                                                   \
            __next->prev = (info);                                                                                                                   \
            (info)->prev = __prev;                                                                                                                   \
            if (__prev == NULL)                                                                                                                      \
                (queue)->head = (info);                                                                                                              \
            else                                                                                                                                     \
                __prev->next = (info);                                                                                                               \
        }                                                                                                                                            \
    } while (0)

void OSRegisterShutdownFunction(OSShutdownFunctionInfo* info) {
    EnqueuePrio(&ShutdownFunctionQueue, info);
}

BOOL __OSCallShutdownFunctions(BOOL final, u32 event) {
    OSShutdownFunctionInfo* info;
    BOOL err;
    u32 priority = 0;

    err = FALSE;
    info = ShutdownFunctionQueue.head;
    while (info) {
        if (err && priority != info->priority) {
            break;
        }

        err |= !info->func(final, event);
        priority = info->priority;
        info = info->next;
    }
    err |= !__OSSyncSram();
    return err ? FALSE : TRUE;
}

static void KillThreads(void);

void __OSShutdownDevices(u32 event) {
    BOOL rc, disableRecalibration, doRecal;

    switch (event) {
    case 0:
    case 5:
    case 6:
        doRecal = FALSE;
        break;
    case 2:
    case 3:
    case 4:
    case 1:
    default:
        doRecal = TRUE;
        break;
    }

    __OSStopAudioSystem();

    if (!doRecal) {
        disableRecalibration = __PADDisableRecalibration(TRUE);
    }

    while (!__OSCallShutdownFunctions(FALSE, event))
        ;

    while (!__OSSyncSram())
        ;

    OSDisableInterrupts();
    rc = __OSCallShutdownFunctions(TRUE, event);
    LCDisable();

    if (!doRecal) {
        __PADDisableRecalibration(disableRecalibration);
    }

    KillThreads();
}

u8 __OSGetDiscState(u8 last) {
    u32 flags;

    if (__DVDGetCoverStatus() != 2) {
        return 3;
    } else {
        if ((last == 1) && (__OSGetRTCFlags(&flags) && !flags)) {
            return 1;
        } else {
            return 2;
        }
    }
}

void OSRebootSystem(void) {
    OSStateFlags state;

    __OSStopPlayRecord();
    __OSUnRegisterStateEvent();
    __DVDPrepareReset();
    __OSReadStateFlags(&state);
    state.discState = __OSGetDiscState(state.discState);
    state.shutdownType = 2;
    __OSClearRTCFlags();
    __OSWriteStateFlags(&state);
    OSDisableScheduler();
    __OSShutdownDevices(1);
    __OSHotResetForError();
}

void OSShutdownSystem(void) {
    SCIdleModeInfo idleModeInfo;
    OSIOSRev iosRev;
    OSStateFlags state;

    memset(&idleModeInfo, 0, sizeof(SCIdleModeInfo));

    SCInit();
    while (SCCheckStatus() == 1) {
    }

    SCGetIdleMode(&idleModeInfo);
    __OSStopPlayRecord();
    __OSUnRegisterStateEvent();
    __DVDPrepareReset();
    __OSReadStateFlags(&state);
    state.discState = __OSGetDiscState(state.discState);

    if (idleModeInfo.standby == 1) {
        state.shutdownType = 5;
    } else {
        state.shutdownType = 1;
    }

    __OSClearRTCFlags();
    __OSWriteStateFlags(&state);
    __OSGetIOSRev(&iosRev);

    if (idleModeInfo.standby == 1) {
        __OSIsReturnToIdle = TRUE;

        OSDisableScheduler();
        __OSShutdownDevices(5);
        OSEnableScheduler();
        __OSLaunchMenu();
    } else {
        OSDisableScheduler();
        __OSShutdownDevices(2);
        __OSShutdownToSBY();
    }
}

void OSRestart(u32 resetCode) {
    u8 type = OSGetAppType();
    __OSStopPlayRecord();
    __OSUnRegisterStateEvent();

    if (type == 0x81) {
        OSDisableScheduler();
        __OSShutdownDevices(4);
        OSEnableScheduler();
        __OSRelaunchTitle(resetCode);
    } else if (type == 0x80) {
        OSDisableScheduler();
        __OSShutdownDevices(4);
        OSEnableScheduler();
        __OSReboot(resetCode, bootThisDol);
    }

    OSDisableScheduler();
    __OSShutdownDevices(1);
    __OSHotResetForError();
}

static void KillThreads(void) {
    OSThread* thread;
    OSThread* next;

    for (thread = __OSActiveThreadQueue.head; thread; thread = next) {
        next = thread->linkActive.next;

        switch (thread->state) {
        case 1:
        case 4:
            OSCancelThread(thread);
            break;
        default:
            break;
        }
    }
}

void __OSReturnToMenu(u8 menuMode) {
    OSStateFlags state;

    __OSStopPlayRecord();
    __OSUnRegisterStateEvent();
    __DVDPrepareReset();
    __OSReadStateFlags(&state);
    state.discState = __OSGetDiscState(state.discState);
    state.shutdownType = 3;
    state.menuMode = menuMode;
    __OSClearRTCFlags();
    __OSWriteStateFlags(&state);
    OSDisableScheduler();
    __OSShutdownDevices(5);
    OSEnableScheduler();
    __OSLaunchMenu();
    OSDisableScheduler();
    __VISetRGBModeImm();
    __OSHotResetForError();
}

void OSReturnToMenu(void) {
    __OSReturnToMenu(0);
    OSPanic(__FILE__, 0x348, "OSReturnToMenu(): Falied to boot system menu.\n");
}

void OSReturnToDataManager(void) {
    __OSReturnToMenu(1);
    OSPanic(__FILE__, 858, "OSReturnToDataManager(): Falied to boot system menu.\n");
}

void OSReturnToSetting(u8 setting) {
    char* url;

    switch (setting) {
    case OS_SETTING_CALENDAR: {
        url = "Calendar/Calendar_index.html";
        break;
    }
    case OS_SETTING_DISPLAY: {
        url = "Display/Display_index.html";
        break;
    }
    case OS_SETTING_SOUND: {
        url = "Sound/Sound_index.html";
        break;
    }
    case OS_SETTING_PARENTAL_CONTROL: {
        url = "Parental_Control/Parental_Control_index.html";
        break;
    }
    case OS_SETTING_INTERNET: {
        url = "Internet/Internet_index.html";
        break;
    }
    case OS_SETTING_WIICONNECT24: {
        url = "WiiConnect24/Wiiconnect24_index.html";
        break;
    }
    case OS_SETTING_UPDATE: {
        url = "Update/Update_index.html";
        break;
    }
    default: {
        OSReport("OSReturnToSetting(): You can't specify %d.  \n", setting);
        OSPanic(__FILE__, 0x394, "");
        // NOT REACHED
    }
    }

    __OSReturnToMenul(1, url, NULL);
}

void __OSReturnToMenuForError(void) {
    OSStateFlags state;
    __OSReadStateFlags(&state);
    state.discState = 2;
    state.shutdownType = 3;
    __OSClearRTCFlags();
    __OSWriteStateFlags(&state);
    __OSLaunchMenu();
    OSDisableScheduler();
    __VISetRGBModeImm();
    __OSHotResetForError();
    OSPanic(__FILE__, 0x3BB, "__OSReturnToMenu(): Falied to boot system menu.\n");
}

void __OSHotResetForError(void) {
    if (__OSInNandBoot || __OSInReboot) {
        __OSInitSTM();
    }

    __OSHotReset();

    OSPanic(__FILE__, 0x3D3, "__OSHotReset(): Falied to reset system.\n");
}

u32 OSGetResetCode(void) {
    u32 code;

    if (__OSRebootParams.valid) {
        code = (0x80000000 | __OSRebootParams.restartCode);
    } else {
        code = (__PIRegs[9] & 0xFFFFFFF8) >> 3;
    }

    return code;
}

void OSResetSystem(int, u32, int) {
    OSPanic(__FILE__, 1130, "OSResetSystem() is obsoleted. It doesn't work any longer.\n");
}
DECOMP_FORCE_STRING(OSReset_c, "OSSetBootDol() is obsoleted. It doesn't work any longer.\n");

#if 0 // not linked in City Folk
u32 OSSetBootDol(u32 dolOffset) {
    OSPanic(__FILE__, __LINE__, "OSSetBootDol() is obsoleted. It doesn't work any longer.\n");
    return 0;
}
#endif
