#include <prfile2/pf_standard.h>

s32 pf2_fchdir(struct PF_DIR* p_dir) {
    s32 err;

    err = PFDIR_fchdir(p_dir);
    err = PFAPI_convertReturnValue(err);
    return err;
}
