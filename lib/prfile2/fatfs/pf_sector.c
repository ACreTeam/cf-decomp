#include <prfile2/pf_cache.h>
#include <prfile2/pf_clib.h>
#include <prfile2/pf_driver.h>
#include <prfile2/pf_sector.h>

s32 PFSEC_ReadFAT(struct PF_VOLUME* p_vol, u8* p_buf, u32 sector, u16 offset, u16 size) {
    struct PF_CACHE_PAGE* p_page;
    s32 err;

    if (((p_vol->flags & PF_VOL_FLAG_ATTACHED) == 0) || !PFDRV_IsInserted(p_vol)) {
        return PF_IERR_NOT_MOUNTED;
    }
    if (sector >= p_vol->bpb.total_sectors || offset >= p_vol->bpb.bytes_per_sector ||
        offset + size > p_vol->bpb.bytes_per_sector) {
        return PF_IERR_INVALID_PARAM;
    }
    err = PFCACHE_ReadFATPage(p_vol, sector, &p_page);
    if (err) {
        return err;
    }

    pf_memcpy(p_buf, &p_page->p_buf[offset], size);
    return 0;
}

s32 PFSEC_ReadData(struct PF_VOLUME* p_vol, u8* p_buf, u32 sector, u16 offset, u32 size, u32* p_success_size, u32 set_sig) {
    struct PF_CACHE_PAGE* p_page;
    u32 num_success;
    u32 num_sector;
    u32 adjust_sector;
    u32 rest_sector;
    s32 err;

    *p_success_size = 0;
    if (((p_vol->flags & PF_VOL_FLAG_ATTACHED) == 0) || !PFDRV_IsInserted(p_vol)) {
        return PF_IERR_NOT_MOUNTED;
    }
    if ((sector >= p_vol->bpb.total_sectors || offset >= p_vol->bpb.bytes_per_sector ||
         offset + size > p_vol->bpb.bytes_per_sector) &&
        offset != 0) {
        return PF_IERR_INVALID_PARAM;
    }
    if (offset != 0 || size < p_vol->bpb.bytes_per_sector) {
        err = PFCACHE_ReadDataPage(p_vol, sector, &p_page, set_sig);
        if (err) {
            return err;
        }
        pf_memcpy(p_buf, &p_page->p_buf[offset], size);
        *p_success_size = size;
    } else if (offset == 0 && (size & (p_vol->bpb.bytes_per_sector - 1)) == 0) {
        err = PFCACHE_ReadDataNumSector(p_vol, p_buf, sector, size >> p_vol->bpb.log2_bytes_per_sector, &num_success);
        if (err) {
            return err;
        }
        *p_success_size += num_success << p_vol->bpb.log2_bytes_per_sector;
        if (num_success != (size >> p_vol->bpb.log2_bytes_per_sector)) {
            return PF_IERR_IO;
        }
    } else if (offset == 0) {
        p_page = PFCACHE_SearchDataCache(p_vol, sector);
        if (p_page) {
            rest_sector = p_page->sector + p_page->size - sector;
            num_sector = size >> p_vol->bpb.log2_bytes_per_sector;
            if (rest_sector <= num_sector) {
                pf_memcpy(p_buf, p_page->p_buf, rest_sector << p_vol->bpb.log2_bytes_per_sector);
                *p_success_size += rest_sector << p_vol->bpb.log2_bytes_per_sector;
                sector += rest_sector;
                size -= rest_sector << p_vol->bpb.log2_bytes_per_sector;
            } else if (rest_sector > num_sector) {
                pf_memcpy(p_buf, p_page->p_buf, size);
                *p_success_size += size;
                size = 0;
            }
        }
        if (size != 0) {
            num_sector = size >> p_vol->bpb.log2_bytes_per_sector;
            rest_sector = (sector + num_sector) % p_vol->cache.data_buff_size;
            if (num_sector > rest_sector) {
                adjust_sector = num_sector - rest_sector;
                err = PFCACHE_ReadDataNumSector(p_vol, p_buf + *p_success_size, sector, adjust_sector, &num_success);
                if (err) {
                    return err;
                }
                *p_success_size += num_success << p_vol->bpb.log2_bytes_per_sector;
                if (num_success != adjust_sector) {
                    return PF_IERR_IO;
                }
                sector += adjust_sector;
                num_sector -= adjust_sector;
            }
            if (num_sector != 0) {
                err = PFCACHE_ReadDataPage(p_vol, sector, &p_page, set_sig);
                if (err) {
                    return err;
                }
                pf_memcpy(p_buf + *p_success_size, p_page->p_buf, num_sector << p_vol->bpb.log2_bytes_per_sector);
                *p_success_size += num_sector << p_vol->bpb.log2_bytes_per_sector;
            }
        }
    } else {
        return PF_IERR_INVALID_PARAM;
    }
    return 0;
}

s32 PFSEC_WriteFAT(struct PF_VOLUME* p_vol, const u8* p_buf, u32 sector, u16 offset, u16 size) {
    struct PF_CACHE_PAGE* p_page;
    s32 err;

    if ((p_vol->flags & PF_VOL_FLAG_ATTACHED) == 0 || !PFDRV_IsInserted(p_vol) || (p_vol->flags & PF_VOL_FLAG_WRITE_PROTECTED)) {
        return PF_IERR_NOT_MOUNTED;
    }
    if (sector >= p_vol->bpb.total_sectors || offset >= p_vol->bpb.bytes_per_sector ||
        offset + size > p_vol->bpb.bytes_per_sector) {
        return PF_IERR_INVALID_PARAM;
    }
    err = PFCACHE_ReadFATPage(p_vol, sector, &p_page);
    if (err) {
        return err;
    }

    pf_memcpy(&p_page->p_buf[offset], p_buf, size);
    err = PFCACHE_WriteFATPage(p_vol, p_page);
    if (err) {
        return err;
    }

    return 0;
}

s32 PFSEC_WriteData(struct PF_VOLUME* p_vol, const u8* p_buf, u32 sector, u16 offset, u32 size, u32* p_success_size, u32 set_sig) {
    s32 err;
    struct PF_CACHE_PAGE* p_page;
    u32 num_success;
    u32 num_sector;
    u32 adjust_sector;
    u32 rest_sector;

    *p_success_size = 0;

    if (((p_vol->flags & PF_VOL_FLAG_ATTACHED) == 0) || !PFDRV_IsInserted(p_vol) || (p_vol->flags & PF_VOL_FLAG_WRITE_PROTECTED)) {
        return PF_IERR_NOT_MOUNTED;
    }
    if ((sector >= p_vol->bpb.total_sectors || offset >= p_vol->bpb.bytes_per_sector ||
         offset + size > p_vol->bpb.bytes_per_sector) &&
        offset != 0) {
        return PF_IERR_INVALID_PARAM;
    }
    if (offset != 0 || size < p_vol->bpb.bytes_per_sector) {
        err = PFCACHE_ReadDataPageAndFlushIfNeeded(p_vol, sector, &p_page, set_sig);
        if (err) {
            return err;
        }

        pf_memcpy(&p_page->p_buf[offset], p_buf, size);

        err = PFCACHE_WriteDataPage(p_vol, p_page, set_sig);
        if (err) {
            return err;
        }
        *p_success_size = size;
    } else if (offset == 0 && (size & (p_vol->bpb.bytes_per_sector - 1)) == 0) {
        err = PFCACHE_WriteDataNumSectorAndFreeIfNeeded(p_vol, p_buf, sector, size >> p_vol->bpb.log2_bytes_per_sector, &num_success);
        if (err) {
            return err;
        }

        *p_success_size = num_success << p_vol->bpb.log2_bytes_per_sector;
        if (num_success != (size >> p_vol->bpb.log2_bytes_per_sector)) {
            return PF_IERR_IO;
        }
    } else if (offset == 0) {
        num_sector = size >> p_vol->bpb.log2_bytes_per_sector;
        rest_sector = (sector + num_sector) % p_vol->cache.data_buff_size;

        if (num_sector > rest_sector) {
            adjust_sector = num_sector - rest_sector;
            err = PFCACHE_WriteDataNumSectorAndFreeIfNeeded(p_vol, p_buf, sector, adjust_sector, &num_success);
            if (err) {
                return err;
            }

            *p_success_size = num_success << p_vol->bpb.log2_bytes_per_sector;

            if (num_success != adjust_sector) {
                return PF_IERR_IO;
            }

            sector += adjust_sector;
            num_sector -= adjust_sector;
        }

        if (num_sector != 0) {
            err = PFCACHE_ReadDataPageAndFlushIfNeeded(p_vol, sector, &p_page, set_sig);
            if (err) {
                return err;
            }

            pf_memcpy(p_page->p_buf, p_buf + *p_success_size, num_sector << p_vol->bpb.log2_bytes_per_sector);

            err = PFCACHE_WriteDataPage(p_vol, p_page, set_sig);
            if (err) {
                return err;
            }

            *p_success_size += num_sector << p_vol->bpb.log2_bytes_per_sector;
        }
    } else {
        return PF_IERR_INVALID_PARAM;
    }
    return 0;
}
