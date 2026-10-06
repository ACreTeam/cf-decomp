// Fireworks show manager. See include/game/game/d_fireworks.hpp and notes/d_fireworks.txt.
// .text 8008FE50..80090964, .rodata 8046EEC0..8046EEF0, .data 804DFB60..804DFB90,
// .sdata 80749F60..80749F70, .sbss 8074E330..8074E338, .sdata2 807505C0..80750608.
#include <game/game/d_fireworks.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_base.hpp>
#include <game/game/d_scene.hpp>
#include <game/cLib/c_math.hpp>
#include <game/sLib/s_lib.hpp>
#include <revolution/MTX.h>

// Not split yet (C linkage keeps the target names).
class dSkyLight_c;
extern "C" {
void fn_8000F828(int se);                                        // 8000F828: play a sound effect
BOOL fn_80199BBC(void *glow, const GXColor *color, const nw4r::math::VEC3 *pos, f32 size); // 80199BBC: queue a glow

extern nw4r::math::VEC3 lbl_80623FEC; // 80623FEC: town center the show is above
extern u8 *lbl_8074E538;        // 8074E538: [0x3671] picks the alternative sounds
extern void *lbl_8074EA60;      // 8074EA60: glow drawer (NULL: no sounds / glow without the effect)
extern dSkyLight_c *lbl_8074E830; // 8074E830: sky light
extern f32 lbl_8074EA1C;        // 8074EA1C..8074EA28: burst light color
extern f32 lbl_8074EA20;
extern f32 lbl_8074EA24;
extern f32 lbl_8074EA28;
extern f32 lbl_8074C010;        // 8074C010: burst light fade speed
}

// Sky light actor (8074E830); only its first own virtual is used.
class dSkyLight_c : public dBase_c {
public:
    virtual void setFireworksBrightness(f32 brightness); // vtable 0x4C
};

// 8046EEC0 / 8046EECC: the effect's two register colors per shell color.
static const GXColor sEffectColor0[3] = {{0xFF, 0xC0, 0xB8, 0xFF}, {0xB8, 0xFF, 0xA0, 0xFF}, {0xF8, 0xFF, 0xE0, 0xFF}};
static const GXColor sEffectColor1[3] = {{0xFF, 0x40, 0x60, 0xFF}, {0x00, 0xF8, 0xA0, 0xFF}, {0xFF, 0x90, 0x20, 0xFF}};
// 8046EED8: glow color.
static const GXColor sGlowColor[3] = {{0xFF, 0x40, 0x00, 0xFF}, {0x40, 0xFF, 0xC0, 0xFF}, {0xF8, 0xF8, 0x00, 0xFF}};
// 8046EEE4: burst light color.
static const GXColor sLightColor[3] = {{0x59, 0x20, 0x00, 0x20}, {0x20, 0x50, 0x00, 0x20}, {0x48, 0x38, 0x00, 0x20}};

dFireWorksMgr_c *dFireWorksMgr_c::sInstance;

// The shells sit 768 above the town center.
static inline void setBasePos(nw4r::math::VEC3 &pos) {
    f32 height = 768.0f;
    pos = lbl_80623FEC;
    pos.y += height;
}

// 8008FE50
void dFireWork_c::reset() {
    setBasePos(mPos);
    mTimer = 0;
    mState = STATE_IDLE;
    mGlow = 0.0f;
}

// 8008FE94
void dFireWork_c::launch(BOOL withEffect) {
    setBasePos(mPos);
    mTimer = 60;
    mState = STATE_WAIT;
    mGlow = 0.0f;
    mBrightness = 0.0f;
    mMtx_c mtx;
    f32 y = cM::rndRange(200.0f, 300.0f);
    PSMTXTrans(mtx.mtx, cM::rndRange(-200.0f, 200.0f), y, 0.0f);
    mScale = cM::rndRange(0.7f, 1.3f);
    mColor = cM::rndRange(0, 3);
    if (withEffect) {
        dEffect_c effect;
        effect.createEffect("af_fld_firework", &mtx);
        if (effect.mHandle.IsValid()) {
            effect.setScale(mScale);
            effect.setRegisterColor(sEffectColor0[mColor], sEffectColor1[mColor], 0);
            effect.EGG::Effect::update();
        }
        if (lbl_8074E538[0x3671]) {
            fn_8000F828(0x19A1);
        } else {
            fn_8000F828(0x199E);
        }
    } else if (lbl_8074EA60 != NULL) {
        fn_8000F828(0x19A4);
    }
}

// Stripped by the linker; its int-to-float conversion pools the 0x4330000080000000 double here.
f32 dFireWork_c::getTimerF() {
    return mTimer;
}

// 8009004C
void dFireWork_c::execute(BOOL withEffect) {
    switch (mState) {
    case STATE_IDLE:
        break;
    case STATE_WAIT:
        if (sLib::calcTimer(&mTimer) == 0) {
            mState = STATE_RISE;
            mTimer = 128;
        }
        break;
    case STATE_RISE: {
        f32 height = 768.0f;
        f32 glow = withEffect ? 1024.0f : 618.0f;
        f32 rate = 1.0f / mTimer;
        mGlow += rate * (glow - mGlow);
        mBrightness += rate * (0.625f - mBrightness);
        mPos = lbl_80623FEC;
        mPos.y += height;
        if (sLib::calcTimer(&mTimer) == 0) {
            GXColor color;
            *(u32 *)&color = *(const u32 *)&sLightColor[mColor];
            mState = STATE_BURST;
            mTimer = 20;
            lbl_8074EA1C = color.r;
            lbl_8074EA20 = color.g;
            lbl_8074EA24 = color.b;
            lbl_8074EA28 = color.a;
            lbl_8074C010 = 0.005f;
        } else if (mTimer == 12) {
            if (withEffect) {
                int se;
                u8 alt = lbl_8074E538[0x3671];
                if (mScale > 1.0f) {
                    if (alt) {
                        se = 0x19A3;
                    } else {
                        se = 0x19A0;
                    }
                } else {
                    if (alt) {
                        se = 0x19A2;
                    } else {
                        se = 0x199F;
                    }
                }
                fn_8000F828(se);
            } else if (lbl_8074EA60 != NULL) {
                fn_8000F828(mScale > 1.0f ? 0x19A6 : 0x19A5);
            }
        }
        break;
    }
    case STATE_BURST:
        if (sLib::calcTimer(&mTimer) == 0) {
            mState = STATE_FADE;
            mTimer = 90;
        }
        break;
    case STATE_FADE: {
        f32 glow = 0.0f;
        f32 height = 768.0f;
        f32 rate = 1.0f / mTimer;
        mGlow += rate * (glow - mGlow);
        mBrightness += -mBrightness * rate;
        mPos = lbl_80623FEC;
        mPos.y += height;
        if (sLib::calcTimer(&mTimer) == 0) {
            reset();
        }
        break;
    }
    }
    if (mState != STATE_IDLE && lbl_8074E830 != NULL) {
        lbl_8074E830->setFireworksBrightness(mBrightness);
    }
}

// 80090370
void dFireWork_c::draw(BOOL withEffect) {
    if (mState != STATE_IDLE && lbl_8074EA60 != NULL) {
        GXColor color;
        *(u32 *)&color = *(const u32 *)&sGlowColor[mColor];
        fn_80199BBC(lbl_8074EA60, &color, &mPos, mGlow);
    }
}

// 8009042C
inline dFireWork_c::dFireWork_c() : mId(-1) {}

// 800903D0
dFireWorksMgr_c::dFireWorksMgr_c() {
    sInstance = this;
}

// 80090438
dFireWorksMgr_c::~dFireWorksMgr_c() {
    sInstance = NULL;
}

// 800904A8
void dFireWorksMgr_c::init(BOOL withEffect) {
    BOOL allowed = FALSE;
    mWithEffect = withEffect;
    if (!isCityScene(getCurrentScene()) && !isSceneAttr(getCurrentScene(), SCENE_ATTR_CHKP | SCENE_ATTR_ROOM) &&
        !isSceneAttr(getCurrentScene(), SCENE_ATTR_BUS) && getCurrentScene() != SCENE_DM_PL_SEL &&
        getCurrentScene() != SCENE_DM_SAVE && getCurrentScene() != SCENE_DM_LOAD) {
        allowed = TRUE;
    }
    mAllowed = allowed;
    for (int i = 0; i < FIREWORKS_SHELL_NUM; i++) {
        mShells[i].reset();
    }
    mWasShowTime = isShowTime();
    mWait = 0;
    mBurstLeft = cM::rndRange(2, 8);
}

// 800905A8
void dFireWorksMgr_c::execute() {
    if (isShowTime()) {
        if (!mWasShowTime) {
            mBurstLeft = 8;
            mWait = 0x260;
        }
        if (sLib::chase(&mWait, 0, 1)) {
            if (sLib::chase(&mBurstLeft, 0, 1)) {
                mWait = cM::rndRange(180, 300);
                mBurstLeft = cM::rndRange(2, 8);
            } else {
                launch();
                mWait = cM::rndRange(30, 90);
            }
        }
        mWasShowTime = TRUE;
    } else {
        mWasShowTime = FALSE;
    }
    for (int i = 0; i < FIREWORKS_SHELL_NUM; i++) {
        mShells[i].execute(mWithEffect);
    }
}

// 800906AC
void dFireWorksMgr_c::draw() {
    for (int i = 0; i < FIREWORKS_SHELL_NUM; i++) {
        mShells[i].draw(mWithEffect);
    }
}

// 80090708
BOOL dFireWorksMgr_c::isShowTime() {
    dTime_c now = *dTime_c::getCurrent();
    if (mAllowed && (dEvent::isOngoing(EVENT_FIREWORKS) || (dEvent::isOngoing(EVENT_COUNTDOWN) && now.hour < 2))) {
        return TRUE;
    }
    return FALSE;
}

// 800907C8
BOOL dFireWorksMgr_c::launch() {
    dFireWork_c *shell = getFreeShell();
    if (shell != NULL) {
        shell->launch(mWithEffect);
        return TRUE;
    }
    return FALSE;
}

// 80090810
dFireWork_c *dFireWorksMgr_c::getFreeShell() {
    for (int i = 0; i < FIREWORKS_SHELL_NUM; i++) {
        if (mShells[i].mState == dFireWork_c::STATE_IDLE) {
            return &mShells[i];
        }
    }
    return NULL;
}
