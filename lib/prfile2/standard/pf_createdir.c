#include <prfile2/pf_standard.h>

s32 pf2_createdir(const s8* path, s32 option, struct PF_DTA* p_dta) {
    s32 err;
    struct PF_STR path_str;

    err = PFSTR_InitStr(&path_str, path, PF_STR_CODEMODE_OEM);
    if (err == 0) {
        err = PFDIR_createdir(&path_str, option, p_dta);
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
