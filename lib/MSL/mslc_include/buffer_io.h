#ifndef MSL_BUFFER_IO_H
#define MSL_BUFFER_IO_H

#include <file_struct.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    __align_buffer,
    __dont_align_buffer
};

void __prep_buffer(FILE* file);
int __load_buffer(FILE* file, size_t* bytes_loaded, int alignment);
int __flush_buffer(FILE* file, size_t* bytes_flushed);

#ifdef __cplusplus
}
#endif

#endif // MSL_BUFFER_IO_H
