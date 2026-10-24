#include <prfile2/pf_standard.h>

s32 pf2_derrnum(s8 drv_char) {
    s32 err;

    err = PFVOL_derrnum(drv_char);
    err = PFAPI_convertDriverError(err);
    return err;
}
