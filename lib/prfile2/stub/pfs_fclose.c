#include <prfile2/pf_stub.h>
#include <prfile2/pf_api.h>

s32 pfstub_fclose(struct PF_FILE* p_file) {
    struct PF_STUB_MSG msg;

    msg.func = 2;
    msg.ptr0 = p_file;
    msg.status = 0;
    if (pfstub_com_massage(&msg) == -1) {
        return -1;
    }
    return msg.result;
}
