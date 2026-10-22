#include <prfile2/pf_api.h>

s32 pf2_feof(struct PF_FILE* p_file) {
    s32 err;

    err = PFFILE_feof(p_file);
    err = PFAPI_convertReturnValue4feof(err);
    return err;
}
