#include <prfile2/pf_stub.h>
#include <prfile2/pf_api.h>

s32 pfstub_attach(struct PF_DRV_TBL** drv_tbl, void* p_param) {
    struct PF_STUB_MSG msg;

    msg.func = PFSTUB_ATTACH;
    msg.ptr0 = drv_tbl;
    msg.callback = NULL;
    msg.ptr1 = p_param;
    if (pfstub_com_massage(&msg) == PF_RET_ERR) {
        return PF_RET_ERR;
    }
    return msg.result;
}
