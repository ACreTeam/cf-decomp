#include <prfile2/pf_api.h>

s32 pf2_w_xdivide(const u16* p1, const u16* p2, u32 size) {
    s32 err;
    struct PF_STR str1;
    struct PF_STR str2;

    err = PFSTR_InitStr(&str1, (const s8*)p1, 2);
    err |= PFSTR_InitStr(&str2, (const s8*)p2, 2);
    if (err == 0) {
        err = PFFILE_divide(&str1, &str2, size);
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
