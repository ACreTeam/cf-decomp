#ifndef MSL_WCHAR_H
#define MSL_WCHAR_H

#include <size_t.h>
#include <wchar_t.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t wcslen(const wchar_t* s);
wchar_t* wcscpy(wchar_t* dest, const wchar_t* src);
wchar_t* wcsncpy(wchar_t* dest, const wchar_t* src, size_t n);
int wcscmp(const wchar_t* lhs, const wchar_t* rhs);
wchar_t* wcschr(const wchar_t* s, wchar_t c);
wchar_t* wmemcpy(wchar_t* dest, const wchar_t* src, size_t n);
wchar_t* wmemchr(const wchar_t* s, wchar_t c, size_t n);

#ifdef __cplusplus
}
#endif

#endif // MSL_WCHAR_H
