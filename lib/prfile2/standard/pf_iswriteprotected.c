#include <prfile2/pf_standard.h>

s32 pf2_iswriteprotected(s8 drv_char) {
    s32 err;
    u32 result = 0;

    err = PFVOL_iswriteprotected(drv_char, &result);
    if (err == 0) {
        return result;
    }
    return PFAPI_convertReturnValue(err);
}
