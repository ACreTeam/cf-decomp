// .text 80090D70..80090E2C, .data 804DFB90..804DFBA0, .sdata2 80750628..80750630.
#include <game/game/d_star.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_sky.hpp>
#include <revolution/MTX.h>
#include <game/game/d_snd_util.hpp>

// Not decompiled yet (C linkage keeps the target names).
extern "C" {
bool fn_80087A30(const char *name, const mMtx_c *mtx); // 80087A30: likely a static dEffect_c member
}

// 80090D70
void dStar_c::execute() {
    if (mActive && countDown() == 0) {
        mMtx_c mtx;
        PSMTXTrans(mtx.mtx, mX, mY - lbl_8074E830->mPos.y, 0.0f);
        // The original constructs this local, but does not pass it to fn_80087A30.
        dEffect_c effect;
        fn_80087A30("af_fld_star", &mtx);
        playSe(0x17CE);
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
