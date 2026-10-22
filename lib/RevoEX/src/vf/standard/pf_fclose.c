#include <revolution/vf/pf_fclose.h>
#include <revolution/vf/pf_file.h>
#include <revolution/vf/pf_api_util.h>

int VFipf2_fclose(struct PF_FILE* p_file) {
    s32 err;
    err = VFiPFAPI_convertReturnValue(VFiPFFILE_fclose(p_file));
    return err;
}
