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

const u8 mpEmptyMacAddress[6] = {0};

s32 MPDisconnectAll(void) {
    MPWork *work;
    const u8 *mac;
    u32 aid;
    u32 ownAid;
    if (!mpInitialized) {
        return -236;
    }
    if (mpFatalError) {
        return -1;
    }
    work = MPi_Work;
    ownAid = work->aid;
    mac = work->childMacAddress[0];
    for (aid = 0; aid < 16; ++aid, mac += 6) {
        if (aid != ownAid && !MPiIsEqualMacAddress(mac, mpEmptyMacAddress)) {
            s32 result = MPiDisconnect(aid, FALSE, FALSE);
            if (result != -255 && result < 0) {
                return result;
            }
        }
    }
    return 0;
}

u32 MPGetConnectedAIDs(void) {
    if (mpInitialized != 0) {
        return *(u32 *)(mpWork + 0xc7e4);
    }
    return 0;
}

u32 MPiStartupConnection(void) {
    MPWork *work = MPi_Work;
    BOOL enabled;
    work->connectedBitmap = 0;
    memset(work->childMacAddress, 0, sizeof(work->childMacAddress));
    memset(MPi_Work->disconnectReason, 0, sizeof(MPi_Work->disconnectReason));
    memset(MPi_Work->rssi, 0, sizeof(MPi_Work->rssi));
    memset(MPi_Work->rssiIndex, 0, sizeof(MPi_Work->rssiIndex));
    memset(MPi_Work->lifeTime, 0, sizeof(MPi_Work->lifeTime));
    switch (MPi_Work->config.mode) {
    case 1: {
        enabled = OSDisableInterrupts();
        work = MPi_Work;
        work->aid = 0;
        memcpy(work->childMacAddress[0], work->macAddress, 6);
        MPi_Work->disconnectReason[0] = 0;
        OSRestoreInterrupts(enabled);
        break;
    }
    default:
        return -255;
    }
    return 0;
}

void MPiCleanupConnection(void) {
    int iVar1;
    iVar1 = mpWork;
    if (mpWork != 0) {
        *(u32 *)(mpWork + 0xc7e0) = 0;
        *(u32 *)(iVar1 + 0xc7e4) = 0;
        memset((void *)((void *)(iVar1 + 0xc7f2)), (int)(0), (u32)(0x60));
        memset((void *)((void *)(iVar1 + 0xda40)), (int)(0), (u32)(0x20));
        memset((void *)((void *)(iVar1 + 0xd5c0)), (int)(0), (u32)(0x400));
        memset((void *)((void *)(iVar1 + 0xd9c0)), (int)(0), (u32)(0x40));
        memset((void *)((void *)(iVar1 + 0xda00)), (int)(0), (u32)(0x40));
    }
    return;
}

int MPiDisconnect(u32 aid, int expired, int force) {
    u16 reason;
    BOOL enabled;
    MPWork *work;
    const u8 *mac;
    u8 address[6];
    s32 result;
    if (aid >= 16) {
        return -255;
    }
    reason = 0xf002;
    if (expired) {
        reason = 0x8001;
    }
    enabled = OSDisableInterrupts();
    work = MPi_Work;
    mac = work->childMacAddress[aid];
    memcpy(address, mac, 6);
    work->disconnectReason[aid] = reason;
    OSRestoreInterrupts(enabled);
    if (MPiIsEqualMacAddress(mac, mpEmptyMacAddress)) {
        return -255;
    }
    result = MPiDisAssoc(address);
    if (result == -255) {
        force = TRUE;
    }
    if (force) {
        MPiProcessDisconnectedNotification(aid, reason, (u32)address);
    }
    return result;
}

void MPiProcessConnectedNotification(u32 aid, void *address, u32 gameInfo,
                                     u32 size) {
    BOOL accept = FALSE;
    BOOL enabled;
    MPWork *work;
    if (aid >= 16) {
        return;
    }
    if (size > 32) {
        return;
    }
    if (MPi_Work->config.entryFlag) {
        accept = TRUE;
    }
    enabled = OSDisableInterrupts();
    memcpy(MPi_Work->childMacAddress[aid], address, 6);
    work = MPi_Work;
    work->disconnectReason[aid] = 0;
    memset(work->rssi[aid], 0, sizeof(work->rssi[aid]));
    work = MPi_Work;
    work->rssiIndex[aid] = 0;
    {
        OSTick now = OSGetTick();
        work = MPi_Work;
        work->lifeTime[(s32)aid] = now;
        if (accept) {
            work->connectedBitmap |= 1 << aid;
        } else {
            work->connectedBitmap &= ~(1 << aid);
        }
    }
    MPiResetPortByAid(aid);
    OSRestoreInterrupts(enabled);
    if (!accept) {
        MPiDisAssoc(address);
    } else {
        MPiNotifyConnectedToPort(aid, address, (void *)gameInfo, size);
    }
}

void MPiProcessDisconnectedNotification(u32 aid, u32 reason, u32 address) {
    u32 *bitmap;
    MPWork *current;
    BOOL connected;
    BOOL enabled;
    MPWork *work;
    if (aid >= 16) {
        return;
    }
    enabled = OSDisableInterrupts();
    MPiResetPortByAid(aid);
    work = MPi_Work;
    bitmap = &work->connectedBitmap;
    connected = ((1 << aid) & *bitmap) != 0;
    if (connected &&
        MPiIsEqualMacAddress(work->childMacAddress[aid], (const u8 *)address) &&
        work->disconnectReason[aid]) {
        reason = work->disconnectReason[aid];
    }
    {
        current = MPi_Work;
        memset(current->childMacAddress[aid], 0, 6);
        current->disconnectReason[aid] = 0;
        *bitmap &= ~(1 << aid);
        memset(current->rssi[aid], 0, sizeof(current->rssi[aid]));
        current->rssiIndex[aid] = 0;
        current->lifeTime[aid] = 0;
    }
    OSRestoreInterrupts(enabled);
    if (connected) {
        MPiNotifyDisconnectedToPort(aid, reason, (void *)address);
    }
}

void MPiAddRssi(int aid, u32 rssi) {
    MPWork *work = (MPWork *)mpWork;
    u32 index = work->rssiIndex[aid];
    work->rssi[aid][index++] = rssi;
    if (index >= 16) {
        index = 0;
    }
    work->rssiIndex[aid] = index;
}

void MPiUpdateLifeTime(int aid) {
    u32 tick = OSGetTick();
    MPWork *work = (MPWork *)mpWork;
    work->lifeTime[aid] = tick;
}

u32 MPiCompareLifeTime(int aid, int tick) {
    MPWork *work = (MPWork *)mpWork;
    return (u32)(work->lifeTime[aid] - tick) >> 31;
}

void MPiProcessLifeTimeExpiration(u32 param_1) {
    MPiDisconnect((u32)(param_1), (int)(1), (int)(0));
    return;
}

void MPiProcessForcibleLifeTimeExpiration(u32 param_1) {
    MPiDisconnect((u32)(param_1), (int)(1), (int)(1));
    return;
}
