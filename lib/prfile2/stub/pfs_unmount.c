#include <prfile2/pf_stub.h>
#include <prfile2/pf_api.h>

s32 pfstub_unmount(s8 drive, u32 mode) {
    struct PF_STUB_MSG msg;

    msg.func = 39;
    msg.drive = drive;
    msg.arg0 = mode;
    msg.status = 0;
    if (pfstub_com_massage(&msg) == -1) {
        return -1;
    }
    return msg.result;
}
