#include <prfile2/pf_api.h>

s32 pf2_rewinddir(struct PF_DIR* p_dir) {
    s32 err;

    err = PFDIR_seekdir(p_dir, 0);
    err = PFAPI_convertReturnValue(err);
    return err;
}
