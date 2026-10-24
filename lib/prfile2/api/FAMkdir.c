#include <prfile2/fa.h>

s32 FAMkdir(const s8* dirName) {
    return pfstub_mkdir(dirName) != 0 ? FA_RET_ERR : 0;
}
