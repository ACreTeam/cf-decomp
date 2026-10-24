#include <prfile2/pf_standard.h>

s32 pf2_attach(struct PF_DRV_TBL** drv_tbl, struct PF_ATTACH_PARAM* p_param) {
    s32 err;

    if (drv_tbl == NULL || *drv_tbl == NULL) {
        err = PF_IERR_INVALID_PARAM;
        pf_vol_set.last_error = err;
    } else {
        while (*drv_tbl != NULL) {
            if (p_param != NULL) {
                err = PFVOL_attach(*drv_tbl, p_param, p_param->nand_size);
            } else {
                err = PFVOL_attach(*drv_tbl, NULL, 0);
            }
            if (err != 0) {
                return PFAPI_convertReturnValue(err);
            }
            drv_tbl++;
        }
    }

    return PFAPI_convertReturnValue(err);
}
