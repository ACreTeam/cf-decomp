// REL .text 0x5C..0x6D8, .rodata 0x0..0x14, .data 0x0..0x128.
#include <game/game/d_sky_aurora.hpp>
#include <game/game/d_sky.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_play_util.hpp>
#include <game/framework/f_profile.hpp>

// 0x5C
CUSTOM_BASE_PROFILE(SKY_AURORA, dSkyAurora_c, 0xAB, 6);

// 0x8C
dSkyAurora_c::dSkyAurora_c() {}

// 0x128
dSkyAurora_c::~dSkyAurora_c() {}

// 0x1C4
int dSkyAurora_c::create() {
    BOOL ok = TRUE;
    EGG::Heap *heap = dHeap::localHeap_p;
    if (!mRes.load("/Sky/bg_aurora.brres", heap, 0)) {
        ok = FALSE;
    }
    if (!ok) {
        return ok;
    }

    mAllocator.attach(heap, 0x20);
    mMdl.create(nw4r::g3d::ResFile(mRes.getData()).GetResMdl(0), &mAllocator, 0, 1, NULL);
    mAnm.create(nw4r::g3d::ResFile(mRes.getData()).GetResMdl(0),
                nw4r::g3d::ResFile(mRes.getData()).GetResAnmTexSrt(0), &mAllocator, NULL, 1);
    mMdl.setAnm(mAnm);
    return ok;
}

// 0x2E8
void dSkyAurora_c::calcMtx() {
    if (g_sky != NULL) {
        mMtx = g_sky->mSkyMtx;
    }
    mMdl.setLocalMtx(&mMtx);
}

// 0x368
int dSkyAurora_c::execute() {
    f32 hours;
    if (calcWeatherHoursA(&hours)) {
        if (hours > 1.0f || hours < 0.0f) {
            mAlpha = 0;
        } else if (hours < 0.25f) {
            mAlpha = 255.0f * hours / 0.25f;
        } else if (hours > 0.75f) {
            mAlpha = 255.0f * (1.0f - hours) / 0.25f;
        } else {
            mAlpha = 0xFF;
        }
    } else {
        mAlpha = 0;
    }

    if (mAlpha != 0) {
        mAnm.play();
        calcMtx();
    }
    return SUCCEEDED;
}

// 0x474
int dSkyAurora_c::draw() {
    if (mAlpha != 0) {
        nw4r::g3d::ResMat mat = nw4r::g3d::ResFile(mRes.getData()).GetResMdl(0).GetResMat(0);
        GXColor color;
        if (mat.GetResMatTevColor().GXGetTevKColor(GX_KCOLOR1, &color)) {
            color.a = mAlpha;
            mat.GetResMatTevColor().GXSetTevKColor(GX_KCOLOR1, color);
            mat.GetResMatTevColor().DCStore(false);
        }
        mMdl.setPriorityDraw(2, 5);
        mMdl.entry();
    }
    return SUCCEEDED;
}

// 0x5A4
int dSkyAurora_c::doDelete() {
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
