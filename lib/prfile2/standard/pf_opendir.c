#include <prfile2/pf_api.h>

struct PF_DIR* pf2_opendir(const s8* path) {
    s32 err;
    struct PF_STR str;
    struct PF_DIR* p_dir;

    err = PFSTR_InitStr(&str, path, 1);
    if (err == 0) {
        err = PFDIR_opendir(&str, &p_dir);
    } else {
        pf_vol_set.last_error = err;
    }
    return PFAPI_convertReturnValue2NULL(err, p_dir);
}
