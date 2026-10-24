#include <prfile2/fa.h>
#include <prfile2/pf_api.h>
#include <prfile2/pfd_drv.h>
#include <string.h>

typedef struct FAInitDiskSet {
    PDM_INIT_DISK* p_tbl;  // 0x00
    u32 num;               // 0x04
} FAInitDiskSet;

static PDM_INIT_DISK s_init_disk_tbl[FA_DEVICE_CUSTOM] = {
    {pfd_sddrv_init_drv_tbl, 0},
    {usbh_msc_blk_init_drv_tbl, 0},
    {fa_nanddrv_init_drv_tbl, 0},
};

static FAInitDiskSet s_init_disk_set = {s_init_disk_tbl, FA_DEVICE_CUSTOM};

PDM_DISK* gOpenDisk[PDM_DRIVE_COUNT];
PDM_PARTITION* gOpenPartition[PDM_DRIVE_COUNT];

s32 FAAttach(u8 device, const s8* nand_path, u32 nand_size, struct PF_DRV_TBL* p_drv_tbl) {
    PDM_DISK* p_disk;
    struct PF_DRV_TBL* drv_tbl[2];
    struct PF_ATTACH_PARAM param;
    s32 err;
    s32 idx;

    if (device >= FA_DEVICE_NUM) {
        return FA_RET_INVALID_PARAM;
    }
    if (p_drv_tbl == NULL) {
        return FA_RET_INVALID_PARAM;
    }
    if (device == FA_DEVICE_NAND && nand_path == NULL) {
        return FA_RET_INVALID_PARAM;
    }
    if (device == FA_DEVICE_CUSTOM && p_drv_tbl->p_part == NULL) {
        return FA_RET_INVALID_PARAM;
    }
    if (device <= FA_DEVICE_NAND) {
        if (pdm_open_disk(&s_init_disk_set.p_tbl[device], &p_disk) != 0) {
            return FA_RET_ERR;
        }
        if (pdm_open_partition(p_disk, 0, &p_drv_tbl->p_part) != 0) {
            return FA_RET_ERR;
        }
    }
    drv_tbl[0] = p_drv_tbl;
    drv_tbl[1] = NULL;
    if (device == FA_DEVICE_NAND) {
        memcpy(param.path, nand_path, sizeof(param.path));
        param.nand_size = nand_size;
    }
    if (device == FA_DEVICE_NAND) {
        err = pfstub_attach(drv_tbl, &param);
    } else {
        err = pfstub_attach(drv_tbl, NULL);
    }
    if (err != 0) {
        return FA_RET_ERR;
    }
    idx = p_drv_tbl->drive - 'A';
    if (device <= FA_DEVICE_NAND) {
        gOpenDisk[idx] = p_disk;
        gOpenPartition[idx] = p_drv_tbl->p_part;
        if (device == FA_DEVICE_SD) {
            if (pfd_sddrv_is_media_insert()) {
                drv_tbl[0]->stat |= PF_DRV_STAT_INSERTED;
            }
        } else if (device == FA_DEVICE_USB) {
            if (pfd_mscdrv_is_media_insert(p_disk)) {
                drv_tbl[0]->stat |= PF_DRV_STAT_INSERTED;
            }
        }
    } else {
        gOpenDisk[idx] = NULL;
        gOpenPartition[idx] = NULL;
    }
    return 0;
}
