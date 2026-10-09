// The drum: the field rolled onto a cylinder around the x axis (namespace dWorld).
// .text 8008278C..80082F24, .ctors, .data 804DE6B8..804DE6D8, .bss 80583A08..80583A60,
// .sdata 80749E20, .sbss 8074E290..8074E2A0, .sdata2 80750540..80750568.
// See include/game/game/d_world.hpp and notes/d_drum.txt.
#include <game/game/d_world.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <lib/egg/math/eggMath.h>
#include <cmath>

// Debug tuning of the drum (RTTI "dDrum_hostIO_c"): the far edge parameters copied by
// dWorld::edge_c.
class dDrum_hostIO_c {
public:
    dDrum_hostIO_c() {
        mEdge.x = 420.0f;
        mEdge.y = 1.0f / 6.0f;
        mEdge.z = 1.0f / 15.0f;
        _10 = 0;
    }

    virtual ~dDrum_hostIO_c() {}

    /* 0x04 */ mVec3_c mEdge;
    /* 0x10 */ u8 _10;
}; // size 0x14

namespace dWorld {

static edge_c l_edge;
static dDrum_hostIO_c l_hostIO;

static bool l_curve;
static mAng l_angle = mAng::fromDegree(50.0f);
static const f32 l_length = lbl_80750520;
static const f32 l_radius = 360.0f * l_length / 314.15927f;

BOOL isCurve() {
    return l_curve;
}

void setCurve(bool curve) {
    l_curve = curve;
}

void clearCurve() {
    l_curve = 0;
}

mAng curvePositionSimple(mVec3_c *result, const mVec3_c *position) {
    if (l_curve) {
        f32 r = position->y + l_radius;
        mAng angle = mAng::fromDegree(50.0f * (position->z / l_length));
        result->x = position->x;
        result->y = r * angle.cos();
        result->z = r * angle.sin();
        return angle;
    }
    *result = *position;
    return mAng(0);
}

void uncurvePosition(mVec3_c *result, const mVec3_c *position) {
    if (l_curve) {
        mAng angle = mAng::fromRadian(EGG::Mathf::atan2(position->z, position->y));
        result->x = position->x;
        result->y = EGG::Mathf::sqrt(cM::square(position->y) + cM::square(position->z)) - l_radius;
        result->z = angleToZ(angle);
    } else {
        *result = *position;
    }
}

f32 angleToZ(mAng angle) {
    f32 deg = angle.degree();
    if (deg < 0.0f) {
        deg += 360.0f;
    }
    return l_length * (deg / 50.0f);
}

mAng getCurveAngle(const mVec3_c *position) {
    return mAng::fromDegree(50.0f * (position->z / l_length));
}

static inline f32 edgeOffset(f32 dist, f32 rate) {
    return dist * rate;
}

mAng curvePosition(mVec3_c *result, const mVec3_c *position) {
    if (l_curve) {
        f32 r = position->y + l_radius;
        f32 margin = l_edge.mParam.x;
        f32 shift = 0.0f;
        if (position->z <= l_edge.mPos.z - margin) {
            f32 dist = std::fabs(position->z - (l_edge.mPos.z - margin));
            r -= edgeOffset(dist, l_edge.mParam.y);
            shift = edgeOffset(dist, l_edge.mParam.z);
        }
        mAng angle = mAng::fromDegree(50.0f * ((position->z - shift) / l_length));
        result->x = position->x;
        result->y = r * angle.cos();
        result->z = r * angle.sin();
        return angle;
    }
    *result = *position;
    return mAng(0);
}

void toFieldPosition(mVec3_c *result, const mVec3_c *position) {
    uncurvePosition(result, position);
}

mAng getAngle() {
    return l_angle;
}

f32 getRadius() {
    return l_radius;
}

BOOL getHorizonPosition(mVec3_c *result, const mVec3_c *position) {
    if (lbl_8074E9B0 != NULL) {
        mVec3_c pos;
        if (position == NULL) {
            dPlayerActor_c *player = fn_800FBC7C(4);
            if (player == NULL) {
                return FALSE;
            }
            pos = player->mPos;
        } else {
            pos = *position;
        }
        if (result != NULL) {
            mVec3_c eye;
            curvePositionSimple(&eye, &lbl_80624004);
            pos.z = angleToZ(getHorizonAngle(&eye, &pos));
            *result = pos;
            return TRUE;
        }
    }
    return FALSE;
}

mAng getPlayerHorizonAngle(bool *ok) {
    bool dummy;
    if (ok == NULL) {
        ok = &dummy;
    }
    dCamera_c *camera = lbl_8074E9B0;
    dPlayerActor_c *player = fn_800FBC7C(4);
    if (camera != NULL && player != NULL) {
        mVec3_c eye;
        curvePositionSimple(&eye, &lbl_80624004);
        *ok = true;
        return getHorizonAngle(&eye, &player->mPos);
    }
    *ok = false;
    return mAng(0);
}

mAng getHorizonAngle(const mVec3_c *eye, const mVec3_c *position) {
    f32 r;
    if (position != NULL) {
        r = l_radius + dBGCF::getUnitY(position);
    } else {
        r = l_radius;
    }
    if (l_curve) {
        f32 sq = cM::square(eye->y) + cM::square(eye->z) - cM::square(r);
        if (sq < 0.0f) {
            sq = 0.0f;
        }
        f32 tangent = EGG::Mathf::sqrt(sq);
        s16 angle = cM::atan2s(eye->z, eye->y);
        return mAng(angle - cM::atan2s(tangent, r));
    }
    return mAng(0);
}

edge_c *getEdge() {
    return &l_edge;
}

edge_c::edge_c() : mPos(0.0f, 0.0f, 0.0f) {
    mParam = l_hostIO.mEdge;
}

edge_c::edge_c(const mVec3_c &pos) {
    mPos = pos;
    mParam = l_hostIO.mEdge;
}

} // namespace dWorld
