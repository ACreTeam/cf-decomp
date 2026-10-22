#ifndef MSL_CHAR_IO_H
#define MSL_CHAR_IO_H

#include <file_struct.h>

#ifdef __cplusplus
extern "C" {
#endif

int __get_char(FILE* file);
int fgetc(FILE* file);
int __put_char(int c, FILE* stream);
int fputs(const char* s, FILE* stream);

#ifdef __cplusplus
}
#endif

#endif // MSL_CHAR_IO_H
