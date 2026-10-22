#include <lib/egg/core/eggAsyncDisplay.h>

#include <lib/egg/core/eggSystem.h>
#include <lib/egg/core/eggVideo.h>
#include <lib/egg/core/eggXfbManager.h>
#include <lib/egg/math/eggMatrix.h>
#include <revolution/GX.h>
#include <revolution/MTX.h>
#include <revolution/OS.h>
#include <revolution/VI.h>

namespace {

static GXTexObj clear_z_tobj;

static u32 clear_z_TX[4 * 4] ALIGN(32) = {
    0x00FF00FF, 0x00FF00FF, 0x00FF00FF, 0x00FF00FF, 0x00FF00FF, 0x00FF00FF, 0x00FF00FF, 0x00FF00FF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
};

} // namespace

namespace EGG {

static AsyncDisplay *spSelector = NULL;

AsyncDisplay::AsyncDisplay(u8 frameRate) : Display(frameRate) {
    mSyncMode = 0;
    mTimeScale = 1.0f;
    mRetraceCountA = 0;
    mRetraceCountB = 0;
    mPendingFrames = 0;
    _B8 = 0;
    spSelector = this;

    OSInitThreadQueue(&mSyncQueue);
    OSInitMessageQueue(&mMsgQueue, mMsgBuf, 4);
}

void AsyncDisplay::calcTimeScale() {
    mTimeScale += mTimeStep;

    if (mTimeScale > 1.0f) {
        if (mScreenStateFlag & 0x10) {
            mTimeScale -= 1.0f;
            mScreenStateFlag &= ~0x10;
        } else {
            mTimeScale -= mTimeStep;
            mScreenStateFlag |= 0x10;
        }
    }
}

void AsyncDisplay::beginFrame() {
    BaseSystem::mConfigData->getXfbMgr();
    GXDrawDone();

    if (onBit(1)) {
        OSSleepThread(&mSyncQueue);
    }

    bool clear = false;
    if (BaseSystem::mConfigData->getXfbMgr()->m_0c == 1) {
        if ((u32)(OSGetTick() - mRetraceTick) > sTickThreshold) {
            VIWaitForRetrace();
        }
    } else {
        clear = false;
        if (mSyncMode == 1 && (mScreenStateFlag & 0x40)) {
            clear = true;
        }

        while ((BaseSystem::mConfigData->getXfbMgr()->m_0c - mPendingFrames) <= 1 ||
               (BaseSystem::mConfigData->getXfbMgr()->m_04 == 0 && !clear)) {
            OSReceiveMessage(&mMsgQueue, mMsgBuf, OS_MSG_BLOCKING);
            while (OSReceiveMessage(&mMsgQueue, mMsgBuf, 0)) {
            }
        }
    }

    if (clear) {
        clearEFB();
        mScreenStateFlag &= ~0x40;
        GXDrawDone();
    } else {
        copyEFBtoXFB();
    }

    if (mSyncMode == 1) {
        calcTimeScale();
    }

    mPendingFrames++;
    mFrameCount++;
    calcFrequency();
}

void AsyncDisplay::endFrame() {
    if (mScreenStateFlag & 0x10) {
        mScreenStateFlag |= 0x20;
    }
}

void AsyncDisplay::endRender() {
    if (mScreenStateFlag & 0x20) {
        mScreenStateFlag = (mScreenStateFlag | 0x40) & ~0x20;
    }
}

void AsyncDisplay::postVRetrace() {
    mRetraceTick = OSGetTick();

    if (mSyncMode == 0) {
        mRetraceCountB++;
        if (mRetraceCountB - mRetraceCountA >= mFrameRate) {
            mScreenStateFlag |= 0x08;
            mRetraceCountA = mRetraceCountB;
            OSSendMessage(&mMsgQueue, mMsgBuf, 0);
            if (mPendingFrames > 0) {
                mPendingFrames--;
            }
        }
        OSWakeupThread(&mSyncQueue);
    }

    if (mScreenStateFlag & 0x08) {
        if (VIGetRetraceCount() - mRetraceCount >= mFrameRate) {
            if (mScreenStateFlag & 1) {
                BaseSystem::mConfigData->getVideo()->changeBlack();
                mScreenStateFlag &= ~1;
            }

            BaseSystem::mConfigData->getXfbMgr()->setNextFrameBuffer();
            mRetraceCount = VIGetRetraceCount();
            mScreenStateFlag &= ~0x08;
        }
    }
}

void AsyncDisplay::clearEFB() {
    const GXRenderModeObj *rm = BaseSystem::mConfigData->getVideo()->pRenderMode;
    clearEFB(rm->fbWidth, rm->efbHeight, 0, 0, rm->fbWidth, rm->efbHeight, mClearColor);
}

u32 AsyncDisplay::getTickPerFrame() {
    if (mSyncMode == 1) {
        return Video::getTickPerVRetrace(0);
    }
    return Video::getTickPerVRetrace();
}

void AsyncDisplay::clearEFB(u16 fbWidth, u16 fbHeight, u16 x, u16 y, u16 width, u16 height,
                            nw4r::ut::Color color) {
    GXInitTexObj(&clear_z_tobj, clear_z_TX, 4, 4, GX_TF_Z24X8, GX_REPEAT, GX_REPEAT, GX_FALSE);
    GXInitTexObjLOD(&clear_z_tobj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);

    Mtx44 projMtx;
    C_MTXOrtho(projMtx, 0.0f, fbHeight, 0.0f, fbWidth, 0.0f, 1.0f);
    GXSetProjection(projMtx, GX_ORTHOGRAPHIC);

    GXSetViewport(0.0f, 0.0f, fbWidth, fbHeight, 0.0f, 1.0f);
    GXSetScissor(0, 0, fbWidth, fbHeight);

    Matrix34f::ident.loadPosMtx(GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_U16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U8, 0);

    GXSetNumChans(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);

    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXLoadTexObj(&clear_z_tobj, GX_TEXMAP0);

    GXSetNumTevStages(1);
    GXSetTevColor(GX_TEVREG0, color);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetZTexture(GX_ZT_REPLACE, GX_TF_Z24X8, 0);
    GXSetZCompLoc(GX_FALSE);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ZERO, GX_BL_ZERO, GX_LO_NOOP);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetDstAlpha(GX_TRUE, color.a);
    GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
    GXSetCullMode(GX_CULL_BACK);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition2u16(x, y);
    GXTexCoord2u8(0, 0);
    GXPosition2u16(x + width, y);
    GXTexCoord2u8(1, 0);
    GXPosition2u16(x + width, y + height);
    GXTexCoord2u8(1, 1);
    GXPosition2u16(x, y + height);
    GXTexCoord2u8(0, 1);
    GXEnd();

    GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z24X8, 0);
    GXSetZCompLoc(GX_TRUE);
    GXSetDstAlpha(GX_FALSE, color.a);
}

} // namespace EGG
