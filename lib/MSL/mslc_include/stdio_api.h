#ifndef MSL_STDIO_API_H
#define MSL_STDIO_API_H

#include <size_t.h>
#include <wchar_t.h>

enum __ReadProcActions {
    __GetAChar,
    __UngetAChar,
    __TestForError
};

typedef struct {
    char* CharStr;
    size_t MaxCharCount;
    size_t CharsWritten;
} __OutStrCtrl;

typedef struct {
    char* NextChar;
    int NullCharDetected;
} __InStrCtrl;

typedef struct {
    wchar_t* wCharStr;
    size_t MaxCharCount;
    size_t CharsWritten;
} __wOutStrCtrl;

#ifdef __cplusplus
extern "C" {
#endif

int __StringRead(void*, int, int);

#ifdef __cplusplus
}
#endif

#endif // MSL_STDIO_API_H
