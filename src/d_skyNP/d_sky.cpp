// REL .text 0x5C..0x1AB8, .ctors 0x0..0x4, .rodata 0x0..0xAC, .data 0x0..0x1B8, .bss 0x8..0x1C.
#include <game/game/d_sky.hpp>
#include <game/game/d_base.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_scene.hpp>
#include <game/framework/f_profile.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/sLib/s_lib.hpp>
#include <revolution/MTX.h>
#include <math.h>
#include <game/game/d_heap.hpp>

// Not decompiled yet (C linkage keeps the target names).
extern "C" {

void fn_80197414(mColor *color, int idx); // 80197414: a sky color for the time of day
u8 fn_801977BC();                         // 801977BC: the star color's alpha
}

void *dSky_c_classInit();
f32 getMoonAge(const dTime_c *now);
f32 getOffsetY();
f32 getOffsetZ();

fProf::fBaseProfile_c g_profile_SKY = {&dSky_c_classInit, 0xAA, 5};

// Added to the moon's tint.
static const GXColor sMoonAdd = {0x10, 0x10, 0x0C, 0xFF};

// Unused: only __sinit touches it. Five zero-initialized 4-byte members, of which bytes 0x2..0x11
// are then set; the layout of the original type is unknown.
struct dSkyColor_c {
    dSkyColor_c() : r(0), g(0), b(0), a(0) {}

    u8 r, g, b, a;
};
struct dSkyUnused_c {
    dSkyUnused_c() {
        m0.b = 0xE5;
        m0.a = 0xE7;
        m1.r = 0xFF;
        m1.g = 0xC0;
        m1.b = 0x76;
        m1.a = 0x62;
        m2.r = 0x20;
        m2.g = 0x00;
        m2.b = 0x29;
        m2.a = 0x2B;
        m3.r = 0xFF;
        m3.g = 0x00;
        m3.b = 0x00;
        m3.a = 0x10;
        m4.r = 0x20;
        m4.g = 0x00;
    }

    dSkyColor_c m0, m1, m2, m3, m4;
};
static dSkyUnused_c sUnusedColors;

// 0x5C
void *dSky_c_classInit() {
    return new dSky_c();
}

// 0x8C
dSky_c::dSky_c() : mBrightness(0.0f), mCreated(false), mSeasonActorCreated(false) {}

// 0x370
dSky_c::~dSky_c() {}

// 0x4BC
int dSky_c::create() {
    mIsCity = isCityScene(getCurrentScene());
    if (!mIsCity && !isSceneAttr(getCurrentScene(), 0x10) && !mSeasonActorCreated) {
        if (isWinterSeasonType()) {
            dBase_c::createBase(0xAB, this, 0, 0);
        } else if (!isLateSeason()) {
            dBase_c::createBase(0xAC, this, 0, 0);
        }
        mSeasonActorCreated = true;
    }

    BOOL ok = TRUE;
    EGG::Heap *heap = dHeap::localHeap_p;
    if (!mResCloud.load("/Sky/bg_cloud.brres", heap, 0)) {
        ok = FALSE;
    }
    if (!mResSky.load("/Sky/bg_sky.brres", heap, 0)) {
        ok = FALSE;
    }
    if (!mResMoon.load("/Sky/bg_moon.brres", heap, 0)) {
        ok = FALSE;
    }
    if (!mStarDraw.create(heap)) {
        ok = FALSE;
    }
    if (!ok) {
        return ok;
    }

    mAllocator.attach(heap, 0x20);
    mMdlCloud.create(nw4r::g3d::ResFile(mResCloud.getData()).GetResMdl(0), &mAllocator, 0, 1, NULL);
    mMdlSky.create(nw4r::g3d::ResFile(mResSky.getData()).GetResMdl(0), &mAllocator, 0, 1, NULL);
    mMdlMoon.create(nw4r::g3d::ResFile(mResMoon.getData()).GetResMdl(0), &mAllocator, 0, 1, NULL);

    if (mIsCity || isSceneAttr(getCurrentScene(), 0x110)) {
        mStarDraw.mMode = 1;
    } else {
        mStarDraw.mMode = 0;
    }

    mAnmCloud.create(nw4r::g3d::ResFile(mResCloud.getData()).GetResMdl(0),
                     nw4r::g3d::ResFile(mResCloud.getData()).GetResAnmTexSrt(0), &mAllocator, NULL, 1);
    mMdlCloud.setAnm(mAnmCloud);
    mAnmMoon.create(nw4r::g3d::ResFile(mResMoon.getData()).GetResMdl(0),
                    nw4r::g3d::ResFile(mResMoon.getData()).GetResAnmTexSrt(0), &mAllocator, NULL, 1);
    mMdlMoon.setAnm(mAnmMoon);

    mStarMgr.init();
    mCreated = true;
    lbl_8074E830 = this;
    return ok;
}

// 0x7E8
int dSky_c::execute() {
    mAnmCloud.play();
    mAnmMoon.setFrame(getMoonAge(dTime_c::getCurrent()), 0);
    sLib::addCalc2(&mBrightness, 0.0f, 0.05f, 0.1f);
    calc();
    return SUCCEEDED;
}

static inline u8 addClamp(u8 value, int add) {
    int sum = value + add;
    return sum >= 0xFF ? 0xFF : sum;
}

// 0x868
int dSky_c::draw() {
    nw4r::g3d::ResMat cloudMat = nw4r::g3d::ResFile(mResCloud.getData()).GetResMdl(0).GetResMat("m_cloud");
    nw4r::g3d::ResMat skyMat = nw4r::g3d::ResFile(mResSky.getData()).GetResMdl(0).GetResMat("m_sky");

    mColor cloudColor0;
    mColor cloudColor1;
    fn_80197414(&mSkyColor0, 4);
    fn_80197414(&mSkyColor1, 5);
    fn_80197414(&cloudColor0, 6);
    fn_80197414(&cloudColor1, 7);
    cloudColor0.a = fn_801977BC();

    u8 alpha;
    if (dTime_c::getCurrent()->hour < 4 || dTime_c::getCurrent()->hour > 19) {
        alpha = 0xFF;
    } else if (dTime_c::getCurrent()->hour == 4 || dTime_c::getCurrent()->hour == 5) {
        f32 rate = dTime_c::getCurrent()->min * (1.0f / 120.0f);
        rate += (dTime_c::getCurrent()->hour - 4) * 0.5f;
        rate = 1.0f - rate;
        alpha = 255.0f * rate;
    } else if (dTime_c::getCurrent()->hour == 18 || dTime_c::getCurrent()->hour == 19) {
        f32 rate = dTime_c::getCurrent()->min * (1.0f / 120.0f);
        rate += (dTime_c::getCurrent()->hour - 18) * 0.5f;
        alpha = 255.0f * rate;
    } else {
        alpha = 0;
    }
    mStarDraw.mAlpha = alpha * (1.0f - mBrightness);

    cloudMat.GetResMatTevColor().GXSetTevKColor(GX_KCOLOR1, cloudColor0);
    cloudMat.GetResMatTevColor().GXSetTevKColor(GX_KCOLOR2, cloudColor1);
    cloudMat.GetResMatTevColor().DCStore(false);
    skyMat.GetResMatTevColor().GXSetTevKColor(GX_KCOLOR1, mSkyColor0);
    skyMat.GetResMatTevColor().GXSetTevKColor(GX_KCOLOR2, mSkyColor1);
    skyMat.GetResMatTevColor().DCStore(false);

    mMdlSky.setPriorityDraw(2, 3);
    mMdlSky.entry();
    if (mStarDraw.mAlpha != 0) {
        mStarDraw.setPriorityDraw(2, 3);
        mStarDraw.entry();
    }

    if (!isSceneAttr(getCurrentScene(), 0x110) && !isSceneAttr(getCurrentScene(), 0x4000)) {
        nw4r::g3d::ResMat moonMat = nw4r::g3d::ResFile(mResMoon.getData()).GetResMdl(0).GetResMat(0);
        mColor moonColor;
        moonColor.lerp(mSkyColor0, mSkyColor1, 0.125f);
        moonColor.r = addClamp(moonColor.r, sMoonAdd.r);
        moonColor.g = addClamp(moonColor.g, sMoonAdd.g);
        moonColor.b = addClamp(moonColor.b, sMoonAdd.b);
        moonColor.a = addClamp(moonColor.a, sMoonAdd.a);
        moonMat.GetResMatTevColor().GXSetTevKColor(GX_KCOLOR1, moonColor);
        moonMat.GetResMatTevColor().DCStore(false);
        mMdlMoon.setPriorityDraw(2, 4);
        mMdlMoon.entry();
    }

    mMdlCloud.setPriorityDraw(2, 6);
    mMdlCloud.entry();
    return SUCCEEDED;
}

// 0xE78
int dSky_c::doDelete() {
    BOOL ok = TRUE;
    if (mCreated) {
        mMdlCloud.remove();
        mAnmCloud.remove();
        mMdlSky.remove();
        mMdlMoon.remove();
        mCreated = false;
    }
    if (!mResCloud.unload(FALSE)) {
        ok = FALSE;
    }
    if (!mResSky.unload(FALSE)) {
        ok = FALSE;
    }
    if (!mResMoon.unload(FALSE)) {
        ok = FALSE;
    }
    if (!mStarDraw.unload()) {
        ok = FALSE;
    }
    if (!ok) {
        return ok;
    }
    lbl_8074E830 = NULL;
    return ok;
}

// 0xF88
void dSky_c::setFireworksBrightness(f32 brightness) {
    if (brightness > mBrightness) {
        sLib::addCalc2(&mBrightness, brightness, 0.5f, 0.5f);
    }
}

// 0xFAC: the moon's age in days (0..28) for the animation frame. Days start at noon.
f32 getMoonAge(const dTime_c *now) {
    dTime_c base;
    base.init();
    dTime_c time = *now;
    time.add(0, -12, 0, 0);
    int days = dTime_c::diffDays(&time, &base, FALSE);
    return (((days * 100 + 2420) % 2953) * 28) / 2953.0f;
}

// 0x10B8
f32 getOffsetY() {
    if (isSceneAttr(getCurrentScene(), 0x110)) {
        return 256.0f;
    }
    if (isSceneAttr(getCurrentScene(), 0x4000)) {
        return 248.0f;
    }
    return 0.0f;
}

// 0x1128
f32 getOffsetZ() {
    if (isSceneAttr(getCurrentScene(), 0x4000)) {
        return -42.0f;
    }
    return 0.0f;
}

// 0x1170
void dSky_c::calc() {
    mMtx_c view;
    view = lbl_8074E9B0->mViewMtx;
    mSkyMtx = view;
    mCloudMtx = view;
    mMoonMtx = view;

    f32 skyMinY = -145.0f;
    f32 cityY;
    f32 starMinY = -380.0f;
    f32 starMaxY = -110.0f;
    if (mIsCity) {
        cityY = 220.0f;
    } else {
        cityY = 110.0f;
    }
    f32 pitch = mAng(lbl_8074E9B0->mPitch).degree();
    f32 rate = nw4r::math::TanDeg(pitch) - nw4r::math::TanDeg(-52.0f);
    rate /= nw4r::math::TanDeg(-22.0f) - nw4r::math::TanDeg(-52.0f);
    f32 inv = 1.0f - rate;

    mVec3_c skyPos(0.0f, cityY * inv + skyMinY * rate, -1300.0f);
    mVec3_c cloudPos(0.0f, 32.0f, 0.0f);
    skyPos.y += getOffsetY();
    cloudPos += skyPos;
    mPos.set(lbl_8074E9B0->mYaw * 0.05f, starMaxY * inv + starMinY * rate, -1300.0f);
    mPos.y += getOffsetY();
    mPos.z += getOffsetZ();

    mSkyMtx.concat(mMtx_c::createTrans(skyPos));
    mSkyMtx.concat(mMtx_c::createScale(1.0f, 1.0f, 1.0f));
    mMtx538.trans(skyPos);
    mCloudMtx.concat(mMtx_c::createTrans(cloudPos));
    mCloudMtx.concat(mMtx_c::createScale(0.9375f, 0.8f, 1.0f));
    mMdlCloud.setLocalMtx(&mCloudMtx);
    mMdlCloud.calc(false);

    nw4r::g3d::ResMdl cloudMdl = nw4r::g3d::ResFile(mResCloud.getData()).GetResMdl(0);
    f32 scroll = 0.0f;
    if (lbl_8074E9B0 != NULL) {
        scroll = 0.0016f * getViewCenter<nw4r::math::VEC3>()->x;
        scroll += -0.0002f * lbl_8074E9B0->mYaw;
        scroll = fmod(scroll, 4.0);
    }
    for (u32 i = 0; i < cloudMdl.GetResMatNumEntries(); i++) {
        nw4r::g3d::ResTexSrt texSrt = cloudMdl.GetResMat("m_cloud").GetResTexSrt();
        for (u32 j = 0; j < 3; j++) {
            nw4r::math::MTX34 srt;
            texSrt.GetEffectMtx(j, &srt);
            srt._03 = scroll;
            texSrt.SetEffectMtx(j, &srt);
        }
    }
    mMdlSky.setLocalMtx(&mSkyMtx);

    dTime_c now = *dTime_c::getCurrent();
    dTime_c moonset = now;
    moonset.add(0, -12, 0, 0);
    moonset.set(moonset.year, moonset.month, moonset.mday, 23, 30, 0);
    f32 secs = OS_TICKS_TO_SEC(dTime_c::diffTicks(&now, &moonset, FALSE));
    f32 angle = 20.0f * (secs / 16200.0f);
    mVec3_c moonPos(0.0f, -330.0f, 0.0f);
    moonPos += mPos;
    mMoonMtx.concat(mMtx_c::createTrans(moonPos));
    mMoonMtx.ZrotM(mAng(angle * mAng::DegreeToAngleCoefficient));
    if (mIsCity) {
        mMoonMtx.concat(mMtx_c::createTrans(0.0f, 752.0f, 0.0f));
    } else {
        mMoonMtx.concat(mMtx_c::createTrans(0.0f, 875.0f, 0.0f));
    }
    mMdlMoon.setLocalMtx(&mMoonMtx);

    mStarDraw.calc(&mPos, dStarDraw_c::getSkyAngle(&now));
    mStarMgr.execute();
}
