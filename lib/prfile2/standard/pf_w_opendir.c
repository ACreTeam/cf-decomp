#include <prfile2/pf_standard.h>

struct PF_DIR* pf2_w_opendir(const u16* path) {
    s32 err;
    struct PF_STR path_str;
    struct PF_DIR* p_dir;

    err = PFSTR_InitStr(&path_str, (const s8*)path, PF_STR_CODEMODE_UNICODE);
    if (err == 0) {
        err = PFDIR_opendir(&path_str, &p_dir);
    } else {
        pf_vol_set.last_error = err;
    }
    return PFAPI_convertReturnValue2NULL(err, p_dir);
}
