#include <prfile2/pf_standard.h>

s32 pf2_settailbuf(s8 drv_char, s32 size, u32* p_buf) {
    s32 err;

    err = PFVOL_settailbuf(drv_char, size, p_buf);
    err = PFAPI_convertReturnValue(err);
    return err;
}
