#include <prfile2/fa.h>
#include <prfile2/pf_types.h>

s32 FAErrnum(void) {
    s32 err = pfstub_errnum();

    if (err == PF_ERR_EIO) {
        return PF_ERR_EIO;
    }
    return err == 0 ? 0 : err;
}
