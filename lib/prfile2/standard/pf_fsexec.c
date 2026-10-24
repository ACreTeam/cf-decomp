#include <prfile2/pf_standard.h>

s32 pf2_fsexec(struct PF_DTA* p_dta, s32 type, u8 param) {
    s32 err;

    err = PFDIR_fsexec(p_dta, type, param);
    err = PFAPI_convertReturnValue(err);
    return err;
}
