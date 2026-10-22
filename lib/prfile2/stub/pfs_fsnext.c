#include <prfile2/pf_stub.h>
#include <prfile2/pf_api.h>

s32 pfstub_fsnext(struct PF_DTA* p_dta) {
    struct PF_STUB_MSG msg;

    msg.func = 29;
    msg.ptr0 = p_dta;
    msg.status = 0;
    if (pfstub_com_massage(&msg) == -1) {
        return -1;
    }
    return msg.result;
}
