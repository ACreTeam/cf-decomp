#include <private/nwc24.h>
#include <revolution/nwc24.h>

static const char* MIMEEncStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

#define ALPHABET_LEN 64

#define MAX_OCTETS 4
#define DEFAULT_LINE_LEN 75

static NWC24Err ConcatEncodedText(char* pDst, u32 dstSize, u32* pWritten, char encoding, const char* pText, u32 textLen, u32* pRead);

static NWC24Err Base64Encode(char* pData, s32 dataSize, u32* pDataRead, char* pText, s32 textSize, u32* pTextWritten) {
    NWC24Err result = NWC24_OK;
    s32 textIdx;
    s32 dataIdx = 0;
    BOOL hasThird;
    BOOL hasSecond;
    u32 combined;

    for (textIdx = 0; dataIdx < dataSize; dataIdx += 3) {
        if (textIdx + 3 >= textSize) {
            result = NWC24_ERR_OVERFLOW;
            break;
        }

        hasThird = FALSE;
        hasSecond = FALSE;

        combined = (u8)pData[dataIdx];
        combined <<= 8;

        if (dataIdx + 1 < dataSize) {
            hasSecond = TRUE;
            combined |= (u8)pData[dataIdx + 1];
        }

        combined <<= 8;

        if (dataIdx + 2 < dataSize) {
            hasThird = TRUE;
            combined |= (u8)pData[dataIdx + 2];
        }

        pText[textIdx + 0] = MIMEEncStr[combined >> 18 & (ALPHABET_LEN - 1)];
        pText[textIdx + 1] = MIMEEncStr[combined >> 12 & (ALPHABET_LEN - 1)];
        pText[textIdx + 2] = MIMEEncStr[hasSecond ? (combined >> 6 & (ALPHABET_LEN - 1)) : ALPHABET_LEN];
        pText[textIdx + 3] = MIMEEncStr[hasThird ? (combined & (ALPHABET_LEN - 1)) : ALPHABET_LEN];

        textIdx += 4;
    }

    *pTextWritten = textIdx;

    if (pDataRead != NULL) {
        textIdx = dataSize;

        if (dataSize < dataIdx) {
            dataIdx = dataSize;
        }

        *pDataRead = dataIdx;
    }

    return result;
}

NWC24Err NWC24Base64Encode(const u8* pData, s32 dataSize, char* pText, s32 textSize, u32* pTextWritten) {
    return Base64Encode((char*)pData, dataSize, NULL, pText, textSize, pTextWritten);
}

NWC24Err NWC24Base64Decode(const char* pText, s32 textSize, u8* pData, u32 dataSize, u32* pDataWritten) {
    s32 bits;
    s32 accum;
    u32 dataIdx;
    const char* pTable;
    s32 i;

    bits = 0;
    accum = 0;
    dataIdx = 0;
    pTable = NWC24WorkP->base64Work;

    for (i = 0; i < textSize; i++) {
        char value = pTable[(u8)pText[i]];

        if (value < 0 || value >= ALPHABET_LEN) {
            continue;
        }

        bits += 6;
        accum <<= 6;
        accum |= value;

        if (bits >= 8) {
            if (dataIdx >= dataSize) {
                *pDataWritten = dataSize;
                return NWC24_ERR_OVERFLOW;
            }

            bits -= 8;
            dataIdx++;
            *pData++ = accum >> bits;
        }
    }

    *pDataWritten = dataIdx;
    return NWC24_OK;
}

void NWC24InitBase64Table(char* pTable) {
    int i;
    for (i = 0; i < NWC24_BASE64_TABLE_SIZE; i++) {
        pTable[i] = 0xFF;
    }

    for (i = 'A'; i <= 'Z'; i++) {
        pTable[i] = 0 + i - 'A';
    }

    for (i = 'a'; i <= 'z'; i++) {
        pTable[i] = 26 + i - 'a';
    }

    for (i = '0'; i <= '9'; i++) {
        pTable[i] = 26 + 26 + i - '0';
    }

    pTable['+'] = 26 + 26 + 10;
    pTable['/'] = 26 + 26 + 10 + 1;
}

static NWC24Err QEncode(u8* pText, u32 textSize, u32* pTextWritten, u8* pData, u32 dataSize, u32* pDataRead, s32 maxLineLength) {
    u8* pWritePtr;
    u32 textWritten;
    u32 dataIdx;
    NWC24Err result;
    s32 lineLength;
    u8 octets[MAX_OCTETS];
    BOOL lineBreak;
    s32 emitCount;
    s32 bytesRead;
    s16 currentByte;
    s32 i;

    result = NWC24_OK;

    if (pText == NULL) {
        return NWC24_ERR_INVALID_VALUE;
    }

    if (pData == NULL) {
        return NWC24_ERR_INVALID_VALUE;
    }

    if (dataSize <= 0) {
        return NWC24_ERR_OVERFLOW;
    }

    if (pDataRead != NULL) {
        *pDataRead = 0;
    }

    if (pTextWritten != NULL) {
        *pTextWritten = 0;
    }

    pWritePtr = pText;
    lineLength = 0;
    textWritten = 0;
    dataIdx = 0;

    while (dataIdx < dataSize) {
        currentByte = *pData;
        lineBreak = FALSE;

        if ((char)currentByte >= '!' && (char)currentByte <= '~' && (char)currentByte != '=') {
            bytesRead = 1;
            emitCount = 1;
            octets[0] = *pData;

        } else if (dataIdx + 1 < dataSize && (char)currentByte == '\r' && (char)pData[1] == '\n') {
            bytesRead = 2;
            emitCount = 1;
            octets[0] = *pData;
            octets[1] = pData[1];

        } else {
            octets[0] = '=';

            if ((*pData >> 4) >= 10) {
                octets[1] = (*pData >> 4) % 10 + 'A';
            } else {
                octets[1] = (*pData >> 4) % 10 + '0';
            }

            if ((*pData & 0x0F) >= 10) {
                octets[2] = (*pData & 0x0F) % 10 + 'A';
            } else {
                octets[2] = (*pData & 0x0F) % 10 + '0';
            }

            bytesRead = 1;
            emitCount = 3;
        }

        if (maxLineLength == 0 && lineLength + emitCount >= DEFAULT_LINE_LEN) {
            lineBreak = TRUE;
            emitCount += 3;
        }

        if (textWritten + emitCount >= textSize) {
            result = NWC24_ERR_OVERFLOW;
            break;
        }

        textWritten += emitCount;

        if (lineBreak) {
            pWritePtr[0] = '=';
            pWritePtr[1] = '\r';
            pWritePtr[2] = '\n';

            pWritePtr += 3;
            emitCount -= 3;

            lineLength = 0;
        }

        lineLength += emitCount;

        for (i = 0; i < emitCount; i++) {
            *(pWritePtr++) = octets[i];
        }

        dataIdx += bytesRead;
        pData += bytesRead;
    }

    if (pDataRead != NULL) {
        *pDataRead = dataIdx;
    }

    if (pTextWritten != NULL) {
        *pTextWritten = textWritten;
    }

    return result;
}

NWC24Err NWC24EncodeQuotedPrintable(char* pText, u32 textSize, u32* pTextWritten, const u8* pData, u32 dataSize, u32* pDataRead) {
    return QEncode((u8*)pText, textSize, pTextWritten, (u8*)pData, dataSize, pDataRead, 0);
}

#define APPEND_TEXT(result, pDst, dstSize, pSrc, need, count) \
    do { \
        if (Mail_strlen(pDst) + (need) < (dstSize)) { \
            Mail_strncat(pDst, pSrc, count); \
        } else { \
            result = NWC24_ERR_OVERFLOW; \
            break; \
        } \
        result = NWC24_OK; \
    } while (0)

static NWC24Err EncodeWord(char* pDst, u32 dstSize, u32* pWritten, const char* pCharset, u32 charsetLen, char encoding, const char* pText,
                           u32 textLen, u32* pRead) {
    NWC24Err result;
    BOOL overflow = FALSE;
    char encChar;

    if (pRead != NULL) {
        *pRead = 0;
    }

    if (dstSize == 0) {
        if (pRead != NULL) {
            *pRead = 0;
        }
        return NWC24_ERR_INVALID_VALUE;
    }

    pDst[0] = '\0';
    if (pWritten != NULL) {
        *pWritten = 1;
    }

    APPEND_TEXT(result, pDst, dstSize, "=?", 2, 2);
    if (result != NWC24_OK) {
        pDst[0] = '\0';
        return result;
    }

    if (pCharset == NULL) {
        result = NWC24_ERR_INVALID_VALUE;
    } else {
        APPEND_TEXT(result, pDst, dstSize, pCharset, charsetLen, Mail_strlen(pCharset));
    }
    if (result != NWC24_OK) {
        pDst[0] = '\0';
        return result;
    }

    APPEND_TEXT(result, pDst, dstSize, "?", 2, 1);
    if (result != NWC24_OK) {
        pDst[0] = '\0';
        return result;
    }

    encChar = encoding;
    if (encoding != 'B' && encoding != 'b' && encoding != 'Q' && encoding != 'q') {
        result = NWC24_ERR_INVALID_VALUE;
    } else {
        APPEND_TEXT(result, pDst, dstSize, &encChar, 1, 1);
    }
    if (result != NWC24_OK) {
        pDst[0] = '\0';
        return result;
    }

    APPEND_TEXT(result, pDst, dstSize, "?", 2, 1);
    if (result != NWC24_OK) {
        pDst[0] = '\0';
        return result;
    }

    result = ConcatEncodedText(pDst, dstSize, pWritten, encoding, pText, textLen, pRead);
    if (result != NWC24_OK && result != NWC24_ERR_OVERFLOW) {
        pDst[0] = '\0';
        *pRead = 1;
        return result;
    }

    if (result == NWC24_ERR_OVERFLOW) {
        overflow = TRUE;
    }

    APPEND_TEXT(dstSize, pDst, dstSize, "?=", 2, 2);
    if (dstSize != NWC24_OK) {
        pDst[0] = '\0';
        *pRead = 1;
        return dstSize;
    }

    *pWritten = Mail_strlen(pDst) + 1;

    if (dstSize == NWC24_OK && overflow) {
        dstSize = NWC24_ERR_OVERFLOW;
    }

    return dstSize;
}

NWC24Err NWC24EncodeWord(char* pDst, u32 dstSize, u32* pWritten, const char* pCharset, u32 charsetLen, char encoding, const char* pText,
                         u32 textLen) {
    NWC24Err result;

    if (pWritten == NULL) {
        return NWC24_ERR_INVALID_VALUE;
    }

    if (pDst == NULL || dstSize == 0) {
        return NWC24_ERR_INVALID_VALUE;
    }

    if (pCharset == NULL || charsetLen == 0) {
        return NWC24_ERR_INVALID_VALUE;
    }

    if (encoding == 0) {
        return NWC24_ERR_INVALID_VALUE;
    }

    if (pText == NULL) {
        return NWC24_ERR_INVALID_VALUE;
    }

    pDst[0] = '\0';
    *pWritten = 1;

    result = EncodeWord(pDst, dstSize, pWritten, pCharset, charsetLen, encoding, pText, textLen, NULL);
    if (result == NWC24_OK || result == NWC24_ERR_OVERFLOW) {
        pDst[*pWritten - 1] = '\0';
    }

    return result;
}

static NWC24Err ConcatEncodedText(char* pDst, u32 dstSize, u32* pWritten, char encoding, const char* pText, u32 textLen, u32* pRead) {
    NWC24Err result;
    s32 len = Mail_strlen(pDst);
    s32 space = dstSize - len - 2;

    if (space > 0) {
        if (encoding == 'B' || encoding == 'b') {
            result = Base64Encode((char*)pText, textLen, pRead, pDst + len, space - 1, pWritten);
        } else if (encoding == 'Q' || encoding == 'q') {
            result = QEncode((u8*)pDst + len, space - 1, pWritten, (u8*)pText, textLen, pRead, 1);
        } else {
            result = NWC24_ERR_NOT_SUPPORTED;
        }

        if (result == NWC24_OK || result == NWC24_ERR_OVERFLOW) {
            pDst[len + *pWritten] = '\0';
            *pWritten = *pWritten + 1;
        }
    } else {
        result = NWC24_ERR_OVERFLOW;
    }

    return result;
}
