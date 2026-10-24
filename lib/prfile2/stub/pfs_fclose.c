#include <prfile2/pf_stub.h>
#include <prfile2/pf_api.h>

s32 pfstub_fclose(struct PF_FILE* p_file) {
    struct PF_STUB_MSG msg;

    msg.func = PFSTUB_FCLOSE;
    msg.ptr0 = p_file;
    msg.callback = NULL;
    if (pfstub_com_massage(&msg) == PF_RET_ERR) {
        return PF_RET_ERR;
    }
    return msg.result;
}
