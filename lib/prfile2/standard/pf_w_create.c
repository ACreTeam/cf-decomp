#include <prfile2/pf_api.h>

struct PF_FILE* pf2_w_create(const u16* path, s32 mode) {
    s32 err;
    struct PF_STR str;
    struct PF_FILE* p_file;

    err = PFSTR_InitStr(&str, (const s8*)path, 2);
    if (err == 0) {
        err = PFFILE_fopen(&str, 0x19, &p_file);
    } else {
        pf_vol_set.last_error = err;
    }
    return PFAPI_convertReturnValue2NULL(err, p_file);
}
