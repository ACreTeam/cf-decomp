#ifndef PRFILE2_PF_STANDARD_H
#define PRFILE2_PF_STANDARD_H

// The pf2_* API (lib/prfile2/standard) that the stub task calls (lib/prfile2/system/pf_stub_standard.c).

#include <prfile2/pf_api.h>

#ifdef __cplusplus
extern "C" {
#endif

struct PF_FILE* pf2_create(const s8* path, s32 mode);
struct PF_FILE* pf2_fopen(const s8* path, const s8* mode);
s32 pf2_fclose(struct PF_FILE* p_file);
u32 pf2_fread(void* p_buf, u32 size, u32 count, struct PF_FILE* p_file);
u32 pf2_fwrite(void* p_buf, u32 size, u32 count, struct PF_FILE* p_file);
s32 pf2_fseek(struct PF_FILE* p_file, s32 offset, s32 origin);
s32 pf2_remove(const s8* path);
s32 pf2_rename(const s8* path, const s8* new_name);
s32 pf2_mkdir(const s8* path);
s32 pf2_rmdir(const s8* path);
s32 pf2_chdir(const s8* path);
s32 pf2_fstat(const s8* path, struct PF_STAT* p_stat);
s32 pf2_chmod(const s8* path, u32 attr);
s32 pf2_mount(s8 drv_char);
s32 pf2_format(s8 drv_char, const s8* param);
s32 pf2_buffering(s8 drv_char, s32 mode);
s32 pf2_ferror(struct PF_FILE* p_file);
s32 pf2_feof(struct PF_FILE* p_file);
s32 pf2_errnum(void);
s32 pf2_devinf(s8 drv_char, struct PF_DEV_INF* p_dev_inf);
s32 pf2_setvol(s8 drv_char, const s8* name);
s32 pf2_getvol(s8 drv_char, struct PF_VOLTAB* p_vol_tab);
s32 pf2_rmvvol(s8 drv_char);
s32 pf2_chdmod(const s8* path, u32 attr);
s32 pf2_fconcat(const s8* path1, const s8* path2);
s32 pf2_attach(struct PF_DRV_TBL** drv_tbl, struct PF_ATTACH_PARAM* p_param);
s32 pf2_fsfirst(const s8* path, u8 attr, struct PF_DTA* p_dta);
s32 pf2_fsnext(struct PF_DTA* p_dta);
s32 pf2_setupfsi(s8 drv_char, s16 mode);
s32 pf2_setclstlink(s8 drv_char, s32 mode, struct PF_CLUSTER_LINK_RES* p_link);
s32 pf2_fsetclstlink(struct PF_FILE* p_file, s32 mode, struct PF_CLUSTER_LINK_RES* p_link);
s32 pf2_sync(s8 drv_char, s32 mode);
s32 pf2_fsync(struct PF_FILE* p_file);
u32 pf2_fappend(struct PF_FILE* p_file, u32 size);
s32 pf2_settailbuf(s8 drv_char, s32 size, u32* p_buf);
s32 pf2_derrnum(s8 drv_char);
s32 pf2_fsexec(struct PF_DTA* p_dta, s32 type, u8 param);
s32 pf2_unmount(s8 drv_char, u32 mode);
s32 pf2_combine(const s8* path1, const s8* path2);
s32 pf2_detach(s8 drv_char);
s32 pf2_divide(const s8* path1, const s8* path2, u32 offset);
s32 pf2_xdivide(const s8* path1, const s8* path2, u32 offset);
s32 pf2_fadjust(struct PF_FILE* p_file);
s32 pf2_finfo(struct PF_FILE* p_file, struct PF_INFO* p_info);
s32 pf2_move(const s8* path1, const s8* path2);
u32 pf2_cinsert(const s8* path, u32 start, u32 size);
s32 pf2_insert(const s8* path, u32 size);
u32 pf2_cdelete(const s8* path, u32 start, u32 size);
s32 pf2_cut(const s8* path, u32 size);
s32 pf2_setvolcfg(s8 drv_char, struct PF_VOL_CFG* p_config);
s32 pf2_getvolcfg(s8 drv_char, struct PF_VOL_CFG* p_config);
s32 pf2_setcode(struct PF_CHARCODE* p_codeset);
struct PF_DIR* pf2_opendir(const s8* path);
s32 pf2_closedir(struct PF_DIR* p_dir);
s32 pf2_readdir(struct PF_DIR* p_dir, struct PF_DIRENT* p_dirent);
s32 pf2_telldir(struct PF_DIR* p_dir, u32* p_offset);
s32 pf2_seekdir(struct PF_DIR* p_dir, s32 offset);
s32 pf2_rewinddir(struct PF_DIR* p_dir);
s32 pf2_fchdir(struct PF_DIR* p_dir);
s32 pf2_regctx(void);
s32 pf2_unregctx(void);
s32 pf2_flock(struct PF_FILE* p_file, s32 mode);
s32 pf2_setencode(s32 encode);
s32 pf2_createdir(const s8* path, s32 option, struct PF_DTA* p_dta);
s32 pf2_iswriteprotected(s8 drv_char);

struct PF_FILE* pf2_w_create(const u16* path, s32 mode);
struct PF_FILE* pf2_w_fopen(const u16* path, const u16* mode);
s32 pf2_w_remove(const u16* path);
s32 pf2_w_rename(const u16* path, const u16* new_name);
s32 pf2_w_mkdir(const u16* path);
s32 pf2_w_rmdir(const u16* path);
s32 pf2_w_chdir(const u16* path);
s32 pf2_w_fstat(const u16* path, struct PF_STAT* p_stat);
s32 pf2_w_chmod(const u16* path, u32 attr);
s32 pf2_w_chdmod(const u16* path, u32 attr);
s32 pf2_w_fconcat(const u16* path1, const u16* path2);
s32 pf2_w_fsfirst(const u16* path, u8 attr, struct PF_DTA* p_dta);
s32 pf2_w_fsnext(struct PF_DTA* p_dta);
s32 pf2_w_fsexec(struct PF_DTA* p_dta, s32 type, u8 param);
s32 pf2_w_combine(const u16* path1, const u16* path2);
s32 pf2_w_divide(const u16* path1, const u16* path2, u32 offset);
s32 pf2_w_xdivide(const u16* path1, const u16* path2, u32 offset);
s32 pf2_w_move(const u16* path1, const u16* path2);
u32 pf2_w_cinsert(const u16* path, u32 start, u32 size);
s32 pf2_w_insert(const u16* path, u32 size);
u32 pf2_w_cdelete(const u16* path, u32 start, u32 size);
s32 pf2_w_cut(const u16* path, u32 size);
struct PF_DIR* pf2_w_opendir(const u16* path);
s32 pf2_w_createdir(const u16* path, s32 option, struct PF_DTA* p_dta);

#ifdef __cplusplus
}
#endif
#endif
