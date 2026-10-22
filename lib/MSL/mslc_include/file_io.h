#ifndef MSL_FILE_IO_H
#define MSL_FILE_IO_H

#include <file_struct.h>

#ifdef __cplusplus
extern "C" {
#endif

FILE* fopen(const char* filename, const char* mode);
FILE* freopen(const char* filename, const char* mode, FILE* stream);
int fclose(FILE* file);
int fflush(FILE* file);
int __get_file_modes(const char* mode, file_modes* modes);
int __msl_strnicmp(const char* str1, const char* str2, int n);
char* __msl_itoa(int value, char* str, unsigned int base);

#ifdef __cplusplus
}
#endif

#endif // MSL_FILE_IO_H
