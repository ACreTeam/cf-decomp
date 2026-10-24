#include <prfile2/pf_stub.h>
#include <prfile2/pf_api.h>

s32 pfstub_fsfirst(const s8* path, u32 attr, struct PF_DTA* p_dta) {
    struct PF_STUB_MSG msg;

    msg.func = PFSTUB_FSFIRST;
    msg.arg0 = attr;
    msg.ptr0 = (void*)path;
    msg.ptr1 = p_dta;
    msg.callback = NULL;
    if (pfstub_com_massage(&msg) == PF_RET_ERR) {
        return PF_RET_ERR;
    }
    return msg.result;
}
