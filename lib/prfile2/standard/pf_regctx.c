#include <prfile2/pf_standard.h>

s32 pf2_regctx(void) {
    s32 err;

    err = PFVOL_regctx();
    err = PFAPI_convertReturnValue(err);
    return err;
}
