#ifndef PRFILE2_PF_ENTRY_H
#define PRFILE2_PF_ENTRY_H

#include <prfile2/pf_types.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    PFENT_NO_MATCH = -1,          // the entry does not match the pattern or the attributes
    PFENT_ROOT_PATH_LEN = 3,      // PF_DIR_ENT.path_len of a root directory ("A:\\")
    PFENT_NUM_DOT_ENTRIES = 2,    // "." and ".." at the start of a subdirectory
};

// PFENT_updateEntry flag
enum {
    PFENT_UPDATE_KEEP_ATTR = 0,
    PFENT_UPDATE_SET_ARCHIVE = 1,  // also set PF_ATTR_ARCHIVE
};

u8 PFENT_CalcCheckSum(struct PF_DIR_ENT* p_ent);
void PFENT_LoadShortNameFromBuf(struct PF_DIR_ENT* p_ent, const u8* buf);
void PFENT_loadEntryNumericFieldsFromBuf(struct PF_DIR_ENT* p_ent, const u8* buf);
s32 PFENT_LoadLFNEntryFieldsFromBuf(struct PF_DIR_ENT* p_ent, const u8* buf);
void PFENT_storeLFNEntryFieldsToBuf(u8* buf, struct PF_DIR_ENT* p_ent, u8 ord, u8 sum, u32 is_last);
s32 PFENT_findEntryPos(struct PF_FFD* p_ffd, struct PF_DIR_ENT* p_ent, u32 index_search_from, struct PF_STR* p_pattern, u8 attr_required, u8 attr_unwanted, u32* p_lpos, u32* p_ppos);
s32 PFENT_findEntry(struct PF_FFD* p_ffd, struct PF_DIR_ENT* p_ent, u32 index_search_from, struct PF_STR* p_pattern, u8 attr_required, u8 attr_unwanted);
s32 PFENT_allocateEntryPos(struct PF_DIR_ENT* p_ent, u8 num_entries, struct PF_FFD* p_ffd, u32* p_next_chain, struct PF_STR* p_filename, u32* p_pos);
s32 PFENT_allocateEntry(struct PF_DIR_ENT* p_ent, u8 num_entries, struct PF_FFD* p_ffd, u32* p_next_chain, struct PF_STR* p_filename);
s32 PFENT_GetRootDir(struct PF_VOLUME* p_vol, struct PF_DIR_ENT* p_ent);
s32 PFENT_MakeRootDir(struct PF_VOLUME* p_vol);
s32 PFENT_updateEntry(struct PF_DIR_ENT* p_ent, u32 flag);
s32 PFENT_AdjustSFN(struct PF_DIR_ENT* p_ent, s8* p_short_name);
s32 PFENT_RemoveEntry(struct PF_DIR_ENT* p_ent, struct PF_ENT_ITER* p_iter);
void PFENT_StoreEntryNumericFieldsToBuf(u8* buf, const struct PF_DIR_ENT* p_ent);
u8 PFENT_getcurrentDateTimeForEnt(u16* p_date, u16* p_time);

#ifdef __cplusplus
}
#endif
#endif
