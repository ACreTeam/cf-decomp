#ifndef PRFILE2_PF_ENTRY_ITERATOR_H
#define PRFILE2_PF_ENTRY_ITERATOR_H

#include <prfile2/pf_types.h>

#ifdef __cplusplus
extern "C" {
#endif

s32 PFENT_ITER_DoFindEntry(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent, struct PF_STR* p_pattern, u8 attr_required, u8 attr_unwanted, u32* p_is_found);
s32 PFENT_ITER_DoGetEntry(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent, struct PF_VOLUME* p_vol, struct PF_STR* p_path, u32 wildcard, u32 no_look_last_token);
s32 PFENT_ITER_IteratorInitialize(struct PF_ENT_ITER* p_iter, u32 index_start_from);
u32 PFENT_ITER_IsAtLogicalEnd(struct PF_ENT_ITER* p_iter);
s32 PFENT_ITER_MoveTo(struct PF_ENT_ITER* p_iter, u32 index, u32 may_allocate);
s32 PFENT_ITER_Advance(struct PF_ENT_ITER* p_iter, u32 may_allocate);
s32 PFENT_ITER_FindCluster(struct PF_DIR_ENT* p_ent, u32 cluster, u32* p_is_found);
s32 PFENT_ITER_Retreat(struct PF_ENT_ITER* p_iter, u32 may_allocate);
s32 PFENT_ITER_FindDirEntryFromCluster(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent, u32 cluster, u32* p_is_found);
s32 PFENT_ITER_FindEntry(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent, struct PF_STR* p_pattern, u8 attr_required, u8 attr_unwanted, u32* p_is_found);
s32 PFENT_ITER_FindEntryNoPathCheck(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent, struct PF_STR* p_pattern, u8 attr_required, u8 attr_unwanted, u32* p_is_found);
s32 PFENT_ITER_GetEntryOfPath(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent, struct PF_VOLUME* p_vol, struct PF_STR* p_path, u32 no_look_last_token);
s32 PFENT_ITER_GetEntryOfPattern(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent, struct PF_VOLUME* p_vol, struct PF_STR* p_path);
s32 PFENT_ITER_GetEntryOfIter(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent);
s32 PFENT_ITER_GetLFNEntryName(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent);

#ifdef __cplusplus
}
#endif
#endif
