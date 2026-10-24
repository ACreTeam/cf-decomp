#include <prfile2/pf_standard.h>

s32 pf2_setupfsi(s8 drv_char, s16 mode) {
    s32 err;

    err = PFVOL_setupfsi(drv_char, mode);
    err = PFAPI_convertReturnValue(err);
    return err;
}
