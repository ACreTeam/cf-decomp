#include <prfile2/pf_api.h>

u32 pf2_cdelete(const s8* path, u32 start, u32 size) {
    s32 err;
    struct PF_STR str;
    u32 deleted = 0;

    err = PFSTR_InitStr(&str, path, 1);
    if (err == 0) {
        err = PFFILE_cdelete(&str, start, size, &deleted);
    } else {
        pf_vol_set.last_error = err;
    }
    return deleted;
}
