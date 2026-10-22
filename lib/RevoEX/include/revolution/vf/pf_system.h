#ifndef RVL_SDK_VF_PF_SYSTEM_H
#define RVL_SDK_VF_PF_SYSTEM_H
#include <revolution/types.h>
#ifdef __cplusplus
extern "C" {
#endif

extern struct PF_SYSTEM_SET VFipf_sys_set;

void VFiPFSYS_initializeSYS();
s32 VFiPFSYS_GetCurrentContextID(s32* context_id);
void VFiPFSYS_TimeStamp(struct PF_SYS_DATE* sdate, struct PF_SYS_TIME* stime);

#ifdef __cplusplus
}
#endif
#endif
