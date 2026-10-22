#include <prfile2/pf_stub.h>
#include <prfile2/pf_api.h>

s32 pfstub_remove(const s8* path) {
    struct PF_STUB_MSG msg;

    msg.func = 7;
    msg.ptr0 = (void*)path;
    msg.status = 0;
    if (pfstub_com_massage(&msg) == -1) {
        return -1;
    }
    return msg.result;
}
