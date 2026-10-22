#include <prfile2/fa.h>

u32 FAFwrite(const void* buffer, u32 size, u32 count, void* stream) {
    return pfstub_fwrite(buffer, size, count, stream);
}
