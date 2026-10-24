#include <prfile2/pf_stub.h>
#include <prfile2/pf_api.h>

struct PF_FILE* pfstub_create(const s8* path, u32 mode) {
    struct PF_STUB_MSG msg;

    msg.func = PFSTUB_CREATE;
    msg.ptr0 = (void*)path;
    msg.arg0 = mode;
    msg.callback = NULL;
    if (pfstub_com_massage(&msg) == PF_RET_ERR) {
        return NULL;
    }
    return (struct PF_FILE*)msg.result;
}
