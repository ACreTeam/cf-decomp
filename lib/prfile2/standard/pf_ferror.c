#include <prfile2/pf_api.h>

s32 pf2_ferror(struct PF_FILE* p_file) {
    s32 err;

    err = PFFILE_ferror(p_file);
    err = PFAPI_convertError(err);
    return err;
}
