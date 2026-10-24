#ifndef PRFILE2_PF_FILELOCK_H
#define PRFILE2_PF_FILELOCK_H

#include <prfile2/pf_types.h>

#ifdef __cplusplus
extern "C" {
#endif

void PF_InitLockFile(void);
s32 PF_LockFile(struct PF_FILE* p_file, struct PF_FILE* p_owner);
s32 PF_UnLockFile(struct PF_FILE* p_file);

#ifdef __cplusplus
}
#endif
#endif
