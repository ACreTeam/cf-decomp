#pragma once

// The game camera (RTTI "dCamera_c"); not decompiled yet. Only what the sky, the star field, the drum
// (d_drum) and d_objc use.

#include <egg/gfxe/eggCamera.h>
#include <egg/gfxe/eggFrustum.h>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_vec.hpp>
#include <nw4r/math.h>

class dCamera_c {
public:
    f32 getAspect() const; // 8018B2EC
    static f32 getFovy();  // 8018B2FC

    void getProjectionMtx(nw4r::math::MTX44 *mtx) const {
        if (mFrustum.mProjType == EGG::Frustum::PROJ_ORTHO) {
            mFrustum.GetOrthographicMtx(mtx);
        } else {
            mFrustum.GetPerspectiveMtx(mtx);
        }
    }

    // 801B0194 (weak copy kept in d_objc). Non-const: a const version stores y before x.
    mVec3_c getEyePos() { return mVec3_c(mCamera.mPos); }

    // Talk camera (d_a_npc talk_c; names provisional).
    bool fn_8018A748() const;                                // 8018A748: (f240 <= f248)
    f32 fn_8018A460() const;                                 // 8018A460: f248 - f240
    void fn_8018B5B8(const mVec3_c *target);                 // 8018B5B8: focus one actor
    void fn_8018B67C(const mVec3_c *a, const mVec3_c *b);    // 8018B67C: focus two actors
    void fn_8018B768(const mVec3_c *target);                 // 8018B768: change the focus (0x228..)

    /* 0x000 */ u8 _000[0x64];
    /* 0x064 */ EGG::Frustum mFrustum;
    /* 0x0A0 */ u8 _0A0[0xCC - 0xA0];
    /* 0x0CC */ EGG::LookAtCamera mCamera; // mPos (0x130): the eye position
    /* 0x154 */ u8 _154[0x16C - 0x154];
    /* 0x16C */ mMtx_c mViewMtx;
    /* 0x19C */ u8 _19C[0x1A4 - 0x19C];
    /* 0x1A4 */ int _1A4;
    /* 0x1A8 */ int _1A8; // talk_c::finish restores it from _1A4
    /* 0x1AC */ u8 _1AC[0x1CC - 0x1AC];
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
