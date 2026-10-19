#pragma once

// The environment lighting, fog and fireworks (RTTI dEnvironment_c) and its lights. Class names are from the
// RTTI; the other names are inferred.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_fireworks.hpp>
#include <game/game/d_light.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_color.hpp>
#include <game/mLib/m_vec.hpp>
#include <nw4r/math.h>
#include <revolution/GX.h>

// Only declared here (d_str.hpp): the users of the lights, e.g. d_a_rmobj_theater, don't include the town
// structures (d_str.hpp's dDvd::brresBank_c subclasses change their weak function order).
namespace dSTR {
class light_c;
}

// The sun and the moon: a far light turning with the time of day.
class dEnvLight_c : public dLight::FarLight_c {
public:
    dEnvLight_c() : dLight::FarLight_c(mVec3_c(0.0f, 0.0f, -1.0f)) {}

    void setDir(const mAng &x, const mAng &y);
}; // size 0x20

class dSunLight_c : public dEnvLight_c {
public:
    virtual void calc();
};

// The sun through the windows of a room (brighter).
class dSunLightRmIns_c : public dSunLight_c {
public:
    virtual void calc();
};

class dMoonLight_c : public dEnvLight_c {
public:
    virtual void calc();
};

class dMoonLightRmIns_c : public dMoonLight_c {
public:
    virtual void calc();
};

class dSubLightField_c : public dEnvLight_c {
public:
    virtual void calc();
};

class dSubLightRoom_c : public dEnvLight_c {
public:
    dSubLightRoom_c();
    virtual void calc();

    /* 0x20 */ f32 _20;
    /* 0x24 */ u8 mInit;

    static int sLightMode;
    static u8 sLightOn;
    static dSTR::light_c sLight;
}; // size 0x28

class dEnvAmbLight_c : public dLight::AmbLight_c {
public:
    virtual void calc();
};

class dFixAmbLight_c : public dLight::AmbLight_c {
public:
    struct Data_c {
        /* 0x0 */ GXColor mColor;
        /* 0x4 */ u8 mAlpha;
    };

    dFixAmbLight_c(Data_c *data) {
        mAlpha = data->mAlpha;
        mData = data;
        mLightColor = mColor(data->mColor);
    }
    virtual void calc();
    virtual void apply();

    /* 0x14 */ Data_c *mData;
    /* 0x18 */ u8 mAlpha;
}; // size 0x1C

class dFixAmbSwitchLight_c : public dFixAmbLight_c {
public:
    struct Data_c : dFixAmbLight_c::Data_c {
        /* 0x05 */ GXColor mColorOn;
        /* 0x09 */ u8 mOn;
        /* 0x0C */ f32 mSpeedOn;
        /* 0x10 */ f32 mSpeedOff;
    };

    dFixAmbSwitchLight_c(Data_c *data) : dFixAmbLight_c(data) {
        mColorOn = data->mColorOn;
        mSpeedOn = data->mSpeedOn;
        mSpeedOff = data->mSpeedOff;
        mOn = data->mOn;
        if (mOn) {
            mRatio = 1.0f;
        } else {
            mRatio = 0.0f;
        }
        sInstance = this;
        mColorOff = mLightColor;
    }
    virtual void calc();

    /* 0x1C */ nw4r::ut::Color mColorOff;
    /* 0x20 */ nw4r::ut::Color mColorOn;
    /* 0x24 */ f32 mRatio;
    /* 0x28 */ f32 mSpeedOn;
    /* 0x2C */ f32 mSpeedOff;
    /* 0x30 */ u8 mOn;

    static dFixAmbSwitchLight_c *sInstance;
}; // size 0x34

// The ambient light at the Happy Room Academy check point; mixes the environment's ambient color in.
class dCheckPAmbLight_c : public dFixAmbLight_c {
public:
    dCheckPAmbLight_c(Data_c *data) : dFixAmbLight_c(data), mEnvColor(0x000000FF) {
        mFixColor = mLightColor;
        mRatio = 0.0f;
        sInstance = this;
    }
    virtual ~dCheckPAmbLight_c() {
        sInstance = NULL;
    }
    virtual void calc();

    /* 0x1C */ mColor mEnvColor;
    /* 0x20 */ GXColor mFixColor;
    /* 0x24 */ f32 mRatio;

    static dCheckPAmbLight_c *sInstance;
}; // size 0x28

class dBusAmbLight_c : public dFixAmbLight_c {
public:
    dBusAmbLight_c(Data_c *data) : dFixAmbLight_c(data), mEnvColor(0x000000FF) {
        mFixColor = mLightColor;
        mRatio = 1.0f;
        sInstance = this;
    }
    virtual ~dBusAmbLight_c() {
        sInstance = NULL;
    }
    virtual void calc();

    /* 0x1C */ mColor mEnvColor;
    /* 0x20 */ GXColor mFixColor;
    /* 0x24 */ f32 mRatio;

    static dBusAmbLight_c *sInstance;
    static dBusAmbLight_c *getInstance() { return sInstance; }
}; // size 0x28

class dFixFarLight_c : public dLight::FarLight_c {
public:
    struct Data_c {
        /* 0x0 */ GXColor mColor;
        /* 0x4 */ s16 mAngX;
        /* 0x6 */ s16 mAngY;
    };

    dFixFarLight_c(Data_c *data) {
        mAngX = data->mAngX;
        mAngY = data->mAngY;
        mData = data;
        mLightColor = mColor(data->mColor);
    }
    virtual void calc();

    /* 0x20 */ Data_c *mData;
    /* 0x24 */ mAng mAngX;
    /* 0x26 */ mAng mAngY;
}; // size 0x28

class dFixFarSwitchLight_c : public dFixFarLight_c {
public:
    struct Data_c : dFixFarLight_c::Data_c {
        /* 0x08 */ GXColor mColorOn;
        /* 0x0C */ u8 mOn;
        /* 0x10 */ f32 mSpeedOn;
        /* 0x14 */ f32 mSpeedOff;
    };

    dFixFarSwitchLight_c(Data_c *data) : dFixFarLight_c(data) {
        mSpeedOn = data->mSpeedOn;
        mSpeedOff = data->mSpeedOff;
        mOn = data->mOn;
        if (mOn) {
            mRatio = 1.0f;
        } else {
            mRatio = 0.0f;
        }
        sInstance = this;
        mColorOff = mLightColor;
        mColorOn = data->mColorOn;
    }
    virtual ~dFixFarSwitchLight_c() {
        sInstance = NULL;
    }
    // Switches the light on or off at once (d_a_npc_sp_totakeke).
    void setOn(u8 on) {
        mOn = on;
        if (mOn) {
            mRatio = 1.0f;
        } else {
            mRatio = 0.0f;
        }
    }
    virtual void calc();

    /* 0x28 */ nw4r::ut::Color mColorOff;
    /* 0x2C */ nw4r::ut::Color mColorOn;
    /* 0x30 */ f32 mRatio;
    /* 0x34 */ f32 mSpeedOn;
    /* 0x38 */ f32 mSpeedOff;
    /* 0x3C */ u8 mOn;

    static dFixFarSwitchLight_c *sInstance;
}; // size 0x40

// A far light switching between two colors and directions.
class dFixFarSwitch2Light_c : public dFixFarLight_c {
public:
    struct Data_c : dFixFarLight_c::Data_c {
        /* 0x08 */ GXColor mColorOn;
        /* 0x0C */ u8 _0C[4];
        /* 0x10 */ u8 mOn;
        /* 0x14 */ f32 mSpeedOn;
        /* 0x18 */ f32 mSpeedOff;
    };

    dFixFarSwitch2Light_c(Data_c *data) : dFixFarLight_c(data) {
        mSpeedOn = data->mSpeedOn;
        mSpeedOff = data->mSpeedOff;
        mOn = data->mOn;
        if (mOn) {
            mRatio = 1.0f;
        } else {
            mRatio = 0.0f;
        }
        sInstance = this;
        mColorOff = mLightColor;
        mColorOn = data->mColorOn;
    }
    virtual ~dFixFarSwitch2Light_c() {
        sInstance = NULL;
    }
    virtual void calc();

    /* 0x28 */ nw4r::ut::Color mColorOff;
    /* 0x2C */ nw4r::ut::Color mColorOn;
    /* 0x30 */ mAng mAngOnX;
    /* 0x32 */ mAng mAngOnY;
    /* 0x34 */ mAng mAngOffX;
    /* 0x36 */ mAng mAngOffY;
    /* 0x38 */ f32 mRatio;
    /* 0x3C */ f32 mSpeedOn;
    /* 0x40 */ f32 mSpeedOff;
    /* 0x44 */ u8 mOn;

    static dFixFarSwitch2Light_c *sInstance;
}; // size 0x48

class dFixSpotLight_c : public dLight::SpotLight_c {
public:
    struct Data_c {
        /* 0x00 */ GXColor mColor;
        /* 0x04 */ nw4r::math::VEC3 mPos;
        /* 0x10 */ s16 mAngX;
        /* 0x12 */ s16 mAngY;
        /* 0x14 */ f32 mDistAttn[3];
        /* 0x20 */ f32 mCutoff;
        /* 0x24 */ GXSpotFn mSpotFn;
    };

    dFixSpotLight_c(Data_c *data)
        : dLight::SpotLight_c(data->mPos, data->mDistAttn[0], data->mDistAttn[1], data->mDistAttn[2], data->mCutoff,
                              data->mSpotFn) {
        mAngX = data->mAngX;
        mAngY = data->mAngY;
        mData = data;
        mLightColor = mColor(data->mColor);
        mVec3_c dir(0.0f, 0.0f, -1.0f);
        dir.rotX(data->mAngX);
        dir.rotY(data->mAngY);
        mDir = dir;
    }
    virtual void calc();

    /* 0x40 */ Data_c *mData;
    /* 0x44 */ mAng mAngX;
    /* 0x46 */ mAng mAngY;
}; // size 0x48

class dFixSpotSwitchLight_c : public dFixSpotLight_c {
public:
    struct Data_c : dFixSpotLight_c::Data_c {
        /* 0x28 */ u8 mOn;
        /* 0x2C */ f32 mSpeedOn;
        /* 0x30 */ f32 mSpeedOff;
    };

    dFixSpotSwitchLight_c(Data_c *data) : dFixSpotLight_c(data) {
        mSpeedOn = data->mSpeedOn;
        mSpeedOff = data->mSpeedOff;
        mOn = data->mOn;
        if (mOn) {
            mRatio = 1.0f;
        } else {
            mRatio = 0.0f;
        }
        sInstance = this;
        mColorOn = mLightColor;
    }
    virtual ~dFixSpotSwitchLight_c() {
        sInstance = NULL;
    }
    // Switches the light on or off at once (d_a_rmobj_mirrorball).
    void setOnImmediate(u8 on) {
        mOn = on;
        if (mOn) {
            mRatio = 1.0f;
        } else {
            mRatio = 0.0f;
        }
    }
    virtual void calc();

    /* 0x48 */ nw4r::ut::Color mColorOn;
    /* 0x4C */ f32 mRatio;
    /* 0x50 */ f32 mSpeedOn;
    /* 0x54 */ f32 mSpeedOff;
    /* 0x58 */ u8 mOn;

    static dFixSpotSwitchLight_c *sInstance;
}; // size 0x5C

class dFixSpotTheaterLight_c : public dFixSpotLight_c {
public:
    dFixSpotTheaterLight_c(Data_c *data) : dFixSpotLight_c(data), mRatio(0.0f), mMode(0) {
        sInstance = this;
        mColorOn = mLightColor;
    }
    virtual ~dFixSpotTheaterLight_c() {
        sInstance = NULL;
    }
    virtual void calc();

    /* 0x48 */ nw4r::ut::Color mColorOn;
    /* 0x4C */ f32 mRatio;
    /* 0x50 */ f32 mValue;
    /* 0x54 */ u8 mMode;

    static dFixSpotTheaterLight_c *sInstance;
}; // size 0x58

class dResetCenterLight_c : public dFixFarLight_c {
public:
    struct Data_c : dFixFarLight_c::Data_c {
        /* 0x08 */ GXColor mColor2;
        /* 0x0C */ f32 mSpeedOn;
        /* 0x10 */ f32 mSpeedOff;
        /* 0x14 */ s16 mWaveSpeed;
    };

    dResetCenterLight_c(Data_c *data) : dFixFarLight_c(data), mColor1(mLightColor) {
        mColor2 = data->mColor2;
        mRatio = 0.0f;
        mSpeedOn = data->mSpeedOn;
        mSpeedOff = data->mSpeedOff;
        mWave = 0;
        mWaveSpeed = data->mWaveSpeed;
        mOn = FALSE;
        mLightColor = mColor2;
        sInstance = this;
    }
    virtual ~dResetCenterLight_c() {
        sInstance = NULL;
    }
    virtual void calc();

    /* 0x28 */ GXColor mColor1;
    /* 0x2C */ nw4r::ut::Color mColor2;
    /* 0x30 */ f32 mRatio;
    /* 0x34 */ f32 mSpeedOn;
    /* 0x38 */ f32 mSpeedOff;
    /* 0x3C */ s16 mWave;
    /* 0x3E */ s16 mWaveSpeed;
    /* 0x40 */ u8 mOn;

    static dResetCenterLight_c *sInstance;
}; // size 0x44

class dCheckPLight_c : public dFixFarLight_c {
public:
    dCheckPLight_c(Data_c *data) : dFixFarLight_c(data) {
        mEnvColor = mFixColor;
        mFixColor = mLightColor;
        mRatio = 0.0f;
        sInstance = this;
    }
    virtual ~dCheckPLight_c() {
        sInstance = NULL;
    }
    virtual void calc();

    /* 0x28 */ GXColor mEnvColor;
    /* 0x2C */ GXColor mFixColor;
    /* 0x30 */ f32 mRatio;

    static dCheckPLight_c *sInstance;
}; // size 0x34

class dBusLight_c : public dFixFarLight_c {
public:
    dBusLight_c(Data_c *data) : dFixFarLight_c(data) {
        mEnvColor = mFixColor;
        mFixColor = mLightColor;
        mRatio = 1.0f;
        mOff = FALSE;
        sInstance = this;
    }
    virtual ~dBusLight_c() {
        sInstance = NULL;
    }
    virtual void calc();

    /* 0x28 */ GXColor mEnvColor;
    /* 0x2C */ GXColor mFixColor;
    /* 0x30 */ f32 mRatio;
    /* 0x34 */ u8 mOff;

    static dBusLight_c *sInstance;
    static dBusLight_c *getInstance() { return sInstance; }
}; // size 0x38

// The spot light moving along the bus tunnel.
class dBusTunnelLight_c : public dFixSpotLight_c {
public:
    struct Data_c : dFixSpotLight_c::Data_c {
        /* 0x28 */ nw4r::math::VEC3 mEnd;
        /* 0x34 */ f32 _34;
    };

    dBusTunnelLight_c(Data_c *data)
        : dFixSpotLight_c(data), mStart(data->mPos), mEnd(data->mEnd), mRatio(0.0f), _64(data->_34) {
        mColorOn = data->mColor;
        mTimer = 0;
        sInstance = this;
    }
    virtual ~dBusTunnelLight_c() {
        sInstance = NULL;
    }
    virtual void calc();

    /* 0x48 */ mVec3_c mStart;
    /* 0x54 */ mVec3_c mEnd;
    /* 0x60 */ f32 mRatio;
    /* 0x64 */ f32 _64;
    /* 0x68 */ nw4r::ut::Color mColorOn;
    /* 0x6C */ int mTimer;

    static dBusTunnelLight_c *sInstance;
    static dBusTunnelLight_c *getInstance() { return sInstance; }
}; // size 0x70

class dPlayerFollowSpotLight_c : public dLight::SpotLight_c {
public:
    dPlayerFollowSpotLight_c() : mOffset(0.0f, 0.0f, 0.0f), mAngX(0), mAngY(0) {
        mLightColor = ::mColor(0x000000FF);
        mVec3_c dir(0.0f, 0.0f, -1.0f);
        dir.rotX(mAngX);
        dir.rotY(mAngY);
        mDir = dir;
    }
    virtual void calc();

    /* 0x40 */ mVec3_c mOffset;
    /* 0x4C */ mAng mAngX;
    /* 0x4E */ mAng mAngY;
}; // size 0x50

// Point light at the weighted center of the glows of the frame (fireworks, thunder).
class dEffectLight_c : public dLight::PointLight_c {
public:
    enum { GLOW_MAX = 24 };

    struct Glow_c {
        Glow_c() {}
        ~Glow_c() {}

        /* 0x00 */ mVec3_c mPos;
        /* 0x0C */ nw4r::ut::Color mColor;
        /* 0x10 */ f32 mSize;
    }; // size 0x14

    dEffectLight_c();
    virtual ~dEffectLight_c();
    virtual void apply();

    BOOL addGlow(const nw4r::ut::Color *color, const nw4r::math::VEC3 *pos, f32 size);

    /* 0x02C */ Glow_c mGlows[GLOW_MAX];
    /* 0x20C */ int mGlowNum;
    /* 0x210 */ mColor mGlowColor;
    /* 0x214 */ mVec3_c mCenter;
    /* 0x220 */ mVec3_c mPrevCenter;
    /* 0x22C */ f32 mTotalSize;
    /* 0x230 */ f32 mBrightness;
    /* 0x234 */ f32 mScale;
    /* 0x238 */ u8 mInit;

    static dEffectLight_c *sInstance;
}; // size 0x23C

class dBookLight_c : public dFixFarLight_c {
public:
    dBookLight_c(Data_c *data) : dFixFarLight_c(data) {}
    virtual ~dBookLight_c();
    virtual void apply();
    virtual void regist();

    static int sLightObjIdx;
}; // size 0x28

class dBookAmbLight_c : public dLight::AmbLight_c {
public:
    dBookAmbLight_c(dFixAmbLight_c::Data_c *data) : mData(data) {
        mLightColor = mColor(data->mColor);
    }
    virtual void calc();

    /* 0x14 */ dFixAmbLight_c::Data_c *mData;
}; // size 0x18

class dEnvironment_c : public fBase_c {
public:
    enum { LIGHT_MAX = 16 };

    enum LIGHT_TYPE_e {
        LIGHT_SUN,
        LIGHT_MOON,
        LIGHT_SUB_ROOM,
        LIGHT_SUB_FIELD,
        LIGHT_ENV_AMB,
        LIGHT_FIX_AMB,
        LIGHT_FIX_AMB_SWITCH,
        LIGHT_CHECKP_AMB,
        LIGHT_BUS_AMB,
        LIGHT_FIX_FAR,
        LIGHT_FIX_FAR_SWITCH,
        LIGHT_FIX_FAR_SWITCH2,
        LIGHT_FIX_SPOT,
        LIGHT_FIX_SPOT_SWITCH,
        LIGHT_FIX_SPOT_THEATER,
        LIGHT_RESET_CENTER,
        LIGHT_CHECKP,
        LIGHT_BUS,
        LIGHT_BUS_TUNNEL,
        LIGHT_EFFECT,
        LIGHT_PLAYER_FOLLOW_SPOT,
        LIGHT_SUN_RM_INS,
        LIGHT_MOON_RM_INS,
        LIGHT_23,
        LIGHT_BOOK,
        LIGHT_BOOK_AMB,
        LIGHT_TYPE_NUM,
    };

    // The time-of-day color tables (calcColor).
    enum COLOR_e {
        COLOR_SUN,
        COLOR_MOON,
        COLOR_SUB_FIELD,
        COLOR_AMB,
        COLOR_SKY0,
        COLOR_SKY1,
        COLOR_CLOUD0,
        COLOR_CLOUD1,
        COLOR_8,
        COLOR_PARTICLE,
        COLOR_NUM,
    };

    typedef dLight::LightReq_c LightReq_c;

    dEnvironment_c()
        : mFogStart(512.0f), mFogColor(0x000000FF), mLightRatio(1.0f), mFixAlpha(0xFF), mAlpha(0xFF),
          mStarAlpha(0xFF), mUseFixAlpha(FALSE) {
        for (int i = 0; i < LIGHT_MAX; i++) {
            mLights[i] = NULL;
        }
    }
    virtual ~dEnvironment_c() {}

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    dLight::AbstractLight_c *createSunLight(void *data);
    dLight::AbstractLight_c *createMoonLight(void *data);
    dLight::AbstractLight_c *createSubLightRoom(void *data);
    dLight::AbstractLight_c *createSubLightField(void *data);
    dLight::AbstractLight_c *createEnvAmbLight(void *data);
    dLight::AbstractLight_c *createFixAmbLight(void *data);
    dLight::AbstractLight_c *createFixAmbSwitchLight(void *data);
    dLight::AbstractLight_c *createCheckPAmbLight(void *data);
    dLight::AbstractLight_c *createBusAmbLight(void *data);
    dLight::AbstractLight_c *createFixFarLight(void *data);
    dLight::AbstractLight_c *createFixFarSwitchLight(void *data);
    dLight::AbstractLight_c *createFixFarSwitch2Light(void *data);
    dLight::AbstractLight_c *createFixSpotLight(void *data);
    dLight::AbstractLight_c *createFixSpotSwitchLight(void *data);
    dLight::AbstractLight_c *createFixSpotTheaterLight(void *data);
    dLight::AbstractLight_c *createResetCenterLight(void *data);
    dLight::AbstractLight_c *createCheckPLight(void *data);
    dLight::AbstractLight_c *createBusLight(void *data);
    dLight::AbstractLight_c *createBusTunnelLight(void *data);
    dLight::AbstractLight_c *createEffectLight(void *data);
    dLight::AbstractLight_c *createPlayerFollowSpotLight(void *data);
    dLight::AbstractLight_c *createSunLightRmIns(void *data);
    dLight::AbstractLight_c *createMoonLightRmIns(void *data);
    dLight::AbstractLight_c *createBookLight(void *data);
    dLight::AbstractLight_c *createBookAmbLight(void *data);
    dLight::AbstractLight_c *createLight(int type, void *data);

    void addLight(const LightReq_c *req) {
        dLight::AbstractLight_c *light = createLight(req->mType, req->mData);
        for (int i = 0; i < LIGHT_MAX; i++) {
            if (mLights[i] == NULL) {
                mLights[i] = light;
                light->regist();
                break;
            }
        }
    }

    u8 getAlphaTable(int weather, int hour);
    f32 getFogTable(int weather, int hour);

    static BOOL isCloudy();
    static BOOL isNextCloudy();
    static mAng getSunAngle();
    static mAng getMoonAngle();
    static mAng getCurveAngle();
    static f32 getHourRatio();
    static f32 calcHourBlend(f32 ratio);
    static f32 getHourBlend();
    static void calcColor(mColor *color, int type);
    static void setCheckPRatio(f32 value);
    static void setBusRatio(f32 value);
    static u8 getShadowAlpha();
    static u8 getAlpha();
    static u8 getStarAlpha();
    static const GXColor *getColor(int type, int weather, int hour);

    /* 0x064 */ dLight::AbstractLight_c *mLights[LIGHT_MAX];
    /* 0x0A4 */ dFireWorksMgr_c mFireWorks;
    /* 0x1F4 */ f32 mFogStart;
    /* 0x1F8 */ mColor mFogColor;
    /* 0x1FC */ f32 mLightRatio;
    /* 0x200 */ u8 mFixAlpha;
    /* 0x201 */ u8 mAlpha;
    /* 0x202 */ u8 mStarAlpha;
    /* 0x203 */ u8 mUseFixAlpha;

    static dEnvironment_c *sInstance;
    static f32 sFlashR;
    static f32 sFlashG;
    static f32 sFlashB;
    static f32 sFlashA;
    static f32 sFlashSpeed;
}; // size 0x204
