#pragma once
#include <game/mLib/m_3d.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_thunder.hpp>

// RTTI identifies the draw classes at global scope.
namespace dWeatherSnow {
struct Particle_c {
    Particle_c() : mActive(0) {
    }

    void init(const mVec3_c &pos);
    BOOL calc(BOOL remove);
    BOOL calcBus(BOOL remove, f32 wind);
    void draw(const mMtx_c &view);
    f32 getFade(f32 distance);

    mVec3_c mPos;
    mVec3_c mSpeed;
    mAng mAngle;
    mAng mAngleSpeed;
    u8 mAlpha;
    u8 _1D;
    u8 mActive;
    u8 mFading;
};

int getLightCount();
int getHeavyCount();
} // namespace dWeatherSnow

namespace dWeatherSakura {
struct Particle_c {
    Particle_c() : mActive(0) {
    }

    void init(const mVec3_c &pos);
    BOOL calc(BOOL remove);
    static f32 getFade(f32 distance);
    void draw(nw4r::g3d::ResShp shape, const mMtx_c &view);

    mVec3_c mPos;
    mVec3_c mSpeed;
    mVec3_c mViewPos;
    u32 _24;
    mAng mAngle;
    mAng mAngleSpeed;
    mAng3_c mRotation;
    mAng3_c mSpin;
    mAng mGroundAngle;
    u8 mAlpha;
    u8 mActive;
    u8 mFading;
    u8 _3D[3];
};

int getCount();
BOOL isSeason();
} // namespace dWeatherSakura

namespace dWeatherConfetti {
struct Particle_c {
    Particle_c() : mActive(0) {
    }

    void init(const mVec3_c &pos);
    BOOL calc(BOOL remove);
    static f32 getFade(f32 distance);
    void draw(nw4r::g3d::ResShp shape, const mMtx_c &view);

    mVec3_c mPos;
    mVec3_c mSpeed;
    mVec3_c mViewPos;
    int mColor;
    mAng mAngle;
    mAng mAngleSpeed;
    mAng3_c mRotation;
    mAng3_c mSpin;
    mAng mGroundAngle;
    u8 mAlpha;
    u8 mActive;
    u8 mFading;
    u8 _3D[3];
};

} // namespace dWeatherConfetti

class dWeatherSnowDraw_c : public m3d::proc_c {
  public:
    dWeatherSnowDraw_c() : mTexture(NULL) {
    }

    virtual void drawXlu();
    BOOL create(nw4r::g3d::ResFile *file, mAllocator_c *allocator);
    void init(int count);
    BOOL calc(int weather);
    BOOL calcBus(int weather);
    BOOL draw();

    dWeatherSnow::Particle_c mParticles[200];
    nw4r::g3d::ResTex mTexture;
    mVec3_c mCameraPos;
    f32 mTargetCount;
    int mCount;
};

class dWeatherSakuraDraw_c : public m3d::proc_c {
  public:
    dWeatherSakuraDraw_c() : mMaterial(NULL), mShape(NULL) {
    }

    virtual void drawXlu();
    BOOL create(nw4r::g3d::ResFile *file, mAllocator_c *allocator);
    void init(int count);
    BOOL calc(int count);
    BOOL draw();

    dWeatherSakura::Particle_c mParticles[120];
    nw4r::g3d::ResMat mMaterial;
    nw4r::g3d::ResShp mShape;
    mVec3_c mCameraPos;
    f32 mTargetCount;
    int mCount;
};

class dWeatherConfettiDraw_c : public m3d::proc_c {
  public:
    dWeatherConfettiDraw_c() : mMaterial(NULL), mShape(NULL) {
    }

    virtual void drawXlu();
    BOOL create(nw4r::g3d::ResFile *file, mAllocator_c *allocator);
    void init();
    BOOL calc();
    BOOL draw();

    dWeatherConfetti::Particle_c mParticles[120];
    nw4r::g3d::ResMat mMaterial;
    nw4r::g3d::ResShp mShape;
    mVec3_c mCameraPos;
    f32 mTargetCount;
    int mCount;
    bool mEnabled;
};

namespace dWeatherRain {
class Rain_c {
  public:
    BOOL init(int weather);
    BOOL remove();
    static f32 getRate(int weather, int nextWeather);
    static f32 getLevel(int weather, int nextWeather);
    f32 getLevel();
    BOOL calc(int weather, int nextWeather);
    BOOL calcBus(int weather);

    dOneEmitterEffect_c mGround;
    dOneEmitterEffect_c mWater;
    dLevelEffect_c mSplash;
    dLevelEffect_c mRipple;
    f32 mRate;
    f32 mFade;
    dThunder::Thunder_c mThunder;
};

} // namespace dWeatherRain
