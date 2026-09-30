#include "game/mLib/m_angle.hpp"
#include "game/mLib/m_vec.hpp"
#include <game/game/d_actor.hpp>
#include <game/game/d_base.hpp>
#include <game/game/d_world.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <game/mLib/m_mtx.hpp>
#include <nw4r/ut.h>

const mVec3_c* dActor_c::m_tmpCtPosP;
const mAng3_c* dActor_c::m_tmpCtAngleP;

dActor_c::dActor_c() : fBase_c() {
    if (m_tmpCtPosP != nullptr) {
        mPos = *m_tmpCtPosP;
    }

    if (m_tmpCtAngleP != nullptr) {
        mAngle = *m_tmpCtAngleP;
        mAngle3D = *m_tmpCtAngleP;
    }
}

int dActor_c::preCreate() {
    mLastPos = mPos;
    return SUCCEEDED;
}

void dActor_c::postCreate(MAIN_STATE_e state) {
    fBase_c::postCreate(state);
}

int dActor_c::preDelete() {
    return fBase_c::preDelete() != NOT_READY;
}

void dActor_c::postDelete(MAIN_STATE_e state) {
    fBase_c::postDelete(state);
}

int dActor_c::preExecute() {
    if (fBase_c::preExecute() == NOT_READY) {
        return NOT_READY;
    }
    mLastPos = mPos;
    return SUCCEEDED;
}

void dActor_c::postExecute(MAIN_STATE_e state) {}

int dActor_c::preDraw() {
    return fBase_c::preDraw() != NOT_READY;
}

void dActor_c::postDraw(MAIN_STATE_e state) {}

void dActor_c::setTmpCtData(const mVec3_c *position, const mAng3_c *rotation) {
    m_tmpCtPosP = position;
    m_tmpCtAngleP = rotation;
}

dActor_c *dActor_c::construct(ProfileName profile, fBase_c *parent, unsigned long param,
                             const mVec3_c *position, const mAng3_c *rotation) {
    setTmpCtData(position, rotation);
    return static_cast<dActor_c *>(dBase_c::createBase(profile, parent, param, ACTOR));
}

void dActor_c::calcSpeed() {
    float sin = mAngle3D.y.sin();
    float cos = mAngle3D.y.cos();
    float newZ = mSpeedF * cos;
    mSpeed.y = nw4r::ut::Max<float>(mSpeed.y + mAccelY, mMaxSpeedY);
    mSpeed.z = newZ;
    mSpeed.x = mSpeedF * sin;
}

void dActor_c::makeMtx(mMtx_c *matrix, const mVec3_c *position, const mAng *yaw) {
    mVec3_c convertedPos;
    u16 rotation = dWorld::curvePosition(&convertedPos, position);
    PSMTXTrans(*matrix, convertedPos.x, convertedPos.y, convertedPos.z);
    matrix->XrotM(mAng(rotation));
    matrix->YrotM(*yaw);
}

float dActor_c::getSpeedF(const mVec3_c *speed) {
    float cos = mAngle3D.y.cos();
    float sin = mAngle3D.y.sin();
    return speed->z * cos + speed->x * sin;
}

s16 dActor_c::targetAngleY(const mVec3_c *origin, const mVec3_c *target) {
    return cM::atan2s(target->x - origin->x, target->z - origin->z);
}

void dActor_c::getOffsetPos(mVec3_c *result, float distance, int snapToCardinal) {
    s16 angle;
    if (snapToCardinal) {
        angle = (static_cast<s16>(mAngle.y) + 0x2000) & 0xC000;
    } else {
        angle = mAngle.y;
    }
    mVec3_c offset(0.0f, 0.0f, distance);
    cLib::offsetPos(result, mPos, angle, offset);
}
