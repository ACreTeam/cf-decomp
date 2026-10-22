#include "buffer_io.h"
#include "stdio.h"
#include "stdlib.h"

void __prep_buffer(FILE* file)
{
    file->buffer_ptr = file->buffer;
    file->buffer_length = file->buffer_size;
    file->buffer_length = file->buffer_length - (file->position & file->buffer_alignment);
    file->buffer_position = file->position;
    return;
}

int __load_buffer(FILE* file, size_t* bytes_loaded, int alignment)
{
    FILE* f;
    size_t* loaded;
    int ioresult;
    long n;
    unsigned char* p;

    f = file;
    loaded = bytes_loaded;

    __prep_buffer(f);

    if (alignment == __dont_align_buffer)
    {
        f->buffer_length = f->buffer_size;
    }

    ioresult = (*f->read_fn)(f->handle, f->buffer, (size_t*)&f->buffer_length, f->idle_fn);

    if (ioresult == __io_EOF)
    {
        f->buffer_length = 0;
    }

    if (loaded != NULL)
    {
        *loaded = f->buffer_length;
    }

    if (ioresult != __no_io_error)
    {
        return ioresult;
    }

    f->position += f->buffer_length;

    if (!f->file_mode.binary_io)
    {
        n = f->buffer_length;
        p = f->buffer;

        while (n--)
        {
            if (*p++ == '\n')
            {
                f->position++;
            }
        }
    }

    return __no_io_error;
}

int __flush_buffer(FILE* file, size_t* length)
{
    size_t bufferLen;
    int writeCode;

    bufferLen = file->buffer_ptr - file->buffer;
    if (bufferLen)
    {
        file->buffer_length = bufferLen;
        writeCode = file->write_fn(file->handle, file->buffer, &file->buffer_length, file->idle_fn);
        if (length)
        {
            *length = file->buffer_length;
        }
        if (writeCode)
        {
            return writeCode;
        }
        file->position += file->buffer_length;
    }

    file->buffer_ptr = file->buffer;
    file->buffer_length = file->buffer_size;
    file->buffer_length = file->buffer_length - (file->position & file->buffer_alignment);
    file->buffer_position = file->position;
    return 0;
}

int setvbuf(FILE* file, char* buff, int mode, size_t size)
{
    int kind;

    kind = file->file_mode.file_kind;

    if (mode == _IONBF)
        fflush(file);

    if (file->file_state.io_state != neutral || kind == file_closed)
        return -1;

    if (mode != _IONBF && mode != _IOLBF && mode != _IOFBF)
        return -1;

    if (file->buffer && file->file_state.free_buffer)
    {
        free(file->buffer);
    }

    file->file_mode.buffer_mode = mode;
    file->file_state.free_buffer = 0;
    file->buffer = (unsigned char*)&file->char_buffer;
    file->buffer_ptr = (unsigned char*)&file->char_buffer;
    file->buffer_size = 1;
    file->buffer_length = 0;
    file->buffer_alignment = 0;

    if (mode == _IONBF || size < 1)
    {
        *(file->buffer_ptr) = '\0';
        return 0;
    }

    if (!buff)
    {
        if (!(buff = (char*)malloc(size)))
        {
            return -1;
        }

        file->file_state.free_buffer = 1;
    }

    file->buffer = (unsigned char*)buff;
    file->buffer_ptr = file->buffer;
    file->buffer_size = size;
    file->buffer_alignment = 0;

    return 0;
}
