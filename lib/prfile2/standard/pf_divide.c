#include <prfile2/pf_api.h>

s32 pf2_divide(const s8* p1, const s8* p2, u32 size) {
    s32 err;
    struct PF_STR str1;
    struct PF_STR str2;

    err = PFSTR_InitStr(&str1, p1, 1);
    err |= PFSTR_InitStr(&str2, p2, 1);
    if (err == 0) {
        err = PFFILE_divide(&str1, &str2, size);
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
