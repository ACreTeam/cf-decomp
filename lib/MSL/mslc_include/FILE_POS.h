#ifndef MSL_FILE_POS_H
#define MSL_FILE_POS_H

#include <file_struct.h>

#ifdef __cplusplus
extern "C" {
#endif

int _ftell(FILE* file);
long ftell(FILE* stream);
int _fseek(FILE* file, fpos_t offset, int whence);
int fseek(FILE* stream, long offset, int whence);
void rewind(FILE* stream);

#ifdef __cplusplus
}
#endif

#endif // MSL_FILE_POS_H
