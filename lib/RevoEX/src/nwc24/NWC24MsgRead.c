#include <private/nwc24.h>
#include <revolution/nwc24.h>
#include <revolution/enc.h>

#include <string.h>

static NWC24Err ReadBase64Data(NWC24File* pFile, const NWC24Data* pData, u8* pDst, u32 dstSize, u32* pRead);

static NWC24Err SelectMBox(const NWC24iMsgObj* pMsg, NWC24MBoxType* pBox) {
    if (pMsg->flags & NWC24_MSGOBJ_TO_SEND) {
        *pBox = NWC24_MBOX_TYPE_SEND;
    } else if (pMsg->flags & NWC24_MSGOBJ_TO_RECV) {
        *pBox = NWC24_MBOX_TYPE_RECV;
    } else {
        return NWC24_ERR_INVALID_VALUE;
    }

    return NWC24_OK;
}

NWC24Err NWC24ReadMsgAttached(const NWC24MsgObj* pMsg, u32 index, u8* pDst, u32 size) {
    NWC24iMsgObj* pMsgImpl = (NWC24iMsgObj*)pMsg;
    NWC24File file;
    NWC24Err result;
    NWC24Err closeResult;
    NWC24MBoxType box;
    u32 readSize;
    NWC24Data* pAttached;

    if (!NWC24IsMsgLibOpened() && !NWC24IsMsgLibOpenedByTool()) {
        return NWC24_ERR_LIB_NOT_OPENED;
    }

    if (!(pMsgImpl->flags & (1 << 9))) {
        return NWC24_ERR_PROTECTED;
    }

    if (index >= pMsgImpl->numAttached) {
        return NWC24_ERR_NOT_FOUND;
    }

    result = SelectMBox(pMsgImpl, &box);
    if (result != NWC24_OK) {
        return result;
    }

    pAttached = &pMsgImpl->attached[index];
    if (pAttached->size == 0 || pMsgImpl->attachedSize[index] == 0) {
        return NWC24_ERR_NULL;
    }

    result = NWC24iMBoxOpenStoredMsg(box, pMsgImpl->id, &file);
    if (result != NWC24_OK) {
        return result;
    }

    readSize = 0;
    result = ReadBase64Data(&file, pAttached, pDst, size, &readSize);
    if (result == NWC24_OK && readSize != pMsgImpl->attachedSize[index]) {
        result = NWC24_ERR_FORMAT;
    }

    closeResult = NWC24iMBoxCloseMsg(&file);
    if (result == NWC24_OK) {
        result = closeResult;
    }

    return result;
}

static NWC24Err ReadBase64Data(NWC24File* pFile, const NWC24Data* pData, u8* pDst, u32 dstSize, u32* pRead) {
    NWC24Err result;
    s32 chunk;
    u32 decoded;
    s32 i;
    s32 offset = pData->offset;
    char* pBuf = (char*)NWC24WorkP + 0x400;
    s32 remain = pData->size;
    BOOL done = FALSE;
    s32 lineEnd;
    s32 total = 0;

    result = NWC24_OK;
    while (remain > 0 && !done) {
        chunk = remain > 0x400 ? 0x400 : remain;

        NWC24FSeek(pFile, offset, NWC24_SEEK_BEG);
        result = NWC24FRead(pBuf, chunk, pFile);
        if (result != NWC24_OK) {
            break;
        }

        lineEnd = 0;
        for (i = 0; i < chunk; i++) {
            char c = pBuf[i];
            if (c == '\r') {
                lineEnd = i;
            }

            if (c == '=') {
                lineEnd = i + 1;
                done = TRUE;
                result = NWC24_OK;
                break;
            }
        }

        if (lineEnd == 0) {
            break;
        }

        result = NWC24Base64Decode(pBuf, lineEnd, pDst, dstSize - total, &decoded);
        if (result != NWC24_OK) {
            break;
        }

        offset += lineEnd;
        remain -= lineEnd;
        total += decoded;
        pDst += decoded;
    }

    if (result == NWC24_OK) {
        *pRead = total;
    }

    return result;
}

static const char* sCharsetsJpn[] = {"us-ascii", "iso-2022-jp", "utf-8"};
static const char* sCharsetsOther[] = {"us-ascii", "iso-8859-1", "windows-1252", "iso-8859-2", "iso-8859-7", "iso-8859-10", "iso-8859-3", "utf-8"};

static NWC24Err NWC24iGetDefaultCharset(char* pDst, u32 size, NWC24EncodingRegion region);
static NWC24Err NWC24iConvertFromInternalEncoding(char* pDst, u32* pDstSize, const u16* pSrc, u32* pSrcLen, char* pName, u32 nameMax,
                                            NWC24EncodingRegion region, u16 altChar);
static NWC24Err NWC24iDetectEncodingToSend(char* pDst, u32 size, const u16* pText, u32 textLen, const u16* pText2, u32 textLen2,
                                     NWC24EncodingRegion region);
static NWC24Err NWC24iDetectBreakPoint(u32* pOut, NWC24Charset charset, const u8* pStr, u32 len, u32 maxLen);
static NWC24Err NWC24iSetMsgSubjectPlain(NWC24MsgObj* pMsg, const u16* pSubject, u32 subjectLen, NWC24EncodingRegion region, u16 altChar, u8* pWork,
                                   u32 workSize, NWC24Charset charset, const char* pName);
static NWC24Err NWC24iSetMsgSubjectQP(NWC24MsgObj* pMsg, const u16* pSubject, u32 subjectLen, NWC24EncodingRegion region, u16 altChar, u8* pWork,
                                u32 workSize, NWC24Charset charset, const char* pName);
static NWC24Err NWC24iSetMsgSubjectBase64(NWC24MsgObj* pMsg, const u16* pSubject, u32 subjectLen, NWC24EncodingRegion region, u16 altChar, u8* pWork,
                                    u32 workSize, NWC24Charset charset, const char* pName);

static NWC24Encoding GetEncodingForRegion(NWC24EncodingRegion region) {
    switch (region) {
    case NWC24_ENCODING_REGION_JPN:
        return NWC24_ENC_BASE64;
    case NWC24_ENCODING_REGION_USA:
    case NWC24_ENCODING_REGION_EUR:
        return NWC24_ENC_QUOTED_PRINTABLE;
    default:
        return NWC24_ENC_BASE64;
    }
}

NWC24Err NWC24SetMsgSubjectPublic(NWC24MsgObj* pMsg, const u16* pSubject, u32 subjectLen, NWC24EncodingRegion region, u16 altChar, u8* pWork,
                                  u32 workSize) {
    NWC24iMsgObj* pMsgImpl = (NWC24iMsgObj*)pMsg;
    NWC24Charset charset;
    char* pName;

    if (!(pMsgImpl->flags & NWC24_MSGOBJ_INITIALIZED) || (pMsgImpl->flags & NWC24_MSGOBJ_DELIVERING)) {
        return NWC24_ERR_PROTECTED;
    }

    if (pSubject == NULL || pSubject[0] == 0) {
        return NWC24_ERR_NULL;
    }

    if (pSubject[subjectLen] != 0) {
        return NWC24_ERR_STRING_END;
    }

    if (pWork == NULL) {
        return NWC24_ERR_INVALID_VALUE;
    }

    pName = (char*)NWC24WorkP;
    {
        NWC24Err result = NWC24iDetectEncodingToSend(pName, 0x40, pSubject, subjectLen, NULL, 0, region);
        if (result != NWC24_OK) {
            return result;
        }
    }

    if (NWC24ParseCharsetStr(&charset, pName) != NWC24_OK) {
        charset = NWC24_US_ASCII;
    }

    if (charset == NWC24_US_ASCII) {
        return NWC24iSetMsgSubjectPlain(pMsg, pSubject, subjectLen, region, altChar, pWork, workSize, charset, pName);
    }

    if (GetEncodingForRegion(region) == NWC24_ENC_QUOTED_PRINTABLE) {
        return NWC24iSetMsgSubjectQP(pMsg, pSubject, subjectLen, region, altChar, pWork, workSize, charset, pName);
    }

    return NWC24iSetMsgSubjectBase64(pMsg, pSubject, subjectLen, region, altChar, pWork, workSize, charset, pName);
}

NWC24Err NWC24SetMsgSubjectAndTextPublic(NWC24MsgObj* pMsg, const u16* pSubject, u32 subjectLen, const u16* pText, u32 textLen,
                                         NWC24EncodingRegion region, u16 altChar, u8* pWork, u32 workSize) {
    NWC24iMsgObj* pMsgImpl = (NWC24iMsgObj*)pMsg;
    BOOL is7Bit;
    NWC24Encoding encoding;
    u32 subjWorkSize;
    u8* pSubjWork;
    u32 textBytes;
    char* pName;
    NWC24Err result;
    u32 textConv;
    NWC24Charset charset;

    if (!(pMsgImpl->flags & NWC24_MSGOBJ_INITIALIZED) || (pMsgImpl->flags & NWC24_MSGOBJ_DELIVERING)) {
        return NWC24_ERR_PROTECTED;
    }

    if (pSubject == NULL || pSubject[0] == 0) {
        return NWC24_ERR_NULL;
    }

    if (pText == NULL || textLen == 0) {
        NWC24Data_Init(&pMsgImpl->text);
        return NWC24SetMsgSubjectPublic(pMsg, pSubject, subjectLen, region, altChar, pWork, workSize);
    }

    if (pText[textLen] != 0 || pSubject[subjectLen] != 0) {
        return NWC24_ERR_STRING_END;
    }

    if (pWork == NULL) {
        return NWC24_ERR_INVALID_VALUE;
    }

    pName = (char*)NWC24WorkP;
    result = NWC24iDetectEncodingToSend(pName, 0x40, pSubject, subjectLen, pText, textLen, region);
    if (result != NWC24_OK) {
        return result;
    }

    if (NWC24ParseCharsetStr(&charset, pName) != NWC24_OK) {
        charset = NWC24_US_ASCII;
    }

    textBytes = workSize * textLen / (textLen + subjectLen * 4);
    textConv = textLen;
    subjWorkSize = workSize - textBytes;
    pSubjWork = pWork + textBytes;

    result = NWC24iConvertFromInternalEncoding((char*)pWork, &textBytes, pText, &textConv, pName, 0x40, region, altChar);
    if (result != NWC24_OK) {
        return result;
    }

    result = ENCIs7BitEncoding(&is7Bit, pName);
    if (!is7Bit || result != ENC_OK) {
        encoding = GetEncodingForRegion(region);
    } else {
        encoding = NWC24_ENC_7BIT;
    }

    result = NWC24SetMsgText(pMsg, (const char*)pWork, textBytes - 1, charset, encoding);
    if (result != NWC24_OK) {
        return result;
    }

    if (charset == NWC24_US_ASCII) {
        return NWC24iSetMsgSubjectPlain(pMsg, pSubject, subjectLen, region, altChar, pSubjWork, subjWorkSize, charset, pName);
    }

    if (GetEncodingForRegion(region) == NWC24_ENC_QUOTED_PRINTABLE) {
        return NWC24iSetMsgSubjectQP(pMsg, pSubject, subjectLen, region, altChar, pSubjWork, subjWorkSize, charset, pName);
    }

    return NWC24iSetMsgSubjectBase64(pMsg, pSubject, subjectLen, region, altChar, pSubjWork, subjWorkSize, charset, pName);
}

static NWC24Err NWC24iGetDefaultCharset(char* pDst, u32 size, NWC24EncodingRegion region) {
    NWC24Err result = NWC24_OK;

    switch (region) {
    case NWC24_ENCODING_REGION_JPN:
        strncpy(pDst, "iso-2022-jp", size - 1);
        pDst[size - 1] = '\0';
        break;
    case NWC24_ENCODING_REGION_USA:
        strncpy(pDst, "iso-8859-1", size - 1);
        pDst[size - 1] = '\0';
        break;
    case NWC24_ENCODING_REGION_EUR:
        strncpy(pDst, "iso-8859-1", size - 1);
        pDst[size - 1] = '\0';
        break;
    default:
        strncpy(pDst, "iso-8859-1", size - 1);
        pDst[size - 1] = '\0';
        result = NWC24_ERR_INVALID_VALUE;
        break;
    }

    return result;
}

static NWC24Err NWC24iConvertFromInternalEncoding(char* pDst, u32* pDstSize, const u16* pSrc, u32* pSrcLen, char* pName, u32 nameMax,
                                            NWC24EncodingRegion region, u16 altChar) {
    ENCContext ctx;
    ENCResult encResult;
    u32 dstSize = *pDstSize;
    s32 dstLen;
    s32 srcBytes;

    *pDstSize = 0;
    if (dstSize < 1) {
        return NWC24_ERR_OVERFLOW;
    }

    if (pName[0] == '\0') {
        NWC24Err result = NWC24iGetDefaultCharset(pName, nameMax, region);
        if (result != NWC24_OK) {
            return result;
        }
    }

    encResult = ENCInitContext(&ctx);
    if (encResult == ENC_OK) {
        encResult = ENCSetExternalEncoding(&ctx, pName);
        if (encResult == ENC_OK) {
            encResult = ENCSetBreakType(&ctx, ENC_BR_CRLF);
            if (encResult == ENC_OK) {
                if (altChar == 0 || (encResult = ENCSetAlternativeCharacter(&ctx, altChar, altChar)) == ENC_OK) {
                    dstLen = dstSize - 1;
                    srcBytes = *pSrcLen * 2;
                    encResult = ENCConvertFromInternalEncoding(&ctx, pDst, &dstLen, pSrc, &srcBytes);
                    pDst[dstLen] = '\0';
                    *pSrcLen = (u32)srcBytes / 2;
                    *pDstSize = dstLen + 1;
                }
            }
        }
    }

    switch (encResult) {
    case ENC_OK:
        return NWC24_OK;
    case ENC_ERR_INVALID_PARAM:
        return NWC24_ERR_INVALID_VALUE;
    case ENC_ERR_NO_BUF_LEFT:
        return NWC24_ERR_OVERFLOW;
    case ENC_ERR_NO_MAP_RULE:
    case ENC_ERR_INVALID_FORMAT:
        return NWC24_ERR_INVALID_CHAR;
    case ENC_ERR_UNKNOWN_ENCODING:
    case ENC_ERR_UNSUPPORTED:
        return NWC24_ERR_NOT_SUPPORTED;
    default:
        return NWC24_ERR_FATAL;
    }
}

static NWC24Err NWC24iDetectEncodingToSend(char* pDst, u32 size, const u16* pText, u32 textLen, const u16* pText2, u32 textLen2,
                                     NWC24EncodingRegion region) {
    int index = 0;
    int offset = 0;
    int prev = -1;
    const char** pTable;
    int num;

    switch (region) {
    case NWC24_ENCODING_REGION_JPN:
        pTable = sCharsetsJpn;
        num = 3;
        break;
    case NWC24_ENCODING_REGION_USA:
    case NWC24_ENCODING_REGION_EUR:
    case NWC24_ENCODING_REGION_KOR:
    case NWC24_ENCODING_REGION_CHN:
    default:
        pTable = sCharsetsOther;
        num = 8;
        break;
    }

    while (index < num - 1) {
        if (ENCCheckEncoding(&index, &pTable[offset], num - 1 - offset, pText, textLen * 2) != ENC_OK) {
            return NWC24_ERR_NOT_SUPPORTED;
        }

        if (index == ENC_CHECK_ENCODING_NOT_FOUND) {
            index = num - 1;
            break;
        }

        offset += index;
        if (pText2 == NULL || prev == offset) {
            index = offset;
            break;
        }

        if (ENCCheckEncoding(&index, &pTable[offset], num - 1 - offset, pText2, textLen2 * 2) != ENC_OK) {
            return NWC24_ERR_NOT_SUPPORTED;
        }

        if (index == ENC_CHECK_ENCODING_NOT_FOUND) {
            index = num - 1;
            break;
        }

        prev = offset + index;
        if (prev == offset) {
            index = prev;
            break;
        }

        offset = prev;
    }

    strncpy(pDst, pTable[index], size - 1);
    pDst[size - 1] = '\0';
    return NWC24_OK;
}

static NWC24Err NWC24iDetectBreakPoint(u32* pOut, NWC24Charset charset, const u8* pStr, u32 len, u32 maxLen) {
    const u8* p;
    u32 i = 0;
    u32 j;
    BOOL singleByte;
    u32 pos = 0;

    if (pOut == NULL || pStr == NULL) {
        return NWC24_ERR_INVALID_VALUE;
    }

    if (charset == NWC24_US_ASCII) {
        singleByte = TRUE;
    } else if (charset == NWC24_UTF_8) {
        singleByte = FALSE;
    } else if ((charset & 0xFFFF0000) == 0x80000 || charset == NWC24_WINDOWS_1252) {
        singleByte = TRUE;
    } else {
        return NWC24_ERR_NOT_SUPPORTED;
    }

    if (len <= maxLen) {
        *pOut = len;
        return NWC24_OK;
    }

    p = pStr;
    for (i = 0; i < maxLen && *pStr != '\0'; i++, p++) {
        if (*p == ' ') {
            pos = i;
        }
    }

    if (pos != 0) {
        *pOut = pos;
        return NWC24_OK;
    }

    if (singleByte) {
        *pOut = maxLen;
        return NWC24_OK;
    }

    p = pStr;
    for (j = 0; j < maxLen && *pStr != '\0'; j++, p++) {
        if ((*p & 0xC0U) != 0x80) {
            pos = j;
        }
    }

    *pOut = pos;
    return NWC24_OK;
}

static NWC24Err NWC24iSetMsgSubjectPlain(NWC24MsgObj* pMsg, const u16* pSubject, u32 subjectLen, NWC24EncodingRegion region, u16 altChar, u8* pWork,
                                   u32 workSize, NWC24Charset charset, const char* pName) {
    NWC24Err result;
    u32 size;
    u32 srcLen;
    u32 split;
    u32 srcPos;
    u32 i;
    u32 half = workSize >> 1;
    u32 outSize = workSize - half;
    u8* pText = pWork;
    char* pOut = (char*)pWork + half;
    u32 outPos;

    srcLen = subjectLen;
    size = half;
    split = 0;
    result = NWC24iConvertFromInternalEncoding((char*)pText, &size, pSubject, &srcLen, (char*)pName, 0x40, region, altChar);
    if (result != NWC24_OK) {
        return result;
    }

    srcLen = size - 1;
    for (i = 0; i < size; i++) {
        if (pText[i] == '\n' || pText[i] == '\r') {
            pText[i] = ' ';
        }
    }

    result = NWC24iDetectBreakPoint(&split, charset, pText, srcLen, 0x3F);
    srcPos = split;
    if (srcPos == srcLen) {
        return NWC24SetMsgSubject(pMsg, (char*)pText, srcLen);
    }

    size = outSize;
    if (outSize <= srcPos) {
        return NWC24_ERR_OVERFLOW;
    }

    outPos = NWC24iStrLCpy(pOut, (char*)pText, srcPos + 1);
    while (srcPos < srcLen) {
        if (size - outPos < 3) {
            return NWC24_ERR_OVERFLOW;
        }

        pOut[outPos++] = '\r';
        pOut[outPos++] = '\n';
        if (pText[srcPos] != ' ') {
            pOut[outPos++] = ' ';
        }

        result = NWC24iDetectBreakPoint(&split, charset, pText + srcPos, srcLen - srcPos, 0x47);
        if (size - outPos <= split) {
            return NWC24_ERR_OVERFLOW;
        }

        outPos += NWC24iStrLCpy(pOut + outPos, (char*)pText + srcPos, split + 1);
        srcPos += split;
    }

    pOut[outPos] = '\0';
    return NWC24SetMsgSubject(pMsg, pOut, outPos);
}

static NWC24Err NWC24iSetMsgSubjectQP(NWC24MsgObj* pMsg, const u16* pSubject, u32 subjectLen, NWC24EncodingRegion region, u16 altChar, u8* pWork,
                                u32 workSize, NWC24Charset charset, const char* pName) {
    NWC24Err result;
    u32 size;
    u32 srcLen;
    u32 split;
    u32 outLen;
    u32 srcPos;
    u32 i;
    u32 maxLen;
    u32 half = workSize >> 1;
    u32 outSize = workSize - half;
    u8* pText = pWork;
    char* pOut = (char*)pWork + half;
    u32 outPos;

    srcLen = subjectLen;
    split = 0;
    size = half;
    result = NWC24iConvertFromInternalEncoding((char*)pText, &size, pSubject, &srcLen, (char*)pName, 0x40, region, altChar);
    if (result != NWC24_OK) {
        return result;
    }

    srcLen = size - 1;
    for (i = 0; i < size; i++) {
        if (pText[i] == '\n' || pText[i] == '\r') {
            pText[i] = ' ';
        }
    }

    maxLen = (0x38 - strlen(pName)) / 3;
    result = NWC24iDetectBreakPoint(&split, charset, pText, srcLen, maxLen);
    result = NWC24EncodeWord(pOut, outSize, &outLen, pName, 0x40, 'Q', (char*)pText, split);
    if (result != NWC24_OK) {
        return result;
    }

    srcPos = split;
    if (srcPos == srcLen) {
        return NWC24SetMsgSubject(pMsg, pOut, outLen - 1);
    }

    size = outSize;
    maxLen = (0x40 - strlen(pName)) / 3;
    outPos = outLen - 1;
    while (srcPos < srcLen) {
        if (size - outPos < 3) {
            return NWC24_ERR_OVERFLOW;
        }

        pOut[outPos++] = '\r';
        pOut[outPos++] = '\n';
        pOut[outPos++] = ' ';
        result = NWC24iDetectBreakPoint(&split, charset, pText + srcPos, srcLen - srcPos, maxLen);
        result = NWC24EncodeWord(pOut + outPos, size - outPos, &outLen, pName, 0x40, 'Q', (char*)pText + srcPos, split);
        if (result != NWC24_OK) {
            return result;
        }

        outPos = outPos + outLen - 1;
        srcPos += split;
    }

    return NWC24SetMsgSubject(pMsg, pOut, outPos);
}

static NWC24Err NWC24iSetMsgSubjectBase64(NWC24MsgObj* pMsg, const u16* pSubject, u32 subjectLen, NWC24EncodingRegion region, u16 altChar, u8* pWork,
                                    u32 workSize, NWC24Charset charset, const char* pName) {
    u32 maxBytes;
    NWC24Err result;
    u32 size;
    u32 half = workSize >> 1;
    char* pOut = (char*)pWork + half;
    u32 outSize = workSize - half;
    u32 srcLen;
    u8* pText = pWork;
    u32 srcPos;
    u32 outLen;
    u32 outPos;

    maxBytes = (0x36 - strlen(pName)) * 3 / 4;
    if (maxBytes > half) {
        return NWC24_ERR_OVERFLOW;
    }

    srcLen = subjectLen;
    size = maxBytes + 1;
    result = NWC24iConvertFromInternalEncoding((char*)pText, &size, pSubject, &srcLen, (char*)pName, 0x40, region, altChar);
    if (result != NWC24_OK && result != NWC24_ERR_OVERFLOW) {
        return result;
    }

    result = NWC24EncodeWord(pOut, outSize, &outLen, pName, 0x40, 'B', (char*)pText, size - 1);
    if (result != NWC24_OK) {
        return result;
    }

    srcPos = srcLen;
    if (srcPos == subjectLen) {
        return NWC24SetMsgSubject(pMsg, pOut, outLen - 1);
    }

    maxBytes = (0x3E - strlen(pName)) * 3 / 4 + 1;
    outPos = outLen - 1;
    while (srcPos < subjectLen) {
        srcLen = subjectLen - srcPos;
        size = maxBytes;
        if (outSize - outPos < 3) {
            return NWC24_ERR_OVERFLOW;
        }

        pOut[outPos++] = '\r';
        pOut[outPos++] = '\n';
        pOut[outPos++] = ' ';
        result = NWC24iConvertFromInternalEncoding((char*)pText, &size, pSubject + srcPos, &srcLen, (char*)pName, 0x40, region, altChar);
        if (result != NWC24_OK && result != NWC24_ERR_OVERFLOW) {
            return result;
        }

        result = NWC24EncodeWord(pOut + outPos, outSize - outPos, &outLen, pName, 0x40, 'B', (char*)pText, size - 1);
        if (result != NWC24_OK) {
            return result;
        }

        outPos = outPos + outLen - 1;
        srcPos += srcLen;
    }

    return NWC24SetMsgSubject(pMsg, pOut, outPos);
}
