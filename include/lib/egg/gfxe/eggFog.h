#pragma once

#include <lib/egg/prim/eggBinary.h>
#include <nw4r/g3d.h>
#include <nw4r/ut.h>
#include <revolution/GX.h>

namespace EGG {

class Fog : public IBinary<Fog> {
public:
    enum {
        FLAG_ENABLE = 1 << 0,
    };

    struct BinData {
        /* 0x10 */ f32 mStartZ;
        /* 0x14 */ f32 mEndZ;
        /* 0x18 */ f32 mNearZ;
        /* 0x1C */ f32 mFarZ;
        /* 0x20 */ GXColor mColor;
        /* 0x24 */ u8 mFogType;
        /* 0x25 */ u8 mFlags;
        /* 0x26 */ u8 _26[10];
    };

    Fog();

    virtual void SetBinaryInner(const Bin &);
    virtual void GetBinaryInner(Bin *) const;

    void Reset();
    void CopyToG3D(nw4r::g3d::Fog fog) const;

    f32 GetStartZ() const { return mStartZ; }
    f32 GetEndZ() const { return mEndZ; }
    f32 GetNearZ() const { return mNearZ; }
    f32 GetFarZ() const { return mFarZ; }

    /* 0x04 */ GXColor mColor;
    /* 0x08 */ f32 mStartZ;
    /* 0x0C */ f32 mEndZ;
    /* 0x10 */ f32 mNearZ;
    /* 0x14 */ f32 mFarZ;
    /* 0x18 */ GXFogType mFogType;
    /* 0x1C */ u8 mFlags;
};

} // namespace EGG
