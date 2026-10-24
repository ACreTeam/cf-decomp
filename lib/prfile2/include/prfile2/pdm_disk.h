#ifndef PRFILE2_PDM_DISK_H
#define PRFILE2_PDM_DISK_H

#include <prfile2/pdm_api.h>

#ifdef __cplusplus
extern "C" {
#endif

s32 pdm_disk_convert_sector_into_block(PDM_DISK* p_disk, u32 sector, u32 num_sector, u16 bps, u32* p_block, u32* p_num_block);
s32 pdm_disk_convert_block_into_sector(PDM_DISK* p_disk, u32 block, u32 num_block, u16 bps, u32* p_sector, u32* p_num_sector);
s32 pdm_disk_init_disk_manager(u32 config, void* param);
s32 pdm_disk_add_disk(PDM_INIT_DISK* p_init_disk_tbl, PDM_DISK** pp_disk);
s32 pdm_disk_del_disk(PDM_DISK* p_disk);
s32 pdm_disk_set_disk(PDM_DISK* p_disk, PDM_PARTITION* p_part);
s32 pdm_disk_get_media_attribute(PDM_DISK* p_disk, PDM_DISK_INFO* p_disk_info);
s32 pdm_disk_get_media_bps(PDM_DISK* p_disk, u16* p_lba_size);
s32 pdm_disk_init(PDM_DISK* p_disk);
s32 pdm_disk_mount(PDM_DISK* p_disk);
s32 pdm_disk_format(PDM_DISK* p_disk, const u8* param);
s32 pdm_disk_physical_read(PDM_DISK* p_disk, u8* buf, u32 psector, u32 num_sector, u16 bps, u32* p_num_success);
s32 pdm_disk_physical_write(PDM_DISK* p_disk, const u8* buf, u32 psector, u32 num_sector, u16 bps, u32* p_num_success);
s32 pdm_disk_unmount(PDM_DISK* p_disk, u32 mode);
s32 pdm_disk_finalize(PDM_DISK* p_disk);
void pdm_disk_notify_media_eject(PDM_DISK* p_disk);
void pdm_disk_notify_media_insert(PDM_DISK* p_disk);

#ifdef __cplusplus
}
#endif
#endif
