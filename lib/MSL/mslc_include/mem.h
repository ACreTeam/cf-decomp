#ifndef MSL_MEM_H
#define MSL_MEM_H

#include <size_t.h>

#ifdef __cplusplus
extern "C" {
#endif

void* memset(void* dest, int value, size_t size);

#ifdef __cplusplus
}
#endif

#endif  // MSL_MEM_H
