#include <prfile2/fa.h>

s32 FAFclose(void* stream) {
    return pfstub_fclose(stream) != 0 ? FA_RET_ERR : 0;
}
