#include <prfile2/pf_api.h>

s32 pf2_chmod(const s8* path, u32 attr) {
    s32 err;
    struct PF_STR path_str;

    err = PFSTR_InitStr(&path_str, path, 1);
    if (err == 0) {
        err = PFDIR_chmod(&path_str, attr);
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
