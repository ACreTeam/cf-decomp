#include <prfile2/pf_standard.h>

struct PF_FILE* pf2_fopen(const s8* path, const s8* mode) {
    s32 open_mode;
    struct PF_FILE* p_file;
    struct PF_STR path_str;
    s32 err;

    open_mode = PFAPI_ParseOpenModeString(mode);
    if (open_mode == 0) {
        pf_vol_set.last_error = PF_IERR_INVALID_PARAM;
        return NULL;
    }
    err = PFSTR_InitStr(&path_str, path, PF_STR_CODEMODE_OEM);
    if (err == 0) {
        err = PFFILE_fopen(&path_str, open_mode, &p_file);
    } else {
        pf_vol_set.last_error = err;
    }
    return PFAPI_convertReturnValue2NULL(err, p_file);
}
