#include <prfile2/pf_standard.h>

s32 pf2_seekdir(struct PF_DIR* p_dir, s32 offset) {
    s32 err;

    err = PFDIR_seekdir(p_dir, offset);
    err = PFAPI_convertReturnValue(err);
    return err;
}
