#include <game/mLib/m_3d/face_ex.hpp>

#include <revolution/GX.h>
#include <revolution/RFL.h>

// m3d::bface_c / m3d::faceEx_c (City Folk only, no Skyward Sword counterpart).

void m3d::bface_c::drawOpa() {
    if (mModel.isReady()) {
        nw4r::math::MTX34 mtx;
        getViewMtx(&mtx);
        mModel.setMtx(&mtx);
        drawOpaCore();
    }
}

void m3d::bface_c::drawXlu() {
    if (mModel.isReady()) {
        nw4r::math::MTX34 mtx;
        getViewMtx(&mtx);
        mModel.setMtx(&mtx);
        drawXluCore();
    }
}

void m3d::bface_c::remove() {
    mModel.remove();
    scnLeaf_c::remove();
}

bool m3d::bface_c::create(
    RFLDataSource source, u16 index, RFLResolution resolution, u32 expressionFlag,
    mAllocator_c *allocator, RFLMiddleDB *middleDB
) {
    if (!proc_c::create(allocator, nullptr)) {
        return false;
    }
    return mModel.create(source, index, resolution, expressionFlag, allocator, middleDB);
}

m3d::faceEx_c::faceEx_c() {
    mDrawSetting.txcGenNum = 1;
    mDrawSetting.txcID = GX_TEXCOORD0;
    mDrawSetting.texMapID = GX_TEXMAP0;
    mDrawSetting.tevStageNum = 1;
    mDrawSetting.tevSwapTable = GX_TEV_SWAP0;
    mDrawSetting.tevKColorID = GX_KCOLOR0;
    mDrawSetting.tevOutRegID = GX_TEVPREV;
    mDrawSetting.posNrmMtxID = GX_PNMTX0;
    mDrawSetting.reverseCulling = FALSE;
}

void m3d::faceEx_c::drawOpaCore() {
    RFLLoadVertexSetting(&mDrawSetting);
    RFLLoadMaterialSetting(&mDrawSetting);
    GXSetZCompLoc(GX_TRUE);
    setupDrawOpa();
    RFLDrawOpaCore(&mModel, &mDrawSetting);
}

void m3d::faceEx_c::drawXluCore() {
    RFLLoadVertexSetting(&mDrawSetting);
    RFLLoadMaterialSetting(&mDrawSetting);
    GXSetZCompLoc(GX_FALSE);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_NEVER, 0);
    setupDrawXlu();
    RFLDrawXluCore(&mModel, &mDrawSetting);
}
