#ifndef PRIVATE_NWC24_MIME_H
#define PRIVATE_NWC24_MIME_H

#include <revolution/types.h>

#include <revolution/nwc24/NWC24Err.h>
#include <revolution/nwc24/NWC24Types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NWC24_BASE64_TABLE_SIZE 256

void NWC24InitBase64Table(char* table);

NWC24Err NWC24Base64Encode(const u8* decoded, s32 decodedSize, char* encoded, s32 encodedSize, u32* stringSize);
NWC24Err NWC24Base64Decode(const char* encoded, s32 encodedSize, u8* decoded, u32 decodedSize, u32* stringSize);

NWC24Err NWC24EncodeQuotedPrintable(char* encData, u32 encDataSize, u32* encodedSize, const u8* decData, u32 decDataSize, u32* decodedSize);
NWC24Err NWC24DecodeQuotedPrintable(char* decData, u32 decDataSize, u32* decodedSize, u8* encData, u32 encDataSize, u32* encodedSize);

NWC24Err NWC24EncodeWord(char* encData, u32 encDataSize, u32* encodedSize, const char* charset, u32 charsetSize, char encoding, const char* decData,
                         u32 decDataSize);

NWC24Err NWC24DecodeMIMEHeaderFieldBody(u8* charsetData, u32 charsetDataSize, u8* decData, u32 decDataSize, u32* decodedSize, u8* encData,
                                        u32 encDataSize);

#ifdef __cplusplus
}
#endif

#endif  // PRIVATE_NWC24_MIME_H
