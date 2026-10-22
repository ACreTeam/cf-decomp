#include <prfile2/pf_api.h>

struct PF_FILE* pf2_w_fopen(const u16* path, const s8* mode) {
    s32 open_mode;
    struct PF_FILE* p_file;
    struct PF_STR str;
    s32 err;

    open_mode = PFWAPI_ParseOpenModeString(mode);
    if (open_mode == 0) {
        pf_vol_set.last_error = 10;
        return NULL;
    } else {
        err = PFSTR_InitStr(&str, (const s8*)path, 2);
        if (err == 0) {
            err = PFFILE_fopen(&str, open_mode, &p_file);
        } else {
            pf_vol_set.last_error = err;
        }
        return PFAPI_convertReturnValue2NULL(err, p_file);
    }
}
