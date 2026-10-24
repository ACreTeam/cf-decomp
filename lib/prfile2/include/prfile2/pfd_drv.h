#ifndef PRFILE2_PFD_DRV_H
#define PRFILE2_PFD_DRV_H

// Media drivers of the standalone PrFILE2 (lib/prfile2/driver, lib/prfile2/msc).

#include <prfile2/pdm_api.h>
#include <prfile2/pf_service.h>

// little-endian field access. The aligned path indexes element (off / size + off % size), which is the field itself
// only when off is size-aligned, and FA_LD16 has no outer parentheses.
#define FA_LD16(buf, off) ((u32)&(buf)[off] & 1) ? ((buf)[off] | ((buf)[(off) + 1] << 8)) : (SWAP16(((u16*)(buf))[(off) / 2 + (off) % 2]))
#define FA_LD32(buf, off) SWAP32(*(u32*)&(buf)[off])
#define FA_ST16(buf, off, v)                                     \
    if ((u32)&(buf)[off] & 1) {                                  \
        (buf)[off] = (v);                                        \
        (buf)[(off) + 1] = (v) >> 8;                             \
    } else {                                                     \
        ((u16*)(buf))[(off) / 2 + (off) % 2] = SWAP16(v);        \
    }
#define FA_ST32(buf, off, v)                                     \
    if ((u32)&(buf)[off] & 3) {                                  \
        (buf)[off] = (v);                                        \
        (buf)[(off) + 1] = (v) >> 8;                             \
        (buf)[(off) + 2] = (v) >> 16;                            \
        (buf)[(off) + 3] = (v) >> 24;                            \
    } else {                                                     \
        ((u32*)(buf))[(off) / 4 + (off) % 4] = SWAP32(v);        \
    }

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*PFD_MEDIA_CALLBACK)(s8 drive);

s32 pfd_sddrv_init_drv_tbl(PDM_DISK_TBL* p_disk_tbl, u32 ui_ext);
BOOL pfd_sddrv_is_media_insert(void);
s32 pfd_sddrv_registar_callback(PFD_MEDIA_CALLBACK insert, PFD_MEDIA_CALLBACK eject);

s32 usbh_msc_blk_init_drv_tbl(PDM_DISK_TBL* p_disk_tbl, u32 ui_ext);
BOOL pfd_mscdrv_is_media_insert(PDM_DISK* p_disk);
s32 pfd_mscdrv_registar_callback(PFD_MEDIA_CALLBACK insert, PFD_MEDIA_CALLBACK eject);
BOOL uhf_msc_is_media_insert(PDM_DISK* p_disk);

s32 fa_nanddrv_init_drv_tbl(PDM_DISK_TBL* p_disk_tbl, u32 ui_ext);

u8 pfd_get_media_drv_char(PDM_DISK* p_disk, s8* p_drv_char, u8 num);

#ifdef __cplusplus
}
#endif
#endif
