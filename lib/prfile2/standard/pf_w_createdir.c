#include <prfile2/pf_api.h>

s32 pf2_w_createdir(const u16* path, u32 a, u32 b) {
    s32 err;
    struct PF_STR str;

    err = PFSTR_InitStr(&str, (const s8*)path, 2);
    if (err == 0) {
        err = PFDIR_createdir(&str, a, b);
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
