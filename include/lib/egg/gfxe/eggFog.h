#pragma once

#include <nw4r/ut.h>
#include <revolution/GX.h>

namespace EGG {

class Fog {
public:
    enum {
        FLAG_ENABLE = 1 << 0,
    };

    virtual ~Fog();

    f32 GetStartZ() const { return mStartZ; }
    f32 GetEndZ() const { return mEndZ; }
    f32 GetNearZ() const { return mNearZ; }
    f32 GetFarZ() const { return mFarZ; }

    /* 0x04 */ nw4r::ut::Color mColor;
    /* 0x08 */ f32 mStartZ;
    /* 0x0C */ f32 mEndZ;
    /* 0x10 */ f32 mNearZ;
    /* 0x14 */ f32 mFarZ;
    /* 0x18 */ GXFogType mFogType;
    /* 0x1C */ u8 mFlags;
};

} // namespace EGG
