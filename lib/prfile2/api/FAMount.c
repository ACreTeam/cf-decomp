#include <prfile2/fa.h>

s32 FAMount(s8 drive) {
    return pfstub_mount(drive) != 0 ? -1 : 0;
}
