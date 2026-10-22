#ifndef _MSL_STDDEF_H
#define _MSL_STDDEF_H

#include <size_t.h>

#ifndef NULL
#define NULL 0
#endif

typedef long ptrdiff_t;

#define offsetof(ST, M) ((size_t) & (((ST*)0)->M))

#endif  // _MSL_STDDEF_H
