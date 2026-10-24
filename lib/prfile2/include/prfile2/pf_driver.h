#ifndef PRFILE2_PF_DRIVER_H
#define PRFILE2_PF_DRIVER_H

#include <prfile2/pdm_api.h>
#include <prfile2/pf_types.h>

#ifdef __cplusplus
extern "C" {
#endif

void PFDRV_LoadVolumeLabelFromBuf(PF_VOLUME* p_vol, const u8* buf);
void PFDRV_StoreVolumeLabelToBuf(u8* buf, PF_VOLUME* p_vol);
s32 PFDRV_StoreVolumeLabelToBPB(PF_VOLUME* p_vol, const u8* label);
void PFDRV_MediaEject(PDM_PARTITION* p_part);
void PFDRV_MediaInsert(PDM_PARTITION* p_part);
BOOL PFDRV_IsInserted(PF_VOLUME* p_vol);
BOOL PFDRV_IsMountRequested(PF_VOLUME* p_vol);
void PFDRV_ClearMountRequested(PF_VOLUME* p_vol);
BOOL PFDRV_IsUnmountRequested(PF_VOLUME* p_vol);
void PFDRV_ClearUnmountRequested(PF_VOLUME* p_vol);
s32 PFDRV_init(PF_VOLUME* p_vol);
s32 PFDRV_finalize(PF_VOLUME* p_vol);
s32 PFDRV_mount(PF_VOLUME* p_vol);
s32 PFDRV_unmount(PF_VOLUME* p_vol, u32 mode);
s32 PFDRV_format(PF_VOLUME* p_vol, const u8* param);
s32 PFDRV_lread(PF_VOLUME* p_vol, u8* buf, u32 sector, u32 num_sectors, u32* p_num_success);
s32 PFDRV_lwrite(PF_VOLUME* p_vol, const u8* buf, u32 sector, u32 num_sectors, u32* p_num_success);

#ifdef __cplusplus
}
#endif
#endif
