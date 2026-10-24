#ifndef PRFILE2_PF_DIR_H
#define PRFILE2_PF_DIR_H

#include <prfile2/pf_types.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    PF_DTA_STAT_OEM = 0x1,
    PF_DTA_STAT_UNICODE = 0x2,
};

enum {
    PF_FSEXEC_REMOVE = 0x01,
    PF_FSEXEC_CHMOD = 0x02,
    PF_FSEXEC_CHDMOD = 0x04,
    PF_FSEXEC_FOPEN = 0x10,
    PF_FSEXEC_OPENDIR = 0x20,
    PF_FSEXEC_DIRECT = 0x80000000,
};

s32 PFDIR_CheckDirIsEmpty(struct PF_ENT_ITER* p_iter, BOOL* p_is_empty);
struct PF_SDD* PFDIR_GetSDD(struct PF_VOLUME* p_vol, struct PF_DIR_ENT* p_ent);
s32 PFDIR_DoFsexecOpenDir(struct PF_DIR_ENT* p_ent, struct PF_ENT_ITER* p_iter, struct PF_DIR** pp_dir);
s32 PFDIR_p_opendir(struct PF_VOLUME* p_vol, struct PF_STR* p_path, struct PF_DIR** pp_dir);
s32 PFDIR_p_readdir(struct PF_DIR* p_dir, struct PF_DIRENT* p_dirent);
s32 PFDIR_p_mkdir(struct PF_VOLUME* p_vol, struct PF_STR* p_path, u32 option, struct PF_DTA* p_dta);
s32 PFDIR_p_chdir(struct PF_VOLUME* p_vol, struct PF_STR* p_path);
s32 PFDIR_p_chmod(struct PF_VOLUME* p_vol, struct PF_STR* p_path, u8 attr);
s32 PFDIR_p_rmdir(struct PF_VOLUME* p_vol, struct PF_STR* p_path);
s32 PFDIR_p_fstat(struct PF_VOLUME* p_vol, struct PF_STR* p_path, struct PF_STAT* p_stat);
s32 PFDIR_p_rename(struct PF_VOLUME* p_vol, struct PF_STR* p_old_path, struct PF_STR* p_new_path);
s32 PFDIR_p_move(struct PF_VOLUME* p_vol, struct PF_STR* p_src_path, struct PF_STR* p_dst_path);
s32 PFDIR_p_fsnext(struct PF_DTA* p_dta);
s32 PFDIR_p_fsfirst(struct PF_VOLUME* p_vol, struct PF_STR* p_path, u8 attr, struct PF_DTA* p_dta);
s32 PFDIR_p_fsexec_chmod(struct PF_DTA* p_dta, struct PF_DIR_ENT* p_ent, u32 type, u8 attr);
s32 PFDIR_p_fsexec_remove(struct PF_DTA* p_dta, struct PF_DIR_ENT* p_ent, struct PF_ENT_ITER* p_iter);
s32 PFDIR_p_fsexec_fopen(struct PF_DTA* p_dta, struct PF_DIR_ENT* p_ent, struct PF_ENT_ITER* p_iter, u32 type, u32 mode);
s32 PFDIR_p_fsexec_opendir(struct PF_DTA* p_dta, struct PF_DIR_ENT* p_ent, struct PF_ENT_ITER* p_iter, u32 type);
s32 PFDIR_p_fsexec(struct PF_DTA* p_dta, u32 type, u8 param);
void PFDIR_FinalizeAllDirs(struct PF_VOLUME* p_vol);
s32 PFDIR_fsfirst(struct PF_STR* p_path, u8 attr, struct PF_DTA* p_dta);
s32 PFDIR_fsnext(struct PF_DTA* p_dta);
s32 PFDIR_fsexec(struct PF_DTA* p_dta, u32 type, u8 param);
s32 PFDIR_fstat(struct PF_STR* p_path, struct PF_STAT* p_stat);
s32 PFDIR_rename(struct PF_STR* p_old_path, struct PF_STR* p_new_path);
s32 PFDIR_move(struct PF_STR* p_src_path, struct PF_STR* p_dst_path);
s32 PFDIR_opendir(struct PF_STR* p_path, struct PF_DIR** pp_dir);
s32 PFDIR_closedir(struct PF_DIR* p_dir);
s32 PFDIR_readdir(struct PF_DIR* p_dir, struct PF_DIRENT* p_dirent);
s32 PFDIR_telldir(struct PF_DIR* p_dir, u32* p_offset);
s32 PFDIR_seekdir(struct PF_DIR* p_dir, u32 offset);
s32 PFDIR_rmdir(struct PF_STR* p_path);
s32 PFDIR_mkdir(struct PF_STR* p_path_str);
s32 PFDIR_createdir(struct PF_STR* p_path, u32 option, struct PF_DTA* p_dta);
s32 PFDIR_chdir(struct PF_STR* p_path);
s32 PFDIR_fchdir(struct PF_DIR* p_dir);
s32 PFDIR_chmod(struct PF_STR* p_path, u8 attr);
s32 PFDIR_chdmod(struct PF_STR* p_path, u8 attr);

#ifdef __cplusplus
}
#endif
#endif
