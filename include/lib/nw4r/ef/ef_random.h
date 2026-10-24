#ifndef NW4R_EF_RANDOM_H
#define NW4R_EF_RANDOM_H
#include <nw4r/types_nw4r.h>

namespace nw4r {
namespace ef {

class Random {
private:
    u32 mSeed; // at 0x0

public:
    void Srand(u32 seed) {
        mSeed = seed;
    }

    int Rand() {
        MixRandomSeed();
        return static_cast<int>(mSeed >> 16);
    }

    f32 RandFloat() {
        MixRandomSeed();
        return static_cast<f32>(mSeed >> 16) / 0x10000;
    }

private:
    void MixRandomSeed() {
        mSeed = mSeed * 0x343FD + 0x269EC3;
    }
};

inline u32 RANDOM_INIT(u16 objectSeed, u16 keySeed, u16 keyIdx, u32 keyDiv) {
    union {
        u32 whole;
        u8 part[4];
    } result;

    result.whole = objectSeed * 0x3F81F635UL + keySeed * 0x30A74193UL +
                   keyIdx * 0x371097E7UL + keyDiv * 0x7B929UL + 0x4BF53UL;

    result.part[2] ^= result.part[3];
    result.part[1] ^= result.part[2];
    result.part[0] ^= result.part[1];

    return result.whole;
}

inline u32 RANDOM_NEXT(u32 prev) {
    return prev * 0x343FDUL + 0x269EC3UL;
}

} // namespace ef
} // namespace nw4r

#endif
