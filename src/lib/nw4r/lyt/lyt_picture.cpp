#include <nw4r/lyt.h>
#include <nw4r/ut.h>

namespace nw4r {
namespace lyt {

NW4R_UT_RTTI_DEF_DERIVED(Picture, Pane);

Picture::Picture(u8 num) : Pane(NULL) {
    num = ut::Min<u8>(num, GX_MAX_TEXCOORD);
    Init(num);
}

Picture::Picture(const res::Picture* pResPic, const ResBlockSet& resBlockSet)
    : Pane(pResPic) {
    u8 texCoordNum = ut::Min<u8>(pResPic->texCoordNum, GX_MAX_TEXCOORD);

    Init(texCoordNum);

    for (int i = 0; i < VERTEXCOLOR_MAX; i++) {
        mVtxColors[i] = pResPic->vtxCols[i];
    }

    if (texCoordNum != 0 && !mTexCoordAry.IsEmpty()) {
        mTexCoordAry.Copy(&pResPic[1], texCoordNum);
    }

    const u32* const matOffsTbl = detail::ConvertOffsToPtr<u32>(
        resBlockSet.pMaterialList, sizeof(res::MaterialList));

    const res::Material* const pResMaterial =
        detail::ConvertOffsToPtr<res::Material>(
            resBlockSet.pMaterialList, matOffsTbl[pResPic->materialIdx]);

    mpMaterial = Layout::NewObj<Material>(pResMaterial, resBlockSet);
}

void Picture::Init(u8 texNum) {
    if (texNum != 0) {
        ReserveTexCoord(texNum);
    }
}

Picture::~Picture() {
    if (mpMaterial != NULL && !mpMaterial->IsUserAllocated()) {
        Layout::DeleteObj(mpMaterial);
        mpMaterial = NULL;
    }

    mTexCoordAry.Free();
}

void Picture::Append(const TexMap& texMap) {
    if (mpMaterial->GetTextureNum() >= mpMaterial->GetTextureCap() ||
        mpMaterial->GetTextureNum() >= mpMaterial->GetTexCoordGenCap()) {
        return;
    }

    u8 texIdx = mpMaterial->GetTextureNum();

    mpMaterial->SetTextureNum(texIdx + 1);
    mpMaterial->SetTexture(texIdx, texMap);

    mpMaterial->SetTexCoordGenNum(mpMaterial->GetTextureNum());
    mpMaterial->SetTexCoordGen(texIdx, TexCoordGen());

    SetTexCoordNum(mpMaterial->GetTextureNum());

    if (mSize == Size(0.0f, 0.0f) && mpMaterial->GetTextureNum() == 1) {
        mSize = detail::GetTextureSize(mpMaterial, 0);
    }
}

void Picture::ReserveTexCoord(u8 num) {
    mTexCoordAry.Reserve(num);
}

void Picture::SetTexCoordNum(u8 num) {
    mTexCoordAry.SetSize(num);
}

ut::Color Picture::GetVtxColor(u32 idx) const {
    return mVtxColors[idx];
}

void Picture::SetVtxColor(u32 idx, ut::Color color) {
    mVtxColors[idx] = color;
}

u8 Picture::GetVtxColorElement(u32 idx) const {
    return detail::GetVtxColorElement(mVtxColors, idx);
}

void Picture::SetVtxColorElement(u32 idx, u8 value) {
    detail::SetVtxColorElement(mVtxColors, idx, value);
}

void Picture::DrawSelf(const DrawInfo& drawInfo) {
    if (mpMaterial == NULL) {
        return;
    }

    LoadMtx(drawInfo);

    bool useVtxColor = mpMaterial->SetupGX(
        detail::IsModulateVertexColor(mVtxColors, mGlbAlpha), mGlbAlpha);

    detail::SetVertexFormat(useVtxColor, mTexCoordAry.GetSize());

    detail::DrawQuad(GetVtxPos(), mSize, mTexCoordAry.GetSize(),
                     mTexCoordAry.GetArray(), useVtxColor ? mVtxColors : NULL,
                     mGlbAlpha);
}

} // namespace lyt
} // namespace nw4r
