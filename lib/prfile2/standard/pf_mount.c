#include <prfile2/pf_standard.h>

s32 pf2_mount(s8 drv_char) {
    s32 err;

    err = PFVOL_mount(drv_char);
    err = PFAPI_convertReturnValue(err);
    return err;
}
