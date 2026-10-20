#ifndef DWC_SUPPORT_NITRO_MATH_CRC_H
#define DWC_SUPPORT_NITRO_MATH_CRC_H

#include <revolution/types.h>

#define MATH_CRC8_STANDARD_POLY 0x07
#define MATH_CRC32_STANDARD_POLY 0xEDB88320

typedef u8 MATHCRC8Context;
typedef u32 MATHCRC32Context;

typedef struct MATHCRC8Table {
    u8 table[256];
} MATHCRC8Table;

typedef struct MATHCRC32Table {
    u32 table[256];
} MATHCRC32Table;

void MATHi_CRC8InitTable(MATHCRC8Table *table, u8 poly);
void MATHi_CRC8Update(const MATHCRC8Table *table, MATHCRC8Context *context, const void *input, u32 length);
void MATHi_CRC32InitTableRev(MATHCRC32Table *table, u32 poly);
void MATHi_CRC32UpdateRev(const MATHCRC32Table *table, MATHCRC32Context *context, const void *input, u32 length);
u8 MATH_CalcCRC8(const MATHCRC8Table *table, const void *data, u32 dataLength);
u32 MATH_CalcCRC32(const MATHCRC32Table *table, const void *data, u32 dataLength);

static inline void MATH_CRC8InitTable(MATHCRC8Table *table) {
    MATHi_CRC8InitTable(table, MATH_CRC8_STANDARD_POLY);
}

static inline void MATH_CRC32InitTable(MATHCRC32Table *table) {
    MATHi_CRC32InitTableRev(table, MATH_CRC32_STANDARD_POLY);
}

#endif
