#include <prfile2/pf_standard.h>

s32 pf2_insert(const s8* path, u32 size) {
    s32 err;
    struct PF_STR path_str;
    u32 inserted;
    u32 deleted;

    err = PFSTR_InitStr(&path_str, path, PF_STR_CODEMODE_OEM);
    if (err == 0) {
        PFFILE_cinsert(&path_str, 0, size, &inserted);
        if (size != inserted || size == 0) {
            if (inserted != 0) {
                PFSTR_InitStr(&path_str, path, PF_STR_CODEMODE_OEM);
                PFFILE_cdelete(&path_str, 0, inserted, &deleted);
            }
            err = PF_IERR_SYSTEM;
        }
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
