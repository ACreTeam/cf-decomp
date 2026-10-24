#include <nw4r/lyt.h>

#include <revolution/gx.h>
#include <revolution/tpl.h>

namespace nw4r {
namespace lyt {

void TexMap::Get(GXTexObj* pTexObj) const {
    if (detail::IsCITexelFormat(GetTexelFormat())) {
        ulong tlutName = GXGetTexObjTlut(pTexObj);

        GXInitTexObjCI(pTexObj, GetImage(), GetWidth(), GetHeight(),
                       static_cast<GXCITexFmt>(GetTexelFormat()),
                       GetWrapModeS(), GetWrapModeT(), IsMipMap(), tlutName);
    } else {
        GXInitTexObj(pTexObj, GetImage(), GetWidth(), GetHeight(),
                     GetTexelFormat(), GetWrapModeS(), GetWrapModeT(),
                     IsMipMap());
    }

    GXInitTexObjLOD(pTexObj, GetMinFilter(), GetMagFilter(), GetMinLOD(),
                    GetMaxLOD(), GetLODBias(), IsBiasClampEnable(),
                    IsEdgeLODEnable(), GetAnisotropy());
}

void TexMap::Get(GXTlutObj* pTLUTObj) const {
    GXInitTlutObj(pTLUTObj, GetPalette(), GetPaletteFormat(),
                  GetPaletteEntryNum());
}

void TexMap::Set(const GXTexObj& texObj) {
    void* pImage;
    u16 width, height;
    GXTexFmt format;
    GXTexWrapMode wrapS, wrapT;
    GXBool mipMap;
    GXGetTexObjAll(&texObj, &pImage, &width, &height, &format, &wrapS,
                   &wrapT, &mipMap);

    SetImage(pImage);
    SetSize(width, height);
    SetTexelFormat(format);
    SetWrapMode(wrapS, wrapT);
    SetMipMap(mipMap);

    GXTexFilter minFilter, magFilter;
    f32 minLOD, maxLOD, LODBias;
    GXBool biasClampEnable, edgeLODEnable;
    GXAnisotropy anisotropy;
    GXGetTexObjLODAll(&texObj, &minFilter, &magFilter, &minLOD, &maxLOD,
                      &LODBias, &biasClampEnable, &edgeLODEnable,
                      &anisotropy);

    SetFilter(minFilter, magFilter);
    SetLOD(minLOD, maxLOD);
    SetLODBias(LODBias);
    SetBiasClampEnable(biasClampEnable);
    SetEdgeLODEnable(edgeLODEnable);
    SetAnisotropy(anisotropy);
}

void TexMap::Set(TPLPalette* pal, ulong id) {
    if (reinterpret_cast<ulong>(pal->descriptorArray) < 0x80000000) {
        TPLBind(pal);
    }

    Set(TPLGet(pal, id));
}

void TexMap::Set(const TPLDescriptor* pTPLDesc) {
    const TPLHeader& tplHead = *pTPLDesc->textureHeader;

    SetImage(tplHead.data);
    SetSize(tplHead.width, tplHead.height);
    SetTexelFormat(static_cast<GXTexFmt>(tplHead.format));
    SetWrapMode(tplHead.wrapS, tplHead.wrapT);

    SetMipMap(tplHead.minLOD != tplHead.maxLOD);
    SetFilter(tplHead.minFilter, tplHead.magFilter);
    SetLOD(static_cast<f32>(tplHead.minLOD), static_cast<f32>(tplHead.maxLOD));
    SetLODBias(tplHead.LODBias);
    SetEdgeLODEnable(tplHead.edgeLODEnable);

    const TPLClutHeader* const pTPLCLUTHeader = pTPLDesc->CLUTHeader;

    if (pTPLCLUTHeader != NULL) {
        SetPalette(pTPLCLUTHeader->data);
        SetPaletteFormat(pTPLCLUTHeader->format);
        SetPaletteEntryNum(pTPLCLUTHeader->numEntries);
    } else {
        SetPalette(NULL);
        SetPaletteFormat(GX_TL_IA8);
        SetPaletteEntryNum(0);
    }
}

void TexMap::ReplaceImage(TPLPalette* pal, ulong id) {
    if (reinterpret_cast<ulong>(pal->descriptorArray) < 0x80000000) {
        TPLBind(pal);
    }

    ReplaceImage(TPLGet(pal, id));
}

void TexMap::ReplaceImage(const TPLDescriptor* pTPLDesc) {
    const TPLHeader& tplHead = *pTPLDesc->textureHeader;

    SetImage(tplHead.data);
    SetSize(tplHead.width, tplHead.height);
    SetTexelFormat(static_cast<GXTexFmt>(tplHead.format));

    const TPLClutHeader* const pTPLCLUTHeader = pTPLDesc->CLUTHeader;

    if (pTPLCLUTHeader != NULL) {
        SetPalette(pTPLCLUTHeader->data);
        SetPaletteFormat(pTPLCLUTHeader->format);
        SetPaletteEntryNum(pTPLCLUTHeader->numEntries);
    } else {
        SetPalette(NULL);
        SetPaletteFormat(GX_TL_IA8);
        SetPaletteEntryNum(0);
    }
}

} // namespace lyt
} // namespace nw4r
