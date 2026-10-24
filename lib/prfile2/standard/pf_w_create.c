#include <prfile2/pf_standard.h>

struct PF_FILE* pf2_w_create(const u16* path, s32 mode) {
    s32 err;
    struct PF_STR path_str;
    struct PF_FILE* p_file;

    err = PFSTR_InitStr(&path_str, (const s8*)path, PF_STR_CODEMODE_UNICODE);
    if (err == 0) {
        err = PFFILE_fopen(&path_str, (PF_OPEN_MODE_WRITE | PF_OPEN_MODE_PLUS | PF_OPEN_MODE_NEW), &p_file);
    } else {
        pf_vol_set.last_error = err;
    }
    return PFAPI_convertReturnValue2NULL(err, p_file);
}
