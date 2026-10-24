#include <prfile2/pf_api.h>
#include <prfile2/pf_volume.h>

s32 PFVOL_iswriteprotected(s8 drv_char, u32* p_result) {
    struct PF_VOLUME* p_vol;

    p_vol = PFVOL_GetVolumeFromDrvChar(drv_char);
    if ((p_vol->flags & PF_VOL_FLAG_MOUNTED) == 0) {
        pf_vol_set.last_error = PF_IERR_NOT_MOUNTED;
        p_vol->last_error = PF_IERR_NOT_MOUNTED;
        return PF_IERR_NOT_MOUNTED;
    }
    if (p_vol->flags & PF_VOL_FLAG_WRITE_PROTECTED) {
        *p_result = TRUE;
    } else {
        *p_result = FALSE;
    }
    return 0;
}
