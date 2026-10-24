#include <prfile2/pf_standard.h>

s32 pf2_setencode(s32 encode) {
    s32 err;

    err = PFVOL_setencode(encode);
    err = PFAPI_convertReturnValue(err);
    return err;
}
