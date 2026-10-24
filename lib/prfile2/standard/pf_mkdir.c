#include <prfile2/pf_standard.h>

s32 pf2_mkdir(const s8* sPath) {
    s32 err;
    struct PF_STR path_str;

    err = PFSTR_InitStr(&path_str, sPath, PF_STR_CODEMODE_OEM);
    if (err == 0) {
        err = PFDIR_mkdir(&path_str);
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
