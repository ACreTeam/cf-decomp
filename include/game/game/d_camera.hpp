#pragma once

// The game camera (RTTI "dCamera_c"); not decompiled yet. Only what the sky, the drum (d_drum)
// and d_objc use.

#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_vec.hpp>
#include <nw4r/math.h>

class dCamera_c {
public:
    // 801B0194 (weak copy kept in d_objc). Non-const: a const version stores y before x.
    mVec3_c getEyePos() { return mVec3_c(mEyePos); }

    /* 0x000 */ u8 _000[0x130];
    /* 0x130 */ nw4r::math::VEC3 mEyePos;
    /* 0x13C */ u8 _13C[0x16C - 0x13C];
    /* 0x16C */ mMtx_c mViewMtx;
    /* 0x19C */ u8 _19C[0x1CC - 0x19C];
    /* 0x1CC */ s16 mPitch;
    /* 0x1CE */ s16 mYaw;
};

extern "C" {
extern nw4r::math::VEC3 lbl_80623FEC; // 80623FEC: the camera's target (the view center)
extern mVec3_c lbl_80624004;          // 80624004: the camera's eye position
extern dCamera_c *lbl_8074E9B0;       // 8074E9B0: the camera
}

template <typename T>
T *getViewCenter() {
    return &lbl_80623FEC;
}
