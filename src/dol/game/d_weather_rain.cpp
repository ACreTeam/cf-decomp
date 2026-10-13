// Rain emitters for the town and the bus ride.
#include <game/game/d_weather.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/cLib/c_math.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/sLib/s_lib.hpp>

extern "C" f32 fn_801973F0();

namespace dWeatherRain {

BOOL Rain_c::init(int weather) {
    if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_BUS)) {
        mGround.create("af_bus_rain");
        mWater.create("afm_fld_rain_water_st");
    } else {
        mGround.create("afm_fld_rain_ground_st");
        mWater.create("afm_fld_rain_water_st");
    }

    switch (weather) {
    case dWeather::HEAVY_RAIN:
        mRate = 6.0f;
        break;
    case dWeather::RAIN:
        mRate = 3.0f;
        break;
    default:
        mRate = 0.0f;
        break;
    }
    mFade = 0.0f;
    return TRUE;
}

BOOL Rain_c::remove() {
    return TRUE;
}

f32 Rain_c::getRate(int weather, int nextWeather) {
    bool rain = false;
    f32 rate;
    switch (weather) {
    case dWeather::HEAVY_RAIN:
        rate = 6.0f;
        rain = true;
        break;
    case dWeather::RAIN:
        rate = 3.0f;
        rain = true;
        break;
    default:
        rate = 0.0f;
        break;
    }
    f32 result = 0.0f;
    if (rain) {
        f32 next;
        switch (nextWeather) {
        case dWeather::HEAVY_RAIN:
            next = 6.0f;
            break;
        case dWeather::RAIN:
            next = 3.0f;
            break;
        default:
            next = 0.0f;
            break;
        }
        f32 blend = fn_801973F0();
        result = next * blend + rate * (1.0f - blend);
    }
    return result;
}

f32 Rain_c::getLevel(int weather, int nextWeather) {
    return getRate(weather, nextWeather) * (1.0f / 6.0f);
}

BOOL Rain_c::calc(int weather, int nextWeather) {
    f32 target = getRate(weather, nextWeather);
    if (mRate < target) {
        sLib::chase(&mRate, target, 0.004f);
    } else {
        mRate = target;
    }

    mFade += mRate;
    int count = mFade;
    mFade -= count;

    mVec3_c pos;
    mVec3_c center;
    if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_ROOM | dScene::SCENE_ATTR_CHKP)) {
        center.set(256.0f, 0.0f, 80.0f);
    } else {
        center = lbl_80623FEC;
    }

    for (int i = 0; i < count; ++i) {
        pos = center;
        pos.x += cM::rndRange(-150.0f, 150.0f);
        pos.z += cM::rndRange(-250.0f, 150.0f);
        dBGCF::groundChk_c ground(&pos, 0, 0, 0);
        pos.y = ground.getHeight(FALSE);
        mAng3_c angle(0, cM::rad2s(cM::rndCenterF(1.5707964f)), 0);
        if (ground.mWater) {
            pos.y = ground.mWaterY;
            mWater.emit(&pos, &angle, NULL);
        } else {
            mGround.emit(&pos, &angle, NULL);
        }
    }
    return TRUE;
}

BOOL Rain_c::calcBus(int weather) {
    static dLevelEffect_c effect;
    int count;
    int windowCount;
    f32 windowRate;
    switch (weather) {
    case dWeather::HEAVY_RAIN:
        count = 12;
        windowCount = 6;
        windowRate = 2.0f;
        sLib::chase(&mRate, 6.0f, 0.004f);
        break;
    case dWeather::RAIN:
        count = 6;
        windowCount = 3;
        windowRate = 1.0f;
        sLib::chase(&mRate, 3.0f, 0.004f);
        break;
    default:
        sLib::chase(&mRate, 0.0f, 0.003f);
        return FALSE;
    }

    mVec3_c pos;
    mVec3_c center;
    mVec3_c min;
    mVec3_c max;
    min.set(192.0f, 0.0f, 256.0f);
    max.set(370.0f, 81.0f, 500.0f);
    center.set(256.0f, 0.0f, 0.0f);
    mAng nearAngle(mAng::fromDegree(7.5f));
    mAng farAngle(mAng::fromDegree(82.5f));
    mAng cameraAngle = lbl_8074E9B0->getYaw();
    f32 farCos = farAngle.cos();
    f32 nearCos = nearAngle.cos();
    f32 blend = (cameraAngle.cos() - farCos) / (nearCos - farCos);
    blend = blend < 0.0f ? 0.0f : blend > 1.0f ? 1.0f : blend;
    f32 other = 1.0f - blend;
    f32 rainAngle = blend * -30.0f + other * -50.0f;
    f32 spread = other * 3.0f;
    f32 tiltAngle = blend * -7.0f;
    f32 minX = blend * -128.0f + other * -128.0f;
    f32 maxX = blend * 128.0f + other * -32.0f;
    f32 maxZ = blend * 400.0f + other * 550.0f;
    f32 minZ = blend * -64.0f + other * 300.0f;
    f32 random = cM::rndRange(-spread, spread);
    mAng slant(mAng::fromDegree(rainAngle + random));
    mAng tilt(mAng::fromDegree(tiltAngle));
    f32 slope = -slant.sin();
    f32 front = min.z + slope * max.y;
    f32 back = max.z + slope * max.y;

    for (int i = 0; i < count; ++i) {
        pos = center;
        pos.x += cM::rndRange(minX, maxX);
        pos.z += cM::rndRange(minZ, maxZ);
        dBGCF::groundChk_c ground(&pos, 0, 0, 0);
        mAng3_c angle;
        angle.x = slant.mAngle;
        angle.y = 0;
        angle.z = tilt.mAngle;
        if (pos.x > min.x && pos.x < max.x && pos.z > min.z && pos.z < back) {
            if (pos.z < front && nw4r::math::FAbs(slope) > FLT_EPSILON) {
                pos.y = (pos.z - min.z) / slope;
                pos.z = min.z;
            } else {
                pos.y = max.y;
                pos.z -= max.y * slope;
            }
        }
        mGround.emit(&pos, &angle, NULL);
    }

    mMtx_c matrix;
    PSMTXIdentity(matrix);
    mSplash.create("af_bus_rain_window00", &matrix, 0);
    mSplash.vf2C(windowCount);
    mRipple.create("af_bus_rain_window01", &matrix, 0);
    mRipple.vf2C(windowRate);
    return TRUE;
}

f32 Rain_c::getLevel() {
    f32 rate = mRate;
    return rate * (1.0f / 6.0f);
}

} // namespace dWeatherRain
