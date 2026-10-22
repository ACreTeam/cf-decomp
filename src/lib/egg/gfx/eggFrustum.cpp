// EGG::Frustum. .text 80447BE8..80448360
#include <lib/egg/gfxe/eggFrustum.h>
#include <lib/egg/gfxe/eggGXUtility.h>
#include <lib/egg/gfxe/eggStateGX.h>
#include <nw4r/g3d.h>
#include <nw4r/math.h>
#include <revolution/GX.h>

// Float pool order: CF's .sdata2 for this unit starts with 1.0f, which only a dead-stripped function used.
// (SS's EGG has the same trick.) This function is not in CF's DOL; the linker strips it.
void eggFrustum_float_ordering() {
    1.0f;
}

namespace EGG {

nw4r::math::VEC2 Frustum::sGlobalScale(1.0f, 1.0f);
nw4r::math::VEC2 Frustum::sGlobalOffset(0.0f, 0.0f);

Frustum::Frustum(ProjectionType projType, const nw4r::math::VEC2 &rSize, f32 nearZ, f32 farZ, CanvasMode canvasMode)
    : mProjType(projType), mCanvasMode(canvasMode), mSize(rSize), mFovY(45.0f),
      mTanFovY(2.4142134f), // cos(fovy/2) / sin(fovy/2) (CF stores the cotangent here)
      mNearZ(nearZ), mFarZ(farZ), mOffset(0.0f, 0.0f), mScale(1.0f, 1.0f, 1.0f), mFlags(FLAG_DIRTY) {}

void Frustum::ResetOrthographic(f32 top, f32 bottom, f32 left, f32 right, f32 nearZ, f32 farZ) {
    0.5f; // float pool order: these two constants are first used by dead-stripped code before
    2.0f; // the -1.0f below

    mProjType = PROJ_ORTHO;
    mScale.z = 1.0f;
    mScale.y = 1.0f;
    mScale.x = 1.0f;
    mSize.x = right - left;
    if (mSize.x < 0.0f) {
        mScale.x = -1.0f;
        mSize.x *= -1.0f;
    }

    mSize.y = top - bottom;
    if (mSize.y >= 0.0f) {
        SetCanvasMode(CANVAS_CC);
        mOffset = nw4r::math::VEC2(0.5f * (right + left), 0.5f * (top + bottom));
    } else {
        SetCanvasMode(CANVAS_LU);
        mOffset = nw4r::math::VEC2(0.0f, top);
        mSize.y *= -1.0f;
    }
    mNearZ = nearZ;
    mFarZ = farZ;
    mFlags |= FLAG_DIRTY;
}

void Frustum::SetProjectionPerspectiveGX_() const {
    f32 proj[7];
    GetPerspectiveParam_(proj);
    StateGX::GXSetProjectionv_(proj);
}

void Frustum::SetProjectionOrthographicGX_() const {
    f32 proj[7];
    GetOrthographicParam_(proj);
    StateGX::GXSetProjectionv_(proj);
}

void Frustum::CopyToG3D_Perspective_(nw4r::g3d::Camera camera) const {
    camera.SetPerspective(mFovY, GetAspect(), mNearZ, mFarZ);

    nw4r::math::MTX44 mtx;
    GetPerspectiveMtx(&mtx);
    camera.SetProjectionMtxDirectly(&mtx);
}

void Frustum::CopyToG3D_Orthographic_(nw4r::g3d::Camera camera) const {
    f32 t, b, l, r;
    GetOrthographicParam_(&t, &b, &l, &r);

    camera.SetOrtho(t, b, l, r, mNearZ, mFarZ);
}

void Frustum::GetPerspectiveMtx(nw4r::math::MTX44 *pMtx) const {
    f32 proj[7];
    GetPerspectiveParam_(proj);

    pMtx->_03 = 0.0f;
    pMtx->_01 = 0.0f;
    pMtx->_00 = proj[1];
    pMtx->_02 = proj[2];

    pMtx->_13 = 0.0f;
    pMtx->_10 = 0.0f;
    pMtx->_11 = proj[3];
    pMtx->_12 = proj[4];

    pMtx->_21 = 0.0f;
    pMtx->_20 = 0.0f;
    pMtx->_22 = proj[5];
    pMtx->_23 = proj[6];

    pMtx->_33 = 0.0f;
    pMtx->_31 = 0.0f;
    pMtx->_30 = 0.0f;
    pMtx->_32 = -1.0f;
}

void Frustum::GetOrthographicMtx(nw4r::math::MTX44 *pMtx) const {
    f32 proj[7];
    GetOrthographicParam_(proj);

    pMtx->_02 = 0.0f;
    pMtx->_01 = 0.0f;
    pMtx->_00 = proj[1];
    pMtx->_03 = proj[2];

    pMtx->_12 = 0.0f;
    pMtx->_10 = 0.0f;
    pMtx->_11 = proj[3];
    pMtx->_13 = proj[4];

    pMtx->_21 = 0.0f;
    pMtx->_20 = 0.0f;
    pMtx->_22 = proj[5];
    pMtx->_23 = proj[6];

    pMtx->_32 = 0.0f;
    pMtx->_31 = 0.0f;
    pMtx->_30 = 0.0f;
    pMtx->_33 = 1.0f;
}

void Frustum::GetPerspectiveParam_(f32 *p) const {
    f32 invW = 1.0f / (0.5f * mSize.x);
    f32 invH = 1.0f / (0.5f * mSize.y);

    p[0] = static_cast<f32>(GX_PERSPECTIVE);
    p[1] = mTanFovY / (mScale.x * GetAspect());
    p[2] = mOffset.x * invW;
    p[3] = mTanFovY / mScale.y;
    p[4] = mOffset.y * invH;

    // Multiply by -N early
    f32 invrange = -mNearZ / (mFarZ - mNearZ);

    p[5] = invrange;
    p[6] = mFarZ * invrange;

    GXUtility::setScaleOffsetPerspective(p, sGlobalScale.x, sGlobalScale.y, sGlobalOffset.x * invW,
                                         sGlobalOffset.y * invH);
}

void Frustum::GetOrthographicParam_(f32 *p) const {
    f32 t, b, l, r;
    GetOrthographicParam_(&t, &b, &l, &r);

    // C_MTXOrtho
    f32 tmp = 1.0f / (r - l);
    f32 tmp2 = 1.0f / (t - b);
    f32 tmp3 = 1.0f / (mFarZ - mNearZ);

    p[0] = static_cast<f32>(GX_ORTHOGRAPHIC);
    p[1] = 2.0f * tmp;
    p[2] = tmp * -(r + l);
    p[3] = 2.0f * tmp2;
    p[4] = tmp2 * -(t + b);
    p[5] = -1.0f * tmp3;
    p[6] = -mFarZ * tmp3;
}

void Frustum::GetOrthographicParam_(f32 *pT, f32 *pB, f32 *pL, f32 *pR) const {
    f32 ox = mOffset.x + sGlobalOffset.x;
    f32 oy = mOffset.y + sGlobalOffset.y;

    if (mCanvasMode == CANVAS_CC) {
        nw4r::math::VEC2 scale(mScale.x * sGlobalScale.x, mScale.y * sGlobalScale.y);

        *pT = scale.y * (0.5f * mSize.y + oy);
        *pB = scale.y * (-0.5f * mSize.y + oy);
        *pL = scale.x * (-0.5f * mSize.x + ox);
        *pR = scale.x * (0.5f * mSize.x + ox);
    } else if (mCanvasMode == CANVAS_LU) {
        f32 x = -(sGlobalScale.x - 1.0f) * (0.5f * mSize.x);
        f32 y = -(sGlobalScale.y - 1.0f) * (0.5f * mSize.y);

        *pT = y + -sGlobalScale.y * oy;
        *pB = y + -sGlobalScale.y * (oy + -1.0f * mSize.y * mScale.y);
        *pL = x + sGlobalScale.x * ox;
        *pR = x + sGlobalScale.x * (ox + mSize.x * mScale.x);
    }
}

} // namespace EGG
