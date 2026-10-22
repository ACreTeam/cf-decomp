#pragma once

#include <lib/egg/math/eggMatrix.h>
#include <lib/egg/math/eggVector.h>
#include <nw4r/math.h>

namespace nw4r { namespace g3d { class Camera; } }

// EGG cameras (eggCamera.cpp 804459FC..80445D18). The vtable layout is from LookAtCamera's vtable
// (80561958); LookAtCamera's mPos/mAt/mUp offsets match dCamera_c's ctor (801870E4).

namespace EGG {

class BaseCamera {
public:
    virtual Matrix34f &getViewMatrix() = 0;             // 0x08
    virtual const Matrix34f &getViewMatrix() const = 0; // 0x0C
    virtual void updateMatrix();                        // 0x10
    virtual void doUpdateMatrix() = 0;                  // 0x14
    virtual void loadMatrix() = 0;                      // 0x18
    virtual void loadOldMatrix() = 0;                   // 0x1C
    virtual Vector3f getPosition() = 0;                 // 0x20
    virtual void draw(BaseCamera *camera);              // 0x24
    virtual void doDraw() = 0;                          // 0x28
    virtual Matrix34f &getViewMatrixOld() = 0;          // 0x2C

    void setG3DCamera(nw4r::g3d::Camera &camera);
};

class LookAtCamera : public BaseCamera {
public:
    LookAtCamera() : mPos(0.0f, 10.0f, 0.0f), mAt(0.0f, 0.0f, 0.0f), mUp(0.0f, 1.0f, 0.0f) {}

    // inline: the kept weak copies are 800107A4 (d_snd_util) and 80445D10 (eggCamera.cpp's tail)
    virtual Matrix34f &getViewMatrix() { return mViewMtx; }
    virtual const Matrix34f &getViewMatrix() const { return mViewMtx; }
    virtual void doUpdateMatrix();
    virtual void loadMatrix();
    virtual void loadOldMatrix();
    virtual Vector3f getPosition() { return mPos; }
    virtual void doDraw();
    virtual Matrix34f &getViewMatrixOld();

    /* 0x04 */ Matrix34f mViewMtx;
    /* 0x34 */ Matrix34f mOldViewMtx;
    /* 0x64 */ Vector3f mPos;
    /* 0x70 */ Vector3f mAt;
    /* 0x7C */ Vector3f mUp;
}; // size 0x88

} // namespace EGG
