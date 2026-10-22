#include <prfile2/pf_api.h>

s32 pf2_seekdir(struct PF_DIR* p_dir, u32 pos) {
    s32 err;

    err = PFDIR_seekdir(p_dir, pos);
    err = PFAPI_convertReturnValue(err);
    return err;
}
