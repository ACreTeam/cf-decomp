// Object collision (namespace dObjc): the pushes between actors and what the Wii remote pointer
// points at. .text 801AD6E0..801B0230, .ctors 80465840, .data 804FB8A0..804FB9A0,
// .bss 80625370..80625470, .sdata 8074C1E0..8074C210, .sbss 8074EAD8..8074EB08,
// .sdata2 80751800..80751848.
// See include/game/game/d_objc.hpp and notes/d_objc.txt.
#include <game/game/d_objc.hpp>
#include <game/cLib/c_math.hpp>
#include <game/framework/f_manager.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_world.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_pad.hpp>
#include <lib/MSL/arith.h>
#include <lib/egg/math/eggMath.h>
#include <revolution/MTX.h>
#include <cmath>

// Not decompiled yet (C linkage keeps the target names).
extern "C" {

// 800FA6AC (d_pad): the pointer's screen position
void fn_800FA6AC(mVec2_c *pos);
// 8018A080 (d_camera): a screen position to a world position
void fn_8018A080(mVec3_c *pos, const dCamera_c *cam, const mVec2_c *screen);
// 801692A0 (d_str): the structure manager
void *fn_801692A0();
// 80169424 (d_str): the structure standing on a unit
fBase_c *fn_80169424(void *mgr, int unitX, int unitZ, int);
}

// The process that runs the collision every frame (RTTI "dObjc_c").
class dObjc_c : public fBase_c {
public:
    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual ~dObjc_c() {}
};

// Debug settings (RTTI "dObjc_hostIO_c").
class dObjc_hostIO_c {
public:
    dObjc_hostIO_c() {
        _5 = 0;
        _6 = 0;
        _4 = 0;
    }

    virtual ~dObjc_hostIO_c() {}

    /* 0x4 */ u8 _4;
    /* 0x5 */ u8 _5;
    /* 0x6 */ u8 _6;
}; // size 0x8

namespace dObjc {

static dpdPos_c l_dpdPos;
static dpdHit_c l_dpdHit;
static dObjc_hostIO_c l_hostIO;

#ifdef MUST_MATCH
// 806253C4: nothing in the DOL references it (an unused object, kept by the original link). Its
// size is unknown: 0x15..0x17 bytes fit (MWCC 8-aligns .bss objects of 0x18 bytes or more).
#pragma force_active on
u8 lbl_806253C4[0x17];
#pragma force_active reset
#endif

static inline BOOL isZeroVec(const mVec3_c &v) {
    return std::fabs(nw4r::math::VEC3LenSq(v)) <= FLT_EPSILON;
}

const dpdPos_c *getDpdPos() {
    if (isZeroVec(l_dpdPos.mPos)) {
        return NULL;
    }
    return &l_dpdPos;
}

const dpdHit_c *getDpdHit() {
    if (isZeroVec(l_dpdHit.mPos)) {
        return NULL;
    }
    return &l_dpdHit;
}

void dpdPos_c::clear() {
    mPos.set(0.0f, 0.0f, 0.0f);
    mWater = 0;
}

void dpdPos_c::set(const mVec3_c &pos, BOOL water) {
    mPos = pos;
    mWater = water;
}

void dpdHit_c::clear() {
    mBase.set(NULL);
    mType = 0;
    mPos.set(0.0f, 0.0f, 0.0f);
    mParam = 0;
}

void dpdHit_c::set(const mVec3_c &pos, u32 type, fBase_c *base, u32 param) {
    mType = type;
    mPos = pos;
    mBase.set(base);
    mParam = param;
}

void baseID_c::set(fBase_c *base) {
    if (base != NULL) {
        mID = base->mUniqueID;
    } else {
        mID = (fBaseID_e)0;
    }
}

fBase_c *baseID_c::get() const {
    if (mID == 0) {
        return NULL;
    }
    return fManager_c::searchBaseByID(mID);
}

dpdTri_c *dpdTri_c::s_head;
int dpdTri_c::s_num;

dpdTri_c::dpdTri_c() {
    mOwnerID = (fBaseID_e)0;
    _40 = 0;
    mType = 0;
    mRegistered = 0;
    mNext = NULL;
}

void dpdTri_c::set(const dBGC::poly_c &poly, u32 type, fBase_c *owner, const mVec3_c *center,
                   u32 param) {
    mParam = param;
    if (center != NULL) {
        mCenter = *center;
    } else {
        mCenter.set(0.0f, 0.0f, 0.0f);
        mCenter += poly.mPos0;
        mCenter += poly.mPos1;
        mCenter += poly.mPos2;
        f32 inv = 1.0f / 3.0f;
        mCenter.x *= inv;
        mCenter.y *= inv;
        mCenter.z *= inv;
    }
    if (dWorld::isCurve()) {
        mVec3_c p0;
        mVec3_c p1;
        mVec3_c p2;
        p0 = poly.mPos0;
        p1 = poly.mPos1;
        p2 = poly.mPos2;
        dWorld::curvePositionSimple(&p0, &p0);
        dWorld::curvePositionSimple(&p1, &p1);
        dWorld::curvePositionSimple(&p2, &p2);
        dBGC::poly_c::set(p0, p1, p2);
    } else {
        dBGC::poly_c::set(poly.mPos0, poly.mPos1, poly.mPos2, poly.mNormal);
    }
    mType = type;
    mOwnerID = (fBaseID_e)0;
    if (owner != NULL) {
        mOwnerID = owner->mUniqueID;
    }
}

void dpdTri_c::regist() {
    init();
    s_num++;
    if (mRegistered) {
        clearList();
    } else {
        mNext = s_head;
        s_head = this;
        mRegistered = TRUE;
    }
}

void dpdTri_c::init() {
    mNext = NULL;
    _40 = 0;
}

void dpdTri_c::create() {
    clearList();
}

void dpdTri_c::clearList() {
    for (dpdTri_c *tri = s_head; tri != NULL; tri = tri->mNext) {
        tri->mRegistered = FALSE;
    }
    s_head = NULL;
    s_num = 0;
}

void dpdTri_c::destroy() {
    clearList();
}

void dpdQuad_c::set(const mVec3_c &p0, const mVec3_c &p1, const mVec3_c &p2, const mVec3_c &p3,
                    u32 type, fBase_c *owner, const mVec3_c *center, u32 param) {
    dBGC::poly_c tri0(p0, p1, p2);
    dBGC::poly_c tri1(p0, p2, p3);
    if (center != NULL) {
        mTri[0].set(tri0, type, owner, center, param);
        mTri[1].set(tri1, type, owner, center, param);
    } else {
        mVec3_c c(0.0f, 0.0f, 0.0f);
        c += p0;
        c += p1;
        c += p2;
        c += p3;
        c /= 4.0f;
        mTri[0].set(tri0, type, owner, &c, param);
        mTri[1].set(tri1, type, owner, &c, param);
    }
}

void dpdQuad_c::regist() {
    for (u32 i = 0; i < 2; i++) {
        mTri[i].regist();
    }
}

void dpdBox_c::set(const mVec3_c &pos, f32 sizeX, f32 sizeZ, f32 height, s16 angle, u32 type,
                   fBase_c *owner, u32 param) {
    f32 hx = 0.5f * sizeX;
    f32 hz = 0.5f * sizeZ;
    f32 nx = -hx;
    f32 nz = -hz;
    mMtx_c mtx;
    PSMTXTrans(mtx, pos.x, pos.y, pos.z);
    mtx.YrotM(angle);

    mVec3_c v0;
    mVec3_c v1;
    mVec3_c v2;
    mVec3_c v3;

    v0 = mVec3_c(nx, height, hz);
    v1 = mVec3_c(nx, 0.0f, hz);
    v2 = mVec3_c(hx, 0.0f, hz);
    v3 = mVec3_c(hx, height, hz);
    fn_803911A4(mtx, v0, v0);
    fn_803911A4(mtx, v1, v1);
    fn_803911A4(mtx, v2, v2);
    fn_803911A4(mtx, v3, v3);
    mQuad[0].set(v0, v1, v2, v3, type, owner, &pos, param);

    v0 = mVec3_c(hx, height, hz);
    v1 = mVec3_c(hx, 0.0f, hz);
    v2 = mVec3_c(hx, 0.0f, nz);
    v3 = mVec3_c(hx, height, nz);
    fn_803911A4(mtx, v0, v0);
    fn_803911A4(mtx, v1, v1);
    fn_803911A4(mtx, v2, v2);
    fn_803911A4(mtx, v3, v3);
    mQuad[1].set(v0, v1, v2, v3, type, owner, &pos, param);

    v0 = mVec3_c(nx, height, nz);
    v1 = mVec3_c(nx, 0.0f, nz);
    v2 = mVec3_c(nx, 0.0f, hz);
    v3 = mVec3_c(nx, height, hz);
    fn_803911A4(mtx, v0, v0);
    fn_803911A4(mtx, v1, v1);
    fn_803911A4(mtx, v2, v2);
    fn_803911A4(mtx, v3, v3);
    mQuad[2].set(v0, v1, v2, v3, type, owner, &pos, param);

    v0 = mVec3_c(hx, height, nz);
    v1 = mVec3_c(hx, 0.0f, nz);
    v2 = mVec3_c(nx, 0.0f, nz);
    v3 = mVec3_c(nx, height, nz);
    fn_803911A4(mtx, v0, v0);
    fn_803911A4(mtx, v1, v1);
    fn_803911A4(mtx, v2, v2);
    fn_803911A4(mtx, v3, v3);
    mQuad[3].set(v0, v1, v2, v3, type, owner, &pos, param);

    v0 = mVec3_c(nx, height, nz);
    v1 = mVec3_c(nx, height, hz);
    v2 = mVec3_c(hx, height, hz);
    v3 = mVec3_c(hx, height, nz);
    fn_803911A4(mtx, v0, v0);
    fn_803911A4(mtx, v1, v1);
    fn_803911A4(mtx, v2, v2);
    fn_803911A4(mtx, v3, v3);
    mQuad[4].set(v0, v1, v2, v3, type, owner, &pos, param);
}

void dpdBox_c::regist() {
    for (u32 i = 0; i < 5; i++) {
        mQuad[i].regist();
    }
}

base_c *base_c::s_head;

base_c::base_c() {
    init();
    mWeightFunc = NULL;
    mHitFunc = NULL;
    mRegistered = FALSE;
    mOnceHit = FALSE;
    mNext = NULL;
}

void base_c::create() {
    clearList();
}

void base_c::clearList() {
    for (base_c *obj = s_head; obj != NULL; obj = obj->mNext) {
        obj->mRegistered = FALSE;
    }
    s_head = NULL;
}

void base_c::destroy() {
    clearList();
}

// Turns v about the x axis by the angle whose sine and cosine are given.
static inline void rotYZ(mVec3_c *v, f32 sn, f32 cs) {
    f32 y = v->y;
    f32 z = v->z;
    v->y = cs * y - sn * z;
    v->z = sn * y + cs * z;
}

// Pushes end out of the column (radius, height) standing on pos, coming from start. The test is
// done in the drum's frame at pos (turned back by its curve angle).
static BOOL checkColumn(mVec3_c *end, const mVec3_c *start, const mVec3_c *pos, f32 radius,
                        f32 height) {
    mVec3_c center;
    mAng angle = dWorld::curvePosition(&center, pos);
    mVec3_c s(start->x, start->y, start->z);
    mAng back(-angle.mAngle);
    mVec3_c e(end->x, end->y, end->z);
    mVec3_c c(center.x, center.y, center.z);
    f32 sn = back.sin();
    f32 cs = back.cos();
    rotYZ(&s, sn, cs);
    rotYZ(&e, sn, cs);
    rotYZ(&c, sn, cs);

    dBGC::column_c col(c, radius, height);
    if (col.crossTopBottom(&e, s) || col.crossSideInf(&e, s)) {
        e.rotX(angle);
        end->set(e.x, e.y, e.z);
        return TRUE;
    }
    return FALSE;
}

static inline s16 angleDiff(mAng a, mAng b) {
    return a.mAngle - b.mAngle;
}

BOOL base_c::checkPushAngle(mAng *angle) const {
    if (mHitType != 0) {
        angle->mAngle += 0x8000;
        f32 z = getPush()->z;
        s16 push = cM::atan2s(getPush()->x, z);
        return labs(angleDiff(push, *angle)) <= 0x2000;
    }
    return FALSE;
}

void base_c::set(f32 radius, f32 height, u32 type, u32 mask, u8 weight, u32 param) {
    mRadius = radius;
    mHeight = height;
    mType = type;
    mMask = mask;
    mWeight = weight;
    mParam = param;
}

void base_c::init() {
    mNext = NULL;
    mPush.set(0.0f, 0.0f, 0.0f);
    mHitID = (fBaseID_e)0;
    mHitType = 0;
    _35 = 0;
}

void base_c::regist() {
    init();
    if (mRegistered) {
        clearList();
    } else {
        mNext = s_head;
        s_head = this;
        mRegistered = TRUE;
    }
}

BOOL base_c::check(base_c *other) {
    u32 typeA = mType & 0x7FFFFFFF;
    u32 maskA = mMask & 0x7FFFFFFF;
    u32 typeB = other->mType & 0x7FFFFFFF;
    BOOL hit = (typeA & other->mMask) && (maskA & typeB);
    if (hit) {
        fBaseID_e id = getID();
        if (id != 0 && id == other->getID()) {
            return FALSE;
        }
        return this != other;
    }
    return FALSE;
}

static inline BOOL isFront(const dBGC::poly_c *poly, const mVec3_c &pos) {
    return poly->calcDist(pos) >= 0.0f;
}

static inline mAng getHorizonAngle(const mVec3_c &eye, const mVec3_c &pos) {
    return dWorld::getHorizonAngle(&eye, &pos);
}

// 1. Pushes registered base_c apart (each pair once). 2. Casts the pointer ray: from the camera
// eye through the pointer, 1.5 blocks long; finds where it meets the ground (the drum or a flat
// plane at the player's height: l_dpdPos) and the nearest column, field object, furniture,
// dpdTri_c, base_c (TYPE_DPD) or insect it hits (l_dpdHit), shortening the ray at each hit.
void calc() {
    dCamera_c *cam;
    dPlayerActor_c *player;
    dFdBase_c *field;
    u32 param;
    fBase_c *base;

    for (base_c *obj = base_c::s_head; obj != NULL; obj = obj->mNext) {
        if (obj->mWeight == 0) {
            obj->mDepth = -1.0f;
        }
    }

    for (base_c *a = base_c::s_head; a != NULL; a = a->mNext) {
        const mVec3_c *aPos = a->getPos();
        for (base_c *b = a->mNext; b != NULL; b = b->mNext) {
            if (!a->check(b)) {
                continue;
            }
            mVec3_c d = *b->getPos() - *aPos;
            f32 overlapY;
            if (d.y < 0.0f) {
                overlapY = b->mHeight + d.y;
            } else {
                overlapY = a->mHeight - d.y;
            }
            if (!(overlapY > 0.0f)) {
                continue;
            }
            f32 dist = EGG::Mathf::sqrt(d.x * d.x + d.z * d.z);
            if (std::fabs(dist) < 0.001f) {
                d.x = 1.0f;
                d.z = 0.0f;
                dist = 1.0f;
            }
            f32 depth = (a->mRadius + b->mRadius) - dist;
            if (!(depth > 0.0f)) {
                continue;
            }
            u8 aWeight = a->mWeight;
            u8 bWeight = b->mWeight;
            f32 aRate = 1.0f;
            f32 bRate = 1.0f;
            if (a->mWeightFunc != NULL) {
                aWeight = a->mWeightFunc(a, b, &aRate);
            }
            if (b->mWeightFunc != NULL) {
                bWeight = b->mWeightFunc(b, a, &bRate);
            }

            if (aWeight == 0) {
                if (depth >= a->mDepth) {
                    a->mDepth = depth;
                    if (b->mType & base_c::TYPE_ONCE) {
                        if (!b->mOnceHit) {
                            a->mHitType |= b->mType;
                            b->mOnceHit = TRUE;
                        }
                    } else {
                        a->mHitType |= b->mType;
                    }
                    a->mHitID = b->getID();
                    if (a->mHitFunc != NULL) {
                        a->mHitFunc(b);
                    }
                }
            } else {
                if (b->mType & base_c::TYPE_ONCE) {
                    if (!b->mOnceHit) {
                        a->mHitType |= b->mType;
                        b->mOnceHit = TRUE;
                    }
                } else {
                    a->mHitType |= b->mType;
                }
                a->mHitID = b->getID();
                if (a->mHitFunc != NULL) {
                    a->mHitFunc(b);
                }
            }

            // b's side differs from a's in the target: no mDepth update, and no hit callback when b
            // has a weight.
            if (bWeight == 0) {
                if (depth >= b->mDepth) {
                    if (a->mType & base_c::TYPE_ONCE) {
                        if (!a->mOnceHit) {
                            b->mHitType |= a->mType;
                            a->mOnceHit = TRUE;
                        }
                    } else {
                        b->mHitType |= a->mType;
                    }
                    b->mHitID = a->getID();
                    if (b->mHitFunc != NULL) {
                        b->mHitFunc(a);
                    }
                }
            } else {
                if (a->mType & base_c::TYPE_ONCE) {
                    if (!a->mOnceHit) {
                        b->mHitType |= a->mType;
                        a->mOnceHit = TRUE;
                    }
                } else {
                    b->mHitType |= a->mType;
                }
                b->mHitID = a->getID();
            }

            if (aWeight == 0 || bWeight == 0) {
                continue;
            }
            if (aWeight == 0xFF && bWeight == 0xFF) {
                continue;
            }
            if (aWeight == 0xFF) {
                f32 scale = depth / dist;
                b->mPush.y = 0.0f;
                b->mPush.x += bRate * (d.x * scale);
                b->mPush.z += bRate * (d.z * scale);
            } else if (bWeight == 0xFF) {
                f32 scale = depth / dist;
                a->mPush.y = 0.0f;
                a->mPush.x += -(aRate * (d.x * scale));
                a->mPush.z += -(aRate * (d.z * scale));
            } else {
                f32 aw = (f32)aWeight - 1.0f;
                f32 bw = (f32)bWeight - 1.0f;
                if (aWeight == 1 && bWeight == 1) {
                    bw = 1.0f;
                    aw = 1.0f;
                }
                f32 sum = aw + bw;
                b->mPush.y = 0.0f;
                a->mPush.y = 0.0f;
                f32 scale = 0.5f * (depth / dist);
                f32 aScale = scale * (bw / sum);
                f32 bScale = scale * (aw / sum);
                a->mPush.x += -(aRate * (d.x * aScale));
                a->mPush.z += -(aRate * (d.z * aScale));
                b->mPush.x += bRate * (d.x * bScale);
                b->mPush.z += bRate * (d.z * bScale);
            }
        }
    }

    cam = lbl_8074E9B0;
    player = fn_800FBC7C(4);
    field = fn_80190C44(0);
    l_dpdPos.clear();
    l_dpdHit.clear();
    if (cam != NULL && mPad::getCore()->getDpdValidFlag() > 0 && player != NULL && field != NULL) {
        dBGC::column_c hitCol;
        mVec2_c screen;
        fn_800FA6AC(&screen);
        mVec3_c target;
        fn_8018A080(&target, cam, &screen);
        mVec3_c eye = cam->getEyePos();
        mVec3_c start = eye;
        mVec3_c end = target - start;
        end.normalizeRS();
        end *= 1.5f * lbl_80750524;
        end += eye;
        f32 groundY = player != NULL ? player->mPos.y : 0.0f;
        f32 radius = groundY + dWorld::getRadius();
        static f32 s_range = 7.0f * lbl_80750520;

        if (dWorld::isCurve()) {
            mVec3_c s = start;
            mVec3_c e = end;
            dBGC::column_c drum(mVec3_c(0.0f, 0.0f, 0.0f), radius);
            drum.mHeight = s_range;
            s.rotZ(0x4000);
            e.rotZ(0x4000);
            BOOL ground = FALSE;
            BOOL water = FALSE;
            mVec3_c from = s;
            mVec3_c to = e;
            mVec3_c orig = e;
            if (drum.crossSideInf(&to, from)) {
                to.rotZ(-0x4000);
                mVec3_c pos;
                dWorld::toFieldPosition(&pos, &to);
                dBGCF::groundChk_c chk(&pos, 0, 0, 0);
                if (chk.mWater) {
                    ground = FALSE;
                    water = TRUE;
                } else {
                    ground = TRUE;
                }
                l_dpdPos.set(pos, water);
            }
            if (!ground && !water) {
                nw4r::math::VEC3 *const center = &lbl_80623FEC;
                mVec3_c s2 = start;
                mVec3_c e2 = end;
                static f32 s_width = s_range;
                mVec3_c v0(-s_width, s_width, 0.0f);
                mVec3_c v1(-s_width, -s_width, 0.0f);
                mVec3_c v2(s_width, -s_width, 0.0f);
                mVec3_c v3(s_width, s_width, 0.0f);
                mAng horizon = getHorizonAngle(cam->getEyePos(), player->mPos);
                mMtx_c mtx;
                mtx.XrotS(horizon);
                mtx.concat(mMtx_c::createTrans(center->x, radius, 0.0f));
                mtx.XrotM(-0x1000);
                fn_803911A4(mtx, v0, v0);
                fn_803911A4(mtx, v1, v1);
                fn_803911A4(mtx, v2, v2);
                fn_803911A4(mtx, v3, v3);
                mVec3_c normal;
                dBGC::poly_c::calcNormal(&normal, v0, v1, v2);
                dBGC::poly_c polys[2] = {dBGC::poly_c(v0, v1, v2, normal),
                                         dBGC::poly_c(v0, v2, v3, normal)};
                dBGC::poly_c *poly = polys;
                for (u32 i = 0; i < 2; i++, poly++) {
                    mVec3_c cross;
                    if (isFront(poly, s2) && !isFront(poly, e2) && poly->crossSeg(&cross, s2, e2)) {
                        mVec3_c pos;
                        dWorld::toFieldPosition(&pos, &cross);
                        if (pos.z < 0.0f && cross.z < 0.0f) {
                            pos.z = 1.0f;
                        } else if (isCurrentSceneAttr(9) && cross.z < 0.0f) {
                            pos.z = 1.0f;
                        }
                        l_dpdPos.set(pos, FALSE);
                        break;
                    }
                }
            }
        } else {
            mVec3_c s = start;
            mVec3_c e = end;
            mVec3_c v0;
            mVec3_c v1;
            mVec3_c v2;
            mVec3_c v3;
            v0.x = -s_range;
            v0.y = groundY;
            v0.z = -s_range;
            v1.x = -s_range;
            v1.y = groundY;
            v1.z = s_range;
            v2.x = s_range;
            v2.y = groundY;
            v2.z = s_range;
            v3.x = s_range;
            v3.y = groundY;
            v3.z = -s_range;
            static mVec3_c s_up(0.0f, 1.0f, 0.0f);
            dBGC::poly_c polys[2];
            polys[0].set(v0, v1, v3, s_up);
            polys[1].set(v1, v2, v3, s_up);
            dBGC::poly_c *poly = polys;
            for (u32 i = 0; i < 2; i++, poly++) {
                mVec3_c cross;
                if (isFront(poly, s) && !isFront(poly, e) && poly->crossSeg(&cross, s, e)) {
                    mVec3_c pos = cross;
                    l_dpdPos.set(pos, FALSE);
                    break;
                }
            }
        }

        if (player != NULL) {
            int unitX;
            int unitZ;
            dBGCF::posToUnit(&unitX, &unitZ, &player->mPos);
            getCurrentScene();
            for (int z = unitZ + 3; z >= unitZ - 3; z--) {
                for (int x = unitX + 3; x >= unitX - 3; x--) {
                    f32 colRadius;
                    f32 colHeight;
                    int attr;
                    if (dBGCF::getColumnAttr(x, z, &colRadius, &colHeight, &attr) && attr != 1) {
                        mVec3_c pos;
                        dBGCF::unitToPos(&pos, x, z);
                        pos.y = dBGCF::getUnitBaseY(x, z);
                        if (checkColumn(&end, &start, &pos, colRadius, colHeight)) {
                            dItem::Item *item = field->getItem(x, z, 0);
                            BOOL hit = FALSE;
                            if (item != NULL) {
                                u16 id = item->mId;
                                if (ITEM_NAME_TYPE(id) == 0) {
                                    l_dpdHit.set(pos, HIT_FG, NULL, 0);
                                    hit = TRUE;
                                } else if (id == 0x7003) {
                                    l_dpdHit.set(pos, HIT_STR,
                                                 fn_80169424(fn_801692A0(), x, z, 1), 0);
                                    hit = TRUE;
                                } else if (fn_80169424(fn_801692A0(), x, z, 1) != NULL) {
                                    l_dpdHit.set(pos, HIT_STR,
                                                 fn_80169424(fn_801692A0(), x, z, 1), 0);
                                    hit = TRUE;
                                }
                            }
                            if (hit) {
                                hitCol.set(pos, colRadius, colHeight);
                            }
                        }
                    }
                }
            }
        }

        for (dpdTri_c *tri = dpdTri_c::s_head; tri != NULL; tri = tri->mNext) {
            mVec3_c cross;
            if (isFront(tri, start) && !isFront(tri, end) && tri->crossSeg(&cross, start, end)) {
                end = cross;
                l_dpdHit.set(tri->mCenter, tri->mType,
                             fManager_c::searchBaseByID(tri->mOwnerID), 0);
                hitCol.mRadius = 0.0f;
                hitCol.mHeight = 0.0f;
            }
        }

        if (isCurrentSceneAttr(0x210) || getCurrentScene() == 0x33) {
            for (int z = 0; z < 16; z++) {
                for (int x = 0; x < 16; x++) {
                    dItem::Item *item = field->getItem(x, z, 0);
                    if (item != NULL && dItem::isRealItemId(item->mId)) {
                        const dItem::BITM *bitm = dItem::getBITM(item->mId);
                        if (bitm != NULL && dItem::clampField(bitm->m_ftrFunc, 0x41, 1) == 0) {
                            mVec3_c pos;
                            dBGCF::unitToPos(&pos, x, z);
                            pos.y = dBGCF::getUnitBaseY(&pos);
                            if (checkColumn(&end, &start, &pos, 12.8f, 11.2f)) {
                                l_dpdHit.set(pos, HIT_FTR, NULL, 0);
                                hitCol.set(pos, 12.8f, 11.2f);
                            }
                        }
                    }
                }
            }
        }

        for (base_c *obj = base_c::s_head; obj != NULL; obj = obj->mNext) {
            if (obj->mType & base_c::TYPE_DPD) {
                if (checkColumn(&end, &start, obj->getPos(), obj->getRadius(), obj->getHeight())) {
                    param = obj->mParam;
                    base = fManager_c::searchBaseByID(obj->getID());
                    l_dpdHit.set(*obj->getPos(), obj->mType, base, param);
                    hitCol.mRadius = 0.0f;
                    hitCol.mHeight = 0.0f;
                }
            }
        }

        for (ins_c *ins = ins_c::s_head; ins != NULL; ins = ins->mNext) {
            dBGC::line_c line(start, end);
            mVec3_c cross;
            if (ins->crossLine(&cross, line)) {
                end = cross;
                mVec3_c pos;
                dWorld::toFieldPosition(&pos, &ins->mCenter);
                l_dpdHit.set(pos, HIT_INSECT, NULL, ins->mParam);
                hitCol.mRadius = 0.0f;
                hitCol.mHeight = 0.0f;
            }
        }
    }
}

const mVec3_c *base_c::getPush() const {
    if (mHitType != 0) {
        return &mPush;
    }
    return &mVec3_c::Zero;
}

BOOL base_c::isHit(u32 type) const {
    return (type & mHitType) == type;
}

const mVec3_c *ac_c::getPos() {
    if (mActor != NULL) {
        return mActor->getPosP();
    }
    static mVec3_c s_zero;
    return &s_zero;
}

fBaseID_e ac_c::getID() const {
    return mActor->mUniqueID;
}

void ac_c::set(dActor_c *actor, f32 radius, f32 height, u32 type, u32 mask, u8 weight,
               u32 param) {
    mActor = actor;
    base_c::set(radius, height, type, mask, weight, param);
}

void pos_c::set(const mVec3_c &pos, f32 radius, f32 height, u32 type, u32 mask, u8 weight,
                u32 param) {
    mPos = pos;
    base_c::set(radius, height, type, mask, weight, param);
}

void acpos_c::set(dActor_c *actor, const mVec3_c &pos, f32 radius, f32 height, u32 type,
                  u32 mask, u8 weight, u32 param) {
    ac_c::set(actor, radius, height, type, mask, weight, param);
    mPos = pos;
}

ins_c *ins_c::s_head;

ins_c::ins_c() {
    mNext = NULL;
    mHit = FALSE;
    mRegistered = FALSE;
    mEnabled = TRUE;
}

void ins_c::regist(const mVec3_c *pos, f32 radius, u32 param, BOOL enable) {
    mNext = NULL;
    if (mRegistered) {
        clearList();
        return;
    }
    if (s_head == NULL) {
        s_head = this;
    } else {
        mNext = s_head;
        s_head = this;
    }
    mRegistered = TRUE;
    mParam = param;
    mEnabled = enable;
    mVec3_c center;
    dWorld::curvePosition(&center, pos);
    mCenter = center;
    mRadius = radius;
}

BOOL ins_c::check(const mVec3_c *start, const mVec3_c *end, f32 radius, u32 *param) {
    BOOL hit = FALSE;
    u32 dummy;
    if (param == NULL) {
        param = &dummy;
    }
    *param = 0;
    ins_c *ins = s_head;
    mVec3_c s;
    dWorld::curvePosition(&s, start);
    mVec3_c e;
    dWorld::curvePosition(&e, end);
    mVec3_c step = e - s;
    step.x /= 6.0f;
    step.y /= 6.0f;
    step.z /= 6.0f;
    static mVec3_c s_points[7];
    for (u32 i = 0; i < 7; i++) {
        s_points[i].set(s.x + (f32)i * step.x, s.y + (f32)i * step.y, s.z + (f32)i * step.z);
    }
    for (; ins != NULL; ins = ins->mNext) {
        ins->mHit = FALSE;
        f32 r = radius + ins->mRadius;
        f32 rr = r * r;
        for (u32 i = 0; i <= 6; i++) {
            if (!hit && ins->mEnabled && PSVECSquareDistance(s_points[i], ins->mCenter) <= rr) {
                ins->mHit = TRUE;
                hit = TRUE;
                *param = ins->mParam;
            }
        }
    }
    return hit;
}

void ins_c::clearList() {
    ins_c *ins = s_head;
    while (ins != NULL) {
        ins_c *next = ins->mNext;
        ins->mRegistered = FALSE;
        ins->mNext = NULL;
        ins = next;
    }
    s_head = NULL;
}

} // namespace dObjc

CUSTOM_BASE_PROFILE(OBJC, dObjc_c, 0x12A, 0x44);

int dObjc_c::create() {
    dObjc::base_c::create();
    dObjc::dpdTri_c::create();
    dObjc::ins_c::s_head = NULL;
    return 1;
}

int dObjc_c::execute() {
    dObjc::calc();
    dObjc::base_c::clearList();
    dObjc::dpdTri_c::clearList();
    return 1;
}

int dObjc_c::draw() {
    return 1;
}

int dObjc_c::doDelete() {
    dObjc::base_c::destroy();
    dObjc::dpdTri_c::destroy();
    dObjc::ins_c::s_head = NULL;
    return 1;
}
