#include <prfile2/pf_standard.h>

s32 pf2_getvol(s8 drv_char, struct PF_VOLTAB* p_vol_tab) {
    s32 err;

    err = PFVOL_getvol(drv_char, p_vol_tab);
    err = PFAPI_convertReturnValue(err);
    return err;
}
