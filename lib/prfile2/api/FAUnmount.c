#include <prfile2/fa.h>

s32 FAUnmount(s8 drive, u32 force) {
    s32 err = pfstub_unmount(drive, force);
    if (err == -1) {
        return -1;
    }
    if (err == 1) {
        return 1;
    }
    return err != 0 ? -1 : 0;
}
