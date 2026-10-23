#ifndef MSL_EXTRAS_H
#define MSL_EXTRAS_H
#include <types.h>
#ifdef __cplusplus
extern "C" {
#endif

int stricmp(const char*, const char*);
int __msl_wcsnicmp(const wchar_t*, const wchar_t*, size_t);

#ifdef __cplusplus
}
#endif
#endif
