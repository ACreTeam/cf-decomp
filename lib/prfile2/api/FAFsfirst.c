#include <prfile2/fa.h>

s32 FAFsfirst(const s8* dirName, u32 attr, void* dta) {
    return pfstub_fsfirst(dirName, attr, dta) != 0 ? -1 : 0;
}
