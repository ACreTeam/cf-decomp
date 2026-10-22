#include <prfile2/fa.h>

s32 FAErrnum(void) {
    s32 err = pfstub_errnum();
    if (err == 5) {
        return 5;
    }
    return err == 0 ? 0 : err;
}
