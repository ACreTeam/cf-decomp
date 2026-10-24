#ifndef PRIVATE_WDP_TYPES_H
#define PRIVATE_WDP_TYPES_H

#include <revolution/os.h>

typedef enum WDPSegType {
    WDP_SEG_TYPE_0,
    WDP_SEG_TYPE_1,
    WDP_SEG_TYPE_2,

    WDP_SEG_TYPE_NUM
}  WDPSegType;

typedef struct WDPSegmentRegion {
    WDPSegType type;
    u32 offset;
    u32 size;
} WDPSegmentRegion;

// size = 0x10
typedef struct WDPSegmentData {
    u8 raddr[4]; // 0x00
    u8 laddr[4]; // 0x04
    u8 size[4]; // 0x08
    u8 _0C_0 : 7; // 0x0C
    u8 _0C_7 : 1; // 0x0C
    u8 _0D[3]; // 0x0D
} WDPSegmentData;

typedef struct WDPSegmentInfo {
    u8 _00[4];
    u8 _04[4];
    u8 _08[4];
    WDPSegmentData data[WDP_SEG_TYPE_NUM];
    u8 _3C[0xE4 - 0x3C];
} WDPSegmentInfo;

typedef struct WDPSegmentMapper {
    u32 blockSize;
    u16 blockCount;
    u16 firstBlock[3];
    WDPSegmentInfo info;
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
