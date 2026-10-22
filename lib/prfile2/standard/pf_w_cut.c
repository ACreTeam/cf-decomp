#include <prfile2/pf_api.h>

s32 pf2_w_cut(const u16* path, u32 size) {
    s32 err;
    struct PF_STR str;
    u32 deleted;

    err = PFSTR_InitStr(&str, (const s8*)path, 2);
    if (err == 0) {
        PFFILE_cdelete(&str, size, 1, &deleted);
        if (deleted != 1) {
            err = -1;
        }
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
