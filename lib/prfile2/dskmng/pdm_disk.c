#include <prfile2/pdm_disk.h>
#include <prfile2/pdm_partition.h>
#include <prfile2/pf_clib.h>

PDM_DISK_SET pdm_disk_set;
PDM_DISK_TBL pdm_drv_tbl[PDM_DRIVE_COUNT];

s32 pdm_disk_convert_sector_into_block(PDM_DISK* p_disk, u32 sector, u32 num_sector, u16 bps, u32* p_block, u32* p_num_block) {
    s32 err;
    u16 delta;

    *p_block = sector;
    err = 0;
    *p_num_block = num_sector;
    if (bps > PDM_SECTOR_SIZE) {
        delta = bps / PDM_SECTOR_SIZE;
        if (delta == 2) {
            *p_block <<= 1;
        } else if (delta == 4) {
            *p_block <<= 2;
        } else if (delta == 8) {
            *p_block <<= 3;
        }

        if (delta == 2) {
            *p_num_block <<= 1;
        } else if (delta == 4) {
            *p_num_block <<= 2;
        } else if (delta == 8) {
            *p_num_block <<= 3;
        }
    } else if (bps < PDM_SECTOR_SIZE) {
        err = PDM_ERR_INVALID_BPB;
    }
    return err;
}

s32 pdm_disk_convert_block_into_sector(PDM_DISK* p_disk, u32 block, u32 num_block, u16 bps, u32* p_sector, u32* p_num_sector) {
    s32 err;
    u16 delta;

    *p_sector = block;
    err = 0;
    *p_num_sector = num_block;
    if (bps > PDM_SECTOR_SIZE) {
        delta = bps / PDM_SECTOR_SIZE;
        if (delta == 2) {
            *p_sector >>= 1;
        } else if (delta == 4) {
            *p_sector >>= 2;
        } else if (delta == 8) {
            *p_sector >>= 3;
        }

        if (delta == 2) {
            *p_num_sector >>= 1;
        } else if (delta == 4) {
            *p_num_sector >>= 2;
        } else if (delta == 8) {
            *p_num_sector >>= 3;
        }
    } else if (bps < PDM_SECTOR_SIZE) {
        err = PDM_ERR_INVALID_BPB;
    }
    return err;
}

s32 pdm_disk_init_disk_manager(u32 config, void* param) {
    pf_memset(&pdm_disk_set, 0, sizeof(pdm_disk_set));
    return 0;
}

s32 pdm_disk_add_disk(PDM_INIT_DISK* p_init_disk_tbl, PDM_DISK** pp_disk) {
    PDM_DISK* p_disk;
    PDM_DISK_TBL* p_drv_tbl;
    u16 disk_no;

    *pp_disk = NULL;
    if (pdm_disk_set.num_allocated_disk >= PDM_DRIVE_COUNT) {
        return PDM_ERR_NOT_EXIST_FREE_DISK_STRUCT;
    }

    for (disk_no = 0; disk_no < PDM_DRIVE_COUNT; disk_no++) {
        if ((pdm_disk_set.disk[disk_no].status & PDM_DISK_STAT_OPENED) == 0) {
            break;
        }
    }
    if (disk_no >= PDM_DRIVE_COUNT) {
        return PDM_ERR_NOT_EXIST_FREE_DISK_STRUCT;
    }

    p_drv_tbl = &pdm_drv_tbl[disk_no];
    p_init_disk_tbl->p_func(p_drv_tbl, p_init_disk_tbl->ui_ext);

    p_disk = &pdm_disk_set.disk[disk_no];
    p_disk->disk_tbl = *p_drv_tbl;
    p_disk->status |= PDM_DISK_STAT_OPENED;
    pdm_disk_set.num_allocated_disk++;
    *pp_disk = p_disk;
    return 0;
}

s32 pdm_disk_del_disk(PDM_DISK* p_disk) {
    if (p_disk->attach_part_cnt != 0) {
        return PDM_ERR_DISK_ATTACHED;
    }
    if (p_disk->mount_part_cnt != 0) {
        return PDM_ERR_DISK_MOUNTED;
    }
    p_disk->status &= ~PDM_DISK_STAT_OPENED;
    pdm_disk_set.num_allocated_disk--;
    return 0;
}

s32 pdm_disk_set_disk(PDM_DISK* p_disk, PDM_PARTITION* p_part) {
    p_disk->p_cur_part = p_part;
    return 0;
}

s32 pdm_disk_get_media_attribute(PDM_DISK* p_disk, PDM_DISK_INFO* p_disk_info) {
    *p_disk_info = p_disk->disk_info;
    return 0;
}

s32 pdm_disk_get_media_bps(PDM_DISK* p_disk, u16* p_lba_size) {
    *p_lba_size = p_disk->disk_info.bytes_per_sector;
    return 0;
}

s32 pdm_disk_init(PDM_DISK* p_disk) {
    s32 err;

    if (p_disk->attach_part_cnt == 0) {
        err = p_disk->disk_tbl.p_func->init(p_disk);
        if (err != 0) {
            pdm_part_set_driver_error_code(p_disk->p_cur_part, err);
            return PDM_ERR_DRIVER_ERROR;
        }
    }
    p_disk->attach_part_cnt++;
    return 0;
}

s32 pdm_disk_mount(PDM_DISK* p_disk) {
    s32 err;

    if (p_disk->mount_part_cnt == 0) {
        err = p_disk->disk_tbl.p_func->mount(p_disk);
        if (err != 0) {
            pdm_part_set_driver_error_code(p_disk->p_cur_part, err);
            return PDM_ERR_DRIVER_ERROR;
        }
        err = p_disk->disk_tbl.p_func->get_disk_info(p_disk, &p_disk->disk_info);
        if (err != 0) {
            pdm_part_set_driver_error_code(p_disk->p_cur_part, err);
            return PDM_ERR_DRIVER_ERROR;
        }
        if (p_disk->disk_info.media_attr & PDM_MEDIA_ATTR_WRITE_PROTECTED) {
            p_disk->status |= PDM_DISK_STAT_WRITE_PROTECTED;
        } else {
            p_disk->status &= ~PDM_DISK_STAT_WRITE_PROTECTED;
        }
    }
    p_disk->mount_part_cnt++;
    return 0;
}

s32 pdm_disk_format(PDM_DISK* p_disk, const u8* param) {
    s32 err;

    err = p_disk->disk_tbl.p_func->format(p_disk, param);
    if (err != 0) {
        pdm_part_set_driver_error_code(p_disk->p_cur_part, err);
        return PDM_ERR_DRIVER_ERROR;
    }
    return 0;
}

s32 pdm_disk_physical_read(PDM_DISK* p_disk, u8* buf, u32 psector, u32 num_sector, u16 bps, u32* p_num_success) {
    s32 err;
    u32 block;
    u32 num_block;
    u32 tmp_sector;

    err = pdm_disk_convert_sector_into_block(p_disk, psector, num_sector, bps, &block, &num_block);
    if (err != 0) {
        return err;
    }

    err = p_disk->disk_tbl.p_func->physical_read(p_disk, buf, block, num_block, p_num_success);
    pdm_disk_convert_block_into_sector(p_disk, block, *p_num_success, bps, &tmp_sector, p_num_success);
    if (err != 0) {
        pdm_part_set_driver_error_code(p_disk->p_cur_part, err);
        return PDM_ERR_DRIVER_ERROR;
    }
    return 0;
}

s32 pdm_disk_physical_write(PDM_DISK* p_disk, const u8* buf, u32 psector, u32 num_sector, u16 bps, u32* p_num_success) {
    s32 err;
    u32 block;
    u32 num_block;
    u32 tmp_sector;

    err = pdm_disk_convert_sector_into_block(p_disk, psector, num_sector, bps, &block, &num_block);
    if (err != 0) {
        return err;
    }

    err = p_disk->disk_tbl.p_func->physical_write(p_disk, buf, block, num_block, p_num_success);
    pdm_disk_convert_block_into_sector(p_disk, block, *p_num_success, bps, &tmp_sector, p_num_success);
    if (err != 0) {
        pdm_part_set_driver_error_code(p_disk->p_cur_part, err);
        return PDM_ERR_DRIVER_ERROR;
    }
    return 0;
}

s32 pdm_disk_unmount(PDM_DISK* p_disk, u32 mode) {
    s32 err;

    err = 0;
    if (p_disk->mount_part_cnt == 1) {
        err = p_disk->disk_tbl.p_func->unmount(p_disk);
        if (err != 0) {
            pdm_part_set_driver_error_code(p_disk->p_cur_part, err);
            err = PDM_ERR_DRIVER_ERROR;
        }
    } else if (p_disk->mount_part_cnt == 0) {
        return PDM_ERR_NOT_MOUNTED;
    }

    if (err == 0 || mode == PDM_UNMOUNT_FORCE) {
        p_disk->mount_part_cnt--;
    }
    return err;
}

s32 pdm_disk_finalize(PDM_DISK* p_disk) {
    s32 err;

    if (p_disk->attach_part_cnt == 1) {
        err = p_disk->disk_tbl.p_func->finalize(p_disk);
        if (err != 0) {
            pdm_part_set_driver_error_code(p_disk->p_cur_part, err);
            return PDM_ERR_DRIVER_ERROR;
        }
    } else if (p_disk->attach_part_cnt == 0) {
        return PDM_ERR_NOT_ATTACHED;
    }
    p_disk->attach_part_cnt--;
    return 0;
}

void pdm_disk_notify_media_eject(PDM_DISK* p_disk) {
    p_disk->status &= ~PDM_DISK_STAT_INSERTED;
    pdm_part_set_change_media_state(p_disk, PDM_MEDIA_EJECTED);
}

void pdm_disk_notify_media_insert(PDM_DISK* p_disk) {
    p_disk->status |= PDM_DISK_STAT_INSERTED;
    pdm_part_set_change_media_state(p_disk, PDM_MEDIA_INSERTED);
}
