#include <prfile2/pf_standard.h>

s32 pf2_w_xdivide(const u16* path1, const u16* path2, u32 offset) {
    s32 err;
    struct PF_STR path1_str;
    struct PF_STR path2_str;

    err = PFSTR_InitStr(&path1_str, (const s8*)path1, PF_STR_CODEMODE_UNICODE);
    err |= PFSTR_InitStr(&path2_str, (const s8*)path2, PF_STR_CODEMODE_UNICODE);
    if (err == 0) {
        err = PFFILE_divide(&path1_str, &path2_str, offset);
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
