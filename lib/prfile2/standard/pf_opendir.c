#include <prfile2/pf_standard.h>

struct PF_DIR* pf2_opendir(const s8* path) {
    s32 err;
    struct PF_STR path_str;
    struct PF_DIR* p_dir;

    err = PFSTR_InitStr(&path_str, path, PF_STR_CODEMODE_OEM);
    if (err == 0) {
        err = PFDIR_opendir(&path_str, &p_dir);
    } else {
        pf_vol_set.last_error = err;
    }
    return PFAPI_convertReturnValue2NULL(err, p_dir);
}
