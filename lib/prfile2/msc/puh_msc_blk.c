#include <prfile2/puh_msc.h>
#include <prfile2/pdm_disk.h>
#include <revolution/os.h>
#include <string.h>

static PDM_FUNCTBL uhf_msc_blk_func = {
    uhf_msc_blk_init,
    uhf_msc_blk_finalize,
    uhf_msc_blk_mount,
    uhf_msc_blk_unmount,
    uhf_msc_blk_format,
    uhf_msc_blk_pread,
    uhf_msc_blk_pwrite,
    uhf_msc_blk_get_disk_info,
};

UHF_MSC_DEVICE uhg_msc_blk_device_tbl[UHF_MSC_DEVICE_COUNT];
u8* uhg_msc_bounce_buf;

static inline s32 uhf_msc_blk_check_disk(PDM_DISK* p_disk) {
    return (s32)~((u32)p_disk | -(u32)p_disk) >> 31;
}

static UHF_MSC_DEVICE* uhf_msc_blk_get_device(PDM_DISK* p_disk) {
    UHF_MSC_DEVICE* p_dev;
    u32 i;

    p_dev = NULL;
    for (i = 0; i < UHF_MSC_DEVICE_COUNT; i++) {
        if (uhg_msc_blk_device_tbl[i].p_disk == p_disk) {
            p_dev = &uhg_msc_blk_device_tbl[i];
            break;
        }
    }
    return p_dev;
}

s32 usbh_msc_blk_init_drv_tbl(PDM_DISK_TBL* p_disk_tbl, u32 ui_ext) {
    if (p_disk_tbl == NULL) {
        return UHF_MSC_ERR;
    }
    p_disk_tbl->p_func = &uhf_msc_blk_func;
    p_disk_tbl->ui_ext = ui_ext;
    return 0;
}

s32 uhf_msc_blk_init(PDM_DISK* p_disk) {
    UHF_MSC_REQ req = {0};

    req.p_disk = p_disk;

    return _uhf_msc_blk_send_message(&req, UHF_MSC_CMD_BLK_INIT);
}

s32 uhf_msc_blk_get_disk_info(PDM_DISK* p_disk, PDM_DISK_INFO* p_disk_info) {
    UHF_MSC_DEVICE* p_dev;

    if (p_disk == NULL || p_disk_info == NULL) {
        return UHF_MSC_ERR;
    }
    p_dev = uhf_msc_blk_get_device(p_disk);
    if (p_dev == NULL) {
        return UHF_MSC_ERR;
    }

    p_disk_info->total_sectors = p_dev->num_blocks;
    p_disk_info->cylinders = 0;
    p_disk_info->heads = 0;
    p_disk_info->sectors_per_track = 0;
    p_disk_info->bytes_per_sector = p_dev->block_size;
    p_disk_info->media_attr = 0;
    if (p_dev->flags & UHF_MSC_DEV_FLAG_WRITE_PROTECTED) {
        p_disk_info->media_attr |= PDM_MEDIA_ATTR_WRITE_PROTECTED;
    }
    p_disk_info->format_param = NULL;
    return 0;
}

s32 uhf_msc_blk_mount(PDM_DISK* p_disk) {
    UHF_MSC_DEVICE* p_dev;

    if (p_disk == NULL) {
        return UHF_MSC_ERR;
    }
    p_dev = uhf_msc_blk_get_device(p_disk);
    if (p_dev == NULL) {
        return UHF_MSC_ERR;
    }
    if ((p_dev->flags & UHF_MSC_DEV_FLAG_MEDIA_PRESENT) == 0) {
        return UHF_MSC_ERR;
    }
    return 0;
}

s32 uhf_msc_blk_pread(PDM_DISK* p_disk, u8* buf, u32 block, u32 num_block, u32* p_num_success) {
    u16 count;
    UHF_MSC_DEVICE* p_dev;
    u32 size;
    s32 err;
    u32 block_size;
    u8* p_buf;
    u32 max_blocks;
    BOOL direct;
    u8 sense[UHF_MSC_SENSE_LEN] = {0};

    *p_num_success = 0;
    if (p_disk == NULL || buf == NULL) {
        return UHF_MSC_ERR;
    }
    if (num_block == 0) {
        return 0;
    }
    p_dev = uhf_msc_blk_get_device(p_disk);
    if (p_dev == NULL) {
        return UHF_MSC_ERR;
    }
    if ((p_dev->flags & UHF_MSC_DEV_FLAG_MEDIA_PRESENT) == 0) {
        return UHF_MSC_ERR;
    }
    block_size = p_dev->block_size;
    if (block_size == 0) {
        return UHF_MSC_ERR;
    }

    if (!SDOS_IsDmaBuffer(buf)) {
        max_blocks = UHF_MSC_BOUNCE_BUF_SIZE / block_size;
        if (max_blocks == 0) {
            return UHF_MSC_ERR;
        }
        memset(uhg_msc_bounce_buf, 0, UHF_MSC_BOUNCE_BUF_SIZE);
        p_buf = uhg_msc_bounce_buf;
        direct = FALSE;
    } else {
        p_buf = buf;
        max_blocks = UHF_MSC_MAX_TRANSFER_SIZE / block_size;
        direct = TRUE;
    }

    while (1) {
        if (num_block > max_blocks) {
            count = max_blocks;
        } else {
            count = num_block;
        }
        err = usbh_msc_read10(p_dev, block, count, p_buf, sense);
        if (err == 0) {
            if (!direct) {
                size = count * block_size;
                memcpy(buf, p_buf, size);
                buf += size;
            } else {
                p_buf += count * block_size;
            }
            *p_num_success += count;
            block += count;
            num_block -= count;
            if (num_block == 0) {
                break;
            }
        } else {
            if (err == UHF_MSC_ERR_CHECK_CONDITION) {
                switch (__uhf_msc_cmd_check_sense(sense)) {
                    case UHF_MSC_SENSE_MEDIUM_CHANGED:
                        if (p_dev->p_disk != NULL) {
                            pdm_disk_notify_media_insert(p_dev->p_disk);
                        }
                        break;
                    case UHF_MSC_SENSE_MEDIUM_NOT_PRESENT:
                        break;
                }
                err = UHF_MSC_ERR;
            } else {
                err = UHF_MSC_ERR;
            }
            break;
        }
    }
    return err;
}

s32 uhf_msc_blk_pwrite(PDM_DISK* p_disk, const u8* buf, u32 block, u32 num_block, u32* p_num_success) {
    u16 count;
    UHF_MSC_DEVICE* p_dev;
    u32 block_size;
    u8* p_buf;
    u32 max_blocks;
    BOOL direct;
    s32 err;
    u8 sense[UHF_MSC_SENSE_LEN] = {0};

    *p_num_success = 0;
    if (p_disk == NULL || buf == NULL) {
        return UHF_MSC_ERR;
    }
    if (num_block == 0) {
        return 0;
    }
    p_dev = uhf_msc_blk_get_device(p_disk);
    if (p_dev == NULL) {
        return UHF_MSC_ERR;
    }
    if ((p_dev->flags & UHF_MSC_DEV_FLAG_MEDIA_PRESENT) == 0) {
        return UHF_MSC_ERR;
    }
    block_size = p_dev->block_size;
    if (block_size == 0) {
        return UHF_MSC_ERR;
    }

    if (!SDOS_IsDmaBuffer(buf)) {
        max_blocks = UHF_MSC_BOUNCE_BUF_SIZE / block_size;
        if (max_blocks == 0) {
            return UHF_MSC_ERR;
        }
        p_buf = uhg_msc_bounce_buf;
        direct = FALSE;
    } else {
        p_buf = (u8*)buf;
        max_blocks = UHF_MSC_MAX_TRANSFER_SIZE / block_size;
        direct = TRUE;
    }

    while (1) {
        if (num_block > max_blocks) {
            count = max_blocks;
        } else {
            count = num_block;
        }
        if (!direct) {
            memcpy(p_buf, buf, count * block_size);
        }
        err = usbh_msc_write10(p_dev, block, count, p_buf, sense);
        if (err == 0) {
            if (!direct) {
                buf += count * block_size;
            } else {
                p_buf += count * block_size;
            }
            *p_num_success += count;
            block += count;
            num_block -= count;
            if (num_block == 0) {
                break;
            }
        } else {
            if (err == UHF_MSC_ERR_CHECK_CONDITION) {
                switch (__uhf_msc_cmd_check_sense(sense)) {
                    case UHF_MSC_SENSE_MEDIUM_CHANGED:
                        if (p_dev->p_disk != NULL) {
                            pdm_disk_notify_media_insert(p_dev->p_disk);
                        }
                        break;
                    case UHF_MSC_SENSE_MEDIUM_NOT_PRESENT:
                    case UHF_MSC_SENSE_WRITE_PROTECTED:
                        break;
                }
                err = UHF_MSC_ERR;
            } else {
                err = UHF_MSC_ERR;
            }
            break;
        }
    }
    return err;
}

s32 uhf_msc_blk_format(PDM_DISK* p_disk, const u8* param) {
    return uhf_msc_blk_check_disk(p_disk);
}

s32 uhf_msc_blk_finalize(PDM_DISK* p_disk) {
    UHF_MSC_REQ req = {0};

    req.p_disk = p_disk;

    return _uhf_msc_blk_send_message(&req, UHF_MSC_CMD_BLK_FINALIZE);
}

s32 uhf_msc_blk_unmount(PDM_DISK* p_disk) {
    return uhf_msc_blk_check_disk(p_disk);
}

BOOL uhf_msc_is_media_insert(PDM_DISK* p_disk) {
    UHF_MSC_DEVICE* p_dev;

    p_dev = uhf_msc_blk_get_device(p_disk);
    if (p_dev == NULL) {
        return FALSE;
    }
    if ((p_dev->flags & UHF_MSC_DEV_FLAG_MEDIA_PRESENT) == 0) {
        return FALSE;
    }
    pdm_disk_notify_media_insert(p_disk);
    return TRUE;
}

s32 _uhf_msc_blk_send_message(UHF_MSC_REQ* p_req, u32 cmd) {
    s32 sem_id;
    UHF_MSC_MSG* p_msg;
    s32 mbx_id;
    s32 result;

    result = 0;
    mbx_id = uhf_msc_get_message_id();
    p_msg = SDOS_AllocMem(uhg_msc_mem_pool, sizeof(UHF_MSC_MSG), UHF_MSC_MSG_ALIGN);
    if (p_msg == NULL) {
        return UHF_MSC_ERR;
    }

    sem_id = SDOS_CreateSemaphore(0, 0);
    if (sem_id <= 0) {
        SDOS_FreeMem(uhg_msc_mem_pool, p_msg);
        return UHF_MSC_ERR;
    }

    p_msg->cmd = cmd;
    p_msg->sem_id = sem_id;
    p_msg->p_result = &result;
    p_msg->req = *p_req;
    if (SDOS_SendMessage(mbx_id, p_msg, OS_MESSAGE_NOBLOCK) != 0) {
        SDOS_FreeMem(uhg_msc_mem_pool, p_msg);
        SDOS_DeleteSemaphore(sem_id);
        return UHF_MSC_ERR;
    }
    if (SDOS_WaitSemaphore(sem_id) != 0) {
        SDOS_DeleteSemaphore(sem_id);
        return UHF_MSC_ERR;
    }
    if (SDOS_DeleteSemaphore(sem_id) != 0) {
        return UHF_MSC_ERR;
    }
    return result;
}
