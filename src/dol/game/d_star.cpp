// .text 80090D70..80090E2C, .data 804DFB90..804DFBA0, .sdata2 80750628..80750630.
#include <game/game/d_star.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_sky.hpp>
#include <revolution/mtx.h>
#include <game/game/d_snd_util.hpp>
#include <game/snd/snd_id.hpp>

// 80090D70
void dStar_c::execute() {
    if (mActive && countDown() == 0) {
        mMtx_c mtx;
        PSMTXTrans(mtx.mtx, mX, mY - g_sky->mPos.y, 0.0f);
        // The original constructs this local, but does not pass it to dEffect::create.
        dEffect_c effect;
        dEffect::create("af_fld_star", &mtx);
        playSe(SE_sky_NAGAREBOSHI);
        mActive = false;
    }
}

// 80090E10
int dStar_c::countDown() {
    if (mWait != 0) {
        mWait--;
    }
    return mWait;
}
