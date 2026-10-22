#include <prfile2/pf_api.h>

s32 pf2_w_insert(const u16* path, u32 size) {
    s32 err;
    struct PF_STR str;
    u32 inserted;
    u32 deleted;

    err = PFSTR_InitStr(&str, (const s8*)path, 2);
    if (err == 0) {
        PFFILE_cinsert(&str, 0, size, &inserted);
        if (size != inserted || size == 0) {
            if (inserted != 0) {
                PFSTR_InitStr(&str, (const s8*)path, 2);
                PFFILE_cdelete(&str, 0, inserted, &deleted);
            }
            err = -1;
        }
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
