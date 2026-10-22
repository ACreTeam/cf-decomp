#include <prfile2/pf_api.h>

s32 pf2_attach(struct PF_DRV_TBL** drv_tbl, struct PF_ATTACH_PARAM* p_param) {
    s32 err;

    if (drv_tbl == 0 || !*drv_tbl) {
        err = 10;
        pf_vol_set.last_error = err;
    } else {
        for (; *drv_tbl; drv_tbl++) {
            if (p_param != NULL) {
                err = PFVOL_attach(*drv_tbl, p_param, p_param->param);
            } else {
                err = PFVOL_attach(*drv_tbl, NULL, NULL);
            }
            if (err) {
                return PFAPI_convertReturnValue(err);
            }
        }
    }

    return PFAPI_convertReturnValue(err);
}
