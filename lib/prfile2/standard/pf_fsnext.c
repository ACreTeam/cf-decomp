#include <prfile2/pf_standard.h>

s32 pf2_fsnext(struct PF_DTA* p_dta) {
    s32 err;

    err = PFDIR_fsnext(p_dta);
    err = PFAPI_convertReturnValue(err);
    return err;
}
