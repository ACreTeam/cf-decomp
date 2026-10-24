#ifndef PRFILE2_PF_CLUSTER_H
#define PRFILE2_PF_CLUSTER_H

#include <prfile2/pf_types.h>

#ifdef __cplusplus
extern "C" {
#endif

void PFCLUSTER_UpdateLastAccessCluster(struct PF_FILE* p_file, u32 sector);
s32 PFCLUSTER_AppendCluster(struct PF_FILE* p_file, u32 byte, u32* p_success);
s32 PFCLUSTER_AdjustCluster(struct PF_FILE* p_file);
s32 PFCLUSTER_GetAppendSize(struct PF_FILE* p_file, u32* p_size);
s32 PFCLUSTER_CombineFiles(struct PF_ENT_ITER* p_iter1, struct PF_ENT_ITER* p_iter2, struct PF_DIR_ENT* p_ent1, struct PF_DIR_ENT* p_ent2);
s32 PFCLUSTER_DivideFile(struct PF_ENT_ITER* p_iter_src, struct PF_ENT_ITER* p_iter_dst, struct PF_DIR_ENT* p_ent_src, struct PF_DIR_ENT* p_ent_dst, u32 offset);
s32 PFCLUSTER_InsertCluster(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent, u32 start, u32 num, u32* p_num_inserted);
s32 PFCLUSTER_DeleteCluster(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent, u32 start, u32 num, u32* p_num_deleted);

#ifdef __cplusplus
}
#endif
#endif
