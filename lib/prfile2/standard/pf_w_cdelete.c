#include <prfile2/pf_api.h>

u32 pf2_w_cdelete(const u16* path, u32 start, u32 size) {
    s32 err;
    struct PF_STR str;
    u32 deleted = 0;

    err = PFSTR_InitStr(&str, (const s8*)path, 2);
    if (err == 0) {
        err = PFFILE_cdelete(&str, start, size, &deleted);
    } else {
        pf_vol_set.last_error = err;
    }
    return deleted;
}
