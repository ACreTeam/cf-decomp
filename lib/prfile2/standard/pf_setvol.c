#include <prfile2/pf_api.h>

s32 pf2_setvol(s8 drv_char, const s8* name) {
    s32 err;
    struct PF_STR str;

    err = PFSTR_InitStr(&str, name, 1);
    if (err == 0) {
        err = PFVOL_setvol(drv_char, &str);
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
