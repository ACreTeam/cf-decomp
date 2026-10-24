#include <prfile2/fa.h>
#include <prfile2/pf_types.h>

s32 FAUnmount(s8 drive, u32 force) {
    s32 err = pfstub_unmount(drive, force);

    if (err == PF_RET_ERR) {
        return FA_RET_ERR;
    }
    if (err == PF_RET_UNMOUNT_FORCED) {
        return FA_RET_UNMOUNT_FORCED;
    }
    return err != 0 ? FA_RET_ERR : 0;
}
