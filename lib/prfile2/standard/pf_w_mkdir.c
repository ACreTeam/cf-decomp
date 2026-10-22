#include <prfile2/pf_api.h>

s32 pf2_w_mkdir(const u16* path) {
    s32 err;
    struct PF_STR path_str;

    err = PFSTR_InitStr(&path_str, (const s8*)path, 2);
    if (err == 0) {
        err = PFDIR_mkdir(&path_str);
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
