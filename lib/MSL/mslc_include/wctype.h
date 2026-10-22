#ifndef MSL_WCTYPE_H
#define MSL_WCTYPE_H

#include "internal/locale.h"
#include "locale.h"
#include <wchar_t.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const unsigned short __wctype_map[256];
extern const wchar_t __wupper_map[256];
extern const wchar_t __wlower_map[256];

#ifdef __cplusplus
}
#endif

inline int iswdigit(wint_t value)
{
    return value < 0 || value >= 256
        ? 0
        : _current_locale.ctype_cmpt_ptr->wctype_map_ptr[value] & (1 << 3);
}

inline int iswupper(wint_t value)
{
    return value < 0 || value >= 256
        ? 0
        : _current_locale.ctype_cmpt_ptr->wctype_map_ptr[value] & (1 << 9);
}

#endif // MSL_WCTYPE_H
