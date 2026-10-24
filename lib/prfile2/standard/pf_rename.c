#include <prfile2/pf_standard.h>

s32 pf2_rename(const s8* path, const s8* new_name) {
    s32 err;
    struct PF_STR path_str;
    struct PF_STR new_name_str;

    err = PFSTR_InitStr(&path_str, path, PF_STR_CODEMODE_OEM);
    err |= PFSTR_InitStr(&new_name_str, new_name, PF_STR_CODEMODE_OEM);
    if (err == 0) {
        err = PFDIR_rename(&path_str, &new_name_str);
    } else {
        pf_vol_set.last_error = err;
    }
    err = PFAPI_convertReturnValue(err);
    return err;
}
