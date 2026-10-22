#ifndef WSTRING_H
#define WSTRING_H

#include <size_t.h>
#include <wchar_t.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t wcslen(const wchar_t* pStr);
wchar_t* wcscpy(wchar_t* pDest, const wchar_t* pSrc);
wchar_t* wcsncpy(wchar_t* pDest, const wchar_t* pSrc, size_t num);
int wcscmp(const wchar_t* pStr1, const wchar_t* pStr2);
wchar_t* wcschr(const wchar_t* pStr, wchar_t chr);

#ifdef __cplusplus
}
#endif

#endif
