#pragma once

#include <lib/egg/gfxe/eggFog.h>
#include <lib/egg/prim/eggBinary.h>
#include <nw4r/g3d.h>

namespace EGG {

class FogManager : public IBinary<FogManager> {
public:
    struct BinData {
        /* 0x10 */ u16 mCount;
        /* 0x12 */ u8 _0x12[2];
        /* 0x14 */ IBinary<Fog>::Bin mFogData[1];
    };

    FogManager(unsigned short count);

    virtual void SetBinaryInner(const Bin &);
    virtual void GetBinaryInner(Bin *) const;
    virtual u32 GetBinarySize() const;
    virtual ~FogManager() {}

    void Calc();
    void CopyToG3D(nw4r::g3d::ScnRoot *) const;

    void DoneDraw() { mFlags &= ~FLAG_CALC_DONE; }

    enum {
        FLAG_CALC_DONE = 1 << 2,
    };

    /* 0x04 */ u8 mFlags;
    /* 0x05 */ u8 _05[3];
    /* 0x08 */ u16 mCount;
    /* 0x0A */ u16 mCount2;
    /* 0x0C */ Fog **mFog;
};

} // namespace EGG
