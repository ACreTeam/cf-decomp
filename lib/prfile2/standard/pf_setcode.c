#include <prfile2/pf_api.h>

s32 pf2_setcode(u32 code) {
    s32 err;

    err = PFVOL_setcode(code);
    err = PFAPI_convertReturnValue(err);
    return err;
}
