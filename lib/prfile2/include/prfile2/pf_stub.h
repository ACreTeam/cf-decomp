#ifndef PRFILE2_PF_STUB_H
#define PRFILE2_PF_STUB_H

// Message passed to the PrFILE2 stub task (lib/prfile2/system/pf_stub.c); the stub entry points in lib/prfile2/stub fill one in.

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct PF_STUB_MSG {
    u32 _00;
    u32 func;       // 0x04
    s8 drive;       // 0x08
    u8 _09[3];
    u32 arg0;       // 0x0C
    u32 arg1;       // 0x10
    void* ptr0;     // 0x14
    void* ptr1;     // 0x18
    u8 _1C[0x18];
    s32 status;     // 0x34
    u8 _38[4];
    s32 result;     // 0x3C
    u8 _40[4];
};

s32 pfstub_com_massage(struct PF_STUB_MSG* p_msg);

struct PF_DRV_TBL;
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
