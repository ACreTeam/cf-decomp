#pragma once

// The lights of the light settings (m3d::getLightSettingP). dLight::*_c are the RTTI names; the other names are
// inferred.

#include <types.h>
#include <game/mLib/m_color.hpp>
#include <game/mLib/m_vec.hpp>
#include <nw4r/g3d/g3d_light.h>
#include <nw4r/ut.h>
#include <revolution/GX.h>

namespace dLight {

enum {
    LIGHT_SET_NUM = 2,
};

// A light for dEnvironment_c to create: the light type and its data (a scene's light list).
struct LightReq_c {
    int mType;
    void *mData;
};

typedef void (*LightReqFunc)(void *req, void *arg);

// A light of the light settings. Created by dEnvironment_c, which calls calc() every frame, apply() when drawing
// and regist() once after creating it.
class AbstractLight_c {
public:
    AbstractLight_c() : mLightId(-1) {}
    virtual ~AbstractLight_c() {}
    virtual void calc() {}
    virtual void apply() = 0;
    virtual void regist();

    /* 0x04 */ mColor mLightColor;
    /* 0x08 */ int mLightId;
    /* 0x0C */ int mLightObjIdx;
    /* 0x10 */ int mLightSetIdx;
}; // size 0x14

class AmbLight_c : public AbstractLight_c {
public:
    virtual void apply();
    virtual void regist();
}; // size 0x14

class FarLight_c : public AbstractLight_c {
public:
    FarLight_c(const mVec3_c &dir = mVec3_c(0.0f, 0.0f, 1.0f)) : mDir(dir) {}
    virtual void apply();

    /* 0x14 */ mVec3_c mDir;
}; // size 0x20

class PointLight_c : public AbstractLight_c {
public:
    PointLight_c() : mPos(mVec3_c::Zero) {
        mDistAttn[0] = 1.0f;
        mDistAttn[1] = 0.0f;
        mDistAttn[2] = 1.0f / 128.0f;
    }
    virtual void apply();

    /* 0x14 */ mVec3_c mPos;
    /* 0x20 */ f32 mDistAttn[3];
}; // size 0x2C

class SpotLight_c : public AbstractLight_c {
public:
    SpotLight_c(const mVec3_c &pos = mVec3_c(0.0f, 0.0f, 0.0f), f32 attn0 = 1.0f, f32 attn1 = 0.0f, f32 attn2 = 0.0f,
                f32 cutoff = 45.0f, GXSpotFn spotFn = GX_SP_SHARP)
        : mPos(pos), mSpotFn(spotFn), mCutoff(cutoff) {
        mDistAttn[0] = attn0;
        mDistAttn[1] = attn1;
        mDistAttn[2] = attn2;
    }
    virtual void apply();

    /* 0x14 */ mVec3_c mPos;
    /* 0x20 */ mVec3_c mDir;
    /* 0x2C */ GXSpotFn mSpotFn;
    /* 0x30 */ f32 mCutoff;
    /* 0x34 */ f32 mDistAttn[3];
}; // size 0x40

BOOL init();
BOOL setLightReq(void *reqs, u32 num);
BOOL execLightReq(LightReqFunc func, void *arg);
void resetLightNum();
int getNextLightId();
int getNextLightObjIdx();
void changeLightSet(int lightSet);
void restoreLightSet();
BOOL setPointLight(u32 lightId, mColor &color, const mVec3_c &pos, f32 k0, f32 k1, f32 k2, int lightSet);
BOOL setSpotLight(u32 lightId, mColor &color, const mVec3_c &pos, const mVec3_c &dir, f32 k0, f32 k1, f32 k2,
                  f32 cutoff, GXSpotFn spotFn, int lightSet);
BOOL setAmbColor(mColor &color, int lightSet);
mColor getAmbColor(int lightSet);
nw4r::g3d::LightSet getLightSet(int lightSet);
nw4r::g3d::LightObj *getLightObj(u32 lightId);
GXLightID getLightMask();

} // namespace dLight
