#include <prfile2/pf_standard.h>

s32 pf2_fsetclstlink(struct PF_FILE* p_file, s32 mode, struct PF_CLUSTER_LINK_RES* p_link) {
    s32 err;

    err = PFFILE_fsetclstlink(p_file, mode, p_link);
    err = PFAPI_convertReturnValue(err);
    return err;
}
