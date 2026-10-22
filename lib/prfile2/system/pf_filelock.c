#include <revolution/types.h>

// PrFILE2 file lock: one semaphore per open file (see pfk_api.c), created on first lock.

struct PF_LOCK {
    u16 mode;         // 0x00
    u16 count;        // 0x02
    u32 wcount;       // 0x04
    void* owner;      // 0x08
    s32 resource;     // 0x0C
};

struct PF_SFD {
    u8 _000[0x27C];
    struct PF_LOCK lock;  // 0x27C
    u16 num_handlers;     // 0x28C
};

struct PF_FILE {
    u32 stat;             // 0x00
    u32 open_mode;        // 0x04
    struct PF_SFD* p_sfd; // 0x08
};

struct PF_SYSTEM_SET {
    u8 _0000[0x2398];
    u32 flock_count;  // 0x2398
    u8 _239C[4];
};

struct PFK_SEMAPHORE_INFO {
    u32 initial;   // 0x00
    u32 max;       // 0x04
    void* p_name;  // 0x08
};

extern struct PF_SYSTEM_SET pf_sys_set;

s32 pfk_create_semaphore(struct PFK_SEMAPHORE_INFO* p_info);
s32 pfk_delete_semaphore(s32 id);
s32 pfk_get_semaphore(s32 id);
s32 pfk_release_semaphore(s32 id);

void PF_InitLockFile(void) {
    pf_sys_set.flock_count = 0;
}

s32 PF_LockFile(struct PF_FILE* p_file) {
    s32 err = -1;
    struct PF_SFD* p_sfd = p_file->p_sfd;

    if (p_sfd->lock.count == 0 && p_sfd->lock.resource == 0) {
        if (pf_sys_set.flock_count < 0xFFFFFFFF) {
            struct PFK_SEMAPHORE_INFO info;

            info.initial = 1;
            info.max = 1;
            info.p_name = &p_file->p_sfd->num_handlers;
            p_file->p_sfd->lock.resource = pfk_create_semaphore(&info);
            err = 0;
            pf_sys_set.flock_count++;
        } else {
            err = -1;
        }
    }
    pfk_get_semaphore(p_file->p_sfd->lock.resource);
    return err;
}

s32 PF_UnLockFile(struct PF_FILE* p_file) {
    pfk_release_semaphore(p_file->p_sfd->lock.resource);
    if (p_file->p_sfd->lock.count == 0 && p_file->p_sfd->lock.wcount == 0) {
        if (pf_sys_set.flock_count != 0) {
            pfk_delete_semaphore(p_file->p_sfd->lock.resource);
            p_file->p_sfd->lock.resource = 0;
            pf_sys_set.flock_count--;
        }
    }
    return 0;
}
