#ifndef PRFILE2_PF_FILE_H
#define PRFILE2_PF_FILE_H

#include <prfile2/pf_types.h>

#ifdef __cplusplus
extern "C" {
#endif

u32 PFFILE_Cursor_AdvanceToRead(struct PF_FILE* p_file, u32 n, u32 sector);
s32 PFFILE_Cursor_ReadHeadSector(struct PF_FILE* p_file, u8* p_buf, u32 size, u32* p_size_read);
s32 PFFILE_Cursor_ReadBodySectors(struct PF_FILE* p_file, u8* p_buf, u32 size, u32* p_size_read);
s32 PFFILE_Cursor_ReadTailSector(struct PF_FILE* p_file, u8* p_buf, u32 size, u32* p_size_read);
s32 PFFILE_Cursor_Read(struct PF_FILE* p_file, u8* p_buf, u32 size, u32* p_size_read);
s32 PFFILE_Cursor_WriteHeadSector(struct PF_FILE* p_file, u8* p_buf, u32 size, u32* p_size_write);
s32 PFFILE_Cursor_WriteBodySectors(struct PF_FILE* p_file, u8* p_buf, u32 size, u32* p_size_write);
s32 PFFILE_Cursor_WriteTailSector(struct PF_FILE* p_file, u8* p_buf, u32 size, u32* p_size_write);
s32 PFFILE_Cursor_Write(struct PF_FILE* p_file, u8* p_buf, u32 size, u32* p_size_write);
void PFFILE_Cursor_MoveToEnd(struct PF_FILE* p_file);
void PFFILE_Cursor_MoveToClusterEnd(struct PF_FILE* p_file, u32 size);
struct PF_SFD* PFFILE_GetSFD(struct PF_VOLUME* p_vol, struct PF_DIR_ENT* p_ent);
s32 PFFILE_createEmptyFile(struct PF_VOLUME* p_vol, struct PF_DIR_ENT* p_file_ent, struct PF_STR* p_filename);
s32 PFFILE_p_remove(struct PF_VOLUME* p_vol, struct PF_STR* p_path_str);
s32 PFFILE_p_fopen(struct PF_VOLUME* p_vol, struct PF_STR* p_path_str, s32 mode, struct PF_FILE** pp_file);
s32 PFFILE_p_fread(struct PF_VOLUME* p_vol, u8* p_buf, u32 size, u32 count, struct PF_FILE* p_file, u32* p_count_read);
s32 PFFILE_p_fwrite(struct PF_VOLUME* p_vol, u8* p_buf, u32 size, u32 count, struct PF_FILE* p_file, u32* p_count_written);
s32 PFFILE_p_fappend(struct PF_FILE* p_file, u32 size, u32* p_size_appended);
s32 PFFILE_p_finfo(struct PF_FILE* p_file, struct PF_INFO* p_info);
s32 PFFILE_p_combine(struct PF_VOLUME* p_vol, struct PF_STR* p_path_str1, struct PF_STR* p_path_str2);
s32 PFFILE_p_divide(struct PF_VOLUME* p_vol, struct PF_STR* p_src_str, struct PF_STR* p_dst_str, u32 offset);
s32 PFFILE_p_cinsert(struct PF_VOLUME* p_vol, struct PF_STR* p_path_str, u32 start, u32 num, u32* p_num_inserted);
s32 PFFILE_p_cdelete(struct PF_VOLUME* p_vol, struct PF_STR* p_path_str, u32 start, u32 num, u32* p_num_deleted);
s32 PFFILE_p_flock(struct PF_FILE* p_file, u32 mode);
s32 PFFILE_FsexecOpenFile(struct PF_DIR_ENT* p_ent, struct PF_ENT_ITER* p_iter, u32 mode, struct PF_FILE** pp_file);
BOOL PFFILE_IsOpened(struct PF_DIR_ENT* p_ent);
void PFFILE_FinalizeAllFiles(struct PF_VOLUME* p_vol);
s32 PFFILE_remove(struct PF_STR* p_path_str);
s32 PFFILE_ferror(struct PF_FILE* p_file);
s32 PFFILE_fopen(struct PF_STR* p_path_str, s32 mode, struct PF_FILE** pp_file);
s32 PFFILE_fclose(struct PF_FILE* p_file);
s32 PFFILE_fread(u8* p_buf, u32 size, u32 count, struct PF_FILE* p_file, u32* p_count_read);
s32 PFFILE_fwrite(u8* p_buf, u32 size, u32 count, struct PF_FILE* p_file, u32* p_count_written);
s32 PFFILE_feof(struct PF_FILE* p_file);
s32 PFFILE_fseek(struct PF_FILE* p_file, s32 lOffset, s32 nOrigin);
s32 PFFILE_fsetclstlink(struct PF_FILE* p_file, u32 mode, struct PF_CLUSTER_LINK_RES* p_link);
s32 PFFILE_fsync(struct PF_FILE* p_file);
s32 PFFILE_fappend(struct PF_FILE* p_file, u32 size, u32* p_size_appended);
s32 PFFILE_fadjust(struct PF_FILE* p_file);
s32 PFFILE_finfo(struct PF_FILE* p_file, struct PF_INFO* p_info);
s32 PFFILE_combine(struct PF_STR* p_path_str1, struct PF_STR* p_path_str2);
s32 PFFILE_divide(struct PF_STR* p_src_str, struct PF_STR* p_dst_str, u32 offset);
s32 PFFILE_cinsert(struct PF_STR* p_path_str, u32 start, u32 num, u32* p_num_inserted);
s32 PFFILE_cdelete(struct PF_STR* p_path_str, u32 start, u32 num, u32* p_num_deleted);
s32 PFFILE_flock(struct PF_FILE* p_file, u32 mode);

#ifdef __cplusplus
}
#endif
#endif
