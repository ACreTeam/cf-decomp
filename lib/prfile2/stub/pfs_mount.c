#include <prfile2/pf_stub.h>
#include <prfile2/pf_api.h>

s32 pfstub_mount(s8 drive) {
    struct PF_STUB_MSG msg;

    msg.func = PFSTUB_MOUNT;
    msg.drive = drive;
    msg.callback = NULL;
    if (pfstub_com_massage(&msg) == PF_RET_ERR) {
        return PF_RET_ERR;
    }
    return msg.result;
}
