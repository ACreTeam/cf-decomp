#include <prfile2/pf_standard.h>

s32 pf2_readdir(struct PF_DIR* p_dir, struct PF_DIRENT* p_dirent) {
    s32 err;

    err = PFDIR_readdir(p_dir, p_dirent);
    err = PFAPI_convertReturnValue(err);
    return err;
}
