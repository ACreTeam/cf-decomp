#ifndef MBSTRING_H
#define MBSTRING_H

#include <size_t.h>
#include <wchar_t.h>

#ifdef __cplusplus
extern "C" {
#endif

int __mbtowc_noconv(wchar_t* pDest, const char* pSrc, size_t n);
int __wctomb_noconv(char* pSrc, wchar_t cr);

#ifdef __cplusplus
}
#endif

#endif
