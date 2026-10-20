#ifndef DWC_SUPPORT_NITRO_MATH_RAND_H
#define DWC_SUPPORT_NITRO_MATH_RAND_H

#include <revolution/types.h>

typedef struct {
    u64 x;
    u64 mul;
    u64 add;
} MATHRandContext32;

static inline void MATH_InitRand32(MATHRandContext32 *context, u64 seed) {
    context->x = seed;
    context->mul = (1566083941LL << 32) + 1812433253LL;
    context->add = 2531011;
}

static inline u32 MATH_Rand32(MATHRandContext32 *context, u32 max) {
    context->x = context->mul * context->x + context->add;
    if (max == 0) {
        return (u32)(context->x >> 32);
    }
    return (u32)(((context->x >> 32) * max) >> 32);
}

#endif
