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

u32 MPiStartupParent(void) {
    MPWork *work = MPi_Work;
    s32 i;
    work->interval = 0;
    work->maxSendSize = work->config.sendSize + 6;
    *(u32 *)((u8 *)work + 0xf7ac) = work->config.recvSize + 4;
    OSCreateAlarm(&work->intervalAlarm);
    OSCreateAlarm(&MPi_Work->pictureAlarm);
    OSCreateAlarm(&MPi_Work->periodicAlarm);
    OSInitSemaphore(&MPi_Work->pictureSemaphore, 0);
    OSInitSemaphore(&MPi_Work->intervalSemaphore, 0);
    OSInitMessageQueue(&MPi_Work->pictureQueue, MPi_Work->pictureMessages, 1);
    OSInitMessageQueue(&MPi_Work->mpEndQueue, MPi_Work->mpEndMessages, 32);
    OSInitMessageQueue(&MPi_Work->periodicQueue, &MPi_Work->periodicMessage, 1);
    memset(MPi_Work->freeReceiveMessages, 0,
           sizeof(MPi_Work->freeReceiveMessages));
    OSInitMessageQueue(&MPi_Work->freeReceiveQueue,
                       MPi_Work->freeReceiveMessages, 32);
    memset(MPi_Work->receiveCommands, 0, sizeof(MPi_Work->receiveCommands));
    work = MPi_Work;
    for (i = 0; i < 32; ++i) {
        OSSendMessage(&work->freeReceiveQueue, work->receiveCommands[i], 0);
    }
    memset(MPi_Work->receiveStack, 0, sizeof(MPi_Work->receiveStack));
    memset(&MPi_Work->receiveThread, 0, sizeof(OSThread));
    memset(MPi_Work->sequenceStack, 0, sizeof(MPi_Work->sequenceStack));
    memset(&MPi_Work->sequenceThread, 0, sizeof(OSThread));
    return 0;
}

u32 MPiStartupParentLater(void) {
    MPWork *work;
    s32 i;
    OSMessage message;
    if (!OSCreateThread(
            &MPi_Work->receiveThread, (void *(*)(void *))HandleMPEndIndication,
            NULL, MPi_Work->receiveStack + sizeof(MPi_Work->receiveStack),
            sizeof(MPi_Work->receiveStack), MPi_Work->config.priority + 1, 0)) {
        return -239;
    }
    OSResumeThread(&MPi_Work->receiveThread);
    memset(MPi_Work->freeReceiveMessages, 0,
           sizeof(MPi_Work->freeReceiveMessages));
    OSInitMessageQueue(&MPi_Work->freeReceiveQueue,
                       MPi_Work->freeReceiveMessages, 32);
    memset(MPi_Work->receiveCommands, 0, sizeof(MPi_Work->receiveCommands));
    work = MPi_Work;
    for (i = 0; i < 32; ++i) {
        OSSendMessage(&work->freeReceiveQueue, work->receiveCommands[i], 0);
    }
    while (OSReceiveMessage(&MPi_Work->mpEndQueue, &message, 0)) {
    }
    if (!OSCreateThread(
            &MPi_Work->sequenceThread, (void *(*)(void *))HandleMPSequence,
            NULL, MPi_Work->sequenceStack + sizeof(MPi_Work->sequenceStack),
            sizeof(MPi_Work->sequenceStack), MPi_Work->config.priority, 0)) {
        return -239;
    }
    OSResumeThread(&MPi_Work->sequenceThread);
    MPi_Work->pictureTime = OSGetTime() + OSMicrosecondsToTicks(16720);
    OSSetPeriodicAlarm(&MPi_Work->pictureAlarm, OSGetTime(),
                       OSMicrosecondsToTicks(16720),
                       (OSAlarmHandler)PictureFrameAlarmHandler);
    OSSetPeriodicAlarm(&MPi_Work->periodicAlarm, OSGetTime(),
                       OSMicrosecondsToTicks(1000000),
                       (OSAlarmHandler)ParentPeriodicAlarmHandler);
    return 0;
}

void MPiCleanupParentEarlier(void) {
    MPWork *work = MPi_Work;
    if (work) {
        MPWork *current;
        if (MPi_Work->sequenceThread.state) {
            OSJoinThread(&MPi_Work->sequenceThread, NULL);
        }
        current = MPi_Work;
        memset(&MPi_Work->sequenceThread, 0, sizeof(OSThread));
        if (current->receiveThread.state) {
            OSJoinThread(&work->receiveThread, NULL);
        }
        work = MPi_Work;
        memset(&MPi_Work->receiveThread, 0, sizeof(OSThread));
        OSCancelAlarm(&work->periodicAlarm);
        OSCancelAlarm(&work->pictureAlarm);
        OSCancelAlarm(&work->intervalAlarm);
    }
}

void MPiCleanupParent(void) {
    int iVar1;
    iVar1 = mpWork;
    if (mpWork != 0) {
        memset((void *)((void *)(mpWork + 0xdb74)), (int)(0), (u32)(0x80));
        memset((void *)((void *)(iVar1 + 0xdc14)), (int)(0), (u32)(4));
    }
    return;
}

void MPiProcessParentDataSendError(void) {
    MPWork *work = MPi_Work;
    u32 *command;
    OSReceiveMessage(&work->freeReceiveQueue, (OSMessage *)&command,
                     OS_MESSAGE_BLOCK);
    {
        u32 *received = command;
        received[0] = 0;
        received[1] = 0;
        OSSendMessage(&work->mpEndQueue, received, OS_MESSAGE_BLOCK);
    }
}

u32 HandleMPEndIndication(void) {
    u8 *buffers[2];
    OSMessageQueue *freeQueue;
    OSMessageQueue *queue;
    void *frame;
    MPWork *work = MPi_Work;
    s32 index;
    if (!work) {
        return 0;
    }
    buffers[0] = work->portStack;
    buffers[1] = work->receiveBuffer;
    queue = &work->mpEndQueue;
    freeQueue = &work->freeReceiveQueue;
    index = 0;
    for (;;) {
        s32 size;
        frame = buffers[index];
        size = MPiReceiveFrame(frame, 0x2000);
        if (size == (s32)0x80008000) {
            OSSendMessage(&work->mpEndQueue, NULL, OS_MESSAGE_BLOCK);
            break;
        }
        if (size >= 0) {
            u32 *slot;
            u32 *command;
            OSReceiveMessage(freeQueue, (OSMessage *)&slot, OS_MESSAGE_BLOCK);
            command = slot;
            command[0] = (u32)frame;
            command[1] = size;
            OSSendMessage(queue, command, OS_MESSAGE_BLOCK);
        }
        if (++index >= 2) {
            index = 0;
        }
    }
    return 0;
}

u32 HandleMPSequence(void) {
    BOOL bVar1;
    u16 uVar2;
    int iVar3;
    u32 *puVar4;
    int iVar5;
    int iVar6;
    u16 uVar7;
    BOOL enable;
    u32 uVar8;
    u32 uVar9;
    u32 uVar10;
    int local_48;
    u32 *local_44;
    int local_40[5];
    iVar3 = mpWork;
    uVar10 = 0;
    do {
        do {
            while (*(int *)(iVar3 + 0xc7e4) == 0) {
                iVar6 = OSReceiveMessage((OSMessageQueue *)(mpWork + 0xdb54),
                                         (OSMessage *)(local_40), (s32)(0));
                if ((iVar6 != 0) && (local_40[0] == 0)) {
                    return 0;
                }
                MPiClearSendQueue();
                OSSetAlarm((OSAlarm *)(mpWork + 0xe248),
                           (s64)((OS_BUS_CLOCK / 500000) * 0x5dc >> 3),
                           (OSAlarmHandler)IntervalAlarmHandler);
                OSWaitSemaphore((OSSemaphore *)(mpWork + 0xe2d8));
                OSWaitSemaphore((OSSemaphore *)(mpWork + 0xe2e4));
            }
            iVar6 = OSReceiveMessage((OSMessageQueue *)(iVar3 + 0xdbf4),
                                     (OSMessage *)(local_40), (s32)(0));
            if ((iVar6 != 0) &&
                (CheckLifeTime(), *(int *)(mpWork + 0x38) == 0)) {
                MPiChangeVTSF();
            }
            iVar6 = OSReceiveMessage((OSMessageQueue *)(mpWork + 0xe2f0),
                                     (OSMessage *)(local_40), (s32)(0));
            if (iVar6 != 0) {
                if (*(int *)(mpWork + 0x38) != 0) {
                    MPiChangeVTSF();
                }
                iVar6 = mpWork;
                iVar5 = *(int *)(mpWork + 0x38);
                if ((iVar5 < 1) || (iVar5 > 4)) {
                    iVar5 = 4;
                }
                *(int *)(mpWork + 0xe320) = iVar5;
                uVar10 = *(u32 *)(iVar6 + 0xc7e4) & 0xffff;
            }
            bVar1 = *(int *)(mpWork + 0xe320) < 2;
            MPiClearSendQueue();
            iVar6 = MPiBuildSendFrame((u32)(mpWork + 0xc1c0),
                                      (u32)(*(u32 *)(mpWork + 0xf7a8)),
                                      (u32)(uVar10), (int)(bVar1));
        } while (iVar6 < 0);
        memset((void *)((void *)(mpWork + 0xc5c0)), (int)(0), (u32)(0x10));
        iVar5 = mpWork;
        *(u16 *)(mpWork + 0xc5c0) = 0;
        *(u16 *)(iVar5 + 0xc5c2) = 0;
        *(u16 *)(iVar5 + 0xc5c4) = 0;
        *(short *)(iVar5 + 0xc5c6) = (short)*(u32 *)(iVar5 + 0x2c) + 4;
        *(short *)(iVar5 + 0xc5c8) = (short)uVar10;
        if (bVar1) {
            uVar7 = GetTmptt((int)(iVar6), (u32)(*(u16 *)(iVar5 + 0xc5c6)),
                             (u32)(*(u16 *)(iVar5 + 0xc5c8)));
            *(u16 *)(mpWork + 0xc5ca) = uVar7;
        } else {
            *(u16 *)(iVar5 + 0xc5ca) = 0;
        }
        iVar5 = mpWork;
        *(u16 *)(mpWork + 0xc5cc) = 0;
        uVar2 = *(u16 *)(iVar5 + 0xc5c8);
        iVar6 = MPiSendFrame((u16 *)((u16 *)(iVar5 + 0xc1c0)), (u16)(iVar6),
                             (void *)(iVar5 + 0xc5c0));
        if (iVar6 == 0) {
            OSReceiveMessage((OSMessageQueue *)(mpWork + 0xdb54),
                             (OSMessage *)(&local_44), (s32)(1));
            puVar4 = local_44;
            if (local_44 == (u32 *)0x0) {
                return 0;
            }
            uVar9 = *local_44;
            uVar8 = local_44[1];
            memset((void *)(local_44), (int)(0), (u32)(8));
            OSSendMessage((OSMessageQueue *)(mpWork + 0xf708),
                          (OSMessage)(puVar4), (s32)(1));
            MPiPrepareSendQueue();
            if (uVar8 > 0x2000) {
                uVar8 = 0;
            }
            uVar8 =
                ParseMPEndIndication((int)(uVar9), (u32)(uVar8), (u32)(uVar2));
        } else {
            iVar6 = OSReceiveMessage((OSMessageQueue *)(mpWork + 0xdb54),
                                     (OSMessage *)(&local_48), (s32)(0));
            if ((iVar6 == 1) && (local_48 == 0)) {
                return 0;
            }
            uVar8 = 0;
        }
        if ((uVar2 & 0xfffe) == uVar8) {
            enable = OSDisableInterrupts();
            *(int *)(mpWork + 0xe320) = *(int *)(mpWork + 0xe320) - 1;
            OSRestoreInterrupts(enable);
        } else if (bVar1) {
            uVar10 = uVar10 & ~uVar8;
        }
        OSSetAlarm((OSAlarm *)(mpWork + 0xe248),
                   (s64)((OS_BUS_CLOCK / 500000) * 0x5dc >> 3),
                   (OSAlarmHandler)IntervalAlarmHandler);
        OSWaitSemaphore((OSSemaphore *)(mpWork + 0xe2d8));
        if (*(int *)(mpWork + 0xe320) < 1) {
            OSWaitSemaphore((OSSemaphore *)(mpWork + 0xe2e4));
        }
    } while (TRUE);
}

void CheckLifeTime(void) {
    s32 lifetime = MPi_Work->config.lifeTime;
    if (lifetime) {
        OSTick forceLimit = OSGetTick() - OSMillisecondsToTicks(lifetime * 2);
        OSTick limit = OSGetTick() - OSMillisecondsToTicks(lifetime);
        u32 expired = 0;
        u32 forced = 0;
        BOOL enabled = OSDisableInterrupts();
        u32 connected = MPGetConnectedAIDs();
        u32 aid;
        for (aid = 1; aid < 16; ++aid) {
            if ((connected & (1 << aid)) && MPiCompareLifeTime(aid, limit)) {
                if (!MPi_Work->disconnectReason[aid]) {
                    expired |= 1 << aid;
                } else if (MPiCompareLifeTime(aid, forceLimit)) {
                    forced |= 1 << aid;
                }
            }
        }
        OSRestoreInterrupts(enabled);
        for (aid = 1; aid < 16; ++aid) {
            if (expired & (1 << aid)) {
                MPiProcessLifeTimeExpiration(aid);
            } else if (forced & (1 << aid)) {
                MPiProcessForcibleLifeTimeExpiration(aid);
            }
        }
    }
}

static inline u16 ReadParentFrameHeader(const MPMPEndRecord *record) {
    u32 word = record->header;
    return ((word >> 8) & 0xff) | ((word << 8) & 0xff00);
}
u32 ParseMPEndIndication(int address, u32 size, u32 aidBitmap) {
    const MPMPEndHeader *header = (const MPMPEndHeader *)address;
    const MPMPEndRecord *records[15] = {NULL};
    u32 received = 0;
    s32 i;
    if (header) {
        const MPMPEndRecord *record;
        if (size !=
            (u16)(header->count * header->stride + sizeof(MPMPEndHeader))) {
            return 0;
        }
        MPiSetCurrentTsf(header->tsf);
        record = (const MPMPEndRecord *)(header + 1);
        for (i = 0; i < header->count; ++i) {
            u16 aid = record->aid;
            if (aid) {
                if (aid >= 16) {
                    break;
                }
                if (record->size != 0xffff && record->size != 0) {
                    records[aid - 1] = record;
                    received |= 1 << aid;
                }
            }
            record =
                (const MPMPEndRecord *)((const u8 *)record + header->stride);
        }
    }
    for (i = 0; i < 15; ++i) {
        u8 aid = i + 1;
        if (records[i]) {
            MPiParseReceivedFrame(aid, ReadParentFrameHeader(records[i]),
                                  (u16 *)(records[i] + 1),
                                  records[i]->size - 2);
            MPiAddRssi(aid, records[i]->rssi & 0xff);
            MPiUpdateLifeTime(aid);
        } else if (aidBitmap & (1 << aid)) {
            MPiAddRssi(aid, 0xaf);
        }
    }
    MPiProcessDataSentNotification(received);
    return received;
}

u32 GetTmptt(int sendSize, u32 recvSize, u32 aidBitmap) {
    u32 recvTime = (recvSize + 28) * 4 + 102;
    u32 count;
    u32 required;
    s32 available;
    if (recvSize & 0x8000) {
        recvTime = recvSize & 0x7fff;
    }
    count = MPiCountPopulation(aidBitmap);
    required = (sendSize + 34) * 4 + 96;
    required += recvTime * (count & 0xff) + 904;
    available = OSTicksToMicroseconds(
                    (u32)((u32)MPi_Work->pictureTime - (u32)OSGetTime())) /
                    10 -
                20;
    if (available < 0) {
        available = 0;
    }
    if (available * 10 < required) {
        available = 0;
    }
    return available & 0xffff;
}

void IntervalAlarmHandler(void) {
    OSSemaphore *semaphore = &MPi_Work->intervalSemaphore;
    OSTryWaitSemaphore(semaphore);
    OSSignalSemaphore(semaphore);
}

void PictureFrameAlarmHandler(void) {
    MPWork *work = MPi_Work;
    OSSemaphore *semaphore = &work->pictureSemaphore;
    work->pictureTime = OSGetTime() + OSMicrosecondsToTicks(16720);
    OSTryWaitSemaphore(semaphore);
    OSSignalSemaphore(semaphore);
    OSSendMessage(&work->pictureQueue, NULL, 0);
}

void ParentPeriodicAlarmHandler(void) {
    OSSendMessage((OSMessageQueue *)(mpWork + 0xdbf4), (OSMessage)(0),
                  (s32)(0));
    return;
}
