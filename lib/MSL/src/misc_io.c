#include <misc_io.h>
#include <file_struct.h>
#include <abort_exit.h>
#include <ansi_files.h>

void clearerr(FILE* file)
{
    file->file_state.eof = 0;
    file->file_state.error = 0;
}

void __stdio_atexit(void)
{
    __stdio_exit = __close_all;
}
