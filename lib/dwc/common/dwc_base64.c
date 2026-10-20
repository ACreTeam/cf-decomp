#include <dwc/dwc_base64.h>
#include <dwc/dwc_nonport.h>

#include <string.h>

static const char* sBase64EncodeAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-";

int DWC_Base64Encode(const char* src, u32 srclen, char* dst, u32 dstlen) {
  u32 copySize;
  u32 encodeCount;
  const char* srcPos;
  const char* srcEnd;
  char* dstPos;
  int remain;
  u32 dstSize;
  u8 work[3];

  dstSize = (srclen / 3) * 4 + (srclen % 3 != 0 ? 4 : 0);

  if (dst == NULL)
    return dstSize;

  if (dstlen < dstSize)
    return -1;

  srcEnd = src + srclen;
  srcPos = src;
  dstPos = dst;

  while (srcPos != srcEnd) {
    remain = srcEnd - srcPos;
    encodeCount = ((u32)remain * 8) / 6 + (((u32)remain * 8) % 6 != 0);

    copySize = remain < 3 ? remain : 3;

    memset(work, 0, sizeof(work));
    DWCi_Np_CpuCopy8(srcPos, work, copySize);

    dstPos[0] = sBase64EncodeAlphabet[work[0] >> 2];
    if (encodeCount >= 2)
      dstPos[1] = sBase64EncodeAlphabet[((work[0] & 3) << 4) | (work[1] >> 4)];
    else
      dstPos[1] = '*';

    if (encodeCount >= 3)
      dstPos[2] = sBase64EncodeAlphabet[((work[1] & 15) << 2) | (work[2] >> 6)];
    else
      dstPos[2] = '*';

    if (encodeCount >= 4)
      dstPos[3] = sBase64EncodeAlphabet[work[2] & 63];
    else
      dstPos[3] = '*';

    srcPos += copySize;
    dstPos += 4;
  }

  return dstPos - dst;
}

int DWC_Base64Decode(const char* src, u32 srclen, char* dst, u32 dstlen) {
  const char* srcPos;
  char* dstPos;
  int decodeSize;
  char work[4];
  int result;
  int j;
  int i;

  if (srclen % 4 != 0)
    return -1;

  srcPos = src;
  decodeSize = 0;
  for (i = 0; i < (int)srclen; ++i) {
    if (*srcPos != '*')
      decodeSize += 6;
    ++srcPos;
  }
  decodeSize /= 8;

  if (dst == NULL)
    return decodeSize;

  if (dstlen < decodeSize)
    return -1;

  if (srclen == 0) {
    *dst = '\0';
    return 0;
  }

  dstPos = dst;
  do {
    for (j = 0; j < 4; ++j) {
      if (src[j] >= 'A' && src[j] <= 'Z')
        work[j] = src[j] - 'A';
      else if (src[j] >= 'a' && src[j] <= 'z')
        work[j] = src[j] - 'a' + 26;
      else if (src[j] >= '0' && src[j] <= '9')
        work[j] = src[j] - '0' + 52;
      else if (src[j] == '.')
        work[j] = 62;
      else if (src[j] == '-')
        work[j] = 63;
      else
        work[j] = 0;
    }
    src += 4;

    dstPos[0] = (work[0] << 2) | (work[1] >> 4);
    result = dstPos + 1 - dst;
    if (result >= decodeSize)
      break;

    dstPos[1] = (work[1] << 4) | (work[2] >> 2);
    result = dstPos + 2 - dst;
    if (result >= decodeSize)
      break;

    dstPos[2] = (work[2] << 6) | work[3];
    dstPos += 3;
    result = dstPos - dst;
  } while (result < decodeSize);

  return result;
}
