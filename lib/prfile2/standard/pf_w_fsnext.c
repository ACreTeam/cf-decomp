#include <prfile2/pf_api.h>

s32 pf2_w_fsnext(struct PF_DTA* p_dta) {
    s32 err;

    err = PFDIR_fsnext(p_dta);
    err = PFAPI_convertReturnValue(err);
    return err;
}
