#include <prfile2/pf_standard.h>

s32 pf2_fadjust(struct PF_FILE* p_file) {
    s32 err;

    err = PFFILE_fadjust(p_file);
    err = PFAPI_convertReturnValue(err);
    return err;
}
