#include <prfile2/pf_stub.h>
#include <prfile2/pf_api.h>

s32 pfstub_attach(struct PF_DRV_TBL** drv_tbl, void* p_param) {
    struct PF_STUB_MSG msg;

    msg.func = 27;
    msg.ptr0 = drv_tbl;
    msg.status = 0;
    msg.ptr1 = p_param;
    if (pfstub_com_massage(&msg) == -1) {
        return -1;
    }
    return msg.result;
}
