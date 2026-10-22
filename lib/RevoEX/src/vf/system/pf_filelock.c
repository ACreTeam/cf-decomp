#include <revolution/vf/pf_filelock.h>
#include <revolution/vf/pf_system.h>

void VFiPF_InitLockFile(void) {
    VFipf_sys_set.flock_count = 0;
}

s32 VFiPF_UnLockFile(struct PF_FILE* p_file) {
    return -1;
}
