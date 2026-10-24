#ifndef NW4R_EF_POST_FIELD_H
#define NW4R_EF_POST_FIELD_H
#include <nw4r/types_nw4r.h>
#include <nw4r/ef/ef_emitter.h>
#include <nw4r/math.h>

namespace nw4r {
namespace ef {

struct PostFieldInfo {
    struct AnimatableParams {
        math::VEC3 mSize, mRotate, mTranslate;
    } mAnimatableParams;
    f32 mReferenceSpeed;
    u8 mControlSpeedType, mCollisionShapeType, mCollisionShapeOption,
        mCollisionType;
    u16 mCollisionOption, mStartFrame;
    math::VEC3 mSpeedFactor;
    struct ChildOption {
        EmitterInheritSetting mInheritSetting;
        u16 mNameIdx;
    } mChildOption;
    u8 mWrapOption, padding[3];
    math::VEC3 mWrapScale, mWrapRotate, mWrapTranslate;
};

bool calcPostField(Particle *pp, PostFieldInfo::AnimatableParams &params,
                   PostFieldInfo *resource, EmitterResource *childResource,
                   const math::MTX34 &emitterToGlobal,
                   const math::MTX34 &globalToLocal,
                   const math::VEC3 &prevPosition, math::VEC3 addPosition,
                   math::VEC3 *nextVelocity, bool *requestKilling);

} // namespace ef
} // namespace nw4r
#endif
