#include <prfile2/pf_standard.h>

s32 pf2_setvolcfg(s8 drv_char, struct PF_VOL_CFG* p_config) {
    s32 err;

    err = PFVOL_setvolcfg(drv_char, p_config);
    err = PFAPI_convertReturnValue(err);
    return err;
}
