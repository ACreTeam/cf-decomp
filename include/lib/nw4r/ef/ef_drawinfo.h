#ifndef NW4R_EF_DRAW_INFO_H
#define NW4R_EF_DRAW_INFO_H
#include <nw4r/types_nw4r.h>

#include <nw4r/math.h>

#include <revolution/GX.h>

namespace nw4r {
namespace ef {

class DrawInfo {
public:
    math::MTX34 mViewMtx; // at 0x0
    math::MTX34 mProjMtx; // at 0x30
    bool mLightEnable;    // at 0x60
    GXLightID mLightMask; // at 0x64
    u32 mLightType; // City Folk: 0x68
    bool mIsSpotLight;    // at 0x6C
    GXFogType mFogType;   // at 0x70
    f32 mFogStartz;       // at 0x74
    f32 mFogEndz;         // at 0x78
    f32 mFogNearz;        // at 0x7C
    f32 mFogFarz;         // at 0x80
    GXColor mFogColor;    // at 0x84
    f32 mZOffset;         // at 0x88
    f32 mZScale;          // at 0x8C
    f32 mClipNear;        // at 0x90
    f32 mClipFar;         // at 0x94
    GXColor mMaterialColor; // at 0x98
    GXColor mAmbientColor;  // at 0x9C

public:
    DrawInfo() {
        math::MTX34Identity(&mViewMtx);
        math::MTX34Identity(&mProjMtx);

        mLightEnable = false;
        mLightMask = GX_LIGHT_NULL;
        mLightType = 0;
        mIsSpotLight = true;

        mFogType = GX_FOG_NONE;
        mFogStartz = 0.0f;
        mFogEndz = 1.0f;
        mFogNearz = 0.0f;
        mFogFarz = 1.0f;
        mZOffset = 0.0f;
        mZScale = 0.0f;
        mClipNear = 0.0f;
        mClipFar = 0.0f;
        mMaterialColor.r = mMaterialColor.g = mMaterialColor.b = 255;
        mMaterialColor.a = 255;
        mAmbientColor.r = mAmbientColor.g = mAmbientColor.b = 0;
        mAmbientColor.a = 255;
    }

    void SetAmbientColor(GXColor color) { mAmbientColor = color; }
    void SetFog(GXFogType type, f32 start, f32 end, f32 nearZ, f32 farZ, GXColor color) {
        mFogType = type;
        mFogStartz = start; mFogEndz = end; mFogNearz = nearZ; mFogFarz = farZ;
        mFogColor = color;
    }

    const math::MTX34* GetViewMtx() const {
        return &mViewMtx;
    }
    void SetViewMtx(const math::MTX34& rMtx) {
        mViewMtx = rMtx;
    }

    const math::MTX34* GetProjMtx() const {
        return &mProjMtx;
    }
    void SetProjMtx(const math::MTX34& rMtx) {
        mProjMtx = rMtx;
    }

    bool IsLightEnable() const {
        return mLightEnable;
    }

    GXLightID GetLightMask() const {
        return mLightMask;
    }

    bool IsSpotLight() const {
        return mIsSpotLight;
    }

    void GetFog(GXFogType* pType, f32* pStartZ, f32* pEndZ, f32* pNearZ,
                f32* pFarZ, GXColor* pColor) const {

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
