#include <revolution/vf/pf_init_prfile2.h>
#include <revolution/vf/vf_struct.h>
#include <revolution/vf/pf_api_util.h>
#include <revolution/vf/pf_fatfs.h>
#include <revolution/vf/pf_system.h>

int VFipf2_init_prfile2(s32 config, void* param) {
    s32 err;
    err = VFiPFFATFS_initializeFATFS(config, param);
    if (err == 0) {
        VFiPFSYS_initializeSYS();
    }
    return VFiPFAPI_convertReturnValue(err);
}
