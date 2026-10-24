#include <prfile2/pf_standard.h>

s32 pf2_sync(s8 drv_char, s32 mode) {
    s32 err;

    err = PFVOL_sync(drv_char, mode);
    err = PFAPI_convertReturnValue(err);
    return err;
}
