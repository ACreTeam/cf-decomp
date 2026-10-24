#include <prfile2/pf_standard.h>

s32 pf2_fsync(struct PF_FILE* p_file) {
    s32 err;

    err = PFFILE_fsync(p_file);
    err = PFAPI_convertReturnValue(err);
    return err;
}
