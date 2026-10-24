#include <prfile2/pf_stub.h>
#include <prfile2/pf_api.h>

s32 pfstub_mkdir(const s8* path) {
    struct PF_STUB_MSG msg;

    msg.func = PFSTUB_MKDIR;
    msg.ptr0 = (void*)path;
    msg.callback = NULL;
    if (pfstub_com_massage(&msg) == PF_RET_ERR) {
        return PF_RET_ERR;
    }
    return msg.result;
}
