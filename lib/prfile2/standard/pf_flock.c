#include <prfile2/pf_standard.h>

s32 pf2_flock(struct PF_FILE* p_file, s32 mode) {
    s32 err;

    err = PFFILE_flock(p_file, mode | PF_LOCK_NO_WAIT);
    err = PFAPI_convertReturnValue(err);
    return err;
}
