#include <prfile2/pf_stub.h>
#include <prfile2/pf_api.h>

u32 pfstub_fwrite(const void* p_buf, u32 size, u32 count, struct PF_FILE* p_file) {
    struct PF_STUB_MSG msg;

    msg.func = PFSTUB_FWRITE;
    msg.arg0 = size;
    msg.arg1 = count;
    msg.ptr0 = (void*)p_buf;
    msg.ptr1 = p_file;
    msg.callback = NULL;
    if (pfstub_com_massage(&msg) == PF_RET_ERR) {
        return 0;
    }
    return msg.result;
}
