// The waiting icon on a black screen (dBlackWait). .text 800805BC..80080D7C, .rodata 8046D428..8046D460,
// .data 804A7648..804A76B8, .sbss 8074E280, .sdata2 807504F0..80750520. See include/game/game/d_black_wait.hpp.
#include <lib/egg/gfx/eggColor.h>
#include <game/game/d_black_wait.hpp>
#include <game/game/d_reset.hpp>
#include <game/mLib/m_fader.hpp>
#include <lib/egg/core/eggHeap.h>
#include <lib/egg/gfxe/eggDrawGX.h>
#include <lib/egg/gfxe/eggScreen.h>
#include <revolution/gx.h>
#include <revolution/os.h>

namespace dBlackWait {

#define BLACK_WAIT_DELAY_MS 2000.0f // black time before the icon shows
#define BLACK_WAIT_ICON_FRAMES 45

static BlackWait_c *sInstance;

static const char *const l_texName[BLACK_WAIT_ICON_NUM] = {
    "BlackWait/waitIcon_00.brtex",
    "BlackWait/waitIcon_01.brtex",
    "BlackWait/waitIcon_02.brtex",
    "BlackWait/waitIcon_03.brtex",
};

static void (BlackWait_c::*const l_stepProc[])() = {
    &BlackWait_c::stepLoad,
    &BlackWait_c::stepWait,
    &BlackWait_c::stepAnim,
};

// 800805BC
void BlackWait_c::create(EGG::Heap *heap) {
    sInstance = new (heap, 4) BlackWait_c(heap);
}

// 80080608
BlackWait_c *BlackWait_c::getInstance() {
    return sInstance;
}

// 80080610
BOOL BlackWait_c::load() {
    BOOL done = TRUE;
    for (int i = 0; i < BLACK_WAIT_ICON_NUM; i++) {
        nw4r::g3d::ResTex *tex = &mTex[i];
        if (!tex->IsValid()) {
            void *data = mLoader[i].request(l_texName[i], 0, mpHeap);
            if (data != nullptr) {
                nw4r::g3d::ResFile file(data);
                *tex = file.GetResTex(0);
                tex->Init();
            }
            done = FALSE;
        }
    }
    if (done) {
        mState = STATE_WAIT;
    }
    return done;
}

// 800806C8
void BlackWait_c::execute() {
    (this->*l_stepProc[mState])();
}

// 80080700
void BlackWait_c::draw() {
    if (mFrame >= 0 && mFrame < BLACK_WAIT_ICON_NUM && !mHide) {
        EGG::Screen screen;
        screen.SetCanvasMode(EGG::Frustum::CANVASMODE_0);
        screen.mProjType = EGG::Frustum::PROJ_ORTHO;
        screen.mNearZ = -10000.0f;
        screen.mFarZ = 10000.0f;
        screen.SetProjectionGX();

        nw4r::math::VEC2 size(68.0f, 20.0f);
        nw4r::math::VEC2 pos;
        u16 halfWidth = EGG::Screen::sTVModeInfo[EGG::Screen::sTVMode].width / 2;
        pos.x = halfWidth;
        if (EGG::Screen::sTVMode == EGG::Screen::TV_MODE_16_9) {
            pos.x += -104.0f;
        } else {
            pos.x += -90.0f;
        }
        pos.y = -170.0f;

        EGG::Color color0(0x60, 0xC3, 0xAE, 0x00);
        EGG::Color color1(0x60, 0xC3, 0xAE, 0xFF);

        EGG::DrawGX::SetMat_ColorChannel(EGG::DrawGX::COLORCHAN_1);
        EGG::DrawGX::SetMat_TexGen(EGG::DrawGX::TEXGEN_1);
        EGG::DrawGX::SetMat_Ind();
        GXSetNumTevStages(1);
        GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
        GXSetTevDirect(GX_TEVSTAGE0);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColor(GX_TEVREG0, color0);
        GXSetTevColor(GX_TEVREG1, color1);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_C1, GX_CC_TEXC, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_A0, GX_CA_A1, GX_CA_TEXA, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        EGG::DrawGX::SetMat_PE(EGG::DrawGX::ZMODE_0, EGG::DrawGX::BLEND_0);
        EGG::DrawGX::SetVtxState(EGG::DrawGX::VTX_TYPE_10);
        EGG::DrawGX::LoadTexture(mTex[mFrame], GX_TEXMAP0);

        f32 top = pos.y + size.y;
        f32 bottom = pos.y - size.y;
        f32 left = pos.x - size.x;
        f32 right = pos.x + size.x;
        nw4r::math::MTX34 mtx;
        screen.CalcMatrixForDrawQuad(&mtx, left, top, nw4r::math::FAbs(right - left), nw4r::math::FAbs(bottom - top));
        EGG::DrawGX::DrawDL(EGG::DrawGX::DL_16, mtx);
    }
}

// 800809D0
BlackWait_c::BlackWait_c(EGG::Heap *heap) : mpHeap(heap) {
    mState = STATE_LOAD;
    mStartTick = 0;
    mFrame = -1;
    mTimer = 0;
    mHide = 0;
}

// 80080B54
void BlackWait_c::stepLoad() {}

// 80080B58
void BlackWait_c::stepWait() {
    if (isBlack()) {
        if (mStartTick == 0) {
            mStartTick = OSGetTick();
        } else if (OSTicksToMilliseconds(OSGetTick() - mStartTick) >
                   BLACK_WAIT_DELAY_MS) {
            mState = STATE_ANIM;
            mFrame = 0;
            mTimer = 0;
        }
    } else {
        mStartTick = 0;
    }
}

// 80080C14
void BlackWait_c::stepAnim() {
    if (isBlack()) {
        if (++mTimer >= BLACK_WAIT_ICON_FRAMES) {
            mTimer = 0;
            if (++mFrame >= BLACK_WAIT_ICON_NUM) {
                mFrame = 0;
            }
        }
    } else {
        mState = STATE_WAIT;
        mFrame = -1;
        mStartTick = 0;
        mHide = 0;
    }
}

// 80080C9C
BOOL BlackWait_c::isBlack() {
    BOOL black = TRUE;
    if (!mFader_c::isStatus(mFaderBase_c::OPAQUE) && !dReset::Manage_c::GetInstance()->IsFadeOutDone()) {
        black = FALSE;
    }
    return black;
}

} // namespace dBlackWait
