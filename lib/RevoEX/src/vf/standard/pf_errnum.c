#include <revolution/vf/vf_struct.h>
#include <revolution/vf/pf_errnum.h>
#include <revolution/vf/pf_api_util.h>
#include <revolution/vf/pf_volume.h>

int VFipf2_errnum(void) {
    s32 err;
    err = VFiPFAPI_convertError(VFiPFVOL_errnum());
    return err;
}
