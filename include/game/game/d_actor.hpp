#pragma once

#include <types.h>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_vec.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_base.hpp>

class dActor_c : public fBase_c {
public:
    dActor_c();

    virtual int preCreate();
    virtual void postCreate(MAIN_STATE_e state);
    virtual int preDelete();
    virtual void postDelete(MAIN_STATE_e state);
    virtual int preExecute();
    virtual void postExecute(MAIN_STATE_e state);
    virtual int preDraw();
    virtual void postDraw(MAIN_STATE_e state);
    virtual ~dActor_c() {};

    static void setTmpCtData(const mVec3_c *position, const mAng3_c *rotation);
    static dActor_c *construct(ProfileName profile, fBase_c *parent, unsigned long param,
                               const mVec3_c *position, const mAng3_c *rotation);

    void calcSpeed();
    static void makeMtx(mMtx_c *matrix, const mVec3_c *position, const mAng *yaw);
    float getSpeedF(const mVec3_c *speed);
    static s16 targetAngleY(const mVec3_c *origin, const mVec3_c *target);
    void getOffsetPos(mVec3_c *result, float distance, int snapToCardinal);

    static const mVec3_c* m_tmpCtPosP;
    static const mAng3_c* m_tmpCtAngleP;

protected:
    mVec3_c mPos;
    mVec3_c mLastPos;
    mAng3_c mAngle;
    mAng3_c mAngle3D;
    float mSpeedF;
    float mAccelY;
    float mMaxSpeedY;
    mVec3_c mSpeed;
};
