#include <lib/RegEad/RegEadPrc.hpp>

namespace RegEad {

u32 sPrcVersion = 0xFFFFFFFF;

MEMiHeapHead *sPrcExpHeap;
MEMiHeapHead *sPrcFrmHeap;
void **sPrcPatterns;
int sPrcPatternNum;

u16 sPrcKanaTable[0x60][2] = {
    {0x30A1, 0x30A2}, {0x30A3, 0x30A4}, {0x30A5, 0x30A6}, {0x30A7, 0x30A8},
    {0x30A9, 0x30AA}, {0x30C3, 0x30C4}, {0x30E3, 0x30E4}, {0x30E5, 0x30E6},
    {0x30E7, 0x30E8}, {0x30EE, 0x30EF}, {0x30F5, 0x30AB}, {0x30F6, 0x30B1},
    {0xFF66, 0x30F2}, {0xFF67, 0x30A1}, {0xFF68, 0x30A3}, {0xFF69, 0x30A5},
    {0xFF6A, 0x30A7}, {0xFF6B, 0x30A9}, {0xFF6C, 0x30E3}, {0xFF6D, 0x30E5},
    {0xFF6E, 0x30E7}, {0xFF6F, 0x30C3}, {0xFF70, 0x30FC}, {0xFF71, 0x30A2},
    {0xFF72, 0x30A4}, {0xFF73, 0x30A6}, {0xFF74, 0x30A8}, {0xFF75, 0x30AA},
    {0xFF76, 0x30AB}, {0xFF77, 0x30AD}, {0xFF78, 0x30AF}, {0xFF79, 0x30B1},
    {0xFF7A, 0x30B3}, {0xFF7B, 0x30B5}, {0xFF7C, 0x30B7}, {0xFF7D, 0x30B9},
    {0xFF7E, 0x30BB}, {0xFF7F, 0x30BD}, {0xFF80, 0x30BF}, {0xFF81, 0x30C1},
    {0xFF82, 0x30C4}, {0xFF83, 0x30C6}, {0xFF84, 0x30C8}, {0xFF85, 0x30CA},
    {0xFF86, 0x30CB}, {0xFF87, 0x30CC}, {0xFF88, 0x30CD}, {0xFF89, 0x30CE},
    {0xFF8A, 0x30CF}, {0xFF8B, 0x30D2}, {0xFF8C, 0x30D5}, {0xFF8D, 0x30D8},
    {0xFF8E, 0x30DB}, {0xFF8F, 0x30DE}, {0xFF90, 0x30DF}, {0xFF91, 0x30E0},
    {0xFF92, 0x30E1}, {0xFF93, 0x30E2}, {0xFF94, 0x30E4}, {0xFF95, 0x30E6},
    {0xFF96, 0x30E8}, {0xFF97, 0x30E9}, {0xFF98, 0x30EA}, {0xFF99, 0x30EB},
    {0xFF9A, 0x30EC}, {0xFF9B, 0x30ED}, {0xFF9C, 0x30EF}, {0xFF9D, 0x30F3},
    {0xFF9E, 0x3099}, {0xFF9F, 0x309A}, {0xFF73, 0x30F4}, {0xFF76, 0x30AC},
    {0xFF77, 0x30AE}, {0xFF78, 0x30B0}, {0xFF79, 0x30B2}, {0xFF7A, 0x30B4},
    {0xFF7B, 0x30B6}, {0xFF7C, 0x30B8}, {0xFF7D, 0x30BA}, {0xFF7E, 0x30BC},
    {0xFF7F, 0x30BE}, {0xFF80, 0x30C0}, {0xFF81, 0x30C2}, {0xFF82, 0x30C5},
    {0xFF83, 0x30C7}, {0xFF84, 0x30C9}, {0xFF8A, 0x30D0}, {0xFF8B, 0x30D3},
    {0xFF8C, 0x30D6}, {0xFF8D, 0x30D9}, {0xFF8E, 0x30DC}, {0xFF8A, 0x30D1},
    {0xFF8B, 0x30D4}, {0xFF8C, 0x30D7}, {0xFF8D, 0x30DA}, {0xFF8E, 0x30DD},
};

// 801F7FC4
int prcSetup(void *work, int workSize, const void *data, int dataSize) {
    if (workSize < PRC_WORK_SIZE_MIN) {
        return PRC_ERR_INVALID_ARG;
    }
    if (sPrcPatterns != NULL) {
        return PRC_ERR_ALREADY_SETUP;
    }
    sPrcExpHeap = MEMCreateExpHeapEx(work, PRC_EXP_HEAP_SIZE, 0);
    sPrcFrmHeap = MEMCreateFrmHeapEx((u8 *)work + PRC_EXP_HEAP_SIZE, workSize - PRC_EXP_HEAP_SIZE, 0);
    setHeap(sPrcExpHeap);
    sPrcPatterns = (void **)MEMAllocFromFrmHeapEx(sPrcFrmHeap, PRC_PATTERN_MAX * sizeof(void *), 4);
    if (*(const u16 *)data != 0x504C) {
        return PRC_ERR_BAD_DATA;
    }
    sPrcVersion = *(const u32 *)((const u8 *)data + 4);
    const void *packed = (const u8 *)data + 0x20;
    int size = CXGetUncompressedSize(packed);
    u16 *text = (u16 *)MEMAllocFromFrmHeapEx(sPrcFrmHeap, size, 4);
    CXSecureUncompressLZ(packed, dataSize, text);
    if (text[0] != 0xFEFF) {
        return PRC_ERR_BAD_DATA;
    }
    int i = 1;
    int start = 1;
    for (; i * 2 < size; i++) {
        int term;
        u16 ch = text[i];
        if (ch == 0xD) {
            i++;
            if (text[i] == 0xA) {
                term = 2;
                i++;
            } else {
                term = 1;
            }
        } else if (ch == 0xA) {
            term = 1;
            i++;
        } else if (ch == 0) {
            return PRC_ERR_BAD_DATA;
        } else {
            continue;
        }
        int result = prcAddPattern(text + start, i - term - start);
        if (prcIsFatalError(result)) {
            return result;
        }
        start = i;
    }
    if (i - 1 != start) {
        int result = prcAddPattern(text + start, i - start);
        if (result != PRC_OK) {
            return result;
        }
    }
    return PRC_OK;
}

// 801F81B4
int prcAddPattern(const u16 *line, int len) {
    RegNfa nfa;
    RegDfa dfa;
    if (sPrcPatternNum == PRC_PATTERN_MAX) {
        return PRC_ERR_TOO_MANY;
    }
    u16 *buf = (u16 *)MEMAllocFromExpHeapEx(sPrcExpHeap, (len + 1) * 2, 4);
    prcStripSpaces(buf, line, len);
    nfa.compile(buf, 3);
    MEMFreeToExpHeap(sPrcExpHeap, buf);
    int error = nfa.mError;
    if (error == RegNfa::ERR_NO_MEMORY) {
        return PRC_ERR_NO_MEMORY;
    }
    if (error != RegNfa::ERR_NONE) {
        return PRC_ERR_SYNTAX;
    }
    if (!dfa.build(&nfa)) {
        return PRC_ERR_NO_MEMORY;
    }
    sPrcPatterns[sPrcPatternNum] = MEMAllocFromFrmHeapEx(sPrcFrmHeap, dfa.getSerializedSize(), 4);
    void *dst = sPrcPatterns[sPrcPatternNum];
    if (dst == NULL) {
        return PRC_ERR_DFA_ALLOC;
    }
    dfa.serialize(dst);
    sPrcPatternNum++;
    return PRC_OK;
}

// 801F8384
void prcStripSpaces(u16 *dst, const u16 *src, int len) {
    u16 *out = dst;
    int n = 0;
    for (int i = 0; i < len; i++) {
        u16 ch = src[i];
        if (ch != 0x20 && ch != 0x3000) {
            *out++ = ch;
            n++;
        }
    }
    dst[n] = 0;
}

// 801F83D0
int prcCheck(const u16 **words, int num, u8 *results, int *hits) {
    if (words == NULL || results == NULL || hits == NULL || num < 0 || num > PRC_CHECK_MAX) {
        return PRC_ERR_INVALID_ARG;
    }
    int i, j;
    for (i = 0; i < num; i++) {
        if (words[i] == NULL) {
            return PRC_ERR_INVALID_ARG;
        }
    }
    if (sPrcPatterns == NULL) {
        return PRC_ERR_NOT_SETUP;
    }
    u16 **norm = (u16 **)MEMAllocFromExpHeapEx(sPrcExpHeap, num * sizeof(u16 *), 4);
    if (norm == NULL) {
        return PRC_ERR_ALLOC;
    }
    for (i = 0; i < num; i++) {
        const u16 *p = words[i];
        int len = 0;
        while (true) {
            if (*p == 0) {
                break;
            }
            p++;
            len++;
        }
        norm[i] = (u16 *)MEMAllocFromExpHeapEx(sPrcExpHeap, (len * 2 + 1) * 2, 4);
        if (norm[i] == NULL) {
            for (j = 0; j < i; j++) {
                MEMFreeToExpHeap(sPrcExpHeap, norm[j]);
            }
            MEMFreeToExpHeap(sPrcExpHeap, norm);
            return PRC_ERR_ALLOC;
        }
        prcNormalize(norm[i], words[i]);
    }
    *hits = 0;
    for (i = 0; i < num; i++) {
        results[i] = 0;
    }
    for (i = 0; i < sPrcPatternNum; i++) {
        RegDfa dfa;
        if (!dfa.load(sPrcPatterns[i])) {
            continue;
        }
        for (j = 0; j < num; j++) {
            int flag = results[j];
            if (flag != 1 && dfa.match(norm[j])) {
                (*hits)++;
                results[j] = 1;
            }
        }
    }
    for (i = 0; i < num; i++) {
        MEMFreeToExpHeap(sPrcExpHeap, norm[i]);
    }
    MEMFreeToExpHeap(sPrcExpHeap, norm);
    return PRC_OK;
}

// 801F86D8
void prcNormalize(u16 *dst, const u16 *src) {
    const u16 (*table)[2] = sPrcKanaTable;
    const u16 * current;
    u16 *out;
    int d = 0;
    int s = 0;
    while (true) {
        current = &src[s];
        u16 ch = *current;
        if (ch == 0x20 || ch == 0x3000) {
            s++;
            continue;
        }
        if (ch >= 0xFF41 && ch <= 0xFF5A) {
            dst[d++] = ch - 0xFEE0;
            s++;
        } else if (ch >= 0xFF21 && ch <= 0xFF3A) {
            dst[d++] = ch - 0xFEE0;
            s++;
        } else if (ch >= 0xFF10 && ch <= 0xFF19) {
            dst[d++] = ch - 0xFEE0;
            s++;
        } else if (ch >= 0x3041 && ch <= 0x3096) {
            dst[d] = ch + 0x60;
            prcToLargeKana(&dst[d]);
            s++;
            d++;
        } else if (ch >= 0x30A1 && ch <= 0x30FA) {
            dst[d] = ch;
            prcToLargeKana(&dst[d]);
            s++;
            d++;
        } else if (ch >= 0xFF66 && ch <= 0xFF9F) {
            if (current[1] == 0xFF9E) {
                if (prcConvertChar(&dst[d], ch, &table[70], 0x15)) {
                    s += 2;
                    d++;
                    continue;
                }
            } else if (current[1] == 0xFF9F) {
                if (prcConvertChar(&dst[d], ch, &table[91], 5)) {
                    s += 2;
                    d++;
                    continue;
                }
            }
            out = &dst[d];
            prcConvertChar(out, *current, &table[12], 0x3A);
            prcToLargeKana(out);
            s++;
            d++;
        } else {
            dst[d] = ch;
            if (ch == 0) {
                return;
            }
            s++;
            d++;
        }
    }
}

// 801F88BC
void prcToLargeKana(u16 *ch) {
    for (int i = 0; i < 12; i++) {
        if (*ch == sPrcKanaTable[i][0]) {
            *ch = sPrcKanaTable[i][1];
            return;
        }
    }
}

// 801F890C
bool prcConvertChar(u16 *dst, u16 ch, const u16 (*table)[2], int num) {
    for (int i = 0; i < num; i++) {
        if (ch == table[i][0]) {
            *dst = table[i][1];
            return true;
        }
    }
    return false;
}

// 801F8958
BOOL prcIsFatalError(int result) {
    return result <= PRC_ERR_ALREADY_SETUP;
}

} // namespace RegEad
