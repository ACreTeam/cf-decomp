#include <prfile2/pf_api.h>

s32 pf2_getvolcfg(s8 drv_char) {
    s32 err;

    err = PFVOL_getvolcfg(drv_char);
    err = PFAPI_convertReturnValue(err);
    return err;
}
