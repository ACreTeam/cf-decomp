#include <prfile2/pf_standard.h>

u32 pf2_w_cdelete(const u16* path, u32 start, u32 size) {
    s32 err;
    struct PF_STR path_str;
    u32 deleted = 0;

    err = PFSTR_InitStr(&path_str, (const s8*)path, PF_STR_CODEMODE_UNICODE);
    if (err == 0) {
        err = PFFILE_cdelete(&path_str, start, size, &deleted);
    } else {
        pf_vol_set.last_error = err;
    }
    return deleted;
}
