#include <prfile2/pf_api.h>

s32 pf2_w_fsfirst(const u16* path, u8 attr, struct PF_DTA* p_dta) {
    s32 err;
    struct PF_STR str;

    err = PFSTR_InitStr(&str, (const s8*)path, 2);
    if (err == 0) {
        err = PFDIR_fsfirst(&str, attr, p_dta);
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
