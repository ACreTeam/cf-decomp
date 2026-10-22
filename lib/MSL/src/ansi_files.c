#include <ansi_files.h>
#include <file_struct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char stdin_buff[0x100];
static unsigned char stdout_buff[0x100];
static unsigned char stderr_buff[0x100];

FILE __files[4] = {
    {
        0,
        { __must_exist, __read, _IOLBF, file_console, file_unoriented, 0 },
        { neutral, 0, 0, 0 },
        0,
        0,
        0,
        { 0, 0 },
        { 0, 0 },
        0,
        stdin_buff,
        sizeof(stdin_buff),
        stdin_buff,
        0,
        0,
        0,
        0,
        0,
        __read_console,
        __write_console,
        __close_console,
        0,
        &__files[1],
    },
    {
        1,
        { __must_exist, __write, _IOLBF, file_console, file_unoriented, 0 },
        { neutral, 0, 0, 0 },
        0,
        0,
        0,
        { 0, 0 },
        { 0, 0 },
        0,
        stdout_buff,
        sizeof(stdout_buff),
        stdout_buff,
        0,
        0,
        0,
        0,
        0,
        __read_console,
        __write_console,
        __close_console,
        0,
        &__files[2],
    },
    {
        2,
        { __must_exist, __write, _IONBF, file_console, file_unoriented, 0 },
        { neutral, 0, 0, 0 },
        0,
        0,
        0,
        { 0, 0 },
        { 0, 0 },
        0,
        stderr_buff,
        sizeof(stderr_buff),
        stderr_buff,
        0,
        0,
        0,
        0,
        0,
        __read_console,
        __write_console,
        __close_console,
        0,
        &__files[3],
    },
};

FILE* __find_unopened_file(void)
{
    FILE* file = __files[2].next_file;
    FILE* last_file;

    while (file != 0)
    {
        if (file->file_mode.file_kind == file_closed)
        {
            return file;
        }
        else
        {
            last_file = file;
            file = file->next_file;
        }
    }

    if ((file = (FILE*)malloc(sizeof(FILE))) != 0)
    {
        memset(file, 0, sizeof(FILE));
        file->is_dynamically_allocated = 1;
        last_file->next_file = file;
        return file;
    }

    return 0;
}

void __init_file(FILE* file, file_modes mode, char* buff, size_t size)
{
    file->handle = 0;
    file->file_mode = mode;
    file->file_state.io_state = neutral;
    file->file_state.free_buffer = 0;
    file->file_state.eof = 0;
    file->file_state.error = 0;
    file->position = 0;

    if (size)
        setvbuf(file, buff, _IOFBF, size);
    else
        setvbuf(file, 0, _IONBF, 0);

    file->buffer_ptr = file->buffer;
    file->buffer_length = 0;

    if (file->file_mode.file_kind == file_disk)
    {
        file->position_fn = __position_file;
        file->read_fn = __read_file;
        file->write_fn = __write_file;
        file->close_fn = __close_file;
    }

    file->idle_fn = 0;
}

void __close_all(void)
{
    FILE* file = &__files[0];
    FILE* last_file;

    while (file != 0)
    {
        if (file->file_mode.file_kind != file_closed)
        {
            fclose(file);
        }

        last_file = file;
        file = file->next_file;

        if (last_file->is_dynamically_allocated)
        {
            free(last_file);
        }
        else
        {
            last_file->file_mode.file_kind = file_unavailable;
            if (file != 0 && file->is_dynamically_allocated)
            {
                last_file->next_file = 0;
            }
        }
    }
}

int __flush_line_buffered_output_files(void)
{
    int result = 0;
    FILE* file = &__files[0];

    while (file != 0)
    {
        if (file->file_mode.file_kind != file_closed && (file->file_mode.buffer_mode & _IOLBF) && file->file_state.io_state == writing)
        {
            if (fflush(file))
            {
                result = EOF;
            }
        }
        file = file->next_file;
    }

    return result;
}

unsigned int __flush_all(void)
{
    unsigned int result = 0;
    FILE* file = &__files[0];

    while (file != 0)
    {
        if (file->file_mode.file_kind != file_closed && fflush(file))
        {
            result = -1;
        }
        file = file->next_file;
    }

    return result;
}
