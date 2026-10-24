// REL .text 0x5C..0x64C, .rodata 0x0..0x14, .data 0x0..0x128.
#include <game/game/d_sky_rainbow.hpp>
#include <game/game/d_sky.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_play_util.hpp>
#include <game/framework/f_profile.hpp>

// 0x5C
CUSTOM_BASE_PROFILE(SKY_RAINBOW, dSkyRainbow_c, 0xAC, 7);

// 0x8C
dSkyRainbow_c::dSkyRainbow_c() {}

// 0x128
dSkyRainbow_c::~dSkyRainbow_c() {}

// 0x1C4
int dSkyRainbow_c::create() {
    BOOL ok = TRUE;
    EGG::Heap *heap = dHeap::localHeap_p;
    if (!mRes.load("/Sky/bg_rainbow.brres", heap, 0)) {
        ok = FALSE;
    }
    if (!ok) {
        return ok;
    }

    mAllocator.attach(heap, 0x20);
    mMdl.create(nw4r::g3d::ResFile(mRes.getData()).GetResMdl(0), &mAllocator, 0, 1, NULL);
    return ok;
}

// 0x284
void dSkyRainbow_c::calcMtx() {
    if (g_sky != NULL) {
        mMtx = g_sky->mSkyMtx;
    }
    mMdl.setLocalMtx(&mMtx);
}

// 0x304
int dSkyRainbow_c::execute() {
    f32 hours;
    if (calcWeatherHoursB(&hours)) {
        if (hours > 1.0f || hours < 0.0f) {
            mAlpha = 0;
        } else if (hours < 0.25f) {
            mAlpha = 255.0f * hours / 0.25f;
        } else {
            mAlpha = 255.0f * (1.0f - hours) / 0.75f;
        }
    } else {
        mAlpha = 0;
    }

    if (mAlpha != 0) {
        calcMtx();
    }
    return SUCCEEDED;
}

// 0x3E8
int dSkyRainbow_c::draw() {
    if (mAlpha != 0) {
        nw4r::g3d::ResMat mat = nw4r::g3d::ResFile(mRes.getData()).GetResMdl(0).GetResMat(0);
        GXColor color;
        if (mat.GetResMatTevColor().GXGetTevColor(GX_TEVREG0, &color)) {
            color.a = mAlpha;
            mat.GetResMatTevColor().GXSetTevColor(GX_TEVREG0, color);
            mat.GetResMatTevColor().DCStore(false);
        }
        mMdl.setPriorityDraw(2, 7);
        mMdl.entry();
    }
    return SUCCEEDED;
}

// 0x518
int dSkyRainbow_c::doDelete() {
    BOOL ok = TRUE;
    if (!mRes.unload(FALSE)) {
        ok = FALSE;
    }
    if (!ok) {
        return ok;
    }
    mMdl.remove();
    return ok;
}
