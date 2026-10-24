#ifndef NW4R_EF_DRAW_INFO_H
#define NW4R_EF_DRAW_INFO_H
#include <nw4r/types_nw4r.h>

#include <nw4r/math.h>
#include <nw4r/ef/ef_util.h>

#include <revolution/gx.h>

namespace nw4r {
namespace ef {

class DrawInfo {
  public:
    math::MTX34 mViewMtx;      // at 0x0
    math::MTX34 mProjMtx;      // at 0x30
    bool mLightEnable;         // at 0x60
    GXLightID mLightMask;      // at 0x64
    GXLightID mLightAlphaMask; // City Folk: 0x68
    bool mIsSpotLight;         // at 0x6C
    GXFogType mFogType;        // at 0x70
    f32 mFogStartz;            // at 0x74
    f32 mFogEndz;              // at 0x78
    f32 mFogNearz;             // at 0x7C
    f32 mFogFarz;              // at 0x80
    GXColor mFogColor;         // at 0x84
    f32 mZOffset;              // at 0x88
    math::VEC3 mZOffsetOrig;   // at 0x8C
    GXColor mChanMatColor;     // at 0x98
    GXColor mChanAmbColor;     // at 0x9C

  public:
    DrawInfo() {
        math::MTX34Identity(&mViewMtx);
        math::MTX34Identity(&mProjMtx);

        mLightEnable = false;
        mLightMask = GX_LIGHT_NULL;
        mLightAlphaMask = GX_LIGHT_NULL;
        mIsSpotLight = true;

        mFogType = GX_FOG_NONE;
        mFogStartz = 0.0f;
        mFogEndz = 1.0f;
        mFogNearz = 0.0f;
        mFogFarz = 1.0f;
        mZOffset = 0.0f;
        mZOffsetOrig.x = 0.0f;
        mZOffsetOrig.y = 0.0f;
        mZOffsetOrig.z = 0.0f;
        mChanMatColor.r = mChanMatColor.g = mChanMatColor.b = 255;
        mChanMatColor.a = 255;
        mChanAmbColor.r = mChanAmbColor.g = mChanAmbColor.b = 0;
        mChanAmbColor.a = 255;
    }

    void SetZOffset(f32 offset, const math::VEC3 &pos) {
        mZOffset = offset;
        mZOffsetOrig = pos;
    }

    void SetAmbientColor(GXColor color) {
        mChanAmbColor = color;
    }
    void SetFog(GXFogType type, f32 start, f32 end, f32 nearZ, f32 farZ,
                GXColor color) {
        mFogType = type;
        mFogStartz = start;
        mFogEndz = end;
        mFogNearz = nearZ;
        mFogFarz = farZ;
        mFogColor = color;
    }

    const math::MTX34 *GetViewMtx() const {
        return &mViewMtx;
    }
    math::MTX34 &GetViewMtxCooked(math::MTX34 *result) const {
        *result = mViewMtx;

        if (mZOffset != 0.0f) {
            f32 proj[GX_PROJECTION_SZ];
            GXGetProjectionv(proj);

            switch (static_cast<GXProjectionType>(proj[0])) {
            case GX_PERSPECTIVE: {
                math::VEC3 emPos;
                math::VEC3TransformCoord(&emPos, &mViewMtx, &mZOffsetOrig);

                if (Normalize(&emPos, &emPos)) {
                    if (emPos.z >= 0.0f) {
                        result->_03 += emPos.x * mZOffset;
                        result->_13 += emPos.y * mZOffset;
                        result->_23 += emPos.z * mZOffset;
                    } else {
                        result->_03 -= emPos.x * mZOffset;
                        result->_13 -= emPos.y * mZOffset;
                        result->_23 -= emPos.z * mZOffset;
                    }
                } else {
                    result->_23 += mZOffset;
                }
                break;
            }

            case GX_ORTHOGRAPHIC: {
                result->_23 += mZOffset;
                break;
            }
            }
        }

        return *result;
    }

    void SetViewMtx(const math::MTX34 &rMtx) {
        mViewMtx = rMtx;
    }

    const math::MTX34 *GetProjMtx() const {
        return &mProjMtx;
    }
    void SetProjMtx(const math::MTX34 &rMtx) {
        mProjMtx = rMtx;
    }

    bool IsLightEnable() const {
        return mLightEnable;
    }

    GXLightID GetLightMask() const {
        return mLightMask;
    }

    GXLightID GetLightAlphaMask() const {
        return mLightAlphaMask;
    }

    bool IsSpotLight() const {
        return mIsSpotLight;
    }

    GXColor GetChanMatColor() const {
        return mChanMatColor;
    }

    GXColor GetChanAmbColor() const {
        return mChanAmbColor;
    }

    void GetFog(GXFogType *pType, f32 *pStartZ, f32 *pEndZ, f32 *pNearZ,
                f32 *pFarZ, GXColor *pColor) const {

        *pType = mFogType;
        *pStartZ = mFogStartz;
        *pEndZ = mFogEndz;
        *pNearZ = mFogNearz;
        *pFarZ = mFogFarz;
        *pColor = mFogColor;
    }
};

} // namespace ef
} // namespace nw4r

#endif
