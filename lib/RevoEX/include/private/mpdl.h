#ifndef PRIVATE_MPDL_H
#define PRIVATE_MPDL_H

#include <revolution/os.h>
#include <revolution/mpdl.h>
#include <private/mpdl_types.h>

extern const char *sRegistrationList__14dNetDsDownload;

extern MPDLPlayerInfo mpdlPlayers[16];
extern u32 mpdlBeacon[48];
extern u8 mpdlPacket[0x200];
extern OSThread mpdlBeaconThread;
extern u8 mpdlBeaconStack[0x1000];
extern MPDLConfig *mpdlWork;
extern u32 mpdlSendArg;
extern u32 mpdlFlags;
extern BOOL mpdlDownloading;
extern u32 mpdlBootedBitmap;
extern u32 mpdlDownloadedBitmap;
extern s32 mpdlConnectedCount;
extern BOOL mpdlRunning;

void OSi_TPanic(const char *file, int line, const char *format, ...);
void MPDLiWDPCallback(WDPParentContext *context, s32 event, u32 arg);
BOOL WDPSegmentCallback(WDPSession *session, void *dst,
                        const WDPSegmentRegion *region);
void *MPDLiSyncBeaconTask(void *arg);
void MPDLiUpdatePacket(s32 result, const void *data);
void MPDLiPortCallback(s32 event, const void *data);
BOOL MPDLiCheckRegistration(const char *first, const char *second,
                            const void *program);
void MPDLiReportProgram(const u8 *rom);
u16 MPDLiReadLE16(const void *p);

#endif
