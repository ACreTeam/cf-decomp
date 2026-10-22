#include <prfile2/pf_api.h>

s32 pf2_fsetclstlink(struct PF_FILE* p_file) {
    s32 err;

    err = PFFILE_fsetclstlink(p_file);
    err = PFAPI_convertReturnValue(err);
    return err;
}
