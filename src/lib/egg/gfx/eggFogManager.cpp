// EGG::FogManager. .text 80447798..80447BE8
#include <lib/egg/gfxe/eggFogManager.h>

namespace EGG {

template <>
const char *IBinary<FogManager>::GetBinaryType() {
    return "FOGM";
}

FogManager::FogManager(unsigned short count) : mFlags(0), mCount(count), mCount2(count) {
    mFog = new Fog *[count];
    for (int i = 0; i < mCount; i++) {
        mFog[i] = new Fog();
    }
}

void FogManager::Calc() {
    if (!(mFlags & FLAG_CALC_DONE)) {
        mFlags |= FLAG_CALC_DONE;
    }
}

void FogManager::CopyToG3D(nw4r::g3d::ScnRoot *root) const {
    for (int i = 0; i < mCount; i++) {
        mFog[i]->CopyToG3D(root->GetFog(i));
    }
}

void FogManager::SetBinaryInner(const Bin &bin) {
    switch (bin.mHeader.mVersion) {
    case 0: {
        u32 loopMax = bin.mData.mCount >= mCount2 ? mCount2 : bin.mData.mCount;
        const IBinary<Fog>::Bin *pBinData = bin.mData.mFogData;
        for (u16 i = 0; i < loopMax; i++) {
            mFog[i]->SetBinary(&pBinData[i]);
        }
        break;
    }
    }
}

void FogManager::GetBinaryInner(Bin *pOutBin) const {
    pOutBin->mData.mCount = mCount2;
    u16 max = mCount2;
    IBinary<Fog>::Bin *pOutData = pOutBin->mData.mFogData;
    for (u16 i = 0; i < max; i++) {
        mFog[i]->GetBinary(&pOutData[i]);
    }
}

u32 FogManager::GetBinarySize() const {
    return sizeof(Bin) + (mCount2 - 1) * sizeof(Fog::Bin);
}

} // namespace EGG
