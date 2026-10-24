#ifndef PRFILE2_PF_SYSTEM_H
#define PRFILE2_PF_SYSTEM_H

// System state of the standalone PrFILE2: the stub task that runs every API call (lib/prfile2/system/pf_stub.c).

#include <revolution/os.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PF_STUB_STACK_SIZE 0x2000
#define PF_STUB_PRIORITY 16
#define PF_STUB_MSG_NUM 20
#define PF_FLOCK_COUNT_MAX 0xFFFFFFFF

typedef struct PF_SYSTEM_SET {
    s32 initialized;                 // 0x0000
    s32 tsk_id;                      // 0x0004
    s32 mbx_id;                      // 0x0008
    u8 stack[PF_STUB_STACK_SIZE];    // 0x000C
    OSThread thread;                 // 0x2010
    OSMessageQueue queue;            // 0x2328
    OSMessage msgs[PF_STUB_MSG_NUM]; // 0x2348
    u32 flock_count;                 // 0x2398
} PF_SYSTEM_SET;

typedef struct PF_SYS_DATE {
    u32 sys_year;   // 0x00
    u32 sys_month;  // 0x04
    u32 sys_day;    // 0x08
} PF_SYS_DATE;

typedef struct PF_SYS_TIME {
    u32 sys_hour;  // 0x00
    u32 sys_min;   // 0x04
    u32 sys_sec;   // 0x08
    u32 sys_ms;    // 0x0C
} PF_SYS_TIME;

extern PF_SYSTEM_SET pf_sys_set;

void PFSYS_initializeSYS(void);
s32 PFSYS_GetCurrentContextID(s32* context_id);
void PFSYS_TimeStamp(PF_SYS_DATE* sdate, PF_SYS_TIME* stime);

#ifdef __cplusplus
}
#endif
#endif
