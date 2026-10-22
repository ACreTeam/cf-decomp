#include <prfile2/pf_stub.h>
#include <prfile2/pf_api.h>

s32 pfstub_fsfirst(const s8* path, u32 attr, struct PF_DTA* p_dta) {
    struct PF_STUB_MSG msg;

    msg.func = 28;
    msg.arg0 = attr;
    msg.ptr0 = (void*)path;
    msg.ptr1 = p_dta;
    msg.status = 0;
    if (pfstub_com_massage(&msg) == -1) {
        return -1;
    }
    return msg.result;
}
