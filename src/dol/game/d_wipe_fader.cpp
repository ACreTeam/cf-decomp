// The circle wipe fader. .text 8017CD3C..8017D09C, .data 804F4B60..804F4BB0, .bss 80621A40..80623A40,
// .sdata 8074B5E8..8074B5F0, .sdata2 80751380..807513A8.
#include <game/game/d_wipe_fader.hpp>
#include <game/mLib/m_tex.hpp>
#include <revolution/mtx.h>

#define WIPE_TEX_SIZE 128
#define WIPE_RADIUS 63

static u8 sWipeTexture[WIPE_TEX_SIZE * WIPE_TEX_SIZE / 2] ALIGN(32); // 80621A40

// 8017CD3C
void *getWipeTexture() {
    return sWipeTexture;
}

// 8017CD48
int getWipeTextureWidth() {
    return WIPE_TEX_SIZE;
}

// 8017CD50
int getWipeTextureHeight() {
    return WIPE_TEX_SIZE;
}

// 8017CD58
dWipeFader_c::dWipeFader_c(mColor col, mFaderBase_c::EStatus status) : mWipeFader_c(col, status) {
    mCenterX = mCenterY = 0.5f;
}

// 8017CDC4
void dWipeFader_c::init() {
    setTexture(sWipeTexture, WIPE_TEX_SIZE, WIPE_TEX_SIZE);
    mTex::edit4b_c tex(WIPE_TEX_SIZE, WIPE_TEX_SIZE, sWipeTexture);
    for (u32 x = 0; x < WIPE_TEX_SIZE; x++) {
        for (u32 y = 0; y < WIPE_TEX_SIZE; y++) {
            tex.set(x, y, 0xF, FALSE);
        }
    }
    for (u32 x = 1; x < WIPE_TEX_SIZE - 1; x++) {
        for (u32 y = 1; y < WIPE_TEX_SIZE - 1; y++) {
            if (((int)x - 64.0f) * ((int)x - 64.0f) + ((int)y - 64.0f) * ((int)y - 64.0f) <=
                (f32)(WIPE_RADIUS * WIPE_RADIUS)) {
                tex.set(x, y, 0, FALSE);
            }
        }
    }
    tex.flush();
}

// 8017CF80
void dWipeFader_c::calcMtx() {
    f32 scale = 196.0f / (256 - mProgress);

    // Move to the center, scale, move back
    // HACK: getCenterY (a by-value getter for Y only) puts the Y load first, as in the target; plain reads score
    // 95.20. Not a plausible original form (see notes/d_wipe_fader.txt).
    PSMTXTrans(mTexMtx, mCenterX, getCenterY(), 0.0f);

    Mtx scaleMtx;
    f32 scrW = mAspectRatioFactor * 608.0f;
    f32 scrH = 456.0f;
    PSMTXScale(scaleMtx, scale, scale * scrH / scrW, 0.0f);
    PSMTXConcat(mTexMtx, scaleMtx, mTexMtx);

    Mtx transMtx;
    PSMTXTrans(transMtx, -mCenterX, -getCenterY(), 0.0f);
    PSMTXConcat(mTexMtx, transMtx, mTexMtx);
}
