#include <prfile2/fa.h>

s32 FARemove(const s8* fileName) {
    return pfstub_remove(fileName) != 0 ? -1 : 0;
}
