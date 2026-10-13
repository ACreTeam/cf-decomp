// d_thunder: the lightning of a thunderstorm (dThunder::Thunder_c) and five empty stubs.
// .text 8016CEF8..8016D264, .ctors 8046576C..80465770. See include/game/game/d_thunder.hpp.
#include <game/game/d_thunder.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_environment.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/cLib/c_math.hpp>
#include <game/mLib/m_color.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/snd/snd_id.hpp>

// 8016CEF8
void fn_8016CEF8(EGG::Heap *heap) {}

// 8016CEFC
void fn_8016CEFC() {}

// 8016CF00
void fn_8016CF00() {}

// 8016CF04
void fn_8016CF04() {}

// 8016CF08
int fn_8016CF08(int idx) {
    return 0;
}

namespace dThunder {

static mColor l_glowColor(0x50, 0xC0, 0x90, 0xFF);  // 8074E8A8
static mColor l_lightColor(0x20, 0x18, 0x10, 0xFF); // 8074E8AC

// The glow sits 768 above the view center.
static inline void setBasePos(nw4r::math::VEC3 &pos) {
    f32 height = 768.0f;
    pos = lbl_80623FEC;
    pos.y += height;
}

// Moves value the remaining 1 / timer of the way to target.
static inline void approach(f32 &value, f32 target, int timer) {
    value += (target - value) / timer;
}

// 8016CF10
void Thunder_c::reset() {
    setBasePos(mPos);
    mTimer = cM::rndRange(30, 510);
    mState = STATE_WAIT;
    mGlow = 0.0f;
}

// 8016CF84
void Thunder_c::calc(BOOL outdoor) {
    switch (mState) {
    case STATE_WAIT:
        if (sLib::calcTimer(&mTimer) == 0) {
            mState = STATE_FLASH;
            mTimer = 15;
        }
        break;
    case STATE_FLASH: {
        f32 height = 768.0f;
        f32 glow = outdoor ? 1024.0f : 672.0f;
        approach(mGlow, glow, mTimer);
        mPos = lbl_80623FEC;
        mPos.y += height;
        if (sLib::calcTimer(&mTimer) == 0) {
            dEnvironment_c::sFlashR = l_lightColor.r;
            dEnvironment_c::sFlashG = l_lightColor.g;
            dEnvironment_c::sFlashB = l_lightColor.b;
            dEnvironment_c::sFlashA = l_lightColor.a;
            dEnvironment_c::sFlashSpeed = 0.125f;
            mState = STATE_FADE;
            mTimer = 90;
        }
        break;
    }
    case STATE_FADE:
        approach(mGlow, 0.0f, mTimer);
        setBasePos(mPos);
        if (sLib::calcTimer(&mTimer) == 0) {
            reset();
            int se;
            if (outdoor) {
                se = SE_Kaminari;
            } else {
                se = SE_Kaminari_room;
            }
            playSe(se);
        }
        break;
    }
}

// 8016D188
void Thunder_c::draw(BOOL outdoor) {
    if (mState != STATE_WAIT && dEffectLight_c::sInstance != NULL) {
        dEffectLight_c::sInstance->addGlow(&l_glowColor, &mPos, mGlow);
    }
}

} // namespace dThunder
