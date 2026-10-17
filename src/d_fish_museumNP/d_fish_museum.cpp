// The museum fish tanks (REL d_fish_museumNP). See include/game/game/d_fish_museum.hpp.
#include <game/game/d_fish_mus_base_move.hpp>
#include <game/game/d_fish_mus_big.hpp>
#include <game/game/d_fish_mus_create.hpp>
#include <game/game/d_fish_mus_cuttlefish.hpp>
#include <game/game/d_fish_mus_eel.hpp>
#include <game/game/d_fish_mus_elegance.hpp>
#include <game/game/d_fish_mus_fast.hpp>
#include <game/game/d_fish_mus_flatfish.hpp>
#include <game/game/d_fish_mus_float.hpp>
#include <game/game/d_fish_mus_frog.hpp>
#include <game/game/d_fish_mus_globefish.hpp>
#include <game/game/d_fish_mus_keep_swimming.hpp>
#include <game/game/d_fish_mus_koi.hpp>
#include <game/game/d_fish_mus_lobster.hpp>
#include <game/game/d_fish_mus_lotus.hpp>
#include <game/game/d_fish_mus_moray.hpp>
#include <game/game/d_fish_mus_octopus.hpp>
#include <game/game/d_fish_mus_piranha.hpp>
#include <game/game/d_fish_mus_pufferfish.hpp>
#include <game/game/d_fish_mus_ray.hpp>
#include <game/game/d_fish_mus_slow.hpp>
#include <game/game/d_fish_mus_small.hpp>
#include <game/game/d_fish_museum.hpp>
#include <game/game/d_fish_museum_obj.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_snd_obj.hpp>
#include <game/mLib/m_3d/global.hpp>
#include <lib/egg/math/eggMath.h>
#include <game/cLib/c_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <game/sLib/s_lib.hpp>
#include <cstdio>
#include <lib/egg/core/eggFrmHeap.h>
#include <game/mLib/m_heap.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_bg_util.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_museum.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/snd/snd_id.hpp>
#include <cstring>
#include <game/game/d_camera.hpp>
#include <game/mLib/m_sphere.hpp>

namespace dFishMuseum {

struct HitCyl_c {
    HitCyl_c(const mVec3_c &pos, f32 radius, f32 height) : mPos(pos), mRadius(radius), mHeight(height) {}

    f32 getRadius() const { return mRadius; }
    f32 getHeight() const { return mHeight; }

    /* 0x00 */ mVec3_c mPos; // bottom centre
    /* 0x0C */ f32 mRadius;
    /* 0x10 */ f32 mHeight;
}; // size 0x14

struct HitCapsule_c {
    HitCapsule_c(const mVec3_c &start, const mVec3_c &end, f32 radius, f32 height)
        : mStart(start), mEnd(end), mRadius(radius), mHeight(height) {}

    f32 getRadius() const { return mRadius; }
    f32 getHeight() const { return mHeight; }

    /* 0x00 */ mVec3_c mStart;
    /* 0x0C */ mVec3_c mEnd;
    /* 0x18 */ f32 mRadius;
    /* 0x1C */ f32 mHeight;
}; // size 0x20

struct HitQuad_c {
    HitQuad_c(const mVec3_c &c0, const mVec3_c &c1, const mVec3_c &c2, const mVec3_c &c3, f32 radius, f32 height)
        : mRadius(radius), mHeight(height) {
        mCorners[0] = c0;
        mCorners[1] = c1;
        mCorners[2] = c2;
        mCorners[3] = c3;
    }

    f32 getRadius() const { return mRadius; }
    f32 getHeight() const { return mHeight; }

    /* 0x00 */ mVec3_c mCorners[4];
    /* 0x30 */ f32 mRadius;
    /* 0x34 */ f32 mHeight;
}; // size 0x38

typedef BOOL (*HitFishFunc)(dFishMusBase_c *a, dFishMusBase_c *b, mVec3_c *pushA, mVec3_c *pushB);

struct FishCreate_c {
    dFishMusBase_c *(*mCreate)(EGG::Heap *heap);
    const char *mName;
};

BOOL hitCylFish(dFishMusBase_c *a, dFishMusBase_c *b, mVec3_c *pushA, mVec3_c *pushB);
BOOL hitCylCapsuleFish(dFishMusBase_c *a, dFishMusBase_c *b, mVec3_c *pushA, mVec3_c *pushB);
BOOL hitCylBoxFish(dFishMusBase_c *a, dFishMusBase_c *b, mVec3_c *pushA, mVec3_c *pushB);
BOOL hitCapsuleCylFish(dFishMusBase_c *a, dFishMusBase_c *b, mVec3_c *pushA, mVec3_c *pushB);
BOOL hitCapsuleFish(dFishMusBase_c *a, dFishMusBase_c *b, mVec3_c *pushA, mVec3_c *pushB);
BOOL hitBoxCylFish(dFishMusBase_c *a, dFishMusBase_c *b, mVec3_c *pushA, mVec3_c *pushB);

extern FishCreate_c *sFishCreate[FISH_KIND_NUM];
extern HitFishFunc sHitFishFuncs[3][3];

// 0xCC
BOOL hitCyl(const HitCyl_c *a, const HitCyl_c *b, mVec3_c *push) {
    f32 dy, distSq, height, radius;
    mVec3_c diff = a->mPos - b->mPos;
    dy = nw4r::math::FAbs(diff.y);
    height = 0.5f * a->getHeight() + b->getHeight();
    radius = a->getRadius() + b->getRadius();
    if (dy < height) {
        distSq = diff.x * diff.x + diff.z * diff.z;
        if (distSq < radius * radius) {
            radius -= nw4r::math::FSqrt(distSq);
            diff.y = 0.0f;
            if (!diff.normalizeRS()) {
                diff.set(0.0f, 0.0f, 1.0f);
            }
            mVec3_c v = diff * radius;
            f32 dh = height - dy;
            if (dh < 2.0f) {
                v.set(0.0f, dh, 0.0f);
            }
            *push = v;
            return TRUE;
        }
    }
    return FALSE;
}

// 0x260
BOOL hitCapsule(const HitCapsule_c *a, const HitCyl_c *b, mVec3_c *push) {
    mVec3_c start = a->mStart;
    mVec3_c end = a->mEnd;
    mVec3_c pos = b->mPos;
    f32 dy = nw4r::math::FAbs(pos.y - start.y);
    f32 height = 0.5f * a->getHeight() + b->getHeight();
    f32 radius = a->getRadius() + b->getRadius();
    if (dy < height) {
        mVec3_c toPos = pos - end;
        toPos.y = 0.0f;
        mVec3_c axis = start - end;
        f32 dot = toPos.dot(axis);
        f32 lenSq = nw4r::math::VEC3LenSq(&axis);
        mVec3_c nearest;
        if (dot < 0.0f) {
            nearest = end;
        } else if (dot > lenSq) {
            nearest = start;
        } else {
            f32 t = dot / lenSq;
            nearest = start * t + end * (1.0f - t);
        }
        mVec3_c d = nearest - pos;
        f32 distSq = d.x * d.x + d.z * d.z;
        if (distSq < radius * radius) {
            f32 ratio = 2.0f * (height - dy) / height;
            ratio = ratio > 1.0f ? 1.0f : ratio;
            f32 depth = radius * ratio - nw4r::math::FSqrt(distSq);
            f32 len = depth < 0.0f ? 0.0f : depth;
            d.y = 0.0f;
            if (!d.normalizeRS()) {
                d.set(0.0f, 0.0f, 1.0f);
            }
            mVec3_c v = d * len;
            f32 dh = height - dy;
            if (dh < 2.0f) {
                v.set(0.0f, dh, 0.0f);
            }
            *push = v;
            return TRUE;
        }
    }
    return FALSE;
}

// 0x574
BOOL hitQuad(const HitQuad_c *a, const HitCyl_c *b, mVec3_c *push) {
    mVec3_c diff = a->mCorners[0] - b->mPos;
    f32 height = 0.5f * (a->getHeight() + b->getHeight());
    f32 radius = a->getRadius() + b->getRadius();
    f32 dy = nw4r::math::FAbs(diff.y);
    push->set(0.0f, 0.0f, 0.0f);
    if (dy < height) {
        for (int i = 0; i < 4; i++) {
            mVec3_c corner = a->mCorners[i];
            mVec3_c d = corner - b->mPos;
            f32 distSq = d.x * d.x + d.z * d.z;
            if (distSq < radius * radius) {
                f32 len = radius - nw4r::math::FSqrt(distSq);
                d.y = 0.0f;
                if (!d.normalizeRS()) {
                    d.set(0.0f, 0.0f, 1.0f);
                }
                *push += d * len;
            }
        }
        return TRUE;
    }
    return FALSE;
}

// Speculative stand-in. The REL's constant pool has -1.0f and FLT_EPSILON here (made after hitCyl's constants and
// before dFishMusBaseMove_c::execute's) but no code uses them: a function the linker stripped, or code the optimizer
// removed, created them. d_a_cockroachNP has the same unreferenced pair. 
// Should take a look after d_a_cockroachNP.
BOOL isOppositeDir(f32 dot) {
    return __fabsf(dot - -1.0f) <= FLT_EPSILON;
}

} // namespace dFishMuseum

#include "d_fish_mus.inc"
