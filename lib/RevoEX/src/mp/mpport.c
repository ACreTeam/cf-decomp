#include <private/mp.h>
#include <string.h>

u32 MPiStartupPort(void) {
    MPWork *work = MPi_Work;
    MPPortResult *result;
    s32 port;
    OSMessageQueue *queue;
    u32 aid;
    memset(work->sendSequence, 0, sizeof(work->sendSequence));
    memset(work->portSequence, 0, sizeof(work->portSequence));
    for (aid = 0; aid < 16; ++aid) {
        for (port = 0; port < 8; ++port) {
            work->portSequence[port][aid] = 1;
        }
    }
    work = MPi_Work;
    result = work->portResults;
    memset(result, 0, sizeof(work->portResults));
    queue = &work->freePortQueue;
    OSInitMessageQueue(queue, work->freePortMessages, 32);
    for (port = 0; port < 32; ++port) {
        OSSendMessage(queue, result, 0);
        ++result;
    }
    return 0;
}

static inline MPPortResult *AllocatePortResult(OSMessageQueue *queue) {
    MPPortResult *result;
    if (MPiAllocateCommandBlockFromQueue(queue, (MPCommand **)&result, 1) ==
        0) {
        return result;
    }
    return NULL;
}

static inline void SendPortResult(OSMessageQueue *queue, MPPortResult *result,
                                  u32 command, u32 port) {
    result->result = 0;
    result->command = command;
    result->port = port;
    if (MPiSendCallback((u32)result)) {
        MPiFreeCommandBlockToQueue((u32)queue, (u32)result);
    }
}

u32 MPiStartupPortLater(void) {
    OSMessageQueue *queue = &MPi_Work->freePortQueue;
    MPPortResult *result = AllocatePortResult(queue);
    if (result) {
        SendPortResult(queue, result, 1, -1);
        return 0;
    }
    MPiSetFatalError(1);
    return -1;
}

void MPiCleanupPort(void) {
    if (MPi_Work) {
        OSMessageQueue *queue = &MPi_Work->freePortQueue;
        MPPortResult *result = AllocatePortResult(queue);
        MPWork *work;
        if (result) {
            SendPortResult(queue, result, 2, -1);
        }
        work = MPi_Work;
        memset(work->sendSequence, 0, sizeof(work->sendSequence));
        memset(work->portSequence, 0, sizeof(work->portSequence));
    }
}

void MPiResetPortByAid(int aid) {
    MPWork *work = MPi_Work;
    int port;
    for (port = 0; port < 8; ++port) {
        work->portSequence[port][aid] = 1;
    }
}

void MPiNotifyDataSentToPort(u32 status, int address) {
    MPCommand *command = (MPCommand *)address;
    if (command->sendResult) {
        *command->sendResult = command->remainingBitmap;
    }
    MPiNotifyCallback(status, command);
}

u32 MPiNotifyDataReceivedToPort(u32 port, u32 aid, u32 data, u32 size,
                                u32 sequence) {
    OSMessageQueue *queue;
    MPPortResult *result;
    if (!size) {
        return -255;
    }
    if (port >= 16) {
        return -255;
    }
    if (size >= 0x204) {
        return -255;
    }
    queue = &MPi_Work->freePortQueue;
    result = AllocatePortResult(queue);
    if (result) {
        result->result = 0;
        result->aid = aid;
        result->data.recv.data = (const u8 *)data;
        result->data.recv.size = size;
        result->data.recv.sequence = sequence;
        if (port >= 16) {
            MPiFreeCommandBlockToQueue((u32)queue, (u32)result);
            return -255;
        }
        result->result = 0;
        result->port = port;
        result->command = 3;
        if (MPiSendCallback((u32)result)) {
            MPiFreeCommandBlockToQueue((u32)queue, (u32)result);
        }
        return 0;
    }
    return -255;
}

void MPiNotifyConnectedToPort(u32 aid, void *macAddress, void *gameInfo,
                              u32 frameSize) {
    OSMessageQueue *queue = &MPi_Work->freePortQueue;
    MPPortResult *result = AllocatePortResult(queue);
    if (result) {
        result->result = 0;
        result->aid = aid;
        result->data.connected.frameSize = frameSize;
        result->data.connected.receiveSize = frameSize >= 8 ? frameSize - 8 : 0;
        memcpy(result->data.connected.macAddress, macAddress, 6);
        memcpy(result->data.connected.gameInfo, gameInfo, 32);
        SendPortResult(queue, result, 4, -1);
    }
}

void MPiNotifyDisconnectedToPort(u32 aid, u32 reason, void *macAddress) {
    OSMessageQueue *queue = &MPi_Work->freePortQueue;
    MPPortResult *result = AllocatePortResult(queue);
    if (result) {
        result->result = 0;
        result->aid = aid;
        result->data.connected.receiveSize = reason;
        memcpy(result->data.connected.macAddress, macAddress, 6);
        SendPortResult(queue, result, 5, -1);
    }
}

void MPiHandlePortCallback(int *address) {
    MPPortResult *result = (MPPortResult *)address;
    MPWork *work;
    if (result->port == (u32)-1) {
        u32 port;
        MPPortConfig *config;
        work = MPi_Work;
        for (port = 0; port < 16; ++port) {
            MPCallback callback;
            result->port = port;
            config = &work->config.ports[port];
            callback = config->callback;
            if (callback) {
                result->arg = (void *)config->arg;
                callback(result->command, result);
            }
            switch (result->command) {
            case 3: {
                MPiConsumeSendQueue((u32)result->data.recv.data);
            } break;
            default:
                break;
            }
        }
    } else {
        MPCallback callback;
        work = MPi_Work;
        callback = work->config.ports[result->port].callback;
        if (callback) {
            result->arg = (void *)work->config.ports[result->port].arg;
            callback(result->command, result);
        }
        switch (result->command) {
        case 3: {
            MPiConsumeSendQueue((u32)result->data.recv.data);
        } break;
        default:
            break;
        }
    }
    MPiFreeCommandBlockToQueue((u32)&work->freePortQueue, (u32)result);
}
