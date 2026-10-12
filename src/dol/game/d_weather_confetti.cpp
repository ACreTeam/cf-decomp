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

#include <game/game/d_event.hpp>
#include <nw4r/g3d/g3d_draw1mat1shp.h>

namespace dWeatherConfetti {

static const GXColor sColors[6] = {{255, 100, 100, 255}, {255, 255, 100, 255},
                                   {255, 100, 255, 255}, {100, 255, 100, 255},
                                   {100, 255, 255, 255}, {100, 100, 255, 255}};

void getBounds(mVec3_c *min, mVec3_c *max) {
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

void Particle_c::init(const mVec3_c &pos) {
    mPos = pos;
    mSpeed = mVec3_c::Zero;
    mActive = true;
    mFading = false;
    mColor = cM::rndRange(0, 6);
    mAngle = cM::rndRange(-32767, 32767);
    mAlpha = 0;
    mAngleSpeed = cM::rndRange(-512, 512);
    mRotation =
        mAng3_c(cM::rndRange(-32768, 32767), cM::rndRange(-32768, 32767),
                cM::rndRange(-32768, 32767));
    mSpin =
        mAng3_c(cM::rndRange(mAng::fromDegree(3.0f), mAng::fromDegree(9.0f)),
                cM::rndRange(mAng::fromDegree(3.0f), mAng::fromDegree(9.0f)),
                cM::rndRange(mAng::fromDegree(3.0f), mAng::fromDegree(9.0f)));
}

BOOL Particle_c::calc(BOOL remove) {
    mVec3_c min, max;
    getBounds(&min, &max);

    if (mFading) {
        if (sLib::chase(&mAlpha, 0, 4)) {
            if (remove) {
                mActive = false;
                return FALSE;
            }
            mVec3_c curved;
            mVec3_c pos(cM::rndRange(min.x, max.x), max.y,
                        cM::rndRange(min.z, max.z));
            dWorld::curvePositionSimple(&curved, &pos);
            init(curved);
        } else {
            s16 x = abs(mRotation.x.mAngle - mGroundAngle.mAngle) < 0x4000
                        ? mGroundAngle.mAngle
                        : (s16)(mGroundAngle.mAngle + 0x8000);
            s16 z = abs(mRotation.z.mAngle) < 0x4000 ? 0 : -0x8000;
            sLib::addCalcAngle(&mRotation.x.mAngle, x, 8, 0x800);
            sLib::addCalcAngle(&mRotation.z.mAngle, z, 8, 0x800);
            mRotation.y += mSpin.y;
            sLib::chaseAngle(&mSpin.y.mAngle, 0, 0x40);
            mAngle += mAngleSpeed;
            mSpeed *= 0.9f;
            mPos += mSpeed;
        }
        return TRUE;
    } else {
        mRotation += mSpin;
        sLib::chase(&mAlpha, 255, 32);
        mAngle += mAngleSpeed;
        mAng cameraAngle = lbl_8074E9B0->getCurveAngle();
        f32 x = mAngle.sin();
        f32 y = mAng(mAngle.mAngle * 2).cos();
        f32 z = mAng(mAngle.mAngle * 3).cos();
        mVec3_c accel(x * 0.03f, y * 0.03f + -0.06f, z * 0.03f);
        accel.rotX(cameraAngle);
        accel.x += 0.05f;
        mSpeed += accel;
        mSpeed *= 0.9f;
        mPos += mSpeed;

        mVec3_c pos;
        dWorld::uncurvePosition(&pos, &mPos);

        bool wrap = false;
        if (dSTR::gArea.isInside(pos) ||
            (pos.y < dBGCF::getGroundY(&pos, FALSE) + 2.0f)) {
            mSpin.y = 0x800;
            mSpeed.y = mSpeed.z = 0.0f;
            mGroundAngle = cM::atan2s(mPos.z, mPos.y);
            mFading = true;
        } else if (pos.y < min.y) {
            mSpin.y = 0x800;
            mSpeed.y = mSpeed.z = 0.0f;
            mGroundAngle = cM::atan2s(mPos.z, mPos.y);
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

f32 Particle_c::getFade(f32 distance) {
    f32 near = 50.0f;
    f32 nearFull = 150.0f;
    f32 farFull = 600.0f;
    f32 far = 800.0f;
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

void Particle_c::draw(nw4r::g3d::ResShp shape, const mMtx_c &view) {
    mMtx_c matrix;
    PSMTXTrans(matrix, mPos.x, mPos.y, mPos.z);
    matrix.XrotM(mRotation.x);
    matrix.YrotM(mRotation.y);
    matrix.ZrotM(mRotation.z);

    mMtx_c normal;
    normal = matrix;
    PSMTXConcat(view, matrix, matrix);
    matrix.multVecZero(mViewPos);

    f32 size = 0.75f;
    f32 fade = getFade(-matrix.mtx[2][3]);
    if (fade <= 0.0f) {
        return;
    }

    size *= fade;
    u8 alpha = fade * 255.0f;
    alpha = alpha * mAlpha / 255.0f;
    mMtx_c scale;
    PSMTXScale(scale, size, size, size);
    PSMTXConcat(matrix, scale, matrix);
    PSMTXConcat(view, normal, normal);

    GXColor color = sColors[mColor];
    color.a = alpha;
    GXSetTevColor(GX_TEVREG0, color);

    ::mColor tint;
    ::mColor black(0x000000FF);
    tint.lerp(black, sColors[mColor], 0.4f);
    GXSetTevKColor(GX_KCOLOR0, tint);

    nw4r::g3d::G3DState::SetViewPosNrmMtxArray(&matrix, NULL, NULL);
    nw4r::g3d::Draw1Mat1ShpDirectly(nw4r::g3d::ResMat(NULL), shape, &matrix,
                                    &normal, 0, NULL, NULL);
}

} // namespace dWeatherConfetti

BOOL dWeatherConfettiDraw_c::create(nw4r::g3d::ResFile *file,
                                    mAllocator_c *allocator) {
    nw4r::g3d::ResMdl model = file->GetResMdl("ef_kamifubuki");
    mMaterial = model.GetResMat(0);
    mShape = model.GetResShp(0);
    m3d::proc_c::create(allocator, NULL);
    mCameraPos = lbl_8074E9B0->getTargetPos();
    mTargetCount = 0.0f;
    mCount = 0;

    if (!dScene::isCityScene(dScene::getCurrentScene()) &&
        !dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_ROOM | dScene::SCENE_ATTR_CHKP) &&
        !dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_BUS) &&
        dEvent::isOngoing(EVENT_FESTIVALE)) {
        mEnabled = true;
    } else {
        mEnabled = false;
    }
    return TRUE;
}

void dWeatherConfettiDraw_c::init() {
    mVec3_c min, max;
    dWeatherConfetti::getBounds(&min, &max);
    int active = 0;
    int count = 0;
    if (mEnabled) {
        count = 120;
    }
    for (int i = 0; i < 120; i++) {
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

BOOL dWeatherConfettiDraw_c::calc() {
    int count = 0;
    if (mEnabled) {
        count = 120;
    }
    mVec3_c camera = lbl_8074E9B0->getTargetPos();
    if (PSVECSquareDistance((const Vec *)&camera, (const Vec *)&mCameraPos) >
        1024.0f) {
        init();
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
    dWeatherConfetti::getBounds(&min, &max);
    for (int i = 0; i < 120; i++) {
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

BOOL dWeatherConfettiDraw_c::draw() {
    if (mCount) {
        setPriorityDraw(0x7f, 0x84);
        entry();
    }
    return TRUE;
}

void dWeatherConfettiDraw_c::drawXlu() {
    m3d::resetMaterial();
    nw4r::g3d::Draw1Mat1ShpDirectly(mMaterial, nw4r::g3d::ResShp(NULL), NULL,
                                    NULL, 0, NULL, NULL);
    mMtx_c view;
    getViewMtx(&view);
    for (int i = 0; i < 120; ++i) {
        if (mParticles[i].mActive) {
            mParticles[i].draw(mShape, view);
        }
    }
}
