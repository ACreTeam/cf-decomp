#include <prfile2/pf_api.h>

s32 pf2_regctx(s32 context_id) {
    s32 err;

    err = PFVOL_regctx(context_id);
    err = PFAPI_convertReturnValue(err);
    return err;
}
