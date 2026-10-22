#include <private/enc.h>
#include <revolution/enc.h>

#include <string.h>

static ENCEncoding ENCiGetEncodingTypeFromHashTable(const struct encoding_hash_table_entry* table, int tableLength, const u8* encodingName);

static ENCResult ENCiConvertFromInternalEncoding(ENCContext* context, void* dst, s32* dstSize, const u16* src, s32* srcSize);

const struct encoding encoding_array[] = {
    {(0), FALSE, ""},
    {sizeof(char), TRUE, "US-ASCII"},
    {sizeof(u8), FALSE, "UTF-8"},
    {sizeof(u16), FALSE, "UTF-16BE"},
    {sizeof(u32), FALSE, "UTF-32BE"},
    {sizeof(u8), FALSE, "ISO-8859-1"},
    {sizeof(u8), FALSE, "ISO-8859-2"},
    {sizeof(u8), FALSE, "ISO-8859-3"},
    {sizeof(u8), FALSE, "ISO-8859-7"},
    {sizeof(u8), FALSE, "ISO-8859-10"},
    {sizeof(u8), FALSE, "ISO-8859-15"},
    {sizeof(char), TRUE, "ISO-2022-JP"},
    {sizeof(u8), FALSE, "Shift_JIS"},
    {sizeof(u8), FALSE, "windows-1252"},
    {sizeof(char), TRUE, "UTF-7"},
    {sizeof(u16), FALSE, "UTF-16"},
    {sizeof(u16), FALSE, "UTF-16LE"},
    {sizeof(u8), FALSE, "windows-1250"},
    {sizeof(u8), FALSE, "windows-1253"},
    {sizeof(u8), FALSE, "macintosh"},
    {sizeof(u8), FALSE, "x-mac-greek"},
    {sizeof(u8), FALSE, "x-mac-ce"},
    {sizeof(u8), FALSE, "IBM850"},
    {sizeof(u8), FALSE, "IBM852"},
};

const struct encoding_hash_table_entry encoding_table[] = {
    {"windows1252", ENC_ENCODING_WINDOWS_1252, 0},
    {"windows1253", ENC_ENCODING_WINDOWS_1253, 1},
    {"macroman", ENC_ENCODING_MACINTOSH, 34},
    {"macgreek", ENC_ENCODING_X_MAC_GREEK, 3},
    {"us", ENC_ENCODING_US_ASCII, 31},
    {"isolatin2", ENC_ENCODING_ISO_8859_2, 5},
    {"isolatin3", ENC_ENCODING_ISO_8859_3, 41},
    {"ansix341986", ENC_ENCODING_US_ASCII, 7},
    {"isoir6", ENC_ENCODING_US_ASCII, 8},
    {"isolatin6", ENC_ENCODING_ISO_8859_10, 9},
    {"unicode20utf8", ENC_ENCODING_UTF_8, 24},
    {"isoir100", ENC_ENCODING_ISO_8859_1, 11},
    {"isoir101", ENC_ENCODING_ISO_8859_2, 12},
    {"ibm850", ENC_ENCODING_IBM850, 37},
    {"iso88591windows31latin1", ENC_ENCODING_WINDOWS_1252, 14},
    {"ibm852", ENC_ENCODING_IBM852, 15},
    {"ibm367", ENC_ENCODING_US_ASCII, 16},
    {"unicode20", ENC_ENCODING_UTF_16BE, 29},
    {"ibm819", ENC_ENCODING_ISO_8859_1, 18},
    {"isoir126", ENC_ENCODING_ISO_8859_7, 19},
    {"isoir109", ENC_ENCODING_ISO_8859_3, 20},
    {"unicodeascii", ENC_ENCODING_UTF_16BE, 21},
    {"iso10646ucsbasic", ENC_ENCODING_UTF_16BE, 22},
    {"isoir157", ENC_ENCODING_ISO_8859_15, 23},
    {"iso10646unicodelatin1", ENC_ENCODING_UTF_16BE, 24},
    {"isolatingreek", ENC_ENCODING_ISO_8859_7, 25},
    {"pcp852", ENC_ENCODING_IBM852, 26},
    {"iso10646ucs4", ENC_ENCODING_UTF_32BE, 27},
    {"iso10646ucs2be", ENC_ENCODING_UTF_16BE, 28},
    {"l1", ENC_ENCODING_ISO_8859_1, 44},
    {"iso10646ucs4be", ENC_ENCODING_UTF_32BE, 30},
    {"isolatin1", ENC_ENCODING_ISO_8859_1, 31},
    {"greek8", ENC_ENCODING_ISO_8859_7, 32},
    {"elot928", ENC_ENCODING_ISO_8859_7, 33},
    {"l6", ENC_ENCODING_ISO_8859_10, 34},
    {"iso8859101992", ENC_ENCODING_ISO_8859_15, 35},
    {"mscp932", ENC_ENCODING_SHIFT_JIS, 36},
    {"iso88591windows30latin1", ENC_ENCODING_WINDOWS_1252, 37},
    {"iso10646ucs2le", ENC_ENCODING_UTF_16LE, 38},
    {"iso646us", ENC_ENCODING_US_ASCII, 39},
    {"unicode20utf7", ENC_ENCODING_UTF_7, 40},
    {"cp65000", ENC_ENCODING_UTF_7, 41},
    {"ucs2le", ENC_ENCODING_UTF_16LE, 42},
    {"windows31latin2", ENC_ENCODING_WINDOWS_1250, 43},
    {"850", ENC_ENCODING_IBM850, 44},
    {"utf8n", ENC_ENCODING_UTF_8, 45},
    {"852", ENC_ENCODING_IBM852, 46},
    {"", ENC_ENCODING_NONE, 47},
    {"", ENC_ENCODING_NONE, 48},
    {"macce", ENC_ENCODING_X_MAC_CE, 49},
    {"", ENC_ENCODING_NONE, 50},
    {"ibm1200", ENC_ENCODING_UTF_16, 51},
    {"", ENC_ENCODING_NONE, 52},
    {"", ENC_ENCODING_NONE, 53},
    {"macintosh", ENC_ENCODING_MACINTOSH, 54},
    {"", ENC_ENCODING_NONE, 55},
    {"", ENC_ENCODING_NONE, 56},
    {"iso10646ucs2", ENC_ENCODING_UTF_16BE, 57},
    {"", ENC_ENCODING_NONE, 58},
    {"iso10646j1", ENC_ENCODING_UTF_16BE, 27},
    {"", ENC_ENCODING_NONE, 60},
    {"", ENC_ENCODING_NONE, 61},
    {"", ENC_ENCODING_NONE, 62},
    {"iso2022jp", ENC_ENCODING_ISO_2022_JP, 63},
    {"", ENC_ENCODING_NONE, 64},
    {"ascii", ENC_ENCODING_US_ASCII, 22},
    {"", ENC_ENCODING_NONE, 66},
    {"windows1250", ENC_ENCODING_WINDOWS_1250, 67},
    {"", ENC_ENCODING_NONE, 68},
    {"usascii", ENC_ENCODING_US_ASCII, 0},
    {"greek", ENC_ENCODING_ISO_8859_7, 1},
    {"", ENC_ENCODING_NONE, 71},
    {"", ENC_ENCODING_NONE, 72},
    {"windows31j", ENC_ENCODING_SHIFT_JIS, 73},
    {"", ENC_ENCODING_NONE, 74},
    {"pc850multilingual", ENC_ENCODING_IBM850, 75},
    {"", ENC_ENCODING_NONE, 76},
    {"mac", ENC_ENCODING_MACINTOSH, 77},
    {"", ENC_ENCODING_NONE, 78},
    {"", ENC_ENCODING_NONE, 79},
    {"", ENC_ENCODING_NONE, 80},
    {"", ENC_ENCODING_NONE, 81},
    {"", ENC_ENCODING_NONE, 82},
    {"", ENC_ENCODING_NONE, 83},
    {"", ENC_ENCODING_NONE, 84},
    {"", ENC_ENCODING_NONE, 85},
    {"", ENC_ENCODING_NONE, 86},
    {"", ENC_ENCODING_NONE, 87},
    {"", ENC_ENCODING_NONE, 88},
    {"", ENC_ENCODING_NONE, 89},
    {"", ENC_ENCODING_NONE, 90},
    {"unicode11utf7", ENC_ENCODING_UTF_7, 40},
    {"unicode11utf8", ENC_ENCODING_UTF_8, 10},
    {"", ENC_ENCODING_NONE, 93},
    {"", ENC_ENCODING_NONE, 94},
    {"windows30latin1", ENC_ENCODING_WINDOWS_1252, 95},
    {"windows31latin1", ENC_ENCODING_WINDOWS_1252, 96},
    {"mskanji ", ENC_ENCODING_SHIFT_JIS, 43},
    {"", ENC_ENCODING_NONE, 98},
    {"", ENC_ENCODING_NONE, 99},
    {"", ENC_ENCODING_NONE, 100},
    {"", ENC_ENCODING_NONE, 101},
    {"", ENC_ENCODING_NONE, 102},
    {"", ENC_ENCODING_NONE, 103},
    {"ecma118", ENC_ENCODING_ISO_8859_7, 104},
    {"", ENC_ENCODING_NONE, 105},
    {"", ENC_ENCODING_NONE, 106},
    {"", ENC_ENCODING_NONE, 107},
    {"", ENC_ENCODING_NONE, 108},
    {"", ENC_ENCODING_NONE, 109},
    {"", ENC_ENCODING_NONE, 110},
    {"", ENC_ENCODING_NONE, 111},
    {"iso2022jp1", ENC_ENCODING_ISO_2022_JP, 112},
    {"iso2022jp2", ENC_ENCODING_ISO_2022_JP, 113},
    {"", ENC_ENCODING_NONE, 114},
    {"", ENC_ENCODING_NONE, 115},
    {"", ENC_ENCODING_NONE, 116},
    {"", ENC_ENCODING_NONE, 117},
    {"", ENC_ENCODING_NONE, 118},
    {"", ENC_ENCODING_NONE, 119},
    {"", ENC_ENCODING_NONE, 120},
    {"", ENC_ENCODING_NONE, 121},
    {"", ENC_ENCODING_NONE, 122},
    {"", ENC_ENCODING_NONE, 123},
    {"ucs2be", ENC_ENCODING_UTF_16BE, 21},
    {"", ENC_ENCODING_NONE, 125},
    {"ucs4be", ENC_ENCODING_UTF_32BE, 32},
    {"", ENC_ENCODING_NONE, 127},
    {"iso646irv1991", ENC_ENCODING_US_ASCII, 128},
    {"latin1", ENC_ENCODING_ISO_8859_1, 129},
    {"latin2", ENC_ENCODING_ISO_8859_2, 130},
    {"latin3", ENC_ENCODING_ISO_8859_3, 131},
    {"iso10646", ENC_ENCODING_UTF_16BE, 132},
    {"", ENC_ENCODING_NONE, 133},
    {"latin6", ENC_ENCODING_ISO_8859_10, 42},
    {"iso885911987", ENC_ENCODING_ISO_8859_1, 135},
    {"iso885921987", ENC_ENCODING_ISO_8859_2, 136},
    {"latin9", ENC_ENCODING_ISO_8859_15, 36},
    {"iso885931988", ENC_ENCODING_ISO_8859_3, 138},
    {"unicode20utf16", ENC_ENCODING_UTF_16BE, 139},
    {"cp850", ENC_ENCODING_IBM850, 140},
    {"iso885971987", ENC_ENCODING_ISO_8859_7, 141},
    {"cp852", ENC_ENCODING_IBM852, 142},
    {"cp367", ENC_ENCODING_US_ASCII, 33},
    {"iso88592windowslatin2", ENC_ENCODING_WINDOWS_1250, 144},
    {"cp819", ENC_ENCODING_ISO_8859_1, 145},
    {"iso88591", ENC_ENCODING_ISO_8859_1, 146},
    {"iso88592", ENC_ENCODING_ISO_8859_2, 3},
    {"iso88593", ENC_ENCODING_ISO_8859_3, 8},
    {"", ENC_ENCODING_NONE, 149},
    {"", ENC_ENCODING_NONE, 150},
    {"", ENC_ENCODING_NONE, 151},
    {"iso88597", ENC_ENCODING_ISO_8859_7, 152},
    {"ucs2", ENC_ENCODING_UTF_16BE, 153},
    {"", ENC_ENCODING_NONE, 154},
    {"ucs4", ENC_ENCODING_UTF_32BE, 155},
    {"", ENC_ENCODING_NONE, 156},
    {"unicode11", ENC_ENCODING_UTF_16BE, 17},
    {"l2", ENC_ENCODING_ISO_8859_2, 158},
    {"l3", ENC_ENCODING_ISO_8859_3, 46},
    {"", ENC_ENCODING_NONE, 160},
    {"", ENC_ENCODING_NONE, 161},
    {"utf7", ENC_ENCODING_UTF_7, 2},
    {"utf8", ENC_ENCODING_UTF_8, 163},
    {"", ENC_ENCODING_NONE, 164},
    {"", ENC_ENCODING_NONE, 165},
    {"", ENC_ENCODING_NONE, 166},
    {"", ENC_ENCODING_NONE, 167},
    {"", ENC_ENCODING_NONE, 168},
    {"", ENC_ENCODING_NONE, 169},
    {"", ENC_ENCODING_NONE, 170},
    {"", ENC_ENCODING_NONE, 171},
    {"", ENC_ENCODING_NONE, 172},
    {"", ENC_ENCODING_NONE, 173},
    {"suneugreek", ENC_ENCODING_ISO_8859_7, 174},
    {"", ENC_ENCODING_NONE, 175},
    {"", ENC_ENCODING_NONE, 176},
    {"", ENC_ENCODING_NONE, 177},
    {"cp1200", ENC_ENCODING_UTF_16, 178},
    {"utf32be", ENC_ENCODING_UTF_32BE, 35},
    {"", ENC_ENCODING_NONE, 180},
    {"utf16be", ENC_ENCODING_UTF_16BE, 181},
    {"ansix341968", ENC_ENCODING_US_ASCII, 7},
    {"cp1250", ENC_ENCODING_WINDOWS_1250, 183},
    {"shiftjis", ENC_ENCODING_SHIFT_JIS, 184},
    {"cp1252", ENC_ENCODING_WINDOWS_1252, 185},
    {"cp1253", ENC_ENCODING_WINDOWS_1253, 186},
    {"", ENC_ENCODING_NONE, 187},
    {"unicodelatin1", ENC_ENCODING_UTF_16BE, 188},
    {"", ENC_ENCODING_NONE, 189},
    {"", ENC_ENCODING_NONE, 190},
    {"utf16le", ENC_ENCODING_UTF_16LE, 191},
    {"", ENC_ENCODING_NONE, 192},
    {"", ENC_ENCODING_NONE, 193},
    {"iso885910", ENC_ENCODING_ISO_8859_10, 194},
    {"", ENC_ENCODING_NONE, 195},
    {"", ENC_ENCODING_NONE, 196},
    {"", ENC_ENCODING_NONE, 197},
    {"", ENC_ENCODING_NONE, 198},
    {"iso885915", ENC_ENCODING_ISO_8859_15, 199},
    {"", ENC_ENCODING_NONE, 200},
    {"", ENC_ENCODING_NONE, 201},
    {"", ENC_ENCODING_NONE, 202},
    {"", ENC_ENCODING_NONE, 203},
    {"", ENC_ENCODING_NONE, 204},
    {"", ENC_ENCODING_NONE, 205},
    {"", ENC_ENCODING_NONE, 206},
    {"", ENC_ENCODING_NONE, 207},
    {"utf32", ENC_ENCODING_UTF_32BE, 208},
    {"", ENC_ENCODING_NONE, 209},
    {"utf16", ENC_ENCODING_UTF_16, 210},
    {"", ENC_ENCODING_NONE, 211},
    {"", ENC_ENCODING_NONE, 212},
    {"sjis", ENC_ENCODING_SHIFT_JIS, 213},
    {"", ENC_ENCODING_NONE, 214},
    {"", ENC_ENCODING_NONE, 215},
    {"", ENC_ENCODING_NONE, 216},
    {"", ENC_ENCODING_NONE, 217},
    {"", ENC_ENCODING_NONE, 218},
    {"", ENC_ENCODING_NONE, 219},
    {"", ENC_ENCODING_NONE, 220},
    {"", ENC_ENCODING_NONE, 221},
    {"", ENC_ENCODING_NONE, 222},
    {"", ENC_ENCODING_NONE, 223},
    {"", ENC_ENCODING_NONE, 224},
    {"", ENC_ENCODING_NONE, 225},
    {"", ENC_ENCODING_NONE, 226},
    {"", ENC_ENCODING_NONE, 227},
};

ENCResult ENCInitContext(ENCContext* context) {
    ENCiRegisterVersion();

    if (context == NULL) {
        return ENC_ERR_INVALID_PARAM;
    }

    context->encoding = ENC_ENCODING_NONE;
    context->brtype = ENC_BR_KEEP;
    context->state = ENC_STATE_INITIAL;
    context->nomap = L'\0';
    context->invalid = L'\0';

    return ENC_OK;
}

ENCResult ENCSetExternalEncoding(ENCContext* context, const char* name) {
    if (context == NULL || name == NULL) {
        return ENC_ERR_INVALID_PARAM;
    }

    context->encoding = ENCiGetEncodingTypeFromHashTable(encoding_table, ARRAY_LENGTH(encoding_table), (const u8*)name);

    if (context->encoding == ENC_ENCODING_NONE) {
        return ENC_ERR_UNKNOWN_ENCODING;
    }

    if (!enc_tbl_jp_loaded && (context->encoding == ENC_ENCODING_SHIFT_JIS || context->encoding == ENC_ENCODING_ISO_2022_JP)) {
        return ENC_ERR_NOT_LOADED;
    }

    if (context->encoding == ENC_ENCODING_UTF_16BE || context->encoding == ENC_ENCODING_UTF_16) {
        context->state = ENC_UTF16_STATE_BIG_ENDIAN;
    } else if (context->encoding == ENC_ENCODING_UTF_16LE) {
        context->state = ENC_UTF16_STATE_LITTLE_ENDIAN;
    } else if (context->encoding == ENC_ENCODING_ISO_2022_JP) {
        context->state = ENC_JIS_STATE_US_ASCII;
    } else {
        context->state = ENC_STATE_INITIAL;
    }

    return ENC_OK;
}

ENCResult ENCSetBreakType(ENCContext* context, ENCBreakType breakType) {
    if (context == NULL || breakType < ENC_BR_KEEP || breakType > ENC_BR_LF) {
        return ENC_ERR_INVALID_PARAM;
    }

    context->brtype = breakType;

    return ENC_OK;
}

ENCResult ENCSetAlternativeCharacter(ENCContext* context, u16 nomap, u16 invalid) {
    if (context == NULL) {
        return ENC_ERR_INVALID_PARAM;
    }

    context->nomap = nomap;
    context->invalid = invalid;

    return ENC_OK;
}

ENCResult ENCConvertFromInternalEncoding(ENCContext* context, void* dst, s32* dstSize, const u16* src, s32* srcSize) {
    ENCResult ret;

    u32 width;

    s32 tmpDstSize;
    s32 tmpSrcSize;
    s32 dstCnt;
    s32 srcCnt;

    if (context == NULL || context->brtype < ENC_BR_KEEP || context->brtype > ENC_BR_LF) {
        return ENC_ERR_INVALID_PARAM;
    }

    if (context->encoding <= ENC_ENCODING_NONE || context->encoding >= ENC_BAD_MAX_ENCODING) {
        return ENC_ERR_UNKNOWN_ENCODING;
    }

    // first encoding not convertible to "internal encoding" from
    if (context->encoding >= ENC_ENCODING_UTF_7) {
        return ENC_ERR_UNSUPPORTED;
    }

    if (srcSize == NULL) {
        return ENC_ERR_INVALID_PARAM;
    }

    width = encoding_array[context->encoding].width;

    if (dst != NULL && *dstSize > 0) {
        *dstSize /= width;
    }

    if (*srcSize > 0) {
        *srcSize /= sizeof(*src);
    }

    tmpDstSize = *dstSize;
    tmpSrcSize = *srcSize;
    dstCnt = 0;
    srcCnt = 0;

    ret = ENCiConvertFromInternalEncoding(context, dst, &tmpDstSize, src, &tmpSrcSize);

    while ((ret == ENC_ERR_NO_MAP_RULE && context->nomap != L'\0') || (ret == ENC_ERR_INVALID_FORMAT && context->invalid != L'\0')) {
        u16 replacement = L'\0';

        dstCnt += tmpDstSize;
        srcCnt += tmpSrcSize;

        tmpDstSize = *dstSize - dstCnt;
        srcCnt++;
        tmpSrcSize = *srcSize > 0 ? *srcSize - srcCnt : *srcSize;

        if (ret == ENC_ERR_INVALID_FORMAT && context->invalid != L'\uffff') {
            replacement = context->invalid;
        } else if (ret == ENC_ERR_NO_MAP_RULE && context->nomap != L'\uffff') {
            replacement = context->nomap;
        }

        if (replacement) {
            s32 replacementSize = sizeof(u8);

            if (dst) {
                ret = ENCiConvertFromInternalEncoding(context, (u8*)dst + dstCnt, &tmpDstSize, &replacement, &replacementSize);
            } else {
                ret = ENCiConvertFromInternalEncoding(context, NULL, &tmpDstSize, &replacement, &replacementSize);
            }

            if (ret != ENC_OK) {
                break;
            }

            dstCnt += tmpDstSize;
            tmpDstSize = *dstSize - dstCnt;
        }

        if (dst) {
            ret = ENCiConvertFromInternalEncoding(context, (u8*)dst + dstCnt, &tmpDstSize, src + srcCnt, &tmpSrcSize);
        } else {
            ret = ENCiConvertFromInternalEncoding(context, NULL, &tmpDstSize, src + srcCnt, &tmpSrcSize);
        }
    }

    *dstSize = dstCnt + tmpDstSize;
    *srcSize = srcCnt + tmpSrcSize;

    if (*dstSize > 0) {
        *dstSize *= width;
    }

    if (srcSize != NULL && *srcSize > 0) {
        *srcSize *= sizeof(*src);
    }

    return ret;
}

ENCResult ENCCheckEncoding(int* lastIndex, const char* names[], int num, const u16* src, u32 srcSize) {
    int i;

    ENCEncoding encodings[ENC_CHECK_ENCODING_MAX_NUM];
    s32 dstSize = ENC_DST_INVALID;

    ENCiRegisterVersion();

    if (lastIndex == NULL || names == NULL || num <= 0 || num > ENC_CHECK_ENCODING_MAX_NUM - 1 || src == NULL) {
        return ENC_ERR_INVALID_PARAM;
    }

    for (i = 0; i < num; i++) {
        const char* encodingName = names[i];
        ENCEncoding encoding;
        *lastIndex = i;

        if (encodingName == NULL) {
            return ENC_ERR_INVALID_PARAM;
        }

        encoding = ENCiGetEncodingTypeFromHashTable(encoding_table, ARRAY_LENGTH(encoding_table), (const u8*)encodingName);

        if (encoding == ENC_ENCODING_NONE) {
            return ENC_ERR_UNKNOWN_ENCODING;
        }

        if (encoding >= ENC_BAD_MAX_ENCODING) {
            return ENC_ERR_UNSUPPORTED;
        }

        if (!enc_tbl_jp_loaded && (encoding == ENC_ENCODING_SHIFT_JIS || encoding == ENC_ENCODING_ISO_2022_JP)) {
            return ENC_ERR_NOT_LOADED;
        }

        encodings[i] = encoding;
    }

    {
        ENCContext context;
        ENCInitContext(&context);

        for (i = 0; i < num; i++) {
            s32 tmpSrcSize = srcSize / sizeof(*src);

            *lastIndex = i;
            context.encoding = encodings[i];

            {
                ENCResult ret = ENCiConvertFromInternalEncoding(&context, NULL, &dstSize, src, &tmpSrcSize);

                if (ret == ENC_OK) {
                    return ENC_OK;
                }
            }
        }
    }

    *lastIndex = ENC_CHECK_ENCODING_NOT_FOUND;
    return ENC_OK;
}

ENCResult ENCIs7BitEncoding(BOOL* is7Bit, const char* encodingName) {
    ENCEncoding encoding;
    ENCiRegisterVersion();

    if (is7Bit == NULL || encodingName == NULL) {
        return ENC_ERR_INVALID_PARAM;
    }

    encoding = ENCiGetEncodingTypeFromHashTable(encoding_table, ARRAY_LENGTH(encoding_table), (const u8*)encodingName);

    *is7Bit = encoding_array[encoding].is7bit;

    if (encoding == ENC_ENCODING_NONE) {
        return ENC_ERR_UNKNOWN_ENCODING;
    }

    return ENC_OK;
}

static ENCEncoding ENCiGetEncodingTypeFromHashTable(const struct encoding_hash_table_entry* table, int tableLength, const u8* encodingName) {
    int i;

    u8 stripped[ENCODING_HASH_TABLE_ENTRY_NAME_LENGTH];
    int hash = 0;
    s32 cnt = 0;

    // /^(x-|cs)/i
    if (((encodingName[0] == 'x' || encodingName[0] == 'X') && encodingName[1] == '-') ||
        ((encodingName[0] == 'c' || encodingName[0] == 'C') && (encodingName[1] == 's' || encodingName[1] == 'S'))) {
        i = 2;  // skip them
    } else {
        i = 0;
    }

    while (cnt < ENCODING_HASH_TABLE_ENTRY_NAME_LENGTH) {
        /* Not sure this follows UTS #22 1.4 exactly; I think this code doesn't
         * strip leading zeroes of digit sequences.
         */
        u8 cc = encodingName[i];

        if (cc == '\0') {
            stripped[cnt++] = 0x00;
            break;
        }

        if ((cc >= '0' && cc < '9' + 1) || (cc > 'a' - 1 && cc <= 'z')) {
            stripped[cnt++] = cc;
        } else if (cc > 'A' - 1 && cc <= 'Z') {
            stripped[cnt++] = cc + 0x20u;  // ascii-tolower
        }

        i++;
    }

    for (i = 0; i < cnt; i++) {
        hash += stripped[i];
    }

    {
        s32 index;
        for (index = hash % tableLength; strncmp((char*)stripped, table[index].name, ENCODING_HASH_TABLE_ENTRY_NAME_LENGTH) != 0;
             index = table[index].nextIndex) {
            if (index == table[index].nextIndex) {
                return ENC_ENCODING_NONE;
            }
        }

        return table[index].encoding;
    }
}

static ENCResult ENCiConvertFromInternalEncoding(ENCContext* context, void* dst, s32* dstSize, const u16* src, s32* srcSize) {
    switch (context->encoding) {
        case ENC_ENCODING_NONE:
            return ENC_ERR_UNKNOWN_ENCODING;

        case ENC_ENCODING_US_ASCII: {
            return ENCiConvertStringUnicodeToAscii(dst, dstSize, src, srcSize, context->brtype);
        }
        case ENC_ENCODING_UTF_8: {
            return ENCiConvertStringUtf16ToUtf8(dst, dstSize, src, srcSize, context->brtype);
        }
        case ENC_ENCODING_UTF_16BE: {
            return ENCiConvertStringUtf16ToUtf16(dst, dstSize, src, srcSize, context->brtype, (ENCMBState*)context->state);
        }
        case ENC_ENCODING_UTF_32BE: {
            return ENCiConvertStringUtf16ToUtf32(dst, dstSize, src, srcSize, context->brtype);
        }
        case ENC_ENCODING_ISO_8859_1: {
            return ENCiConvertStringUnicodeToLatin1(dst, dstSize, src, srcSize, context->brtype);
        }
        case ENC_ENCODING_ISO_8859_2: {
            return ENCiConvertStringUnicodeToLatin2(dst, dstSize, src, srcSize, context->brtype);
        }
        case ENC_ENCODING_ISO_8859_3: {
            return ENCiConvertStringUnicodeToLatin3(dst, dstSize, src, srcSize, context->brtype);
        }
        case ENC_ENCODING_ISO_8859_7: {
            return ENCiConvertStringUnicodeToGreek(dst, dstSize, src, srcSize, context->brtype);
        }
        case ENC_ENCODING_ISO_8859_10: {
            return ENCiConvertStringUnicodeToLatin6(dst, dstSize, src, srcSize, context->brtype);
        }
        case ENC_ENCODING_ISO_8859_15: {
            return ENCiConvertStringUnicodeToLatin9(dst, dstSize, src, srcSize, context->brtype);
        }
        case ENC_ENCODING_ISO_2022_JP: {
            return ENCiConvertStringUnicodeToJis(dst, dstSize, src, srcSize, context->brtype, &context->state);
        }
        case ENC_ENCODING_SHIFT_JIS: {
            return ENCiConvertStringUnicodeToSjis(dst, dstSize, src, srcSize, context->brtype);
        }
        case ENC_ENCODING_WINDOWS_1252: {
            return ENCiConvertStringUnicodeToWin1252(dst, dstSize, src, srcSize, context->brtype);
        }
        default: {
            return ENC_ERR_UNSUPPORTED;
        }
    }
}
