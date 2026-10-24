#ifndef PRFILE2_PF_PATH_H
#define PRFILE2_PF_PATH_H

#include <prfile2/pf_types.h>

#ifdef __cplusplus
extern "C" {
#endif

s32 PFPATH_DoSplitPath(struct PF_STR* p_path, struct PF_STR* p_dir_path, struct PF_STR* p_filename, u32 wildcard);
u16 PFPATH_GetNextCharOfPattern(struct PF_STR* p_pattern, u32 is_long_name);
u32 PFPATH_DoMatchFileNameWithPattern(u16 c_name, struct PF_FILE_NAME_ITER* p_name, u16 c_pat, struct PF_STR* p_pattern, u32 is_long_name);
s32 PFPATH_cmpNameImpl(const s8* sName, const s8* sPattern, u32* p_is_end);
s32 PFPATH_cmpNameUni(const u16* p_name, struct PF_STR* sPattern);
s32 PFPATH_cmpName(const s8* sShort, struct PF_STR* p_pattern, u32 is_short_search);
s32 PFPATH_cmpTailSFN(const s8* sfn_name, const s8* pattern);
void PFPATH_InitTokenOfPath(struct PF_STR* p_str, s8* path, u32 code_mode);
s32 PFPATH_GetNextTokenOfPath(struct PF_STR* p_str, u32 wildcard);
s32 PFPATH_SplitPath(struct PF_STR* p_path, struct PF_STR* p_dir_path, struct PF_STR* p_filename);
s32 PFPATH_SplitPathPattern(struct PF_STR* p_path, struct PF_STR* p_dir_path, struct PF_STR* p_filename);
struct PF_VOLUME* PFPATH_GetVolumeFromPath(struct PF_STR* p_path);
u32 PFPATH_MatchFileNameWithPattern(const s8* file_name, struct PF_STR* p_pattern, u32 is_long_name);
s32 PFPATH_putShortName(u8* pDirEntry, const s8* short_name, u8 attr);
s32 PFPATH_getShortName(s8* short_name, const u8* pDirEntry, u8 attr);
void PFPATH_getLongNameformShortName(s8* short_name, s8* long_name, u8 flag);
u32 PFPATH_GetLengthFromShortname(const s8* sSrc);
u32 PFPATH_GetLengthFromUnicode(const u16* sSrc);
s32 PFPATH_transformFromUnicodeToNormal(s8* sDest, const u16* sSrc);
s32 PFPATH_transformInUnicode(u16* sDestStr, const s8* sSrcStr);
u32 PFPATH_parseShortName(s8* pDest, struct PF_STR* p_pattern);
s32 PFPATH_parseShortNameNumeric(s8* p_char, u32 count);
u32 PFPATH_CheckExtShortNameSignature(struct PF_STR* p_str);
u32 PFPATH_CheckExtShortName(struct PF_STR* p_str, u32 target, u32 wildcard);
u32 PFPATH_GetExtShortNameIndex(struct PF_STR* p_str, u32* p_index);
s32 PFPATH_AdjustExtShortName(s8* pName, u32 position);
void PFPATH_SetSearchPattern(s8* p_buf_local, u16* p_buf_unicode, struct PF_STR* p_pattern);

#ifdef __cplusplus
}
#endif
#endif
