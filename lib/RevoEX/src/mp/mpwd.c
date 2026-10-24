#include <private/mp.h>
#include <private/wd.h>
#include <revolution/wpad.h>
#include <string.h>

u32 mpReceiveBuffer[40];

static inline s32 SetParentConfig(void) {
    WDMpConfig *config = (WDMpConfig *)MPi_Work->parentParameters;
    MPWork *work;
    s32 result;
    memset(config, 0, sizeof(WDMpConfig));
    config->type = 4;
    work = MPi_Work;
    config->interval = work->config.beaconPeriod;
    config->options = work->config.options;
    config->reservedB2 = 0;
    config->reserved138 = 0;
    config->reservedB1 = 0;
    config->channel = work->config.channel;
    result = WD_SetConfig(config, 0x30000, 0x3007f, 0);
    if (result < 0) {
        return result;
    }
    return 0;
}

s32 MPiStartupWD(void) {
    s32 result;
    OSTime deadline;
    switch ((s32)MPi_Work->config.mode) {
    case 1:
        break;
    default:
        return -255;
    }
    deadline = OSGetTime() + OSMillisecondsToTicks(30000);
    do {
        result = WD_Startup(1);
        if (result != -6) {
            goto opened;
        }
        OSSleepMilliseconds(10);
    } while (OSGetTime() <= deadline);
    return -235;
opened:
    if (result < 0) {
        return result;
    }

    if (WD_GetInfo((WD_Info *)mpReceiveBuffer) >= 0) {
        memcpy(MPi_Work->macAddress, ((WD_Info *)mpReceiveBuffer)->MAC, 6);
        MPi_Work->allowedChannels =
            ((WD_Info *)mpReceiveBuffer)->NTRallowedChannel;
    }
    result = StartParent();
    if (result < 0) {
        WD_Cleanup();
        return result;
    }
    return 0;
}

void MPiCleanupWD(void) {
    if (MPi_Work) {
        MPWork *work = MPi_Work;
        s32 count = 0;
        s32 delay;
        u32 aid;
        u32 ownAid;
        work->config.options = 0;
        SetParentConfig();
        if (!MPSetEntryFlag(FALSE)) {
            MPiMakeAndChangeBeacon();
        }
        work = MPi_Work;
        ownAid = work->aid;
        for (aid = 0; aid < 16; ++aid) {
            if (aid != ownAid &&
                !MPiIsEqualMacAddress(work->childMacAddress[aid],
                                      mpEmptyMacAddress)) {
                ++count;
            }
        }
        delay = MPi_Work->config.beaconPeriod * count;
        if (delay < 600) {
            delay = 600;
        }
        if (delay > 4000) {
            delay = 4000;
        }
        MPDisconnectAll();
        OSSleepTicks(OSMillisecondsToTicks((OSTime)delay));
    }
    if (WD_GetLinkState() == 1 && WD_SetLinkState(0) >= 0) {
        OSTime deadline = OSGetTime() + OSMillisecondsToTicks(30000);
        do {
            if (WD_GetLinkState() != 1) {
                break;
            }
            OSSleepMilliseconds(10);
        } while (OSGetTime() <= deadline);
    }
    WD_Cleanup();
}

s32 MPiSendFrame(u16 *frame, u32 size, void *result) {
    s32 status;
    size = (u16)size;
    __sthbrx(*frame, frame, 0);
    status = WD_MpSendFrame(frame, (u16)size, result);
    return status < 0 ? status : 0;
}

s32 MPiReceiveFrame(void *frame, u16 size) {
    return WD_ReceiveFrame(frame, size);
}

s32 MPiChangeBeacon(void *beacon) {
    s32 result = WD_ChangeBeacon((u16)MPiGetVTsf(), beacon);
    return result < 0 ? result : 0;
}

s32 MPiChangeVTSF(void) {
    s32 result = WD_ChangeVTSF((u16)MPiGetVTsf());
    return result < 0 ? result : 0;
}

s32 MPiDisAssoc(const u8 *mac) {
    s32 result = WD_DisAssoc(mac);
    return result == (s32)0x80008001 ? -255 : result;
}

s32 StartParent(void) {
    MPWork *work = MPi_Work;
    s32 result;
    OSTime deadline;
    if (!work->allowedChannels ||
        (work->config.channel &&
         !(work->allowedChannels & (1 << work->config.channel)))) {
        return -238;
    }
    result = SetParentConfig();
    if (result < 0) {
        return result;
    }
    result = MPiMakeAndChangeBeacon();
    if (result < 0) {
        return result;
    }
    result = WD_SetLinkState(1);
    if (result < 0) {
        return result;
    }
    deadline = OSGetTime() + OSMillisecondsToTicks(30000);
    while (WD_GetLinkState() == 0) {
        OSSleepMilliseconds(10);
        if (OSGetTime() > deadline) {
            return -235;
        }
    }
    DCInvalidateRange((void *)0x800031a0, 0x20);
    if (*(volatile u8 *)0x800031a2) {
        WPADStartSimpleSync();
    }
    return 0;
}
