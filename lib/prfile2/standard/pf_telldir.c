#include <prfile2/pf_api.h>

s32 pf2_telldir(struct PF_DIR* p_dir) {
    s32 err;

    err = PFDIR_telldir(p_dir);
    err = PFAPI_convertReturnValue(err);
    return err;
}
