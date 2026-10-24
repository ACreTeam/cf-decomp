#ifndef PRFILE2_PDM_PARTITION_H
#define PRFILE2_PDM_PARTITION_H

#include <prfile2/pdm_api.h>

#ifdef __cplusplus
extern "C" {
#endif

s32 pdm_part_is_master_boot_sector(volatile u8* buf, u32 total_sectors, u32* is_master_boot);
u32 pdm_part_is_boot_sector(u8* buf);
s32 pdm_part_get_start_sector(PDM_PARTITION* p_part);
u32 pdm_part_chg_ltop(PDM_PARTITION* p_part, u32 lsector, u16 lbps);
s32 pdm_part_get_partition(PDM_DISK* p_disk, s32 part_id, PDM_PARTITION** pp_part);
s32 pdm_part_release_partition(PDM_PARTITION* p_part);
BOOL pdm_part_is_attached_partition(PDM_PARTITION* p_part);
void pdm_part_set_driver_error_code(PDM_PARTITION* p_part, s32 error_code);
s32 pdm_part_get_driver_error_code(PDM_PARTITION* p_part);
s32 pdm_part_get_media_attribute(PDM_PARTITION* p_part, PDM_DISK_INFO* p_disk_info);
s32 pdm_part_init(PDM_PARTITION* p_part);
s32 pdm_part_mount(PDM_PARTITION* p_part);
s32 pdm_part_format(PDM_PARTITION* p_part, const u8* param);
s32 pdm_part_logical_read(PDM_PARTITION* p_part, u8* buf, u32 lsector, u32 num_sector, u16 bps, u32* p_num_success);
s32 pdm_part_logical_write(PDM_PARTITION* p_part, const u8* buf, u32 lsector, u32 num_sector, u16 bps, u32* p_num_success);
s32 pdm_part_unmount(PDM_PARTITION* p_part, u32 mode);
s32 pdm_part_finalize(PDM_PARTITION* p_part);
s32 pdm_part_register_callback(PDM_PARTITION* p_part, PDM_PART_FUNC* p_func);
s32 pdm_part_unregister_callback(PDM_PARTITION* p_part);
void pdm_part_set_change_media_state(PDM_DISK* p_disk, u32 event);

#ifdef __cplusplus
}
#endif
#endif
