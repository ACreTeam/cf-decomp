#include <prfile2/pf_standard.h>

s32 pf2_cut(const s8* path, u32 size) {
    s32 err;
    struct PF_STR path_str;
    u32 deleted;

    err = PFSTR_InitStr(&path_str, path, PF_STR_CODEMODE_OEM);
    if (err == 0) {
        PFFILE_cdelete(&path_str, size, 1, &deleted);
        if (deleted != 1) {
            err = PF_IERR_SYSTEM;
        }
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
