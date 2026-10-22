#include <decomp.h>
#include <private/ax.h>
#include <revolution/ax.h>

#include <revolution/os.h>

const char* __AXVersion = "<< RVL_SDK - AX \trelease build: Aug  8 2007 01:58:18 (0x4199_60831) >>";

static BOOL __init = FALSE;

void AXInit(void) {
    AXInitEx(0);
}

BOOL AXIsInit(void) {
    return __init;
}

static void AXInitEx(u32 mode) {
    if (!__init) {
        OSRegisterVersion(__AXVersion);

        __AXAllocInit();
        __AXVPBInit();
        __AXSPBInit();
        __AXAuxInit();
        __AXClInit();
        __AXOutInit(mode);

        __init = TRUE;
    }
}
