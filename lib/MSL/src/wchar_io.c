#include <wchar_io.h>

int __fwide(FILE* stream, int mode)
{
    return fwide(stream, mode);
}

int fwide(FILE* pFile, int file_mode)
{
    int orientation;
    int res;

    if ((pFile == NULL) || (pFile->file_mode.file_kind == file_closed))
    {
        return 0;
    }

    orientation = pFile->file_mode.file_orientation;

    switch (orientation)
    {
    case file_unoriented:
        if (file_mode > 0)
        {
            pFile->file_mode.file_orientation = file_wide_oriented;
        }
        else if (file_mode < 0)
        {
            pFile->file_mode.file_orientation = file_char_oriented;
        }

        res = file_mode;
        break;

    case file_wide_oriented:
        res = 1;
        break;

    case file_char_oriented:
        res = -1;
        break;
    }

    return res;
}
