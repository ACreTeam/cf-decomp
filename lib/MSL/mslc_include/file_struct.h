#ifndef FILE_STRUCT_H
#define FILE_STRUCT_H

#include <ansi_files.h>
#include <stdio.h>
#include <wchar_t.h>

enum io_states {
    neutral,
    writing,
    reading,
    rereading
};

enum file_kinds {
    file_closed,
    file_disk,
    file_console,
    file_unavailable
};

enum file_orientation {
    file_unoriented,
    file_char_oriented,
    file_wide_oriented
};

typedef struct {
    unsigned int io_state : 3;
    unsigned int free_buffer : 1;
    unsigned char eof;
    unsigned char error;
} file_state;

typedef DSIOResult (*__pos_proc)(__file_handle, fpos_t*, int, __idle_proc);
typedef DSIOResult (*__io_proc)(__file_handle, unsigned char*, size_t*, __idle_proc);
typedef DSIOResult (*__close_proc)(__file_handle);

struct _FILE {
    __file_handle handle;
    file_modes file_mode;
    file_state file_state;
    unsigned char is_dynamically_allocated;
    unsigned char char_buffer;
    unsigned char char_buffer_overflow;
    unsigned char ungetc_buffer[2];
    wchar_t ungetc_wide_buffer[2];
    unsigned long position;
    unsigned char* buffer;
    unsigned long buffer_size;
    unsigned char* buffer_ptr;
    unsigned long buffer_length;
    unsigned long buffer_alignment;
    unsigned long save_buffer_length;
    unsigned long buffer_position;
    __pos_proc position_fn;
    __io_proc read_fn;
    __io_proc write_fn;
    __close_proc close_fn;
    __idle_proc idle_fn;
    struct _FILE* next_file;
};

extern FILE __files[];

#define stdin  (&__files[0])
#define stdout (&__files[1])
#define stderr (&__files[2])

#endif
