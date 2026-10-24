#include <prfile2/pf_standard.h>

s32 pf2_setclstlink(s8 drv_char, s32 mode, struct PF_CLUSTER_LINK_RES* p_link) {
    s32 err;

    err = PFVOL_setclstlink(drv_char, mode, p_link);
    err = PFAPI_convertReturnValue(err);
    return err;
}
