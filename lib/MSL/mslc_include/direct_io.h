#ifndef MSL_DIRECT_IO_H
#define MSL_DIRECT_IO_H

#include <file_struct.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t __fread(void* buffer, size_t size, size_t count, FILE* stream);
size_t fread(void* buffer, size_t size, size_t count, FILE* stream);
size_t __fwrite(const void* buffer, size_t size, size_t count, FILE* stream);
size_t fwrite(const void* buffer, size_t size, size_t count, FILE* stream);

#ifdef __cplusplus
}
#endif

#endif // MSL_DIRECT_IO_H
