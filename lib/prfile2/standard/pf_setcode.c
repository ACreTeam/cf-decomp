#include <prfile2/pf_standard.h>

s32 pf2_setcode(struct PF_CHARCODE* p_codeset) {
    s32 err;

    err = PFVOL_setcode(p_codeset);
    err = PFAPI_convertReturnValue(err);
    return err;
}
