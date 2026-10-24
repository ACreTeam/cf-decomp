#ifndef PRIVATE_WDP_TYPES_H
#define PRIVATE_WDP_TYPES_H

#include <revolution/os.h>

typedef struct WDPSegmentRegion {
    s32 type;
    u32 offset;
    u32 size;
} WDPSegmentRegion;

typedef struct WDPSegmentMapper {
    u32 blockSize;
    u16 blockCount;
    u16 firstBlock[3];
    u8 header[0xe4];
} WDPSegmentMapper;

typedef struct WDPPlayerInfo {
    union {
        u8 playerAndColor;
        struct {
            u8 playerNumber : 4;
            u8 favoriteColor : 4;
        };
    };
    u8 nameLength;
    u16 name[10];
} WDPPlayerInfo;

typedef struct WDPBeaconHeader {
    u8 ggid[4];
    union {
        u8 flags;
        struct {
            u8 sessionIndex : 6;
            u8 imageMode : 2;
        };
    };
    u8 reserved;
    u8 sequence;
    u8 part;
} WDPBeaconHeader;

typedef struct WDPGameInfo {
    u8 data[0x358];
    WDPBeaconHeader header;
    u16 playerBitmap;
    u16 changedBitmap;
    WDPPlayerInfo players[16];
    u8 extraInfo[8];
    u16 connectedBitmap;
    u16 changedPlayers;
} WDPGameInfo;

typedef struct WDPSession WDPSession;
typedef struct WDPParentContext WDPParentContext;
typedef BOOL (*WDPReadSegment)(WDPSession *, void *, const WDPSegmentRegion *);
typedef void (*WDPCallback)(WDPParentContext *, s32, u32);

struct WDPSession {
    WDPSession *next;
    void *userData;
    WDPReadSegment readSegment;
    union {
        WDPGameInfo game;
        struct {
            u8 gameInfo[0x358];
            u8 ggid[4];
            union {
                u8 flags;
                struct {
                    u8 sessionIndex : 6;
                    u8 imageMode : 2;
                };
            };
            u8 reserved;
            u8 sequence;
            u8 part;
            u16 playerBitmap;
            u16 changedBitmap;
            WDPPlayerInfo players[16];
            u8 extraInfo[8];
            u16 connectedBitmap;
            u16 changedPlayers;
        };
    };
    WDPSegmentMapper mapper;
    u16 sendBlock;
    u16 ackBlock;
    u16 requestedBitmap;
    u16 transferBitmap;
    u16 activeBitmap;
    u16 padding;
};

typedef struct WDPChildInfo {
    s32 status;
    WDPSession *session;
    union {
        u8 request[0x20];
        struct {
            u8 ggid[4];
            WDPPlayerInfo player;
            u8 reserved[6];
        };
    };
    u32 receivedParts;
    u32 partCount;
    u32 receivedBlocks;
} WDPChildInfo;

typedef struct WDPPacket {
    u8 *data;
    u16 size;
    u16 aidBitmap;
} WDPPacket;

struct WDPParentContext {
    WDPCallback callback;
    WDPSession *sessions;
    s32 lockDepth;
    BOOL interrupts;
    WDPSession *beaconSession;
    u16 beaconBitmap;
    u8 beaconPart;
    u8 beaconSequence;
    WDPSession *controlSession;
    WDPSession *dataSession;
    u32 connectedBitmap;
    u32 packetSize;
    WDPChildInfo children[16];
    u16 pendingBitmap;
    u16 activeBitmap;
    u16 rejectedBitmap;
    u16 bootBitmap;
    u16 deniedBitmap;
    u16 requestedSessions;
    u32 padding;
};

#endif
