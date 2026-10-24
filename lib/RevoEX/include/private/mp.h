#ifndef PRIVATE_MP_H
#define PRIVATE_MP_H

#include <revolution/os.h>
#include <revolution/mp.h>
#include <private/mp_types.h>

typedef s32 MPGenericCallback();

extern u32 mpFatalCommand[8];
extern u32 mpReceiveBuffer[40];
extern u32 mpNotificationBuffer[40];
extern const char *__MPVersion;
extern s32 mpWirelessLock;
extern BOOL mpCheckMem2;
extern BOOL mpInitialized;
extern s32 mpFatalError;
extern BOOL mpVersionRegistered;
extern u32 mpWork;
#define MPi_Work ((MPWork *)mpWork)
extern const u8 mpEmptyMacAddress[6];

u32 MPiStartupBeacon(void);
void MPiCleanupBeacon(void);
void MPiProcessBeaconSentNotification(void);
void MPiMakeGameInfo(u16 *buffer);
s32 MPiMakeAndChangeBeacon(void);
void MPiSetCurrentTsf(u16 tsf);
s32 MPiGetVTsf(void);
s32 MPiUpdateBeacon(u32 param_1, u32 param_2, int param_3);
u32 MPiStartupConnection(void);
void MPiCleanupConnection(void);
int MPiDisconnect(u32 aid, int expired, int force);
void MPiProcessConnectedNotification(u32 aid, void *address, u32 gameInfo,
                                     u32 size);
void MPiProcessDisconnectedNotification(u32 aid, u32 reason, u32 address);
void MPiAddRssi(int aid, u32 rssi);
void MPiUpdateLifeTime(int aid);
u32 MPiCompareLifeTime(int aid, int tick);
void MPiProcessLifeTimeExpiration(u32 param_1);
void MPiProcessForcibleLifeTimeExpiration(u32 param_1);
void MPiInitList(MPListNode *list, MPListNode *nodes, u32 count);
void MPiInsertList(int *position, int *node);
int *MPiRemoveList(int *address);
u32 MPiSetMultiBootFlag(int flag);
u32 MPiStartupNotification(void);
s32 MPiStartupNotificationLater(void);
void MPiCleanupNotificationEarlier(void);
void MPiCleanupNotification(void);
u32 HandleNotification(void);
u32 MPiStartupParent(void);
u32 MPiStartupParentLater(void);
void MPiCleanupParentEarlier(void);
void MPiCleanupParent(void);
void MPiProcessParentDataSendError(void);
u32 HandleMPEndIndication(void);
u32 HandleMPSequence(void);
void CheckLifeTime(void);
u32 ParseMPEndIndication(int address, u32 size, u32 aidBitmap);
u32 GetTmptt(int sendSize, u32 recvSize, u32 aidBitmap);
void IntervalAlarmHandler(void);
void PictureFrameAlarmHandler(void);
void ParentPeriodicAlarmHandler(void);
u32 MPiStartupPort(void);
u32 MPiStartupPortLater(void);
void MPiCleanupPort(void);
void MPiResetPortByAid(int aid);
void MPiNotifyDataSentToPort(u32 status, int address);
u32 MPiNotifyDataReceivedToPort(u32 port, u32 aid, u32 data, u32 size,
                                u32 sequence);
void MPiNotifyConnectedToPort(u32 aid, void *macAddress, void *gameInfo,
                              u32 frameSize);
void MPiNotifyDisconnectedToPort(u32 aid, u32 reason, void *macAddress);
void MPiHandlePortCallback(int *address);
void *MPiCallbackThread(void *arg);
u32 MPiUnlockWirelessDriver(void);
s32 MPiStartupSystem(const MPConfig *config);
s32 MPiAllocateCommandBlockFromQueue(OSMessageQueue *queue, MPCommand **out,
                                     s32 flags);
void MPiFreeCommandBlockToQueue(u32 param_1, u32 param_2);
s32 MPiAllocateCommandBlock(MPCommand **out, s32 flags);
void MPiFreeCommandBlock(u32 param_1);
u32 MPiSendRequest(u32 param_1, int param_2, int param_3);
void MPiRespondToWaitingRequest(int param_1, int param_2);
void MPiNotifyCallback(s32 result, MPCommand *command);
BOOL MPiIsEqualMacAddress(const u8 *a, const u8 *b);
u32 MPiGetNewTgid(void);
u32 MPiGetNewBeaconPeriod(void);
u32 MPiCountPopulation(u32 value);
s32 MPiSendCallback(u32 message);
void MPiSetFatalError(u32 param_1);
u32 MPiStartupTxRx(void);
u32 MPiStartupTxRxLater(void);
void MPiCleanupTxRxEarlier(void);
void MPiCleanupTxRx(void);
int MPiSend(u32 data, u32 size, u32 port, u32 aidBitmap, u32 sendResult,
            u32 callback, u32 arg, int synchronous);
void MPiParseReceivedFrame(int aid, u32 header, u16 *data, u32 size);
void MPiProcessDataSentNotification(u32 aidBitmap);
void MPiClearSendQueue(void);
void MPiConsumeSendQueue(u32 size);
void MPiPrepareSendQueue(void);
u32 MPiMakeSendFrame(u16 *frame, u32 capacity, int address);
int MPiBuildSendFrame(u32 address, u32 capacity, u32 aidBitmap, int last);
s32 MPiStartupWD(void);
void MPiCleanupWD(void);
s32 MPiSendFrame(u16 *frame, u32 size, void *result);
s32 MPiReceiveFrame(void *frame, u16 size);
s32 MPiChangeBeacon(void *beacon);
s32 MPiChangeVTSF(void);
s32 MPiDisAssoc(const u8 *mac);
s32 StartParent(void);
void MI_CpuCopy8(const void *src, void *dst, u32 size);
u32 MATH_CountPopulation(u32 value);

#endif
