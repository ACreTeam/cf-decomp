#include <prfile2/pf_filelock.h>
#include <prfile2/pf_system.h>
#include <prfile2/pfk_api.h>

void PF_InitLockFile(void) {
    pf_sys_set.flock_count = 0;
}

s32 PF_LockFile(PF_FILE* p_file, PF_FILE* p_owner) {
    s32 err = PF_RET_ERR;

    if (p_file->p_sfd->lock.count == 0 && p_file->p_sfd->lock.resource == 0) {
        if (pf_sys_set.flock_count < PF_FLOCK_COUNT_MAX) {
            PFK_SEMAPHORE_INFO info;

            info.initial = 1;
            info.max = 1;
            info.sem = &p_file->p_sfd->lock_sem;
            p_file->p_sfd->lock.resource = pfk_create_semaphore(&info);
            err = PF_RET_NO_ERR;
            pf_sys_set.flock_count++;
        } else {
            err = PF_RET_ERR;
        }
    }
    pfk_get_semaphore(p_file->p_sfd->lock.resource);
    return err;
}

s32 PF_UnLockFile(PF_FILE* p_file) {
    pfk_release_semaphore(p_file->p_sfd->lock.resource);
    if (p_file->p_sfd->lock.count == 0 && p_file->p_sfd->lock.wcount == 0) {
        if (pf_sys_set.flock_count != 0) {
            pfk_delete_semaphore(p_file->p_sfd->lock.resource);
            p_file->p_sfd->lock.resource = 0;
            pf_sys_set.flock_count--;
        }
    }
    return PF_RET_NO_ERR;
}
