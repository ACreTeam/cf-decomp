// EGG::GfxEngine::Configuration + EGG::GXUtility helpers. .text 80448360..804485FC.
#include <lib/egg/core/eggSystem.h>
#include <lib/egg/gfxe/eggGXUtility.h>
#include <lib/egg/gfxe/eggGfxEngine.h>

namespace EGG {

GfxEngine::Configuration::Configuration()
    : m_00(640), m_02(528), mColor((GXColor){0, 0, 0, 0}), mPixelFmt(GX_PF_RGB8_Z24), m_0C(32), m_0E(256),
      mBufferSize(0), m_14(608), m_16(832), m_18(0) {
    if (BaseSystem::mConfigData->getVideo() != NULL) {
        m_00 = BaseSystem::mConfigData->getVideo()->pRenderMode->fbWidth;
        m_02 = BaseSystem::mConfigData->getVideo()->pRenderMode->efbHeight;
    }
    if (BaseSystem::mConfigData->getDisplay() != NULL) {
        mColor = BaseSystem::mConfigData->getDisplay()->getClearColor();
    }
    mBufferSize = GXGetTexBufferSize(m_00, m_02, GX_TF_RGBA8, GX_FALSE, 0) * 3;
}

void GXUtility::setScaleOffsetPerspective(f32 *pOut, f32 f1, f32 f2, f32 f3, f32 f4) {
    pOut[1] *= (1.0f / f1);
    pOut[3] *= (1.0f / f2);
    pOut[2] += f3;
    pOut[4] += f4;
}

void GXUtility::getTexObj(GXTexObj *obj, nw4r::g3d::ResTex resTex, GXTexWrapMode wrapS, GXTexWrapMode wrapT,
                          GXTexFilter minFilter, GXTexFilter magFilter) {
    void *pTexData;
    u16 width;
    u16 height;
    GXTexFmt format;
    f32 minLod;
    f32 maxLod;
    GXBool mipMap;
    if (resTex.GetTexObjParam(&pTexData, &width, &height, &format, &minLod, &maxLod, &mipMap)) {
        GXInitTexObj(obj, pTexData, width, height, format, wrapS, wrapT, mipMap);
        GXInitTexObjLOD(obj, minFilter, magFilter, minLod, maxLod, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
    }
}

} // namespace EGG
