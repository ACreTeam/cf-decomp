#include "extras.h"
#include "file_io.h"
#include "locale.h"

inline int tolower_inline(int c)
{
    return ((c < 0) || (c >= 0x100)) ? c : (int)(_current_locale.ctype_cmpt_ptr->lower_map_ptr[c]);
}

int stricmp(const char* s1, const char* s2)
{
    char c1, c2;

    while (1)
    {
        c1 = tolower_inline(*s1++);
        c2 = tolower_inline(*s2++);

        if (c1 < c2)
        {
            return -1;
        }

        if (c1 > c2)
        {
            return 1;
        }

        if (c1 == 0)
        {
            return 0;
        }
    }
}

int strncasecmp(const char* s1, const char* s2, size_t n)
{
    return __msl_strnicmp(s1, s2, n);
}

int strcmpi(const char* str1, const char* str2)
{
    return stricmp(str1, str2);
}

int strcasecmp(const char* s1, const char* s2)
{
    return stricmp(s1, s2);
}

inline wchar_t towlower_inline(wchar_t c)
{
    return (c >= 0x100) ? c : _current_locale.ctype_cmpt_ptr->wlower_map_ptr[c];
}

int __msl_wcsnicmp(const wchar_t* s1, const wchar_t* s2, size_t n)
{
    size_t i;
    wchar_t c1, c2;

    for (i = 0; i < n; i++)
    {
        c1 = towlower_inline(*s1++);
        c2 = towlower_inline(*s2++);

        if (c1 < c2)
        {
            return -1;
        }

        if (c1 > c2)
        {
            return 1;
        }

        if (c1 == 0)
        {
            return 0;
        }
    }

    return 0;
}

char* itoa(int value, char* str, int base)
{
    return __msl_itoa(value, str, base);
}
