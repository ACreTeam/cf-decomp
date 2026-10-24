#include <nw4r/ef.h>
#include <nw4r/math.h>
#include <nw4r/ef/ef_postfield.h>

namespace nw4r {
namespace ef {

enum CollisionStatus {
    COLLISION_STATUS_NONE = 0,
    COLLISION_STATUS_BORDER = 1,
    COLLISION_STATUS_INNER = 2,
    COLLISION_STATUS_OUTER = 3,
    COLLISION_STATUS_PLANEOUTER = 4,

    COLLISION_CROSS = 128,
    COLLISION_STATUS_BORDER_CROSS = COLLISION_STATUS_BORDER | COLLISION_CROSS,
    COLLISION_STATUS_INNER_CROSS = COLLISION_STATUS_INNER | COLLISION_CROSS,
    COLLISION_STATUS_OUTER_CROSS = COLLISION_STATUS_OUTER | COLLISION_CROSS,
};

static const f32 COLLISION_EPSILON = 1.1920929e-7f;

static CollisionStatus GetTypePositionSphere(const math::VEC3& origPos,
                                             u8 option) {
    math::VEC3 pos = origPos;

    if (pos.y < -1.0f || pos.y > 1.0f) {
        return COLLISION_STATUS_OUTER;
    }

    if ((option & 1) && pos.y < 0.0f) {
        return COLLISION_STATUS_OUTER;
    }

    if ((option & 2) && pos.y > 0.0f) {
        return COLLISION_STATUS_OUTER;
    }

    if (pos.x < -1.0f || pos.x > 1.0f) {
        return COLLISION_STATUS_OUTER;
    }

    if (pos.z < -1.0f || pos.z > 1.0f) {
        return COLLISION_STATUS_OUTER;
    }

    f32 lensq = math::VEC3LenSq(&pos);

    if (lensq > 1.0f) {
        return COLLISION_STATUS_OUTER;
    }

    if (lensq == 1.0f) {
        return COLLISION_STATUS_BORDER;
    }

    if (option != 0 && pos.y == 0.0f) {
        return COLLISION_STATUS_BORDER;
    }

    return COLLISION_STATUS_INNER;
}

static CollisionStatus GetTypePositionCube(const math::VEC3& origPos,
                                           u8 option) {
#pragma unused(option)

    math::VEC3 pos = origPos;
    bool surface = false;

    if (pos.x < -1.0f || pos.x > 1.0f) {
        return COLLISION_STATUS_OUTER;
    }

    if (pos.x == 1.0f || pos.x == -1.0f) {
        surface = true;
    }

    if (pos.y < -1.0f || pos.y > 1.0f) {
        return COLLISION_STATUS_OUTER;
    }

    if (pos.y == 1.0f || pos.y == -1.0f) {
        surface = true;
    }

    if (pos.z < -1.0f || pos.z > 1.0f) {
        return COLLISION_STATUS_OUTER;
    }

    if (surface || pos.z == 1.0f || pos.z == -1.0f) {
        return COLLISION_STATUS_BORDER;
    }

    return COLLISION_STATUS_INNER;
}

static CollisionStatus GetTypePositionCylinder(const math::VEC3& origPos,
                                               u8 option) {
#pragma unused(option)

    math::VEC3 pos = origPos;

    if (pos.y < -1.0f || pos.y > 1.0f) {
        return COLLISION_STATUS_OUTER;
    }

    if (pos.x < -1.0f || pos.x > 1.0f) {
        return COLLISION_STATUS_OUTER;
    }

    if (pos.z < -1.0f || pos.z > 1.0f) {
        return COLLISION_STATUS_OUTER;
    }

    f32 lensq = pos.x * pos.x + pos.z * pos.z;

    if (lensq > 1.0f) {
        return COLLISION_STATUS_OUTER;
    }

    if (lensq == 1.0f) {
        return COLLISION_STATUS_BORDER;
    }

    if (pos.y == -1.0f || pos.y == 1.0f) {
        return COLLISION_STATUS_BORDER;
    }

    return COLLISION_STATUS_INNER;
}

static CollisionStatus GetTypePositionCircle(const math::VEC3& origPos,
                                             u8 option) {
    math::VEC3 pos;

    switch (option) {
    case 0: {
        pos = origPos;
        break;
    }

    case 1: {
        pos.x = origPos.x;
        pos.y = origPos.z;
        pos.z = -origPos.y;
        break;
    }

    case 2: {
        pos.x = -origPos.y;
        pos.y = origPos.x;
        pos.z = origPos.z;
        break;
    }
    }

    if (pos.y > 0.0f) {
        return COLLISION_STATUS_OUTER;
    }

    if (pos.y < 0.0f) {
        return COLLISION_STATUS_OUTER;
    }

    if (pos.x < -1.0f || pos.x > 1.0f) {
        return COLLISION_STATUS_PLANEOUTER;
    }

    if (pos.z < -1.0f || pos.z > 1.0f) {
        return COLLISION_STATUS_PLANEOUTER;
    }

    f32 lensq = pos.x * pos.x + pos.z * pos.z;

    if (lensq > 1.0f) {
        return COLLISION_STATUS_PLANEOUTER;
    }

    return COLLISION_STATUS_BORDER;
}

static CollisionStatus GetTypePositionRectangle(const math::VEC3& origPos,
                                                u8 option) {
    math::VEC3 pos;

    switch (option) {
    case 0: {
        pos = origPos;
        break;
    }

    case 1: {
        pos.x = origPos.x;
        pos.y = origPos.z;
        pos.z = -origPos.y;
        break;
    }

    case 2: {
        pos.x = -origPos.y;
        pos.y = origPos.x;
        pos.z = origPos.z;
        break;
    }
    }

    if (pos.y > 0.0f) {
        return COLLISION_STATUS_OUTER;
    }

    if (pos.y < 0.0f) {
        return COLLISION_STATUS_OUTER;
    }

    if (pos.x < -1.0f || pos.x > 1.0f) {
        return COLLISION_STATUS_PLANEOUTER;
    }

    if (pos.z < -1.0f || pos.z > 1.0f) {
        return COLLISION_STATUS_PLANEOUTER;
    }

    return COLLISION_STATUS_BORDER;
}

static CollisionStatus GetTypePositionPlane(const math::VEC3& origPos,
                                            u8 option) {
    math::VEC3 pos;

    switch (option) {
    case 0: {
        pos = origPos;
        break;
    }

    case 1: {
        pos.x = origPos.x;
        pos.y = origPos.z;
        pos.z = -origPos.y;
        break;
    }

    case 2: {
        pos.x = -origPos.y;
        pos.y = origPos.x;
        pos.z = origPos.z;
        break;
    }
    }

    if (pos.y > 0.0f) {
        return COLLISION_STATUS_INNER;
    }

    if (pos.y < 0.0f) {
        return COLLISION_STATUS_OUTER;
    }

    return COLLISION_STATUS_BORDER;
}

#define CROSS_CHECK_AXIS(A, INBOUNDS)                                          \
    if (math::FAbs(prevPosInForm.A) < COLLISION_EPSILON &&                     \
        math::FAbs(nextPosInForm.A) < COLLISION_EPSILON) {                     \
        return 2.0f;                                                           \
    }                                                                          \
                                                                               \
    if (prevPosInForm.A > 0.0f && nextPosInForm.A < COLLISION_EPSILON) {       \
        math::VEC3 vec;                                                        \
        t = -prevPosInForm.A / currentMoveInForm->A;                           \
        math::VEC3Lerp(crossPosInForm, &prevPosInForm, &nextPosInForm, t);     \
                                                                               \
        if (INBOUNDS) {                                                        \
            if (postfieldResource->mCollisionOption & 2) {                     \
                velocityInForm->A *= -1.0f;                                    \
                currentMoveInForm->A *= -1.0f;                                 \
                crossPosInForm->A += COLLISION_EPSILON;                        \
            }                                                                  \
                                                                               \
            math::VEC3Transform(crossPos, &mtxFormToLocal, crossPosInForm);    \
            return t;                                                          \
        } else {                                                               \
            return 2.0f;                                                       \
        }                                                                      \
    }                                                                          \
                                                                               \
    if (prevPosInForm.A < 0.0f && nextPosInForm.A > COLLISION_EPSILON) {       \
        math::VEC3 vec;                                                        \
        t = -prevPosInForm.A / currentMoveInForm->A;                           \
        math::VEC3Lerp(crossPosInForm, &prevPosInForm, &nextPosInForm, t);     \
                                                                               \
        if (INBOUNDS) {                                                        \
            if (postfieldResource->mCollisionOption & 2) {                     \
                velocityInForm->A *= -1.0f;                                    \
                currentMoveInForm->A *= -1.0f;                                 \
                crossPosInForm->A -= COLLISION_EPSILON;                        \
            }                                                                  \
                                                                               \
            math::VEC3Transform(crossPos, &mtxFormToLocal, crossPosInForm);    \
            return t;                                                          \
        } else {                                                               \
            return 2.0f;                                                       \
        }                                                                      \
    }                                                                          \
                                                                               \
    return 2.0f;

#define IN_RECTANGLE(B, C)                                                     \
    math::FAbs(crossPosInForm->B) < 1.0f + COLLISION_EPSILON &&                \
        math::FAbs(crossPosInForm->C) < 1.0f + COLLISION_EPSILON

#define IN_CIRCLE(B, C)                                                        \
    crossPosInForm->B * crossPosInForm->B +                                    \
            crossPosInForm->C * crossPosInForm->C <                            \
        (1.0f + COLLISION_EPSILON) * (1.0f + COLLISION_EPSILON)

static f32 crossCheckPlane(const math::MTX34& mtxFormToLocal,
                           const PostFieldInfo* postfieldResource,
                           const math::VEC3& nextPosInForm,
                           const math::VEC3& prevPosInForm, u8 currentStatus,
                           math::VEC3* currentMoveInForm,
                           math::VEC3* velocityInForm,
                           math::VEC3* crossPosInForm, math::VEC3* crossPos) {
#pragma unused(currentStatus)

    f32 t = 2.0f;

    switch (postfieldResource->mCollisionShapeOption) {
    case 0: {
        CROSS_CHECK_AXIS(y, true);
    }

    case 1: {
        CROSS_CHECK_AXIS(z, true);
    }

    case 2: {
        CROSS_CHECK_AXIS(x, true);
    }
    }

    return 2.0f;
}

static f32 crossCheckRectangle(const math::MTX34& mtxFormToLocal,
                               const PostFieldInfo* postfieldResource,
                               const math::VEC3& nextPosInForm,
                               const math::VEC3& prevPosInForm,
                               u8 currentStatus, math::VEC3* currentMoveInForm,
                               math::VEC3* velocityInForm,
                               math::VEC3* crossPosInForm,
                               math::VEC3* crossPos) {
#pragma unused(currentStatus)

    f32 t = 2.0f;

    switch (postfieldResource->mCollisionShapeOption) {
    case 0: {
        CROSS_CHECK_AXIS(y, IN_RECTANGLE(x, z));
    }

    case 1: {
        CROSS_CHECK_AXIS(z, IN_RECTANGLE(x, y));
    }

    case 2: {
        CROSS_CHECK_AXIS(x, IN_RECTANGLE(y, z));
    }
    }

    return 2.0f;
}

static f32 crossCheckCircle(const math::MTX34& mtxFormToLocal,
                            const PostFieldInfo* postfieldResource,
                            const math::VEC3& nextPosInForm,
                            const math::VEC3& prevPosInForm, u8 currentStatus,
                            math::VEC3* currentMoveInForm,
                            math::VEC3* velocityInForm,
                            math::VEC3* crossPosInForm, math::VEC3* crossPos) {
#pragma unused(currentStatus)

    f32 t = 2.0f;

    switch (postfieldResource->mCollisionShapeOption) {
    case 0: {
        CROSS_CHECK_AXIS(y, IN_CIRCLE(x, z));
    }

    case 1: {
        CROSS_CHECK_AXIS(z, IN_CIRCLE(x, y));
    }

    case 2: {
        CROSS_CHECK_AXIS(x, IN_CIRCLE(y, z));
    }
    }

    return 2.0f;
}

#undef CROSS_CHECK_AXIS
#undef IN_RECTANGLE
#undef IN_CIRCLE

#define CROSS_CHECK_CAP(PY, CMP_OUT, CMP_IN, EPS_OP, RET)                      \
    if (math::FAbs(prevPosInForm.y - (PY)) < COLLISION_EPSILON &&              \
        math::FAbs(nextPosInForm.y - (PY)) < COLLISION_EPSILON) {              \
        ;                                                                      \
    } else if (prevPosInForm.y > (PY) &&                                       \
               nextPosInForm.y < (PY) + COLLISION_EPSILON) {                   \
        math::VEC3 vec;                                                        \
        a3 = ((PY) - prevPosInForm.y) / currentMoveInForm->y;                  \
        math::VEC3Lerp(&vec, &prevPosInForm, &nextPosInForm, a3);              \
                                                                               \
        if (a3 < t && vec.x * vec.x + vec.z * vec.z <                          \
                          (1.0f + COLLISION_EPSILON) *                         \
                              (1.0f + COLLISION_EPSILON)) {                    \
            *crossPosInForm = vec;                                             \
                                                                               \
            if (postfieldResource->mCollisionOption & 2) {                     \
                velocityInForm->y *= -1.0f;                                    \
                currentMoveInForm->y *= -1.0f;                                 \
                crossPosInForm->y += COLLISION_EPSILON;                        \
            }                                                                  \
                                                                               \
            math::VEC3Transform(crossPos, &mtxFormToLocal, crossPosInForm);    \
            return a3;                                                         \
        }                                                                      \
    } else if (prevPosInForm.y < (PY) &&                                       \
               nextPosInForm.y > (PY) + COLLISION_EPSILON) {                   \
        math::VEC3 vec;                                                        \
        a3 = ((PY) - prevPosInForm.y) / currentMoveInForm->y;                  \
        math::VEC3Lerp(&vec, &prevPosInForm, &nextPosInForm, a3);              \
                                                                               \
        if (a3 < t && vec.x * vec.x + vec.z * vec.z <                          \
                          (1.0f + COLLISION_EPSILON) *                         \
                              (1.0f + COLLISION_EPSILON)) {                    \
            *crossPosInForm = vec;                                             \
                                                                               \
            if (postfieldResource->mCollisionOption & 2) {                     \
                velocityInForm->y *= -1.0f;                                    \
                currentMoveInForm->y *= -1.0f;                                 \
                crossPosInForm->y -= COLLISION_EPSILON;                        \
            }                                                                  \
                                                                               \
            math::VEC3Transform(crossPos, &mtxFormToLocal, crossPosInForm);    \
            return a3;                                                         \
        }                                                                      \
    }

static f32 crossCheckSphere(const math::MTX34& mtxFormToLocal,
                            const PostFieldInfo* postfieldResource,
                            const math::VEC3& nextPosInForm,
                            const math::VEC3& prevPosInForm, u8 currentStatus,
                            math::VEC3* currentMoveInForm,
                            math::VEC3* velocityInForm,
                            math::VEC3* crossPosInForm, math::VEC3* crossPos) {
#pragma unused(currentStatus)

    f32 t = 2.0f;

    f32 p2 = math::VEC3LenSq(&prevPosInForm);
    f32 n2 = math::VEC3LenSq(&nextPosInForm);
#pragma unused(n2)
    f32 d2 = math::VEC3LenSq(currentMoveInForm);
    f32 pd = math::VEC3Dot(&prevPosInForm, currentMoveInForm);

    f32 s = 4.0f * pd * pd - 4.0f * d2 * (p2 - 1.0f);
    if (s < 0.0f) {
        return 2.0f;
    }

    s = math::FSqrt(s);

    f32 a1 = (-2.0f * pd - s) / (2.0f * d2);
    f32 a2 = (-2.0f * pd + s) / (2.0f * d2);

    if (a1 > 0.0f && a1 < 1.0f + COLLISION_EPSILON) {
        math::VEC3Lerp(crossPosInForm, &prevPosInForm, &nextPosInForm, a1);

        if ((postfieldResource->mCollisionShapeOption == 1 &&
             crossPosInForm->y < -COLLISION_EPSILON) ||
            (postfieldResource->mCollisionShapeOption == 2 &&
             crossPosInForm->y > COLLISION_EPSILON)) {
            ;
        } else {
            t = a1;
        }
    }

    if (a2 > 0.0f && a2 < 1.0f + COLLISION_EPSILON && a2 < t) {
        math::VEC3 vec;
        math::VEC3Lerp(&vec, &prevPosInForm, &nextPosInForm, a2);

        if ((postfieldResource->mCollisionShapeOption == 1 &&
             vec.y < -COLLISION_EPSILON) ||
            (postfieldResource->mCollisionShapeOption == 2 &&
             vec.y > COLLISION_EPSILON)) {
            ;
        } else {
            *crossPosInForm = vec;
            t = a2;
        }
    }

    f32 a3 = 2.0f;

    if (postfieldResource->mCollisionShapeOption == 1 ||
        postfieldResource->mCollisionShapeOption == 2) {
        CROSS_CHECK_CAP(0.0f, >, <, +=, a3);
    }

    if (t < 2.0f) {
        if (postfieldResource->mCollisionOption & 2) {
            f32 cx = math::FSqrt(1.0f - crossPosInForm->z * crossPosInForm->z);
            f32 sz;
            f32 cz;

            if (cx > 1.1920929e-7f) {
                cz = crossPosInForm->y / cx;
                sz = -crossPosInForm->x / cx;
            } else {
                cz = 1.0f;
                sz = 0.0f;
            }

            math::MTX34 mtx;
            mtx._00 = cz;
            mtx._01 = crossPosInForm->x;
            mtx._02 = crossPosInForm->z * sz;
            mtx._03 = 0.0f;
            mtx._10 = sz;
            mtx._11 = crossPosInForm->y;
            mtx._12 = -cz * crossPosInForm->z;
            mtx._13 = 0.0f;
            mtx._20 = 0.0f;
            mtx._21 = crossPosInForm->z;
            mtx._22 = cx;
            mtx._23 = 0.0f;
            math::MTX34Inv(&mtx, &mtx);

            math::VEC3 vec;
            math::VEC3 v1;

            math::VEC3Transform(&vec, &mtx, currentMoveInForm);
            math::VEC3Scale(&v1, crossPosInForm, vec.y);
            *currentMoveInForm -= v1;
            *currentMoveInForm -= v1;

            math::VEC3Transform(&vec, &mtx, velocityInForm);
            math::VEC3Scale(&v1, crossPosInForm, vec.y);
            *velocityInForm -= v1;
            *velocityInForm -= v1;

            if (p2 > 1.0f) {
                math::VEC3Scale(crossPosInForm, crossPosInForm, 1.00001f);
            } else {
                math::VEC3Scale(crossPosInForm, crossPosInForm, 0.99999f);
            }
        }

        math::VEC3Transform(crossPos, &mtxFormToLocal, crossPosInForm);
    }

    return t;
}

static f32 crossCheckCylinder(const math::MTX34& mtxFormToLocal,
                              const PostFieldInfo* postfieldResource,
                              const math::VEC3& nextPosInForm,
                              const math::VEC3& prevPosInForm,
                              u8 currentStatus, math::VEC3* currentMoveInForm,
                              math::VEC3* velocityInForm,
                              math::VEC3* crossPosInForm,
                              math::VEC3* crossPos) {
#pragma unused(currentStatus)

    f32 t = 2.0f;

    f32 p2 = prevPosInForm.x * prevPosInForm.x + prevPosInForm.z * prevPosInForm.z;
    f32 n2 = nextPosInForm.x * nextPosInForm.x + nextPosInForm.z * nextPosInForm.z;
#pragma unused(n2)
    f32 d2 = currentMoveInForm->x * currentMoveInForm->x +
             currentMoveInForm->z * currentMoveInForm->z;
    f32 pd = prevPosInForm.x * currentMoveInForm->x +
             prevPosInForm.z * currentMoveInForm->z;

    f32 s = 4.0f * pd * pd - 4.0f * d2 * (p2 - 1.0f);
    if (s < 0.0f) {
        return 2.0f;
    }

    s = math::FSqrt(s);

    f32 a1 = (-2.0f * pd - s) / (2.0f * d2);
    f32 a2 = (-2.0f * pd + s) / (2.0f * d2);

    if (a1 > 0.0f && a1 < 1.0f + COLLISION_EPSILON) {
        math::VEC3Lerp(crossPosInForm, &prevPosInForm, &nextPosInForm, a1);

        if (math::FAbs(crossPosInForm->y) < 1.0f + COLLISION_EPSILON) {
            t = a1;
        }
    }

    if (a2 > 0.0f && a2 < 1.0f + COLLISION_EPSILON && a2 < t) {
        math::VEC3 vec;
        math::VEC3Lerp(&vec, &prevPosInForm, &nextPosInForm, a2);

        if (math::FAbs(vec.y) < 1.0f + COLLISION_EPSILON) {
            *crossPosInForm = vec;
            t = a2;
        }
    }

    f32 a3 = 2.0f;

    {
        f32 py = 1.0f;
        CROSS_CHECK_CAP(py, >, <, +=, a3);
    }

    {
        f32 py = -1.0f;
        CROSS_CHECK_CAP(py, >, <, +=, a3);
    }

    if (t < 2.0f) {
        if (postfieldResource->mCollisionOption & 2) {
            f32 len;
            f32 co;
            math::VEC3 v1;

            len = math::FSqrt(currentMoveInForm->x * currentMoveInForm->x +
                              currentMoveInForm->z * currentMoveInForm->z);
            co = currentMoveInForm->x * crossPosInForm->x +
                 currentMoveInForm->z * crossPosInForm->z;
            v1.x = crossPosInForm->x;
            v1.y = 0.0f;
            v1.z = crossPosInForm->z;
            math::VEC3Normalize(&v1, &v1);
            math::VEC3Scale(&v1, &v1, co);
            *currentMoveInForm -= v1;
            *currentMoveInForm -= v1;

            len = math::FSqrt(velocityInForm->x * velocityInForm->x +
                              velocityInForm->z * velocityInForm->z);
            co = velocityInForm->x * crossPosInForm->x +
                 velocityInForm->z * crossPosInForm->z;
            v1.x = crossPosInForm->x;
            v1.y = 0.0f;
            v1.z = crossPosInForm->z;
            math::VEC3Normalize(&v1, &v1);
            math::VEC3Scale(&v1, &v1, co);
            *velocityInForm -= v1;
            *velocityInForm -= v1;

            if (p2 > 1.0f) {
                math::VEC3Scale(crossPosInForm, crossPosInForm, 1.00001f);
            } else {
                math::VEC3Scale(crossPosInForm, crossPosInForm, 0.99999f);
            }
        }

        math::VEC3Transform(crossPos, &mtxFormToLocal, crossPosInForm);
    }

    return t;
}

#undef CROSS_CHECK_CAP

#define CUBE_FACE(VAL, PLANE, A, B, C, OUT)                                    \
    work = ((PLANE) - prevPosInForm.A) / currentMoveInForm->A;                 \
    if (work > 0.0f && work <= 1.0f + COLLISION_EPSILON) {                     \
        math::VEC3Lerp(&vec, &prevPosInForm, &nextPosInForm, work);            \
        if (math::FAbs(vec.B) <= 1.0f + COLLISION_EPSILON &&                   \
            math::FAbs(vec.C) <= 1.0f + COLLISION_EPSILON) {                   \
            OUT = work;                                                        \
            if (OUT < t) {                                                     \
                t = OUT;                                                       \
            }                                                                  \
        }                                                                      \
    }

static f32 crossCheckCube(const math::MTX34& mtxFormToLocal,
                          const PostFieldInfo* postfieldResource,
                          const math::VEC3& nextPosInForm,
                          const math::VEC3& prevPosInForm, u8 currentStatus,
                          math::VEC3* currentMoveInForm,
                          math::VEC3* velocityInForm,
                          math::VEC3* crossPosInForm, math::VEC3* crossPos) {
    f32 t = 2.0f;

    f32 xp = 2.0f;
    f32 xm = 2.0f;
    f32 yp = 2.0f;
    f32 ym = 2.0f;
    f32 zp = 2.0f;
    f32 zm = 2.0f;

    f32 work;
    math::VEC3 vec;

    CUBE_FACE(xp, 1.0f, x, y, z, xp);
    CUBE_FACE(xm, -1.0f, x, y, z, xm);
    CUBE_FACE(yp, 1.0f, y, x, z, yp);
    CUBE_FACE(ym, -1.0f, y, x, z, ym);
    CUBE_FACE(zp, 1.0f, z, x, y, zp);
    CUBE_FACE(zm, -1.0f, z, x, y, zm);

    if (t == 2.0f) {
        return 2.0f;
    }

    crossPosInForm->x =
        (nextPosInForm.x - prevPosInForm.x) * t + prevPosInForm.x;
    crossPosInForm->y =
        (nextPosInForm.y - prevPosInForm.y) * t + prevPosInForm.y;
    crossPosInForm->z =
        (nextPosInForm.z - prevPosInForm.z) * t + prevPosInForm.z;

    if ((postfieldResource->mCollisionOption & 4) &&
        (currentStatus == COLLISION_STATUS_BORDER ||
         currentStatus == COLLISION_STATUS_INNER)) {
        if (math::FAbs(xp - t) <= COLLISION_EPSILON) {
            crossPosInForm->x = -1.0f;
        }
        if (math::FAbs(xm - t) <= COLLISION_EPSILON) {
            crossPosInForm->x = 1.0f;
        }
        if (math::FAbs(yp - t) <= COLLISION_EPSILON) {
            crossPosInForm->y = -1.0f;
        }
        if (math::FAbs(ym - t) <= COLLISION_EPSILON) {
            crossPosInForm->y = 1.0f;
        }
        if (math::FAbs(zp - t) <= COLLISION_EPSILON) {
            crossPosInForm->z = -1.0f;
        }
        if (math::FAbs(zm - t) <= COLLISION_EPSILON) {
            crossPosInForm->z = 1.0f;
        }
    }

    math::VEC3Transform(crossPos, &mtxFormToLocal, crossPosInForm);

    if (postfieldResource->mCollisionOption & 2) {
        if (math::FAbs(xp - t) <= COLLISION_EPSILON ||
            math::FAbs(xm - t) <= COLLISION_EPSILON) {
            velocityInForm->x *= -1.0f;
            currentMoveInForm->x *= -1.0f;
        }
        if (math::FAbs(yp - t) <= COLLISION_EPSILON ||
            math::FAbs(ym - t) <= COLLISION_EPSILON) {
            velocityInForm->y *= -1.0f;
            currentMoveInForm->y *= -1.0f;
        }
        if (math::FAbs(zp - t) <= COLLISION_EPSILON ||
            math::FAbs(zm - t) <= COLLISION_EPSILON) {
            velocityInForm->z *= -1.0f;
            currentMoveInForm->z *= -1.0f;
        }

        if (math::FAbs(xp - t) <= COLLISION_EPSILON) {
            crossPosInForm->x = 1.0f;
        }
        if (math::FAbs(xm - t) <= COLLISION_EPSILON) {
            crossPosInForm->x = -1.0f;
        }
        if (math::FAbs(yp - t) <= COLLISION_EPSILON) {
            crossPosInForm->y = 1.0f;
        }
        if (math::FAbs(ym - t) <= COLLISION_EPSILON) {
            crossPosInForm->y = -1.0f;
        }
        if (math::FAbs(zp - t) <= COLLISION_EPSILON) {
            crossPosInForm->z = 1.0f;
        }
        if (math::FAbs(zm - t) <= COLLISION_EPSILON) {
            crossPosInForm->z = -1.0f;
        }
    }

    return t;
}

#undef CUBE_FACE

static CollisionStatus GetTypePosition(u8 type, const math::VEC3& pos,
                                       u8 option) {
    switch (type) {
    case 0: {
        return GetTypePositionPlane(pos, option);
    }

    case 1: {
        return GetTypePositionRectangle(pos, option);
    }

    case 2: {
        return GetTypePositionCircle(pos, option);
    }

    case 3: {
        return GetTypePositionCube(pos, option);
    }

    case 4: {
        return GetTypePositionSphere(pos, option);
    }

    case 5: {
        return GetTypePositionCylinder(pos, option);
    }
    }

    return COLLISION_STATUS_BORDER;
}

static bool crossCheck(const math::MTX34& mtxFormToLocal,
                       const math::MTX34& mtxFormToLocalNoTrans,
                       const PostFieldInfo* postfieldResource,
                       EmitterResource* postfieldChildRes, Particle* pp,
                       u8 currentStatus, const math::VEC3& prevPosInForm,
                       const math::VEC3& nextPosInForm,
                       math::VEC3 velocityInForm, bool* requestKilling,
                       math::VEC3* crossPosInForm, math::VEC3* crossPos,
                       math::VEC3* resultPos, math::VEC3* resultVelocity) {
    u8 nextStatus =
        GetTypePosition(postfieldResource->mCollisionShapeType, nextPosInForm,
                        postfieldResource->mCollisionShapeOption);
#pragma unused(nextStatus)

    math::VEC3 currentMoveInForm;
    math::VEC3Sub(&currentMoveInForm, &nextPosInForm, &prevPosInForm);

    f32 t = 2.0f;

    switch (postfieldResource->mCollisionShapeType) {
    case 0: {
        t = crossCheckPlane(mtxFormToLocal, postfieldResource, nextPosInForm,
                            prevPosInForm, currentStatus, &currentMoveInForm,
                            &velocityInForm, crossPosInForm, crossPos);
        break;
    }

    case 1: {
        t = crossCheckRectangle(mtxFormToLocal, postfieldResource,
                                nextPosInForm, prevPosInForm, currentStatus,
                                &currentMoveInForm, &velocityInForm,
                                crossPosInForm, crossPos);
        break;
    }

    case 2: {
        t = crossCheckCircle(mtxFormToLocal, postfieldResource, nextPosInForm,
                             prevPosInForm, currentStatus, &currentMoveInForm,
                             &velocityInForm, crossPosInForm, crossPos);
        break;
    }

    case 3: {
        t = crossCheckCube(mtxFormToLocal, postfieldResource, nextPosInForm,
                           prevPosInForm, currentStatus, &currentMoveInForm,
                           &velocityInForm, crossPosInForm, crossPos);
        break;
    }

    case 4: {
        t = crossCheckSphere(mtxFormToLocal, postfieldResource, nextPosInForm,
                             prevPosInForm, currentStatus, &currentMoveInForm,
                             &velocityInForm, crossPosInForm, crossPos);
        break;
    }

    case 5: {
        t = crossCheckCylinder(mtxFormToLocal, postfieldResource,
                               nextPosInForm, prevPosInForm, currentStatus,
                               &currentMoveInForm, &velocityInForm,
                               crossPosInForm, crossPos);
        break;
    }
    }

    if (t > 1.5f) {
        return false;
    }

    if ((postfieldResource->mCollisionOption & 2) ||
        (postfieldResource->mCollisionOption & 8) ||
        ((postfieldResource->mCollisionOption & 4) &&
         (currentStatus == COLLISION_STATUS_BORDER ||
          currentStatus == COLLISION_STATUS_INNER))) {
        math::VEC3Transform(resultVelocity, &mtxFormToLocalNoTrans,
                            &velocityInForm);
        resultVelocity->x *= postfieldResource->mSpeedFactor.x;
        resultVelocity->y *= postfieldResource->mSpeedFactor.y;
        resultVelocity->z *= postfieldResource->mSpeedFactor.z;

        math::VEC3 vec;
        math::VEC3Scale(&vec, &currentMoveInForm, 1.0f - t);
        math::VEC3Transform(&vec, &mtxFormToLocalNoTrans, &vec);
        vec.x *= postfieldResource->mSpeedFactor.x;
        vec.y *= postfieldResource->mSpeedFactor.y;
        vec.z *= postfieldResource->mSpeedFactor.z;

        math::VEC3Add(resultPos, crossPos, &vec);
    }

    if (postfieldResource->mCollisionOption & 0x40) {
        *requestKilling = true;
        *resultPos = *crossPos;
    }

    if ((postfieldResource->mCollisionOption & 0x10) &&
        postfieldChildRes != NULL) {
        pp->mParticleManager->mManagerEM->mManagerEF->mManagerES->mCreationQueue
            .AddParticleCreation(&postfieldResource->mChildOption.mInheritSetting,
                                 pp, postfieldChildRes, pp->mCalcRemain,
                                 crossPos, NULL);
    }

    if ((postfieldResource->mCollisionOption & 0x20) &&
        postfieldChildRes != NULL) {
        pp->mParticleManager->mManagerEM->mManagerEF->mManagerES->mCreationQueue
            .AddEmitterCreation(&postfieldResource->mChildOption.mInheritSetting,
                                pp, postfieldChildRes, pp->mCalcRemain,
                                crossPos, NULL);
    }

    return true;
}

#define LIMIT_SPEED(VEL)                                                       \
    switch (postfieldResource->mControlSpeedType) {                            \
    case 1: {                                                                  \
        if (math::VEC3LenSq(VEL) > postfieldResource->mReferenceSpeed *        \
                                       postfieldResource->mReferenceSpeed) {   \
            Normalize(VEL, VEL);                                               \
            math::VEC3Scale(VEL, VEL, postfieldResource->mReferenceSpeed);     \
        }                                                                      \
        break;                                                                 \
    }                                                                          \
                                                                               \
    case 2: {                                                                  \
        if (math::VEC3LenSq(VEL) > postfieldResource->mReferenceSpeed *        \
                                       postfieldResource->mReferenceSpeed) {   \
            *requestKilling = true;                                            \
        }                                                                      \
        break;                                                                 \
    }                                                                          \
    }

#define CREATE_CHILDREN()                                                      \
    if ((postfieldResource->mCollisionOption & 0x10) &&                        \
        postfieldChildRes != NULL) {                                           \
        pp->mParticleManager->mManagerEM->mManagerEF->mManagerES               \
            ->mCreationQueue.AddParticleCreation(                              \
                &postfieldResource->mChildOption.mInheritSetting, pp,          \
                postfieldChildRes, pp->mCalcRemain, NULL, NULL);               \
    }                                                                          \
                                                                               \
    if ((postfieldResource->mCollisionOption & 0x20) &&                        \
        postfieldChildRes != NULL) {                                           \
        pp->mParticleManager->mManagerEM->mManagerEF->mManagerES               \
            ->mCreationQueue.AddEmitterCreation(                               \
                &postfieldResource->mChildOption.mInheritSetting, pp,          \
                postfieldChildRes, pp->mCalcRemain, NULL, NULL);               \
    }

bool calcPostField(Particle* pp,
                   PostFieldInfo::AnimatableParams& postfieldAnimParams,
                   PostFieldInfo* postfieldResource,
                   EmitterResource* postfieldChildRes,
                   const math::MTX34& mtxEmitterToGlobal,
                   const math::MTX34& mtxGlobalToLocal,
                   const math::VEC3& prevPosition, math::VEC3 addPosition,
                   math::VEC3* nextVelocity, bool* requestKilling) {
    math::MTX34 mtxFormToLocal;

    {
        math::MTX34Identity(&mtxFormToLocal);
        MTX34ScaleNonZero(&mtxFormToLocal, &postfieldAnimParams.mSize,
                          &mtxFormToLocal);

        math::MTX34 rot;
        PSMTX34RotXYZRad(&rot, postfieldAnimParams.mRotate.x,
                         postfieldAnimParams.mRotate.y,
                         postfieldAnimParams.mRotate.z);
        math::MTX34Mult(&mtxFormToLocal, &rot, &mtxFormToLocal);
        math::MTX34Trans(&mtxFormToLocal, &postfieldAnimParams.mTranslate,
                         &mtxFormToLocal);

        if (postfieldResource->mCollisionOption & 1) {
            math::VEC3 vec(mtxEmitterToGlobal._03, mtxEmitterToGlobal._13,
                           mtxEmitterToGlobal._23);
            math::MTX34Trans(&mtxFormToLocal, &vec, &mtxFormToLocal);
        }

        math::MTX34Mult(&mtxFormToLocal, &mtxGlobalToLocal, &mtxFormToLocal);
    }

    math::MTX34 mtxFormToLocalNoTrans(mtxFormToLocal);
    mtxFormToLocalNoTrans._03 = 0.0f;
    mtxFormToLocalNoTrans._13 = 0.0f;
    mtxFormToLocalNoTrans._23 = 0.0f;

    math::MTX34 mtxLocalToForm;
    math::MTX34Inv(&mtxLocalToForm, &mtxFormToLocal);

    math::MTX34 mtxLocalToFormNoTrans(mtxLocalToForm);
    mtxLocalToFormNoTrans._03 = 0.0f;
    mtxLocalToFormNoTrans._13 = 0.0f;
    mtxLocalToFormNoTrans._23 = 0.0f;

    math::VEC3 prevPosInForm;
    math::VEC3Transform(&prevPosInForm, &mtxLocalToForm, &prevPosition);

    u8 currentStatus =
        GetTypePosition(postfieldResource->mCollisionShapeType, prevPosInForm,
                        postfieldResource->mCollisionShapeOption);
    u8 prevStatus = pp->mParameter.mCollisionStatus;
    pp->mParameter.mCollisionStatus = currentStatus;

    {
        f32 nextVelocityLenSq = math::VEC3LenSq(nextVelocity);

        switch (postfieldResource->mControlSpeedType) {
        case 1: {
            if (nextVelocityLenSq > postfieldResource->mReferenceSpeed *
                                        postfieldResource->mReferenceSpeed) {
                Normalize(nextVelocity, nextVelocity);
                math::VEC3Scale(nextVelocity, nextVelocity,
                                postfieldResource->mReferenceSpeed);
            }
            break;
        }

        case 2: {
            if (nextVelocityLenSq > postfieldResource->mReferenceSpeed *
                                        postfieldResource->mReferenceSpeed) {
                *requestKilling = true;
            }
            break;
        }
        }

        if (*requestKilling) {
            return true;
        }
    }

    if (pp->mTick < postfieldResource->mStartFrame) {
        return true;
    }

    if (postfieldResource->mCollisionOption == 0) {
        return true;
    }

    if (postfieldResource->mCollisionType == 1 ||
        postfieldResource->mCollisionType == 2) {
        if (postfieldResource->mCollisionType == 1 &&
            currentStatus != COLLISION_STATUS_INNER) {
            return true;
        }

        if (postfieldResource->mCollisionType == 2 &&
            currentStatus != COLLISION_STATUS_OUTER) {
            return true;
        }

        if (postfieldResource->mCollisionOption & 8) {
            addPosition.x *= postfieldResource->mSpeedFactor.x;
            addPosition.y *= postfieldResource->mSpeedFactor.y;
            addPosition.z *= postfieldResource->mSpeedFactor.z;

            nextVelocity->x *= postfieldResource->mSpeedFactor.x;
            nextVelocity->y *= postfieldResource->mSpeedFactor.y;
            nextVelocity->z *= postfieldResource->mSpeedFactor.z;

            LIMIT_SPEED(nextVelocity);
        }

        if (postfieldResource->mCollisionOption & 0x40) {
            *requestKilling = true;
        }

        CREATE_CHILDREN();
        return true;
    }

    math::VEC3 nextPosInForm;
    nextPosInForm.x = prevPosition.x + (addPosition.x + nextVelocity->x) *
                                           pp->mParameter.mMomentum;
    nextPosInForm.y = prevPosition.y + (addPosition.y + nextVelocity->y) *
                                           pp->mParameter.mMomentum;
    nextPosInForm.z = prevPosition.z + (addPosition.z + nextVelocity->z) *
                                           pp->mParameter.mMomentum;
    math::VEC3Transform(&nextPosInForm, &mtxLocalToForm, &nextPosInForm);

    math::VEC3 velocityInForm;
    math::VEC3Transform(&velocityInForm, &mtxLocalToFormNoTrans, nextVelocity);

    for (int watchdog = 0; watchdog < 5; watchdog++) {
        math::VEC3 crossPosInForm;
        math::VEC3 crossPos;
        math::VEC3 resultPos;
        math::VEC3 resultVelocity;
        bool isCross;

        if (math::VEC3DistSq(&prevPosInForm, &nextPosInForm) <
            COLLISION_EPSILON * COLLISION_EPSILON) {
            isCross = false;
        } else {
            isCross = crossCheck(mtxFormToLocal, mtxFormToLocalNoTrans,
                                 postfieldResource, postfieldChildRes, pp,
                                 currentStatus, prevPosInForm, nextPosInForm,
                                 velocityInForm, requestKilling,
                                 &crossPosInForm, &crossPos, &resultPos,
                                 &resultVelocity);
        }

        if (isCross) {
            pp->mParameter.mPosition = resultPos;
            pp->mParameter.mVelocity = resultVelocity;
            pp->mParameter.mCollisionStatus |= COLLISION_CROSS;

            if (*requestKilling) {
                return false;
            }

            currentStatus = COLLISION_STATUS_BORDER;
            prevPosInForm = crossPosInForm;
            math::VEC3Transform(&nextPosInForm, &mtxLocalToForm, &resultPos);
            math::VEC3Transform(&velocityInForm, &mtxLocalToFormNoTrans,
                                &resultVelocity);
        } else {
            if (watchdog == 0) {
                if (pp->mTick > postfieldResource->mStartFrame &&
                    ((prevStatus == COLLISION_STATUS_INNER &&
                      (currentStatus == COLLISION_STATUS_OUTER ||
                       currentStatus == COLLISION_STATUS_BORDER)) ||
                     (prevStatus == COLLISION_STATUS_OUTER &&
                      (currentStatus == COLLISION_STATUS_INNER ||
                       currentStatus == COLLISION_STATUS_BORDER)))) {
                    if (postfieldResource->mCollisionOption & 0x40) {
                        *requestKilling = true;
                    }

                    CREATE_CHILDREN();
                }

                return true;
            }

            LIMIT_SPEED(&pp->mParameter.mVelocity);
            return false;
        }
    }

    *requestKilling = true;
    return false;
}

#undef LIMIT_SPEED
#undef CREATE_CHILDREN

} // namespace ef
} // namespace nw4r
