#ifndef WCHAR_IO_H
#define WCHAR_IO_H

#include <file_struct.h>

#ifdef __cplusplus
extern "C" {
#endif

int __fwide(FILE*, int);
int fwide(FILE*, int);

#ifdef __cplusplus
}
#endif

#endif
