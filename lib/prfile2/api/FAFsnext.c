#include <prfile2/fa.h>

s32 FAFsnext(void* dta) {
    return pfstub_fsnext(dta) != 0 ? FA_RET_ERR : 0;
}
