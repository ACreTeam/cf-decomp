#ifndef MSL_SCANF_H
#define MSL_SCANF_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

int vsscanf(const char*, const char*, va_list);
int sscanf(const char*, const char*, ...);

#ifdef __cplusplus
}
#endif

#endif // MSL_SCANF_H
