// d_balloon_string.cpp: the string trailing a released balloon.
// .text 80066018..80068474, .data 804A51A8..804A5238,
// .sdata 80749BD8..80749BE8, .sdata2 80750370..807503D0.
#include <game/game/d_balloon_string.hpp>
#include <game/game/d_light.hpp>
#include <nw4r/g3d.h>

extern "C" BOOL fn_802B41E4(EGG::Quatf *out, const Vec *from, const Vec *to);
namespace dBalloonString {
static const GXColor lMaterial = {160, 160, 192, 255};

void multiplyQuaternion(EGG::Quatf *out, const EGG::Quatf *a, const EGG::Quatf *b);
}

// 80066018
dBalloonString_c::~dBalloonString_c() {}

// 800660D0
bool dBalloonString_c::createProc() {
    return m3d::proc_c::create(&mAllocator, NULL);
}

// 800660DC
void dBalloonString_c::setInitialDirection(const mVec3_c &dir) {
    mMtx_c scale;
    mVec3_c up;
    up.set(0.0f, 1.0f, 0.0f);
    mVec3_c direction(dir);
    mRotation.set(0.0f, 0.0f, 0.0f, 1.0f);
    if (!fn_802B41E4(&mRotation, up, direction)) {
        mRotation.set(0.0f, 1.0f, 0.0f, 0.0f);
    }
    mVec3_c axis(1.0f, 0.0f, 0.0f);
    mVec3_c cross(axis.y * direction.z - axis.z * direction.y,
                 axis.z * direction.x - axis.x * direction.z,
                 axis.x * direction.y - axis.y * direction.x);
    fn_80444FE4(&mDrawMtx, &mRotation);
    PSMTXScale(scale, mScale.x, mScale.y, mScale.z);
    PSMTXConcat(mDrawMtx, scale, mDrawMtx);
}

// 800661EC
void dBalloonString_c::updateRotation(const mVec3_c &dir) {
    Mtx scale;
    mVec3_c direction(dir);
    mVec3_c up(0.0f, 1.0f, 0.0f);
    EGG::Quatf fromTangent, fromUp, product, copy;
    fn_804456B0(&fromTangent, &direction, &mTangents[6]);
    if (!fn_802B41E4(&fromUp, up, direction)) {
        fromUp.set(0.0f, 1.0f, 0.0f, 0.0f);
    }
    dBalloonString::multiplyQuaternion(&product, &fromTangent, &fromUp);
    struct Bits {
        u32 x, y, z;
    };
    *(Bits *)&copy = *(const Bits *)&product;
    copy.w = product.w;
    fromTangent.x = copy.x;
    fromTangent.y = copy.y;
    fromTangent.z = copy.z;
    fromTangent.w = copy.w;
    mRotation.slerpTo(fromTangent, 0.12f, mRotation);
    mRotation.normalise();
    if (mRotation.w < 0.0f) {
        mRotation.w *= -1.0f;
        mRotation.x *= -1.0f;
        mRotation.y *= -1.0f;
        mRotation.z *= -1.0f;
    }
    fn_80444FE4(&mDrawMtx, &mRotation);
    PSMTXScale(scale, mScale.x, mScale.y, mScale.z);
    PSMTXConcat(mDrawMtx, scale, mDrawMtx);
}

// 80066358
const mMtx_c *dBalloonString_c::calcInit(const mMtx_c *mtx) {
    mTransform = *mtx;
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
    mVec3_c step = 3.5f * dir;
    for (int i = 0; i < 8; i++) {
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

// 80066670
const mMtx_c *dBalloonString_c::calcHold(const mMtx_c *mtx) {
    mVec3_c x(mtx->_00, mtx->_10, mtx->_20);
    mVec3_c y(mtx->_01, mtx->_11, mtx->_21);
    mVec3_c z(mtx->_02, mtx->_12, mtx->_22);
    mScale.set(PSVECMag(x), PSVECMag(y), PSVECMag(z));
    mVec3_c delta(mtx->_03 - mTransform._03, mtx->_13 - mTransform._13, mtx->_23 - mTransform._23);
    mTransform = *mtx;
    mVec3_c *points = mPoints;
    mVec3_c *tangents = mTangents;
    mVec3_c dir(0.0f, mtx->_13, mtx->_23);
    if (!dir.normalizeRS()) {
        dir.set(0.0f, 1.0f, 0.0f);
    }
    mVec3_c gravity = -0.075f * dir;
    points[0] = mVec3_c::Zero;
    mVec3_c tangent(0.0f, 0.0f, 1.0f);
    tangent.rotX(mAng(0x3874));
    tangent.rotY(mAng(-10732));
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
        back *= (backLength - 3.5f) * 0.35f;
        front *= (frontLength - 3.5f) * 0.35f;
        *velocity += back;
        *velocity += front;
        *velocity += gravity;
        *velocity += ((float)(8 - i) * (0.05f * tangents[0])) / 8.0f;
        float weight = (float)i / 7.0f;
        weight *= weight;
        *velocity += weight * (0.75f * dir);
        *velocity *= 0.75f;
        *point += *velocity;
        mVec3_c stretch = *point;
        stretch -= *previous;
        float length = PSVECMag(stretch);
        if (length > 3.5f * 1.2f) {
            stretch *= (3.5f * 1.2f) / length;
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
    back *= (backLength - 3.5f) * 0.35f;
    mVelocities[7] += back;
    mVelocities[7] += 0.75f * dir;
    mVelocities[7] *= 0.75f;
    *last += mVelocities[7];
    mVec3_c stretch = *last;
    stretch -= *previous;
    float length = PSVECMag(stretch);
    if (length > 3.5f * 1.2f) {
        stretch *= (3.5f * 1.2f) / length;
        *last = *previous;
        *last += stretch;
    }
    updateRotation(dir);
    mDrawMtx._03 = mTransform._03 + points[6].x * mScale.x;
    mDrawMtx._13 = mTransform._13 + points[6].y * mScale.y;
    mDrawMtx._23 = mTransform._23 + points[6].z * mScale.z;
    return &mDrawMtx;
}

namespace dBalloonString {
static inline void cross(mVec3_c &out, const mVec3_c &a, const mVec3_c &b) {
    out.set(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
} // namespace dBalloonString

// 8006706C
const mMtx_c *dBalloonString_c::calcFly(const mMtx_c *) {
    sLib::calcTimer(&mTimer);
    if (mTimer < 30) {
        mAlpha = (mTimer * 255.0f) / 30.0f;
    }
    mVec3_c *points = mPoints;
    mVec3_c dir(0.0f, mTransform._13, mTransform._23);
    if (!dir.normalizeRS()) {
        dir.set(0.0f, 1.0f, 0.0f);
    }
    mVec3_c cross;
    dBalloonString::cross(cross, mVec3_c(1.0f, 0.0f, 0.0f), dir);
    mMtx_c rotation;
    rotation._00 = 1.0f;
    rotation._10 = 0.0f;
    rotation._20 = 0.0f;
    rotation._01 = dir.x;
    rotation._11 = dir.y;
    rotation._21 = dir.z;
    rotation._02 = cross.x;
    rotation._12 = cross.y;
    rotation._22 = cross.z;
    rotation._03 = 0.0f;
    rotation._13 = 0.0f;
    rotation._23 = 0.0f;
    rotation.YrotM(mReleaseAngle);
    mVec3_c wind(0.0f, 0.0f, 1.0f);
    PSMTXMultVecSR(rotation, wind, wind);
    float weight = 0.0f;
    int elapsed = 180 - mTimer;
    if (elapsed < 60) {
        weight = (float)(60 - elapsed) / 60.0f;
    }
    mVec3_c gravity = -0.075f * dir;
    mVec3_c *point;
    mVec3_c *tangentPtr;
    mVec3_c *velocity;
    mVec3_c *last = &points[7];
    mVec3_c back = points[6] - *last;
    float backLength = PSVECMag(back);
    if (!back.normalizeRS()) {
        back = -1.0f * dir;
    }
    mTangents[7].set(-back.x, -back.y, -back.z);
    back *= (backLength - 3.5f) * 0.35f;
    mVelocities[7] += back;
    mVelocities[7] += 0.5f * dir;
    mVelocities[7] += weight * (0.6f * wind);
    mVelocities[7] *= 0.75f;
    *last += mVelocities[7];
    point = &points[6];
    tangentPtr = &mTangents[6];
    velocity = &mVelocities[6];
    for (int i = 6; i > 0; i--) {
        mVec3_c *next = &points[i + 1];
        mVec3_c back = points[i - 1] - *point;
        float backLength = PSVECMag(back);
        mVec3_c front = *next - *point;
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
        back *= (backLength - 3.5f) * 0.35f;
        front *= (frontLength - 3.5f) * 0.35f;
        weight *= 0.8f;
        *velocity += back;
        *velocity += front;
        *velocity += gravity;
        *velocity += weight * (0.6f * wind);
        *velocity *= 0.75f;
        *point += *velocity;
        mVec3_c stretch = *point;
        stretch -= *next;
        float length = PSVECMag(stretch);
        if (length > 3.5f * 1.2f) {
            stretch *= (3.5f * 1.2f) / length;
            *point = *next;
            *point += stretch;
        }
        point--;
        tangentPtr--;
        velocity--;
    }
    mVec3_c *next = &points[1];
    mVec3_c front = *next - points[0];
    float frontLength = PSVECMag(front);
    if (!front.normalizeRS()) {
        front = -1.0f * dir;
    }
    mTangents[0].set(front.x, front.y, front.z);
    front *= (frontLength - 3.5f) * 0.35f;
    mVelocities[0] += front;
    mVelocities[0] += gravity;
    mVelocities[0] *= 0.75f;
    mVec3_c stretch = points[0];
    stretch -= *next;
    float length = PSVECMag(stretch);
    if (length > 3.5f * 1.2f) {
        stretch *= (3.5f * 1.2f) / length;
        points[0] = *next;
        points[0] += stretch;
    }
    mVec3_c delta = points[0];
    for (int i = 0; i < 8; i++) {
        points[i] -= delta;
    }
    mTransform._03 += delta.x;
    mTransform._13 += delta.y;
    mTransform._23 += delta.z;
    updateRotation(dir);
    mDrawMtx._03 = mTransform._03 + points[6].x * mScale.x;
    mDrawMtx._13 = mTransform._13 + points[6].y * mScale.y;
    mDrawMtx._23 = mTransform._23 + points[6].z * mScale.z;
    return &mDrawMtx;
}

// 80067BA0
const mMtx_c *dBalloonString_c::calc(const mMtx_c *mtx) {
    if (mInit) {
        if (mFly) {
            return calcFly(mtx);
        }
        return calcHold(mtx);
    }
    mInit = true;
    return calcInit(mtx);
}

// 80067BF0
void dBalloonString_c::release(mAng angle) {
    mFly = true;
    mReleaseAngle = angle;
    mTimer = 180;
}

// 80067C0C
bool dBalloonString_c::entry() {
    if (mInit) {
        return m3d::scnLeaf_c::entry();
    }
    return false;
}

// 80067C24
void dBalloonString_c::drawXlu() {
    m3d::resetMaterial();
    nw4r::g3d::G3DState::Invalidate(0x7FF);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetZCompLoc(GX_TRUE);
    GXColor color = {255, 255, 255, mAlpha};
    GXSetTevColor(GX_TEVREG0, color);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_TRUE, GX_SRC_REG, GX_SRC_REG, fn_800CD250(), GX_DF_NONE, GX_AF_SPOT);
    GXSetChanAmbColor(GX_COLOR0A0, fn_800CD168(0));
    GXSetChanMatColor(GX_COLOR0A0, dBalloonString::lMaterial);
    GXSetNumTexGens(0);
    GXSetNumIndStages(0);
    GXSetNumTevStages(1);
    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetCullMode(GX_CULL_NONE);
    mMtx_c view, normal;
    getViewMtx(&view);
    getViewMtx(&normal);
    PSMTXInvXpose(normal, normal);
    mMtx_c model = view;
    model.concat(mMtx_c::createTrans(mTransform._03, mTransform._13, mTransform._23));
    model.concat(mMtx_c::createScale(mScale));
    GXLoadPosMtxImm(model, GX_PNMTX0);
    GXLoadNrmMtxImm(normal, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 14);
    mVec3_c *point = mPoints;
    mVec3_c *tangentPtr = mTangents;
    for (int i = 0; i < 7; i++) {
        mVec3_c right = *point;
        mVec3_c left = right;
        mVec3_c tangent = *tangentPtr;
        mVec3_c facing(view._20, view._21, view._22);
        mVec3_c edge(tangent.y * facing.z - tangent.z * facing.y,
                     tangent.z * facing.x - tangent.x * facing.z,
                     tangent.x * facing.y - tangent.y * facing.x);
        if (!edge.normalizeRS()) {
            edge.set(1.0f, 0.0f, 0.0f);
        }
        float width = 0.25f;
        edge *= width;
        left -= edge;
        right += edge;
        GXPosition3f32(left.x, left.y, left.z);
        GXPosition3f32(right.x, right.y, right.z);
        point++;
        tangentPtr++;
    }
    GXEnd();
}

// 800680F4
void dBalloonString_c::destroy() {
    this->~dBalloonString_c();
}

// 80068108
dBalloonString_c *dBalloonString_c::create(EGG::Heap *heap) {
    dBalloonString_c *balloon = new (heap, 4) dBalloonString_c();
    balloon->mAllocator.attach(heap, 4);
    balloon->createProc();
    return balloon;
}

namespace dBalloonString {

// 800681FC: vector cross product, also used by m_quat.cpp.
void crossVector(Vec *out, const Vec *a, const Vec *b) {
    out->x = a->y * b->z - a->z * b->y;
    out->y = a->z * b->x - a->x * b->z;
    out->z = a->x * b->y - a->y * b->x;
}

// 80068248
void scaleVector(Vec *out, const Vec *a, float scale) {
    float z = a->z * scale;
    float y = a->y * scale;
    float x = a->x * scale;
    out->z = z;
    out->x = x;
    out->y = y;
}

// 80068270
void addVector(Vec *out, const Vec *a, const Vec *b) {
    out->z = a->z + b->z;
    out->y = a->y + b->y;
    out->x = a->x + b->x;
}

void setQuaternion(EGG::Quatf *out, const Vec *xyz, float w);

// 800682A4: Hamilton product of two quaternions.
void multiplyQuaternion(EGG::Quatf *out, const EGG::Quatf *a, const EGG::Quatf *b) {
    Vec cross;
    Vec scaledB;
    Vec sum;
    Vec scaledA;
    Vec xyz;
    crossVector(&cross, (const Vec *)a, (const Vec *)b);
    scaleVector(&scaledB, (const Vec *)b, a->w);
    addVector(&sum, &cross, &scaledB);
    scaleVector(&scaledA, (const Vec *)a, b->w);
    addVector(&xyz, &sum, &scaledA);
    float dot = a->x * b->x + a->y * b->y + a->z * b->z;
    float w = a->w * b->w - dot;
    setQuaternion(out, &xyz, w);
}

// 80068374
void setQuaternion(EGG::Quatf *out, const Vec *xyz, float w) {
    struct VectorBits {
        u32 x, y, z;
    };
    *(VectorBits *)out = *(const VectorBits *)xyz;
    out->w = w;
}



} // namespace dBalloonString
