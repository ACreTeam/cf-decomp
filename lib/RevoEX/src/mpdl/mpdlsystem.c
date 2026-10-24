#include <revolution/net/NETDigest.h>
#include <private/mp.h>
#include <private/mpdl.h>
#include <private/mpdl_types.h>
#include <private/wdp.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static const char *__MPDLVersion =
    "<< RVL_SDK - MPDL \trelease build: Aug  6 2008 15:31:39 (0x4199_60831) >>";
MPDLPlayerInfo mpdlPlayers[16];
u32 mpdlBeacon[48];
u8 mpdlPacket[0x200];
OSThread mpdlBeaconThread;
u8 mpdlBeaconStack[0x1000];
MPDLConfig *mpdlWork;
u32 mpdlSendArg;
u32 mpdlFlags;
BOOL mpdlDownloading;
u32 mpdlBootedBitmap;
u32 mpdlDownloadedBitmap;
s32 mpdlConnectedCount;
BOOL mpdlRunning;

void OSi_TPanic(const char *file, int line, const char *format, ...) {
    va_list args;
    va_start(args, format);
    OSReport("%s:%d:panic! ", file, line);
    OSVReport(format, args);
    OSPanic("mpdlsystem.c", 0x9a, "\n\n(program is terminated)");
    va_end(args);
}

void MPDLiWDPCallback(WDPParentContext *context, s32 event, u32 arg) {
    switch (event) {
    case 0: {
        u16 aid = (u16)arg;
        u16 mask = (u16)(1 << aid);
        switch (context->children[aid].status) {
        case 2:
            if (!mpdlDownloading) {
                MPDLPlayerInfo *player;
                WDP_AllowChildStatus(context, aid, 3);
                player = &mpdlPlayers[aid];
                player->playerNumber =
                    context->children[aid].player.playerNumber;
                player->favoriteColor =
                    context->children[aid].player.favoriteColor;
                player->nameLength = context->children[aid].player.nameLength;
                memcpy(player->name, context->children[aid].player.name, 20);
                player->progress = 0;
            } else {
                WDP_AllowChildStatus(context, aid, 0);
            }
            break;
        case 3:
            if (mpdlDownloading) {
                WDP_AllowChildStatus(context, aid, 4);
            }
            break;
        case 4:
            mpdlPlayers[aid].progress =
                (s32)(context->children[aid].receivedBlocks * 100) /
                context->children[aid].session->mapper.blockCount;
            break;
        case 5:
            mpdlDownloadedBitmap |= mask;
            if (mask & ~mpdlFlags) {
                mpdlPlayers[aid].progress = 100;
                WDP_AllowChildStatus(context, aid, 6);
            }
            break;
        }
        break;
    }
    case 1:
    case 2:
    case 3:
        break;
    case 8:
        mpdlDownloadedBitmap &= ~(1 << (u16)arg);
        mpdlBootedBitmap |= 1 << (u16)arg;
        break;
    }
}

BOOL WDPSegmentCallback(WDPSession *session, void *dst,
                        const WDPSegmentRegion *region) {
    const u8 *rom = (const u8 *)session->userData;
    const u8 *src = NULL;
    switch (region->type) {
    case 0:
        src = rom;
        break;
    case 1:
        src = rom + (rom[0x20] | (rom[0x21] << 8) | (rom[0x22] << 16) |
                     (rom[0x23] << 24));
        break;
    case 2:
        src = rom + (rom[0x30] | (rom[0x31] << 8) | (rom[0x32] << 16) |
                     (rom[0x33] << 24));
        break;
    default:
        break;
    }
    memcpy(dst, src + region->offset, region->size);
    return TRUE;
}

void *MPDLiSyncBeaconTask(void *arg) {
    while (mpdlRunning) {
        BOOL enabled = OSDisableInterrupts();
        s32 size = WDP_CallBeaconSendHook(mpdlWork->context, (u8 *)mpdlBeacon);
        MPSetUserGameInfo((u8 *)(mpdlBeacon + 20), size);
        MPSetEntryFlag(mpdlWork->entryFlag);
        MPiSetMultiBootFlag(mpdlWork->multiBootFlag);
        OSRestoreInterrupts(enabled);
        MPUpdateBeacon();
        OSSleepTicks(OSMillisecondsToTicks((OSTime)100));
    }
    return NULL;
}

static inline void MPDLiSendPacket(void) {
    WDPPacket packet;
    packet.data = mpdlPacket;
    packet.size = mpdlWork->mp.sendSize;
    packet.aidBitmap = 0xfffe;
    WDP_CallParentSendHook(mpdlWork->context, &packet);
    MPSendAsync(packet.data, packet.size, packet.aidBitmap, 1, &mpdlSendArg,
                MPDLiUpdatePacket, NULL);
}

inline void MPDLiUpdatePacket(s32 result, const void *data) {
    if (mpdlRunning && mpdlConnectedCount > 0) {
        MPDLiSendPacket();
    }
}

void MPDLiPortCallback(s32 event, const void *data) {
    const MPPortResult *info = (const MPPortResult *)data;
    WDPParentContext *context = mpdlWork->context;
    switch (event) {
    case 1:
        return;
    case 2:
        return;
    case 4: {
        u32 aid = info->aid;
        memcpy(mpdlPlayers[aid].macAddress, info->data.connected.macAddress, 6);
        WDP_CallParentConnectedHook(context, aid);
        ++mpdlConnectedCount;
        if (mpdlConnectedCount == 1) {
            MPDLiUpdatePacket(0, NULL);
        }
        break;
    }
    case 5: {
        u32 aid = info->aid;
        WDP_CallParentDisconnectedHook(context, aid);
        --mpdlConnectedCount;
        if (!(mpdlBootedBitmap & (1 << aid))) {
            memset(&mpdlPlayers[aid], 0, sizeof(MPDLPlayerInfo));
        }
        break;
    }
    case 3: {
        WDPPacket packet;
        packet.data = (u8 *)info->data.recv.data;
        packet.size = info->data.recv.size;
        packet.aidBitmap = 1 << info->aid;
        WDP_CallParentRecvHook(context, info->aid, &packet);
        break;
    }
    }
}

BOOL MPDLiCheckRegistration(const char *first, const char *second,
                            const void *program) {
    return FALSE;
}

void MPDLiReportProgram(const u8 *rom) {
    char key[16];
    u32 checksum = MPDLiReadLE16(rom + 0x15e);
    const char *entry;
    sprintf(key, "\n%04X:%6s", (u16)checksum, rom + 0xc);
    entry = strstr(sRegistrationList__14dNetDsDownload, key);
    if (entry) {
        u32 size = strcspn(entry + 6, "\n");
        OSReport(
            "MPDL| now starting to broadcast DS-program...\nMPDL|   <%.*s>\n",
            size, entry + 6);
    } else if (MPDLiCheckRegistration("", "", rom)) {
        OSReport("MPDL| now starting to broadcast DS-program...\nMPDL|   <%.6s "
                 "(runtime-authenticated program)>\n",
                 key + 6);
    } else {
        OSPanic("mpdlsystem.c", 0x29d,
                "MPDL| cannnot broadcast DS-program without "
                "registration-information!\n    | (please register DS-program "
                "by regds.exe)\n");
    }
}

u16 MPDLiReadLE16(const void *p) {
    return __lhbrx((void *)p, 0);
}

s32 MPDLStartup(void *config) {
    MPDLConfig *work = (MPDLConfig *)config;
    s32 result;
    WDPParentContext *context;
    if (mpdlRunning) {
        OSReport("already MPDL is running now.\n");
        result = -236;
        goto completed;
    }
    {
        MPDLiReportProgram(work->rom);
        mpdlRunning = TRUE;
        mpdlWork = work;
        work->mp.alloc = work->alloc;
        work->mp.free = work->free;
        work->mp.priority = work->priority;
        work->mp.mode = 1;
        work->mp.ggid = work->ggid;
        work->mp.tgid = work->tgid;
        work->mp.channel = work->channel;
        work->mp.lifeTime = 4000;
        work->mp.beaconPeriod = 0;
        work->mp.options = 15;
        work->mp.sendSize = 0x1f0;
        work->mp.recvSize = 8;
        work->mp.entryFlag = FALSE;
        work->mp.multiBootFlag = FALSE;
        work->mp.interval = 0;
        work->mp.gameInfoSize = 0;
        mpdlConnectedCount = 0;
        mpdlDownloadedBitmap = 0;
        mpdlBootedBitmap = 0;
        mpdlFlags = 0;
        mpdlDownloading = FALSE;
        memset(mpdlPlayers, 0, sizeof(mpdlPlayers));
        work->entryFlag = TRUE;
        work->multiBootFlag = TRUE;
        work->context =
            (WDPParentContext *)work->alloc(sizeof(WDPParentContext));
        work->session = (WDPSession *)work->alloc(sizeof(WDPSession));
        context = work->context;
        if (!context || !work->session) {
            OSReport("failed to allocate enough memory to startup MPDL.\n");
            result = -240;
        } else {
            const u8 *rom = work->rom;
            const u8 *banner = rom + NETSwapBytes32(*(const u32 *)(rom + 0x68));
            const void *auth = rom + NETSwapBytes32(*(const u32 *)(rom + 0x80));
            const void *userInfo = work->userInfo;
            const void *icon = banner + 0x20;
            const void *palette;
            if (work->icon) {
                icon = work->icon;
            }
            palette = banner + 0x220;
            if (work->palette) {
                palette = work->palette;
            }
            WDP_InitParentContext(context, MPDLiWDPCallback, work->playerName,
                                  *(u8 *)&work->favoriteColor,
                                  work->mp.sendSize);
            WDP_RegisterSession(context, work->session, icon, palette,
                                work->maxChildren, work->title,
                                work->description, work->ggid, 0, auth,
                                userInfo, rom, WDPSegmentCallback, (void *)rom);
            mpdlPlayers[0].playerNumber =
                context->children[0].player.playerNumber;
            mpdlPlayers[0].favoriteColor =
                context->children[0].player.favoriteColor;
            mpdlPlayers[0].nameLength = context->children[0].player.nameLength;
            memcpy(mpdlPlayers[0].name, context->children[0].player.name, 20);
            mpdlPlayers[0].progress = 0;
            work->mp.ports[1].callback = MPDLiPortCallback;
            work->mp.ports[1].priority = 2;
            work->mp.ports[1].arg = 0;
            result = MPStartup(&work->mp);
        }
    }
completed:
    if (result) {
        if (work->context) {
            work->free(work->context);
            work->context = NULL;
            work->free(work->session);
            work->session = NULL;
        }
        mpdlWork = NULL;
    } else {
        OSCreateThread(&mpdlBeaconThread, MPDLiSyncBeaconTask, NULL,
                       mpdlBeaconStack + sizeof(mpdlBeaconStack),
                       sizeof(mpdlBeaconStack), 5, 1);
        OSResumeThread(&mpdlBeaconThread);
    }
    return result;
}

s32 MPDLCleanup(void) {
    s32 result;
    MPDLConfig *work = mpdlWork;
    if (!mpdlRunning) {
        OSReport("MPDL is not running now.\n");
        result = -236;
    } else {
        mpdlRunning = FALSE;
        OSJoinThread(&mpdlBeaconThread, NULL);
        result = MPCleanup();
        work->free(work->context);
        work->context = NULL;
        work->free(work->session);
        work->session = NULL;
        mpdlWork = NULL;
    }
    return result;
}

static inline u32 GetEntryBitmap(void) {
    u32 bitmap = 0;
    if (mpdlRunning) {
        u32 aid;
        WDPParentContext *context = mpdlWork->context;
        for (aid = 1; aid < 16; ++aid) {
            if (context->children[aid].status >= 2 &&
                context->children[aid].status <= 6) {
                bitmap |= 1 << aid;
            }
        }
    }
    return bitmap;
}
u32 MPDLStartDownload(void) {
    u32 bitmap = 0;
    BOOL enabled = OSDisableInterrupts();
    if (!mpdlWork) {
        OSReport("MPDL is not running now.\n");
    } else {
        s32 aid;
        WDPParentContext *context = mpdlWork->context;
        bitmap = GetEntryBitmap();
        for (aid = 1; aid < 16; ++aid) {
            if (bitmap & (1 << aid)) {
                WDP_AllowChildStatus(context, aid, 4);
            }
        }
        mpdlDownloading = TRUE;
    }
    OSRestoreInterrupts(enabled);
    return bitmap;
}

BOOL MPDLGetPlayerInfo(u32 aid, MPDLPlayerInfo *info) {
    BOOL valid = FALSE;
    if (mpdlRunning) {
        s32 status = mpdlWork->context->children[aid].status;
        BOOL parent = aid == 0;
        BOOL connected = status >= 2 && status <= 6;
        BOOL booted = (mpdlBootedBitmap & (1 << aid)) != 0;
        valid = parent || connected || booted;
        if (valid) {
            *info = mpdlPlayers[aid];
        }
    }
    return valid;
}

u32 MPDLGetEntryBitmap(void) {
    return GetEntryBitmap();
}

u32 MPDLGetBootedBitmap(void) {
    return mpdlBootedBitmap;
}
