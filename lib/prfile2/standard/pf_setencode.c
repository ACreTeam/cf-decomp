#include <prfile2/pf_api.h>

s32 pf2_setencode(u32 code) {
    s32 err;

    err = PFVOL_setencode(code);
    err = PFAPI_convertReturnValue(err);
    return err;
}
