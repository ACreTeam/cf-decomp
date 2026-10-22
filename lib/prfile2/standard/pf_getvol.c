#include <prfile2/pf_api.h>

s32 pf2_getvol(s8 drv_char) {
    s32 err;

    err = PFVOL_getvol(drv_char);
    err = PFAPI_convertReturnValue(err);
    return err;
}
