#include "alloc.h"
#include "buffer_io.h"
#include "critical_regions.h"
#include "ctype.h"
#include "file_io.h"
#include "FILE_POS.h"
#include "misc_io.h"
#include "string.h"

int fclose(FILE* file)
{
    int flush_result, close_result;

    if (file == NULL)
        return -1;
    if (file->file_mode.file_kind == file_closed)
        return 0;

    flush_result = fflush(file);

    close_result = (*file->close_fn)(file->handle);

    file->file_mode.file_kind = file_closed;
    file->handle = NULL;

    if (file->file_state.free_buffer)
        free((FILE*)file->buffer);
    return (flush_result || close_result) ? -1 : 0;
}

int fflush(FILE* file)
{
    int pos;

    if (file == NULL)
    {
        return __flush_all();
    }

    if (file->file_state.error != 0 || file->file_mode.file_kind == file_closed)
    {
        return -1;
    }

    if (file->file_mode.io_mode == __read)
    {
        return 0;
    }

    if (file->file_state.io_state >= rereading)
    {
        file->file_state.io_state = reading;
    }

    if (file->file_state.io_state == reading)
    {
        file->buffer_length = 0;
    }

    if (file->file_state.io_state != writing)
    {
        file->file_state.io_state = neutral;
        return 0;
    }

    if (file->file_mode.file_kind != file_disk)
    {
        pos = 0;
    }
    else
    {
        pos = ftell(file);
    }

    if (__flush_buffer(file, NULL) != 0)
    {
        set_error(file);
        return -1;
    }

    file->file_state.io_state = neutral;
    file->position = pos;
    file->buffer_length = 0;
    return 0;
}

FILE* fopen(const char* filename, const char* mode)
{
    FILE* file;

    __begin_critical_region(files_access);
    file = freopen(filename, mode, __find_unopened_file());
    __end_critical_region(files_access);

    return file;
}

FILE* freopen(const char* filename, const char* mode, FILE* file)
{
    file_modes modes;

    __stdio_atexit();

    if (!file)
        return NULL;

    if (file && file->file_mode.file_kind != file_closed)
    {
        fflush(file);
        (*file->close_fn)(file->handle);
        file->file_mode.file_kind = file_closed;
        file->handle = NULL;
        if (file->file_state.free_buffer)
            free(file->buffer);
    }

    clearerr(file);

    if (!__get_file_modes(mode, &modes))
        return NULL;

    __init_file(file, modes, NULL, 0x400);

    if (__open_file(filename, modes, &file->handle))
    {
        file->file_mode.file_kind = file_closed;
        if (file->file_state.free_buffer)
            free(file->buffer);
        return NULL;
    }

    if (modes.io_mode & __append)
        fseek(file, 0, SEEK_END);

    return file;
}

int __get_file_modes(const char* mode, file_modes* modes)
{
    int mode_char;
    const char* mode_str;
    unsigned char open_mode;
    int io_mode;

    modes->file_kind = file_disk;
    modes->file_orientation = file_unoriented;
    modes->binary_io = 0;

    mode_char = *mode++;

    switch (mode_char)
    {
    case 'r':
        open_mode = __must_exist;
        break;
    case 'w':
        open_mode = __create_or_truncate;
        break;
    case 'a':
        open_mode = __create_if_necessary;
        break;
    default:
        return 0;
    }

    mode_str = mode + 1;
    modes->open_mode = open_mode;

    switch (*mode)
    {
    case 'b':
        modes->binary_io = 1;
        if (*mode_str == '+')
        {
            mode_char = (mode_char << 8) | '+';
        }
        break;
    case '+':
        mode_char = (mode_char << 8) | '+';
        if (*mode_str == 'b')
        {
            modes->binary_io = 1;
        }
        break;
    }

    switch (mode_char)
    {
    case 'r':
        io_mode = __read;
        break;
    case 'w':
        io_mode = __write;
        break;
    case 'a':
        io_mode = __write | __append;
        break;
    case ('r' << 8) | '+':
        io_mode = __read_write;
        break;
    case ('w' << 8) | '+':
        io_mode = __read_write;
        break;
    case ('a' << 8) | '+':
        io_mode = __read | __write | __append;
        break;
    }

    modes->io_mode = io_mode;
    return 1;
}

int __msl_strnicmp(const char* str1, const char* str2, int n)
{
    int i;
    char c1, c2;

    for (i = 0; i < n; i++)
    {
        c1 = tolower(*str1++);
        c2 = tolower(*str2++);

        if (c1 < c2)
            return -1;

        if (c1 > c2)
            return 1;

        if (c1 == '\0')
            return 0;
    }

    return 0;
}

char* __msl_itoa(int value, char* str, unsigned int base)
{
    int var_r7;
    int iStr;
    char c;
    int start;
    int end;

    var_r7 = 0;
    iStr = 0;

    if (value < 0)
    {
        value = -value;
        var_r7 = 1;
    }

    do
    {
        int temp_r9 = value % base;

        if (temp_r9 > 9)
            str[iStr++] = temp_r9 + 0x37;
        else
            str[iStr++] = temp_r9 + 0x30;

        value /= base;
    } while (value != 0);

    if (var_r7 != 0)
        str[iStr++] = '-';

    str[iStr++] = '\0';

    start = 0;
    end = strlen(str) - 1;

    while (start < end)
    {
        c = str[start];
        str[start++] = str[end];
        str[end--] = c;
    }

    return str;
}
