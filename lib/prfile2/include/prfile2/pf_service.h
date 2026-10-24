#ifndef PRFILE2_PF_SERVICE_H
#define PRFILE2_PF_SERVICE_H

#include <prfile2/pf_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SWAP16(x) ((((x) >> 8) & 0xFF) | (((x) << 8) & 0xFF00))
#define SWAP32(x) (((x) >> 24) | (((x) >> 8) & 0xFF00) | (((x) << 8) & 0xFF0000) | ((x) << 24))

u16 PF_GET_LE_U16(const u8* buf);
void PF_LE16_TO_U16_STR(u8* sSrc, u32 num);

#ifdef __cplusplus
}
#endif
#endif
