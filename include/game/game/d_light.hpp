#pragma once

// d_light (DOL TU d_light.cpp, .text 800CCCD8..800CD590, not decompiled). Only what other TUs use. dLight::*
// are the RTTI names; the other names are inferred.

#include <types.h>
#include <game/mLib/m_color.hpp>
#include <game/mLib/m_vec.hpp>
#include <nw4r/ut.h>
#include <revolution/GX.h>

namespace dLight {

// A light of the light settings (m3d::getLightSettingP). Created by dEnvironment_c, which calls calc() every
// frame, apply() when drawing and regist() once after creating it.
class AbstractLight_c {
public:
    AbstractLight_c() : mLightObjIdx(-1) {}
    virtual ~AbstractLight_c() {} // 800CD550
    virtual void calc();          // 800CD54C
    virtual void apply() = 0;
    virtual void regist(); // 800CD264: takes a light object of the current light set

    /* 0x04 */ mColor mLightColor;
    /* 0x08 */ int mLightObjIdx;
    /* 0x0C */ int mLightIdx;
    /* 0x10 */ int mLightSetIdx;
}; // size 0x14

class AmbLight_c : public AbstractLight_c {
public:
    virtual void apply();  // 800CD2F8
    virtual void regist(); // 800CD304
}; // size 0x14

class FarLight_c : public AbstractLight_c {
public:
    FarLight_c(const mVec3_c &dir = mVec3_c(0.0f, 0.0f, 1.0f)) : mDir(dir) {}
    virtual void apply(); // 800CD35C

    /* 0x14 */ mVec3_c mDir;
}; // size 0x20

class PointLight_c : public AbstractLight_c {
public:
    PointLight_c() : mPos(mVec3_c::Zero) {
        mDistAttn[0] = 1.0f;
        mDistAttn[1] = 0.0f;
        mDistAttn[2] = 1.0f / 128.0f;
    }
    virtual void apply(); // 800CD43C

    /* 0x14 */ mVec3_c mPos;
    /* 0x20 */ f32 mDistAttn[3];
}; // size 0x2C

class SpotLight_c : public AbstractLight_c {
public:
    SpotLight_c(const mVec3_c &pos = mVec3_c(0.0f, 0.0f, 0.0f), f32 attn0 = 1.0f, f32 attn1 = 0.0f, f32 attn2 = 0.0f,
                f32 cutoff = 45.0f, GXSpotFn spotFn = GX_SP_SHARP)
        : mPos(pos), mSpotFn(spotFn), mCutoff(cutoff) {
        mAngleAttn[0] = attn0;
        mAngleAttn[1] = attn1;
        mAngleAttn[2] = attn2;
    }
    virtual void apply(); // 800CD460

    /* 0x14 */ mVec3_c mPos;
    /* 0x20 */ mVec3_c mDir;
    /* 0x2C */ GXSpotFn mSpotFn;
    /* 0x30 */ f32 mCutoff;
    /* 0x34 */ f32 mAngleAttn[3];
}; // size 0x40

} // namespace dLight

extern "C" {
BOOL fn_800CCCD8(); // 800CCCD8: resets the light settings (m3d::getLightSettingP); d_sys_gfx create
BOOL fn_800CCDB4(void (*func)(void *item, void *arg), void *arg); // 800CCDB4: calls func on each queued item
void fn_800CCEAC(int lightSet);       // 800CCEAC: switches to the light set (fn_800CCEBC switches back)
void fn_800CCEBC();                   // 800CCEBC
nw4r::ut::Color fn_800CD168(int idx); // 800CD168: an ambient color of the light settings (GXSetChanAmbColor)
GXLightID fn_800CD250();              // 800CD250: the current light mask (GXSetChanCtrl)
}
