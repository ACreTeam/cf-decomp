#include <prfile2/pf_api.h>
#include <prfile2/pf_cache.h>
#include <prfile2/pf_clib.h>
#include <prfile2/pf_cluster.h>
#include <prfile2/pf_entry.h>
#include <prfile2/pf_entry_iterator.h>
#include <prfile2/pf_fat.h>
#include <prfile2/pf_path.h>
#include <prfile2/pf_sector.h>
#include <prfile2/pf_service.h>
#include <prfile2/pf_volume.h>

#define PFENT_ITER_NO_SECTOR_INDEX 0xFFFFFFFF

static s32 PFENT_RecalcEntryIterator(struct PF_ENT_ITER* p_iter, u32 may_allocate) {
    s32 err;
    u32 is_initialize;
    u32 file_sector_index;
    struct PF_CACHE_PAGE* p_page;
    u32 num_sectors;
    u32 success_size;
    u32 is_hit;

    is_initialize = p_iter->file_sector_index;
    file_sector_index = p_iter->index >> p_iter->log2_entries_per_sector;
    if (file_sector_index != is_initialize) {
        p_iter->file_sector_index = file_sector_index;
        err = PFFAT_GetSectorSpecified(&p_iter->ffd, file_sector_index, FALSE, &p_iter->sector);
        if (err != 0) {
            return err;
        }
        if (p_iter->sector == PF_INVALID_SECTOR && may_allocate != 0) {
            err = PFFAT_GetSectorSpecified(&p_iter->ffd, file_sector_index, TRUE, &p_iter->sector);
            if (err != 0) {
                return err;
            }
            if (p_iter->sector != PF_INVALID_SECTOR && is_initialize != PFENT_ITER_NO_SECTOR_INDEX) {
                err = PFCACHE_AllocateDataPage(p_iter->p_vol, PF_INVALID_SECTOR, &p_page, &is_hit);
                if (err != 0) {
                    return err;
                }
                pf_memset(p_page->p_buf, 0, p_iter->p_vol->bpb.bytes_per_sector);
                for (num_sectors = 0; num_sectors < p_iter->p_vol->bpb.sectors_per_cluster; num_sectors++) {
                    err = PFSEC_WriteData(p_iter->p_vol, p_page->p_buf, p_iter->sector + num_sectors, 0, p_iter->p_vol->bpb.bytes_per_sector, &success_size, FALSE);
                    if (err != 0) {
                        PFCACHE_FreeDataPage(p_iter->p_vol, p_page);
                        return err;
                    }
                    if (success_size != p_iter->p_vol->bpb.bytes_per_sector) {
                        PFCACHE_FreeDataPage(p_iter->p_vol, p_page);
                        return PF_IERR_IO;
                    }
                }
                PFCACHE_FreeDataPage(p_iter->p_vol, p_page);
            }
        }
    }
    p_iter->offset = (p_iter->index & p_iter->offset_mask) * PF_DIR_ENTRY_SIZE;
    return 0;
}

static s32 PFENT_ITER_LoadEntry(struct PF_ENT_ITER* p_iter) {
    u32 success_size;
    s32 err;

    if (p_iter->sector == PF_INVALID_SECTOR) {
        return PF_IERR_OUT_OF_RANGE;
    }
    err = PFSEC_ReadData(p_iter->p_vol, p_iter->buf, p_iter->sector, p_iter->offset, PF_DIR_ENTRY_SIZE, &success_size, FALSE);
    if (success_size != PF_DIR_ENTRY_SIZE) {
        return PF_IERR_IO;
    }
    return err;
}

static s32 PFENT_ITER_DoMoveTo(struct PF_ENT_ITER* p_iter, u32 index, u32 may_allocate) {
    u32 prev_index;
    s32 err;

    prev_index = p_iter->index;
    p_iter->index = index;
    err = PFENT_RecalcEntryIterator(p_iter, may_allocate);
    if (err) {
        p_iter->index = prev_index;
        PFENT_RecalcEntryIterator(p_iter, FALSE);
        return err;
    }

    err = PFENT_ITER_LoadEntry(p_iter);
    if (err != 0) {
        return err;
    }
    return 0;
}

static u32 PFENT_ITER_IsAtPhysicalEnd(struct PF_ENT_ITER* p_iter) {
    return p_iter->sector == PF_INVALID_SECTOR;
}

s32 PFENT_ITER_DoFindEntry(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent, struct PF_STR* p_pattern, u8 attr_required, u8 attr_unwanted, u32* p_is_found) {
    u8 attr;
    s32 err;
    s32 lengthName;
    s8 filename[PF_SFN_BUF_LEN];
    u32 index_search_from;
    u32 is_extsfn;

    err = 0;
    *p_is_found = FALSE;
    is_extsfn = FALSE;

    if (PFSTR_StrCmp(p_pattern, (const s8*)"..") == 0 && (p_ent->attr & PF_ATTR_DIR) != 0 && p_ent->start_cluster == PF_CLUSTER_ROOT_DIR) {
        return PF_IERR_INVALID_PATH;
    }
    if (PFSTR_StrCmp(p_pattern, (const s8*)".") == 0) {
        *p_is_found = TRUE;
        return 0;
    }
    p_ent->num_entry_LFNs = 0;
    p_ent->ordinal = 0;
    p_ent->check_sum = 0;
    p_ent->long_name[0] = 0;

    if ((pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) == PF_ENCODE_MODE_UNICODE) {
        is_extsfn = PFPATH_GetExtShortNameIndex(p_pattern, &index_search_from);
    }

    if (is_extsfn == TRUE) {
        err = PFENT_ITER_IteratorInitialize(p_iter, index_search_from - 1);

        for (;; err = PFENT_ITER_Retreat(p_iter, FALSE)) {
            if (err) {
                return err;
            }

            attr = p_iter->buf[PF_DIRENT_ATTR];
            if ((attr & PF_ATTR_LONG_NAME) == PF_ATTR_LONG_NAME) {
                if (PFENT_LoadLFNEntryFieldsFromBuf(p_ent, p_iter->buf) != 0) {
                    return PF_IERR_NOT_FOUND;
                }
                if ((p_iter->buf[0] & PF_LFN_LAST_ENTRY) == 0) {
                    continue;
                }
            } else {
                return 0;
            }

            err = PFENT_ITER_IteratorInitialize(p_iter, index_search_from);
            if (err != 0) {
                return err;
            }

            attr = p_iter->buf[PF_DIRENT_ATTR];
            if (attr == PF_ATTR_NONE) {
                attr = PF_ATTR_NORMAL;
            }

            if ((attr_required & PF_ATTR_STRICT) != 0) {
                attr_required &= PF_ATTR_ALL;
                attr_unwanted &= PF_ATTR_ALL;
                if ((attr_required != 0 && attr_required != (attr_required & attr)) || (attr_unwanted != 0 && attr_unwanted == (attr_unwanted & attr))) {
                    err = PFENT_NO_MATCH;
                }
            } else if (attr_required != PF_ATTR_ALL && attr != attr_required && ((attr & attr_required) == 0 || (attr & attr_unwanted) != 0)) {
                err = PFENT_NO_MATCH;
            }

            if (err == PFENT_NO_MATCH) {
                return 0;
            }
            if ((attr & PF_ATTR_VOLUME) != 0) {
                return PF_IERR_NOT_FOUND;
            }

            PFENT_LoadShortNameFromBuf(p_ent, p_iter->buf);
            if (!PFPATH_MatchFileNameWithPattern(p_ent->short_name, p_pattern, FALSE)) {
                break;
            }
            PFENT_loadEntryNumericFieldsFromBuf(p_ent, p_iter->buf);
            p_ent->entry_sector = p_iter->sector;
            p_ent->entry_offset = p_iter->offset;
            p_ent->p_vol = p_iter->ffd.p_vol;

            if ((p_ent->attr & PF_ATTR_DIR) != 0 && p_ent->start_cluster == PF_CLUSTER_DOTDOT_ROOT) {
                p_ent->start_cluster = PF_CLUSTER_ROOT_DIR;
            }
            *p_is_found = TRUE;
            return 0;
        }
    } else {
        while (!PFENT_ITER_IsAtLogicalEnd(p_iter)) {
            if (err != 0) {
                return err;
            }
            if (p_iter->buf[0] == PF_ENTRY_END) {
                break;
            }
            if (p_iter->buf[0] == PF_ENTRY_DELETED) {
                p_ent->num_entry_LFNs = 0;
                p_ent->long_name[0] = 0;
            } else {
                attr = p_iter->buf[PF_DIRENT_ATTR];
                if ((attr & PF_ATTR_LONG_NAME) == PF_ATTR_LONG_NAME) {
                    if (PFENT_LoadLFNEntryFieldsFromBuf(p_ent, p_iter->buf) != 0) {
                        p_ent->num_entry_LFNs = 0;
                        p_ent->long_name[0] = 0;
                    }
                } else {
                    if (attr == PF_ATTR_NONE) {
                        attr = PF_ATTR_NORMAL;
                    }

                    if ((attr_required & PF_ATTR_STRICT) != 0) {
                        attr_required &= PF_ATTR_ALL;
                        attr_unwanted &= PF_ATTR_ALL;
                        if ((attr_required != 0 && attr_required != (attr_required & attr)) || (attr_unwanted != 0 && attr_unwanted == (attr_unwanted & attr))) {
                            err = PFENT_NO_MATCH;
                        }
                    } else if (attr_required != PF_ATTR_ALL && attr != attr_required && ((attr & attr_required) == 0 || (attr & attr_unwanted) != 0)) {
                        err = PFENT_NO_MATCH;
                    }

                    if (err == PFENT_NO_MATCH) {
                        p_ent->num_entry_LFNs = 0;
                        p_ent->long_name[0] = 0;
                    } else {
                        if ((attr & PF_ATTR_VOLUME) != 0) {
                            p_ent->num_entry_LFNs = 0;
                            p_ent->long_name[0] = 0;
                        }
                        PFENT_LoadShortNameFromBuf(p_ent, p_iter->buf);

                        if (p_ent->num_entry_LFNs != 0 && p_ent->ordinal == PF_LFN_FIRST_ORDINAL && p_ent->check_sum == PFENT_CalcCheckSum(p_ent) && (err = PFPATH_MatchFileNameWithPattern((const s8*)p_ent->long_name, p_pattern, TRUE)) != 0) {
                            PFENT_loadEntryNumericFieldsFromBuf(p_ent, p_iter->buf);
                            p_ent->entry_sector = p_iter->sector;
                            p_ent->entry_offset = p_iter->offset;
                            p_ent->p_vol = p_iter->ffd.p_vol;

                            if ((p_ent->attr & PF_ATTR_DIR) != 0 && p_ent->start_cluster == PF_CLUSTER_DOTDOT_ROOT) {
                                p_ent->start_cluster = PF_CLUSTER_ROOT_DIR;
                            }
                            *p_is_found = TRUE;
                            return 0;
                        }

                        err = PFPATH_MatchFileNameWithPattern(p_ent->short_name, p_pattern, FALSE);
                        if (err != 0) {
                            if ((p_iter->buf[PF_DIRENT_SMALL_LETTER_FLAG] & PF_SMALL_LETTER_MASK) != 0) {
                                PFPATH_getLongNameformShortName(p_ent->short_name, filename, p_iter->buf[PF_DIRENT_SMALL_LETTER_FLAG]);
                                lengthName = PFPATH_transformInUnicode(p_ent->long_name, filename);
                                p_ent->num_entry_LFNs = (lengthName / PF_LFN_CHARS_PER_ENTRY) + ((lengthName % PF_LFN_CHARS_PER_ENTRY) != 0);
                                p_ent->check_sum = PFENT_CalcCheckSum(p_ent);
                                p_ent->ordinal = PF_LFN_FIRST_ORDINAL;
                            }
                            PFENT_loadEntryNumericFieldsFromBuf(p_ent, p_iter->buf);
                            p_ent->entry_sector = p_iter->sector;
                            p_ent->entry_offset = p_iter->offset;
                            p_ent->p_vol = p_iter->ffd.p_vol;

                            if ((p_ent->attr & PF_ATTR_DIR) != 0 && p_ent->start_cluster == PF_CLUSTER_DOTDOT_ROOT) {
                                p_ent->start_cluster = PF_CLUSTER_ROOT_DIR;
                            }
                            *p_is_found = TRUE;
                            return 0;
                        }

                        p_ent->num_entry_LFNs = 0;
                        p_ent->long_name[0] = 0;
                    }
                }
            }
            err = PFENT_ITER_Advance(p_iter, FALSE);
        }
    }
    return 0;
}

s32 PFENT_ITER_DoGetEntry(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent, struct PF_VOLUME* p_vol, struct PF_STR* p_path, u32 wildcard, u32 no_look_last_token) {
    s8* p;
    struct PF_STR token;
    u32 is_found;
    struct PF_FAT_HINT hint;
    u32 start_cluster;
    struct PF_DIR_ENT current_ent;
    s32 err;
    u32 code_mode;

    err = PFSTR_StrNCmp(p_path, (const s8*)":", PF_STR_TARGET_HEAD, PF_DRIVE_LETTER_LEN, 1);
    if (err == 0) {
        PFSTR_MoveStrPos(p_path, PF_DRIVE_SPEC_LEN);
    }

    err = PFSTR_StrNCmp(p_path, (const s8*)"\\", PF_STR_TARGET_HEAD, 0, 1);
    if (err == 0 || PFSTR_StrNCmp(p_path, (const s8*)"/", PF_STR_TARGET_HEAD, 0, 1) == 0) {
        err = PFENT_GetRootDir(p_vol, p_ent);
        if (err != 0) {
            return err;
        }
        if (PFSTR_StrNumChar(p_path, PF_STR_TARGET_HEAD) == 1 && PFSTR_StrNCmp(p_path, (const s8*)"\0\0", PF_STR_TARGET_TAIL, 0, 1) == 0) {
            return 0;
        }
    } else {
        err = PFVOL_GetCurrentDir(p_vol, p_ent);
        if (err != 0) {
            return err;
        }
    }

    p = PFSTR_GetStrPos(p_path, PF_STR_TARGET_HEAD);
    PFFAT_InitFFD(&p_iter->ffd, &hint, p_vol, &p_iter->ffd.start_cluster);
    p_iter->ffd.start_cluster = p_ent->start_cluster;
    code_mode = PFSTR_GetCodeMode(p_path);
    PFPATH_InitTokenOfPath(&token, p, code_mode);
    for (err = PFPATH_GetNextTokenOfPath(&token, wildcard); token.p_head != NULL; err = PFPATH_GetNextTokenOfPath(&token, wildcard)) {
        if (err != 0) {
            return err;
        }
        if (no_look_last_token != 0 && PFSTR_StrNCmp(&token, (const s8*)"\0\0", PF_STR_TARGET_TAIL, 0, 1) == 0) {
            break;
        }
        if ((p_ent->attr & PF_ATTR_DIR) == 0) {
            return PF_IERR_INVALID_PATH;
        }
        if (PFSTR_StrLen(&token) == 0) {
            return PF_IERR_INVALID_PATH;
        }

        p_iter->ffd.start_cluster = p_ent->start_cluster;
        PFFAT_ResetFFD(&p_iter->ffd, &p_iter->ffd.start_cluster);
        err = PFENT_ITER_IteratorInitialize(p_iter, 0);
        if (err != 0) {
            return err;
        }

        start_cluster = *p_iter->ffd.p_start_cluster;
        err = PFENT_ITER_DoFindEntry(p_iter, p_ent, &token, PF_ATTR_ALL, PF_ATTR_NONE, &is_found);
        if (err != 0) {
            return err;
        }

        if (!is_found) {
            err = PFPATH_GetNextTokenOfPath(&token, wildcard);
            if (err != 0) {
                return err;
            }
            if (token.p_head == NULL) {
                return PF_IERR_NOT_FOUND;
            }
            return PF_IERR_INVALID_PATH;
        }

        if (!wildcard) {
            if ((p_ent->attr & PF_ATTR_DIR) != 0 && p_ent->start_cluster == PF_CLUSTER_ROOT_DIR) {
                p_ent->path_len = PFENT_ROOT_PATH_LEN;
            } else if ((p_ent->attr & PF_ATTR_DIR) != 0 && p_ent->short_name[0] == '.' && p_ent->short_name[1] == '.') {
                PFFAT_ResetFFD(&p_iter->ffd, &p_ent->start_cluster);
                err = PFENT_ITER_IteratorInitialize(p_iter, 0);
                if (err != 0) {
                    return err;
                }
                current_ent.attr = p_ent->attr;
                current_ent.start_cluster = p_ent->start_cluster;
                err = PFENT_ITER_FindDirEntryFromCluster(p_iter, &current_ent, start_cluster, &is_found);
                if (err != 0) {
                    return err;
                }
                if (!is_found) {
                    return PF_IERR_INVALID_PATH;
                }
                if (current_ent.num_entry_LFNs != 0) {
                    p_ent->path_len -= PFPATH_GetLengthFromUnicode(current_ent.long_name);
                } else {
                    p_ent->path_len -= PFPATH_GetLengthFromShortname(current_ent.short_name);
                }
                p_ent->path_len -= 1;
            } else if ((p_ent->attr & PF_ATTR_DIR) == 0 || p_ent->short_name[0] != '.') {
                p_ent->path_len += PFSTR_StrNumChar(&token, PF_STR_TARGET_HEAD) + 1;
                if (PFSTR_StrNCmp(&token, (const s8*)"\0\0", PF_STR_TARGET_TAIL, 0, 1) != 0) {
                    p_ent->path_len -= PFSTR_StrNumChar(&token, PF_STR_TARGET_TAIL);
                }
            }
        }
    }

    return 0;
}

s32 PFENT_ITER_IteratorInitialize(struct PF_ENT_ITER* p_iter, u32 index_start_from) {
    p_iter->p_vol = p_iter->ffd.p_vol;
    p_iter->log2_entries_per_sector = p_iter->ffd.p_vol->bpb.log2_bytes_per_sector - PF_DIR_ENTRY_SIZE_LOG2;
    p_iter->offset_mask = (1 << p_iter->log2_entries_per_sector) - 1;
    p_iter->file_sector_index = PFENT_ITER_NO_SECTOR_INDEX;
    return PFENT_ITER_DoMoveTo(p_iter, index_start_from, FALSE);
}

u32 PFENT_ITER_IsAtLogicalEnd(struct PF_ENT_ITER* p_iter) {
    s32 err;
    u32 save_index;
    u32 save_fsindex;

    if (p_iter->sector == PF_INVALID_SECTOR || p_iter->buf[0] == PF_ENTRY_END || p_iter->index >= PF_ENTRY_INDEX_END) {
        save_index = p_iter->index;
        save_fsindex = p_iter->file_sector_index;
        p_iter->index = (p_iter->file_sector_index + 1) << p_iter->log2_entries_per_sector;
        err = PFENT_RecalcEntryIterator(p_iter, FALSE);
        if (err != 0) {
            p_iter->index = save_index;
            p_iter->file_sector_index = save_fsindex;
            return TRUE;
        }
        if (p_iter->sector != PF_INVALID_SECTOR) {
            err = PFENT_ITER_LoadEntry(p_iter);
            return err != 0;
        }
        return TRUE;
    }

    return FALSE;
}

s32 PFENT_ITER_MoveTo(struct PF_ENT_ITER* p_iter, u32 index, u32 may_allocate) {
    s32 err;

    if (p_iter->index != index || !PFENT_ITER_IsAtPhysicalEnd(p_iter) || may_allocate == 0) {
        err = PFENT_ITER_DoMoveTo(p_iter, index, may_allocate);
        if (err != 0) {
            return err;
        }
    }

    return 0;
}

s32 PFENT_ITER_Advance(struct PF_ENT_ITER* p_iter, u32 may_allocate) {
    return PFENT_ITER_MoveTo(p_iter, p_iter->index + 1, may_allocate);
}

s32 PFENT_ITER_FindCluster(struct PF_DIR_ENT* p_ent, u32 cluster, u32* p_is_found) {
    struct PF_ENT_ITER iter;
    struct PF_FAT_HINT hint;
    u32 start_cluster;
    u32 sector_index;
    s32 err;

    *p_is_found = FALSE;
    if (p_ent->entry_sector < p_ent->p_vol->bpb.first_data_sector) {
        return 0;
    }

    start_cluster = (p_ent->entry_sector - p_ent->p_vol->bpb.first_data_sector) >> p_ent->p_vol->bpb.log2_sectors_per_cluster;
    sector_index = (p_ent->entry_sector - p_ent->p_vol->bpb.first_data_sector) & (p_ent->p_vol->bpb.sectors_per_cluster - 1);
    if (start_cluster == cluster) {
        *p_is_found = TRUE;
        return 0;
    }

    iter.ffd.p_start_cluster = &start_cluster;
    iter.ffd.p_hint = &hint;
    iter.ffd.p_vol = p_ent->p_vol;
    iter.ffd.file_version = 1;
    iter.ffd.cluster_link.buffer = NULL;
    iter.log2_entries_per_sector = p_ent->p_vol->bpb.log2_bytes_per_sector - PF_DIR_ENTRY_SIZE_LOG2;
    iter.file_sector_index = sector_index;
    iter.offset_mask = (1 << iter.log2_entries_per_sector) - 1;
    iter.p_vol = p_ent->p_vol;
    iter.sector = p_ent->entry_sector;
    iter.offset = p_ent->entry_offset;
    iter.index = (p_ent->entry_offset / PF_DIR_ENTRY_SIZE) + sector_index * (1 << iter.log2_entries_per_sector);
    hint.chain_index = 0;
    hint.cluster = start_cluster;
    hint.file_version = 1;
    if (iter.index < PFENT_NUM_DOT_ENTRIES) {
        hint.chain_index = 1;
        iter.file_sector_index += p_ent->p_vol->bpb.sectors_per_cluster;
        iter.index += (1 << iter.log2_entries_per_sector) << p_ent->p_vol->bpb.log2_sectors_per_cluster;
    }

    while (iter.index != 0) {
        err = PFENT_ITER_LoadEntry(&iter);
        if (err != 0) {
            return err;
        }
        if (pf_strncmp((const s8*)iter.buf, (const s8*)"..", 2) == 0) {
            break;
        }
        err = PFENT_ITER_Retreat(&iter, FALSE);
        if (err != 0) {
            return err;
        }
        if (start_cluster == cluster) {
            *p_is_found = TRUE;
            return 0;
        }
        if (start_cluster <= PF_CLUSTER_FIRST) {
            return 0;
        }
        if (iter.index < 1) {
            hint.chain_index++;
            iter.file_sector_index += p_ent->p_vol->bpb.sectors_per_cluster;
            iter.index += (1 << iter.log2_entries_per_sector) << p_ent->p_vol->bpb.log2_sectors_per_cluster;
        }
    }

    start_cluster = (SWAP16(*(u16*)&iter.buf[PF_DIRENT_CLUSTER_HI]) << 16) | (u16)SWAP16(*(u16*)&iter.buf[PF_DIRENT_CLUSTER_LO]);
    hint.chain_index = 0;
    iter.file_sector_index = 0;
    hint.cluster = start_cluster;
    iter.sector = p_ent->p_vol->bpb.first_data_sector + ((start_cluster - PF_CLUSTER_FIRST) << p_ent->p_vol->bpb.log2_sectors_per_cluster);
    iter.offset = PF_DIR_ENTRY_SIZE;
    iter.index = 1;

    while (start_cluster > PF_CLUSTER_FIRST) {
        err = PFENT_ITER_LoadEntry(&iter);
        if (err != 0) {
            return err;
        }
        if (pf_strncmp((const s8*)iter.buf, (const s8*)"..", 2) == 0) {
            if (start_cluster == cluster) {
                *p_is_found = TRUE;
                return 0;
            }
            start_cluster = (SWAP16(*(u16*)&iter.buf[PF_DIRENT_CLUSTER_HI]) << 16) | (u16)SWAP16(*(u16*)&iter.buf[PF_DIRENT_CLUSTER_LO]);
            hint.cluster = start_cluster;
            iter.sector = p_ent->p_vol->bpb.first_data_sector + ((start_cluster - PF_CLUSTER_FIRST) << p_ent->p_vol->bpb.log2_sectors_per_cluster);
        } else {
            return 0;
        }
    }
    return 0;
}

s32 PFENT_ITER_Retreat(struct PF_ENT_ITER* p_iter, u32 may_allocate) {
    u32 before_cluster;
    u32 cur_cluster;
    s32 err;

    if (p_iter->index == 0) {
        return 0;
    }
    if (p_iter->file_sector_index != (p_iter->index - 1) >> p_iter->log2_entries_per_sector) {
        cur_cluster = ((p_iter->sector - p_iter->p_vol->bpb.first_data_sector) >> p_iter->p_vol->bpb.log2_sectors_per_cluster) + PF_CLUSTER_FIRST;
        if ((p_iter->file_sector_index % p_iter->p_vol->bpb.sectors_per_cluster == 0) || p_iter->p_vol->bpb.sectors_per_cluster == 1) {
            if (p_iter->p_vol->bpb.fat_type == FAT_32 || (*p_iter->ffd.p_start_cluster > PF_CLUSTER_ROOT_DIR && p_iter->p_vol->bpb.first_data_sector <= p_iter->sector)) {
                err = PFFAT_getBeforeChain(p_iter->p_vol, cur_cluster, cur_cluster, &before_cluster);
                if (err != 0) {
                    return err;
                }
                if (before_cluster == PF_INVALID_CLUSTER) {
                    return PF_IERR_INVALID_CLUSTER;
                }
            } else {
                before_cluster = *p_iter->ffd.p_start_cluster;
            }
        } else {
            before_cluster = cur_cluster;
        }

        p_iter->index--;
        p_iter->file_sector_index--;

        if (p_iter->p_vol->bpb.fat_type == FAT_32 || (*p_iter->ffd.p_start_cluster > PF_CLUSTER_ROOT_DIR && p_iter->p_vol->bpb.first_data_sector <= p_iter->sector)) {
            p_iter->sector = p_iter->p_vol->bpb.first_data_sector + ((before_cluster - PF_CLUSTER_FIRST) << p_iter->p_vol->bpb.log2_sectors_per_cluster) + (p_iter->file_sector_index & (p_iter->p_vol->bpb.sectors_per_cluster - 1));
        } else {
            p_iter->sector--;
        }

        p_iter->offset = (p_iter->index & p_iter->offset_mask) * PF_DIR_ENTRY_SIZE;
        return 0;
    }

    return PFENT_ITER_MoveTo(p_iter, p_iter->index - 1, may_allocate);
}

s32 PFENT_ITER_FindDirEntryFromCluster(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent, u32 cluster, u32* p_is_found) {
    u8 attr;
    s32 err;
    s32 lengthName;
    s8 filename[PF_SFN_BUF_LEN];

    err = 0;
    *p_is_found = FALSE;
    p_ent->num_entry_LFNs = 0;
    p_ent->ordinal = 0;
    p_ent->check_sum = 0;

    if ((p_ent->attr & PF_ATTR_DIR) != 0 && p_ent->start_cluster == PF_CLUSTER_ROOT_DIR) {
        return PF_IERR_INVALID_PATH;
    }

    for (; !PFENT_ITER_IsAtLogicalEnd(p_iter); err = PFENT_ITER_Advance(p_iter, FALSE)) {
        if (err != 0) {
            return err;
        }

        if (p_iter->buf[0] == PF_ENTRY_DELETED) {
            p_ent->num_entry_LFNs = 0;
            p_ent->long_name[0] = 0;
        } else {
            attr = p_iter->buf[PF_DIRENT_ATTR];
            if ((attr & PF_ATTR_LONG_NAME) == PF_ATTR_LONG_NAME) {
                err = PFENT_LoadLFNEntryFieldsFromBuf(p_ent, p_iter->buf);
                if (err != 0) {
                    p_ent->num_entry_LFNs = 0;
                    p_ent->long_name[0] = 0;
                }
            } else if ((attr & PF_ATTR_VOLUME) != 0) {
                p_ent->num_entry_LFNs = 0;
                p_ent->long_name[0] = 0;
            } else if ((attr & PF_ATTR_DIR) == 0) {
                p_ent->num_entry_LFNs = 0;
                p_ent->long_name[0] = 0;
            } else {
                PFENT_LoadShortNameFromBuf(p_ent, p_iter->buf);

                if (p_ent->num_entry_LFNs != 0 && p_ent->ordinal == PF_LFN_FIRST_ORDINAL && p_ent->check_sum == PFENT_CalcCheckSum(p_ent)) {
                    PFENT_loadEntryNumericFieldsFromBuf(p_ent, p_iter->buf);
                    p_ent->entry_sector = p_iter->sector;
                    p_ent->entry_offset = p_iter->offset;
                    p_ent->p_vol = p_iter->ffd.p_vol;

                    if ((p_ent->attr & PF_ATTR_DIR) != 0 && p_ent->start_cluster != cluster) {
                        p_ent->num_entry_LFNs = 0;
                        p_ent->long_name[0] = 0;
                        continue;
                    }
                    if ((p_ent->attr & PF_ATTR_DIR) != 0 && p_ent->start_cluster == PF_CLUSTER_DOTDOT_ROOT) {
                        p_ent->start_cluster = PF_CLUSTER_ROOT_DIR;
                    }
                    *p_is_found = TRUE;
                }

                if ((p_iter->buf[PF_DIRENT_SMALL_LETTER_FLAG] & PF_SMALL_LETTER_MASK) != 0) {
                    PFPATH_getLongNameformShortName(p_ent->short_name, filename, p_iter->buf[PF_DIRENT_SMALL_LETTER_FLAG]);
                    lengthName = PFPATH_transformInUnicode(p_ent->long_name, filename);
                    p_ent->num_entry_LFNs = (lengthName / PF_LFN_CHARS_PER_ENTRY) + ((lengthName % PF_LFN_CHARS_PER_ENTRY) != 0);
                    p_ent->check_sum = PFENT_CalcCheckSum(p_ent);
                    p_ent->ordinal = PF_LFN_FIRST_ORDINAL;
                }

                PFENT_loadEntryNumericFieldsFromBuf(p_ent, p_iter->buf);
                p_ent->entry_sector = p_iter->sector;
                p_ent->entry_offset = p_iter->offset;
                p_ent->p_vol = p_iter->ffd.p_vol;

                if ((p_ent->attr & PF_ATTR_DIR) != 0 && p_ent->start_cluster != cluster) {
                    p_ent->num_entry_LFNs = 0;
                    p_ent->long_name[0] = 0;
                    *p_is_found = FALSE;
                    continue;
                }
                if ((p_ent->attr & PF_ATTR_DIR) != 0 && p_ent->start_cluster == PF_CLUSTER_DOTDOT_ROOT) {
                    p_ent->start_cluster = PF_CLUSTER_ROOT_DIR;
                }
                *p_is_found = TRUE;
                return 0;
            }
        }
    }

    return 0;
}

s32 PFENT_ITER_FindEntry(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent, struct PF_STR* p_pattern, u8 attr_required, u8 attr_unwanted, u32* p_is_found) {
    s32 err;

    *p_is_found = FALSE;
    if ((attr_required & attr_unwanted) != 0) {
        return PF_IERR_INVALID_PARAM;
    }

    if (PFSTR_StrNCmp(p_pattern, (const s8*)":", PF_STR_TARGET_HEAD, PF_DRIVE_LETTER_LEN, 1) == 0) {
        PFSTR_MoveStrPos(p_pattern, PF_DRIVE_SPEC_LEN);
    }

    if (PFSTR_StrNCmp(p_pattern, (const s8*)"\\", PF_STR_TARGET_HEAD, 0, 1) == 0 || PFSTR_StrNCmp(p_pattern, (const s8*)"/", PF_STR_TARGET_HEAD, 0, 1) == 0) {
        err = PFENT_GetRootDir(p_iter->p_vol, p_ent);
        if (err != 0) {
            return err;
        }
        if (PFSTR_StrNumChar(p_pattern, PF_STR_TARGET_HEAD) == 1 && PFSTR_StrNCmp(p_pattern, (const s8*)"\0\0", PF_STR_TARGET_TAIL, 0, 1) == 0) {
            return 0;
        }
    } else {
        err = PFVOL_GetCurrentDir(p_iter->p_vol, p_ent);
        if (err != 0) {
            return err;
        }
    }

    err = PFENT_ITER_DoFindEntry(p_iter, p_ent, p_pattern, attr_required, attr_unwanted, p_is_found);
    if (err != 0) {
        return err;
    }
    return 0;
}

s32 PFENT_ITER_FindEntryNoPathCheck(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent, struct PF_STR* p_pattern, u8 attr_required, u8 attr_unwanted, u32* p_is_found) {
    s32 err;

    *p_is_found = FALSE;
    if ((attr_required & attr_unwanted) != 0) {
        return PF_IERR_INVALID_PARAM;
    }

    err = PFENT_ITER_DoFindEntry(p_iter, p_ent, p_pattern, attr_required, attr_unwanted, p_is_found);
    if (err != 0) {
        return err;
    }
    return 0;
}

s32 PFENT_ITER_GetEntryOfPath(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent, struct PF_VOLUME* p_vol, struct PF_STR* p_path, u32 no_look_last_token) {
    return PFENT_ITER_DoGetEntry(p_iter, p_ent, p_vol, p_path, FALSE, no_look_last_token);
}

s32 PFENT_ITER_GetEntryOfPattern(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent, struct PF_VOLUME* p_vol, struct PF_STR* p_path) {
    return PFENT_ITER_DoGetEntry(p_iter, p_ent, p_vol, p_path, TRUE, TRUE);
}

s32 PFENT_ITER_GetEntryOfIter(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent) {
    PFENT_LoadShortNameFromBuf(p_ent, p_iter->buf);
    PFENT_loadEntryNumericFieldsFromBuf(p_ent, p_iter->buf);
    p_ent->entry_sector = p_iter->sector;
    p_ent->entry_offset = p_iter->offset;
    p_ent->p_vol = p_iter->p_vol;
    return 0;
}

s32 PFENT_ITER_GetLFNEntryName(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent) {
    s32 err;
    u8* p;
    u32 i;

    if (p_ent->num_entry_LFNs != 0 && (p_ent->small_letter_flag & PF_SMALL_LETTER_MASK) == 0) {
        for (i = 0; i < p_ent->num_entry_LFNs; i++) {
            err = PFENT_ITER_Retreat(p_iter, FALSE);
            if (err != 0) {
                return err;
            }
            err = PFENT_ITER_LoadEntry(p_iter);
            if (err != 0) {
                return err;
            }
            p = (u8*)p_ent->long_name + (u32)(i * PF_LFNENT_NAME_SIZE);
            pf_memcpy(p, &p_iter->buf[PF_LFNENT_NAME1], PF_LFNENT_NAME1_SIZE);
            pf_memcpy(p + PF_LFNENT_NAME1_SIZE, &p_iter->buf[PF_LFNENT_NAME2], PF_LFNENT_NAME2_SIZE);
            pf_memcpy(p + PF_LFNENT_NAME1_SIZE + PF_LFNENT_NAME2_SIZE, &p_iter->buf[PF_LFNENT_NAME3], PF_LFNENT_NAME3_SIZE);
        }
        p_ent->long_name[p_ent->num_entry_LFNs * PF_LFNENT_NAME_SIZE / sizeof(u16)] = 0;
    }
    return 0;
}
