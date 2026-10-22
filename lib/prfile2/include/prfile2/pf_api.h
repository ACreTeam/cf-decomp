#ifndef PRFILE2_PF_API_H
#define PRFILE2_PF_API_H

// Internal interface of the standalone PrFILE2 ("fa") library used by its API wrapper layer (lib/prfile2/standard).
// Only what the wrappers need is declared; the layouts of the volume/file structures are not recovered yet.

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct PF_DIR;
struct PF_DTA;
struct PF_FILE;
struct PF_STAT;

struct PF_STR {
    const s8* p_head;   // 0x00
    const s8* p_tail;   // 0x04
    const s8* p_local;  // 0x08
    u32 code_mode;      // 0x0C
};

struct PF_VOLUME_SET {
    u8 _00[0x40];
    s32 last_error;  // 0x40
    u8 _44[0x20];
    s32 context_id;  // 0x64
    // ...
};

extern struct PF_VOLUME_SET pf_vol_set;

s32 pf2_init_prfile2(u32 config, void* param);

// common/pf_str.c
s32 PFSTR_InitStr(struct PF_STR* p_str, const s8* s, u32 code_mode);

// standard/pf_api_util.c
s32 PFAPI_convertReturnValue(s32 err);
s32 PFAPI_convertError(s32 err);
s32 PFAPI_convertDriverError(s32 err);
s32 PFAPI_convertReturnValue4feof(s32 err);

// fatfs
struct PF_DRV_TBL;
struct PF_ATTACH_PARAM {
    u8 _00[0x50];
    void* param;  // 0x50
};
s32 PFVOL_attach(struct PF_DRV_TBL* p_drv, struct PF_ATTACH_PARAM* p_param, void* param);
s32 PFAPI_ParseOpenModeString(const s8* mode_str);
s32 PFWAPI_ParseOpenModeString(const s8* mode_str);
s32 PFVOL_setupfsi(s8 drv_char, u16 mode);
s32 PFVOL_iswriteprotected(s8 drv_char, u32* p_result);
s32 PFFILE_fappend(struct PF_FILE* p_file, u32 size, u32* p_result);
s32 PFFILE_cdelete(struct PF_STR* p_path, u32 start, u32 size, u32* p_result);
s32 PFFILE_cinsert(struct PF_STR* p_path, u32 start, u32 size, u32* p_result);
s32 PFFILE_divide(struct PF_STR* p1, struct PF_STR* p2, u32 size);
s32 PFFILE_fopen(struct PF_STR* p_path, u32 mode, struct PF_FILE** pp_file);
s32 PFDIR_createdir(struct PF_STR* p_path, u32 a, u32 b);
s32 PFDIR_fsfirst(struct PF_STR* p_path, u8 attr, struct PF_DTA* p_dta);
s32 PFDIR_opendir(struct PF_STR* p_path, struct PF_DIR** pp_dir);
void* PFAPI_convertReturnValue2NULL(s32 err, void* p_stream);
s32 PFVOL_buffering(s8 drv_char, u8 mode);
s32 PFVOL_derrnum(s8 drv_char);
s32 PFVOL_getvol(s8 drv_char);
s32 PFVOL_getvolcfg(s8 drv_char);
s32 PFVOL_mount(s8 drv_char);
s32 PFVOL_regctx(s32 context_id);
s32 PFVOL_rmvvol(s8 drv_char);
s32 PFVOL_setclstlink(s8 drv_char);
s32 PFVOL_setcode(u32 code);
s32 PFVOL_setencode(u32 code);
s32 PFVOL_settailbuf(s8 drv_char);
s32 PFVOL_setvolcfg(s8 drv_char);
s32 PFVOL_sync(s8 drv_char, s32 mode);
s32 PFVOL_unregctx(s32 context_id);
s32 PFVOL_setvol(s8 drv_char, struct PF_STR* p_str);

s32 PFFILE_fadjust(struct PF_FILE* p_file);
s32 PFFILE_feof(struct PF_FILE* p_file);
s32 PFFILE_ferror(struct PF_FILE* p_file);
s32 PFFILE_flock(struct PF_FILE* p_file, u32 mode);
s32 PFFILE_fsetclstlink(struct PF_FILE* p_file);
s32 PFFILE_fsync(struct PF_FILE* p_file);
s32 PFFILE_remove(struct PF_STR* p_path);
s32 PFFILE_combine(struct PF_STR* p1, struct PF_STR* p2);

s32 PFDIR_chdir(struct PF_STR* p_path);
s32 PFDIR_chdmod(struct PF_STR* p_path, u8 attr);
s32 PFDIR_chmod(struct PF_STR* p_path, u8 attr);
s32 PFDIR_closedir(struct PF_DIR* p_dir);
s32 PFDIR_fchdir(struct PF_FILE* p_file);
s32 PFDIR_fsexec(struct PF_DTA* p_dta);
s32 PFDIR_fsnext(struct PF_DTA* p_dta);
s32 PFDIR_fstat(struct PF_STR* p_path, struct PF_STAT* p_stat);
s32 PFDIR_mkdir(struct PF_STR* p_path);
s32 PFDIR_move(struct PF_STR* p_src, struct PF_STR* p_dst);
s32 PFDIR_readdir(struct PF_DIR* p_dir);
s32 PFDIR_rename(struct PF_STR* p_src, struct PF_STR* p_dst);
s32 PFDIR_rmdir(struct PF_STR* p_path);
s32 PFDIR_seekdir(struct PF_DIR* p_dir, u32 pos);
s32 PFDIR_telldir(struct PF_DIR* p_dir);

#ifdef __cplusplus
}
#endif
#endif
