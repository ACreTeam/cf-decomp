#ifndef PRIVATE_DVD_H
#define PRIVATE_DVD_H

#include <revolution/types.h>

#include <revolution/dvd.h>

#include <revolution/os/OSAlarm.h>

#include <revolution/esp.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DVD_DEVICE_CODE (1 << 15)

#define DVD_RESETCOVER_TIMELAG_TICKS2 OSMillisecondsToTicks(100)

#define DVD_PRIVATE_PAD32(size) u8 CONCAT(padding, __LINE__)[OSRoundUp32B((size)) - (size)]

enum {
    DVD_INTTYPE_TC = (1 << 0),
    DVD_INTTYPE_DE = (1 << 1),
    DVD_INTTYPE_CVR = (1 << 2),
    DVD_INTTYPE_BR = (1 << 3),
    DVD_INTTYPE_TIME = (1 << 4),
    DVD_INTTYPE_SERR = (1 << 5),
    DVD_INTTYPE_VERR = (1 << 6),
    DVD_INTTYPE_ARGS = (1 << 7),
};

enum {
    DVD_COVER_BUSY = 0,
    DVD_COVER_OPENED,
    DVD_COVER_CLOSED,
};

#define DVD_DICVR_CVR (1 << 0)
#define DVD_DICVR_CVRINTMASK (1 << 1)
#define DVD_DICVR_CVRINT (1 << 2)

typedef struct DVDVideoReportKey {
    u8 data[32];
} DVDVideoReportKey;

typedef struct DVDVideoPhysical {
    u8 data[2048];
} DVDVideoPhysical;

typedef struct DVDVideoDiscKey {
    u8 data[2048];
} DVDVideoDiscKey;

typedef struct DVDDiskBca {
    u8 optionalInfo[52];
    u8 manufacturerCode[2];
    u8 recorderDeviceCode[2];
    u8 APMRecorderDeviceCode[1];
    u8 discManufactureDate[2];
    u8 discManufactureTime[2];
    u8 discNumber[3];
} DVDDiskBca;

typedef struct DVDLowDriveSer {
    u8 data[12];
    u8 padding[20];
} DVDLowDriveSer;

/* DI IOS command/register blocks */

typedef struct diRegVals {
    u32 ImmRegVal;
    u32 CoverRegVal;
    u32 pad[6];
} diRegVals_t;

typedef struct diCommand {
    u8 theCommand;
    u8 pad1[3];
    u32 arg[5];
    u32 pad2[2];
} diCommand_t;

typedef struct DVDAppLoaderHeader {
    char appLoaderDate[16];      // 0x00
    u32 appLoaderFunc1;          // 0x10
    u32 appLoaderLength;         // 0x14
    u32 appLoaderTrailerLength;  // 0x18
    u8 pad_0x1C[4];
} DVDAppLoaderHeader;

typedef struct DVDGamePartition {
    ESTicket ticket;
    u32 tmdSize;
    ESTitleMeta* tmd;
    u32 certBlobSize;
    void* certBlob;
    u8* h3Hashes;
    u8* encryptedArea;
} DVDGamePartition;

typedef struct DVDPartitionInfo {
    DVDGamePartition* gamePartition;  // 0x00
    u32 type;             // 0x04
} DVDPartitionInfo;

typedef struct DVDGameTOC {
    u32 numGamePartitions;             // 0x00
    DVDPartitionInfo* partitionInfos;  // 0x04
} DVDGameTOC;

typedef struct DVDPartitionParams {
    ESTicket ticket;
    DVD_PRIVATE_PAD32(sizeof(ESTicket));
    ESTicketView ticketView;
    DVD_PRIVATE_PAD32(sizeof(ESTicketView));

    u32 numTmdBytes;
    DVD_PRIVATE_PAD32(sizeof(u32));
    ESTitleMeta tmd;
    DVD_PRIVATE_PAD32(sizeof(ESTitleMeta));

    u32 numCertBytes;
    DVD_PRIVATE_PAD32(sizeof(u32));
    u8 certificates[0x1000];

    u32 dataWordOffset;
    DVD_PRIVATE_PAD32(sizeof(u32));
    u8 h3Hash[0x18000];
} DVDPartitionParams;

typedef struct DVDBB2 {
    u32 bootFilePosition;  // 0x00

    u32 FSTPosition;   // 0x04
    u32 FSTLength;     // 0x08
    u32 FSTMaxLength;  // 0x0C
    void* FSTAddress;  // 0x10

    u32 userPosition;  // 0x14
    u32 userLength;    // 0x18

    u32 pad_0x1C;
} DVDBB2;

/* Low level broadway stuff */

bool DVDLowFinalize();
bool DVDLowInit();

bool DVDLowReadDiskID(DVDDiskID* diskID, DVDLowCallback callback);
bool DVDLowReportKey(DVDVideoReportKey* reportKey, u32 format, u32 lsn, DVDLowCallback callback);

bool DVDLowOpenPartition(u32 partitionWordOffset, const ESTicket* eTicket, u32 numCertBytes, const u8* certificates, ESTitleMeta* tmd, DVDLowCallback callback);
bool DVDLowOpenPartitionWithTmdAndTicketView(u32 partitionWordOffset, const ESTicketView* eTicketView, u32 numTmdBytes, const ESTitleMeta* tmd, u32 numCertBytes,
                                             const u8* certificates, DVDLowCallback callback);

bool DVDLowGetNoDiscBufferSizes(const u32 partitionWordOffset, u32* numTmdBytes, u32* numCertBytes, DVDLowCallback callback);
bool DVDLowGetNoDiscOpenPartitionParams(const u32 partitionWordOffset, ESTicket* eTicket, u32* numTmdBytes, ESTitleMeta* tmd, u32* numCertBytes,
                                        u8* certificates, u32* dataWordOffset, u8* h3HashPtr, DVDLowCallback callback);

bool DVDLowClosePartition(DVDLowCallback callback);
bool DVDLowUnencryptedRead(void* destAddr, u32 length, u32 wordOffset, DVDLowCallback callback);

bool DVDLowStopMotor(bool eject, bool saving, DVDLowCallback callback);
bool DVDLowInquiry(DVDDriveInfo* info, DVDLowCallback callback);

bool DVDLowRequestError(DVDLowCallback callback);

bool DVDLowSetSpinupFlag(u32 spinUp);

bool DVDLowReset(DVDLowCallback callback);

bool DVDLowAudioBufferConfig(BOOL enable, u32 size, DVDLowCallback callback);
bool DVDLowRequestAudioStatus(u32 subcmd, DVDLowCallback callback);
bool DVDLowAudioStream(u32 subcmd, u32 length, u32 wordOffset, DVDLowCallback callback);
bool DVDLowSetMaximumRotation(u32 subcmd, DVDLowCallback callback);

bool DVDLowRead(void* destAddr, u32 length, u32 wordOffset, DVDLowCallback callback);
bool DVDLowSeek(u32 wordOffset, DVDLowCallback callback);

u32 DVDLowGetCoverRegister();
u32 DVDLowGetStatusRegister();
bool DVDLowPrepareCoverRegister(DVDLowCallback callback);
bool DVDLowPrepareStatusRegister(DVDLowCallback callback);

u32 DVDLowGetImmBufferReg();

bool DVDLowUnmaskStatusInterrupts();
bool DVDLowMaskCoverInterrupt();
bool DVDLowClearCoverInterrupt(DVDLowCallback callback);

s32 DVDLowGetLastEticketError();

BOOL __DVDLowTestAlarm(const OSAlarm* alarm);

/* Main stuff */

extern DVDCommandBlock __DVDStopMotorCommandBlock;
extern OSThreadQueue __DVDThreadQueue;
extern DVDErrorInfo __ErrorInfo;

void __DVDAudioBufferConfig(DVDCommandBlock* block, u32 enable, u32 size, DVDCommandCallback callback);

s32 __DVDGetDriveStatus();
BOOL __DVDCheckDevice(void);
extern volatile u32 __DVDLayoutFormat;
u32 __DVDGetCoverStatus();

BOOL __DVDGetCoverStatusAsync(DVDCommandBlock* block, DVDCommandCallback callback);

void __DVDPrepareResetAsync(DVDCommandCallback callback);
void __DVDPrepareReset();
void __DVDResetWithNoSpinup();

BOOL __DVDTestAlarm(const OSAlarm* alarm);

BOOL __DVDStopMotorAsync(DVDCommandBlock* block, DVDCommandCallback callback);
void __DVDRestartMotor();

BOOL DVDOpenPartitionAsync(DVDCommandBlock* block, void* addr, u32 offset, DVDCommandCallback callback);
BOOL DVDClosePartitionAsync(DVDCommandBlock* block, DVDCommandCallback callback);
BOOL DVDUnencryptedReadAbsAsyncForBS(DVDCommandBlock* block, void* addr, s32 length, u32 offset, DVDCommandCallback callback);
BOOL DVDDownRotationAsync(DVDCommandBlock* block, DVDCommandCallback callback);
BOOL DVDGetPartitionParamsAsync(DVDCommandBlock* block, DVDPartitionParams* params, u32 offset, DVDCommandCallback callback);
BOOL DVDOpenPartitionWithParamsAsync(DVDCommandBlock* block, DVDPartitionParams* params, u32 offset, DVDCommandCallback callback);

/* Error stuff */

void __DVDStoreErrorCode(u32 error, DVDCommandCallback callback);

/* File system stuff */

void __DVDFSInit();

/* Fatal stuff */

void __DVDShowFatalMessage();
void __DVDShowFatalMessageForSystem(u32 command, u32 intType);
void __DVDPrintFatalMessage();
BOOL __DVDGetAutoFatalMessaging();

/* Queue stuff */

void __DVDClearWaitingQueue();

DVDCommandBlock* __DVDPopWaitingQueue();
BOOL __DVDPushWaitingQueue(s32 prio, DVDCommandBlock* block);
BOOL __DVDDequeueWaitingQueue(DVDCommandBlock* block);

BOOL __DVDCheckWaitingQueue();

DVDCommandBlock* __DVDGetNextWaitingQueue();

#ifdef __cplusplus
}
#endif

#endif  // PRIVATE_DVD_H
