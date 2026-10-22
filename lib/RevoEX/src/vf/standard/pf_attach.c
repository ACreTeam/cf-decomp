#include <revolution/vf/vf_struct.h>
#include <revolution/vf/pf_attach.h>
#include <revolution/vf/pf_api_util.h>
#include <revolution/vf/pf_volume.h>

int VFipf2_attach(struct PF_DRV_TBL** drv_tbl) {
    int err;

    if (drv_tbl == 0 || !*drv_tbl) {
        err = 10;
        VFipf_vol_set.last_error = err;
    } else {
        for (; *drv_tbl; drv_tbl++) {
            err = VFiPFVOL_attach(*drv_tbl);
            if (err) {
                return VFiPFAPI_convertReturnValue(err);
            }
        }
    }

    return VFiPFAPI_convertReturnValue(err);
}
