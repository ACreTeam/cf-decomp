// The balloons of the day (REL d_organ_balloonNP). See
// include/game/game/d_organ_balloon.hpp.
// REL .text 0x5C..0x1918, .rodata 0x0..0x80, .data 0x0..0x1E8, .bss 0x8..0xC.
#include <game/game/d_organ_balloon.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_random.hpp>
#include <game/game/d_search_cand_base.hpp>
#include <game/game/d_str.hpp>
#include <game/mLib/m_heap.hpp>
#include <nw4r/g3d.h>

namespace dOrganBalloon {
// _VEC3, not mVec3_c: a file-scope mVec3_c would need __sinit.
static const nw4r::math::_VEC3 l_ofs = {-17.5f, 23.0f, 0.0f}; // string root
// the wind swings between these two
static const nw4r::math::_VEC3 l_windA = {-0.02f, 0.0f, 0.0f};
static const nw4r::math::_VEC3 l_windB = {0.02f, 0.0f, 0.0f};

// .text 0x5C
BASE_PROFILE(ORGAN_BALLOON, dOrganBalloon_c);
} // namespace dOrganBalloon

dOrganBalloon_c *dOrganBalloon_c::s_instance;

using namespace dOrganBalloon;

// .text 0xF0
int dOrganBalloon_c::create() {
    nw4r::g3d::ResFile file(dSTR::getBrresBank()->getBrres("obj_balloon"));
    nw4r::g3d::ResMdl mdl = file.GetResMdl(0);
    nw4r::g3d::ResAnmClr clr = file.GetResAnmClr(0);
    EGG::Heap *parentHeap = dHeap::localHeap_p;
    m_frmHeap =
        mHeap::createFrmHeap(HEAP_SIZE, parentHeap, "dOrganBalloon::m_frmHeap",
                             0x20, mHeap::OPT_NONE);
    mAllocator.attach(parentHeap, 0x20);
    dRandom_c rnd(RANDOM_SEED);
    rnd.initFromDateWithSalt(*dTime_c::getCurrent(), RANDOM_SALT);
    dSearchCand_c<COLOR_NUM> cand;
    cand.setAll();
    for (int i = 0; i < BALLOON_NUM; i++) {
        mModels[i].create(mdl, &mAllocator,
                          nw4r::g3d::ScnMdl::BUFFER_RESTEVCOLOR, 1, NULL);
        mColorAnms[i].create(mdl, clr, &mAllocator, NULL, 1);
        int color = cand.getRandom(&rnd);
        cand.remove(color);
        mColorAnms[i].setFrame(color, 0);
        mModels[i].setAnm(mColorAnms[i]);
        dOrganBalloonString_c *string =
            dOrganBalloonString_c::create(m_frmHeap);
        mpStrings[i] = string;
        string->mIndex = i;
    }
    mColor = cand.getRandom(&rnd);
    // the parent actor (Phineas) places the balloons
    dActor_c *parent = (dActor_c *)getConnectParent();
    if (parent != NULL) {
        const mVec3_c *pos = parent->getPosP();
        if (pos != NULL) {
            mPos = *pos;
        } else {
            mPos.set(0.0f, 0.0f, 0.0f);
        }
    } else {
        mPos.set(0.0f, 0.0f, 0.0f);
    }
    s_instance = this;
    calcMtx();
    return SUCCEEDED;
}

// .text 0x404
int dOrganBalloon_c::execute() {
    mWindPhase += WIND_PHASE_STEP;
    f32 s = mWindPhase.sin();
    f32 t = 1.0f - s;
    mWind.x = l_windA.x * s + l_windB.x * t;
    mWind.y = l_windA.y * s + l_windB.y * t;
    mWind.z = l_windA.z * s + l_windB.z * t;
    calcMtx();
    return SUCCEEDED;
}

// .text 0x4C0
int dOrganBalloon_c::draw() {
    for (int i = 0; i < BALLOON_NUM; i++) {
        mpStrings[i]->entry();
        mModels[i].entry();
    }
    return SUCCEEDED;
}

// .text 0x540
int dOrganBalloon_c::doDelete() {
    for (int i = 0; i < BALLOON_NUM; i++) {
        mModels[i].remove();
        mColorAnms[i].remove();
        if (mpStrings[i] != NULL) {
            mpStrings[i]->destroy();
            mpStrings[i] = NULL;
        }
    }
    if (m_frmHeap != NULL) {
        m_frmHeap->destroy();
        m_frmHeap = NULL;
    }
    s_instance = NULL;
    return SUCCEEDED;
}

// .text 0x60C
void dOrganBalloon_c::calcMtx() {
    mAng angle = 0;
    for (int i = 0; i < BALLOON_NUM; i++) {
        mMtx_c mtx;
        dActor_c::makeMtx(&mtx, &mPos, mAng(0));
        mVec3_c ofs(l_ofs);
        mtx.concat(mMtx_c::createTrans(ofs));
        mtx.YrotM(angle);
        mMtx_c stringMtx = *mpStrings[i]->calc(&mtx);
        mModels[i].setLocalMtx(&stringMtx);
        mModels[i].calc(false);
        angle += BALLOON_ANGLE_STEP;
    }
}

// @HACK: Keep this scalar read inline: it gives the matrix-copy temporary the
// creation order needed by MWCC's FPR allocator (see the TU notes).
static inline f32 getMatrixElement(const mMtx_c &mtx, int i) {
    return mtx.a[i];
}

// .text 0x794
const mMtx_c *dOrganBalloonString_c::calcInit(const mMtx_c *mtx) {
    // Folded named statics: their initializers are the tail of .rodata.
    static const f32 LEN = 4.0f; // segment length (3.5 in dBalloonString_c)
    static const int POINT_NUM = 8;
    mSwingPhase = 0;
    mSwingAngle = 4096.0f * mSwingPhase.sin();
    for (int i = 0; i < 12; i++) {
        mTransform.a[i] = getMatrixElement(*mtx, i);
    }
    mVec3_c *points = mPoints;
    mVec3_c *tangents = mTangents;
    mVec3_c *offsets = mVelocities;
    mVec3_c x(mtx->_00, mtx->_10, mtx->_20);
    mVec3_c y(mtx->_01, mtx->_11, mtx->_21);
    mVec3_c z(mtx->_02, mtx->_12, mtx->_22);
    mScale.set(PSVECMag(x), PSVECMag(y), PSVECMag(z));
    mVec3_c pos = mVec3_c::Zero;
    mVec3_c dir(0.0f, mtx->_13, mtx->_23);
    if (!dir.normalizeRS()) {
        dir.set(0.0f, 1.0f, 0.0f);
    }
    mVec3_c step = LEN * dir;
    for (int i = 0; i < POINT_NUM; i++) {
        points[i] = pos;
        tangents[i] = dir;
        offsets[i] = mVec3_c::Zero;
        pos += step;
    }
    pos = points[6];
    setInitialDirection(dir);
    mDrawMtx._03 = mTransform._03 + pos.x * mScale.x;
    mDrawMtx._13 = mTransform._13 + pos.y * mScale.y;
    mDrawMtx._23 = mTransform._23 + pos.z * mScale.z;
    return &mDrawMtx;
}

// .text 0xAE8
const mMtx_c *dOrganBalloonString_c::calcHold(const mMtx_c *mtx) {
    static const f32 LEN = 4.0f;
    static const f32 MAX_LEN = LEN * 1.2f;
    static const f32 MIN_LEN = LEN * 0.8f; // unused, but in .rodata
    // declared before organ, assigned after the scale (register order)
    mVec3_c *points;
    mVec3_c *tangents;
    dOrganBalloon_c *organ = dOrganBalloon_c::s_instance;
    mVec3_c x(mtx->_00, mtx->_10, mtx->_20);
    mVec3_c y(mtx->_01, mtx->_11, mtx->_21);
    mVec3_c z(mtx->_02, mtx->_12, mtx->_22);
    mScale.set(PSVECMag(x), PSVECMag(y), PSVECMag(z));
    mVec3_c delta(mtx->_03 - mTransform._03, mtx->_13 - mTransform._13,
                  mtx->_23 - mTransform._23);
    mTransform = *mtx;
    points = mPoints;
    tangents = mTangents;
    mVec3_c dir(0.0f, mtx->_13, mtx->_23);
    if (!dir.normalizeRS()) {
        dir.set(0.0f, 1.0f, 0.0f);
    }
    mVec3_c gravity = -0.075f * dir;
    gravity += organ->mWind;
    mSwingPhase += SWING_PHASE_STEP;
    mSwingAngle = 4096.0f * mSwingPhase.sin();
    points[0] = mVec3_c::Zero;
    mVec3_c tangent(0.0f, 0.0f, 1.0f);
    tangent.rotX(mAng(ROOT_TILT));
    tangent.rotY(mSwingAngle);
    PSMTXMultVecSR(*mtx, tangent, tangents[0]);
    for (int i = 1; i < 8; i++) {
        points[i] -= delta;
    }
    mVec3_c *point = &points[1];
    mVec3_c *tangentPtr = &tangents[1];
    mVec3_c *velocity = &mVelocities[1];
    for (int i = 1; i < 7; i++) {
        mVec3_c *previous = &points[i - 1];
        mVec3_c back = *previous - *point;
        float backLength = PSVECMag(back);
        mVec3_c front = points[i + 1] - *point;
        float frontLength = PSVECMag(front);
        if (!back.normalizeRS()) {
            back = -1.0f * dir;
        }
        if (!front.normalizeRS()) {
            front = dir;
        }
        tangentPtr->set((front.x - back.x) * 0.5f, (front.y - back.y) * 0.5f,
                        (front.z - back.z) * 0.5f);
        if (!tangentPtr->normalizeRS()) {
            *tangentPtr = dir;
        }
        back *= (backLength - LEN) * 0.35f;
        front *= (frontLength - LEN) * 0.35f;
        *velocity += back;
        *velocity += front;
        *velocity += gravity;
        *velocity += ((float)(8 - i) * (0.3f * tangents[0])) / 8.0f;
        float weight = (float)i / 7.0f;
        weight *= weight;
        *velocity += weight * (0.75f * dir);
        *velocity *= 0.75f;
        *point += *velocity;
        mVec3_c stretch = *point;
        stretch -= *previous;
        float length = PSVECMag(stretch);
        if (length > MAX_LEN) {
            stretch *= MAX_LEN / length;
            *point = *previous;
            *point += stretch;
        }
        point++;
        tangentPtr++;
        velocity++;
    }
    mVec3_c *previous = &points[6];
    mVec3_c *last = &points[7];
    mVec3_c back = *previous - *last;
    float backLength = PSVECMag(back);
    if (!back.normalizeRS()) {
        back = -1.0f * dir;
    }
    tangents[7].set(-back.x, -back.y, -back.z);
    back *= (backLength - LEN) * 0.35f;
    mVelocities[7] += back;
    mVelocities[7] += 0.75f * dir;
    mVelocities[7] *= 0.75f;
    *last += mVelocities[7];
    // push the free end away from the other balloons' ends
    for (int i = 0; i < dOrganBalloon_c::BALLOON_NUM; i++) {
        dOrganBalloonString_c *other = organ->mpStrings[i];
        if (other != this) {
            mVec3_c away = *last;
            away -= other->mPoints[7];
            float overlap = 16.0f - PSVECMag(away);
            if (overlap > 0.0f) {
                mVec3_c push;
                if (!away.normalizeRS()) {
                    away.set(0.0f, 0.0f, overlap);
                    push.set(0.0f, 0.0f, 0.1f);
                } else {
                    push = away;
                    push *= 0.1f;
                    away *= overlap;
                }
                *last += away;
                mVelocities[7] += push;
            }
        }
    }
    mVec3_c stretch = *last;
    stretch -= *previous;
    float length = PSVECMag(stretch);
    if (length > MAX_LEN) {
        stretch *= MAX_LEN / length;
        *last = *previous;
        *last += stretch;
    }
    updateRotation(dir);
    mDrawMtx._03 = mTransform._03 + points[6].x * mScale.x;
    mDrawMtx._13 = mTransform._13 + points[6].y * mScale.y;
    mDrawMtx._23 = mTransform._23 + points[6].z * mScale.z;
    return &mDrawMtx;
}
