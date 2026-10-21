#pragma once

#include <gamespy/gt2/gt2.h>
#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef GT2Result (*DWCReliableSendFunc)(GT2Connection connection,
                                         const GT2Byte *message, int len,
                                         GT2Bool reliable);

void DWCi_InitReliableQueue(void);
void DWCi_CleanupReliableQueue(void);
BOOL DWCi_IsReliableQueueFree(void);
BOOL DWCi_ProcessReliableQueue(void);
BOOL DWCi_ProcessReliableQueueEx(DWCReliableSendFunc func);
BOOL DWCi_InsertReliableQueue(GT2Connection connection, const void *buffer,
                              int size);

#ifdef __cplusplus
}
#endif
