#pragma once

#include <types.h>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_mtx.hpp>
#include <lib/egg/math/eggQuat.h>
#include <lib/egg/core/eggHeap.h>

// The held/released balloon's string, allocated in the tool bank's heap.
class dBalloonString_c : public m3d::proc_c {
public:
    dBalloonString_c() : mAlpha(0xFF), mInit(false), mFly(false) {}
    virtual ~dBalloonString_c();                     // 80066018
    virtual bool entry();                           // 80067C0C
    virtual void drawXlu();                          // 80067C24
    virtual const mMtx_c *calc(const mMtx_c *mtx);     // 80067BA0
    virtual const mMtx_c *calcInit(const mMtx_c *mtx);   // 80066358
    virtual const mMtx_c *calcHold(const mMtx_c *mtx);   // 80066670
    virtual const mMtx_c *calcFly(const mMtx_c *mtx);    // 8006706C

    void release(mAng angle);                       // 80067BF0
    void destroy();                                // 800680F4: destruct in place
    static dBalloonString_c *create(EGG::Heap *heap); // 80068108

    bool createProc();
    void setInitialDirection(const mVec3_c &dir);
    void updateRotation(const mVec3_c &dir);

    /* 0x008 */ mAllocator_c mAllocator;
    /* 0x024 */ EGG::Quatf mRotation;
    /* 0x034 */ mMtx_c mTransform;
    /* 0x064 */ mMtx_c mDrawMtx;
    /* 0x094 */ mVec3_c mPoints[8];
    /* 0x0F4 */ mVec3_c mTangents[8];
    /* 0x154 */ mVec3_c mVelocities[8];
    /* 0x1B4 */ mVec3_c mScale;
    /* 0x1C0 */ s16 mTimer;
    /* 0x1C2 */ mAng mReleaseAngle; // player's heading at release
    /* 0x1C4 */ u8 mAlpha;
    /* 0x1C5 */ u8 mInit;
    /* 0x1C6 */ u8 mFly; // released (flying away)
}; // size 0x1C8
