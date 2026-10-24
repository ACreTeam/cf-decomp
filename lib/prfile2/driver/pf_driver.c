#include <prfile2/pdm_bpb.h>
#include <prfile2/pdm_partition.h>
#include <prfile2/pf_cache.h>
#include <prfile2/pf_clib.h>
#include <prfile2/pf_driver.h>

void PFDRV_LoadVolumeLabelFromBuf(PF_VOLUME* p_vol, const u8* buf) {
    pf_memcpy(p_vol->label, buf, PDM_BPB_VOL_LABEL_LEN);
    p_vol->label[PDM_BPB_VOL_LABEL_LEN] = 0;
}

void PFDRV_StoreVolumeLabelToBuf(u8* buf, PF_VOLUME* p_vol) {
    pf_memcpy(buf, p_vol->label, PDM_BPB_VOL_LABEL_LEN);
    buf[PDM_BPB_VOL_LABEL_LEN] = 0;
}

s32 PFDRV_StoreVolumeLabelToBPB(PF_VOLUME* p_vol, const u8* label) {
    s32 err;
    PF_CACHE_PAGE* p_page;
    u32 is_hit;
    u32 num_success;

    err = PFCACHE_AllocateDataPage(p_vol, PF_INVALID_SECTOR, &p_page, &is_hit);
    if (err != 0) {
        return err;
    }

    err = pdm_part_logical_read(p_vol->p_part, p_page->p_buf, PDM_BOOT_SECTOR, 1, p_vol->bpb.bytes_per_sector, &num_success);
    if (err != 0) {
        if (err == PDM_ERR_DRIVER_ERROR) {
            err = pdm_part_get_driver_error_code(p_vol->p_part);
            pf_vol_set.last_driver_error = err;
            p_vol->last_driver_error = err;
            err = PF_IERR_DRIVER;
        } else {
            err = PF_IERR_SYSTEM;
        }
    } else {
        err = 0;
    }
    if (err != 0) {
        PFCACHE_FreeDataPage(p_vol, p_page);
        return err;
    }
    if (num_success != 1) {
        PFCACHE_FreeDataPage(p_vol, p_page);
        return PF_IERR_IO;
    }

    if (p_vol->bpb.fat_type == FAT_32) {
        pf_memcpy(p_page->p_buf + PDM_BPB_VOL_LABEL32, label, PDM_BPB_VOL_LABEL_LEN);
    } else {
        pf_memcpy(p_page->p_buf + PDM_BPB_VOL_LABEL, label, PDM_BPB_VOL_LABEL_LEN);
    }

    err = pdm_part_logical_write(p_vol->p_part, p_page->p_buf, PDM_BOOT_SECTOR, 1, p_vol->bpb.bytes_per_sector, &num_success);
    if (err != 0) {
        if (err == PDM_ERR_DRIVER_ERROR) {
            err = pdm_part_get_driver_error_code(p_vol->p_part);
            pf_vol_set.last_driver_error = err;
            p_vol->last_driver_error = err;
            err = PF_IERR_DRIVER;
        } else {
            err = PF_IERR_SYSTEM;
        }
    } else {
        err = 0;
    }
    PFCACHE_FreeDataPage(p_vol, p_page);
    if (err != 0) {
        return err;
    }
    if (num_success != 1) {
        return PF_IERR_IO;
    }

    PFDRV_LoadVolumeLabelFromBuf(p_vol, label);
    return 0;
}

static void PFDRV_notifyExecuteFormatAfterMount(PF_VOLUME* p_vol, const u8* format_param) {
    p_vol->flags |= PF_VOL_FLAG_FORMAT_AFTER_MOUNT;
    p_vol->format_param = format_param;
}

static void PFDRV_notifyNoExecuteLogicalFormat(PF_VOLUME* p_vol) {
    p_vol->flags |= PF_VOL_FLAG_NO_LOGICAL_FORMAT;
}

void PFDRV_MediaEject(PDM_PARTITION* p_part) {
    PF_VOLUME* p_vol;
    u32 i;

    for (i = 0, p_vol = pf_vol_set.volumes; i < PF_DRIVE_COUNT; p_vol++, i++) {
        if (p_vol->p_part == p_part) {
            if (p_vol->flags & PF_VOL_FLAG_MOUNTED) {
                p_vol->flags |= PF_VOL_FLAG_UNMOUNT_REQUESTED;
            }
            p_vol->flags &= ~(PF_VOL_FLAG_INSERTED | PF_VOL_FLAG_MOUNTED);
        }
    }
}

void PFDRV_MediaInsert(PDM_PARTITION* p_part) {
    PF_VOLUME* p_vol;
    u32 i;

    for (i = 0, p_vol = pf_vol_set.volumes; i < PF_DRIVE_COUNT; p_vol++, i++) {
        if (p_vol->p_part == p_part) {
            p_vol->flags |= (PF_VOL_FLAG_INSERTED | PF_VOL_FLAG_MOUNT_REQUESTED);
        }
    }
}

BOOL PFDRV_IsInserted(PF_VOLUME* p_vol) {
    return (p_vol->flags & PF_VOL_FLAG_INSERTED) != 0;
}

BOOL PFDRV_IsMountRequested(PF_VOLUME* p_vol) {
    return (p_vol->flags & PF_VOL_FLAG_MOUNT_REQUESTED) != 0;
}

void PFDRV_ClearMountRequested(PF_VOLUME* p_vol) {
    p_vol->flags &= ~PF_VOL_FLAG_MOUNT_REQUESTED;
}

BOOL PFDRV_IsUnmountRequested(PF_VOLUME* p_vol) {
    return (p_vol->flags & PF_VOL_FLAG_UNMOUNT_REQUESTED) != 0;
}

void PFDRV_ClearUnmountRequested(PF_VOLUME* p_vol) {
    p_vol->flags &= ~PF_VOL_FLAG_UNMOUNT_REQUESTED;
}

s32 PFDRV_init(PF_VOLUME* p_vol) {
    s32 err;
    PDM_PART_FUNC func;

    func.insert = PFDRV_MediaInsert;
    func.eject = PFDRV_MediaEject;
    err = pdm_part_register_callback(p_vol->p_part, &func);
    if (err != 0) {
        return PF_IERR_SYSTEM;
    }

    err = pdm_part_init(p_vol->p_part);
    if (err != 0) {
        pdm_part_unregister_callback(p_vol->p_part);
        if (err == PDM_ERR_DRIVER_ERROR) {
            err = pdm_part_get_driver_error_code(p_vol->p_part);
            pf_vol_set.last_driver_error = err;
            p_vol->last_driver_error = err;
            return PF_IERR_DRIVER;
        }
        return PF_IERR_SYSTEM;
    }
    return 0;
}

s32 PFDRV_finalize(PF_VOLUME* p_vol) {
    s32 err;

    err = pdm_part_unregister_callback(p_vol->p_part);
    if (err != 0) {
        return PF_IERR_SYSTEM;
    }

    err = pdm_part_finalize(p_vol->p_part);
    if (err != 0) {
        if (err == PDM_ERR_DRIVER_ERROR) {
            err = pdm_part_get_driver_error_code(p_vol->p_part);
            pf_vol_set.last_driver_error = err;
            p_vol->last_driver_error = err;
            return PF_IERR_DRIVER;
        }
        return PF_IERR_SYSTEM;
    }
    return 0;
}

s32 PFDRV_mount(PF_VOLUME* p_vol) {
    s32 err;
    u32 is_valid;
    PF_CACHE_PAGE* p_page;
    u32 nSector;
    PDM_DISK_INFO disk_inf;

    err = pdm_part_mount(p_vol->p_part);
    if (err != 0) {
        if (err == PDM_ERR_DRIVER_ERROR) {
            err = pdm_part_get_driver_error_code(p_vol->p_part);
            pf_vol_set.last_driver_error = err;
            p_vol->last_driver_error = err;
            return PF_IERR_DRIVER;
        }
        return PF_IERR_SYSTEM;
    }

    if (pdm_part_get_media_attribute(p_vol->p_part, &disk_inf) != 0) {
        pdm_part_unmount(p_vol->p_part, PDM_UNMOUNT_NORMAL);
        return PF_IERR_SYSTEM;
    }

    if (disk_inf.media_attr & PDM_MEDIA_ATTR_WRITE_PROTECTED) {
        p_vol->flags |= PF_VOL_FLAG_WRITE_PROTECTED;
    } else {
        p_vol->flags &= ~PF_VOL_FLAG_WRITE_PROTECTED;
    }
    if (disk_inf.media_attr & PDM_MEDIA_ATTR_FORMAT_AFTER_MOUNT) {
        PFDRV_notifyExecuteFormatAfterMount(p_vol, disk_inf.format_param);
    }
    if (disk_inf.media_attr & PDM_MEDIA_ATTR_NO_LOGICAL_FORMAT) {
        PFDRV_notifyNoExecuteLogicalFormat(p_vol);
    }

    PFCACHE_AllocateDataPage(p_vol, PF_INVALID_SECTOR, &p_page, &is_valid);
    err = pdm_part_logical_read(p_vol->p_part, p_page->buffer, PDM_BOOT_SECTOR, 1, PF_CACHE_PAGE_SIZE, &nSector);
    if (err != 0) {
        if (err == PDM_ERR_DRIVER_ERROR) {
            err = pdm_part_get_driver_error_code(p_vol->p_part);
            pf_vol_set.last_driver_error = err;
            p_vol->last_driver_error = err;
            err = PF_IERR_DRIVER;
        } else {
            err = PF_IERR_SYSTEM;
        }
        PFCACHE_FreeDataPage(p_vol, p_page);
        pdm_part_unmount(p_vol->p_part, PDM_UNMOUNT_NORMAL);
        return err;
    }

    if (pdm_bpb_InitVolumeWithBootSector(&p_vol->bpb, p_page->buffer) != 0) {
        PFCACHE_FreeDataPage(p_vol, p_page);
        pdm_part_unmount(p_vol->p_part, PDM_UNMOUNT_NORMAL);
        return PF_IERR_INVALID_FORMAT;
    }

    p_vol->num_free_clusters = PDM_FSI_UNKNOWN;
    p_vol->last_free_cluster = PDM_FSI_UNKNOWN;
    if (p_vol->bpb.fat_type == FAT_32) {
        err = pdm_part_logical_read(p_vol->p_part, p_page->buffer, p_vol->bpb.fs_info_sector, 1, PF_CACHE_PAGE_SIZE, &nSector);
        if (err != 0) {
            if (err == PDM_ERR_DRIVER_ERROR) {
                err = pdm_part_get_driver_error_code(p_vol->p_part);
                pf_vol_set.last_driver_error = err;
                p_vol->last_driver_error = err;
                err = PF_IERR_DRIVER;
            } else {
                err = PF_IERR_SYSTEM;
            }
            PFCACHE_FreeDataPage(p_vol, p_page);
            pdm_part_unmount(p_vol->p_part, PDM_UNMOUNT_NORMAL);
            return err;
        }

        if (pdm_bpb_InitVolumeWithFSINFOSector(&p_vol->bpb, p_page->buffer, &p_vol->num_free_clusters, &p_vol->last_free_cluster) != 0) {
            PFCACHE_FreeDataPage(p_vol, p_page);
            pdm_part_unmount(p_vol->p_part, PDM_UNMOUNT_NORMAL);
            return PF_IERR_SYSTEM;
        }
    }

    PFCACHE_FreeDataPage(p_vol, p_page);
    return 0;
}

s32 PFDRV_unmount(PF_VOLUME* p_vol, u32 mode) {
    s32 err;

    err = pdm_part_unmount(p_vol->p_part, (mode == PF_UNMOUNT_FORCE) ? PDM_UNMOUNT_FORCE : PDM_UNMOUNT_NORMAL);
    if (err != 0) {
        if (err == PDM_ERR_DRIVER_ERROR) {
            err = pdm_part_get_driver_error_code(p_vol->p_part);
            pf_vol_set.last_driver_error = err;
            p_vol->last_driver_error = err;
            return PF_IERR_DRIVER;
        }
        return PF_IERR_SYSTEM;
    }
    return 0;
}

s32 PFDRV_format(PF_VOLUME* p_vol, const u8* param) {
    s32 err;

    err = pdm_part_format(p_vol->p_part, param);
    if (err != 0) {
        if (err == PDM_ERR_DRIVER_ERROR) {
            err = pdm_part_get_driver_error_code(p_vol->p_part);
            pf_vol_set.last_driver_error = err;
            p_vol->last_driver_error = err;
            return PF_IERR_DRIVER;
        }
        return PF_IERR_SYSTEM;
    }
    return 0;
}

s32 PFDRV_lread(PF_VOLUME* p_vol, u8* buf, u32 sector, u32 num_sectors, u32* p_num_success) {
    s32 err;

    err = pdm_part_logical_read(p_vol->p_part, buf, sector, num_sectors, p_vol->bpb.bytes_per_sector, p_num_success);
    if (err != 0) {
        if (err == PDM_ERR_DRIVER_ERROR) {
            err = pdm_part_get_driver_error_code(p_vol->p_part);
            pf_vol_set.last_driver_error = err;
            p_vol->last_driver_error = err;
            return PF_IERR_DRIVER;
        }
        return PF_IERR_SYSTEM;
    }
    return 0;
}

s32 PFDRV_lwrite(PF_VOLUME* p_vol, const u8* buf, u32 sector, u32 num_sectors, u32* p_num_success) {
    s32 err;

    err = pdm_part_logical_write(p_vol->p_part, buf, sector, num_sectors, p_vol->bpb.bytes_per_sector, p_num_success);
    if (err != 0) {
        if (err == PDM_ERR_DRIVER_ERROR) {
            err = pdm_part_get_driver_error_code(p_vol->p_part);
            pf_vol_set.last_driver_error = err;
            p_vol->last_driver_error = err;
            return PF_IERR_DRIVER;
        }
        return PF_IERR_SYSTEM;
    }
    return 0;
}
