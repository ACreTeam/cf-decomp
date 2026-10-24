#ifndef NW4R_EF_FIELD_H
#define NW4R_EF_FIELD_H
#include <nw4r/types_nw4r.h>
#include <nw4r/math.h>

namespace nw4r {
namespace ef {

struct InfoFieldSpeed {
    f32 speed;
};
struct InfoFieldGravity {
    f32 power;
    math::_VEC3 rotate;
};
struct InfoFieldRandom {
    f32 power, diffusion;
    u16 interval;
    u8 padding[2];
};
struct InfoFieldSpin {
    f32 speed;
    math::_VEC3 rotate;
};
struct InfoFieldMagnet {
    f32 power;
    math::_VEC3 translate;
};
struct InfoFieldNewton {
    f32 power, refDistance;
    math::_VEC3 translate;
};
struct InfoFieldVortex {
    f32 innerSpeed, outerSpeed, distance;
    math::_VEC3 rotate;
};
struct InfoFieldTail {
    f32 power;
};

struct InfoField {
    u8 space, addTarget, padding, option;
    union {
        f32 animParam;
        InfoFieldSpeed speed;
        InfoFieldGravity gravity;
        InfoFieldRandom random;
        InfoFieldSpin spin;
        InfoFieldMagnet magnet;
        InfoFieldNewton newton;
        InfoFieldVortex vortex;
        InfoFieldTail tail;
    } u;
};

} // namespace ef
} // namespace nw4r
#endif
