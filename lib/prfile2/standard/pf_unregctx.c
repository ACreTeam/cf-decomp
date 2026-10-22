#include <prfile2/pf_api.h>

s32 pf2_unregctx(s32 context_id) {
    s32 err;

    err = PFVOL_unregctx(context_id);
    err = PFAPI_convertReturnValue(err);
    return err;
}
