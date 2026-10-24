#include <prfile2/pf_standard.h>

s32 pf2_w_remove(const u16* path) {
    s32 err;
    struct PF_STR path_str;

    err = PFSTR_InitStr(&path_str, (const s8*)path, PF_STR_CODEMODE_UNICODE);
    if (err == 0) {
        err = PFFILE_remove(&path_str);
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
