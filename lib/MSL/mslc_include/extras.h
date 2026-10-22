#ifndef MSL_EXTRAS_H
#define MSL_EXTRAS_H

#include <size_t.h>

#ifdef __cplusplus
extern "C" {
#endif

int stricmp(const char* str1, const char* str2);
int strncasecmp(const char* str1, const char* str2, size_t n);
int strcmpi(const char* str1, const char* str2);
int strcasecmp(const char* str1, const char* str2);
char* itoa(int value, char* str, int base);

#ifdef __cplusplus
}
#endif

#endif
