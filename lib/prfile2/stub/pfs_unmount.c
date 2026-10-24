#include <prfile2/pf_stub.h>
#include <prfile2/pf_api.h>

s32 pfstub_unmount(s8 drive, u32 mode) {
    struct PF_STUB_MSG msg;

    msg.func = PFSTUB_UNMOUNT;
    msg.drive = drive;
    msg.arg0 = mode;
    msg.callback = NULL;
    if (pfstub_com_massage(&msg) == PF_RET_ERR) {
        return PF_RET_ERR;
    }
    return msg.result;
}
