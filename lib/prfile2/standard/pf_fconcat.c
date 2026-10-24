#include <prfile2/pf_standard.h>

s32 pf2_fconcat(const s8* path1, const s8* path2) {
    s32 err;
    struct PF_STR path1_str;
    struct PF_STR path2_str;

    err = PFSTR_InitStr(&path1_str, path1, PF_STR_CODEMODE_OEM);
    err |= PFSTR_InitStr(&path2_str, path2, PF_STR_CODEMODE_OEM);
    if (err == 0) {
        err = PFFILE_combine(&path1_str, &path2_str);
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
