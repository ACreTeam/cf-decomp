#ifndef PRIVATE_MP_TYPES_H
#define PRIVATE_MP_TYPES_H

#include <revolution/mp.h>
#include <revolution/os.h>

typedef struct MPPortConfig {
    MPCallback callback;
    u32 priority;
    u32 arg;
    u32 reserved;
} MPPortConfig;

typedef struct MPConfig {
    void *(*alloc)(u32);
    void (*free)(void *);
    s32 priority;
    u32 mode;
    u32 ggid;
    u32 tgid;
    u32 channel;
    u32 lifeTime;
    u32 beaconPeriod;
    u32 options;
    u32 sendSize;
    u32 recvSize;
    BOOL entryFlag;
    BOOL multiBootFlag;
    s32 interval;
    u32 gameInfoSize;
    u8 gameInfo[0x70];
    MPCallback callback;
    MPPortConfig ports[16];
    u32 reserved[3];
} MPConfig;

typedef struct MPCallbackHeader {
    u32 command;
    s32 result;
    u32 port;
    void *arg;
} MPCallbackHeader;

typedef struct MPPortResult {
    union {
        MPCallbackHeader header;
        struct {
            u32 command;
            s32 result;
            u32 port;
            void *arg;
        };
    };
    u32 aid;
    union {
        struct {
            const u8 *data;
            u32 size;
            u32 sequence;
        } recv;
        struct {
            u32 receiveSize;
            u8 macAddress[6];
            u8 padding[2];
            u8 gameInfo[32];
            u32 frameSize;
        } connected;
        struct {
            const void *data;
            u32 size;
            u32 remainingBitmap;
            u32 sequence;
        } send;
        u8 raw[0x4c];
    } data;
} MPPortResult;

typedef struct MPCommand {
    union {
        MPCallbackHeader header;
        struct {
            u32 command;
            s32 result;
            u32 port;
            void *arg;
        };
    };
    const void *data;
    u32 size;
    u32 aidBitmap;
    u32 remainingBitmap;
    u32 *sendResult;
    u32 sequence;
    BOOL synchronous;
    OSSemaphore completion;
    MPCallback callback;
} MPCommand;

typedef struct MPListNode MPListNode;
struct MPListNode {
    MPListNode *next;
    MPListNode *prev;
    MPListNode *owner;
    MPCommand *command;
};

typedef struct MPWork {
    MPConfig config;
    u8 callbackStack[0x2000];
    u8 receiveStack[0x2000];
    u8 sequenceStack[0x2000];
    u8 notificationStack[0x2000];
    u8 portStack[0x2000];
    u8 receiveBuffer[0x2000];
    u8 sendFrame[0x400];
    u8 sendStatus[0x10];
    u8 reservedC5D0[0x10];
    u8 beacon[0x80];
    u8 parentParameters[0x180];
    u32 aid;
    u32 connectedBitmap;
    u32 allowedChannels;
    u8 macAddress[6];
    u8 childMacAddress[16][6];
    u16 portSequence[8][16];
    u16 sendSequence[8];
    u8 reservedC962[6];
    OSThread callbackThread;
    OSMessageQueue callbackQueue;
    OSMessage callbackMessages[64];
    OSMessageQueue freeCommandQueue;
    OSMessage freeCommandMessages[32];
    MPCommand commands[32];
    u32 rssi[16][16];
    u32 rssiIndex[16];
    u32 lifeTime[16];
    u16 disconnectReason[16];
    OSMessageQueue requestQueue;
    OSMessage requestMessages[32];
    u8 reservedDB00[0x1c];
    OSMutex beaconMutex;
    u32 tsfOffset;
    u16 tsfSamples[10];
    u32 tsfIndex;
    s32 tsfCount;
    OSMessageQueue mpEndQueue;
    OSMessage mpEndMessages[32];
    OSMessageQueue periodicQueue;
    OSMessage periodicMessage;
    OSThread receiveThread;
    OSThread sequenceThread;
    OSAlarm intervalAlarm;
    OSAlarm pictureAlarm;
    OSAlarm periodicAlarm;
    OSSemaphore intervalSemaphore;
    OSSemaphore pictureSemaphore;
    OSMessageQueue pictureQueue;
    OSMessage pictureMessages[2];
    OSTime pictureTime;
    s32 interval;
    OSMutex sendMutex;
    MPListNode sendQueues[8];
    MPListNode pendingList;
    MPListNode freeList;
    MPListNode listNodes[32];
    s32 receivePending;
    BOOL waitingReceive;
    OSSemaphore sendSemaphore;
    OSThread notificationThread;
    MPPortResult callbackResult;
    MPPortResult portResults[32];
    OSMessageQueue freePortQueue;
    OSMessage freePortMessages[32];
    u32 receiveCommands[32][2];
    OSMessageQueue freeReceiveQueue;
    OSMessage freeReceiveMessages[32];
    u32 maxSendSize;
    u8 reservedF7AC[0x14];
} MPWork;

typedef struct MPMPEndHeader {
    u16 type;
    u16 tsf;
    u16 count;
    u16 stride;
    u16 reserved;
} MPMPEndHeader;

typedef struct MPMPEndRecord {
    u16 size;
    u16 rssi;
    u16 aid;
    u16 reserved;
    u16 header;
} MPMPEndRecord;

#endif
