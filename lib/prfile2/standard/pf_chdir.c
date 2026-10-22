#include <prfile2/pf_api.h>

s32 pf2_chdir(const s8* path) {
    s32 err;
    struct PF_STR path_str;

    err = PFSTR_InitStr(&path_str, path, 1);
    if (err == 0) {
        err = PFDIR_chdir(&path_str);
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
