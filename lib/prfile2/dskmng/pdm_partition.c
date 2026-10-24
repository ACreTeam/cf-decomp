#include <prfile2/pdm_bpb.h>
#include <prfile2/pdm_disk.h>
#include <prfile2/pdm_mbr.h>
#include <prfile2/pdm_partition.h>

#define MBR_LE32(p) ((p)[0] + ((p)[1] << 8) + ((p)[2] << 16) + ((p)[3] << 24))
#define MBR_START_SECTOR(buf, i) MBR_LE32((buf) + PDM_MBR_PART_ENTRY(i) + PDM_PART_TBL_LBA_START_SECTOR)
#define MBR_NUM_SECTORS(buf, i) MBR_LE32((buf) + PDM_MBR_PART_ENTRY(i) + PDM_PART_TBL_LBA_NUM_SECTORS)

s32 pdm_part_is_master_boot_sector(volatile u8* buf, u32 total_sectors, u32* is_master_boot) {
    s16 part_cnt;
    u32 start_sector[PDM_MBR_PARTITION_COUNT];
    u32 num_sectors[PDM_MBR_PARTITION_COUNT];

    *is_master_boot = FALSE;
    if (buf[PDM_BPB_SIGNATURE1] != PDM_BPB_SIGNATURE1_VALUE || buf[PDM_BPB_SIGNATURE2] != PDM_BPB_SIGNATURE2_VALUE) {
        return PDM_ERR_INVALID_MASTER_BOOT;
    }

    for (part_cnt = 0; part_cnt < PDM_MBR_PARTITION_COUNT; part_cnt++, buf += PDM_MBR_PARTITION_TBL_LEN) {
        start_sector[part_cnt] = 0;
        start_sector[part_cnt] = MBR_START_SECTOR(buf, 0);
        num_sectors[part_cnt] = MBR_NUM_SECTORS(buf, 0);
        if (start_sector[part_cnt] != 0 && num_sectors[part_cnt] != 0) {
            if (part_cnt == 0) {
                *is_master_boot = TRUE;
            }
        } else if (part_cnt == 0) {
            return PDM_ERR_INVALID_MASTER_BOOT;
        }
    }

    for (part_cnt = 0; part_cnt < PDM_MBR_PARTITION_COUNT; part_cnt++) {
        if (start_sector[part_cnt] + num_sectors[part_cnt] > total_sectors) {
            if (part_cnt == 0) {
                *is_master_boot = FALSE;
            }
            return PDM_ERR_INVALID_MASTER_BOOT;
        }
    }
    return 0;
}

u32 pdm_part_is_boot_sector(u8* buf) {
    u32 is_boot;
    u16 byte_per_sector;
    u8 sector_per_cluster;
    u8 media;

    is_boot = TRUE;
    if (buf[PDM_BPB_JUMP_BOOT] != PDM_BPB_JUMP_SHORT || buf[PDM_BPB_JUMP_BOOT + 2] != PDM_BPB_NOP) {
        if (buf[PDM_BPB_JUMP_BOOT] != PDM_BPB_JUMP_NEAR) {
            is_boot = FALSE;
        }
    }
    if (buf[PDM_BPB_SIGNATURE1] != PDM_BPB_SIGNATURE1_VALUE || buf[PDM_BPB_SIGNATURE2] != PDM_BPB_SIGNATURE2_VALUE) {
        is_boot = FALSE;
    }

    byte_per_sector = PDM_LE16(buf, PDM_BPB_BYTES_PER_SECTOR);
    if (byte_per_sector != 512 && byte_per_sector != 1024 && byte_per_sector != 2048 && byte_per_sector != 4096) {
        is_boot = FALSE;
    }

    sector_per_cluster = buf[PDM_BPB_SECTORS_PER_CLUSTER];
    if (sector_per_cluster != 1 && sector_per_cluster != 2 && sector_per_cluster != 4 && sector_per_cluster != 8 && sector_per_cluster != 16 &&
        sector_per_cluster != 32 && sector_per_cluster != 64 && sector_per_cluster != 128) {
        is_boot = FALSE;
    }

    media = buf[PDM_BPB_MEDIA];
    if (media != 0xF0 && media != 0xF8 && media != 0xF9 && media != 0xFA && media != 0xFB && media != 0xFC && media != 0xFD && media != 0xFE &&
        media != 0xFF) {
        is_boot = FALSE;
    }

    return is_boot;
}

s32 pdm_part_get_start_sector(PDM_PARTITION* p_part) {
    s32 err;
    u32 part_index;
    u32 part_id;
    u16 bytes_per_sector = PDM_SECTOR_SIZE;
    u32 num_read = 1;
    u32 sector;
    u32 num_sector;
    u32 ext_start_sector;
    u16 entry;
    u16 i;
    u32 is_master_boot;
    u32 num_success;
    u32 start_sector[PDM_MBR_PARTITION_COUNT];
    u32 num_sectors[PDM_MBR_PARTITION_COUNT];
    u8 buf[PDM_SECTOR_SIZE] ATTRIBUTE_ALIGN(32);

    p_part->start_sector = 0;
    part_id = p_part->part_id;
    p_part->total_sector = p_part->p_disk->disk_info.total_sectors;

    err = pdm_disk_physical_read(p_part->p_disk, buf, PDM_MBR_SECTOR, num_read, bytes_per_sector, &num_success);
    if (err != 0) {
        return err;
    }
    pdm_part_is_master_boot_sector(buf, p_part->p_disk->disk_info.total_sectors, &is_master_boot);
    if (is_master_boot) {
        start_sector[0] = MBR_START_SECTOR(buf, 0);
        start_sector[1] = MBR_START_SECTOR(buf, 1);
        start_sector[2] = MBR_START_SECTOR(buf, 2);
        start_sector[3] = MBR_START_SECTOR(buf, 3);
        num_sectors[0] = MBR_NUM_SECTORS(buf, 0);
        num_sectors[1] = MBR_NUM_SECTORS(buf, 1);
        num_sectors[2] = MBR_NUM_SECTORS(buf, 2);
        num_sectors[3] = MBR_NUM_SECTORS(buf, 3);

        part_index = 0;
        for (i = 0; i < PDM_MBR_PARTITION_COUNT; i++) {
            sector = start_sector[i];
            num_sector = num_sectors[i];
            if (sector == 0 || num_sector == 0) {
                return PDM_ERR_NOT_EXIST_PARTITION;
            }

            err = pdm_disk_physical_read(p_part->p_disk, buf, sector, num_read, bytes_per_sector, &num_success);
            if (err != 0) {
                return err;
            }
            pdm_part_is_master_boot_sector(buf, p_part->p_disk->disk_info.total_sectors, &is_master_boot);
            if (is_master_boot) {
                ext_start_sector = start_sector[i];
                sector = ext_start_sector;
                for (entry = 0; entry < PDM_EBR_ENTRY_COUNT; entry++) {
                    if (entry == PDM_EBR_ENTRY_LOGICAL) {
                        sector += MBR_START_SECTOR(buf, PDM_EBR_ENTRY_LOGICAL);
                        num_sector = MBR_NUM_SECTORS(buf, PDM_EBR_ENTRY_LOGICAL);
                        err = pdm_disk_physical_read(p_part->p_disk, buf, sector, num_read, bytes_per_sector, &num_success);
                        if (err != 0) {
                            return err;
                        }
                        if (part_id == part_index) {
                            p_part->start_sector = sector;
                            p_part->total_sector = num_sector;
                            return 0;
                        }
                        part_index++;
                    } else {
                        sector = ext_start_sector + MBR_START_SECTOR(buf, PDM_EBR_ENTRY_NEXT);
                        num_sector = MBR_NUM_SECTORS(buf, PDM_EBR_ENTRY_NEXT);
                        if (sector == 0 || num_sector == 0) {
                            break;
                        }
                        err = pdm_disk_physical_read(p_part->p_disk, buf, sector, num_read, bytes_per_sector, &num_success);
                        if (err != 0) {
                            return err;
                        }
                        pdm_part_is_master_boot_sector(buf, p_part->p_disk->disk_info.total_sectors, &is_master_boot);
                        if (!is_master_boot) {
                            break;
                        }
                        entry = PDM_EBR_ENTRY_LOGICAL;
                    }
                }
            } else {
                if (part_id == part_index) {
                    p_part->start_sector = sector;
                    p_part->total_sector = num_sector;
                    break;
                }
                part_index++;
            }
        }
    } else {
        if (part_id >= 2) {
            return PDM_ERR_NOT_EXIST_PARTITION;
        }
        p_part->start_sector = 0;
        p_part->total_sector = p_part->p_disk->disk_info.total_sectors;
    }
    return 0;
}

u32 pdm_part_chg_ltop(PDM_PARTITION* p_part, u32 lsector, u16 lbps) {
    u32 psector;
    u32 tmp_start_sector;
    u16 lba_size;
    u16 l_delta;
    u16 p_delta;

    pdm_disk_get_media_bps(p_part->p_disk, &lba_size);

    if (lbps == lba_size) {
        psector = lsector + p_part->start_sector;
    } else {
        p_delta = lba_size / PDM_SECTOR_SIZE;
        tmp_start_sector = p_part->start_sector;

        if (p_delta == 2) {
            tmp_start_sector <<= 1;
        } else if (p_delta == 4) {
            tmp_start_sector <<= 2;
        } else if (p_delta == 8) {
            tmp_start_sector <<= 3;
        }

        l_delta = lbps / PDM_SECTOR_SIZE;
        if (l_delta == 2) {
            tmp_start_sector >>= 1;
        } else if (l_delta == 4) {
            tmp_start_sector >>= 2;
        } else if (l_delta == 8) {
            tmp_start_sector >>= 3;
        }

        psector = lsector + tmp_start_sector;
    }

    return psector;
}

s32 pdm_part_get_partition(PDM_DISK* p_disk, s32 part_id, PDM_PARTITION** pp_part) {
    s32 err;
    u16 part_no;

    *pp_part = NULL;
    if (pdm_disk_set.num_partition >= PDM_DRIVE_COUNT) {
        return PDM_ERR_NOT_EXIST_FREE_PARTITION_STRUCT;
    }

    for (part_no = 0; part_no < PDM_DRIVE_COUNT; part_no++) {
        if ((pdm_disk_set.partition[part_no].status & PDM_PART_STAT_OPENED) == 0) {
            break;
        }
    }
    if (part_no == PDM_DRIVE_COUNT) {
        return PDM_ERR_NOT_EXIST_FREE_PARTITION_STRUCT;
    }

    err = pdm_disk_set_disk(p_disk, &pdm_disk_set.partition[part_no]);
    if (err != 0) {
        return err;
    }

    pdm_disk_set.num_partition++;
    pdm_disk_set.partition[part_no].status |= PDM_PART_STAT_OPENED;
    pdm_disk_set.partition[part_no].p_disk = p_disk;
    pdm_disk_set.partition[part_no].part_id = part_id;
    *pp_part = &pdm_disk_set.partition[part_no];
    return 0;
}

s32 pdm_part_release_partition(PDM_PARTITION* p_part) {
    p_part->status &= ~PDM_PART_STAT_OPENED;
    pdm_disk_set.num_partition--;
    return 0;
}

BOOL pdm_part_is_attached_partition(PDM_PARTITION* p_part) {
    return (p_part->status & PDM_PART_STAT_ATTACHED) != 0;
}

void pdm_part_set_driver_error_code(PDM_PARTITION* p_part, s32 error_code) {
    p_part->driver_last_error = error_code;
}

s32 pdm_part_get_driver_error_code(PDM_PARTITION* p_part) {
    return p_part->driver_last_error;
}

s32 pdm_part_get_media_attribute(PDM_PARTITION* p_part, PDM_DISK_INFO* p_disk_info) {
    s32 err;

    err = pdm_disk_get_media_attribute(p_part->p_disk, p_disk_info);
    if (err != 0) {
        return err;
    }
    return 0;
}

s32 pdm_part_init(PDM_PARTITION* p_part) {
    s32 err;

    if ((p_part->status & PDM_PART_STAT_ATTACHED) == 0) {
        err = pdm_disk_set_disk(p_part->p_disk, p_part);
        if (err != 0) {
            return err;
        }
        err = pdm_disk_init(p_part->p_disk);
        if (err != 0) {
            return err;
        }
    } else {
        return PDM_ERR_ALREADY_ATTACHED;
    }
    p_part->status |= PDM_PART_STAT_ATTACHED;
    return 0;
}

s32 pdm_part_mount(PDM_PARTITION* p_part) {
    s32 err;

    if ((p_part->status & PDM_PART_STAT_MOUNTED) == 0) {
        err = pdm_disk_set_disk(p_part->p_disk, p_part);
        if (err != 0) {
            return err;
        }
        err = pdm_disk_mount(p_part->p_disk);
        if (err != 0) {
            return err;
        }
        err = pdm_part_get_start_sector(p_part);
        if (err != 0) {
            pdm_disk_unmount(p_part->p_disk, PDM_UNMOUNT_NORMAL);
            return err;
        }
    } else {
        return PDM_ERR_ALREADY_MOUNTED;
    }
    p_part->status = (p_part->status & ~PDM_PART_STAT_MEDIA_CHANGED) | PDM_PART_STAT_MOUNTED;
    return 0;
}

s32 pdm_part_format(PDM_PARTITION* p_part, const u8* param) {
    s32 err;

    err = pdm_disk_set_disk(p_part->p_disk, p_part);
    if (err != 0) {
        return err;
    }
    err = pdm_disk_format(p_part->p_disk, param);
    if (err != 0) {
        return err;
    }
    return 0;
}

s32 pdm_part_logical_read(PDM_PARTITION* p_part, u8* buf, u32 lsector, u32 num_sector, u16 bps, u32* p_num_success) {
    s32 err;

    if ((p_part->status & PDM_PART_STAT_MOUNTED) != PDM_PART_STAT_MOUNTED || (p_part->status & PDM_PART_STAT_MEDIA_CHANGED) == PDM_PART_STAT_MEDIA_CHANGED) {
        return PDM_ERR_NOT_ACCESSIBLE;
    }
    err = pdm_disk_set_disk(p_part->p_disk, p_part);
    if (err != 0) {
        return err;
    }
    if (p_part->total_sector <= lsector) {
        return PDM_ERR_OUT_OF_RANGE;
    }
    if (p_part->total_sector < lsector + num_sector) {
        num_sector -= lsector + num_sector - p_part->total_sector;
    }
    err = pdm_disk_physical_read(p_part->p_disk, buf, pdm_part_chg_ltop(p_part, lsector, bps), num_sector, bps, p_num_success);
    if (err != 0) {
        return err;
    }
    return 0;
}

s32 pdm_part_logical_write(PDM_PARTITION* p_part, const u8* buf, u32 lsector, u32 num_sector, u16 bps, u32* p_num_success) {
    s32 err;

    if ((p_part->status & PDM_PART_STAT_MOUNTED) != PDM_PART_STAT_MOUNTED || (p_part->status & PDM_PART_STAT_MEDIA_CHANGED) == PDM_PART_STAT_MEDIA_CHANGED) {
        return PDM_ERR_NOT_ACCESSIBLE;
    }
    err = pdm_disk_set_disk(p_part->p_disk, p_part);
    if (err != 0) {
        return err;
    }
    if (p_part->total_sector <= lsector) {
        return PDM_ERR_OUT_OF_RANGE;
    }
    if (p_part->total_sector < lsector + num_sector) {
        num_sector -= lsector + num_sector - p_part->total_sector;
    }
    err = pdm_disk_physical_write(p_part->p_disk, buf, pdm_part_chg_ltop(p_part, lsector, bps), num_sector, bps, p_num_success);
    if (err != 0) {
        return err;
    }
    return 0;
}

s32 pdm_part_unmount(PDM_PARTITION* p_part, u32 mode) {
    s32 err;
    s32 result;

    result = 0;
    if (p_part->status & PDM_PART_STAT_MOUNTED) {
        err = pdm_disk_set_disk(p_part->p_disk, p_part);
        if (err != 0) {
            return err;
        }
        err = pdm_disk_unmount(p_part->p_disk, mode);
        if (err != 0) {
            result = err;
        }
    } else {
        result = PDM_ERR_NOT_MOUNTED;
    }

    if (result == 0 || mode == PDM_UNMOUNT_FORCE) {
        p_part->status &= ~PDM_PART_STAT_MEDIA_CHANGED;
        p_part->status &= ~PDM_PART_STAT_MOUNTED;
    }
    return result;
}

s32 pdm_part_finalize(PDM_PARTITION* p_part) {
    s32 err;

    if (p_part->status & PDM_PART_STAT_ATTACHED) {
        err = pdm_disk_set_disk(p_part->p_disk, p_part);
        if (err != 0) {
            return err;
        }
        err = pdm_disk_finalize(p_part->p_disk);
        if (err != 0) {
            return err;
        }
    } else {
        return PDM_ERR_NOT_ATTACHED;
    }
    p_part->status &= ~PDM_PART_STAT_ATTACHED;
    return 0;
}

s32 pdm_part_register_callback(PDM_PARTITION* p_part, PDM_PART_FUNC* p_func) {
    p_part->func.insert = p_func->insert;
    p_part->func.eject = p_func->eject;
    return 0;
}

s32 pdm_part_unregister_callback(PDM_PARTITION* p_part) {
    p_part->func.insert = NULL;
    p_part->func.eject = NULL;
    return 0;
}

void pdm_part_set_change_media_state(PDM_DISK* p_disk, u32 event) {
    u16 part_cnt;

    for (part_cnt = 0; part_cnt < PDM_DRIVE_COUNT; part_cnt++) {
        if ((pdm_disk_set.partition[part_cnt].status & PDM_PART_STAT_OPENED) && pdm_disk_set.partition[part_cnt].p_disk == p_disk) {
            pdm_disk_set.partition[part_cnt].status |= PDM_PART_STAT_MEDIA_CHANGED;
            if (event == PDM_MEDIA_EJECTED && pdm_disk_set.partition[part_cnt].func.eject != NULL) {
                pdm_disk_set.partition[part_cnt].func.eject(&pdm_disk_set.partition[part_cnt]);
            } else if (pdm_disk_set.partition[part_cnt].func.insert != NULL) {
                pdm_disk_set.partition[part_cnt].func.insert(&pdm_disk_set.partition[part_cnt]);
            }
        }
    }
}
