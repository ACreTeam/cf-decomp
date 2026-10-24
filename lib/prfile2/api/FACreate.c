#include <prfile2/fa.h>

void* FACreate(const s8* fileName, s32 mode) {
    void* p_file = pfstub_create(fileName, mode);

    if (p_file == NULL) {
        return NULL;
    }
    return p_file;
}
