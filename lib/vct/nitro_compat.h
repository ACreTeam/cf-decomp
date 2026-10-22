#ifndef VCT_NITRO_COMPAT_H
#define VCT_NITRO_COMPAT_H

#include <revolution/types.h>

// The few NITRO MI/MATH routines the voice chat library relies on. The Wii
// build carries its own definitions of them (see filter.c).

void MI_CpuFill32(u32 data, void* dest, u32 size);
void MI_CpuCopyFast(const void* src, void* dest, u32 size);
void MI_CpuCopy8(const void* src, void* dest, u32 size);
void* MI_CpuFill8(void* dest, int data, u32 size);

typedef struct MATHRandContext16 {
    u32 x;
    u32 mul;
    u32 add;
} MATHRandContext16;

static inline void MATH_InitRand16(MATHRandContext16* context, u32 seed) {
    context->x = seed;
    context->mul = 1566083941;
    context->add = 2531011;
}

u32 MATH_CountLeadingZeros(u32 x);
u32 MATH_CountPopulation(u32 x);

#endif
