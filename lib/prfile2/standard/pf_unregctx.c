#include <prfile2/pf_standard.h>

s32 pf2_unregctx(void) {
    s32 err;

    err = PFVOL_unregctx();
    err = PFAPI_convertReturnValue(err);
    return err;
}
