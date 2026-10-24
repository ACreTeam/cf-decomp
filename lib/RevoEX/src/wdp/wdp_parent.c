#include <revolution/net/NETDigest.h>
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

const u32 wdpSegmentFlags[4] = {0, 0, 1, 0};
const u8 wdpHeaderRegion[8] = {0, 0xfe, 0x7f, 2, 0x60, 1, 0, 0};

BOOL WDPi_SwitchActiveSession(WDPParentContext *context) {
    BOOL changed = FALSE;
    if (!context->sessions) {
        context->beaconSession = NULL;
    } else {
        WDPSession *session = context->beaconSession;
        if (session && session->next) {
            session = session->next;
        } else {
            session = context->sessions;
        }
        context->beaconSession = session;
        context->beaconPart = 0;
        context->beaconSequence = session->sequence;
        context->beaconBitmap = 1;
        changed = TRUE;
    }
    return changed;
}

void WDPi_UpdateStatus(WDPParentContext *context, u32 aid, s32 status) {
    WDPChildInfo *child = &context->children[aid];
    u32 mask = 1 << aid;
    u16 shortMask = mask;
    WDPSession *session = child->session;
    switch (child->status) {
    case 1:
        context->pendingBitmap &= ~shortMask;
        break;
    case 7:
        session->activeBitmap &= ~shortMask;
        context->activeBitmap &= ~shortMask;
        break;
    case 6:
        context->bootBitmap &= ~shortMask;
        break;
    }
    child->status = status;
    switch (status) {
    case 1:
        context->pendingBitmap |= shortMask;
        break;
    case 7:
        session->activeBitmap |= shortMask;
        context->activeBitmap |= shortMask;
        session->connectedBitmap |= mask;
        session->changedPlayers |= mask;
        session->transferBitmap |= (u16)mask;
        break;
    case 4:
        session->requestedBitmap |= mask;
        session->sendBlock = 0;
        break;
    case 6:
        context->bootBitmap |= shortMask;
        break;
    }
    if (context->callback) {
        context->callback(context, 0, aid);
    }
}

void WDP_InitParentContext(WDPParentContext *context, WDPCallback callback,
                           const u16 *name, u8 color, u32 packetSize) {
    int aid;
    context->callback = callback;
    context->packetSize = packetSize;
    context->sessions = NULL;
    context->beaconSession = NULL;
    context->controlSession = NULL;
    context->dataSession = NULL;
    context->beaconSequence = 0;
    context->connectedBitmap = 0;
    context->pendingBitmap = 0;
    context->activeBitmap = 0;
    context->rejectedBitmap = 0;
    context->bootBitmap = 0;
    context->deniedBitmap = 0;
    context->requestedSessions = 0;
    context->lockDepth = 0;
    context->children[0].player.nameLength =
        WDP_CopyString(context->children[0].player.name, sizeof(context->children[0].player.name),
                       (const u16 *)(name));
    context->children[0].player.favoriteColor = color;
    for (aid = 1; aid < 16; ++aid) {
        WDPChildInfo *child = &context->children[aid];
        WDPi_UpdateStatus((WDPParentContext *)(context), (u32)(aid), (s32)(0));
        child->session = NULL;
        MI_CpuFill8((void *)(child->request), (int)(0), (u32)(0x20));
        MI_CpuFill8((void *)(&child->receivedParts), (int)(0), (u32)(4));
        child->partCount = 0;
        child->receivedBlocks = 0;
    }
}

s32 WDP_CallBeaconSendHook(WDPParentContext *context, u8 *buffer) {
    s32 result = -1;
    WDPSession *session;
    BOOL enabled = OSDisableInterrupts();
    if (++context->lockDepth == 1) {
        context->interrupts = enabled;
    }
    for (session = context->sessions; session; session = session->next) {
        if (session->changedPlayers) {
            WDPGameInfo *game = &session->game;
            int aid;
            for (aid = 1; aid < 16; ++aid) {
                game->players[aid] = context->children[aid].player;
            }
            session->playerBitmap = session->connectedBitmap | 1;
            session->changedBitmap = session->changedPlayers;
            ++session->sequence;
            session->changedPlayers = 0;
        }
    }
    if (context->beaconSession || WDPi_SwitchActiveSession(context)) {
        u32 part = context->beaconPart;
        u32 offset;
        u32 remaining;
        u32 aid;
        u32 partCount;
        u8 *payload;
        WDPGameInfo *game;
        BOOL dataPart;
        payload = buffer + 0x50;
        session = context->beaconSession;
        offset = session->imageMode == 0 ? 0 : 0x220;
        remaining = 0x358 - offset;
        partCount = (remaining + 0x61) / 0x62;
        dataPart = part < partCount;
        game = &session->game;
        if (dataPart) {
            u32 available;
            u32 partOffset = part * 0x62;
            const u8 *src = session->gameInfo + offset;
            u8 *dst = payload + 0xe;

            u32 size = 0x62;
            available = remaining - partOffset;
            if (available < 0x62) {
                size = available;
            }
            MI_CpuCopy8(src + partOffset, dst, size);
            MI_CpuFill8(payload + 0xe + size, 0, 0x62 - size);
            payload[0xc] = size;
            payload[0xa] = context->beaconPart;
            payload[0xb] = partCount;
        } else {
            u32 pending;
            u8 *dst;
            int count;
            if (context->beaconSequence != game->header.sequence) {
                context->beaconSequence = game->header.sequence;
                context->beaconBitmap = 1;
            }
            {
                int count = (u8)MATH_CountPopulation(session->playerBitmap);
                payload[0xa] = count;
                payload[0xb] = count >> 8;
            }
            {
                int bitmap = game->playerBitmap;
                payload[0xc] = bitmap;
                payload[0xd] = bitmap >> 8;
            }
            {
                int bitmap = game->changedBitmap;
                payload[0xe] = bitmap;
                payload[0xf] = bitmap >> 8;
            }
            if (context->callback) {
                context->callback(context, 2, (u32)session);
            }
            MI_CpuCopy8(game->extraInfo, payload + 0x68, 8);
            if (context->callback) {
                context->callback(context, 3, (u32)session);
            }
            dst = payload + 0x10;
            count = 0;
            for (pending = (context->beaconBitmap ^ game->playerBitmap) & ~1U;
                 count < 4 && pending;) {
                aid = 32 - __cntlzw((pending - 1) & ~pending);
                MI_CpuCopy8(&game->players[aid], dst, sizeof(WDPPlayerInfo));
                pending &= ~(1 << aid);
                context->beaconBitmap |= 1 << aid;
                dst += sizeof(WDPPlayerInfo), ++count;
            }
            MI_CpuFill8(payload + 0x10 + count * sizeof(WDPPlayerInfo), 0,
                        (4 - count) * sizeof(WDPPlayerInfo));
        }
        *(WDPBeaconHeader *)payload = game->header;
        if (!dataPart) {
            ((WDPBeaconHeader *)payload)->imageMode = 2;
        }
        payload[0x7] = context->beaconPart++;
        {
            u16 checksum = WDPi_GetBeaconCheckSum(payload + 0xa);
            payload[0x8] = checksum;
            payload[0x9] = checksum >> 8;
        }
        if (!dataPart && context->beaconBitmap == game->playerBitmap) {
            WDPi_SwitchActiveSession(context);
        }
        result = 0x70;
    }
    if (--context->lockDepth == 0) {
        OSRestoreInterrupts(context->interrupts);
    }
    return result;
}

void WDP_CallParentConnectedHook(WDPParentContext *context, u32 aid) {
    BOOL enabled = OSDisableInterrupts();
    WDPChildInfo *child;
    if (++context->lockDepth == 1) {
        context->interrupts = enabled;
    }
    child = &context->children[aid];
    WDPi_UpdateStatus((WDPParentContext *)(context), (u32)(aid), (s32)(0));
    child->session = NULL;
    MI_CpuFill8((void *)(child->request), (int)(0), (u32)(0x20));
    MI_CpuFill8((void *)(&child->receivedParts), (int)(0), (u32)(4));
    child->partCount = 0;
    child->receivedBlocks = 0;
    context->connectedBitmap |= 1 << aid;
    WDPi_UpdateStatus((WDPParentContext *)(context), (u32)(aid), (s32)(1));
    if (--context->lockDepth == 0) {
        OSRestoreInterrupts(context->interrupts);
    }
}

static inline WDPChildInfo* WDP_GetChildInfo(WDPParentContext *context, u32 aid) {
    return &context->children[aid];
}

void WDP_CallParentDisconnectedHook(WDPParentContext *context, u32 aid) {
    u32 mask = 1 << aid;
    u32 inverse;
    WDPChildInfo *child = WDP_GetChildInfo(context, aid);
    BOOL enabled = OSDisableInterrupts();
    WDPSession *session;
    if (++context->lockDepth == 1) {
        context->interrupts = enabled;
    }

    inverse = ~mask;
    context->rejectedBitmap &= inverse;
    context->deniedBitmap &= inverse;

    MI_CpuFill8((void *)(&child->player), (int)(0),
                (u32)(sizeof(WDPPlayerInfo)));
    session = child->session;
    if (session) {
        session->connectedBitmap &= inverse;
        session->transferBitmap &= inverse;
        session->requestedBitmap &= inverse;
        session->changedPlayers |= mask;
    }
    if (context->children[aid].status == 6 && context->callback) {
        context->callback(context, 8, aid);
    }
    WDPi_UpdateStatus((WDPParentContext *)(context), (u32)(aid), (s32)(0));
    child->session = NULL;
    MI_CpuFill8((void *)(child->request), (int)(0), (u32)(0x20));
    MI_CpuFill8((void *)(&child->receivedParts), (int)(0), (u32)(4));
    child->partCount = 0;
    child->receivedBlocks = 0;
    if (--context->lockDepth == 0) {
        OSRestoreInterrupts(context->interrupts);
    }
}

void WDP_CallParentSendHook(WDPParentContext *context, WDPPacket *packet) {
    WDPChildInfo *child;
    u16 activeBitmap;
    u32 aid;
    u32 sessionMask;
    u8 *data = packet->data;
    BOOL enabled = OSDisableInterrupts();
    if (++context->lockDepth == 1) {
        context->interrupts = enabled;
    }
    *data = 0;
    packet->size = 1;
    packet->aidBitmap = 0xffff;
    if (context->bootBitmap) {
        *data = 5;
        packet->aidBitmap = context->bootBitmap;
    } else if (context->pendingBitmap) {
        *data = 1;
        packet->aidBitmap = context->pendingBitmap;
    } else if (context->deniedBitmap) {
        *data = 6;
        packet->aidBitmap = context->deniedBitmap;
    } else if (context->rejectedBitmap) {
        *data = 2;
        packet->aidBitmap = context->rejectedBitmap;
    } else if ((activeBitmap = context->activeBitmap)) {
        WDPSession *session = context->controlSession ? context->controlSession
                                                      : context->sessions;
        sessionMask = 0;
        child = &context->children[1];
        for (aid = 1; aid < 16; ++child, ++aid) {
            if (activeBitmap & (1 << aid)) {
                sessionMask = 1 << child->reserved[2];
                break;
            }
        }
        if (session) {
            do {
                session = session->next ? session->next : context->sessions;
            } while (!(sessionMask & (1 << (session->sessionIndex))));
            context->controlSession = session;
            *data = 3;
            MI_CpuCopy8(&session->mapper.info, data + 1, sizeof(session->mapper.info));
            packet->size += sizeof(session->mapper.info);
            packet->aidBitmap = session->activeBitmap;
        }
    } else {
        WDPSession *first =
            context->dataSession ? context->dataSession : context->sessions;
        WDPSession *session = first;
        if (session) {
            do {
                session = session->next ? session->next : context->sessions;
            } while (!session->requestedBitmap && session != first);
            if (session->requestedBitmap) {
                WDPSegmentRegion region;
                u16 block;
                context->dataSession = session;
                if (context->requestedSessions &
                    (1 << (session->sessionIndex))) {
                    s32 delta = session->sendBlock - session->ackBlock;
                    if (delta >= 0 && delta <= 2) {
                        ++session->sendBlock;
                        if (session->sendBlock >= session->mapper.blockCount) {
                            session->sendBlock = session->ackBlock;
                        }
                    } else {
                        session->sendBlock = session->ackBlock;
                    }
                }
                block = session->sendBlock;
                if (WDPi_GetSegmentRegion(&session->mapper, block, &region) &&
                    session->readSegment(session, data + 5, &region)) {
                    data[0] = 4;
                    {
                        int sessionIndex = session->sessionIndex;
                        data[1] = sessionIndex;
                        data[2] = sessionIndex >> 8;
                    }
                    data[3] = block;
                    data[4] = block >> 8;
                    packet->size += region.size + 4;
                    packet->aidBitmap = session->requestedBitmap;
                }
            }
        }
    }
    packet->size = (packet->size + 1) & ~1;
    {
        WDPSession *session;
        for (session = context->sessions; session; session = session->next) {
            session->ackBlock = 0;
        }
    }
    context->requestedSessions = 0;
    if (--context->lockDepth == 0) {
        OSRestoreInterrupts(context->interrupts);
    }
}

void WDP_CallParentRecvHook(WDPParentContext *context, u32 aid,
                            const WDPPacket *packet) {
    WDPChildInfo *child = &context->children[aid];
    const u8 *data = packet->data;
    s32 status = child->status;
    BOOL enabled = OSDisableInterrupts();
    if (++context->lockDepth == 1) {
        context->interrupts = enabled;
    }
    switch (data[0]) {
    case 7:
        if (status == 1) {
            u32 part = data[1];
            if (WDP_MergeBitset(&child->receivedParts, 5, part)) {
                u32 offset = part * 6;
                void *dst = child->request + offset;
                const void *src = data + 2;
                u32 size = 6;
                u32 remaining = 30 - offset;
                if (remaining < 6) {
                    size = remaining;
                }
                MI_CpuCopy8(src, dst, size);
                if (++child->partCount >= 5) {
                    WDPSession *session;
                    for (session = context->sessions;
                         session &&
                         (WDP_CompareMemory(session->ggid, child->request, 4) ||
                          session->sessionIndex != child->reserved[2]);
                         session = session->next) {
                    }
                    child->session = session;
                    child->player.playerNumber = aid;
                    if (!session) {
                        u32 event[2];
                        event[0] = aid;
                        event[1] = 5;
                        context->rejectedBitmap |= 1 << aid;
                        context->pendingBitmap &= ~(1 << aid);
                        if (context->callback) {
                            context->callback(context, 1, (u32)event);
                        }
                    } else if ((u8)MATH_CountPopulation(
                                   session->connectedBitmap) >=
                               session->gameInfo[0x236]) {
                        u32 event[2];
                        event[0] = aid;
                        event[1] = 6;
                        context->deniedBitmap |= 1 << aid;
                        context->pendingBitmap &= ~(1 << aid);
                        if (context->callback) {
                            context->callback(context, 1, (u32)event);
                        }
                    } else {
                        WDPi_UpdateStatus(context, aid, 2);
                    }
                }
            }
        }
        break;
    case 8:
        if (status == 7) {
            WDPi_UpdateStatus(context, aid, 3);
        }
        break;
    case 9:
        if (status == 4 && child->session) {
            WDPSession *session = child->session;
            u32 block = data[1] | (data[2] << 8);
            child->receivedBlocks = data[3] | (data[4] << 8);
            if (session->ackBlock < block) {
                session->ackBlock = block;
            }
            context->requestedSessions |= 1 << (session->sessionIndex);
            if (context->callback) {
                context->callback(context, 0, aid);
            }
        }
        break;
    case 10:
        if (status == 4) {
            WDPSession *session = child->session;
            session->requestedBitmap &= ~(1 << aid);
            child->receivedBlocks = session->mapper.blockCount;
            WDPi_UpdateStatus(context, aid, 5);
        }
        break;
    }
    if (--context->lockDepth == 0) {
        OSRestoreInterrupts(context->interrupts);
    }
}

static inline void WriteLE32(u8 *data, u32 value) {
    data[0] = value;
    data[1] = value >> 8;
    data[2] = value >> 16;
    data[3] = value >> 24;
}

void WDP_RegisterSession(WDPParentContext *context, WDPSession *session,
                         const void *icon, const void *palette, u32 maxChildren,
                         const u16 *title, const u16 *description, u32 ggid,
                         u32 index, const void *auth, const void *userInfo,
                         const u8 *rom, WDPReadSegment readSegment,
                         void *userData) {
    WDPSegmentInfo *info = &session->mapper.info;
    WDPSegmentData *data;
    BOOL enabled = OSDisableInterrupts();
    const u8 *regions[3];
    int i;
    WDPSession **link;
    if (++context->lockDepth == 1) {
        context->interrupts = enabled;
    }
    WriteLE32(info->_00, NETSwapBytes32(*(const u32 *)(rom + 0x24)));
    WriteLE32(info->_04, NETSwapBytes32(*(const u32 *)(rom + 0x34)));
    MI_CpuFill8(info->_08, 0, 4);
    MI_CpuCopy8(auth, (u8 *)session + 0x524, 0x88);
    MI_CpuCopy8(userInfo, (u8 *)session + 0x5ac, 0x20);
    regions[0] = wdpHeaderRegion;
    regions[1] = rom + 0x28;
    regions[2] = rom + 0x38;
    for (i = 0; i < WDP_SEG_TYPE_NUM; ++i) {
        const u8 *region = regions[i];
        u32 address;
        u32 size;
        BOOL relocate;

        data = &info->data[i];
        address = MI_GetLE32((void *)region);
        size = MI_GetLE32((void *)(region + 4));
        relocate = FALSE;
        if (!WDPi_IsValidSegmentRegion(i, address, size, TRUE, &relocate)) {
            OSi_TPanic("wdp_parent.c", 0x39a,
                       "invalid segment region : %08X[%08X]\n", address, size);
        }
        WriteLE32(info->data[i].size, size);
        WriteLE32(info->data[i].laddr, address);
        WriteLE32(info->data[i].raddr, address);
        MI_CpuFill8(info->data[i]._0D, 0, sizeof(info->data[i]._0D));
        info->data[i]._0C_7 = wdpSegmentFlags[i];
        info->data[i]._0C_0 = 0;
        if (relocate) {
            WriteLE32(info->data[i].raddr, 0x022C0000);
        }
    }
    if (!WDPi_InitSegmentMapper(&session->mapper, info,
                                context->packetSize)) {
        OSi_TPanic("wdp_parent.c", 0x3ad,
                   "failed to create block-transfer information.\n");
    } else {
        BOOL noImage = FALSE;
        WriteLE32(session->ggid, ggid);
        session->sessionIndex = index;
        session->reserved = 0;
        session->sequence = 0;
        if (!icon || !palette) {
            noImage = TRUE;
        }
        session->imageMode = noImage;
        if (noImage) {
            MI_CpuFill8(session->gameInfo, 0, 0x220);
        } else {
            MI_CpuCopy8(icon, session->gameInfo + 0x20, 0x200);
            MI_CpuCopy8(palette, session->gameInfo, 0x20);
        }
        *(WDPPlayerInfo *)(session->gameInfo + 0x220) =
            context->children[0].player;
        session->gameInfo[0x236] = maxChildren;
        WDP_CopyString((u16*)(session->gameInfo + 0x238), 0x60, title);
        WDP_CopyString((u16*)(session->gameInfo + 0x298), 0xc0, description);
        MI_CpuFill8(session->players, 0, sizeof(session->players));
        MI_CpuFill8(session->extraInfo, 0, sizeof(session->extraInfo));
        session->changedBitmap = 0;
        session->userData = userData;
        session->readSegment = readSegment;
        session->playerBitmap = 1;
        session->connectedBitmap = 1;
        session->transferBitmap = 0;
        session->activeBitmap = 0;
        for (link = &context->sessions; *link; link = &(*link)->next) {
        }
        *link = session;
        session->next = NULL;
    }
    if (--context->lockDepth == 0) {
        OSRestoreInterrupts(context->interrupts);
    }
}

BOOL WDP_AllowChildStatus(WDPParentContext *context, u32 aid, s32 status) {
    BOOL result = FALSE;
    BOOL enabled = OSDisableInterrupts();
    if (++context->lockDepth == 1) {
        context->interrupts = enabled;
    }
    switch (context->children[aid].status) {
    case 2:
        if (status == 3) {
            WDPi_UpdateStatus(context, aid, 7);
            result = TRUE;
        } else if (status == 0) {
            u32 event[2];
            event[0] = aid;
            event[1] = 6;
            context->rejectedBitmap |= 1 << aid;
            context->pendingBitmap &= ~(1 << aid);
            if (context->callback) {
                context->callback(context, 1, (u32)event);
            }
            result = TRUE;
        }
        break;
    case 3:
        if (status == 4) {
            WDPi_UpdateStatus(context, aid, 4);
            result = TRUE;
        }
        break;
    case 5:
        if (status == 6) {
            WDPi_UpdateStatus(context, aid, 6);
            result = TRUE;
        }
        break;
    }
    if (--context->lockDepth == 0) {
        OSRestoreInterrupts(context->interrupts);
    }
    return result;
}
