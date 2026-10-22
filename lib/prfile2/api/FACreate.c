#include <prfile2/fa.h>

void* FACreate(const s8* fileName, s32 unk) {
    void* p_file = pfstub_create(fileName, unk);
    if (p_file == 0) {
        return 0;
    }
    return p_file;
}
