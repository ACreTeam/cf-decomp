#include <prfile2/pf_cache.h>
#include <prfile2/pf_clib.h>
#include <prfile2/pf_cluster.h>
#include <prfile2/pf_driver.h>
#include <prfile2/pf_entry.h>
#include <prfile2/pf_fat.h>
#include <prfile2/pf_path.h>
#include <prfile2/pf_sector.h>
#include <prfile2/pf_service.h>
#include <prfile2/pf_api.h>
#include <prfile2/pf_system.h>
#include <prfile2/pf_volume.h>

#define PF_TAIL_WORD_FULL 0xFFFFFFFF

enum {
    PF_SFN_TAIL_MIN = 1,
    PF_SFN_TAIL_MAX = 999999,
    PF_TAIL_BITS_PER_WORD = 32,
    PF_TAIL_BITS_PER_WORD_LOG2 = 5,
};

s32 PFENT_compareEntry(struct PF_DIR_ENT* p_ent, const u8* buf, struct PF_STR* p_pattern, u8 attr, u8 attr_required, u8 attr_unwanted, u32* logical_index) {
    s32 is_match;
    u32 is_valid;

    is_match = 1;
    is_valid = TRUE;
    if ((attr & PF_ATTR_LONG_NAME) == PF_ATTR_LONG_NAME) {
        is_match = 1;
    } else {
        if (attr == PF_ATTR_NONE) {
            attr = PF_ATTR_NORMAL;
        }
        if ((attr_required & PF_ATTR_STRICT) != 0) {
            attr_required &= PF_ATTR_ALL;
            attr_unwanted &= PF_ATTR_ALL;
            if ((attr_required != 0 && attr_required != (attr_required & attr)) || (attr_unwanted != 0 && attr_unwanted == (attr_unwanted & attr))) {
                is_valid = FALSE;
            }
        } else if (attr_required != PF_ATTR_ALL && attr != attr_required && ((attr & attr_required) == 0 || (attr & attr_unwanted) != 0)) {
            is_valid = FALSE;
        }

        if (!is_valid) {
            is_match = 1;
        } else {
            if ((attr & PF_ATTR_VOLUME) != 0) {
                p_ent->num_entry_LFNs = 0;
                p_ent->long_name[0] = 0;
            }

            PFENT_LoadShortNameFromBuf(p_ent, buf);

            if (p_ent->num_entry_LFNs != 0 && p_ent->ordinal == PF_LFN_FIRST_ORDINAL && p_ent->check_sum == PFENT_CalcCheckSum(p_ent)) {
                is_match = PFPATH_cmpNameUni(p_ent->long_name, p_pattern) != 0;
            }

            if (is_match == 1) {
                if ((pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) == PF_ENCODE_MODE_UNICODE && (attr & PF_ATTR_VOLUME) == 0) {
                    if (PFPATH_cmpName(p_ent->short_name, p_pattern, FALSE) == 0) {
                        is_match = 0;
                    }
                } else if (PFPATH_cmpName(p_ent->short_name, p_pattern, TRUE) == 0) {
                    is_match = 0;
                }
            }

            if (is_match == 1) {
                (*logical_index)++;
            }
        }
    }
    return is_match;
}

static s32 PFENT_getEntry(struct PF_DIR_ENT* p_ent, struct PF_ENT_ITER* p_iter, struct PF_STR* p_pattern, u8 attr_required, u8 attr_unwanted, u32* logical_index) {
    s32 lengthName;
    u8 attr;
    s8 filename[PF_SFN_BUF_LEN];

    attr = p_iter->buf[PF_DIRENT_ATTR];
    if (PFENT_compareEntry(p_ent, p_iter->buf, p_pattern, attr, attr_required, attr_unwanted, logical_index) == 1) {
        if ((attr & PF_ATTR_LONG_NAME) == PF_ATTR_LONG_NAME) {
            if (PFENT_LoadLFNEntryFieldsFromBuf(p_ent, p_iter->buf) != 0) {
                p_ent->num_entry_LFNs = 0;
                p_ent->long_name[0] = 0;
            }
        } else {
            p_ent->num_entry_LFNs = 0;
            p_ent->long_name[0] = 0;
        }
        return PFENT_NO_MATCH;
    }

    if (p_ent->num_entry_LFNs == 0 && (p_iter->buf[PF_DIRENT_SMALL_LETTER_FLAG] & PF_SMALL_LETTER_MASK) != 0) {
        PFPATH_getLongNameformShortName(p_ent->short_name, filename, p_iter->buf[PF_DIRENT_SMALL_LETTER_FLAG]);
        lengthName = PFPATH_transformInUnicode(p_ent->long_name, filename);

        p_ent->num_entry_LFNs = (lengthName / PF_LFN_CHARS_PER_ENTRY) + (lengthName % PF_LFN_CHARS_PER_ENTRY != 0);

        p_ent->check_sum = PFENT_CalcCheckSum(p_ent);
        p_ent->ordinal = PF_LFN_FIRST_ORDINAL;
    }

    PFENT_loadEntryNumericFieldsFromBuf(p_ent, p_iter->buf);

    p_ent->entry_sector = p_iter->sector;
    p_ent->entry_offset = p_iter->offset;

    if ((p_ent->attr & PF_ATTR_DIR) != 0 && p_ent->start_cluster == PF_CLUSTER_DOTDOT_ROOT) {
        p_ent->start_cluster = PF_CLUSTER_ROOT_DIR;
    }
    return 0;
}

static s32 PFENT_searchEmptyTailSFN(struct PF_FFD* p_ffd, u32 tail_index, const s8* pattern, u32* p_tail_bit) {
    struct PF_ENT_ITER iter;
    u8 attr;
    s32 err;
    u32 bit_pos;
    u32 sfn_taillen;
    u32 sfn_baselen;
    u32 i;
    s8 sfnbuf[PF_SFN_BUF_LEN];
    s8 patbuf[PF_SFN_BUF_LEN];
    struct PF_VOLUME* p_vol;

    p_vol = p_ffd->p_vol;
    pf_memset(p_tail_bit, 0, sizeof(u32) * p_vol->tail_entry.tracker_size);
    iter.ffd = *p_ffd;
    for (err = PFENT_ITER_IteratorInitialize(&iter, 0); !PFENT_ITER_IsAtLogicalEnd(&iter); err = PFENT_ITER_Advance(&iter, FALSE)) {
        if (err) {
            return err;
        }
        if (iter.buf[0] == PF_ENTRY_END) {
            break;
        }
        if (iter.buf[0] != PF_ENTRY_DELETED && (iter.buf[PF_DIRENT_ATTR] & PF_ATTR_LONG_NAME) != PF_ATTR_LONG_NAME && (iter.buf[PF_DIRENT_ATTR] & PF_ATTR_VOLUME) == 0) {
            PFPATH_getShortName(sfnbuf, iter.buf, PF_ATTR_NONE);
            for (sfn_taillen = 1; sfnbuf[sfn_taillen] != '~' && sfnbuf[sfn_taillen] != '\0' && sfn_taillen < PF_SFN_BASE_LEN - 1; ++sfn_taillen) {
            }
            if (sfn_taillen < PF_SFN_BASE_LEN - 1 && sfnbuf[sfn_taillen] == '~') {
                for (sfn_baselen = sfn_taillen + 1; sfnbuf[sfn_baselen] >= '0' && sfnbuf[sfn_baselen] <= '9'; ++sfn_baselen) {
                }
                if (sfnbuf[sfn_baselen] == '.' || sfnbuf[sfn_baselen] == '\0') {
                    bit_pos = 0;
                    for (i = sfn_baselen - sfn_taillen - 1; i != 0; i--) {
                        bit_pos = bit_pos * 10;
                        bit_pos += sfnbuf[sfn_baselen - i] - '0';
                    }
                    pf_strcpy(patbuf, pattern);
                    PFPATH_parseShortNameNumeric(patbuf, bit_pos);
                    if (PFPATH_cmpTailSFN(sfnbuf, patbuf) == 0 && bit_pos >= tail_index && bit_pos < PF_TAIL_BITS_PER_WORD * p_vol->tail_entry.tracker_size + tail_index) {
                        bit_pos -= tail_index;
                        p_tail_bit[bit_pos >> PF_TAIL_BITS_PER_WORD_LOG2] |= 1u << bit_pos;
                    }
                }
            }
        }
    }
    return 0;
}

static s32 PFENT_findEmptyTailSFN(struct PF_DIR_ENT* p_ent_containig_dir, const s8* name, u32* p_tails) {
    struct PF_FFD ffd;
    struct PF_FAT_HINT hint;
    s32 err;
    u32 num;
    struct PF_VOLUME* p_vol;
    u32 track_num;

    p_vol = p_ent_containig_dir->p_vol;
    *p_tails = PF_SFN_TAIL_MIN;
    PFFAT_InitFFD(&ffd, &hint, p_ent_containig_dir->p_vol, &p_ent_containig_dir->start_cluster);
    for (num = PF_SFN_TAIL_MIN; num <= PF_SFN_TAIL_MAX; num += PF_TAIL_BITS_PER_WORD * p_vol->tail_entry.tracker_size) {
        err = PFENT_searchEmptyTailSFN(&ffd, num, name, p_vol->tail_entry.tracker_bits);
        if (err) {
            return err;
        }
        for (track_num = 0; track_num < p_vol->tail_entry.tracker_size; ++track_num) {
            if (p_vol->tail_entry.tracker_bits[track_num] != PF_TAIL_WORD_FULL) {
                while ((p_vol->tail_entry.tracker_bits[track_num] & 1) != 0) {
                    p_vol->tail_entry.tracker_bits[track_num] >>= 1;
                    ++*p_tails;
                }
                num = PF_SFN_TAIL_MAX + 1;
                break;
            }
            *p_tails += PF_TAIL_BITS_PER_WORD;
        }
    }
    return 0;
}

u8 PFENT_CalcCheckSum(struct PF_DIR_ENT* p_ent) {
    u16 i;
    u8 sum;
    u8 buf[PF_SFN_BUF_LEN];

    PFPATH_putShortName(buf, p_ent->short_name, PF_ATTR_NONE);
    sum = 0;
    for (i = 0; i < PF_SFN_LEN; i++) {
        sum = (sum >> 1) + (((sum & 1) != 0) ? 0x80 : 0) + buf[i];
    }
    return sum;
}

void PFENT_LoadShortNameFromBuf(struct PF_DIR_ENT* p_ent, const u8* buf) {
    PFPATH_getShortName(p_ent->short_name, buf, buf[PF_DIRENT_ATTR]);
    if (p_ent->short_name[0] == PF_ENTRY_E5_ESCAPE) {
        p_ent->short_name[0] = PF_ENTRY_DELETED;
    }
}

void PFENT_loadEntryNumericFieldsFromBuf(struct PF_DIR_ENT* p_ent, const u8* buf) {
    p_ent->attr = buf[PF_DIRENT_ATTR];
    p_ent->small_letter_flag = buf[PF_DIRENT_SMALL_LETTER_FLAG];
    p_ent->create_time_ms = buf[PF_DIRENT_CREATE_TIME_MS];

    p_ent->create_time = SWAP16(*(u16*)&buf[PF_DIRENT_CREATE_TIME]);
    p_ent->create_date = SWAP16(*(u16*)&buf[PF_DIRENT_CREATE_DATE]);
    p_ent->access_date = SWAP16(*(u16*)&buf[PF_DIRENT_ACCESS_DATE]);
    p_ent->modify_time = SWAP16(*(u16*)&buf[PF_DIRENT_MODIFY_TIME]);
    p_ent->modify_date = SWAP16(*(u16*)&buf[PF_DIRENT_MODIFY_DATE]);
    p_ent->file_size = SWAP32(*(u32*)&buf[PF_DIRENT_FILE_SIZE]);

    p_ent->start_cluster = (SWAP16(*(u16*)&buf[PF_DIRENT_CLUSTER_HI]) << 16) | (u16)SWAP16(*(u16*)&buf[PF_DIRENT_CLUSTER_LO]);
}

void PFENT_StoreEntryNumericFieldsToBuf(u8* buf, const struct PF_DIR_ENT* p_ent) {
    buf[PF_DIRENT_ATTR] = p_ent->attr;
    buf[PF_DIRENT_SMALL_LETTER_FLAG] = p_ent->small_letter_flag;
    buf[PF_DIRENT_CREATE_TIME_MS] = p_ent->create_time_ms;
    *(u16*)&buf[PF_DIRENT_CREATE_TIME] = SWAP16(p_ent->create_time);
    *(u16*)&buf[PF_DIRENT_CREATE_DATE] = SWAP16(p_ent->create_date);
    *(u16*)&buf[PF_DIRENT_ACCESS_DATE] = SWAP16(p_ent->access_date);
    *(u16*)&buf[PF_DIRENT_MODIFY_TIME] = SWAP16(p_ent->modify_time);
    *(u16*)&buf[PF_DIRENT_MODIFY_DATE] = SWAP16(p_ent->modify_date);
    *(u16*)&buf[PF_DIRENT_CLUSTER_HI] = SWAP16((u16)(p_ent->start_cluster >> 16));
    *(u16*)&buf[PF_DIRENT_CLUSTER_LO] = SWAP16((u16)(p_ent->start_cluster));
    *(u32*)&buf[PF_DIRENT_FILE_SIZE] = SWAP32(p_ent->file_size);
}

s32 PFENT_LoadLFNEntryFieldsFromBuf(struct PF_DIR_ENT* p_ent, const u8* buf) {
    u8 ordinal;
    u8 check_sum;
    u32 is_last;
    u8* p;
    u16* q;
    u16* q_after;

    ordinal = buf[0];
    check_sum = buf[PF_LFNENT_CHECK_SUM];

    if ((u8)(ordinal & ~PF_LFN_LAST_ENTRY) > PF_LFN_MAX_ORDINAL) {
        p_ent->ordinal = 0;
        p_ent->check_sum = 0;
        p_ent->num_entry_LFNs = 0;
        return PF_IERR_BROKEN_LFN;
    }

    if (ordinal & PF_LFN_LAST_ENTRY) {
        ordinal &= ~PF_LFN_LAST_ENTRY;
        p_ent->num_entry_LFNs = 0;
        is_last = TRUE;
    } else {
        is_last = FALSE;
        if (p_ent->num_entry_LFNs == 0) {
            return PF_IERR_BROKEN_LFN;
        }

        if (ordinal != p_ent->ordinal - 1 || check_sum != p_ent->check_sum) {
            p_ent->ordinal = 0;
            p_ent->check_sum = 0;
            p_ent->num_entry_LFNs = 0;
            return PF_IERR_BROKEN_LFN;
        }
    }

    p_ent->ordinal = ordinal;
    p_ent->check_sum = check_sum;

    p = (u8*)p_ent->long_name + (ordinal - 1) * PF_LFNENT_NAME_SIZE;

    pf_memcpy(p, buf + PF_LFNENT_NAME1, PF_LFNENT_NAME1_SIZE);
    pf_memcpy(p + PF_LFNENT_NAME1_SIZE, buf + PF_LFNENT_NAME2, PF_LFNENT_NAME2_SIZE);
    pf_memcpy(p + PF_LFNENT_NAME1_SIZE + PF_LFNENT_NAME2_SIZE, buf + PF_LFNENT_NAME3, PF_LFNENT_NAME3_SIZE);

    PF_LE16_TO_U16_STR(p, PF_LFNENT_NAME1_SIZE);
    PF_LE16_TO_U16_STR(p + PF_LFNENT_NAME1_SIZE, PF_LFNENT_NAME2_SIZE);
    PF_LE16_TO_U16_STR(p + PF_LFNENT_NAME1_SIZE + PF_LFNENT_NAME2_SIZE, PF_LFNENT_NAME3_SIZE);

    if (is_last) {
        *(u16*)(p + PF_LFNENT_NAME_SIZE) = 0;
        q_after = (u16*)(p + PF_LFNENT_NAME_SIZE);

        for (q = (u16*)p; q < q_after; q++) {
            if (*q == 0) {
                q++;
                break;
            }
        }

        while (q < q_after) {
            if (*q != PF_LFN_PAD_CHAR) {
                p_ent->num_entry_LFNs = 0;
                return PF_IERR_BROKEN_LFN;
            }
            q++;
        }
    }

    p_ent->num_entry_LFNs++;
    return 0;
}

void PFENT_storeLFNEntryFieldsToBuf(u8* buf, struct PF_DIR_ENT* p_ent, u8 ord, u8 sum, u32 is_last) {
    u8* p_seg;
    u16* p;
    u16* p_after;

    if (is_last) {
        buf[0] = ord | PF_LFN_LAST_ENTRY;
    } else {
        buf[0] = ord;
    }

    buf[PF_DIRENT_ATTR] = PF_ATTR_LONG_NAME;
    buf[PF_LFNENT_CHECK_SUM] = sum;
    buf[PF_DIRENT_SMALL_LETTER_FLAG] = 0;
    *(u16*)&buf[PF_DIRENT_CLUSTER_LO] = 0;

    p_seg = (u8*)p_ent->long_name + (ord - 1) * PF_LFNENT_NAME_SIZE;

    if (is_last) {
        p_after = (u16*)(p_seg + PF_LFNENT_NAME_SIZE);

        for (p = (u16*)p_seg; p < p_after; p++) {
            if (*p == 0) {
                p++;
                break;
            }
        }

        while (p < p_after) {
            *p = PF_LFN_PAD_CHAR;
            p++;
        }
    }

    pf_memcpy(buf + PF_LFNENT_NAME1, p_seg, PF_LFNENT_NAME1_SIZE);
    pf_memcpy(buf + PF_LFNENT_NAME2, p_seg + PF_LFNENT_NAME1_SIZE, PF_LFNENT_NAME2_SIZE);
    pf_memcpy(buf + PF_LFNENT_NAME3, p_seg + PF_LFNENT_NAME1_SIZE + PF_LFNENT_NAME2_SIZE, PF_LFNENT_NAME3_SIZE);

    PF_LE16_TO_U16_STR(buf + PF_LFNENT_NAME1, PF_LFNENT_NAME1_SIZE);
    PF_LE16_TO_U16_STR(buf + PF_LFNENT_NAME2, PF_LFNENT_NAME2_SIZE);
    PF_LE16_TO_U16_STR(buf + PF_LFNENT_NAME3, PF_LFNENT_NAME3_SIZE);
}

s32 PFENT_findEntryPos(struct PF_FFD* p_ffd, struct PF_DIR_ENT* p_ent, u32 index_search_from, struct PF_STR* p_pattern, u8 attr_required, u8 attr_unwanted, u32* p_lpos, u32* p_ppos) {
    s32 err;
    struct PF_ENT_ITER iter;
    u32 logical_index;
    u32 is_extsfn;

    is_extsfn = FALSE;

    *p_lpos = 0;
    *p_ppos = 0;

    p_ent->num_entry_LFNs = 0;
    p_ent->ordinal = 0;
    p_ent->check_sum = 0;
    p_ent->long_name[0] = 0;

    logical_index = 0;

    if ((pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) == PF_ENCODE_MODE_UNICODE) {
        is_extsfn = PFPATH_GetExtShortNameIndex(p_pattern, &index_search_from);
    }

    iter.ffd = *p_ffd;

    if (is_extsfn == TRUE) {
        err = PFENT_ITER_IteratorInitialize(&iter, index_search_from - 1);
        if (err != 0) {
            *p_lpos = PF_ENTRY_INDEX_END;
            *p_ppos = PF_ENTRY_INDEX_END;
            return err;
        }

        err = PFENT_getEntry(p_ent, &iter, p_pattern, attr_required, attr_unwanted, &logical_index);

        if (err == 0) {
            *p_lpos = PF_ENTRY_INDEX_END;
            *p_ppos = PF_ENTRY_INDEX_END;
            return PF_IERR_NOT_FOUND;
        }

        if ((iter.buf[0] & PF_LFN_LAST_ENTRY) != 0) {
            err = PFENT_ITER_IteratorInitialize(&iter, index_search_from);
            if (err != 0) {
                *p_lpos = PF_ENTRY_INDEX_END;
                *p_ppos = PF_ENTRY_INDEX_END;
                return err;
            }

            err = PFENT_getEntry(p_ent, &iter, p_pattern, attr_required, attr_unwanted, &logical_index);

            if (err == 0) {
                p_ent->p_vol = p_ffd->p_vol;
                *p_lpos = logical_index;
                *p_ppos = iter.index;
                return 0;
            }
        }
    } else {
        for (err = PFENT_ITER_IteratorInitialize(&iter, index_search_from); !PFENT_ITER_IsAtLogicalEnd(&iter); err = PFENT_ITER_Advance(&iter, FALSE)) {
            if (err != 0) {
                *p_lpos = PF_ENTRY_INDEX_END;
                *p_ppos = PF_ENTRY_INDEX_END;
                return err;
            }
            if (iter.buf[0] == PF_ENTRY_END) {
                break;
            }
            if (iter.buf[0] == PF_ENTRY_DELETED) {
                p_ent->num_entry_LFNs = 0;
                p_ent->long_name[0] = 0;
            } else {
                err = PFENT_getEntry(p_ent, &iter, p_pattern, attr_required, attr_unwanted, &logical_index);

                if (err == 0) {
                    p_ent->p_vol = p_ffd->p_vol;
                    *p_lpos = logical_index;
                    *p_ppos = iter.index;
                    return 0;
                }
            }
        }
    }

    *p_lpos = PF_ENTRY_INDEX_END;
    *p_ppos = PF_ENTRY_INDEX_END;
    return PF_IERR_NOT_FOUND;
}

s32 PFENT_findEntry(struct PF_FFD* p_ffd, struct PF_DIR_ENT* p_ent, u32 index_search_from, struct PF_STR* p_pattern, u8 attr_required, u8 attr_unwanted) {
    u32 logical_pos;
    u32 physical_pos;

    return PFENT_findEntryPos(p_ffd, p_ent, index_search_from, p_pattern, attr_required, attr_unwanted, &logical_pos, &physical_pos);
}

s32 PFENT_allocateEntryPos(struct PF_DIR_ENT* p_ent, u8 num_entries, struct PF_FFD* p_ffd, u32* p_next_chain, struct PF_STR* p_filename, u32* p_pos) {
    s32 err;
    struct PF_VOLUME* p_vol;
    struct PF_ENT_ITER iter;
    struct PF_DIR_ENT wk_ent;
    u32 wk_sector;
    u16 wk_offset;
    u32 prev_sector;
    u32 next_sector1;
    u32 next_sector2;
    u32 is_found;
    u32 sector;
    u32 count_unused_entries;
    u32 physical_index;
    u32 dummy_index;

    if ((pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) == PF_ENCODE_MODE_UNICODE) {
        if (PFPATH_CheckExtShortNameSignature(p_filename) == TRUE) {
            return PF_IERR_INVALID_NAME;
        }
    }

    p_vol = p_ffd->p_vol;
    count_unused_entries = 0;
    physical_index = 0;

    wk_sector = PF_INVALID_SECTOR;
    wk_offset = 0;
    p_next_chain[1] = PF_INVALID_SECTOR;
    p_next_chain[0] = PF_INVALID_SECTOR;
    prev_sector = PF_INVALID_SECTOR;
    next_sector2 = PF_INVALID_SECTOR;
    next_sector1 = PF_INVALID_SECTOR;
    is_found = FALSE;

    iter.sector = 0;
    iter.index = 0;
    iter.ffd = *p_ffd;
    err = PFENT_ITER_IteratorInitialize(&iter, 0);
    while (1) {
        if (err != 0) {
            if (err == PF_IERR_OUT_OF_RANGE) {
                break;
            }
            return err;
        }

        if (iter.sector == PF_INVALID_SECTOR) {
            break;
        }
        if (!is_found && count_unused_entries == 0) {
            wk_offset = iter.offset;
            wk_sector = iter.sector;
            prev_sector = iter.sector;
        }
        if (!is_found) {
            if (iter.buf[0] == PF_ENTRY_END || iter.buf[0] == PF_ENTRY_DELETED) {
                if (prev_sector != iter.sector) {
                    if (next_sector1 == PF_INVALID_SECTOR) {
                        next_sector1 = iter.sector;
                    } else {
                        next_sector2 = iter.sector;
                    }
                    prev_sector = iter.sector;
                }
                count_unused_entries++;
            } else {
                count_unused_entries = 0;
                next_sector1 = PF_INVALID_SECTOR;
                next_sector2 = PF_INVALID_SECTOR;
            }
        }
        if (count_unused_entries >= num_entries) {
            if (!is_found) {
                physical_index = iter.index;
                is_found = TRUE;
            }
            if (iter.buf[0] == PF_ENTRY_END) {
                break;
            }
        }
        if (iter.buf[0] != PF_ENTRY_END && iter.buf[0] != PF_ENTRY_DELETED) {
            err = PFENT_getEntry(&wk_ent, &iter, p_filename, PF_ATTR_ALL, PF_ATTR_NONE, &dummy_index);
            if (err == 0) {
                *p_ent = wk_ent;
                p_ent->p_vol = p_ffd->p_vol;
                return PF_IERR_EXISTS;
            }
        }
        if (iter.offset + PF_DIR_ENTRY_SIZE == p_vol->bpb.bytes_per_sector) {
            err = PFFAT_GetSectorSpecified(p_ffd, iter.file_sector_index + 1, FALSE, &sector);
            if (err != 0) {
                return err;
            }
            if (sector == PF_INVALID_SECTOR && is_found == TRUE) {
                break;
            }
        }
        err = PFENT_ITER_Advance(&iter, TRUE);
    }

    if (!is_found) {
        return PF_IERR_DIR_FULL;
    }

    p_next_chain[0] = next_sector1;
    p_next_chain[1] = next_sector2;
    p_ent->entry_sector = wk_sector;
    p_ent->entry_offset = wk_offset;
    *p_pos = physical_index;
    return 0;
}

s32 PFENT_allocateEntry(struct PF_DIR_ENT* p_ent, u8 num_entries, struct PF_FFD* p_ffd, u32* p_next_chain, struct PF_STR* p_filename) {
    u32 position;

    return PFENT_allocateEntryPos(p_ent, num_entries, p_ffd, p_next_chain, p_filename, &position);
}

s32 PFENT_GetRootDir(struct PF_VOLUME* p_vol, struct PF_DIR_ENT* p_ent) {
    p_ent->long_name[0] = '\\';
    p_ent->long_name[1] = 0;

    p_ent->num_entry_LFNs = 0;
    p_ent->ordinal = 0;
    p_ent->check_sum = 0;

    p_ent->short_name[0] = '\\';
    p_ent->short_name[1] = 0;
    p_ent->small_letter_flag = 0;

    p_ent->attr = PF_ATTR_DIR;

    p_ent->create_time_ms = 0;
    p_ent->create_time = 0;
    p_ent->create_date = 0;
    p_ent->access_date = 0;
    p_ent->modify_time = 0;
    p_ent->modify_date = 0;
    p_ent->file_size = 0;

    p_ent->p_vol = p_vol;
    p_ent->path_len = PFENT_ROOT_PATH_LEN;
    p_ent->start_cluster = PF_CLUSTER_ROOT_DIR;
    p_ent->entry_sector = PF_INVALID_SECTOR;
    p_ent->entry_offset = 0;

    return 0;
}

s32 PFENT_MakeRootDir(struct PF_VOLUME* p_vol) {
    struct PF_CACHE_PAGE* p_page;
    u32 sector;
    u32 success_size;
    u32 is_hit;
    s32 err;

    switch (p_vol->bpb.fat_type) {
        case FAT_32:
            PFFAT_MakeRootDir(p_vol);
            break;
        case FAT_12:
        case FAT_16:
            err = PFCACHE_AllocateDataPage(p_vol, PF_INVALID_SECTOR, &p_page, &is_hit);
            if (err != 0) {
                return err;
            }

            pf_memset(p_page->p_buf, 0, p_vol->bpb.bytes_per_sector);

            for (sector = p_vol->bpb.first_root_dir_sector; sector < p_vol->bpb.first_data_sector; sector++) {
                err = PFSEC_WriteData(p_vol, p_page->p_buf, sector, 0, p_vol->bpb.bytes_per_sector, &success_size, FALSE);
                if (err != 0) {
                    PFCACHE_FreeDataPage(p_vol, p_page);
                    return err;
                }

                if (success_size != p_vol->bpb.bytes_per_sector) {
                    PFCACHE_FreeDataPage(p_vol, p_page);
                    return PF_IERR_IO;
                }
            }

            PFCACHE_FreeDataPage(p_vol, p_page);
            break;
        default:
            return PF_IERR_INVALID_FORMAT;
    }

    PFDRV_LoadVolumeLabelFromBuf(p_vol, (const u8*)PDM_BPB_VOL_LABEL_NO_NAME);
    return 0;
}

s32 PFENT_updateEntry(struct PF_DIR_ENT* p_ent, u32 flag) {
    struct PF_VOLUME* p_vol;
    u32 success_size;
    u8 buf[PF_DIR_ENTRY_SIZE];
    s32 err;

    p_vol = p_ent->p_vol;

    if (p_ent->start_cluster == PF_CLUSTER_ROOT_DIR) {
        return PF_IERR_INVALID_CLUSTER;
    }

    if (flag == PFENT_UPDATE_SET_ARCHIVE) {
        p_ent->attr |= PF_ATTR_ARCHIVE;
    }

    PFPATH_putShortName(buf, p_ent->short_name, p_ent->attr);

    if (buf[0] == PF_ENTRY_DELETED) {
        buf[0] = PF_ENTRY_E5_ESCAPE;
    }

    PFENT_StoreEntryNumericFieldsToBuf(buf, p_ent);

    err = PFSEC_WriteData(p_vol, buf, p_ent->entry_sector, p_ent->entry_offset, PF_DIR_ENTRY_SIZE, &success_size, FALSE);

    if (err != 0) {
        return err;
    }

    return (success_size != PF_DIR_ENTRY_SIZE) ? PF_IERR_IO : 0;
}

s32 PFENT_AdjustSFN(struct PF_DIR_ENT* p_ent, s8* p_short_name) {
    u32 i;
    u32 tail_num;
    s32 err;

    for (i = 1; p_short_name[i] != '~' && p_short_name[i] != '\0' && i < PF_SFN_BASE_LEN - 1; ++i) {
    }

    if (i < PF_SFN_BASE_LEN - 1 && p_short_name[i] == '~') {
        for (i = i + 1; p_short_name[i] >= '0' && p_short_name[i] <= '9'; ++i) {
        }

        if (p_short_name[i] == '.' || p_short_name[i] == '\0') {
            err = PFENT_findEmptyTailSFN(p_ent, p_short_name, &tail_num);
            if (err != 0) {
                return err;
            }

            if (tail_num != PF_SFN_TAIL_MIN) {
                PFPATH_parseShortNameNumeric(p_short_name, tail_num);
            }
        }
    }
    return 0;
}

u8 PFENT_getcurrentDateTimeForEnt(u16* p_date, u16* p_time) {
    struct PF_SYS_DATE sys_date;
    struct PF_SYS_TIME sys_time;

    PFSYS_TimeStamp(&sys_date, &sys_time);

    *p_date = ((sys_date.sys_day & PF_FAT_DATE_DAY_MASK) << 0) | ((sys_date.sys_month & PF_FAT_DATE_MONTH_MASK) << PF_FAT_DATE_MONTH_SHIFT) |
              (((sys_date.sys_year - PF_FAT_DATE_YEAR_BASE) & PF_FAT_DATE_YEAR_MASK) << PF_FAT_DATE_YEAR_SHIFT);
    *p_time = ((sys_time.sys_sec >> 1) & PF_FAT_TIME_SEC2_MASK) | ((sys_time.sys_min & PF_FAT_TIME_MIN_MASK) << PF_FAT_TIME_MIN_SHIFT) |
              ((sys_time.sys_hour & PF_FAT_TIME_HOUR_MASK) << PF_FAT_TIME_HOUR_SHIFT);

    return sys_time.sys_ms;
}

s32 PFENT_RemoveEntry(struct PF_DIR_ENT* p_ent, struct PF_ENT_ITER* p_iter) {
    u32 success_size;
    s32 err;
    u32 i;
    struct PF_VOLUME* p_vol;
    u8 dir_fb_free[1] = {PF_ENTRY_DELETED};

    p_vol = p_ent->p_vol;

    if (p_ent->num_entry_LFNs != 0 && (p_ent->small_letter_flag & PF_SMALL_LETTER_MASK) == 0) {
        for (i = 1; i <= p_ent->num_entry_LFNs; ++i) {
            err = PFENT_ITER_Retreat(p_iter, FALSE);
            if (err != 0) {
                return err;
            }

            err = PFSEC_WriteData(p_vol, dir_fb_free, p_iter->sector, p_iter->offset, 1, &success_size, FALSE);
            if (err != 0) {
                return err;
            }

            if (success_size != 1) {
                return PF_IERR_IO;
            }
        }
    }

    err = PFSEC_WriteData(p_vol, dir_fb_free, p_ent->entry_sector, p_ent->entry_offset, 1, &success_size, FALSE);
    if (err != 0) {
        return err;
    }

    if (success_size != 1) {
        return PF_IERR_IO;
    }

    err = PFCACHE_FlushDataCacheSpecific(p_vol, NULL);
    if (err != 0) {
        return err;
    }
    return 0;
}
