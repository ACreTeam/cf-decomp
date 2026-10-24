#include <prfile2/pdm_bpb.h>
#include <prfile2/pdm_disk.h>
#include <prfile2/pdm_mbr.h>
#include <prfile2/pf_clib.h>
#include <prfile2/pfd_drv.h>
#include <prfile2/pfd_sddrv.h>
#include <revolution/os.h>

static const PDM_FUNCTBL pfd_sddrv_functbl = {
    pfd_sddrv_init,
    pfd_sddrv_finalize,
    pfd_sddrv_mount,
    pfd_sddrv_unmount,
    pfd_sddrv_format,
    pfd_sddrv_pread,
    pfd_sddrv_pwrite,
    pfd_sddrv_get_disk_info,
};

static const PFD_SDDRV_FORMAT_PARAM pfd_sddrv_format_tbl[PFD_SDDRV_FORMAT_PARAM_COUNT] = {
    {0x00000000, 0x00001000, 16, 2, 16, 16},
    {0x00001000, 0x00004000, 16, 2, 32, 16},
    {0x00004000, 0x00008000, 32, 2, 32, 32},
    {0x00008000, 0x00010000, 32, 4, 32, 32},
    {0x00010000, 0x00020000, 32, 8, 32, 32},
    {0x00020000, 0x00040000, 64, 8, 32, 32},
    {0x00040000, 0x00080000, 64, 16, 32, 32},
    {0x00080000, 0x000FC000, 128, 16, 63, 32},
    {0x000FC000, 0x001F8000, 128, 32, 63, 32},
    {0x001F8000, 0x00200000, 128, 64, 63, 32},
    {0x00200000, 0x003F0000, 128, 64, 63, 64},
    {0x003F0000, 0x00400000, 128, 128, 63, 64},
};

volatile PFD_SDDRV_INFO g_pfd_sddrv_info;
SDDev g_pfd_sddev;
u8 g_pfd_sddrv_buf[PDM_SECTOR_SIZE] ATTRIBUTE_ALIGN(32);

u32 g_pfd_sddrv_intr_event ATTRIBUTE_ALIGN(32);
PFD_MEDIA_CALLBACK g_pfd_sddrv_eject_callback;
PFD_MEDIA_CALLBACK g_pfd_sddrv_insert_callback;

static s32 pfd_sddrv_get_value_with_total_sectors(u32 total_sectors, PFD_SDDRV_FORMAT_VALUE* p_value) {
    u32 i;

    for (i = 0; i < PFD_SDDRV_FORMAT_PARAM_COUNT; i++) {
        if (pfd_sddrv_format_tbl[i].min_sectors < total_sectors && pfd_sddrv_format_tbl[i].max_sectors >= total_sectors) {
            p_value->boundary_unit = pfd_sddrv_format_tbl[i].boundary_unit;
            p_value->heads = pfd_sddrv_format_tbl[i].heads;
            p_value->sectors_per_track = pfd_sddrv_format_tbl[i].sectors_per_track;
            p_value->sectors_per_cluster = pfd_sddrv_format_tbl[i].sectors_per_cluster;
            break;
        }
    }
    if (i == PFD_SDDRV_FORMAT_PARAM_COUNT) {
        return PFD_SDDRV_ERR_INVALID_PARAM;
    }
    return 0;
}

#define PFD_SDDRV_COPY(dst, src, length)  \
    if ((dst) != NULL && (src) != NULL) { \
        for (i = 0; i < (length); i++) {  \
            (dst)[i] = (src)[i];          \
        }                                 \
    }

s32 pfd_st_inter_callback(s32 event, void* arg) {
    s8 drv_char;

    if ((event & PFD_SDDRV_STAT_INSERTED) != PFD_SDDRV_STAT_INSERTED) {
        return 0;
    }

    if (g_pfd_sddrv_info.p_dev != NULL) {
        g_pfd_sddrv_intr_event = PFD_SDDRV_STAT_REMOVED;
        if (ISD_RegisterDeviceIntrHandler(g_pfd_sddrv_info.p_dev, pfd_st_removal_callback, &g_pfd_sddrv_intr_event) != SD_ERROR_SUCCESS) {
            OSReport("ERR:Failed to regist intr handler. pfd_st_inter_callback()\n");
        }
    }

    if (g_pfd_sddrv_info.p_disk != NULL) {
        if (g_pfd_sddrv_info.drv_char == 0) {
            if (pfd_get_media_drv_char(g_pfd_sddrv_info.p_disk, &drv_char, 1) == 1) {
                g_pfd_sddrv_info.drv_char = drv_char;
            }
        }
        pdm_disk_notify_media_insert(g_pfd_sddrv_info.p_disk);
        if (g_pfd_sddrv_info.drv_char != 0 && g_pfd_sddrv_insert_callback != NULL) {
            g_pfd_sddrv_insert_callback(g_pfd_sddrv_info.drv_char);
        }
    }

    g_pfd_sddrv_info.is_inserted = TRUE;
    g_pfd_sddrv_info.is_changed = TRUE;
    return 0;
}

s32 pfd_st_removal_callback(s32 event, void* arg) {
    s8 drv_char;

    if ((event & PFD_SDDRV_STAT_REMOVED) != PFD_SDDRV_STAT_REMOVED) {
        return 0;
    }

    g_pfd_sddrv_info.is_inserted = FALSE;
    if (g_pfd_sddrv_info.p_dev != NULL) {
        g_pfd_sddrv_intr_event = PFD_SDDRV_STAT_INSERTED;
        if (ISD_RegisterDeviceIntrHandler(g_pfd_sddrv_info.p_dev, pfd_st_inter_callback, &g_pfd_sddrv_intr_event) != SD_ERROR_SUCCESS) {
            OSReport("ERR:Failed to regist intr handler. pfd_st_removal_callback()\n");
        }
    }

    if (g_pfd_sddrv_info.p_disk != NULL) {
        if (g_pfd_sddrv_info.drv_char == 0) {
            if (pfd_get_media_drv_char(g_pfd_sddrv_info.p_disk, &drv_char, 1) == 1) {
                g_pfd_sddrv_info.drv_char = drv_char;
            }
        }
        pdm_disk_notify_media_eject(g_pfd_sddrv_info.p_disk);
        if (g_pfd_sddrv_info.drv_char != 0 && g_pfd_sddrv_eject_callback != NULL) {
            g_pfd_sddrv_eject_callback(g_pfd_sddrv_info.drv_char);
        }
    }
    return 0;
}

s32 pfd_sddrv_init(PDM_DISK* p_disk) {
    s32 err;
    SDDev* p_dev;
    u32 status;

    if (p_disk == (PDM_DISK*)NULL) {
        return PFD_SDDRV_ERR_INVALID_PARAM;
    }

    if (g_pfd_sddrv_info.flags & PFD_SDDRV_INITIALIZED) {
        OSReport("INFO SD Card driver is already initialize. pfd_sddrv_init()\n");
        if (p_disk != g_pfd_sddrv_info.p_disk) {
            return PFD_SDDRV_ERR_OTHER_DISK;
        }
        return 0;
    }

    if ((g_pfd_sddrv_info.flags & PFD_SDDRV_CARD_INITIALIZED) == 0) {
        g_pfd_sddrv_info.bytes_per_sector = PDM_SECTOR_SIZE;
        pf_memset(&g_pfd_sddev, 0, sizeof(SDDev));
        if (ISD_InitCard() != SD_ERROR_SUCCESS) {
            OSReport("ERR:Failed to init SD Card Driver in pfd_sddrv_init()\n");
            return PFD_SDDRV_ERR_INIT_CARD;
        }
        g_pfd_sddrv_info.flags |= PFD_SDDRV_CARD_INITIALIZED;
    }

    pf_memset(&g_pfd_sddev, 0, sizeof(SDDev));
    p_dev = &g_pfd_sddev;
    err = ISD_MountCard(PFD_SDDRV_SLOT, &p_dev);
    if (err != SD_ERROR_SUCCESS) {
        OSReport("ERR SD card can not mount [ret = 0x%x]. pfd_sddrv_init()\n", err);
        return PFD_SDDRV_ERR_MOUNT_CARD;
    }

    err = ISD_GetDeviceStatus(p_dev, &status);
    if (err != SD_ERROR_SUCCESS) {
        OSReport("ERR Failed to get sd card status. [ret = 0x%x]\n", err);
        return PDM_ERR_DRIVER_ERROR;
    }
    if (status & PFD_SDDRV_STAT_INSERTED) {
        g_pfd_sddrv_info.is_inserted = TRUE;
    }

    g_pfd_sddrv_info.p_dev = p_dev;
    if (g_pfd_sddrv_info.is_inserted) {
        g_pfd_sddrv_intr_event = PFD_SDDRV_STAT_REMOVED;
        err = ISD_RegisterDeviceIntrHandler(p_dev, pfd_st_removal_callback, &g_pfd_sddrv_intr_event);
        if (err != SD_ERROR_SUCCESS) {
            OSReport("ERR:Failed to regist intr handler1 [ret = 0x%x] pfd_sddrv_init()\n", err);
            ISD_UnmountCard(p_dev);
            g_pfd_sddrv_info.p_dev = NULL;
            return PFD_SDDRV_ERR_INTR_HANDLER;
        }
    } else {
        g_pfd_sddrv_intr_event = PFD_SDDRV_STAT_INSERTED;
        err = ISD_RegisterDeviceIntrHandler(p_dev, pfd_st_inter_callback, &g_pfd_sddrv_intr_event);
        if (err != SD_ERROR_SUCCESS) {
            OSReport("ERR:Failed to regist intr handler1 [ret = 0x%x] pfd_sddrv_init()\n", err);
            ISD_UnmountCard(p_dev);
            g_pfd_sddrv_info.p_dev = NULL;
            return PFD_SDDRV_ERR_INTR_HANDLER;
        }
    }

    g_pfd_sddrv_info.p_disk = p_disk;
    g_pfd_sddrv_info.flags |= PFD_SDDRV_INITIALIZED;
    return 0;
}

s32 pfd_sddrv_mount(PDM_DISK* p_disk) {
    s32 err;
    u32 i;
    u32 ocr;
    u16 rca;
    u8 cid[PFD_SDDRV_CID_LEN];

    if (p_disk == NULL) {
        return PFD_SDDRV_ERR_INVALID_PARAM;
    }
    if (!g_pfd_sddrv_info.is_inserted) {
        return PFD_SDDRV_ERR_NOT_INSERTED;
    }
    if (g_pfd_sddrv_info.flags & PFD_SDDRV_MOUNTED) {
        OSReport("INFO sdcard alrady mounted. pfd_sddrv_mount()\n");
        return 0;
    }

    for (i = 0; i < PFD_SDDRV_RESET_RETRY_COUNT; i++) {
        if (!g_pfd_sddrv_info.is_inserted) {
            return PFD_SDDRV_ERR_NOT_INSERTED;
        }
        err = ISD_ResetDevice(g_pfd_sddrv_info.p_dev);
        if (err == SD_ERROR_SUCCESS) {
            g_pfd_sddrv_info.is_changed = FALSE;
            break;
        }
        OSReport("ERROR Failed to SD Card Reset [ret = 0x%x]. pfd_sddrv_mount()\n", err);
    }
    if (i == PFD_SDDRV_RESET_RETRY_COUNT) {
        return PFD_SDDRV_ERR_RESET;
    }
    if (g_pfd_sddrv_info.is_changed) {
        return PFD_SDDRV_ERR_NOT_INSERTED;
    }

    ocr = 0;
    if (ISD_ReadCardRegister(g_pfd_sddrv_info.p_dev, PFD_SDDRV_REG_OCR, &ocr, sizeof(ocr)) != SD_ERROR_SUCCESS) {
        OSReport("ERR Failed to read OCR reg. pfd_sddrv_mount()\n");
        return PFD_SDDRV_ERR_READ_REGISTER;
    }
    pf_memset(cid, 0, sizeof(cid));
    if (ISD_ReadCardRegister(g_pfd_sddrv_info.p_dev, PFD_SDDRV_REG_CID, (u32*)cid, sizeof(cid)) != SD_ERROR_SUCCESS) {
        OSReport("ERR Failed to read CID reg. pfd_sddrv_mount()\n");
        return PFD_SDDRV_ERR_READ_REGISTER;
    }
    rca = 0;
    if (ISD_ReadCardRegister(g_pfd_sddrv_info.p_dev, PFD_SDDRV_REG_RCA, (u32*)&rca, sizeof(rca)) != SD_ERROR_SUCCESS) {
        OSReport("ERR Failed to read RCA reg. pfd_sddrv_mount()\n");
        return PFD_SDDRV_ERR_READ_REGISTER;
    }

    g_pfd_sddrv_info.flags |= PFD_SDDRV_MOUNTED;
    return 0;
}

s32 pfd_sddrv_format(PDM_DISK* p_disk, const u8* param) {
    s32 err;

    if (p_disk == NULL) {
        return PFD_SDDRV_ERR_INVALID_PARAM;
    }
    if (!g_pfd_sddrv_info.is_inserted) {
        return PFD_SDDRV_ERR_NOT_INSERTED;
    }
    if (param != NULL && pf_strcmp((const s8*)param, (const s8*)"FULL_FORMAT") == 0) {
        err = pfd_sddrv_full_format();
        if (err != 0) {
            OSReport("ERR Failed to full format. pfd_sddrv_full_format()\n");
            return err;
        }
    }
    return 0;
}

s32 pfd_sddrv_pread(PDM_DISK* p_disk, u8* buf, u32 block, u32 num_block, u32* p_num_success) {
    if (p_disk == NULL || buf == NULL || p_num_success == NULL) {
        return PFD_SDDRV_ERR_INVALID_PARAM;
    }
    return pfd_sddrv_physical_read(num_block, buf, block, g_pfd_sddrv_info.bytes_per_sector, p_num_success);
}

s32 pfd_sddrv_pwrite(PDM_DISK* p_disk, const u8* buf, u32 block, u32 num_block, u32* p_num_success) {
    if (p_disk == NULL || buf == NULL || p_num_success == NULL) {
        return PFD_SDDRV_ERR_INVALID_PARAM;
    }
    return pfd_sddrv_physical_write(num_block, buf, block, g_pfd_sddrv_info.bytes_per_sector, p_num_success);
}

s32 pfd_sddrv_unmount(PDM_DISK* p_disk) {
    if (p_disk == NULL) {
        return PFD_SDDRV_ERR_INVALID_PARAM;
    }
    if (g_pfd_sddrv_info.flags & PFD_SDDRV_MOUNTED) {
        g_pfd_sddrv_info.flags &= ~PFD_SDDRV_MOUNTED;
    }
    return 0;
}

s32 pfd_sddrv_finalize(PDM_DISK* p_disk) {
    s32 err;

    if (p_disk == NULL) {
        return PFD_SDDRV_ERR_INVALID_PARAM;
    }
    if (g_pfd_sddrv_info.flags & PFD_SDDRV_MOUNTED) {
        g_pfd_sddrv_info.flags &= ~PFD_SDDRV_MOUNTED;
    }
    if (g_pfd_sddrv_info.flags & PFD_SDDRV_INITIALIZED) {
        err = ISD_UnregisterDeviceIntrHandler(g_pfd_sddrv_info.p_dev);
        if (err != SD_ERROR_SUCCESS) {
            OSReport("WARNING Faild to UnregisterDeviceIntrHandler sd card [ret = %d]\n", err);
        }
        err = ISD_UnmountCard(g_pfd_sddrv_info.p_dev);
        if (err != SD_ERROR_SUCCESS) {
            OSReport("WARNING Faild to unmount sd card [ret = %d]\n", err);
        }
        g_pfd_sddrv_info.p_dev = NULL;
    }
    g_pfd_sddrv_info.flags &= ~PFD_SDDRV_INITIALIZED;
    g_pfd_sddrv_info.is_inserted = FALSE;
    g_pfd_sddrv_info.p_disk = NULL;
    g_pfd_sddrv_info.drv_char = 0;
    return 0;
}

s32 pfd_sddrv_get_disk_info(PDM_DISK* p_disk, PDM_DISK_INFO* p_disk_info) {
    s32 err;
    u32 status;

    if (p_disk == NULL || p_disk_info == NULL) {
        return PFD_SDDRV_ERR_INVALID_PARAM;
    }
    if ((g_pfd_sddrv_info.flags & PFD_SDDRV_MOUNTED) == 0) {
        return PFD_SDDRV_ERR_NOT_MOUNTED;
    }
    if (g_pfd_sddrv_info.is_changed) {
        return PFD_SDDRV_ERR_NOT_MOUNTED;
    }
    if (!g_pfd_sddrv_info.is_inserted) {
        return PFD_SDDRV_ERR_NOT_INSERTED;
    }

    err = ISD_GetDeviceStatus(g_pfd_sddrv_info.p_dev, &status);
    if (err != SD_ERROR_SUCCESS) {
        OSReport("ERR Failed to get sd card status. [ret = 0x%x]\n", err);
        return PFD_SDDRV_ERR_GET_STATUS;
    }
    if ((status & PFD_SDDRV_STAT_INSERTED) == 0) {
        OSReport("INFO Card not inserted! in pfd_sddrv_get_disk_info()\n");
        return PFD_SDDRV_ERR_NOT_INSERTED;
    }
    if (status & PFD_SDDRV_STAT_REMOVED) {
        OSReport("INFO Card removed! in pfd_sddrv_get_disk_info()\n");
        return PFD_SDDRV_ERR_NOT_INSERTED;
    }
    if ((status & PFD_SDDRV_STAT_MEMORY_CARD) == 0) {
        OSReport("INFO SD card type is not memory. in pfd_sddrv_get_disk_info()\n");
        return PFD_SDDRV_ERR_NOT_MEMORY_CARD;
    }

    p_disk_info->media_attr = PDM_MEDIA_ATTR_REMOVABLE;
    if (status & PFD_SDDRV_STAT_WRITE_PROTECTED) {
        p_disk_info->media_attr |= PDM_MEDIA_ATTR_WRITE_PROTECTED;
    }
    err = pfd_sddrv_get_total_sectors(&p_disk_info->total_sectors, &p_disk_info->bytes_per_sector);
    if (err != 0) {
        OSReport("ERR Failed to read master boot sector. pfd_sddrv_get_disk_info()\n");
        return err;
    }
    p_disk_info->cylinders = 0;
    p_disk_info->heads = 0;
    p_disk_info->sectors_per_track = 0;
    p_disk_info->format_param = NULL;
    return 0;
}

s32 pfd_sddrv_init_drv_tbl(PDM_DISK_TBL* p_disk_tbl, u32 ui_ext) {
    p_disk_tbl->p_func = (PDM_FUNCTBL*)&pfd_sddrv_functbl;
    p_disk_tbl->ui_ext = ui_ext;
    return 0;
}

s32 pfd_sddrv_registar_callback(PFD_MEDIA_CALLBACK insert, PFD_MEDIA_CALLBACK eject) {
    g_pfd_sddrv_insert_callback = insert;
    g_pfd_sddrv_eject_callback = eject;
    return 0;
}

BOOL pfd_sddrv_is_media_insert(void) {
    s8 drv_char;

    if (g_pfd_sddrv_info.flags & PFD_SDDRV_INITIALIZED) {
        if (g_pfd_sddrv_info.drv_char == 0) {
            if (pfd_get_media_drv_char(g_pfd_sddrv_info.p_disk, &drv_char, 1) == 1) {
                g_pfd_sddrv_info.drv_char = drv_char;
            }
        }
        if (g_pfd_sddrv_info.is_inserted) {
            pdm_disk_notify_media_insert(g_pfd_sddrv_info.p_disk);
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

s32 pfd_sddrv_physical_read(u32 num_sectors, u8* buf, u32 sector, u32 bps, u32* p_num_success) {
    s32 err;
    u32 size;
    u32 offset;
    u32 i;

    if ((g_pfd_sddrv_info.flags & PFD_SDDRV_MOUNTED) == 0) {
        *p_num_success = 0;
        return PFD_SDDRV_ERR_NOT_MOUNTED;
    }
    if (g_pfd_sddrv_info.is_changed) {
        return PFD_SDDRV_ERR_NOT_MOUNTED;
    }
    size = num_sectors * bps;
    if (size == 0) {
        *p_num_success = 0;
        return PFD_SDDRV_ERR_INVALID_PARAM;
    }
    offset = sector * bps;
    *p_num_success = PFD_SDDRV_NUM_SUCCESS_UNKNOWN;

    if (((u32)buf & (PFD_SDDRV_BUF_ALIGN - 1)) == 0) {
        if (!g_pfd_sddrv_info.is_inserted) {
            *p_num_success = 0;
            return PFD_SDDRV_ERR_NOT_INSERTED;
        }
        if (num_sectors == 1) {
            err = ISD_ReadBlock(g_pfd_sddrv_info.p_dev, offset, buf, size);
            if (err != SD_ERROR_SUCCESS) {
                OSReport("INFO Failed to read SD card1 [ret = 0x%x] pfd_sddrv_physical_read()\n", err);
                *p_num_success = 0;
                return PFD_SDDRV_ERR_READ;
            }
        } else {
            err = ISD_ReadMultiBlock(g_pfd_sddrv_info.p_dev, offset, buf, size);
            if (err != SD_ERROR_SUCCESS) {
                OSReport("INFO Failed to read SD card2 [ret = 0x%x] pfd_sddrv_physical_read()\n", err);
                *p_num_success = 0;
                return PFD_SDDRV_ERR_READ;
            }
        }
    } else {
        for (i = 0; i < num_sectors; i++) {
            if (!g_pfd_sddrv_info.is_inserted) {
                if (i != 0) {
                    *p_num_success = i;
                    return 0;
                }
                *p_num_success = 0;
                return PFD_SDDRV_ERR_NOT_INSERTED;
            }
            err = ISD_ReadBlock(g_pfd_sddrv_info.p_dev, offset, g_pfd_sddrv_buf, bps);
            if (err != SD_ERROR_SUCCESS) {
                OSReport("INFO Failed to read SD card3 [ret = 0x%x] pfd_sddrv_physical_read()\n", err);
                if (i != 0) {
                    *p_num_success = i;
                    return 0;
                }
                *p_num_success = 0;
                return PFD_SDDRV_ERR_READ;
            }
            pf_memcpy(buf, g_pfd_sddrv_buf, bps);
            buf += bps;
            offset += bps;
        }
    }
    *p_num_success = num_sectors;
    return 0;
}

s32 pfd_sddrv_physical_write(u32 num_sectors, const u8* buf, u32 sector, u32 bps, u32* p_num_success) {
    s32 err;
    u32 size;
    u32 offset;
    u32 i;

    if ((g_pfd_sddrv_info.flags & PFD_SDDRV_MOUNTED) == 0) {
        *p_num_success = 0;
        return PFD_SDDRV_ERR_NOT_MOUNTED;
    }
    if (g_pfd_sddrv_info.is_changed) {
        return PFD_SDDRV_ERR_NOT_MOUNTED;
    }
    size = num_sectors * bps;
    if (size == 0) {
        *p_num_success = 0;
        return PFD_SDDRV_ERR_INVALID_PARAM;
    }
    offset = sector * bps;
    *p_num_success = PFD_SDDRV_NUM_SUCCESS_UNKNOWN;

    if (((u32)buf & (PFD_SDDRV_BUF_ALIGN - 1)) == 0) {
        if (!g_pfd_sddrv_info.is_inserted) {
            *p_num_success = 0;
            return PFD_SDDRV_ERR_NOT_INSERTED;
        }
        if (num_sectors == 1) {
            err = ISD_WriteBlock(g_pfd_sddrv_info.p_dev, offset, (u8*)buf, size);
            if (err != SD_ERROR_SUCCESS) {
                OSReport("INFO Failed to write SD card1 [ret = 0x%x] pfd_sddrv_physical_write()\n", err);
                *p_num_success = 0;
                return PFD_SDDRV_ERR_WRITE;
            }
        } else {
            err = ISD_WriteMultiBlock(g_pfd_sddrv_info.p_dev, offset, (u8*)buf, size);
            if (err != SD_ERROR_SUCCESS) {
                OSReport("INFO Failed to write SD card2 [ret = 0x%x] pfd_sddrv_physical_write()\n", err);
                *p_num_success = 0;
                return PFD_SDDRV_ERR_WRITE;
            }
        }
    } else {
        for (i = 0; i < num_sectors; i++) {
            if (!g_pfd_sddrv_info.is_inserted) {
                if (i != 0) {
                    *p_num_success = i;
                    return 0;
                }
                *p_num_success = 0;
                return PFD_SDDRV_ERR_NOT_INSERTED;
            }
            pf_memcpy(g_pfd_sddrv_buf, buf, bps);
            err = ISD_WriteBlock(g_pfd_sddrv_info.p_dev, offset, g_pfd_sddrv_buf, bps);
            if (err != SD_ERROR_SUCCESS) {
                OSReport("INFO Failed to write SD card3 [ret = 0x%x] pfd_sddrv_physical_write()\n", err);
                if (i != 0) {
                    *p_num_success = i;
                    return 0;
                }
                *p_num_success = 0;
                return PFD_SDDRV_ERR_WRITE;
            }
            buf += bps;
            offset += bps;
        }
    }
    *p_num_success = num_sectors;
    return 0;
}

typedef struct PFD_SDDRV_CSD {
    u32 _00;                 // 0x00
    u32 c_size_lo : 10;      // 0x04
    u32 : 12;
    u32 c_size_mult : 3;
    u32 : 7;
    u32 : 20;                // 0x08
    u32 read_bl_len : 4;
    u32 : 6;
    u32 c_size_hi : 2;
    u32 _0C;                 // 0x0C
} PFD_SDDRV_CSD;

s32 pfd_sddrv_get_total_sectors(u32* p_total_sectors, u16* p_bytes_per_sector) {
    u32 c_size;
    u16 mult;
    u16 read_bl_len;
    u16 block_len;
    u32 block_nr;
    PFD_SDDRV_CSD csd;

    if (!g_pfd_sddrv_info.is_inserted) {
        return PFD_SDDRV_ERR_NOT_INSERTED;
    }

    pf_memset(&csd, 0, sizeof(csd));
    if (ISD_ReadCardRegister(g_pfd_sddrv_info.p_dev, PFD_SDDRV_REG_CSD, (u32*)&csd, sizeof(csd)) != SD_ERROR_SUCCESS) {
        OSReport("ERR Failed to get CSD Info. pfd_sddrv_get_total_sectors()\n");
        return PFD_SDDRV_ERR_READ_REGISTER;
    }

    mult = 1 << (csd.c_size_mult + PFD_SDDRV_CSD_C_SIZE_MULT_BIAS);
    c_size = (csd.c_size_hi << PFD_SDDRV_CSD_C_SIZE_LO_BITS) | csd.c_size_lo;
    block_nr = (c_size + 1) * mult;
    read_bl_len = csd.read_bl_len;
    read_bl_len = (read_bl_len < PFD_SDDRV_CSD_READ_BL_LEN_MIN) ? PFD_SDDRV_CSD_READ_BL_LEN_MIN : read_bl_len;
    read_bl_len = (read_bl_len > PFD_SDDRV_CSD_READ_BL_LEN_MAX) ? PFD_SDDRV_CSD_READ_BL_LEN_MAX : read_bl_len;
    block_len = read_bl_len - PDM_SECTOR_SHIFT;
    block_len = 1 << block_len;
    *p_total_sectors = block_nr * block_len;
    *p_bytes_per_sector = PDM_SECTOR_SIZE;
    return 0;
}

static inline u32 pfd_sddrv_cluster_sectors(const PFD_SDDRV_FORMAT_VALUE* value) {
    return value->sectors_per_cluster;
}

s32 pfd_sddrv_calc_mbr_bpb(PFD_SDDRV_MBR_BPB* p_param) {
    s32 err;
    BOOL adjusted;
    u32 area;
    u32 num_clusters;
    u32 boundary_unit;
    u32 total_sectors;
    u32 sectors_per_FAT;
    u32 new_sectors_per_FAT;
    u32 fat_bits;
    u32 num_sectors;
    u32 system_sectors;
    u32 offset;
    u32 n;
    u32 sectors_per_cluster;
    u32 start_sector;
    PFD_SDDRV_FORMAT_VALUE value;
    OSCalendarTime time;

    adjusted = FALSE;
    if (p_param == NULL) {
        return PFD_SDDRV_ERR_INVALID_PARAM;
    }

    pf_memset(&value, 0, sizeof(value));
    err = pfd_sddrv_get_value_with_total_sectors(p_param->total_sectors, &value);
    if (err != 0) {
        OSReport("ERR Failed to get values with total sectors. pfd_sddrv_get_value_with_total_sectors()\n");
        return err;
    }

    sectors_per_cluster = pfd_sddrv_cluster_sectors(&value);
    total_sectors = p_param->total_sectors;
    p_param->sectors_per_cluster = sectors_per_cluster;
    num_clusters = total_sectors / sectors_per_cluster;
    if (num_clusters < PDM_FAT12_MAX_CLUSTERS + PFD_SDDRV_FAT12_MARGIN) {
        fat_bits = 12;
    } else if (num_clusters < PDM_FAT16_MAX_CLUSTERS) {
        fat_bits = 16;
    } else {
        return PFD_SDDRV_ERR_UNSUPPORTED_FAT;
    }
    if (total_sectors % sectors_per_cluster) {
        num_clusters++;
    }
    sectors_per_FAT = num_clusters * fat_bits / PFD_SDDRV_SECTOR_BITS;
    if ((num_clusters * fat_bits) & (PFD_SDDRV_SECTOR_BITS - 1)) {
        sectors_per_FAT++;
    }

    boundary_unit = value.boundary_unit;
    while (1) {
        system_sectors = sectors_per_FAT * PFD_SDDRV_NUM_FATS + PFD_SDDRV_FIXED_SYSTEM_SECTORS;
        offset = boundary_unit;
        n = 1;
        while (1) {
            start_sector = offset - system_sectors;
            if ((s32)start_sector > 0) {
                break;
            }
            offset += boundary_unit;
            n++;
        }
        area = n * boundary_unit;

        while (1) {
            if (!adjusted && boundary_unit != start_sector) {
                start_sector += boundary_unit;
            }
            adjusted = FALSE;
            num_clusters = (total_sectors - start_sector - system_sectors) / sectors_per_cluster + 1;
            if (num_clusters >= PDM_FAT12_MAX_CLUSTERS - PFD_SDDRV_FAT12_MARGIN && num_clusters < PDM_FAT12_MAX_CLUSTERS + PFD_SDDRV_FAT12_MARGIN) {
                area += boundary_unit;
                start_sector = area - system_sectors;
                continue;
            }
            new_sectors_per_FAT = ((num_clusters - 1) * fat_bits + 2) / PFD_SDDRV_SECTOR_BITS;
            if (((num_clusters - 1) * fat_bits + 2) & (PFD_SDDRV_SECTOR_BITS - 1)) {
                new_sectors_per_FAT++;
            }
            if (new_sectors_per_FAT > sectors_per_FAT) {
                start_sector += boundary_unit;
                adjusted = TRUE;
                continue;
            }
            break;
        }
        if (new_sectors_per_FAT == sectors_per_FAT) {
            break;
        }
        sectors_per_FAT = new_sectors_per_FAT;
    }

    num_sectors = p_param->total_sectors - start_sector;
    p_param->sectors_per_FAT = sectors_per_FAT;
    p_param->start_sector = start_sector;
    p_param->num_sectors = num_sectors;
    num_clusters = (num_sectors - (p_param->sectors_per_FAT * PFD_SDDRV_NUM_FATS + PFD_SDDRV_FIXED_SYSTEM_SECTORS)) / p_param->sectors_per_cluster + 1;
    if (num_clusters < PDM_FAT12_MAX_CLUSTERS) {
        p_param->fat_type = PDM_FAT_12;
    } else if (num_clusters >= PDM_FAT12_MAX_CLUSTERS) {
        p_param->fat_type = PDM_FAT_16;
    } else {
        return PFD_SDDRV_ERR_UNSUPPORTED_FAT;
    }

    OSTicksToCalendarTime(OSGetTime(), &time);
    p_param->vol_id = (((time.mday & PF_FAT_DATE_DAY_MASK) | (time.mon & PF_FAT_DATE_MONTH_MASK) << PF_FAT_DATE_MONTH_SHIFT |
                        ((time.year - PFD_SDDRV_VOL_ID_BASE_YEAR) & PF_FAT_DATE_YEAR_MASK) << PF_FAT_DATE_YEAR_SHIFT) << 16) +
                      ((time.sec & PF_FAT_TIME_SEC2_MASK) | (time.min & PF_FAT_TIME_MIN_MASK) << PF_FAT_TIME_MIN_SHIFT |
                       (time.hour & PF_FAT_TIME_HOUR_MASK) << PF_FAT_TIME_HOUR_SHIFT);
    return 0;
}

s32 pfd_sddrv_store_bpb_buf(PFD_SDDRV_MBR_BPB* p_param, u8* buf) {
    s32 err;
    u32 i;
    u16 total_sectors16;
    u32 total_sectors32;
    PFD_SDDRV_FORMAT_VALUE value;

    if (p_param == NULL) {
        return PFD_SDDRV_ERR_INVALID_PARAM;
    }

    pf_memset(&value, 0, sizeof(value));
    err = pfd_sddrv_get_value_with_total_sectors(p_param->total_sectors, &value);
    if (err != 0) {
        OSReport("ERR Failed to get values with total sectors. pfd_sddrv_get_value_with_total_sectors()\n");
        return err;
    }

    total_sectors32 = p_param->num_sectors;
    if (total_sectors32 != 0 && total_sectors32 < PDM_BPB_TOTAL_SECTORS16_LIMIT) {
        total_sectors16 = total_sectors32;
        total_sectors32 = 0;
    } else if (total_sectors32 >= PDM_BPB_TOTAL_SECTORS16_LIMIT) {
        total_sectors16 = 0;
    } else {
        return PFD_SDDRV_ERR_INVALID_PARAM;
    }

    pf_memset(buf, 0, PDM_SECTOR_SIZE);
    PFD_SDDRV_COPY(buf + PDM_BPB_OEM_NAME, "        ", PDM_BPB_OEM_NAME_LEN);
    PFD_SDDRV_COPY(buf + PDM_BPB_VOL_LABEL, PDM_BPB_VOL_LABEL_NO_NAME, PDM_BPB_VOL_LABEL_LEN);
    if (p_param->fat_type == PDM_FAT_12) {
        PFD_SDDRV_COPY(buf + PDM_BPB_FS_TYPE, "FAT12   ", PDM_BPB_FS_TYPE_LEN);
    } else {
        PFD_SDDRV_COPY(buf + PDM_BPB_FS_TYPE, "FAT16   ", PDM_BPB_FS_TYPE_LEN);
    }

    buf[PDM_BPB_JUMP_BOOT] = PDM_BPB_JUMP_SHORT;
    buf[PDM_BPB_JUMP_BOOT + 1] = 0x00;
    buf[PDM_BPB_JUMP_BOOT + 2] = PDM_BPB_NOP;
    FA_ST16(buf, PDM_BPB_BYTES_PER_SECTOR, PDM_SECTOR_SIZE);
    buf[PDM_BPB_SECTORS_PER_CLUSTER] = p_param->sectors_per_cluster;
    FA_ST16(buf, PDM_BPB_NUM_RESERVED_SECTORS, PFD_SDDRV_NUM_RESERVED_SECTORS);
    buf[PDM_BPB_NUM_FATS] = PFD_SDDRV_NUM_FATS;
    FA_ST16(buf, PDM_BPB_NUM_ROOT_DIR_ENTRIES, PFD_SDDRV_ROOT_DIR_ENTRIES);
    FA_ST16(buf, PDM_BPB_TOTAL_SECTORS16, total_sectors16);
    buf[PDM_BPB_MEDIA] = PDM_BPB_MEDIA_FIXED;
    FA_ST16(buf, PDM_BPB_SECTORS_PER_FAT16, p_param->sectors_per_FAT);
    FA_ST16(buf, PDM_BPB_SECTORS_PER_TRACK, value.sectors_per_track);
    FA_ST16(buf, PDM_BPB_NUM_HEADS, value.heads);
    FA_ST32(buf, PDM_BPB_NUM_HIDDEN_SECTORS, p_param->start_sector);
    FA_ST32(buf, PDM_BPB_TOTAL_SECTORS32, total_sectors32);
    buf[PDM_BPB_DRIVE] = PDM_BPB_DRIVE_HARD_DISK;
    buf[PDM_BPB_BOOT_SIG] = PDM_BPB_EXT_BOOT_SIG;
    FA_ST32(buf, PDM_BPB_VOL_ID, p_param->vol_id);
    buf[PDM_BPB_SIGNATURE1] = PDM_BPB_SIGNATURE1_VALUE;
    buf[PDM_BPB_SIGNATURE2] = PDM_BPB_SIGNATURE2_VALUE;
    return 0;
}

s32 pfd_sddrv_store_mbr_buf(PFD_SDDRV_MBR_BPB* p_param, u8* buf) {
    s32 err;
    u8 type;
    u8 start_head;
    u16 start_sector;
    u16 start_cylinder;
    u8 end_head;
    u16 end_sector;
    u16 end_cylinder;
    u32 cylinder_sectors;
    u32 last_sector;
    u32 num_sectors;
    PFD_SDDRV_FORMAT_VALUE value;

    if (p_param == NULL) {
        return PFD_SDDRV_ERR_INVALID_PARAM;
    }

    pf_memset(&value, 0, sizeof(value));
    err = pfd_sddrv_get_value_with_total_sectors(p_param->total_sectors, &value);
    if (err != 0) {
        OSReport("ERR Failed to get values with total sectors. pfd_sddrv_get_value_with_total_sectors()\n");
        return err;
    }

    num_sectors = p_param->num_sectors;
    cylinder_sectors = value.heads * value.sectors_per_track;
    last_sector = p_param->total_sectors - 1;
    start_head = (p_param->start_sector % cylinder_sectors) / value.sectors_per_track;
    start_cylinder = p_param->start_sector / cylinder_sectors;
    start_sector = p_param->start_sector % value.sectors_per_track + 1;
    end_cylinder = last_sector / cylinder_sectors;
    end_head = (last_sector % cylinder_sectors) / value.sectors_per_track;
    end_sector = last_sector % value.sectors_per_track + 1;

    if (num_sectors == 0) {
        return PFD_SDDRV_ERR_INVALID_PARAM;
    }
    if (num_sectors != 0 && num_sectors < PFD_SDDRV_FAT12_MAX_SECTORS) {
        type = PDM_PART_TYPE_FAT12;
    } else if (num_sectors >= PFD_SDDRV_FAT12_MAX_SECTORS && num_sectors < PDM_BPB_TOTAL_SECTORS16_LIMIT) {
        type = PDM_PART_TYPE_FAT16_SMALL;
    } else {
        type = PDM_PART_TYPE_FAT16;
    }

    pf_memset(buf, 0, PDM_SECTOR_SIZE);
    buf[PDM_MBR_PARTITION_TBL + PDM_PART_TBL_BOOT_FLAG] = 0;
    buf[PDM_MBR_PARTITION_TBL + PDM_PART_TBL_S_HEAD] = start_head;
    FA_ST16(buf, PDM_MBR_PARTITION_TBL + PDM_PART_TBL_S_CYL_SECTOR,
            ((start_cylinder & 0xFF) << 8) + ((start_sector & 0x3F) | ((start_cylinder & 0x300) >> 2)));
    buf[PDM_MBR_PARTITION_TBL + PDM_PART_TBL_PARTITION_TYPE] = type;
    FA_ST16(buf, PDM_MBR_PARTITION_TBL + PDM_PART_TBL_E_HEAD, end_head);
    FA_ST16(buf, PDM_MBR_PARTITION_TBL + PDM_PART_TBL_E_CYL_SECTOR,
            ((end_cylinder & 0xFF) << 8) + ((end_sector & 0x3F) | ((end_cylinder & 0x300) >> 2)));
    FA_ST32(buf, PDM_MBR_PARTITION_TBL + PDM_PART_TBL_LBA_START_SECTOR, p_param->start_sector);
    FA_ST32(buf, PDM_MBR_PARTITION_TBL + PDM_PART_TBL_LBA_NUM_SECTORS, p_param->num_sectors);
    buf[PDM_BPB_SIGNATURE1] = PDM_BPB_SIGNATURE1_VALUE;
    buf[PDM_BPB_SIGNATURE2] = PDM_BPB_SIGNATURE2_VALUE;
    return 0;
}

s32 pfd_sddrv_build_mbr_bpb(u32 total_sectors) {
    s32 err;
    u32 offset;
    PFD_SDDRV_MBR_BPB param;

    pf_memset(&param, 0, sizeof(param));
    param.total_sectors = total_sectors;
    err = pfd_sddrv_calc_mbr_bpb(&param);
    if (err != 0) {
        OSReport("ERR Failed to calculate MBR and BPB values. pfd_sddrv_calc_mbr_bpb()\n");
        return err;
    }

    err = pfd_sddrv_store_bpb_buf(&param, g_pfd_sddrv_buf);
    if (err != 0) {
        OSReport("ERR Failed to store BPB values to buf. pfd_sddrv_store_bpb_buf()\n");
        return err;
    }
    offset = param.start_sector << PDM_SECTOR_SHIFT;
    if (!g_pfd_sddrv_info.is_inserted) {
        return PFD_SDDRV_ERR_NOT_INSERTED;
    }
    if (g_pfd_sddrv_info.is_changed) {
        return PFD_SDDRV_ERR_NOT_INSERTED;
    }
    if (ISD_WriteBlock(g_pfd_sddrv_info.p_dev, offset, g_pfd_sddrv_buf, PDM_SECTOR_SIZE) != SD_ERROR_SUCCESS) {
        OSReport("ERR Failed to write BPB fields. ISD_WriteBlock()\n");
        return PFD_SDDRV_ERR_WRITE;
    }

    err = pfd_sddrv_store_mbr_buf(&param, g_pfd_sddrv_buf);
    if (err != 0) {
        OSReport("ERR Failed to store MBR values to buf. pfd_sddrv_store_mbr_buf()\n");
        return err;
    }
    if (!g_pfd_sddrv_info.is_inserted) {
        return PFD_SDDRV_ERR_NOT_INSERTED;
    }
    if (g_pfd_sddrv_info.is_changed) {
        return PFD_SDDRV_ERR_NOT_INSERTED;
    }
    if (ISD_WriteBlock(g_pfd_sddrv_info.p_dev, PDM_MBR_SECTOR << PDM_SECTOR_SHIFT, g_pfd_sddrv_buf, PDM_SECTOR_SIZE) != SD_ERROR_SUCCESS) {
        OSReport("ERR Failed to write MBR fields. ISD_WriteBlock()\n");
        return PFD_SDDRV_ERR_WRITE;
    }
    return 0;
}

s32 pfd_sddrv_full_format(void) {
    s32 err;
    u32 i;
    u32 status;
    PDM_DISK_INFO disk_info;

    for (i = 0; i < PFD_SDDRV_RESET_RETRY_COUNT; i++) {
        if (!g_pfd_sddrv_info.is_inserted) {
            return PFD_SDDRV_ERR_NOT_INSERTED;
        }
        err = ISD_ResetDevice(g_pfd_sddrv_info.p_dev);
        if (err == SD_ERROR_SUCCESS) {
            g_pfd_sddrv_info.is_changed = FALSE;
            break;
        }
        OSReport("ERROR Failed to SD Card Reset [ret = 0x%x]. pfd_sddrv_full_format()\n", err);
    }
    if (i == PFD_SDDRV_RESET_RETRY_COUNT) {
        return PFD_SDDRV_ERR_RESET;
    }
    if (g_pfd_sddrv_info.is_changed) {
        return PFD_SDDRV_ERR_NOT_INSERTED;
    }

    err = ISD_GetDeviceStatus(g_pfd_sddrv_info.p_dev, &status);
    if (err != SD_ERROR_SUCCESS) {
        OSReport("ERR Failed to get sd card status. [ret = 0x%x]\n", err);
        return PFD_SDDRV_ERR_GET_STATUS;
    }
    if (status & PFD_SDDRV_STAT_WRITE_PROTECTED) {
        return PFD_SDDRV_ERR_WRITE_PROTECTED;
    }
    if (g_pfd_sddrv_info.is_changed) {
        return PFD_SDDRV_ERR_NOT_INSERTED;
    }

    err = pfd_sddrv_get_total_sectors(&disk_info.total_sectors, &disk_info.bytes_per_sector);
    if (err != 0) {
        OSReport("ERR Failed to get total sectors. pfd_sddrv_get_total_sectors()\n");
        return err;
    }
    if (disk_info.total_sectors > PFD_SDDRV_MAX_TOTAL_SECTORS) {
        return PFD_SDDRV_ERR_TOO_LARGE;
    }

    err = pfd_sddrv_build_mbr_bpb(disk_info.total_sectors);
    if (err != 0) {
        OSReport("ERR Failed to build up and write MBR and BPB fields. pfd_sddrv_build_mbr_bpb()\n");
        return err;
    }
    return 0;
}
