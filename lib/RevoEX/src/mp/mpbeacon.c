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

s32 MPUpdateBeacon(void) {
    return MPiUpdateBeacon((u32)(0), (u32)(0), (int)(1));
}

s32 MPUpdateBeaconAsync(MPCallback callback, void *arg) {
    return MPiUpdateBeacon((u32)callback, (u32)arg, (int)(0));
}

u32 MPiStartupBeacon(void) {
    MPWork *work;
    u32 tgid;
    u32 period;
    OSInitMessageQueue(&MPi_Work->requestQueue, MPi_Work->requestMessages, 32);
    OSInitMutex(&MPi_Work->beaconMutex);
    memset(MPi_Work->beacon, 0, sizeof(MPi_Work->beacon));
    work = MPi_Work;
    work->tsfOffset = 0;
    memset(work->tsfSamples, 0, sizeof(work->tsfSamples));
    work = MPi_Work;
    work->tsfIndex = 0;
    work->tsfCount = 0;
    tgid = work->config.tgid;
    if (tgid == (u32)-1) {
        tgid = MPiGetNewTgid();
    }
    work = MPi_Work;
    work->config.tgid = tgid;
    period = work->config.beaconPeriod;
    if (!period) {
        period = MPiGetNewBeaconPeriod();
    }
    MPi_Work->config.beaconPeriod = period;
    return 0;
}

void MPiCleanupBeacon(void) {
    if (MPi_Work) {
        OSMessageQueue *queue = &MPi_Work->requestQueue;
        MPCommand *command;
        MPWork *work;
        while (OSReceiveMessage(queue, (OSMessage *)&command, 0)) {
            MPiNotifyCallback(-256, command);
        }
        work = MPi_Work;
        memset(MPi_Work->requestMessages, 0, sizeof(MPi_Work->requestMessages));
        memset(work->beacon, 0, sizeof(work->beacon));
        work->tsfOffset = 0;
        memset(work->tsfSamples, 0, sizeof(work->tsfSamples));
        work->tsfIndex = 0;
        work->tsfCount = 0;
    }
}

void MPiProcessBeaconSentNotification(void) {
    MPCommand *command;
    if (OSReceiveMessage(&MPi_Work->requestQueue, (OSMessage *)&command, 0)) {
        MPiNotifyCallback(0, command);
    }
}

void MPiMakeGameInfo(u16 *buffer) {
    u32 tgid;
    u32 size;
    u32 flags;
    MPWork *work = MPi_Work;
    size = work->config.gameInfoSize;
    tgid = work->config.tgid;
    if (size > 0x70) {
        size = 0x70;
    }
    flags = 0;
    if (MPi_Work->config.entryFlag) {
        flags |= 1;
    }
    if (MPi_Work->config.multiBootFlag) {
        flags |= 2;
    }
    memset(buffer, 0, 0x80);
    buffer[0] = 0x100;
    ((u8 *)buffer)[2] = 1;
    ((u8 *)buffer)[3] = 8;
    work = MPi_Work;
    {
        u32 ggid = work->config.ggid;
        *(u32 *)(buffer + 2) = (ggid >> 24) | ((ggid >> 8) & 0xFF00) |
                               ((ggid << 8) & 0xFF0000) | (ggid << 24);
    }
    buffer[4] = ((tgid >> 8) & 0xff) | ((tgid << 8) & 0xff00);
    ((u8 *)buffer)[10] = size;
    memcpy(buffer + 8, work->config.gameInfo, size);
    ((u8 *)buffer)[11] = flags;
    work = MPi_Work;
    buffer[6] = ((work->config.sendSize >> 8) & 0xff) |
                ((work->config.sendSize << 8) & 0xff00);
    buffer[7] = ((work->config.recvSize >> 8) & 0xff) |
                ((work->config.recvSize << 8) & 0xff00);
}

static inline s32 MakeAndChangeBeacon(void) {
    MPWork *work = MPi_Work;
    OSMutex *mutex = &work->beaconMutex;
    u8 *buffer;
    s32 result;
    OSLockMutex(mutex);
    buffer = work->beacon;
    if (!buffer) {
        OSUnlockMutex(mutex);
        return -240;
    }
    MPiMakeGameInfo((u16 *)buffer);
    result = MPiChangeBeacon(buffer);
    OSUnlockMutex(mutex);
    return result < 0 ? result : 0;
}
s32 MPiMakeAndChangeBeacon(void) {
    return MakeAndChangeBeacon();
}

void MPiSetCurrentTsf(u16 tsf) {
    OSTime time = OSGetTime();
    BOOL enabled = OSDisableInterrupts();
    MPWork *work = MPi_Work;
    u32 *indexp = &work->tsfIndex;
    s32 *countp = &work->tsfCount;
    u32 index = *indexp;
    u32 count = *countp;
    s32 readIndex = index;
    u16 *samples = work->tsfSamples;
    s32 min;
    s32 max;
    s32 wrappedMax;
    u32 i;
    u32 offset;
    samples[index] = tsf - (OSTicksToMicroseconds(time) >> 6);
    if (++count > 10) {
        count = 10;
    }
    *countp = count;
    if (++index >= 10) {
        index = 0;
    }
    *indexp = index;
    OSRestoreInterrupts(enabled);
    min = 0x7fffffff;
    max = -1;
    wrappedMax = -1;
    for (i = 0; i < count; ++i) {
        s32 sample;
        s32 wrapped;
        if (readIndex < 0) {
            readIndex += 10;
        }
        sample = samples[readIndex];
        if (sample < min) {
            min = sample;
        }
        if (sample > max) {
            max = sample;
        }
        wrapped = (sample + 0x8000) & 0xffff;
        if (wrapped > wrappedMax) {
            wrappedMax = wrapped;
        }
        --readIndex;
    }
    offset = wrappedMax - 0x8000;
    if (max - min < 0x8000) {
        offset = max;
    }
    work->tsfOffset = offset & 0xffff;
}

s32 MPiGetVTsf(void) {
    MPWork *work = (MPWork *)mpWork;
    OSTime pictureTime = work->pictureTime;
    if (pictureTime < OSGetTime()) {
        pictureTime += OSMicrosecondsToTicks(16720);
    }
    return work->tsfOffset + (OSTicksToMicroseconds(pictureTime) >> 6);
}

s32 MPiUpdateBeacon(u32 callback, u32 arg, int synchronous) {
    MPCommand *command;
    s32 result;
    MPWork *work;
    if (!mpInitialized) {
        return -236;
    }
    if (mpFatalError) {
        return -1;
    }
    result = MPiAllocateCommandBlock(&command, synchronous);
    if (result < 0) {
        return result;
    }
    work = MPi_Work;
    result = MakeAndChangeBeacon();
    if (result != 0) {
        MPiFreeCommandBlock((u32)command);
        if (result < 0) {
            return result;
        }
    }
    command->command = 0x10002;
    command->callback = (MPCallback)callback;
    command->synchronous = synchronous;
    command->arg = (void *)arg;
    command->data = NULL;
    command->size = 0;
    result =
        MPiSendRequest((u32)&work->requestQueue, (int)command, synchronous);
    if (synchronous || result != 0) {
        MPiFreeCommandBlock((u32)command);
    }
    if (result < 0) {
        return result;
    }
    return 0;
}
