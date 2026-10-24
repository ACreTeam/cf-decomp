#include <prfile2/fa.h>

s32 FARmdir(const s8* dirName) {
    return pfstub_rmdir(dirName) != 0 ? FA_RET_ERR : 0;
}
