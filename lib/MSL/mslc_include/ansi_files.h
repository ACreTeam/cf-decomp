#ifndef MSL_ANSI_FILES_H
#define MSL_ANSI_FILES_H

#include <MetroTRK/trkenum.h>
#include <revolution/types.h>
#include <size_t.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct _FILE FILE;

    typedef u32 __file_handle;
    typedef u32 fpos_t;
    typedef void (*__idle_proc)(void);

#define set_error(file)                                                                            \
    do                                                                                             \
    {                                                                                              \
        (file)->file_state.error = 1;                                                              \
        (file)->buffer_length = 0;                                                                 \
    } while (0)

    typedef struct _file_modes
    {
        unsigned int open_mode : 2;
        unsigned int io_mode : 3;
        unsigned int buffer_mode : 2;
        unsigned int file_kind : 3;
        unsigned int file_orientation : 2;
        unsigned int binary_io : 1;
    } file_modes;

    enum __open_modes
    {
        __must_exist,
        __create_if_necessary,
        __create_or_truncate
    };

    enum __io_modes
    {
        __read = 1,
        __write = 2,
        __read_write = 3,
        __append = 4
    };

    enum __io_results
    {
        __no_io_error,
        __io_error,
        __io_EOF
    };

#define _IONBF 0
#define _IOLBF 1
#define _IOFBF 2

    DSIOResult __position_file(__file_handle handle, fpos_t* position, int mode, __idle_proc idle_proc);
    DSIOResult __close_file(__file_handle file);
    DSIOResult __open_file(const char* name, file_modes mode, __file_handle* handle);
    DSIOResult __write_file(__file_handle file, unsigned char* buf, size_t* count, __idle_proc idle_fn);
    DSIOResult __read_file(__file_handle file, unsigned char* buf, size_t* count, __idle_proc idle_fn);
    DSIOResult __close_console(__file_handle file);
    DSIOResult __TRK_write_console(__file_handle file, unsigned char* buffer, size_t* count, __idle_proc idle_fn);
    DSIOResult __write_console(__file_handle file, unsigned char* buffer, size_t* count, __idle_proc idle_fn);
    DSIOResult __read_console(__file_handle file, unsigned char* buffer, size_t* count, __idle_proc idle_fn);
    FILE* __find_unopened_file(void);
    void __init_file(FILE* file, file_modes mode, char* buff, size_t size);
    void __close_all(void);
    int __flush_line_buffered_output_files(void);
    unsigned int __flush_all(void);

#ifdef __cplusplus
}
#endif

#endif // MSL_ANSI_FILES_H
