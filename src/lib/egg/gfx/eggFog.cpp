// EGG::Fog. .text 804475A0..80447798
#include <lib/egg/gfxe/eggDrawGX.h>
#include <lib/egg/gfxe/eggFog.h>

namespace EGG {

template <>
const char *IBinary<Fog>::GetBinaryType() {
    return "FOGD";
}

template <>
int IBinary<Fog>::GetVersion() {
    return 0;
}

Fog::Fog() {
    Reset();
}

void Fog::Reset() {
    mFlags = 0;
    mColor = DrawGX::WHITE;
    mFarZ = 0.0f;
    mNearZ = 0.0f;
    mEndZ = 0.0f;
    mStartZ = 0.0f;
    mFogType = GX_FOG_NONE;
    mFlags = 0;
}

void Fog::CopyToG3D(nw4r::g3d::Fog fog) const {
    fog.SetFogType((mFlags & 1) ? mFogType : GX_FOG_NONE);
    fog.SetZ(mStartZ, mEndZ);
    fog.SetFogColor(mColor);
    fog.SetFogRangeAdjEnable(false);
}

void Fog::SetBinaryInner(const Bin &bin) {
    if (bin.mHeader.mVersion != 0) {
        return;
    }
    f32 startZ = bin.mData.mStartZ;
    f32 endZ = bin.mData.mEndZ;
    f32 nearZ = bin.mData.mNearZ;
    f32 farZ = bin.mData.mFarZ;
    mStartZ = startZ;
    mEndZ = endZ;
    mNearZ = nearZ;
    mFarZ = farZ;
    mColor = bin.mData.mColor;
    mFogType = (GXFogType)bin.mData.mFogType;
    mFlags = bin.mData.mFlags;
}

void Fog::GetBinaryInner(Bin *pBin) const {
    f32 startZ = mStartZ;
    f32 endZ = mEndZ;
    f32 nearZ = mNearZ;
    f32 farZ = mFarZ;
    pBin->mData.mStartZ = startZ;
    pBin->mData.mEndZ = endZ;
    pBin->mData.mNearZ = nearZ;
    pBin->mData.mFarZ = farZ;
    pBin->mData.mColor = mColor;
    pBin->mData.mFogType = mFogType;
    pBin->mData.mFlags = mFlags;
}

} // namespace EGG
