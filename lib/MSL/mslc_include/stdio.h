#ifndef MSL_STDIO_H
#define MSL_STDIO_H

/* Longest file name the library can open (C89 4.9.1). */
#define FILENAME_MAX 256

#include <stdarg.h>
#include <size_t.h>
#include "stdio_api.h"

#ifndef NULL
#define NULL 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Streams (C89 4.9.1). FILE stays incomplete here: the library owns its
   layout and every user of this header holds it only through a pointer. */
typedef struct _FILE FILE;

#define EOF (-1)

/* SEEK_SET/SEEK_CUR/SEEK_END are defined for C only: nw4hbm's
   ut::FileStream::SeekOrigin spells its enumerators with those names, and a
   macro would rewrite them in every C++ unit that reaches both headers. */
#ifndef __cplusplus
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

/* File operations (C89 4.9.4) and file access (4.9.5). */
int remove(const char* filename);
int rename(const char* old, const char* new_);
FILE* fopen(const char* filename, const char* mode);
FILE* freopen(const char* filename, const char* mode, FILE* stream);
int fclose(FILE* stream);
int fflush(FILE* stream);
int setvbuf(FILE* stream, char* buf, int mode, size_t size);

/* Character input/output (C89 4.9.7). */
int fgetc(FILE* stream);
int fputs(const char* s, FILE* stream);

/* Formatted output (C89 4.9.6). */
int fprintf(FILE* stream, const char* format, ...);
int printf(const char* format, ...);
int sprintf(char* s, const char* format, ...);
int sscanf(const char* s, const char* format, ...);
int snprintf(char* s, size_t n, const char* format, ...);
int vprintf(const char* format, va_list arguments);
int vsprintf(char* s, const char* format, va_list arguments);
int vsnprintf(char* s, size_t n, const char* format, va_list arguments);
#ifdef __cplusplus
/* wchar_t is a keyword only in C++; declaring this unguarded makes <stdio.h>
   invalid for every C translation unit that includes it. */
int vswprintf(wchar_t* s, size_t n, const wchar_t* format, va_list arguments);
#endif

/* Direct input/output (C89 4.9.8). */
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);

/* File positioning (C89 4.9.9). */
int fseek(FILE* stream, long offset, int whence);
long ftell(FILE* stream);
void rewind(FILE* stream);

/* Error handling (C89 4.9.10). */
void clearerr(FILE* stream);
int feof(FILE* stream);
int ferror(FILE* stream);

#ifdef __cplusplus
}
#endif

#endif // MSL_STDIO_H
