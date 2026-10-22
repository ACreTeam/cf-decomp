#include <revolution/vf/vf_all.h>

s32 VFiPFFATFS_initializeFATFS(u32 config, void* param) {
    return VFiPFVOL_InitModule(config, param);
}
