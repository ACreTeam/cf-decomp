#include "FILE_POS.h"
#include "buffer_io.h"
#include "critical_regions.h"
#include "errno.h"

int _ftell(FILE* file)
{
    int charsInUndoBuffer = 0;
    int position;

    unsigned char tmp_kind = file->file_mode.file_kind;
    if (!(tmp_kind == file_disk || tmp_kind == file_console) || file->file_state.error)
    {
        errno = 0x28;
        return -1;
    }

    if (file->file_state.io_state == neutral)
        return file->position;

    position = file->buffer_position + (file->buffer_ptr - file->buffer);

    if (file->file_state.io_state >= rereading)
    {
        charsInUndoBuffer = file->file_state.io_state - rereading + 1;
        position -= charsInUndoBuffer;
    }

    if (!file->file_mode.binary_io)
    {
        int n = file->buffer_ptr - file->buffer - charsInUndoBuffer;
        unsigned char* p = file->buffer;

        while (n--)
            if (*p++ == '\n')
                position++;
    }

    return position;
}

long ftell(FILE* stream)
{
    int retval;

    __begin_critical_region(stdin_access);
    retval = _ftell(stream);
    __end_critical_region(stdin_access);
    return (long)retval;
}

int _fseek(FILE* file, fpos_t offset, int whence)
{
    int bufferCode;
    int pos;
    unsigned char* ptr;

    if (file->file_mode.file_kind != file_disk || file->file_state.error != 0)
    {
        errno = 0x28;
        return -1;
    }

    if (file->file_state.io_state == writing)
    {
        if (__flush_buffer(file, NULL) != 0)
        {
            set_error(file);
            errno = 0x28;
            return -1;
        }
    }

    if (whence == SEEK_CUR)
    {
        whence = SEEK_SET;
        pos = _ftell(file);
        offset += pos;
    }

    if ((whence != SEEK_END) && (file->file_mode.io_mode != __read_write) &&
        (file->file_state.io_state == reading || file->file_state.io_state == rereading))
    {
        if ((offset >= file->position) || !(offset >= file->buffer_position))
        {
            file->file_state.io_state = neutral;
        }
        else
        {
            file->buffer_ptr = file->buffer + (offset - file->buffer_position);
            file->buffer_length = file->position - offset;
            file->file_state.io_state = reading;
        }
    }
    else
    {
        file->file_state.io_state = neutral;
    }

    if (file->file_state.io_state == neutral)
    {
        if (file->position_fn != NULL &&
            (int)file->position_fn(file->handle, &offset, whence, file->idle_fn))
        {
            set_error(file);
            errno = 0x28;
            return -1;
        }
        else
        {
            file->file_state.eof = 0;
            file->position = offset;
            file->buffer_length = 0;
        }
    }

    return 0;
}

int fseek(FILE* stream, long offset, int whence)
{
    int code;

    __begin_critical_region(stdin_access);
    code = _fseek(stream, offset, whence);
    __end_critical_region(stdin_access);
    return code;
}

void rewind(FILE* stream)
{
    stream->file_state.error = 0;
    _fseek(stream, 0, SEEK_SET);
    stream->file_state.error = 0;
}
