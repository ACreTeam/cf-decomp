#include <prfile2/pf_standard.h>

s32 pf2_setvol(s8 drv_char, const s8* name) {
    s32 err;
    struct PF_STR name_str;

    err = PFSTR_InitStr(&name_str, name, PF_STR_CODEMODE_OEM);
    if (err == 0) {
        err = PFVOL_setvol(drv_char, &name_str);
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
