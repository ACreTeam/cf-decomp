#include <game/game/d_weather.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_world.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_light.hpp>
#include <game/cLib/c_math.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/mLib/m_color.hpp>
#include <nw4r/g3d.h>
#include <revolution/GX.h>

namespace dWeatherSnow {

int getLightCount() {
    return 100;
}

int getHeavyCount() {
    return 200;
}

mVec3_c sBusMin(64.0f, 20.0f, -200.0f);
mVec3_c sBusMax(400.0f, 128.0f, 310.0f);
mVec3_c sBusSideMin(-64.0f, 20.0f, 192.0f);
mVec3_c sBusSideMax(288.0f, 128.0f, 620.0f);
mColor sWhite(255, 255, 255, 255);
mColor sBlue(0, 0, 255, 255);
mVec3_c sMin(-100.0f, 0.0f, -100.0f);
mVec3_c sMax(100.0f, 100.0f, 100.0f);
bool sFixedBounds;
bool sCustomFade;
bool sCheckGround = true;
f32 sFadeNear = 50.0f;
f32 sFadeNearFull = 250.0f;
f32 sFadeFarFull = 600.0f;
f32 sFadeFar = 800.0f;

void getBounds(mVec3_c *min, mVec3_c *max) {
    if (sFixedBounds) {
        *min = sMin;
        *max = sMax;
    } else {
        mVec3_c center(*getViewCenter<nw4r::math::VEC3>());
        *min = center;
        *max = center;
        min->x -= 200.0f;
        max->x += 200.0f;
        min->y += -76.0f;
        max->y += 180.0f;
        min->z += -300.0f;
        max->z += 160.0f;
    }
}

void Particle_c::init(const mVec3_c &pos) {
    mPos = pos;
    mSpeed = mVec3_c::Zero;
    mActive = true;
    mAngle = cM::rndRange(-32767, 32767);
    mAlpha = 0;
    mAngleSpeed = cM::rndRange(-512, 512);
    _1D = 0;
    mFading = false;
}

BOOL Particle_c::calc(BOOL remove) {
    mVec3_c min, max;
    getBounds(&min, &max);

    if (mFading) {
        if (sLib::chase(&mAlpha, 0, 16)) {
            if (remove) {
                mActive = false;
                return FALSE;
            }
            mVec3_c curved;
            mVec3_c pos(cM::rndRange(min.x, max.x), max.y,
                        cM::rndRange(min.z, max.z));
            dWorld::curvePosition(&curved, &pos);
            init(curved);
        }
        return TRUE;
    } else {
        sLib::chase(&mAlpha, 255, 32);
        mAngle += mAngleSpeed;
        mAng cameraAngle = dCamera_c::sInstance->getCurveAngle();
        f32 x = mAngle.sin();
        f32 y = mAng(mAngle.mAngle * 2).cos();
        f32 z = mAng(mAngle.mAngle * 3).cos();
        mVec3_c accel(x * 0.02f, y * 0.02f + -0.08f, z * 0.02f);
        accel.rotX(cameraAngle);
        mSpeed += accel;
        mSpeed *= 0.925f;
        mPos += mSpeed;

        mVec3_c pos;
        dWorld::toFieldPosition(&pos, &mPos);

        bool wrap = false;
        if (dSTR::gArea.isInside(pos)) {
            mFading = true;
        } else if (sCheckGround && pos.y < dBGCF::getGroundY(&pos, 0) + 1.5f) {
            mFading = true;
        } else if (pos.y < min.y) {
            mFading = true;
        } else {
            if (pos.x < min.x) {
                pos.x += max.x - min.x;
                wrap = true;
            } else if (pos.x > max.x) {
                pos.x -= max.x - min.x;
                wrap = true;
            }

            if (pos.z < min.z) {
                pos.z += max.z - min.z;
                wrap = true;
            } else if (pos.z > max.z) {
                pos.z -= max.z - min.z;
                wrap = true;
            }
        }

        if (wrap) {
            if (remove) {
                mActive = false;
                return FALSE;
            }
            mVec3_c curved;
            dWorld::curvePosition(&curved, &pos);
            init(curved);
        }
    }
    return TRUE;
}

BOOL Particle_c::calcBus(BOOL remove, f32 wind) {
    mVec3_c min, max;
    mVec3_c busMin(192.0f, 0.0f, 256.0f);
    mVec3_c busMax(370.0f, 81.0f, 500.0f);
    getBounds(&min, &max);

    if (mFading) {
        if (sLib::chase(&mAlpha, 0, 16)) {
            if (remove) {
                mActive = false;
                return FALSE;
            }
            mVec3_c curved;
            mVec3_c pos(cM::rndRange(min.x, max.x), max.y,
                        cM::rndRange(min.z, max.z));
            dWorld::curvePosition(&curved, &pos);
            init(curved);
        }
        return TRUE;
    } else {
        sLib::chase(&mAlpha, 255, 32);
        mAngle += mAngleSpeed;
        mAng cameraAngle = dCamera_c::sInstance->getCurveAngle();
        f32 x = mAngle.sin();
        f32 y = mAng(mAngle.mAngle * 2).cos();
        f32 z = mAng(mAngle.mAngle * 3).cos();
        mVec3_c accel(x * 0.02f, y * 0.02f + -0.08f, wind + z * 0.02f);
        accel.rotX(cameraAngle);
        mSpeed += accel;
        mSpeed *= 0.925f;
        mPos += mSpeed;

        mVec3_c pos;
        dWorld::toFieldPosition(&pos, &mPos);

        bool wrap = false;
        if (sLib::isInRange(pos.x, busMin.x, busMax.x) &&
            sLib::isInRange(pos.y, busMin.y, busMax.y) &&
            sLib::isInRange(pos.z, busMin.z, busMax.z)) {
            mFading = true;
        } else if (pos.y < min.y) {
            pos.x = cM::rndRange(min.x, max.x);
            pos.y = max.y;
            pos.z = cM::rndRange(min.z, max.z);
            wrap = true;
        } else {
            if (pos.x < min.x) {
                pos.x += max.x - min.x;
                wrap = true;
            } else if (pos.x > max.x) {
                pos.x -= max.x - min.x;
                wrap = true;
            }

            if (pos.z < min.z) {
                pos.z += max.z - min.z;
                wrap = true;
            } else if (pos.z > max.z) {
                pos.z -= max.z - min.z;
                wrap = true;
            }
        }

        if (wrap) {
            if (remove) {
                mActive = false;
                return FALSE;
            }
            mVec3_c curved;
            dWorld::curvePosition(&curved, &pos);
            init(curved);
        }
    }
    return TRUE;
}

f32 Particle_c::getFade(f32 distance) {
    f32 near = 50.0f;
    f32 nearFull = 250.0f;
    f32 farFull = 600.0f;
    f32 far = 800.0f;
    if (sCustomFade) {
        near = sFadeNear;
        nearFull = sFadeNearFull;
        farFull = sFadeFarFull;
        far = sFadeFar;
    }
    if (distance < near || distance > far) {
        return 0.0f;
    }
    if (distance < nearFull) {
        return (distance - near) * (1.0f / (nearFull - near));
    }
    if (distance > farFull) {
        return (far - distance) * (1.0f / (far - farFull));
    }
    return 1.0f;
}

void Particle_c::draw(const mMtx_c &view) {
    f32 size = 1.5f;
    mMtx_c matrix;
    PSMTXTrans(matrix, mPos.x, mPos.y, mPos.z);
    PSMTXConcat(view, matrix, matrix);
    matrix.mtx[0][0] = matrix.mtx[1][1] = matrix.mtx[2][2] = 1.0f;
    matrix.mtx[0][1] = matrix.mtx[0][2] = matrix.mtx[1][0] = matrix.mtx[1][2] =
        matrix.mtx[2][0] = matrix.mtx[2][1] = 0.0f;
    f32 distance = -matrix.mtx[2][3];
    f32 fade = getFade(distance);
    fade *= mAlpha * (1.0f / 255.0f);
    if (fade <= 0.0f) {
        return;
    }
    {
        size *= fade;
        u8 alpha = fade * 255.0f;
        alpha = alpha * mAlpha / 255.0f;
        GXLoadPosMtxImm(matrix, GX_PNMTX0);
        GXLoadNrmMtxImm(mMtx_c::Identity, GX_PNMTX0);
        GXSetCurrentMtx(GX_PNMTX0);
        GXColor color = fn_800CD168(0);
        color.a = alpha;
        GXSetTevColor(GX_TEVREG0, color);
        GXSetTevColor(GX_TEVREG1, color);
        mVec3_c topLeft(-size, size, 0.0f);
        mVec3_c topRight(size, size, 0.0f);
        mVec3_c bottomLeft(-size, -size, 0.0f);
        mVec3_c bottomRight(size, -size, 0.0f);
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
        GXPosition3f32(topRight.x, topRight.y, topRight.z);
        GXTexCoord2f32(1.0f, 0.0f);
        GXPosition3f32(topLeft.x, topLeft.y, topLeft.z);
        GXTexCoord2f32(0.0f, 0.0f);
        GXPosition3f32(bottomRight.x, bottomRight.y, bottomRight.z);
        GXTexCoord2f32(1.0f, 1.0f);
        GXPosition3f32(bottomLeft.x, bottomLeft.y, bottomLeft.z);
        GXTexCoord2f32(0.0f, 1.0f);
        GXEnd();
    }
}

} // namespace dWeatherSnow

BOOL dWeatherSnowDraw_c::create(nw4r::g3d::ResFile *file,
                                mAllocator_c *allocator) {
    mTexture = file->GetResTex("af_snow00_ia");
    m3d::proc_c::create(allocator, NULL);
    mCameraPos = dCamera_c::sInstance->getTargetPos();
    mTargetCount = 0.0f;
    mCount = 0;

    using namespace dWeatherSnow;
    if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_ROOM | dScene::SCENE_ATTR_CHKP)) {
        mVec3_c min(128.0f, 0.0f, -140.0f);
        mVec3_c max(384.0f, 180.0f, 200.0f);
        sFixedBounds = true;
        sMin = min;
        sMax = max;
        f32 near = 50.0f;
        f32 nearFull = 250.0f;
        f32 farFull = 320.0f;
        f32 far = 800.0f;
        sCheckGround = false;
        sCustomFade = true;
        sFadeNear = near;
        sFadeNearFull = nearFull;
        sFadeFarFull = farFull;
        sFadeFar = far;
    } else if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_BUS)) {
        sFixedBounds = true;
        sMin = sBusMin;
        sMax = sBusMax;
        f32 near = 50.0f;
        f32 nearFull = 250.0f;
        f32 farFull = 320.0f;
        f32 far = 800.0f;
        sCheckGround = false;
        sCustomFade = true;
        sFadeNear = near;
        sFadeNearFull = nearFull;
        sFadeFarFull = farFull;
        sFadeFar = far;
    } else {
        sFixedBounds = false;
        sCheckGround = true;
        sCustomFade = false;
    }
    return TRUE;
}

void dWeatherSnowDraw_c::init(int count) {
    mCameraPos = dCamera_c::sInstance->getTargetPos();
    int active = 0;
    mVec3_c min, max;
    dWeatherSnow::getBounds(&min, &max);
    for (int i = 0; i < 200; i++) {
        if (mParticles[i].mActive || count != 0) {
            mVec3_c curved;
            mVec3_c pos(cM::rndRange(min.x, max.x), cM::rndRange(min.y, max.y),
                        cM::rndRange(min.z, max.z));
            dWorld::curvePosition(&curved, &pos);
            mParticles[i].init(curved);
            if (count > 0) {
                --count;
            }
            ++active;
        }
    }

    mCount = active;
    mTargetCount = active;
}

BOOL dWeatherSnowDraw_c::calc(int weather) {
    int count = 0;
    switch (weather) {
    case dWeather::SNOW:
        count = 100;
        break;
    case dWeather::HEAVY_SNOW:
        count = 200;
        break;
    }
    mVec3_c camera = dCamera_c::sInstance->getTargetPos();
    if (PSVECSquareDistance((const Vec *)&camera, (const Vec *)&mCameraPos) >
        1024.0f) {
        init(0);
    }
    mCameraPos = camera;
    f32 target = count;

    if (mTargetCount < target) {
        sLib::chase(&mTargetCount, target, 0.25f);
    } else {
        sLib::chase(&mTargetCount, target, 0.5f);
    }

    int needed = (int)mTargetCount - mCount;
    int active = 0;
    mVec3_c min, max;
    dWeatherSnow::getBounds(&min, &max);
    for (int i = 0; i < 200; i++) {
        if (mParticles[i].mActive) {
            if (mParticles[i].calc(needed < 0)) {
                ++active;
            } else {
                ++needed;
            }
        } else if (needed > 0) {
            mVec3_c pos(cM::rndRange(min.x, max.x), cM::rndRange(min.y, max.y),
                        cM::rndRange(min.z, max.z));
            mVec3_c curved;
            dWorld::curvePosition(&curved, &pos);
            mParticles[i].init(curved);
            mParticles[i].calc(FALSE);
            ++active;
            --needed;
        }
    }

    mCount = active;

    return TRUE;
}

BOOL dWeatherSnowDraw_c::calcBus(int weather) {
    using namespace dWeatherSnow;
    mAng cameraAngle = dCamera_c::sInstance->getYaw();
    mAng nearAngle(mAng::fromDegree(7.5f)), farAngle(mAng::fromDegree(82.5f));
    f32 farCos = farAngle.cos(), nearCos = nearAngle.cos();
    f32 blend = (cameraAngle.cos() - farCos) / (nearCos - farCos);
    blend = blend < 0.0f ? 0.0f : blend > 1.0f ? 1.0f : blend;
    f32 other = 1.0f - blend;
    mVec3_c minBounds = blend * sBusMin + other * sBusSideMin;
    mVec3_c maxBounds = blend * sBusMax + other * sBusSideMax;
    sFixedBounds = true;
    sMin = minBounds;
    sMax = maxBounds;
    f32 wind = blend * 0.32f + other * 0.18f;
    int count = 0;
    switch (weather) {
    case dWeather::SNOW:
        count = 100;
        break;
    case dWeather::HEAVY_SNOW:
        count = 200;
        break;
    }
    mVec3_c camera = dCamera_c::sInstance->getTargetPos();
    if (PSVECSquareDistance((const Vec *)&camera, (const Vec *)&mCameraPos) >
        1024.0f) {
        init(0);
    }
    mCameraPos = camera;
    f32 target = count;

    if (mTargetCount < target) {
        sLib::chase(&mTargetCount, target, 0.25f);
    } else {
        sLib::chase(&mTargetCount, target, 0.5f);
    }

    int needed = (int)mTargetCount - mCount;
    int active = 0;
    mVec3_c min, max;
    getBounds(&min, &max);
    for (int i = 0; i < 200; i++) {
        if (mParticles[i].mActive) {
            if (mParticles[i].calcBus(needed < 0, wind)) {
                ++active;
            } else {
                ++needed;
            }
        } else if (needed > 0) {
            mVec3_c pos(cM::rndRange(min.x, max.x), cM::rndRange(min.y, max.y),
                        cM::rndRange(min.z, max.z));
            mVec3_c curved;
            dWorld::curvePosition(&curved, &pos);
            mParticles[i].init(curved);
            mParticles[i].calcBus(FALSE, wind);
            ++active;
            --needed;
        }
    }

    mCount = active;

    return TRUE;
}

BOOL dWeatherSnowDraw_c::draw() {
    if (mCount) {
        if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_BUS)) {
            setPriorityDraw(127, 8);
        } else {
            setPriorityDraw(127, 132);
        }
        entry();
    }
    return TRUE;
}

void dWeatherSnowDraw_c::drawXlu() {
    m3d::resetMaterial();
    nw4r::g3d::G3DState::Invalidate(0x7FF);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0, GX_TRUE, GX_SRC_REG, GX_SRC_REG, fn_800CD250(),
                  GX_DF_NONE, GX_AF_SPOT);
    GXSetChanCtrl(GX_ALPHA0, GX_TRUE, GX_SRC_REG, GX_SRC_REG, fn_800CD250(),
                  GX_DF_NONE, GX_AF_SPOT);
    GXColor white = {255, 255, 255, 255};
    GXSetChanAmbColor(GX_COLOR0A0, fn_800CD168(0));
    GXSetChanMatColor(GX_COLOR0A0, white);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_GREATER, 0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZCompLoc(GX_TRUE);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,
                      GX_FALSE, GX_PTIDENTITY);
    GXTexObj texture;
    void *data;
    u16 width, height;
    GXTexFmt format;
    f32 minLod, maxLod;
    GXBool mipmap;
    mTexture.GetTexObjParam(&data, &width, &height, &format, &minLod, &maxLod,
                            &mipmap);
    GXInitTexObj(&texture, data, width, height, format, GX_CLAMP, GX_CLAMP,
                 GX_FALSE);
    GXInitTexObjLOD(&texture, GX_NEAR, GX_NEAR, minLod, maxLod, 0.0f, GX_FALSE,
                    GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(&texture, GX_TEXMAP0);
    GXSetNumIndStages(0);
    GXSetNumTevStages(1);
    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE,
                          GX_CH_ALPHA);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A0, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_TRUE, GX_TEVPREV);
    GXSetCullMode(GX_CULL_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    mMtx_c view;
    getViewMtx(&view);
    for (int i = 0; i < 200; ++i) {
        if (mParticles[i].mActive) {
            mParticles[i].draw(view);
        }
    }
}
