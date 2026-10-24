// EGG::BaseCamera / EGG::LookAtCamera. .text 804459FC..80445D18
#include <lib/egg/gfxe/eggCamera.h>
#include <nw4r/g3d.h>
#include <revolution/gx.h>

namespace EGG {

void BaseCamera::updateMatrix() {
    doUpdateMatrix();
}

void BaseCamera::draw(BaseCamera *camera) {
    camera->loadMatrix();
    doDraw();
}

void BaseCamera::setG3DCamera(nw4r::g3d::Camera &camera) {
    camera.SetCameraMtxDirectly(getViewMatrix());
}

Matrix34f &LookAtCamera::getViewMatrixOld() {
    return mOldViewMtx;
}

void LookAtCamera::doUpdateMatrix() {
    mOldViewMtx = mViewMtx;

    Vector3f right(mPos);
    Vector3f forward;

    right.x -= mAt.x;
    right.y -= mAt.y;
    right.z -= mAt.z;
    right.normalise();

    forward = mUp.cross(right);
    forward.normalise();

    Vector3f up(right.cross(forward));
    up.normalise();

    f32 t[3];

    f32 tx = -forward.dot(mPos);
    f32 ty = -up.dot(mPos);
    f32 tz = -right.dot(mPos);
    t[0] = tx;
    t[1] = ty;
    t[2] = tz;

    mViewMtx(0, 0) = forward.x;
    mViewMtx(0, 1) = forward.y;
    mViewMtx(0, 2) = forward.z;
    mViewMtx(0, 3) = tx;

    mViewMtx(1, 0) = up.x;
    mViewMtx(1, 1) = up.y;
    mViewMtx(1, 2) = up.z;
    mViewMtx(1, 3) = ty;

    mViewMtx(2, 0) = right.x;
    mViewMtx(2, 1) = right.y;
    mViewMtx(2, 2) = right.z;
    mViewMtx(2, 3) = tz;
}

void LookAtCamera::doDraw() {}

void LookAtCamera::loadMatrix() {
    Matrix34f mtx;
    GXLoadPosMtxImm(mViewMtx, 0);
    mViewMtx.inverseTransposeTo(mtx);
    GXLoadNrmMtxImm(mtx, 0);
}

void LookAtCamera::loadOldMatrix() {}

} // namespace EGG
