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

s32 MPSendAsync(const void *data, u32 size, u16 aidBitmap, u32 port,
                u32 *result, MPCallback callback, void *arg) {
    return MPiSend((u32)data, (u32)(size), (u32)(port), (u32)(aidBitmap),
                   (u32)result, (u32)callback, (u32)arg, (int)(0));
}

u32 MPiStartupTxRx(void) {
    MPListNode *nodes;
    MPWork *work = MPi_Work;
    MPWork *current;
    s32 priority;
    nodes = work->listNodes;
    NETMemSet(nodes, 0, sizeof(work->listNodes));
    MPiInitList(&work->freeList, nodes, 32);
    MPiInitList(&work->pendingList, NULL, 0);
    for (priority = 0; priority < 8; ++priority) {
        MPiInitList(&work->sendQueues[priority], NULL, 0);
    }
    current = MPi_Work;
    OSInitMutex(&current->sendMutex);
    OSInitSemaphore(&current->sendSemaphore, 1);
    current->receivePending = 0;
    current->waitingReceive = FALSE;
    return 0;
}

u32 MPiStartupTxRxLater(void) {
    return 0;
}

void MPiCleanupTxRxEarlier(void) {
    MPWork *work = MPi_Work;
    if (work) {
        int i;
        OSMutex *mutex = &work->sendMutex;
        OSLockMutex(mutex);
        for (i = 0; i < 32; ++i) {
            MPListNode *node = &work->listNodes[i];
            if (node->owner && node->owner != &work->freeList) {
                MPCommand *command = node->command;
                MPiRemoveList((int *)node);
                node->command = NULL;
                MPiInsertList((int *)work->freeList.prev, (int *)node);
                MPiNotifyDataSentToPort(-256, (int)command);
            }
        }
        OSUnlockMutex(mutex);
    }
}

void MPiCleanupTxRx(void) {
    if (mpWork == 0) {
        return;
    }
    NETMemSet((void *)(mpWork + 0xe3dc), (int)(0), (u32)(0x200));
    return;
}

static inline u32 GetAddressOverhead(u16 bitmap) {
    u32 size = 2;
    if ((bitmap & 0xfffe) == 0xfffe) {
        size = 0;
    }
    return size;
}
static inline u32 GetSequenceOverhead(u8 port) {
    u32 size = 0;
    if (port & 8) {
        size = 2;
    }
    return size;
}
static inline s32 GetPortPriority(u32 port, u32 *out) {
    u32 priority;
    if (out == NULL) {
        return -255;
    }
    if (port >= 16) {
        return -255;
    }
    priority = MPi_Work->config.ports[port].priority;
    if (priority >= 4) {
        return -255;
    }
    *out = priority;
    return 0;
}
static inline BOOL RetainConnectedAids(MPCommand *command) {
    command->remainingBitmap &= MPGetConnectedAIDs();
    return command->remainingBitmap != 0;
}
static inline MPListNode *AllocateSendNode(MPCommand *command) {
    MPListNode *node =
        (MPListNode *)MPiRemoveList((int *)MPi_Work->freeList.next);
    if (node != NULL) {
        node->command = command;
    }
    return node;
}
int MPiSend(u32 data, u32 size, u32 port, u32 aidBitmap, u32 sendResult,
            u32 callback, u32 arg, int synchronous) {
    MPCommand *command;
    s32 result;
    u32 priority;
    BOOL enabled;
    MPListNode *node;
    u32 addressSize;
    u32 sequenceSize;
    if (!mpInitialized) {
        return -236;
    }
    if (mpFatalError) {
        return -1;
    }
    addressSize = GetAddressOverhead(aidBitmap);
    sequenceSize = GetSequenceOverhead(port);
    if (size + sequenceSize + addressSize + 2 > MPi_Work->maxSendSize) {
        return -255;
    }
    if (size & 1) {
        return -255;
    }
    result = MPiAllocateCommandBlock(&command, synchronous);
    if (result < 0) {
        return result;
    }
    command->command = 0x10001;
    command->callback = (MPCallback)callback;
    command->synchronous = synchronous;
    command->arg = (void *)arg;
    command->data = (const void *)data;
    command->size = size;
    command->port = port;
    command->aidBitmap = aidBitmap;
    command->remainingBitmap = aidBitmap;
    command->sendResult = (u32 *)sendResult;
    if (port & 8) {
        u16 sequence;
        enabled = OSDisableInterrupts();
        sequence = MPi_Work->sendSequence[port & 7]++;
        OSRestoreInterrupts(enabled);
        command->sequence = sequence;
    } else {
        command->sequence = 0;
    }
    RetainConnectedAids(command);
    result = GetPortPriority(port, &priority);
    if (!result) {
        MPListNode *queue;
        MPCommand *queued;
        enabled = OSDisableInterrupts();
        queue = &MPi_Work->sendQueues[priority * 2 + 1];
        queued = command;
        OSLockMutex(&MPi_Work->sendMutex);
        node = AllocateSendNode(queued);
        if (!node) {
            MPiSetFatalError(2);
        } else {
            MPiInsertList((int *)queue->prev, (int *)node);
        }
        OSUnlockMutex(&MPi_Work->sendMutex);
        MPiRespondToWaitingRequest((int)command, synchronous);
        OSRestoreInterrupts(enabled);
    }
    if (synchronous || result) {
        MPiFreeCommandBlock((u32)command);
    }
    if (result >= 0) {
        result = 0;
    }
    return result;
}

static inline u32 ReadFrameWord(const u16 *data) {
    u32 value = *data;
    return (value >> 8) | ((value & 0xff) << 8);
}

void MPiParseReceivedFrame(int aid, u32 header, u16 *data, u32 size) {
    MPWork *work;
    u32 ownMask;
    BOOL first;
    OSSemaphore *semaphore;
    if (!size || (size & 1)) {
        return;
    }
    work = MPi_Work;
    ownMask = 1 << work->aid;
    semaphore = &work->sendSemaphore;
    first = TRUE;
    while ((s32)size > 0) {
        u32 length;
        u32 addressed;
        u32 sequenced;
        u32 sequence;
        u32 aidBitmap;
        u32 port;
        u16 *payload;
        if (!first) {
            header = ReadFrameWord(data++);
            size -= 2;
        } else {
            first = FALSE;
        }
        length = (header & 0xff) << 1;
        if (!length) {
            length = 0x200;
        }
        addressed = (header >> 12) & 1;
        sequenced = (header >> 11) & 1;
        port = (header >> 8) & 15;
        if (length + (addressed ? 2 : 0) + (sequenced ? 2 : 0) > size) {
            return;
        }
        payload = data;
        data = (u16 *)((u8 *)data + length);
        size -= length;
        if (sequenced) {
            sequence = ReadFrameWord(data++);
            size -= 2;
        } else {
            sequence = 0;
        }
        if (addressed) {
            aidBitmap = ReadFrameWord(data++);
            size -= 2;
        } else {
            aidBitmap = 0xffff;
        }
        if (!(aidBitmap & ownMask)) {
            continue;
        }
        if (sequenced) {
            u16 *last = &work->portSequence[port & 7][aid];
            BOOL accept;
            if (*last & 1) {
                accept = TRUE;
            } else {
                accept = (u16)(*last - sequence * 2) >= 0x100;
            }
            if (!accept) {
                continue;
            }
            *last = sequence << 1;
        }
        {
            BOOL enabled = OSDisableInterrupts();
            s32 result;
            ++work->receivePending;
            OSRestoreInterrupts(enabled);
            result = MPiNotifyDataReceivedToPort(port, aid, (u32)payload,
                                                 length, sequence);
            if (result) {
                enabled = OSDisableInterrupts();
                if (--work->receivePending == 0 && work->waitingReceive) {
                    OSSignalSemaphore(semaphore);
                }
                OSRestoreInterrupts(enabled);
            }
        }
    }
}

void MPiProcessDataSentNotification(u32 aidBitmap) {
    u32 connected = MPGetConnectedAIDs();
    MPWork *work;
    MPListNode *node;
    OSLockMutex(&MPi_Work->sendMutex);
    work = MPi_Work;
    while ((node = (MPListNode *)MPiRemoveList(
                (int *)work->pendingList.next)) != NULL) {
        MPCommand *command = node->command;
        u32 remaining;
        node->command = NULL;
        MPiInsertList((int *)work->freeList.prev, (int *)node);
        remaining = connected & ~aidBitmap & command->remainingBitmap;
        command->remainingBitmap = remaining;
        if ((command->port & 8) && remaining) {
            if (RetainConnectedAids(command) > 0) {
                u32 priority;
                s32 result = GetPortPriority(command->port, &priority);
                if (result == 0) {
                    MPListNode *queue = &MPi_Work->sendQueues[priority * 2];
                    OSLockMutex(&MPi_Work->sendMutex);
                    node = AllocateSendNode(command);
                    if (!node) {
                        MPiSetFatalError(2);
                    } else {
                        MPiInsertList((int *)queue->prev, (int *)node);
                    }
                    OSUnlockMutex(&MPi_Work->sendMutex);
                } else {
                    MPiNotifyDataSentToPort(-255, (int)command);
                }
            } else {
                MPiNotifyDataSentToPort(0, (int)command);
            }
        } else {
            MPiNotifyDataSentToPort(0, (int)command);
        }
    }
    OSUnlockMutex(&MPi_Work->sendMutex);
}

void MPiClearSendQueue(void) {
    u32 connected = MPGetConnectedAIDs();
    MPWork *work = MPi_Work;
    u32 priority;
    OSLockMutex(&work->sendMutex);
    for (priority = 0; priority < 8; ++priority) {
        MPListNode *node = work->sendQueues[priority].next;
        while (node != node->owner) {
            MPListNode *next = node->next;
            MPCommand *command = node->command;
            command->remainingBitmap &= connected;
            if (!command->remainingBitmap) {
                MPiRemoveList((int *)node);
                node->command = NULL;
                MPiInsertList((int *)work->freeList.prev, (int *)node);
                MPiNotifyDataSentToPort(0, (int)command);
            }
            node = next;
        }
    }
    OSUnlockMutex(&work->sendMutex);
}

void MPiConsumeSendQueue(u32 size) {
    int iVar1;
    int iVar2;
    BOOL enable;
    enable = OSDisableInterrupts();
    iVar1 = mpWork;
    iVar2 = *(int *)(mpWork + 0xe5dc) - 1;
    *(int *)(mpWork + 0xe5dc) = iVar2;
    if ((iVar2 == 0) && (*(int *)(iVar1 + 0xe5e0) != 0)) {
        OSSignalSemaphore((OSSemaphore *)(iVar1 + 0xe5e4));
    }
    OSRestoreInterrupts(enable);
    return;
}

void MPiPrepareSendQueue(void) {
    BOOL enabled = OSDisableInterrupts();
    MPWork *work = MPi_Work;
    if (work->receivePending) {
        work->waitingReceive = TRUE;
        OSWaitSemaphore(&work->sendSemaphore);
        work->waitingReceive = FALSE;
    }
    OSRestoreInterrupts(enabled);
}

u32 MPiMakeSendFrame(u16 *frame, u32 capacity, int address) {
    MPCommand *command = (MPCommand *)address;
    u32 size = command->size;
    BOOL sequenced;
    BOOL addressed;
    u32 aidBitmap;
    u32 sequence;
    const void *data;
    u32 port;
    u32 total;
    u32 words;
    u32 index;
    u16 header;
    aidBitmap = command->aidBitmap;
    data = command->data;
    port = command->port;
    sequence = command->sequence;
    addressed = (aidBitmap & 0xfffe) != 0xfffe;
    if ((size & 1) == 1) {
        ++size;
    }
    sequenced = (port & 8) != 0;
    total = size + ((port & 8) ? 2 : 0) + 2 +
            ((aidBitmap & 0xfffe) != 0xfffe ? 2 : 0);
    if (total > capacity) {
        return -255;
    }
    words = ((size + 1) >> 1) & 0xff;
    header = (addressed ? 0x1000 : 0) | ((port & 15) << 8) | words;
    *frame = ((u32)header >> 8) | (((u32)header & 0xff) << 8);
    if (data) {
        memcpy(frame + 1, data, size);
    }
    index = words + 1;
    if (sequenced) {
        frame[index] = ((sequence >> 8) & 0xff) | ((sequence << 8) & 0xff00);
        ++index;
    }
    if (addressed) {
        frame[index] = ((aidBitmap >> 8) & 0xff) | ((aidBitmap << 8) & 0xff00);
    }
    return total;
}

int MPiBuildSendFrame(u32 address, u32 capacity, u32 aidBitmap, int last) {
    u16 *frame = (u16 *)address;
    MPWork *work;
    u16 *out;
    BOOL written = FALSE;
    s32 size = 0;
    u32 priority;
    u32 blockedPorts;
    OSMutex *mutex;
    if (capacity < 2) {
        return -255;
    }
    work = MPi_Work;
    out = frame;
    mutex = &work->sendMutex;
    OSLockMutex(mutex);
    blockedPorts = 0;
    for (priority = 0; priority < 8; ++priority) {
        MPListNode *node = work->sendQueues[priority].next;
        while (node != node->owner) {
            MPListNode *next = node->next;
            MPCommand *command = node->command;
            u32 port = command->port;
            u32 portMask = 1 << port;
            if (!command->remainingBitmap ||
                (command->remainingBitmap & ~aidBitmap) ||
                (blockedPorts & portMask)) {
                blockedPorts &= 1 << port;
            } else {
                s32 length =
                    MPiMakeSendFrame(out, capacity - size, (int)command);
                if (length <= 0) {
                    blockedPorts &= portMask;
                } else {
                    written = TRUE;
                    MPiRemoveList((int *)node);
                    MPiInsertList((int *)work->pendingList.prev, (int *)node);
                    out = (u16 *)((u8 *)out + length);
                    size += length;
                }
            }
            node = next;
        }
    }
    OSUnlockMutex(mutex);
    if (!written) {
        size += 2;
        *out = 0;
    }
    if (last) {
        *frame |= 0x80;
    }
    return size;
}
