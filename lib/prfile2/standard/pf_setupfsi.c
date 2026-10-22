#include <prfile2/pf_api.h>

s32 pf2_setupfsi(s8 drv_char, u32 mode) {
    s32 err;

    err = PFVOL_setupfsi(drv_char, mode);
    err = PFAPI_convertReturnValue(err);
    return err;
}
