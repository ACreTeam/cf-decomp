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

s32 MPSetUserGameInfo(const void *data, u32 size) {
    BOOL enabled;
    MPWork *work;
    if (!mpInitialized) {
        return -236;
    }
    if (mpFatalError) {
        return -1;
    }
    if (size > 0x70) {
        return -255;
    }
    if (data == NULL) {
        return -255;
    }
    enabled = OSDisableInterrupts();
    work = MPi_Work;
    work->config.gameInfoSize = size;
    memcpy(work->config.gameInfo, data, size);
    if (size < 0x70) {
        memset(MPi_Work->config.gameInfo + size, 0, 0x70 - size);
    }
    OSRestoreInterrupts(enabled);
    return 0;
}

s32 MPSetEntryFlag(int flag) {
    if (!mpInitialized) {
        return -236;
    }
    if (mpFatalError) {
        return -1;
    }
    {
        int value;
        switch (flag) {
        default:
            value = 1;
            break;
        case 0:
            value = 0;
            break;
        }
        MPi_Work->config.entryFlag = value;
        return 0;
    }
}

u32 MPiSetMultiBootFlag(int flag) {
    if (!mpInitialized) {
        return -236;
    }
    if (mpFatalError) {
        return -1;
    }
    {
        int value;
        switch (flag) {
        default:
            value = 1;
            break;
        case 0:
            value = 0;
            break;
        }
        MPi_Work->config.multiBootFlag = value;
        return 0;
    }
}
