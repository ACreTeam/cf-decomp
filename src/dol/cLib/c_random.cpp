#include <types.h>
#include <game/cLib/c_random.hpp>
/// @file

/// @ingroup clib


void cRandom_c::setSeed(u32 seed) {
    mSeed = seed;
}

u32 cRandom_c::getRandom() {
    u64 product = (u64)mSeed * 0x19660D;
    u64 sum = (product & 0xFFFFFFFFULL) + 0x3C6EF35F;
    // Fold the product's high word and the addition's carry into the low word.
    mSeed = (u32)sum + (u32)(product >> 32) +
            (u32)((sum & 0x100000000ULL) >> 32);
    return mSeed;
}

float cRandom_c::getRandomF() {
    u32 tmp = 0x3f800000 | (getRandom() >> 9 & 0x7fffff);
    return (*(float *)&tmp)-1.0f;
}
