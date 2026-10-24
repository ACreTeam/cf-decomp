#ifndef PRFILE2_PF_STUB_H
#define PRFILE2_PF_STUB_H

// Message passed to the PrFILE2 stub task (lib/prfile2/system/pf_stub.c); the stub entry points in lib/prfile2/stub fill one in.

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    PFSTUB_CREATE = 0,
    PFSTUB_FOPEN = 1,
    PFSTUB_FCLOSE = 2,
    PFSTUB_FREAD = 3,
    PFSTUB_FWRITE = 4,
    PFSTUB_FSEEK = 5,
    PFSTUB_RESERVED_6 = 6,
    PFSTUB_REMOVE = 7,
    PFSTUB_RENAME = 8,
    PFSTUB_MKDIR = 9,
    PFSTUB_RMDIR = 10,
    PFSTUB_CHDIR = 11,
    PFSTUB_FSTAT = 12,
    PFSTUB_CHMOD = 13,
    PFSTUB_MOUNT = 14,
    PFSTUB_FORMAT = 15,
    PFSTUB_BUFFERING = 16,
    PFSTUB_FERROR = 17,
    PFSTUB_FEOF = 18,
    PFSTUB_ERRNUM = 19,
    PFSTUB_DEVINF = 20,
    PFSTUB_SETVOL = 21,
    PFSTUB_GETVOL = 22,
    PFSTUB_RMVVOL = 23,
    PFSTUB_CHDMOD = 24,
    PFSTUB_FCONCAT = 25,
    PFSTUB_ATTACH = 27,
    PFSTUB_FSFIRST = 28,
    PFSTUB_FSNEXT = 29,
    PFSTUB_SETUPFSI = 30,
    PFSTUB_SETCLSTLINK = 31,
    PFSTUB_FSETCLSTLINK = 32,
    PFSTUB_SYNC = 33,
    PFSTUB_FSYNC = 34,
    PFSTUB_FAPPEND = 35,
    PFSTUB_SETTAILBUF = 36,
    PFSTUB_DERRNUM = 37,
    PFSTUB_FSEXEC = 38,
    PFSTUB_UNMOUNT = 39,
    PFSTUB_COMBINE = 40,
    PFSTUB_DETACH = 41,
    PFSTUB_DIVIDE = 42,
    PFSTUB_XDIVIDE = 43,
    PFSTUB_FADJUST = 44,
    PFSTUB_FINFO = 45,
    PFSTUB_MOVE = 46,
    PFSTUB_CINSERT = 47,
    PFSTUB_INSERT = 48,
    PFSTUB_CDELETE = 49,
    PFSTUB_CUT = 50,
    PFSTUB_SETVOLCFG = 51,
    PFSTUB_GETVOLCFG = 52,
    PFSTUB_SETCODE = 53,
    PFSTUB_OPENDIR = 54,
    PFSTUB_CLOSEDIR = 55,
    PFSTUB_READDIR = 56,
    PFSTUB_TELLDIR = 57,
    PFSTUB_SEEKDIR = 58,
    PFSTUB_REWINDDIR = 59,
    PFSTUB_FCHDIR = 60,
    PFSTUB_REGCTX = 61,
    PFSTUB_UNREGCTX = 62,
    PFSTUB_FLOCK = 63,
    PFSTUB_SETENCODE = 64,
    PFSTUB_CREATEDIR = 65,
    PFSTUB_ISWRITEPROTECTED = 80,
    PFSTUB_STANDARD_END = 100,

    PFSTUB_W_CREATE = 100,
    PFSTUB_W_FOPEN = 101,
    PFSTUB_W_REMOVE = 102,
    PFSTUB_W_RENAME = 103,
    PFSTUB_W_MKDIR = 104,
    PFSTUB_W_RMDIR = 105,
    PFSTUB_W_CHDIR = 106,
    PFSTUB_W_FSTAT = 107,
    PFSTUB_W_CHMOD = 108,
    PFSTUB_W_CHDMOD = 109,
    PFSTUB_W_FCONCAT = 110,
    PFSTUB_W_FSFIRST = 111,
    PFSTUB_W_FSNEXT = 112,
    PFSTUB_W_FSEXEC = 113,
    PFSTUB_W_COMBINE = 114,
    PFSTUB_W_DIVIDE = 115,
    PFSTUB_W_XDIVIDE = 116,
    PFSTUB_W_MOVE = 117,
    PFSTUB_W_CINSERT = 118,
    PFSTUB_W_INSERT = 119,
    PFSTUB_W_CDELETE = 120,
    PFSTUB_W_CUT = 121,
    PFSTUB_W_OPENDIR = 122,
    PFSTUB_W_CREATEDIR = 123,
    PFSTUB_UNICODE_END = 200,
};

struct PDM_DISK;
struct PDM_PARTITION;
struct PF_DRV_TBL;

typedef void (*PF_STUB_CALLBACK)(s32 result, void* param);

struct PF_STUB_MSG {
    s32 tsk_id;                     // 0x00
    s32 func;                       // 0x04
    s8 drive;                       // 0x08
    s32 arg0;                       // 0x0C
    s32 arg1;                       // 0x10
    void* ptr0;                     // 0x14
    void* ptr1;                     // 0x18
    struct PF_DRV_TBL* p_drv_tbl;   // 0x1C
    u8 _20[4];
    s32 sem_id;                     // 0x24
    s32* p_result;                  // 0x28
    struct PDM_DISK* p_disk;        // 0x2C
    struct PDM_PARTITION* p_part;   // 0x30
    PF_STUB_CALLBACK callback;      // 0x34
    void* callback_param;           // 0x38
    s32 result;                     // 0x3C
    u8 _40[4];
};

void pfstub_init_stub(void);
s32 pfstub_com_massage(struct PF_STUB_MSG* p_msg);
s32 pfstub_store_result(struct PF_STUB_MSG* p_msg);
void pfstub_call_function_standard(struct PF_STUB_MSG* p_msg);
void pfstub_call_function_unicode(struct PF_STUB_MSG* p_msg);

struct PF_DTA;
struct PF_FILE;

s32 pfstub_attach(struct PF_DRV_TBL** drv_tbl, void* p_param);
struct PF_FILE* pfstub_create(const s8* path, u32 mode);
s32 pfstub_detach(s8 drive);
s32 pfstub_errnum(void);
s32 pfstub_fclose(struct PF_FILE* p_file);
s32 pfstub_fsfirst(const s8* path, u32 attr, struct PF_DTA* p_dta);
s32 pfstub_fsnext(struct PF_DTA* p_dta);
u32 pfstub_fwrite(const void* p_buf, u32 size, u32 count, struct PF_FILE* p_file);
s32 pfstub_mkdir(const s8* path);
s32 pfstub_mount(s8 drive);
s32 pfstub_remove(const s8* path);
s32 pfstub_rmdir(const s8* path);
s32 pfstub_unmount(s8 drive, u32 mode);
s32 pfstub_init_prfile2(u32 config, void* param);

#ifdef __cplusplus
}
#endif
#endif
