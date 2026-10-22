#pragma once

#include <nw4r/g3d.h>

namespace EGG {

class Frustum {
public:
    enum Flag {
        FLAG_DIRTY = BIT_FLAG(0),
        FLAG_ALIGN_EFB_POS = BIT_FLAG(1),    // Screen: round the EFB position down to even
        FLAG_ALIGN_EFB_SIZE = BIT_FLAG(2),   // Screen: round the EFB size down to a multiple of 4
        FLAG_USE_EFB_RATIO = BIT_FLAG(3),    // Screen: use the EFB-sized TV mode entry
        FLAG_FORCE_STD_ASPECT = BIT_FLAG(5), // Screen: always use the 4:3 TV mode entry
        FLAG_PROJ_ABSOLUTE = BIT_FLAG(6),    // Screen: ignore the global canvas scale/offset
    };

    enum ProjectionType {
        PROJ_ORTHO,
        PROJ_PERSP
    };

    enum CanvasMode {
        CANVASMODE_0, // center-canvas origin
        CANVASMODE_1, // left-upper origin

        CANVAS_CC = CANVASMODE_0,
        CANVAS_LU = CANVASMODE_1,
    };

    ProjectionType mProjType;
    CanvasMode mCanvasMode;
    nw4r::math::VEC2 mSize;
    f32 mFovY;
    f32 mTanFovY;
    f32 mNearZ;
    f32 mFarZ;
    nw4r::math::VEC2 mOffset;
    nw4r::math::VEC3 mScale;
    mutable u16 mFlags;

    Frustum(ProjectionType projType, const nw4r::math::VEC2 &scale, f32 nearZ, f32 farZ, CanvasMode canvasMode);
    Frustum(const Frustum &frustum);

    virtual ~Frustum() {}
    virtual void SetProjectionGX() const {
        if (mProjType == PROJ_ORTHO) {
            SetProjectionOrthographicGX_();
        } else {
            SetProjectionPerspectiveGX_();
        }
    }
    virtual void CopyToG3D(nw4r::g3d::Camera camera) const {
        if (mProjType == PROJ_ORTHO) {
            CopyToG3D_Orthographic_(camera);
        } else {
            CopyToG3D_Perspective_(camera);
        }
    }
    void SetCanvasMode(CanvasMode mode) {
        if (mCanvasMode != mode) {
            mFlags |= FLAG_DIRTY;
            mCanvasMode = mode;
        }
    }

    void SetFovy(f32 fovy) {
        if (mFovY != fovy) {
            mFovY = fovy;
            f32 sin, cos;
            f32 halfFovy = fovy / 2.0f;
            nw4r::math::SinCosDeg(&sin, &cos, halfFovy);
            mTanFovY = cos / sin;
        }
    }

    void ResetOrthographic(f32 top, f32 bottom, f32 left, f32 right, f32 nearZ, f32 farZ);

    // Names inferred.
    void GetPerspectiveMtx(nw4r::math::MTX44 *mtx) const;  // 80447E7C
    void GetOrthographicMtx(nw4r::math::MTX44 *mtx) const; // 80447F0C

    f32 GetAspect() const { return mSize.x / mSize.y; }

    void SetSizeX(f32 sizeX) {
        mFlags |= FLAG_DIRTY;
        mSize.x = sizeX;
    }
    void SetSizeY(f32 sizeY) {
        mFlags |= FLAG_DIRTY;
        mSize.y = sizeY;
    }

    void ConvertFromCanvasCC(f32 x, f32 y, f32 *pX, f32 *pY) const {
        *pX = x + 0.5f * mSize.x;
        *pY = -(y - 0.5f * mSize.y);
    }

    void ConvertToCanvasLU(f32 x, f32 y, f32 *pX, f32 *pY) const {
        if (mCanvasMode == CANVAS_LU) {
            *pX = x;
            *pY = y;
        } else if (mCanvasMode == CANVAS_CC) {
            ConvertFromCanvasCC(x, y, pX, pY);
        }
    }

    static void GetGlobalScaleOffset(f32 *pSX, f32 *pSY, f32 *pOX, f32 *pOY) {
        *pSX = sGlobalScale.x;
        *pSY = sGlobalScale.y;
        *pOX = sGlobalOffset.x;
        *pOY = sGlobalOffset.y;
    }
    static void SetGlobalScaleOffset(f32 sx, f32 sy, f32 ox, f32 oy) {
        sGlobalScale.x = sx;
        sGlobalScale.y = sy;
        sGlobalOffset.x = ox;
        sGlobalOffset.y = oy;
    }

private:
    void SetProjectionPerspectiveGX_() const;                             // 80447D50
    void SetProjectionOrthographicGX_() const;                            // 80447D7C
    void CopyToG3D_Perspective_(nw4r::g3d::Camera camera) const;          // 80447DA8
    void CopyToG3D_Orthographic_(nw4r::g3d::Camera camera) const;         // 80447E14
    void GetPerspectiveParam_(f32 *p) const;                              // 80447F9C
    void GetOrthographicParam_(f32 *p) const;                             // 80448058
    void GetOrthographicParam_(f32 *pT, f32 *pB, f32 *pL, f32 *pR) const; // 80448138

    static nw4r::math::VEC2 sGlobalScale;  // 8074FC90
    static nw4r::math::VEC2 sGlobalOffset; // 8074FC98
};

} // namespace EGG
