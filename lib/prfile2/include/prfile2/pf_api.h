#ifndef PRFILE2_PF_API_H
#define PRFILE2_PF_API_H

// Internal interface of the standalone PrFILE2 ("fa") library used by its API wrapper layer (lib/prfile2/standard).

#include <prfile2/pf_types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct PF_DTA;
struct PF_STAT;

s32 pf2_init_prfile2(u32 config, void* param);

// common/pf_str.c
enum {
    PF_STR_CODEMODE_NONE = 0,
    PF_STR_CODEMODE_OEM = 1,
    PF_STR_CODEMODE_UNICODE = 2,
    PF_STR_CODEMODE_LOCAL = 3,
};

// target of PFSTR_GetStrPos / PFSTR_StrNumChar / PFSTR_StrNCmp
enum {
    PF_STR_TARGET_HEAD = 1,
    PF_STR_TARGET_TAIL = 2,
    PF_STR_TARGET_LOCAL = 3,
};

void PFSTR_SetCodeMode(struct PF_STR* p_str, u32 code_mode);
u32 PFSTR_GetCodeMode(struct PF_STR* p_str);
void PFSTR_SetLocalStr(struct PF_STR* p_str, s8* p_local);
s8* PFSTR_GetStrPos(struct PF_STR* p_str, u32 target);
void PFSTR_MoveStrPos(struct PF_STR* p_str, s16 num_char);
s32 PFSTR_InitStr(struct PF_STR* p_str, const s8* s, u32 code_mode);
u16 PFSTR_StrLen(struct PF_STR* p_str);
u16 PFSTR_StrNumChar(struct PF_STR* p_str, u32 target);
s32 PFSTR_StrCmp(const struct PF_STR* p_str, const s8* s);
s32 PFSTR_StrNCmp(struct PF_STR* p_str, const s8* s, u32 target, s16 offset, u16 num);
void PFSTR_ToUpperNStr(struct PF_STR* p_str, u16 num, s8* dest);

// standard/pf_api_util.c
s32 PFAPI_convertReturnValue(s32 err);
s32 PFAPI_convertError(s32 err);
s32 PFAPI_convertDriverError(s32 err);
s32 PFAPI_convertReturnValue4feof(s32 err);

// fatfs
struct PF_ATTACH_PARAM {
    s8 path[0x4E];  // 0x00
    u32 nand_size;  // 0x50
};
s32 PFVOL_attach(struct PF_DRV_TBL* p_drv, struct PF_ATTACH_PARAM* p_param, u32 nand_size);
s32 PFAPI_ParseOpenModeString(const s8* mode_str);
s32 PFWAPI_ParseOpenModeString(const u16* mode_str);
s32 PFVOL_setupfsi(s8 drv_char, u16 mode);
s32 PFVOL_iswriteprotected(s8 drv_char, u32* p_result);
s32 PFFILE_fappend(struct PF_FILE* p_file, u32 size, u32* p_result);
s32 PFFILE_cdelete(struct PF_STR* p_path, u32 start, u32 size, u32* p_result);
s32 PFFILE_cinsert(struct PF_STR* p_path, u32 start, u32 size, u32* p_result);
s32 PFFILE_divide(struct PF_STR* p1, struct PF_STR* p2, u32 size);
s32 PFFILE_fopen(struct PF_STR* p_path, s32 mode, struct PF_FILE** pp_file);
s32 PFDIR_createdir(struct PF_STR* p_path, u32 option, struct PF_DTA* p_dta);
s32 PFDIR_fsfirst(struct PF_STR* p_path, u8 attr, struct PF_DTA* p_dta);
s32 PFDIR_opendir(struct PF_STR* p_path, struct PF_DIR** pp_dir);
void* PFAPI_convertReturnValue2NULL(s32 err, void* p_stream);
s32 PFVOL_buffering(s8 drv_char, u8 mode);
s32 PFVOL_derrnum(s8 drv_char);
s32 PFVOL_getvol(s8 drv_char, struct PF_VOLTAB* p_voltab);
s32 PFVOL_getvolcfg(s8 drv_char, struct PF_VOL_CFG* p_cfg);
s32 PFVOL_mount(s8 drv_char);
s32 PFVOL_regctx(void);
s32 PFVOL_rmvvol(s8 drv_char);
s32 PFVOL_setclstlink(s8 drv_char, u32 mode, struct PF_CLUSTER_LINK_RES* p_link);
s32 PFVOL_setcode(struct PF_CHARCODE* p_codeset);
s32 PFVOL_setencode(u32 mode);
s32 PFVOL_settailbuf(s8 drv_char, u32 size, u32* buf);
s32 PFVOL_setvolcfg(s8 drv_char, struct PF_VOL_CFG* p_cfg);
s32 PFVOL_sync(s8 drv_char, u32 mode);
s32 PFVOL_unregctx(void);
s32 PFVOL_setvol(s8 drv_char, struct PF_STR* p_str);

s32 PFFILE_fadjust(struct PF_FILE* p_file);
s32 PFFILE_feof(struct PF_FILE* p_file);
s32 PFFILE_ferror(struct PF_FILE* p_file);
s32 PFFILE_flock(struct PF_FILE* p_file, u32 mode);
s32 PFFILE_fsetclstlink(struct PF_FILE* p_file, u32 mode, struct PF_CLUSTER_LINK_RES* p_link);
s32 PFFILE_fsync(struct PF_FILE* p_file);
s32 PFFILE_remove(struct PF_STR* p_path);
s32 PFFILE_combine(struct PF_STR* p1, struct PF_STR* p2);

s32 PFDIR_chdir(struct PF_STR* p_path);
s32 PFDIR_chdmod(struct PF_STR* p_path, u8 attr);
s32 PFDIR_chmod(struct PF_STR* p_path, u8 attr);
s32 PFDIR_closedir(struct PF_DIR* p_dir);
s32 PFDIR_fchdir(struct PF_DIR* p_dir);
s32 PFDIR_fsexec(struct PF_DTA* p_dta, u32 type, u8 param);
s32 PFDIR_fsnext(struct PF_DTA* p_dta);
s32 PFDIR_fstat(struct PF_STR* p_path, struct PF_STAT* p_stat);
s32 PFDIR_mkdir(struct PF_STR* p_path);
s32 PFDIR_move(struct PF_STR* p_src, struct PF_STR* p_dst);
s32 PFDIR_readdir(struct PF_DIR* p_dir, struct PF_DIRENT* p_dirent);
s32 PFDIR_rename(struct PF_STR* p_src, struct PF_STR* p_dst);
s32 PFDIR_rmdir(struct PF_STR* p_path);
s32 PFDIR_seekdir(struct PF_DIR* p_dir, u32 pos);
s32 PFDIR_telldir(struct PF_DIR* p_dir, u32* p_offset);

#ifdef __cplusplus
}
#endif
#endif
