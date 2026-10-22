#include <prfile2/pf_api.h>

s32 pf2_fstat(const s8* path, struct PF_STAT* p_stat) {
    s32 err;
    struct PF_STR path_str;

    err = PFSTR_InitStr(&path_str, path, 1);
    if (err == 0) {
        err = PFDIR_fstat(&path_str, p_stat);
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
