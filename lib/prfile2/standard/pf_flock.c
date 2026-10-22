#include <prfile2/pf_api.h>

s32 pf2_flock(struct PF_FILE* p_file, u32 mode) {
    s32 err;

    err = PFFILE_flock(p_file, mode | 8);
    err = PFAPI_convertReturnValue(err);
    return err;
}
