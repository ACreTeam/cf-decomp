#ifndef PRIVATE_NWC24_DOWNLOAD_H
#define PRIVATE_NWC24_DOWNLOAD_H

#include <revolution/types.h>

#include <revolution/nand.h>

#include <revolution/nwc24/NWC24Dl.h>
#include <revolution/nwc24/NWC24Err.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NWC24iDlEntry {
    u32 app;
    u32 nextTime;
    u32 lastAccess;
    u8 flags;
    char unk_0xD[0x10 - 0xD];
} NWC24iDlEntry;

typedef struct NWC24iDlHeader {
    u32 magic;
    u32 version;
    char unk_0x8[0x10 - 0x8];
    u16 maxSubTasks;
    u16 privateTasks;
    u16 maxTasks;
    char unk_0x16[0x80 - 0x16];
    NWC24iDlEntry entries[NWC24_DL_TASK_MAX];
} PACKED NWC24iDlHeader;

typedef struct NWC24iDlTask {
    u16 id;
    u8 type;
    u8 priority;
    u32 flags;
    u32 appId;
    u32 titleIdHi;
    u32 titleIdLo;
    u16 groupId;
    char unk_0x16[0x2];
    s16 count;
    s16 errorCount;
    u16 interval;
    u16 margin;
    u32 lastError;
    u8 subTaskCounter;
    u8 subTaskType;
    u8 subTaskFlags;
    char unk_0x27[0x1];
    u32 subTaskMask;
    u32 serverInterval;
    u32 lastUpdate;
    u32 lastUpdateSubTask[NWC24_DL_SUBTASK_MAX];
    char url[236];
    char fileName[NAND_MAX_PATH];
    char unk_0x1E0[0x1F8 - 0x1E0];
    u32 userParam;
    u8 optFlags;
    u8 rootCaId;
    char unk_0x1FE[0x200 - 0x1FE];
} PACKED NWC24iDlTask;

NWC24Err NWC24iOpenDlTaskList();
NWC24Err NWC24iCloseDlTaskList();
NWC24Err NWC24iGetDlTitleDir(const NWC24DlTask* pTask, char* pBuf, u32 size);
NWC24iDlHeader* NWC24iGetCachedDlHeader();
NWC24Err NWC24iCheckDlHeaderConsistency(NWC24iDlHeader* pHeader, BOOL clear) NO_INLINE;
NWC24Err NWC24iLoadDlHeader();

#ifdef __cplusplus
}
#endif

#endif  // PRIVATE_NWC24_DOWNLOAD_H
