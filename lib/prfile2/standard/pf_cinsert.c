#include <prfile2/pf_api.h>

u32 pf2_cinsert(const s8* path, u32 start, u32 size) {
    s32 err;
    struct PF_STR str;
    u32 inserted = 0;

    err = PFSTR_InitStr(&str, path, 1);
    if (err == 0) {
        err = PFFILE_cinsert(&str, start, size, &inserted);
    } else {
        pf_vol_set.last_error = err;
    }
    return inserted;
}
