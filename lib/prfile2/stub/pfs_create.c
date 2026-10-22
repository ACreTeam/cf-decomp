#include <prfile2/pf_stub.h>
#include <prfile2/pf_api.h>

struct PF_FILE* pfstub_create(const s8* path, u32 mode) {
    struct PF_STUB_MSG msg;

    msg.func = 0;
    msg.ptr0 = (void*)path;
    msg.arg0 = mode;
    msg.status = 0;
    if (pfstub_com_massage(&msg) == -1) {
        return 0;
    }
    return (struct PF_FILE*)msg.result;
}
