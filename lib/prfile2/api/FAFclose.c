#include <prfile2/fa.h>

s32 FAFclose(void* stream) {
    return pfstub_fclose(stream) != 0 ? -1 : 0;
}
