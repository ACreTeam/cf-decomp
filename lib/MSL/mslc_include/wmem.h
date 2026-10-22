#ifndef WMEM_H
#define WMEM_H

#include "wchar_t.h"
#include "size_t.h"

#ifdef __cplusplus
extern "C" {
#endif

wchar_t* wmemcpy(wchar_t*, const wchar_t*, size_t);
wchar_t* wmemchr(const wchar_t*, wchar_t, size_t);

#ifdef __cplusplus
}
#endif

#endif // WMEM_H
