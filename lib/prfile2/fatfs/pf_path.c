#include <prfile2/pf_api.h>
#include <prfile2/pf_clib.h>
#include <prfile2/pf_code.h>
#include <prfile2/pf_path.h>
#include <prfile2/pf_service.h>
#include <prfile2/pf_volume.h>
#include <prfile2/pf_w_clib.h>

enum {
    PF_FW_SMALL_A = 0xFF41,      // fullwidth 'a'
    PF_FW_SMALL_Z = 0xFF5A,      // fullwidth 'z'
    PF_EXTSFN_SIG0 = 0x01,       // numbered short name of Unicode mode: two signature bytes and the index digits
    PF_EXTSFN_SIG1 = 0x02,
    PF_EXTSFN_SIG_LEN = 2,
    PF_SFN_TAIL_DIGITS_MAX = 6,  // "~999999"
    PF_SFN_FIRST_TAIL_LEN = 2,   // "~1", or "__" in Unicode mode
    PF_DOT_DOT_LEN = 2,          // ".."
};

s32 PFPATH_DoSplitPath(struct PF_STR* p_path, struct PF_STR* p_dir_path, struct PF_STR* p_filename, u32 wildcard) {
    s8* p;
    s8* p_tail_prev;
    u32 code_mode;
    struct PF_STR token;
    struct PF_STR token_prev;
    s32 err;

    code_mode = PFSTR_GetCodeMode(p_path);
    PFSTR_SetCodeMode(p_dir_path, code_mode);
    PFSTR_SetCodeMode(p_filename, code_mode);

    p_dir_path->p_head = NULL;
    if (p_filename != NULL) {
        p_filename->p_head = NULL;
    }

    if (PFSTR_StrNCmp(p_path, (const s8*)":", PF_STR_TARGET_HEAD, PF_DRIVE_LETTER_LEN, 1) == 0) {
        PFSTR_MoveStrPos(p_path, PF_DRIVE_SPEC_LEN);
    }

    p = (s8*)p_path->p_head;
    PFPATH_InitTokenOfPath(&token, (s8*)p_path->p_head, code_mode);
    err = PFPATH_GetNextTokenOfPath(&token, wildcard);

    if (err != 0) {
        return err;
    }

    if (token.p_head == NULL || PFSTR_StrLen(&token) == 0) {
        return PF_IERR_INVALID_PATH;
    }

    if (p_path->p_tail < token.p_tail) {
        return PF_IERR_INVALID_PATH;
    }

    token_prev = token;

    while (1) {
        if (PFSTR_StrNCmp(&token, (const s8*)"\0", PF_STR_TARGET_TAIL, 0, 1) == 0) {
            p = (s8*)token_prev.p_tail;
            break;
        }

        err = PFPATH_GetNextTokenOfPath(&token, wildcard);
        if (err != 0) {
            return err;
        }

        if (PFSTR_StrLen(&token) == 0) {
            return PF_IERR_INVALID_PATH;
        }

        if (p_path->p_tail < token.p_tail) {
            break;
        }

        if (token.p_head == NULL) {
            break;
        }

        p_tail_prev = (s8*)token_prev.p_tail;
        p = p_tail_prev;
        token_prev = token;
    }

    p_dir_path->p_head = p_path->p_head;
    p_dir_path->p_tail = p;
    if (p_filename != NULL) {
        *p_filename = token_prev;
    }

    return 0;
}

static u32 PFPATH_IsValidLongNameChar(u32 wc) {
    u32 is_valid = 1;

    if ((u16)wc < PF_FN_CHAR_END) {
        s32 is_valid_ascii = 0;
        u8 c = wc;

        if (c >= PF_FN_CHAR_FIRST && (c & 0xFF00) == 0) {
            if ((pf_valid_fn_char[(u8)(c - PF_FN_CHAR_FIRST)] & PF_FN_CHAR_LONG) != 0) {
                is_valid_ascii = 1;
            }
        }
        if (is_valid_ascii == 0) {
            is_valid = 0;
        }
    }
    return is_valid;
}

static u32 PFPATH_UNI_ConvertFWchar(u16 src, u16* dst) {
    if (PF_FW_SMALL_A <= src && src <= PF_FW_SMALL_Z) {
        *dst = src - ('a' - 'A');
        return 1;
    } else {
        return 0;
    }
}

static u32 PFPATH_OEM_ConvertFWchar(const s8* src, u16* dst) {
    pf_vol_set.codeset.oem2unicode(src, dst);
    if (PFPATH_UNI_ConvertFWchar(*dst, dst) == 1) {
        pf_vol_set.codeset.unicode2oem(dst, (s8*)dst);
        *dst = (u16)(*dst);
        return 1;
    }
    return 0;
}

static u16 PFPATH_GetNextCharOfShortName(struct PF_FILE_NAME_ITER* p_name) {
    u16 wc;
    s8 c;

    if (p_name->index >= PF_SFN_LEN + 1) {
        return 0;
    }

    wc = p_name->buf[p_name->index++];
    if (pf_vol_set.codeset.is_oem_mb_char(wc, PF_MB_LEAD_BYTE) != 0) {
        c = p_name->buf[p_name->index++];
        wc = ((u8)wc << 8) + (u8)c;
    }

    wc = (wc >= 'a' && wc <= 'z') ? wc - ('a' - 'A') : wc;
    return wc;
}

static u16 PFPATH_GetNextCharOfLongName(struct PF_FILE_NAME_ITER* p_name) {
    u16 wchar;
    u16 wc;

    if (p_name->index > PF_DIR_ENT_LONG_NAME_LEN * sizeof(u16)) {
        return 0;
    }

    wc = *(const u16*)(p_name->buf + p_name->index);
    p_name->index += sizeof(u16);

    wc = (wc >= 'a' && wc <= 'z') ? wc - ('a' - 'A') : wc;

    if (PFPATH_UNI_ConvertFWchar(wc, &wchar) == 1) {
        wc = wchar;
    }
    return wc;
}

static u16 PFPATH_GetNextCharOfFileName(struct PF_FILE_NAME_ITER* p_name) {
    if (p_name->is_long_name != 0) {
        return PFPATH_GetNextCharOfLongName(p_name);
    } else {
        return PFPATH_GetNextCharOfShortName(p_name);
    }
}

u16 PFPATH_GetNextCharOfPattern(struct PF_STR* p_pattern, u32 is_long_name) {
    u16 twc;
    u16 wc;
    u16 tmp_wc;
    s8 pattern[3];

    if (p_pattern->code_mode == PF_STR_CODEMODE_OEM) {
        pattern[0] = *p_pattern->p_head++;
        pattern[1] = '\0';

        if (pattern[0] == '\0' || p_pattern->p_tail < p_pattern->p_head) {
            return 0;
        }

        if (pf_vol_set.codeset.is_oem_mb_char(pattern[0], PF_MB_LEAD_BYTE) != 0) {
            pattern[1] = *p_pattern->p_head++;
            twc = ((u8)pattern[0] << 8) + (u8)pattern[1];
        } else if ((pf_vol_set.config & PF_CONFIG_NO_CHAR_CHECK) || (u8)pattern[0] >= PF_FN_CHAR_END || ((u8)pattern[0] >= PF_FN_CHAR_FIRST && (pf_valid_fn_char[(u8)pattern[0] - PF_FN_CHAR_FIRST] & PF_FN_CHAR_SHORT))) {
            twc = pattern[0];
        } else {
            twc = '_';
        }

        if (is_long_name != 0) {
            pf_vol_set.codeset.oem2unicode(pattern, &twc);
        }
    } else {
        pattern[0] = *(const u16*)p_pattern->p_head;
        pattern[1] = *(const u16*)p_pattern->p_head >> 8;
        p_pattern->p_head += sizeof(u16);
        pattern[2] = '\0';

        if ((pattern[0] == '\0' && pattern[1] == '\0') || p_pattern->p_tail < p_pattern->p_head) {
            return 0;
        }

        twc = ((u8)pattern[1] << 8) + (u8)pattern[0];

        if (is_long_name == 0) {
            pf_vol_set.codeset.unicode2oem(&twc, (s8*)&wc);
            if (pf_vol_set.codeset.is_oem_mb_char(wc >> 8, PF_MB_LEAD_BYTE) != 0) {
                twc = wc;
            } else if ((pf_vol_set.config & PF_CONFIG_NO_CHAR_CHECK) || (u8)pattern[0] >= PF_FN_CHAR_END || ((u8)pattern[0] >= PF_FN_CHAR_FIRST && (pf_valid_fn_char[(u8)pattern[0] - PF_FN_CHAR_FIRST] & PF_FN_CHAR_SHORT))) {
                twc = pattern[0];
            } else {
                twc = '_';
            }
        }
    }

    twc = (twc >= 'a' && twc <= 'z') ? twc - ('a' - 'A') : twc;

    if (is_long_name != 0) {
        if (PFPATH_UNI_ConvertFWchar(twc, &tmp_wc) == 1) {
            twc = tmp_wc;
        }
    } else {
        if (PFPATH_OEM_ConvertFWchar((const s8*)&twc, &tmp_wc) == 1) {
            twc = tmp_wc;
        }
    }

    return twc;
}

u32 PFPATH_DoMatchFileNameWithPattern(u16 c_name, struct PF_FILE_NAME_ITER* p_name, u16 c_pat, struct PF_STR* p_pattern, u32 is_long_name) {
    struct PF_FILE_NAME_ITER name;
    struct PF_STR pattern;

    while (c_pat != '\0') {
        switch (c_pat) {
            case '?': {
                if (c_name == '\0') {
                    return 0;
                }
                break;
            }
            case '*': {
                c_pat = PFPATH_GetNextCharOfPattern(p_pattern, is_long_name);
                while (c_pat == '*' || c_pat == '?') {
                    c_pat = PFPATH_GetNextCharOfPattern(p_pattern, is_long_name);
                }
                if (c_pat == '\0') {
                    return 1;
                }

                while (c_name != '\0') {
                    if (c_name == c_pat) {
                        name = *p_name;
                        pattern = *p_pattern;

                        c_name = PFPATH_GetNextCharOfFileName(&name);
                        c_pat = PFPATH_GetNextCharOfPattern(&pattern, is_long_name);

                        if (PFPATH_DoMatchFileNameWithPattern(c_name, &name, c_pat, &pattern, is_long_name)) {
                            return 1;
                        }
                    }
                    c_name = PFPATH_GetNextCharOfFileName(p_name);
                }
                return 0;
            }
            default: {
                if (c_name != c_pat) {
                    return 0;
                }
                break;
            }
        }
        c_name = PFPATH_GetNextCharOfFileName(p_name);
        c_pat = PFPATH_GetNextCharOfPattern(p_pattern, is_long_name);
    }
    return c_name == '\0';
}

s32 PFPATH_cmpNameImpl(const s8* sName, const s8* sPattern, u32* p_is_end) {
    s32 pw;
    s32 nw;
    s32 ret;
    u16 p;
    u16 n;
    u16 tp;
    u16 tn;

    while (*sPattern != '\0') {
        pw = pf_vol_set.codeset.oem_char_width(sPattern);
        nw = pf_vol_set.codeset.oem_char_width(sName);

        p = (pw == 1) ? pf_toupper(*sPattern) : PF_GET_LE_U16((const u8*)sPattern);
        n = (nw == 1) ? pf_toupper(*sName) : PF_GET_LE_U16((const u8*)sName);

        if (pw == 2 && PFPATH_OEM_ConvertFWchar(sPattern, &tp) == 1) {
            p = tp;
        }

        if (nw == 2 && PFPATH_OEM_ConvertFWchar(sName, &tn) == 1) {
            n = tn;
        }

        sPattern += pw;

        switch (p) {
            case '?': {
                if (n == 0) {
                    return 1;
                }
                break;
            }
            case '*': {
                do {
                    pw = pf_vol_set.codeset.oem_char_width(sPattern);
                    p = (pw == 1) ? pf_toupper(*sPattern) : PF_GET_LE_U16((const u8*)sPattern);

                    sPattern += pw;

                    if (p == '?') {
                        if (n == 0) {
                            return 1;
                        }
                        sName += nw;
                        nw = pf_vol_set.codeset.oem_char_width(sName);
                        n = (nw == 1) ? pf_toupper(*sName) : PF_GET_LE_U16((const u8*)sName);
                    }
                } while (p == '?' || p == '*');

                if (p == 0) {
                    return 0;
                }

                while (n != 0) {
                    sName += nw;
                    if (n == p) {
                        ret = PFPATH_cmpNameImpl(sName, sPattern, p_is_end);
                        if (ret == 0) {
                            return 0;
                        }
                        if (*p_is_end != 0) {
                            return ret;
                        }
                    }

                    nw = pf_vol_set.codeset.oem_char_width(sName);
                    n = (nw == 1) ? pf_toupper(*sName) : PF_GET_LE_U16((const u8*)sName);
                }

                if (*sName == '\0' || *sPattern == '\0') {
                    *p_is_end = 1;
                }
            }

            default: {
                if (n != p) {
                    return 1;
                }
                break;
            }
        }

        sName += nw;
    }

    return *sName != '\0';
}

s32 PFPATH_cmpNameUni(const u16* p_name, struct PF_STR* sPattern) {
    return PFPATH_MatchFileNameWithPattern((const s8*)p_name, sPattern, TRUE) == 0;
}

s32 PFPATH_cmpName(const s8* sShort, struct PF_STR* p_pattern, u32 is_short_search) {
    u32 is_end;
    s8 tmpBuf[PF_SFN_BUF_LEN];
    s8* p_tmpBuf;
    const s8* sPattern;

    is_end = 0;
    p_tmpBuf = tmpBuf;
    sPattern = PFSTR_GetStrPos(p_pattern, PF_STR_TARGET_LOCAL);
    pf_strcpy(tmpBuf, sShort);

    if ((pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) == PF_ENCODE_MODE_UNICODE && PFSTR_GetCodeMode(p_pattern) == PF_STR_CODEMODE_UNICODE && pf_strcmp(tmpBuf, (const s8*)".") != 0 && pf_strcmp(tmpBuf, (const s8*)"..") != 0 && PFPATH_CheckExtShortName(p_pattern, PF_STR_TARGET_LOCAL, TRUE) == 0 && is_short_search == 0) {
        return 1;
    }

    if (pf_strcmp(sPattern, (const s8*)"*.") == 0) {
        while (*p_tmpBuf != '\0' && *p_tmpBuf != '.') {
            p_tmpBuf++;
        }
        if (*p_tmpBuf == '\0') {
            *p_tmpBuf = '.';
            p_tmpBuf[1] = '\0';
        }
    } else {
        if (*sPattern == '\0') {
            return 1;
        }
        if (tmpBuf[0] == '\0') {
            return 1;
        }
    }

    return PFPATH_cmpNameImpl(tmpBuf, sPattern, &is_end);
}

s32 PFPATH_cmpTailSFN(const s8* sfn_name, const s8* pattern) {
    return pf_strcmp(sfn_name, pattern) != 0;
}

void PFPATH_InitTokenOfPath(struct PF_STR* p_str, s8* path, u32 code_mode) {
    p_str->p_head = path;
    p_str->p_tail = path;
    p_str->code_mode = code_mode;
}

s32 PFPATH_GetNextTokenOfPath(struct PF_STR* p_str, u32 wildcard) {
    u32 extsfn_len;
    u32 code_mode;

    p_str->p_head = p_str->p_tail;
    extsfn_len = 0;

    if (PFSTR_StrNCmp(p_str, (const s8*)"\0", PF_STR_TARGET_HEAD, 0, 1) == 0) {
        p_str->p_tail = NULL;
        p_str->p_head = NULL;
        return 0;
    }

    if (PFSTR_StrNCmp(p_str, (const s8*)"\\", PF_STR_TARGET_HEAD, 0, 1) == 0 || PFSTR_StrNCmp(p_str, (const s8*)"/", PF_STR_TARGET_HEAD, 0, 1) == 0) {
        PFSTR_MoveStrPos(p_str, 1);
    }

    if (PFSTR_StrNCmp(p_str, (const s8*)"\\", PF_STR_TARGET_HEAD, 0, 1) == 0 || PFSTR_StrNCmp(p_str, (const s8*)"/", PF_STR_TARGET_HEAD, 0, 1) == 0) {
        return PF_IERR_INVALID_PATH;
    }

    code_mode = PFSTR_GetCodeMode(p_str);
    p_str->p_tail = p_str->p_head;

    if ((pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) == PF_ENCODE_MODE_UNICODE) {
        extsfn_len = PFPATH_CheckExtShortName(p_str, PF_STR_TARGET_TAIL, wildcard);
        if (extsfn_len != 0) {
            if (code_mode == PF_STR_CODEMODE_OEM) {
                p_str->p_tail += extsfn_len;
            } else {
                p_str->p_tail += extsfn_len * sizeof(u16);
            }
        }
    }

    if (extsfn_len == 0) {
        while (PFSTR_StrNCmp(p_str, (const s8*)"\0", PF_STR_TARGET_TAIL, 0, 1) != 0) {
            if (code_mode == PF_STR_CODEMODE_OEM && pf_vol_set.codeset.is_oem_mb_char(*p_str->p_tail, PF_MB_LEAD_BYTE) != 0) {
                p_str->p_tail++;
                if (pf_vol_set.codeset.is_oem_mb_char(*p_str->p_tail, PF_MB_TRAIL_BYTE) == 0 || *p_str->p_tail == '\0') {
                    return PF_IERR_INVALID_PATH;
                }
            } else {
                if (PFSTR_StrNCmp(p_str, (const s8*)"\\", PF_STR_TARGET_TAIL, 0, 1) == 0 || PFSTR_StrNCmp(p_str, (const s8*)"/", PF_STR_TARGET_TAIL, 0, 1) == 0) {
                    break;
                }

                if ((pf_vol_set.config & PF_CONFIG_NO_CHAR_CHECK) == 0) {
                    if ((code_mode == PF_STR_CODEMODE_OEM && (u8)*p_str->p_tail < PF_FN_CHAR_END && ((u8)*p_str->p_tail < PF_FN_CHAR_FIRST || (pf_valid_fn_char[(u8)*p_str->p_tail - PF_FN_CHAR_FIRST] & PF_FN_CHAR_LONG) == 0)) ||
                        (code_mode == PF_STR_CODEMODE_UNICODE && PFPATH_IsValidLongNameChar(((u8)p_str->p_tail[0] << 8) + (u8)p_str->p_tail[1]) == 0)) {
                        if (wildcard == 0 || (PFSTR_StrNCmp(p_str, (const s8*)"*", PF_STR_TARGET_TAIL, 0, 1) != 0 && PFSTR_StrNCmp(p_str, (const s8*)"?", PF_STR_TARGET_TAIL, 0, 1) != 0)) {
                            return PF_IERR_INVALID_PATH;
                        }
                    }
                }
            }

            if (code_mode == PF_STR_CODEMODE_OEM) {
                p_str->p_tail += 1;
            } else {
                p_str->p_tail += sizeof(u16);
            }
        }
    }
    return 0;
}

s32 PFPATH_SplitPath(struct PF_STR* p_path, struct PF_STR* p_dir_path, struct PF_STR* p_filename) {
    return PFPATH_DoSplitPath(p_path, p_dir_path, p_filename, FALSE);
}

s32 PFPATH_SplitPathPattern(struct PF_STR* p_path, struct PF_STR* p_dir_path, struct PF_STR* p_filename) {
    return PFPATH_DoSplitPath(p_path, p_dir_path, p_filename, TRUE);
}

struct PF_VOLUME* PFPATH_GetVolumeFromPath(struct PF_STR* p_path) {
    struct PF_VOLUME* p_vol;
    s8 drv_char[2];

    if (PFSTR_StrNumChar(p_path, PF_STR_TARGET_HEAD) >= PF_DRIVE_SPEC_LEN && PFSTR_StrNCmp(p_path, (const s8*)":", PF_STR_TARGET_HEAD, PF_DRIVE_LETTER_LEN, 1) == 0) {
        PFSTR_ToUpperNStr(p_path, PF_DRIVE_LETTER_LEN, drv_char);
        p_vol = PFVOL_GetVolumeFromDrvChar(drv_char[0]);
    } else {
        p_vol = PFVOL_GetCurrentVolume();
    }
    return p_vol;
}

u32 PFPATH_MatchFileNameWithPattern(const s8* file_name, struct PF_STR* p_pattern, u32 is_long_name) {
    u16 c_name;
    u16 c_pat;
    struct PF_FILE_NAME_ITER name;
    struct PF_STR pattern;
    u32 is_match;

    is_match = 1;

    name.buf = file_name;
    name.dot_inserted = 0;
    name.is_long_name = is_long_name;
    name.index = 0;

    pattern = *p_pattern;

    if (PFSTR_GetCodeMode(p_pattern) == PF_STR_CODEMODE_OEM) {
        if (is_long_name == 0 && (pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) == PF_ENCODE_MODE_UNICODE && PFPATH_CheckExtShortNameSignature(&pattern) == 1) {
            is_match = PFPATH_CheckExtShortNameSignature(&pattern);
            if (is_match == 1) {
                name.index += PF_EXTSFN_SIG_LEN;
                pattern.p_head += PF_EXTSFN_SIG_LEN;
            }
        }
    } else {
        if ((pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) == PF_ENCODE_MODE_UNICODE && is_long_name == 0 && PFSTR_StrNCmp(p_pattern, (const s8*)".", PF_STR_TARGET_HEAD, 0, 1) != 0 && PFSTR_StrNCmp(p_pattern, (const s8*)"..", PF_STR_TARGET_HEAD, 0, PF_DOT_DOT_LEN) != 0 && PFPATH_CheckExtShortName(p_pattern, PF_STR_TARGET_HEAD, FALSE) == 0) {
            is_match = 0;
        }
    }

    if (is_match == 1) {
        c_name = PFPATH_GetNextCharOfFileName(&name);
        c_pat = PFPATH_GetNextCharOfPattern(&pattern, is_long_name);
        is_match = PFPATH_DoMatchFileNameWithPattern(c_name, &name, c_pat, &pattern, is_long_name);
    }

    return is_match;
}

s32 PFPATH_putShortName(u8* pDirEntry, const s8* short_name, u8 attr) {
    s32 i;

    for (i = 0; i < PF_SFN_BASE_LEN && *short_name != '\0' && *short_name != '.'; i++) {
        *pDirEntry++ = *short_name++;
    }

    if (i == 0) {
        for (i = 0; i < PF_DOT_DOT_LEN && *short_name != '\0'; i++) {
            *pDirEntry++ = *short_name++;
        }
    }

    if ((attr & PF_ATTR_VOLUME) == 0) {
        while (i < PF_SFN_BASE_LEN) {
            *pDirEntry++ = ' ';
            i++;
        }
    }

    if (*short_name != '\0') {
        if ((attr & PF_ATTR_VOLUME) == 0) {
            short_name++;
        }
        while (*short_name != '\0') {
            *pDirEntry++ = *short_name++;
            i++;
        }
    }

    while (i < PF_SFN_LEN) {
        *pDirEntry++ = ' ';
        i++;
    }

    return 0;
}

s32 PFPATH_getShortName(s8* short_name, const u8* pDirEntry, u8 attr) {
    s32 i;
    s32 nLen;

    i = 0;
    nLen = -1;
    while (i < PF_SFN_BASE_LEN) {
        short_name[i] = pDirEntry[i];
        if (short_name[i] != ' ') {
            nLen = i;
        }
        i++;
    }

    short_name += nLen + 1;

    nLen = PF_SFN_BASE_LEN - 1;
    for (i = PF_SFN_BASE_LEN; i < PF_SFN_LEN; i++) {
        if (pDirEntry[i] != ' ') {
            nLen = i;
        }
    }

    if (nLen > PF_SFN_BASE_LEN - 1) {
        if ((attr & PF_ATTR_VOLUME) == 0) {
            *short_name++ = '.';
        }
        for (i = PF_SFN_BASE_LEN; i <= nLen; i++) {
            *short_name++ = pDirEntry[i];
        }
    }
    *short_name = '\0';
    return 0;
}

void PFPATH_getLongNameformShortName(s8* short_name, s8* long_name, u8 flag) {
    s32 i;
    s32 j;

    for (i = 0; i < PF_SFN_BASE_LEN; i++) {
        if (short_name[i] == '\0' || short_name[i] == '.') {
            break;
        }

        if ((flag & PF_SMALL_LETTER_BASE) != 0 && short_name[i] >= 'A' && short_name[i] <= 'Z') {
            long_name[i] = short_name[i] + ('a' - 'A');
        } else {
            long_name[i] = short_name[i];
        }
    }

    if (short_name[i] == '.') {
        long_name[i] = '.';
        i++;
    }

    for (j = i + PF_SFN_EXT_LEN; i < j; i++) {
        if (short_name[i] == '\0') {
            break;
        }

        if ((flag & PF_SMALL_LETTER_EXT) != 0 && short_name[i] >= 'A' && short_name[i] <= 'Z') {
            long_name[i] = short_name[i] + ('a' - 'A');
        } else {
            long_name[i] = short_name[i];
        }
    }

    long_name[i] = '\0';
}

u32 PFPATH_GetLengthFromShortname(const s8* sSrc) {
    s32 i;
    u32 szStr;

    for (szStr = i = 0; sSrc[i] != '\0'; i++) {
        if (i == PF_SFN_BASE_LEN && (sSrc[i] != ' ' || sSrc[i + 1] != ' ' || sSrc[i + 2] != ' ')) {
            szStr++;
        }
        if (sSrc[i] != ' ') {
            szStr++;
        }
    }
    return szStr;
}

u32 PFPATH_GetLengthFromUnicode(const u16* sSrc) {
    s32 i;
    s32 width;
    u32 szStr;
    s16 oem_width;
    s16 uni_width;
    s8 Dest[2];

    szStr = 0;
    for (width = 0; sSrc[width] != 0; width += uni_width >> 1) {
        PFCODE_Divide_Width(pf_vol_set.codeset.unicode2oem(sSrc + width, Dest), &oem_width, &uni_width);
        szStr += oem_width;
    }
    return szStr;
}

s32 PFPATH_transformFromUnicodeToNormal(s8* sDest, const u16* sSrc) {
    s32 i;
    s32 width;
    u32 szStr;
    s16 oem_width;
    s16 uni_width;
    u16 dot_buf[2];
    u16 space_buf[2];

    if ((pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) == PF_ENCODE_MODE_UNICODE) {
        dot_buf[0] = '.';
        dot_buf[1] = 0;
        space_buf[0] = ' ';
        space_buf[1] = 0;
        for (i = 0; sSrc[i] != 0; i++) {
            if (pf_w_strncmp(sSrc + i, dot_buf, 1) == 0) {
                *sDest = '.';
            } else if (pf_w_strncmp(sSrc + i, space_buf, 1) == 0) {
                *sDest = ' ';
            } else {
                *sDest = '_';
            }
            sDest++;
        }
    } else {
        for (i = 0; sSrc[i] != 0; i += uni_width >> 1) {
            PFCODE_Divide_Width(pf_vol_set.codeset.unicode2oem(sSrc + i, sDest), &oem_width, &uni_width);
            sDest += oem_width;
        }
    }
    *sDest = '\0';
    return 0;
}

s32 PFPATH_transformInUnicode(u16* sDestStr, const s8* sSrcStr) {
    s32 i;
    s32 width;
    s32 szStr;
    s16 oem_width;
    s16 uni_width;

    szStr = 0;
    for (width = 0; sSrcStr[width] != '\0'; width += oem_width, szStr++) {
        PFCODE_Divide_Width(pf_vol_set.codeset.oem2unicode(sSrcStr + width, sDestStr), &oem_width, &uni_width);
        sDestStr += uni_width >> 1;
    }
    *sDestStr = 0;
    return szStr;
}

u32 PFPATH_parseShortName(s8* pDest, struct PF_STR* p_pattern) {
    s32 width;
    const s8* p_cur_src;
    u32 is_create_long;
    u32 is_create_tail;
    u16* p_name_cnt;
    u16 num_base;
    u16 num_ext;
    u16 last_width;
    u16 prev_last_width;
    u16 src_dot;
    u16 prev_src_dot;
    u16 src_pos;
    u16 dst_pos;
    u16 ext_pos;
    u16 move_cnt;
    s16 i;
    u16 wchar;
    u16 t_wchar;

    is_create_long = 0;
    is_create_tail = 0;
    p_cur_src = PFSTR_GetStrPos(p_pattern, PF_STR_TARGET_LOCAL);

    if ((pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) == 0) {
        while (*p_cur_src == ' ' || *p_cur_src == '.') {
            is_create_tail = 1;
            p_cur_src++;
        }

        src_dot = 0;
        prev_src_dot = 0;
        for (src_pos = 0; p_cur_src[src_pos] != '\0'; src_pos++) {
            if (p_cur_src[src_pos] == '.') {
                if (src_dot != 0) {
                    prev_src_dot = src_dot;
                }
                src_dot = src_pos;
            }
        }

        if (src_dot != 0 && p_cur_src[src_dot + 1] == '\0') {
            if (prev_src_dot != 0) {
                src_dot = prev_src_dot;
            }
            is_create_tail = 1;
        }

        dst_pos = 0;
        src_pos = 0;
        num_base = 0;
        num_ext = 0;
        p_name_cnt = &num_base;
        last_width = 1;
        prev_last_width = 1;

        while (num_ext < PF_SFN_EXT_LEN && p_cur_src[src_pos] != '\0') {
            if ((num_base != PF_SFN_BASE_LEN && (src_dot == 0 || src_pos != src_dot)) || p_name_cnt == &num_ext) {
                if (p_cur_src[src_pos] != ' ' && p_cur_src[src_pos] != '.') {
                    width = pf_vol_set.codeset.oem_char_width(p_cur_src + src_pos);

                    if (width != 1) {
                        if ((src_pos < src_dot || src_dot == 0) && num_base + width > PF_SFN_BASE_LEN) {
                            is_create_tail = 1;
                        } else {
                            if (src_dot != 0 && src_pos > src_dot) {
                                if (num_ext + width > PF_SFN_EXT_LEN) {
                                    is_create_tail = 1;
                                    break;
                                }
                            }

                            if (p_name_cnt == &num_base) {
                                prev_last_width = last_width;
                                last_width = width;
                            }

                            while (width != 0) {
                                wchar = ((u8)p_cur_src[src_pos] << 8) + (u8)p_cur_src[src_pos + 1];
                                if (PFPATH_OEM_ConvertFWchar(p_cur_src + src_pos, &t_wchar)) {
                                    wchar = t_wchar;
                                    is_create_long = 1;
                                }

                                pDest[dst_pos] = wchar >> 8;
                                pDest[dst_pos + 1] = wchar;
                                dst_pos += 2;
                                width -= 2;
                                src_pos += 2;
                                *p_name_cnt += 2;
                            }
                            src_pos--;
                        }
                    } else {
                        if (p_name_cnt == &num_base) {
                            prev_last_width = last_width;
                            last_width = 1;
                        }

                        if ((pf_vol_set.config & PF_CONFIG_NO_CHAR_CHECK) || (u8)p_cur_src[src_pos] >= PF_FN_CHAR_END || ((u8)p_cur_src[src_pos] >= PF_FN_CHAR_FIRST && (pf_valid_fn_char[(u8)p_cur_src[src_pos] - PF_FN_CHAR_FIRST] & PF_FN_CHAR_SHORT))) {
                            pDest[dst_pos] = pf_toupper(p_cur_src[src_pos]);
                            dst_pos++;
                            (*p_name_cnt)++;

                            if (p_cur_src[src_pos] >= 'a' && p_cur_src[src_pos] <= 'z') {
                                is_create_long = 1;
                            }
                        } else {
                            is_create_tail = 1;

                            pDest[dst_pos] = '_';
                            dst_pos++;
                            (*p_name_cnt)++;
                        }
                    }
                } else {
                    is_create_tail = 1;
                }
            } else {
                p_name_cnt = &num_ext;
                if (p_cur_src[src_pos] != '\0' && p_cur_src[src_pos] != '.') {
                    is_create_tail = 1;
                }
                if (src_dot != 0) {
                    pDest[dst_pos++] = '.';
                    src_pos = src_dot;
                } else {
                    break;
                }
            }
            src_pos++;
        }

        if (num_ext == PF_SFN_EXT_LEN && p_cur_src[src_pos] != '\0') {
            is_create_tail = 1;
        }
        pDest[dst_pos] = '\0';

        if (is_create_tail != 0) {
            dst_pos = num_base;
            if (num_base == PF_SFN_BASE_LEN) {
                if (last_width != 1) {
                    dst_pos = PF_SFN_BASE_LEN - last_width;
                } else {
                    dst_pos = PF_SFN_BASE_LEN - (prev_last_width == 1 ? 2 : prev_last_width + 1);
                }
            } else if (num_base == PF_SFN_BASE_LEN - 1) {
                dst_pos = dst_pos - prev_last_width;
            }

            ext_pos = dst_pos + PF_SFN_FIRST_TAIL_LEN;
            if (ext_pos < num_base) {
                move_cnt = num_base - ext_pos;
                for (i = -1; i < num_ext + 1; i++) {
                    pDest[num_base + i] = pDest[num_base + move_cnt + i];
                }
            } else if (ext_pos > num_base) {
                move_cnt = ext_pos - num_base;
                for (i = num_ext + 1; i >= 0; i--) {
                    pDest[num_base + move_cnt + i] = pDest[num_base + i];
                }
            }

            if (num_base != 0) {
                if ((pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) != PF_ENCODE_MODE_UNICODE) {
                    pDest[dst_pos++] = '~';
                    pDest[dst_pos++] = '1';
                } else {
                    pDest[dst_pos++] = '_';
                    pDest[dst_pos++] = '_';
                }
            }
            is_create_long = 1;
        }

        if (PFSTR_GetCodeMode(p_pattern) == PF_STR_CODEMODE_UNICODE) {
            is_create_long = 1;
        }
    } else {
        while (*p_cur_src != '\0' && is_create_tail == 0) {
            if (*p_cur_src != '.' && *p_cur_src != ' ') {
                is_create_tail = 1;
            }
            p_cur_src++;
        }

        if (is_create_tail != 0) {
            pDest[0] = PF_EXTSFN_SIG0;
            pDest[1] = PF_EXTSFN_SIG1;
            pDest[2] = '0';
            pDest[3] = '0';
            pDest[4] = '0';
            pDest[5] = '0';
            pDest[6] = '0';
            pDest[7] = '0';
            pDest[8] = '\0';
        } else {
            pDest[0] = '\0';
        }
        is_create_long = 1;
    }

    return is_create_long;
}

s32 PFPATH_parseShortNameNumeric(s8* p_char, u32 count) {
    u32 numeric_cnt;
    u32 pos_tail;
    u32 pos_dot;
    u32 pos_ext;
    u32 pos_slide;
    u32 pos_end;
    s8 numeric[PF_SFN_TAIL_DIGITS_MAX];

    if (count == 0) {
        return 0;
    }

    for (pos_tail = 1; p_char[pos_tail] != '~'; pos_tail++) {
    }

    for (pos_dot = pos_tail + 1; p_char[pos_dot] != '.' && p_char[pos_dot] != '\0'; pos_dot++) {
    }

    pos_ext = pos_dot + 1;

    if (p_char[pos_dot] == '.' && p_char[pos_ext] != '\0') {
        for (pos_end = pos_ext + 1; p_char[pos_end] != '\0'; pos_end++) {
        }
    } else {
        pos_end = pos_ext;
    }

    for (numeric_cnt = 0; count != 0; numeric_cnt++, count /= 10) {
        numeric[numeric_cnt] = (count % 10) + '0';
    }

    if (pos_tail + numeric_cnt >= pos_ext) {
        pos_slide = pos_tail + numeric_cnt + 1;
        if (pos_slide > PF_SFN_BASE_LEN) {
            pos_slide = PF_SFN_BASE_LEN;
        }
        p_char[pos_slide + 4] = p_char[pos_end];
        p_char[pos_slide + 3] = p_char[pos_ext + 2];
        p_char[pos_slide + 2] = p_char[pos_ext + 1];
        p_char[pos_slide + 1] = p_char[pos_ext];
        p_char[pos_slide] = p_char[pos_dot];
    }

    if (pos_tail + numeric_cnt >= PF_SFN_BASE_LEN) {
        pos_tail = PF_SFN_BASE_LEN - 1 - numeric_cnt;
    }

    p_char[pos_tail] = '~';
    pos_tail++;
    while (numeric_cnt != 0) {
        p_char[pos_tail] = numeric[numeric_cnt - 1];
        numeric_cnt--;
        pos_tail++;
    }

    return 0;
}

void PFPATH_SetSearchPattern(s8* p_buf_local, u16* p_buf_unicode, struct PF_STR* p_pattern) {
    u16 wc[2];

    if (PFSTR_GetCodeMode(p_pattern) == PF_STR_CODEMODE_OEM) {
        if (pf_strcmp(p_pattern->p_head, (const s8*)"*.*") == 0) {
            pf_strcpy(p_buf_local, (const s8*)"*");
        } else {
            pf_strcpy(p_buf_local, p_pattern->p_head);
        }
    } else {
        if (PFSTR_StrCmp(p_pattern, (const s8*)"*.*") == 0) {
            wc[0] = '*';
            wc[1] = 0;
            pf_w_strcpy(p_buf_unicode, wc);
        } else {
            pf_w_strcpy(p_buf_unicode, (const u16*)p_pattern->p_head);
        }
        if ((pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) == PF_ENCODE_MODE_UNICODE) {
            pf_vol_set.setting &= ~PF_ENCODE_MODE_MASK;
            pf_vol_set.setting |= PF_ENCODE_MODE_OEM;
            PFPATH_transformFromUnicodeToNormal(p_buf_local, p_buf_unicode);
            pf_vol_set.setting &= ~PF_ENCODE_MODE_MASK;
            pf_vol_set.setting |= PF_ENCODE_MODE_UNICODE;
        } else {
            PFPATH_transformFromUnicodeToNormal(p_buf_local, p_buf_unicode);
        }
    }
}

u32 PFPATH_CheckExtShortNameSignature(struct PF_STR* p_str) {
    u32 result;
    s8 sig[PF_EXTSFN_SIG_LEN] = {PF_EXTSFN_SIG0, PF_EXTSFN_SIG1};

    result = 0;

    if (PFSTR_StrNCmp(p_str, sig, PF_STR_TARGET_HEAD, 0, PF_EXTSFN_SIG_LEN) == 0) {
        result = 1;
    }
    return result;
}

static u32 PFPATH_IsPathSeparator(struct PF_STR* p_str, u32 target, s16 index) {
    return PFSTR_StrNCmp(p_str, (const s8*)"\\", target, index, 1) == 0 || PFSTR_StrNCmp(p_str, (const s8*)"/", target, index, 1) == 0;
}

u32 PFPATH_CheckExtShortName(struct PF_STR* p_str, u32 target, u32 wildcard) {
    u32 result;
    s16 i;
    s16 num;
    u32 is_wildcard;
    s8 sig[PF_EXTSFN_SIG_LEN] = {PF_EXTSFN_SIG0, PF_EXTSFN_SIG1};
    s8* p_c;
    u16* p_wc;

    result = 0;
    is_wildcard = 0;

    if (PFSTR_StrNCmp(p_str, sig, target, 0, PF_EXTSFN_SIG_LEN) == 0 || (PFSTR_StrNCmp(p_str, (const s8*)"?", target, 0, 1) == 0 && (PFSTR_StrNCmp(p_str, (const s8*)"?", target, 1, 1) == 0 || PFSTR_StrNCmp(p_str, (const s8*)"*", target, 0, 1) == 0)) || PFSTR_StrNCmp(p_str, (const s8*)"*", target, 0, 1) == 0) {
        for (i = PF_EXTSFN_SIG_LEN; i < PF_SFN_BASE_LEN && PFPATH_IsPathSeparator(p_str, target, i) == 0 && PFSTR_StrNCmp(p_str, (const s8*)" ", target, i, 1) != 0 &&
                    PFSTR_StrNCmp(p_str, (const s8*)"\0", target, i, 1) != 0;
             i++) {
            if (PFSTR_GetCodeMode(p_str) == PF_STR_CODEMODE_OEM) {
                p_c = PFSTR_GetStrPos(p_str, target);
                num = p_c[i] - '0';
            } else {
                p_wc = (u16*)PFSTR_GetStrPos(p_str, target);
                num = p_wc[i] - '0';
            }

            if (num < 0 || num > 9) {
                if (wildcard == 0) {
                    break;
                }

                if (PFSTR_StrNCmp(p_str, (const s8*)"*", target, i, 1) == 0 || PFSTR_StrNCmp(p_str, (const s8*)"?", target, i, 1) == 0) {
                    if (wildcard == 1 && PFSTR_StrNCmp(p_str, (const s8*)"*", target, i, 1) == 0) {
                        is_wildcard = 1;
                    }
                } else {
                    break;
                }
            }
        }

        if (i == PF_SFN_BASE_LEN || is_wildcard == 1) {
            if (PFSTR_StrNCmp(p_str, (const s8*)" ", target, i, 1) == 0 || PFPATH_IsPathSeparator(p_str, target, i) == 0 ||
                PFSTR_StrNCmp(p_str, (const s8*)"\0", target, i, 1) == 0) {
                result = i;
            }
        }
    }
    return result;
}

u32 PFPATH_GetExtShortNameIndex(struct PF_STR* p_str, u32* p_index) {
    u32 result;
    s16 i;
    s16 num;
    u32 index;
    s8 sig[PF_EXTSFN_SIG_LEN] = {PF_EXTSFN_SIG0, PF_EXTSFN_SIG1};
    s8* p_c;
    u16* p_wc;

    result = 0;

    if (PFSTR_StrNCmp(p_str, sig, PF_STR_TARGET_HEAD, 0, PF_EXTSFN_SIG_LEN) == 0) {
        index = 0;
        for (i = PF_EXTSFN_SIG_LEN; i < PF_SFN_BASE_LEN || PFSTR_StrNCmp(p_str, (const s8*)"\0", PF_STR_TARGET_HEAD, i, 1) != 0; i++) {
            if (PFSTR_GetCodeMode(p_str) == PF_STR_CODEMODE_OEM) {
                p_c = PFSTR_GetStrPos(p_str, PF_STR_TARGET_HEAD);
                num = p_c[i] - '0';
            } else {
                p_wc = (u16*)PFSTR_GetStrPos(p_str, PF_STR_TARGET_HEAD);
                num = p_wc[i] - '0';
            }

            if (num < 0 || num > 9) {
                break;
            }
            index = index * 10;
            index = index + num;
        }

        if (i == PF_SFN_BASE_LEN) {
            if (PFPATH_IsPathSeparator(p_str, PF_STR_TARGET_HEAD, i) == 0 || PFSTR_StrNCmp(p_str, NULL, PF_STR_TARGET_HEAD, i, 1) == 0) {
                *p_index = index;
                result = 1;
            }
        }
    }
    return result;
}

s32 PFPATH_AdjustExtShortName(s8* pName, u32 position) {
    u32 num;
    u32 i;
    u32 div1;
    u32 div2;

    i = PF_SFN_BASE_LEN - 1;
    div1 = 10;
    div2 = 1;

    while (i > PF_EXTSFN_SIG_LEN - 1 && position != 0) {
        num = position % div1;
        if (num != 0) {
            position -= num;
            num /= div2;
            pName[i] += (s8)num;
        }
        div1 *= 10;
        i--;
        div2 *= 10;
    }

    return 0;
}
