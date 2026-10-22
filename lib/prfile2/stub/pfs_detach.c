#include <prfile2/pf_stub.h>
#include <prfile2/pf_api.h>

s32 pfstub_detach(s8 drive) {
    struct PF_STUB_MSG msg;

    msg.func = 41;
    msg.drive = drive;
    msg.status = 0;
    if (pfstub_com_massage(&msg) == -1) {
        return -1;
    }
    return msg.result;
}
