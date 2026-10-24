#include <prfile2/pf_cache.h>
#include <prfile2/pf_clib.h>
#include <prfile2/pf_driver.h>
#include <prfile2/pf_volume.h>

s32 PFCACHE_InitPageList(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE** pp_head, struct PF_CACHE_PAGE* pages, u8 (*buffers)[PF_CACHE_PAGE_SIZE], u16 num, u32 size, u32 is_fat) {
    u32 i;
    u32 bps_per_buf;

    *pp_head = pages;
    bps_per_buf = (p_vol->bpb.bytes_per_sector >> PF_CACHE_PAGE_SIZE_LOG2) * size;

    if (num / bps_per_buf == 0) {
        return PF_IERR_NO_MEMORY;
    }

    if (num / bps_per_buf == 1) {
        pages->p_prev = pages;
        pages->p_next = pages;
        pages->buffer = *buffers;
        pages->p_buf = *buffers;
        pages->p_mod_sbuf = NULL;
        pages->p_mod_ebuf = NULL;
        pages->size = size;
        pages->sector = PF_INVALID_SECTOR;
        pages->option = PF_CACHE_PAGE_OPT_NONE;
        pages->signature = NULL;
        pages->stat &= ~(PF_CACHE_PAGE_STAT_USED | PF_CACHE_PAGE_STAT_MODIFIED);
        if (is_fat) {
            pages->stat |= PF_CACHE_PAGE_STAT_FAT;
        } else {
            pages->stat &= ~PF_CACHE_PAGE_STAT_FAT;
        }
        return 0;
    }

    pages->p_prev = &pages[num - num % bps_per_buf - bps_per_buf];
    pages->p_next = &pages[bps_per_buf];
    pages->buffer = *buffers;
    pages->p_buf = *buffers;
    pages->p_mod_sbuf = NULL;
    pages->p_mod_ebuf = NULL;
    pages->size = size;
    pages->sector = PF_INVALID_SECTOR;
    pages->option = PF_CACHE_PAGE_OPT_NONE;
    pages->signature = NULL;
    pages->stat &= ~(PF_CACHE_PAGE_STAT_USED | PF_CACHE_PAGE_STAT_MODIFIED);
    if (is_fat) {
        pages->stat |= PF_CACHE_PAGE_STAT_FAT;
    } else {
        pages->stat &= ~PF_CACHE_PAGE_STAT_FAT;
    }

    for (i = bps_per_buf; i < num - num % bps_per_buf - bps_per_buf; i += bps_per_buf) {
        pages[i].p_prev = &pages[i - bps_per_buf];
        pages[i].p_next = &pages[i + bps_per_buf];
        pages[i].buffer = &(*buffers)[PF_CACHE_PAGE_SIZE * i];
        pages[i].p_buf = &(*buffers)[PF_CACHE_PAGE_SIZE * i];
        pages[i].p_mod_sbuf = NULL;
        pages[i].p_mod_ebuf = NULL;
        pages[i].size = size;
        pages[i].sector = PF_INVALID_SECTOR;
        pages[i].option = PF_CACHE_PAGE_OPT_NONE;
        pages[i].signature = NULL;
        pages[i].stat &= ~(PF_CACHE_PAGE_STAT_USED | PF_CACHE_PAGE_STAT_MODIFIED);
        if (is_fat) {
            pages[i].stat |= PF_CACHE_PAGE_STAT_FAT;
        } else {
            pages[i].stat &= ~PF_CACHE_PAGE_STAT_FAT;
        }
    }

    pages[num - num % bps_per_buf - bps_per_buf].p_prev = &pages[num - num % bps_per_buf - bps_per_buf - bps_per_buf];
    pages[num - num % bps_per_buf - bps_per_buf].p_next = pages;
    pages[num - num % bps_per_buf - bps_per_buf].buffer = &(*buffers)[PF_CACHE_PAGE_SIZE * (num - num % bps_per_buf - bps_per_buf)];
    pages[num - num % bps_per_buf - bps_per_buf].p_buf = &(*buffers)[PF_CACHE_PAGE_SIZE * (num - num % bps_per_buf - bps_per_buf)];
    pages[num - num % bps_per_buf - bps_per_buf].p_mod_sbuf = NULL;
    pages[num - num % bps_per_buf - bps_per_buf].p_mod_ebuf = NULL;
    pages[num - num % bps_per_buf - bps_per_buf].size = size;
    pages[num - num % bps_per_buf - bps_per_buf].sector = PF_INVALID_SECTOR;
    pages[num - num % bps_per_buf - bps_per_buf].option = PF_CACHE_PAGE_OPT_NONE;
    pages[num - num % bps_per_buf - bps_per_buf].signature = NULL;
    pages[num - num % bps_per_buf - bps_per_buf].stat &= ~(PF_CACHE_PAGE_STAT_USED | PF_CACHE_PAGE_STAT_MODIFIED);
    if (is_fat) {
        pages[num - num % bps_per_buf - bps_per_buf].stat |= PF_CACHE_PAGE_STAT_FAT;
    } else {
        pages[num - num % bps_per_buf - bps_per_buf].stat &= ~PF_CACHE_PAGE_STAT_FAT;
    }
    return 0;
}

struct PF_CACHE_PAGE* PFCACHE_SearchForPage(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE* p_head, u32 sector) {
    struct PF_CACHE_PAGE* p_page;

    if (sector == PF_INVALID_SECTOR) {
        return NULL;
    }

    if (!(p_head->stat & PF_CACHE_PAGE_STAT_USED)) {
        return NULL;
    }

    if (p_head->sector <= sector && p_head->sector + p_head->size - 1 >= sector) {
        p_head->p_buf = p_head->buffer + ((sector - p_head->sector) << p_vol->bpb.log2_bytes_per_sector);
        return p_head;
    }

    for (p_page = p_head->p_next; p_page != p_head; p_page = p_page->p_next) {
        if (!(p_page->stat & PF_CACHE_PAGE_STAT_USED)) {
            return NULL;
        }
        if (p_page->sector <= sector && p_page->sector + p_page->size - 1 >= sector) {
            p_page->p_buf = p_page->buffer + ((sector - p_page->sector) << p_vol->bpb.log2_bytes_per_sector);
            return p_page;
        }
    }

    return NULL;
}

static struct PF_CACHE_PAGE* PFCACHE_SearchForUsedPage(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE** pp_head, struct PF_CACHE_PAGE* p_page) {
    struct PF_CACHE_PAGE* lp_page;

    if (!p_page) {
        lp_page = *pp_head;
    } else {
        lp_page = p_page->p_next;
        if (lp_page == *pp_head) {
            return NULL;
        }
    }

    while ((lp_page->stat & PF_CACHE_PAGE_STAT_USED) != 0) {
        if (lp_page->sector != PF_INVALID_SECTOR) {
            lp_page->p_buf = lp_page->buffer;
            return lp_page;
        }
        lp_page = lp_page->p_next;
    }
    return NULL;
}

u32 PFCACHE_SearchForFreePage(struct PF_CACHE_PAGE* p_head, struct PF_CACHE_PAGE** pp_page) {
    struct PF_CACHE_PAGE* p_page;

    for (p_page = p_head->p_prev; p_page != p_head; p_page = p_page->p_prev) {
        if (!(p_page->stat & PF_CACHE_PAGE_STAT_USED)) {
            *pp_page = p_page;
            return TRUE;
        }
        if (p_page->sector != PF_INVALID_SECTOR) {
            *pp_page = p_page;
            return FALSE;
        }
    }

    if (!(p_page->stat & PF_CACHE_PAGE_STAT_USED)) {
        *pp_page = p_page;
        return TRUE;
    }
    if (p_page->sector != PF_INVALID_SECTOR) {
        *pp_page = p_page;
        return FALSE;
    }

    *pp_page = NULL;
    return FALSE;
}

static void PFCACHE_MovePageToHead(struct PF_CACHE_PAGE** pp_head, struct PF_CACHE_PAGE* p_page) {
    if (p_page != *pp_head) {
        if (p_page == (*pp_head)->p_prev) {
            *pp_head = p_page;
        } else {
            p_page->p_prev->p_next = p_page->p_next;
            p_page->p_next->p_prev = p_page->p_prev;
            p_page->p_next = *pp_head;
            p_page->p_prev = (*pp_head)->p_prev;
            p_page->p_next->p_prev = p_page;
            p_page->p_prev->p_next = p_page;
            *pp_head = p_page;
        }
    }
}

static void PFCACHE_MovePageToTail(struct PF_CACHE_PAGE** pp_head, struct PF_CACHE_PAGE* p_page) {
    if (p_page == *pp_head) {
        *pp_head = p_page->p_next;
    } else if (p_page != (*pp_head)->p_prev) {
        p_page->p_prev->p_next = p_page->p_next;
        p_page->p_next->p_prev = p_page->p_prev;
        p_page->p_next = *pp_head;
        p_page->p_prev = (*pp_head)->p_prev;
        p_page->p_next->p_prev = p_page;
        p_page->p_prev->p_next = p_page;
    }
}

s32 PFCACHE_FlushPageIfNeeded(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE* p_page) {
    u8* p_buf;
    s32 err;
    u32 num_success;
    u32 size;
    u32 sector;

    if (p_page != NULL && (p_page->stat & PF_CACHE_PAGE_STAT_MODIFIED) && p_page->sector != PF_INVALID_SECTOR) {
        p_buf = p_page->p_mod_sbuf;
        if (p_buf == NULL) {
            size = p_page->size;
            err = PFDRV_lwrite(p_vol, p_page->buffer, p_page->sector, size, &num_success);
        } else {
            sector = p_page->sector + ((u32)(p_buf - p_page->buffer) >> p_vol->bpb.log2_bytes_per_sector);
            size = ((u32)p_page->p_mod_ebuf >> p_vol->bpb.log2_bytes_per_sector) - ((u32)p_buf >> p_vol->bpb.log2_bytes_per_sector) + 1;
            err = PFDRV_lwrite(p_vol, p_buf, sector, size, &num_success);
        }
        if (err != 0) {
            return err;
        }

        if (num_success != size) {
            return PF_IERR_IO;
        }

        p_page->p_mod_sbuf = NULL;
        p_page->p_mod_ebuf = NULL;
        p_page->stat &= ~PF_CACHE_PAGE_STAT_MODIFIED;
    }
    return 0;
}

s32 PFCACHE_DoAllocatePage(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE** pp_head, u32 sector, struct PF_CACHE_PAGE** pp_page, u32* p_is_hit) {
    s32 err;

    *pp_page = PFCACHE_SearchForPage(p_vol, *pp_head, sector);
    if (*pp_page) {
        *p_is_hit = TRUE;
    } else {
        *p_is_hit = FALSE;
        if (!PFCACHE_SearchForFreePage(*pp_head, pp_page)) {
            if (!*pp_page) {
                *pp_page = NULL;
                return PF_IERR_NO_MEMORY;
            }

            err = PFCACHE_FlushPageIfNeeded(p_vol, *pp_page);
            if (err != 0) {
                *pp_page = NULL;
                return err;
            }

            (*pp_page)->p_mod_sbuf = NULL;
            (*pp_page)->p_mod_ebuf = NULL;
        }

        if (sector != PF_INVALID_SECTOR) {
            if (((*pp_page)->stat & PF_CACHE_PAGE_STAT_FAT) != 0) {
                (*pp_page)->sector = sector - (sector % (*pp_page)->size);
                (*pp_page)->p_buf = &(*pp_page)->buffer[(sector % (*pp_page)->size) << p_vol->bpb.log2_bytes_per_sector];
            } else {
                if (p_vol->bpb.fat_type == FAT_32 && (sector < p_vol->bpb.first_data_sector || sector >= p_vol->bpb.total_sectors) && sector == p_vol->bpb.fs_info_sector) {
                    (*pp_page)->size = 1;
                }
                (*pp_page)->sector = sector - (sector % (*pp_page)->size);
                (*pp_page)->p_buf = &(*pp_page)->buffer[(sector % (*pp_page)->size) << p_vol->bpb.log2_bytes_per_sector];
            }
        } else {
            (*pp_page)->sector = sector;
            (*pp_page)->p_buf = (*pp_page)->buffer;
        }

        (*pp_page)->stat |= PF_CACHE_PAGE_STAT_USED;
    }
    PFCACHE_MovePageToHead(pp_head, *pp_page);
    return 0;
}

static void PFCACHE_DoFreeCachePage(struct PF_CACHE_PAGE** pp_head, struct PF_CACHE_PAGE* p_page) {
    p_page->stat &= ~PF_CACHE_PAGE_STAT_USED;
    p_page->stat &= ~PF_CACHE_PAGE_STAT_MODIFIED;
    p_page->p_mod_sbuf = NULL;
    p_page->p_mod_ebuf = NULL;
    p_page->sector = PF_INVALID_SECTOR;
    p_page->signature = NULL;
    PFCACHE_MovePageToTail(pp_head, p_page);
}

s32 PFCACHE_DoReadPage(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE** pp_head, u32 sector, struct PF_CACHE_PAGE** pp_page, u32 set_sig) {
    u32 num_success;
    u32 is_hit;
    s32 err;

    err = PFCACHE_DoAllocatePage(p_vol, pp_head, sector, pp_page, &is_hit);
    if (err) {
        return err;
    }

    if (!is_hit) {
        err = PFDRV_lread(p_vol, (*pp_page)->buffer, (*pp_page)->sector, (*pp_page)->size, &num_success);
        if (err) {
            PFCACHE_DoFreeCachePage(pp_head, *pp_page);
            return err;
        }
        if (num_success != (*pp_page)->size && p_vol->bpb.total_sectors != (*pp_page)->sector + num_success) {
            PFCACHE_DoFreeCachePage(pp_head, *pp_page);
            return PF_IERR_IO;
        }

        (*pp_page)->stat &= ~PF_CACHE_PAGE_STAT_MODIFIED;
        (*pp_page)->p_mod_sbuf = NULL;
        (*pp_page)->p_mod_ebuf = NULL;

        if (set_sig) {
            (*pp_page)->signature = p_vol->cache.signature;
        } else {
            (*pp_page)->signature = NULL;
        }
    }
    return 0;
}

s32 PFCACHE_DoReadPageAndFlushIfNeeded(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE** pp_head, u32 sector, struct PF_CACHE_PAGE** pp_page, u32 set_sig) {
    u32 num_success;
    u32 is_hit;
    s32 err;

    err = PFCACHE_DoAllocatePage(p_vol, pp_head, sector, pp_page, &is_hit);
    if (err) {
        return err;
    }

    if (!is_hit) {
        err = PFDRV_lread(p_vol, (*pp_page)->buffer, (*pp_page)->sector, (*pp_page)->size, &num_success);
        if (err) {
            PFCACHE_DoFreeCachePage(pp_head, *pp_page);
            return err;
        }
        if (num_success != (*pp_page)->size && p_vol->bpb.total_sectors != (*pp_page)->sector + num_success) {
            PFCACHE_DoFreeCachePage(pp_head, *pp_page);
            return PF_IERR_IO;
        }

        (*pp_page)->stat &= ~PF_CACHE_PAGE_STAT_MODIFIED;
        (*pp_page)->p_mod_sbuf = NULL;
        (*pp_page)->p_mod_ebuf = NULL;

        if (set_sig) {
            (*pp_page)->signature = p_vol->cache.signature;
        } else {
            (*pp_page)->signature = NULL;
        }

    } else if (set_sig && ((*pp_page)->stat & PF_CACHE_PAGE_STAT_MODIFIED) != 0 && (p_vol->cache.mode & PF_CACHE_MODE_FLUSH_ON_CLOSE) != 0 && (*pp_page)->signature != p_vol->cache.signature) {
        err = PFCACHE_FlushPageIfNeeded(p_vol, *pp_page);
        if (err) {
            *pp_page = NULL;
            return err;
        }
    }

    return 0;
}

s32 PFCACHE_DoReadNumSector(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE** pp_head, u8* p_buf, u32 sector, u32 num_sector, u32* p_num_success) {
    struct PF_CACHE_PAGE* p_page;
    s32 err;
    u32 num_rest_sector;
    u32 num_success_sector;

    *p_num_success = 0;

    err = PFDRV_lread(p_vol, p_buf, sector, num_sector, p_num_success);
    if (err) {
        return err;
    }

    p_page = NULL;
    num_success_sector = *p_num_success;
    num_rest_sector = *p_num_success;

    do {
        p_page = PFCACHE_SearchForUsedPage(p_vol, pp_head, p_page);

        if (p_page != NULL && p_page->sector != PF_INVALID_SECTOR) {
            if (p_page->sector <= sector && p_page->sector + p_page->size >= sector + num_success_sector) {
                pf_memcpy(p_buf, &p_page->buffer[(sector - p_page->sector) << p_vol->bpb.log2_bytes_per_sector], num_success_sector << p_vol->bpb.log2_bytes_per_sector);
                num_rest_sector -= num_success_sector;
            } else if (p_page->sector >= sector && p_page->sector + p_page->size <= sector + num_success_sector) {
                pf_memcpy(&p_buf[(p_page->sector - sector) << p_vol->bpb.log2_bytes_per_sector], p_page->buffer, p_page->size << p_vol->bpb.log2_bytes_per_sector);
                num_rest_sector -= p_page->size;
            } else if (p_page->sector > sector && p_page->sector < sector + num_success_sector && p_page->sector + p_page->size >= sector + num_success_sector) {
                pf_memcpy(&p_buf[(p_page->sector - sector) << p_vol->bpb.log2_bytes_per_sector], p_page->buffer, (p_page->size - ((p_page->sector + p_page->size) - (sector + num_success_sector))) << p_vol->bpb.log2_bytes_per_sector);
                num_rest_sector -= p_page->size - ((p_page->sector + p_page->size) - (sector + num_success_sector));
            } else if (p_page->sector < sector && p_page->sector + p_page->size > sector && p_page->sector + p_page->size <= sector + num_success_sector) {
                pf_memcpy(p_buf, &p_page->buffer[(sector - p_page->sector) << p_vol->bpb.log2_bytes_per_sector], (p_page->size - (sector - p_page->sector)) << p_vol->bpb.log2_bytes_per_sector);
                num_rest_sector -= p_page->size - (sector - p_page->sector);
            }
        }
    } while (p_page && num_rest_sector);

    if (*p_num_success != num_sector && sector + *p_num_success != p_vol->bpb.total_sectors) {
        return PF_IERR_IO;
    }
    return 0;
}

s32 PFCACHE_DoWritePage(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE** pp_head, struct PF_CACHE_PAGE* p_page, u32 set_sig) {
    s32 err;

    PFCACHE_MovePageToHead(pp_head, p_page);

    p_page->stat |= PF_CACHE_PAGE_STAT_MODIFIED;
    if (p_page->p_mod_sbuf == NULL) {
        p_page->p_mod_sbuf = p_page->p_buf;
        p_page->p_mod_ebuf = p_page->p_buf;
    } else {
        if (p_page->p_buf < p_page->p_mod_sbuf) {
            p_page->p_mod_sbuf = p_page->p_buf;
        } else if (p_page->p_buf > p_page->p_mod_ebuf) {
            p_page->p_mod_ebuf = p_page->p_buf;
        }
    }

    if (set_sig) {
        p_page->signature = p_vol->cache.signature;
    } else {
        p_page->signature = NULL;
    }

    if ((p_vol->cache.mode & PF_CACHE_MODE_WRITE_THROUGH) != 0 || ((p_vol->cache.mode & PF_CACHE_MODE_SYNC_UPDATES) != 0 && (p_page->stat & PF_CACHE_PAGE_STAT_FAT) != 0)) {
        err = PFCACHE_FlushPageIfNeeded(p_vol, p_page);
        if (err) {
            return err;
        }
    }

    return 0;
}

s32 PFCACHE_DoWriteSector(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE** pp_head, const u8* p_buf, u32 sector) {
    struct PF_CACHE_PAGE* p_page;
    u32 num_success;
    s32 err;

    p_page = PFCACHE_SearchForPage(p_vol, *pp_head, sector);

    if (p_page) {
        pf_memcpy(p_page->p_buf, p_buf, p_vol->bpb.bytes_per_sector);

        p_page->stat |= PF_CACHE_PAGE_STAT_MODIFIED;
        if (p_page->p_mod_sbuf == NULL) {
            p_page->p_mod_sbuf = p_page->p_buf;
            p_page->p_mod_ebuf = p_page->p_buf;
        } else {
            if (p_page->p_buf < p_page->p_mod_sbuf) {
                p_page->p_mod_sbuf = p_page->p_buf;
            } else if (p_page->p_buf > p_page->p_mod_ebuf) {
                p_page->p_mod_ebuf = p_page->p_buf;
            }
        }

        err = PFCACHE_FlushPageIfNeeded(p_vol, p_page);
        if (err) {
            return err;
        }
    } else {
        err = PFDRV_lwrite(p_vol, p_buf, sector, 1, &num_success);
        if (err) {
            return err;
        }

        if (num_success != 1) {
            return PF_IERR_IO;
        }
    }

    return 0;
}

s32 PFCACHE_DoWriteNumSectorAndFreeIfNeeded(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE** pp_head, const u8* p_buf, u32 sector, u32 num_sector, u32* p_num_success) {
    struct PF_CACHE_PAGE* p_page;
    s32 err;
    u32 num_rest_sector;
    u8* p_sbuf;
    u8* p_ebuf;

    p_page = NULL;
    num_rest_sector = num_sector;
    *p_num_success = 0;

    do {
        p_page = PFCACHE_SearchForUsedPage(p_vol, pp_head, p_page);

        if (p_page != NULL && p_page->sector != PF_INVALID_SECTOR) {
            if (p_page->sector <= sector && p_page->sector + p_page->size >= sector + num_sector) {
                pf_memcpy(&p_page->buffer[(sector - p_page->sector) << p_vol->bpb.log2_bytes_per_sector], p_buf, num_sector << p_vol->bpb.log2_bytes_per_sector);
                *p_num_success += num_rest_sector;
                num_rest_sector = 0;
                p_page->stat |= PF_CACHE_PAGE_STAT_MODIFIED;
                p_sbuf = &p_page->buffer[(sector - p_page->sector) << p_vol->bpb.log2_bytes_per_sector];
                p_ebuf = p_sbuf + ((num_sector - 1) << p_vol->bpb.log2_bytes_per_sector);
                if (!p_page->p_mod_sbuf) {
                    p_page->p_mod_sbuf = p_sbuf;
                    p_page->p_mod_ebuf = p_ebuf;
                } else if (p_sbuf < p_page->p_mod_sbuf) {
                    p_page->p_mod_sbuf = p_sbuf;
                } else if (p_page->p_mod_ebuf < p_ebuf) {
                    p_page->p_mod_ebuf = p_ebuf;
                }
            } else if (p_page->sector >= sector && p_page->sector + p_page->size <= sector + num_sector) {
                pf_memcpy(p_page->buffer, &p_buf[(p_page->sector - sector) << p_vol->bpb.log2_bytes_per_sector], p_page->size << p_vol->bpb.log2_bytes_per_sector);
                num_rest_sector -= p_page->size;
                *p_num_success += p_page->size;
                p_page->stat |= PF_CACHE_PAGE_STAT_MODIFIED;
                p_page->p_mod_sbuf = p_page->buffer;
                p_page->p_mod_ebuf = &p_page->buffer[(p_page->size - 1) << p_vol->bpb.log2_bytes_per_sector];
            } else if (p_page->sector > sector && p_page->sector < sector + num_sector && p_page->sector + p_page->size >= sector + num_sector) {
                pf_memcpy(p_page->buffer, &p_buf[(p_page->sector - sector) << p_vol->bpb.log2_bytes_per_sector], (p_page->size - ((p_page->sector + p_page->size) - (sector + num_sector))) << p_vol->bpb.log2_bytes_per_sector);
                num_rest_sector -= p_page->size - ((p_page->sector + p_page->size) - (sector + num_sector));
                *p_num_success += p_page->size - ((p_page->sector + p_page->size) - (sector + num_sector));
                p_page->stat |= PF_CACHE_PAGE_STAT_MODIFIED;
                p_ebuf = &p_page->buffer[(p_page->size - ((p_page->sector + p_page->size) - (sector + num_sector)) - 1) << p_vol->bpb.log2_bytes_per_sector];
                p_page->p_mod_sbuf = p_page->buffer;
                if (!p_page->p_mod_ebuf || p_page->p_mod_ebuf < p_ebuf) {
                    p_page->p_mod_ebuf = p_ebuf;
                }
            } else if (p_page->sector < sector && p_page->sector + p_page->size > sector && p_page->sector + p_page->size <= sector + num_sector) {
                pf_memcpy(&p_page->buffer[(sector - p_page->sector) << p_vol->bpb.log2_bytes_per_sector], p_buf, (p_page->size - (sector - p_page->sector)) << p_vol->bpb.log2_bytes_per_sector);
                num_rest_sector -= p_page->size - (sector - p_page->sector);
                *p_num_success += p_page->size - (sector - p_page->sector);
                p_page->stat |= PF_CACHE_PAGE_STAT_MODIFIED;
                p_sbuf = &p_page->buffer[(p_page->size - (sector - p_page->sector) - 1) << p_vol->bpb.log2_bytes_per_sector];
                if (!p_page->p_mod_sbuf || p_sbuf < p_page->p_mod_sbuf) {
                    p_page->p_mod_sbuf = p_sbuf;
                }
                p_page->p_mod_ebuf = &p_page->buffer[(p_page->size - 1) << p_vol->bpb.log2_bytes_per_sector];
            }
        }
    } while (p_page && num_rest_sector);

    if (num_rest_sector != 0) {
        err = PFDRV_lwrite(p_vol, p_buf, sector, num_sector, p_num_success);
        if (err) {
            return err;
        }
    }

    return 0;
}

s32 PFCACHE_DoFlushCache(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE* p_head) {
    struct PF_CACHE_PAGE* p_page;
    s32 err;
    s32 first_err;

    first_err = 0;
    p_page = p_head;

    if ((p_head->stat & PF_CACHE_PAGE_STAT_USED) == 0) {
        return 0;
    }

    if ((p_vol->cache.mode & PF_CACHE_MODE_WRITE_THROUGH) != 0 || ((p_vol->cache.mode & PF_CACHE_MODE_SYNC_UPDATES) != 0 && (p_head->stat & PF_CACHE_PAGE_STAT_FAT) != 0)) {
        return 0;
    }

    do {
        if ((p_page->stat & PF_CACHE_PAGE_STAT_USED) == 0) {
            break;
        }

        err = PFCACHE_FlushPageIfNeeded(p_vol, p_page);
        if (err != 0 && first_err == 0) {
            first_err = err;
        }

        p_page = p_page->p_next;
    } while (p_page != p_head);

    return first_err;
}

void PFCACHE_SetCache(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE* p_cache_page, u8 (*p_cache_buf)[PF_CACHE_PAGE_SIZE], u16 num_fat_pages, u16 num_data_pages) {
    p_vol->cache.pages = p_cache_page;
    p_vol->cache.buffers = p_cache_buf;
    p_vol->cache.num_fat_pages = num_fat_pages;
    p_vol->cache.num_data_pages = num_data_pages;
}

void PFCACHE_SetFATBufferSize(struct PF_VOLUME* p_vol, u32 size) {
    if (size) {
        p_vol->cache.fat_buff_size = size;
    }
}

void PFCACHE_SetDataBufferSize(struct PF_VOLUME* p_vol, u32 size) {
    if (size) {
        p_vol->cache.data_buff_size = size;
    }
}

s32 PFCACHE_InitCaches(struct PF_VOLUME* p_vol) {
    s32 err;

    p_vol->cache.mode &= ~PF_CACHE_MODE_WRITE_THROUGH;
    p_vol->cache.mode |= PF_CACHE_MODE_FLUSH_ON_CLOSE;
    p_vol->cache.mode &= ~PF_CACHE_MODE_SYNC_UPDATES;
    pf_memset(p_vol->cache.buffers, 0, (p_vol->cache.num_fat_pages + p_vol->cache.num_data_pages) * sizeof(*p_vol->cache.buffers));

    err = PFCACHE_InitPageList(p_vol, &p_vol->cache.p_current_fat, p_vol->cache.pages, p_vol->cache.buffers, p_vol->cache.num_fat_pages, p_vol->cache.fat_buff_size, TRUE);
    if (err) {
        return err;
    }

    err = PFCACHE_InitPageList(p_vol, &p_vol->cache.p_current_data, &p_vol->cache.pages[p_vol->cache.num_fat_pages], &p_vol->cache.buffers[p_vol->cache.num_fat_pages], p_vol->cache.num_data_pages, p_vol->cache.data_buff_size, FALSE);
    if (err) {
        return err;
    }
    return 0;
}

s32 PFCACHE_SetWriteThroughMode(struct PF_VOLUME* p_vol) {
    s32 err;

    err = PFCACHE_FlushAllCaches(p_vol);
    if (err) {
        return err;
    }
    p_vol->cache.mode |= PF_CACHE_MODE_WRITE_THROUGH;
    return 0;
}

s32 PFCACHE_SetWriteBackMode(struct PF_VOLUME* p_vol) {
    p_vol->cache.mode &= ~PF_CACHE_MODE_WRITE_THROUGH;
    return 0;
}

s32 PFCACHE_AllocateFATPage(struct PF_VOLUME* p_vol, u32 sector, struct PF_CACHE_PAGE** pp_page, u32* p_is_hit) {
    s32 err;

    if (!p_vol->cache.p_current_fat) {
        p_vol->cache.pages->buffer = *p_vol->cache.buffers;
        p_vol->cache.pages->p_buf = *p_vol->cache.buffers;
        *pp_page = p_vol->cache.pages;
        return 0;
    }

    err = PFCACHE_DoAllocatePage(p_vol, &p_vol->cache.p_current_fat, sector, pp_page, p_is_hit);
    if (err) {
        return err;
    }
    return 0;
}

s32 PFCACHE_AllocateDataPage(struct PF_VOLUME* p_vol, u32 sector, struct PF_CACHE_PAGE** pp_page, u32* p_is_hit) {
    s32 err;

    if (!p_vol->cache.p_current_data) {
        p_vol->cache.pages->buffer = *p_vol->cache.buffers;
        p_vol->cache.pages->p_buf = *p_vol->cache.buffers;
        *pp_page = p_vol->cache.pages;
        return 0;
    }

    err = PFCACHE_DoAllocatePage(p_vol, &p_vol->cache.p_current_data, sector, pp_page, p_is_hit);
    if (err) {
        return err;
    }
    return 0;
}

void PFCACHE_FreeFATPage(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE* p_page) {
    if (p_vol->cache.p_current_fat) {
        p_page->p_mod_sbuf = NULL;
        p_page->p_mod_ebuf = NULL;
        PFCACHE_DoFreeCachePage(&p_vol->cache.p_current_fat, p_page);
    }
}

void PFCACHE_FreeDataPage(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE* p_page) {
    if (p_vol->cache.p_current_data) {
        p_page->p_mod_sbuf = NULL;
        p_page->p_mod_ebuf = NULL;
        PFCACHE_DoFreeCachePage(&p_vol->cache.p_current_data, p_page);
    }
}

s32 PFCACHE_ReadFATPage(struct PF_VOLUME* p_vol, u32 sector, struct PF_CACHE_PAGE** pp_page) {
    s32 err;

    err = PFCACHE_DoReadPage(p_vol, &p_vol->cache.p_current_fat, sector, pp_page, FALSE);
    if (err) {
        return err;
    }
    return 0;
}

s32 PFCACHE_ReadDataPage(struct PF_VOLUME* p_vol, u32 sector, struct PF_CACHE_PAGE** pp_page, u32 set_sig) {
    s32 err;

    err = PFCACHE_DoReadPage(p_vol, &p_vol->cache.p_current_data, sector, pp_page, set_sig);
    if (err) {
        return err;
    }
    return 0;
}

s32 PFCACHE_ReadDataPageAndFlushIfNeeded(struct PF_VOLUME* p_vol, u32 sector, struct PF_CACHE_PAGE** pp_page, u32 set_sig) {
    s32 err;

    err = PFCACHE_DoReadPageAndFlushIfNeeded(p_vol, &p_vol->cache.p_current_data, sector, pp_page, set_sig);
    if (err) {
        return err;
    }
    return 0;
}

s32 PFCACHE_ReadDataNumSector(struct PF_VOLUME* p_vol, u8* p_buf, u32 sector, u32 num_sector, u32* p_num_success) {
    s32 err;

    if (sector >= p_vol->bpb.total_sectors) {
        return PF_IERR_OUT_OF_RANGE;
    }

    err = PFCACHE_DoReadNumSector(p_vol, &p_vol->cache.p_current_data, p_buf, sector, num_sector, p_num_success);
    if (err) {
        return err;
    }
    return 0;
}

s32 PFCACHE_WriteFATPage(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE* p_page) {
    s32 err;

    err = PFCACHE_DoWritePage(p_vol, &p_vol->cache.p_current_fat, p_page, FALSE);
    if (err) {
        return err;
    }
    return 0;
}

s32 PFCACHE_WriteDataPage(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE* p_page, u32 set_sig) {
    s32 err;

    err = PFCACHE_DoWritePage(p_vol, &p_vol->cache.p_current_data, p_page, set_sig);
    if (err) {
        return err;
    }
    return 0;
}

s32 PFCACHE_WriteFATSectorAndFreeIfNeeded(struct PF_VOLUME* p_vol, const u8* p_buf, u32 sector) {
    s32 err;

    if (sector >= p_vol->bpb.total_sectors) {
        return PF_IERR_OUT_OF_RANGE;
    }

    err = PFCACHE_DoWriteSector(p_vol, &p_vol->cache.p_current_fat, p_buf, sector);
    if (err) {
        return err;
    }
    return 0;
}

s32 PFCACHE_WriteDataNumSectorAndFreeIfNeeded(struct PF_VOLUME* p_vol, const u8* p_buf, u32 sector, u32 num_sector, u32* p_num_success) {
    s32 err;

    if (sector >= p_vol->bpb.total_sectors) {
        return PF_IERR_OUT_OF_RANGE;
    }
    err = PFCACHE_DoWriteNumSectorAndFreeIfNeeded(p_vol, &p_vol->cache.p_current_data, p_buf, sector, num_sector, p_num_success);
    if (err) {
        return err;
    }
    return 0;
}

struct PF_CACHE_PAGE* PFCACHE_SearchDataCache(struct PF_VOLUME* p_vol, u32 sector) {
    return PFCACHE_SearchForPage(p_vol, p_vol->cache.p_current_data, sector);
}

s32 PFCACHE_FlushFATCache(struct PF_VOLUME* p_vol) {
    s32 err;

    err = PFCACHE_DoFlushCache(p_vol, p_vol->cache.p_current_fat);
    if (err) {
        return err;
    }
    return 0;
}

s32 PFCACHE_FlushDataCache(struct PF_VOLUME* p_vol) {
    s32 err;

    err = PFCACHE_DoFlushCache(p_vol, p_vol->cache.p_current_data);
    if (err) {
        return err;
    }
    return 0;
}

s32 PFCACHE_FlushDataCacheSpecific(struct PF_VOLUME* p_vol, void* signature) {
    s32 err;
    struct PF_CACHE_PAGE* p_head;
    struct PF_CACHE_PAGE* p_page;

    err = 0;

    if ((p_vol->cache.mode & (PF_CACHE_MODE_WRITE_THROUGH | PF_CACHE_MODE_FLUSH_ON_CLOSE | PF_CACHE_MODE_SYNC_UPDATES)) != 0) {
        p_head = p_vol->cache.p_current_data;

        if (p_head->signature == NULL || p_head->signature == signature) {
            err = PFCACHE_FlushPageIfNeeded(p_vol, p_head);
            if (err) {
                return err;
            }
        }

        for (p_page = p_head->p_next; p_page != p_head; p_page = p_page->p_next) {
            if (p_page->signature == NULL || p_page->signature == signature) {
                err = PFCACHE_FlushPageIfNeeded(p_vol, p_page);
                if (err) {
                    return err;
                }
            }
        }
    }

    return err;
}

s32 PFCACHE_FlushAllCaches(struct PF_VOLUME* p_vol) {
    s32 err;

    err = PFCACHE_DoFlushCache(p_vol, p_vol->cache.p_current_fat);
    if (err) {
        return err;
    }

    err = PFCACHE_DoFlushCache(p_vol, p_vol->cache.p_current_data);
    if (err) {
        return err;
    }
    return 0;
}

void PFCACHE_FreeAllCaches(struct PF_VOLUME* p_vol) {
    while ((p_vol->cache.p_current_fat->stat & PF_CACHE_PAGE_STAT_USED) != 0) {
        PFCACHE_FreeFATPage(p_vol, p_vol->cache.p_current_fat);
    }
    while ((p_vol->cache.p_current_data->stat & PF_CACHE_PAGE_STAT_USED) != 0) {
        PFCACHE_FreeDataPage(p_vol, p_vol->cache.p_current_data);
    }
}
