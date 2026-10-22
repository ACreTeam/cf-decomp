#include <prfile2/pf_api.h>

s32 pf2_fchdir(struct PF_FILE* p_file) {
    s32 err;

    err = PFDIR_fchdir(p_file);
    err = PFAPI_convertReturnValue(err);
    return err;
}
