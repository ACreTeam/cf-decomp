#include <prfile2/pf_api.h>
#include <prfile2/pf_clib.h>

void PFSTR_SetCodeMode(struct PF_STR* p_str, u32 code_mode) {
    p_str->code_mode = code_mode;
}

u32 PFSTR_GetCodeMode(struct PF_STR* p_str) {
    return p_str->code_mode;
}

void PFSTR_SetLocalStr(struct PF_STR* p_str, s8* p_local) {
    if (p_str->code_mode == PF_STR_CODEMODE_OEM || p_local == NULL) {
        p_str->p_local = p_str->p_head;
    } else {
        p_str->p_local = p_local;
    }
}

s8* PFSTR_GetStrPos(struct PF_STR* p_str, u32 target) {
    s8* p_pos;
    if (target == PF_STR_TARGET_HEAD) {
        p_pos = (s8*)p_str->p_head;
    } else if (target == PF_STR_TARGET_TAIL) {
        p_pos = (s8*)p_str->p_tail;
    } else {
        p_pos = (s8*)p_str->p_local;
    }

    return p_pos;
}

void PFSTR_MoveStrPos(struct PF_STR* p_str, s16 num_char) {
    s16 cnt;
    s16 offset;
    s32 width;
    s8* p;
    u16* wp;

    offset = 0;

    if (PFSTR_GetCodeMode(p_str) == PF_STR_CODEMODE_OEM) {
        p = (s8*)p_str->p_head;

        while (num_char != 0) {
            if (pf_vol_set.codeset.is_oem_mb_char(*p, PF_MB_LEAD_BYTE)) {
                offset++;
            }
            offset++;
            num_char--;
        }
    } else {
        wp = (u16*)p_str->p_head;

        for (cnt = 0; cnt < num_char; cnt++) {
            width = pf_vol_set.codeset.unicode_char_width(wp);
            wp += width;
            offset += (s16)width;
        }
    }

    p_str->p_head += offset;
}

s32 PFSTR_InitStr(struct PF_STR* p_str, const s8* s, u32 code_mode) {
    if (code_mode == PF_STR_CODEMODE_OEM) {
        p_str->p_head = s;
        p_str->p_tail = s + pf_strlen(s);
    } else if (code_mode == PF_STR_CODEMODE_UNICODE) {
        p_str->p_head = s;
        p_str->p_tail = s + pf_w_strlen((const u16*)s) * sizeof(u16);
    }

    p_str->code_mode = code_mode;
    return 0;
}

u16 PFSTR_StrLen(struct PF_STR* p_str) {
    return (u16)(p_str->p_tail - p_str->p_head);
}

u16 PFSTR_StrNumChar(struct PF_STR* p_str, u32 target) {
    s8* p;
    u16 cnt;
    s32 width;

    if (target == PF_STR_TARGET_HEAD) {
        p = (s8*)p_str->p_head;
    } else {
        p = (s8*)p_str->p_tail;
    }

    if (p_str->code_mode == PF_STR_CODEMODE_OEM) {
        for (cnt = 0; *p != '\0'; p++, cnt++) {
            if (pf_vol_set.codeset.is_oem_mb_char(*p, PF_MB_LEAD_BYTE)) {
                p++;
            }
        }
    } else {
        for (cnt = 0; p[0] != '\0' || p[1] != '\0'; p += width, cnt++) {
            width = pf_vol_set.codeset.unicode_char_width((u16*)p);
        }
    }

    return cnt;
}

s32 PFSTR_StrCmp(const struct PF_STR* p_str, const s8* s) {
    u16 wc;
    const u16* wp;

    if (p_str->code_mode == PF_STR_CODEMODE_OEM) {
        return pf_strcmp(p_str->p_head, s);
    }

    wp = (const u16*)p_str->p_head;
    do {
        pf_vol_set.codeset.oem2unicode(s, &wc);
        s++;
    } while (*wp++ == wc && wp[-1] != L'\0' && wc != L'\0');

    return wp[-1] - wc;
}

s32 PFSTR_StrNCmp(struct PF_STR* p_str, const s8* s, u32 target, s16 offset, u16 num) {
    u16 wc;
    const u16* wp;
    const s8* p1;

    if (p_str->code_mode == PF_STR_CODEMODE_OEM || target == PF_STR_TARGET_LOCAL) {
        if (target == PF_STR_TARGET_HEAD) {
            p1 = p_str->p_head + offset;
        } else if (target == PF_STR_TARGET_TAIL) {
            p1 = p_str->p_tail + offset;
        } else {
            p1 = p_str->p_local + offset;
        }

        return pf_strncmp(p1, s, num);
    }

    if (target == PF_STR_TARGET_HEAD) {
        wp = (const u16*)p_str->p_head + offset;
    } else {
        wp = (const u16*)p_str->p_tail + offset;
    }

    do {
        pf_vol_set.codeset.oem2unicode(s, &wc);
        s++;
        num--;
    } while (*wp++ == wc && num > 0 && wp[-1] != L'\0' && wc != L'\0');

    return wp[-1] - wc;
}

void PFSTR_ToUpperNStr(struct PF_STR* p_str, u16 num, s8* dest) {
    u16 wc;
    const u16* wp;
    const s8* p;

    if (p_str->code_mode == PF_STR_CODEMODE_OEM) {
        for (p = p_str->p_head; num > 0 && *p != '\0'; p++, num--) {
            *dest++ = pf_toupper(*p);
        }
    } else {
        for (wp = (const u16*)p_str->p_head; num > 0 && *wp != L'\0'; num--, wp++) {
            wc = (*wp >= L'a' && *wp <= L'z') ? *wp - (L'a' - L'A') : *wp;
            dest[0] = (wc >> 0) & 0xFF;
            dest[1] = (wc >> 8) & 0xFF;
            dest += sizeof(u16);
        }

        *dest = '\0';
    }

    *dest = '\0';
}
