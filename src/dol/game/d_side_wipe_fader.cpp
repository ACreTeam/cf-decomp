// The side wipe fader and the shutter fader helpers. .text 80160AE4..80160EA4,
// .data 804F0710..804F0768, .sdata 8074B2B8..8074B2C0, .sbss 8074E828..8074E830,
// .sdata2 80750E78..80750E98.
#include <game/game/d_side_wipe_fader.hpp>
#include <revolution/MTX.h>

// Not split yet (C linkage keeps the target names).
extern "C" {
void fn_802466E8(u32 mask);
void *fn_8017CD3C(); // the wipe texture
int fn_8017CD48(); // its width
int fn_8017CD50(); // its height
void fn_80390CE8(Mtx m, f32 x, f32 y, f32 z); // PSMTXScale
void fn_803905EC(const Mtx a, const Mtx b, Mtx ab); // PSMTXConcat
}

static dShutterFader_c *sShutterFader; // 8074E828

// 80160AE4
bool startShutterFadeOut() {
    if (!isShutterFaderHidden()) {
        return false;
    }
    sShutterFader->setFrame(8);
    return sShutterFader->fadeOut();
}

// 80160B34
bool isShutterFaderHidden() {
    return sShutterFader->isStatus(mFaderBase_c::HIDDEN);
}

// 80160B70
void calcShutterFader() {
    sShutterFader->calc();
    if (sShutterFader->getStatus() == mFaderBase_c::OPAQUE) {
        sShutterFader->setFrame(8);
        sShutterFader->fadeIn();
    }
}

// 80160BDC
void drawShutterFader() {
    fn_802466E8(0x7FF);
    sShutterFader->draw();
}

// 80160C14
BOOL createShutterFader() {
    sShutterFader = new dShutterFader_c();
    if (sShutterFader == NULL) {
        return FALSE;
    }
    sShutterFader->init();
    return TRUE;
}

// 80160C60
dSideWipeFader_c::dSideWipeFader_c(mColor col, mFaderBase_c::EStatus status) : mWipeFader_c(col, status) {}

// 80160CC0
void dSideWipeFader_c::setDefaultTexture() {
    setTexture(fn_8017CD3C(), fn_8017CD48(), fn_8017CD50());
}

// 80160D2C
void dSideWipeFader_c::calcMtx() {
    f32 scale = 196.0f / (256 - mProgress);
    f32 scrW = mAspectRatioFactor * 608.0f;
    f32 scrH = 456.0f;
    f32 x = mCenterX + (0.5f - mCenterX) / scale;
    f32 y = mCenterY + (0.5f - mCenterY) / scale;
    PSMTXTrans(mTexMtx, x, y, 0.0f);

    Mtx scaleMtx;
    fn_80390CE8(scaleMtx, scale, scale * scrH / scrW, 0.0f);
    fn_803905EC(mTexMtx, scaleMtx, mTexMtx);

    Mtx transMtx;
    PSMTXTrans(transMtx, -x, -y, 0.0f);
    fn_803905EC(mTexMtx, transMtx, mTexMtx);
}
