// The New Year's countdown flags (REL d_countdown_flagNP). See
// include/game/game/d_countdown_flag.hpp.
// REL .text 0xCC..0x24BC, .ctors 0x0..0x4, .rodata 0x0..0x90, .data
// 0x0..0x210, .bss 0x8..0x28.
#include <game/game/d_countdown_flag.hpp>
#include <game/framework/f_profile.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_light.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_world.hpp>
#include <game/mLib/m_mtx.hpp>
#include <lib/egg/gfx/eggDrawHelper.h>
#include <revolution/gx.h>
#include <revolution/os.h>

// The distance between two neighbouring points of the cloth.
#define CDFLAG_POINT_DIST 8.0f
#define CDFLAG_POINT_DIST_DIAG 11.313708f // CDFLAG_POINT_DIST * sqrt(2)

// .text:0xCC
namespace dCountdownFlag {
BASE_PROFILE(COUNTDOWN_FLAG, dCountDownFlag_c);

// The wind: blends between these two by sin(mWindAngle).
static const nw4r::math::_VEC3 l_windA = {0.075f, 0.0f, 0.0f};
static const nw4r::math::_VEC3 l_windB = {0.3f, 0.0f, 0.0f};
// The top of the board's flag poles (relative to the board).
static const nw4r::math::_VEC3 l_poleTop = {0.0f, 113.0f, 0.0f};

// .text:0xFC: the spring between two points (rest: its length).
static void addSpring(mVec3_c *force, const mVec3_c *a, const mVec3_c *b,
                      f32 rest) {
    mVec3_c dir = *a;
    dir -= *b;
    f32 len = PSVECMag(dir);
    if (!dir.normalizeRS()) {
        dir.set(0.0f, 0.0f, 1.0f);
    }
    f32 stretch = rest - len;
    f32 k = stretch < 0.5f * -rest  ? 0.5f * -rest
            : stretch > 0.5f * rest ? 0.5f * rest
                                    : stretch;
    mVec3_c f = dir;
    f *= 0.375f * k;
    *force += f;
}

// The cloth's material colour.
static const GXColor l_matColor = {0xFF, 0xFF, 0xFF, 0xFF};

} // namespace dCountdownFlag

using namespace dCountdownFlag;

// .text:0x238
void dCountDownFlag_c::drawProc_c::setupGX() {
    EGG::Graphics::setupGX();
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZMode(TRUE, GX_LEQUAL, TRUE);
    GXSetZCompLoc(TRUE);
    GXColor matColor = *const_cast<GXColor *>(&l_matColor);
    GXSetChanAmbColor(GX_COLOR0A0, dLight::getAmbColor(0));
    GXSetChanMatColor(GX_COLOR0A0, matColor);
    nw4r::g3d::ResTex tex = mResFile.GetResTex(0);
    nw4r::g3d::ResPltt pltt = mResFile.GetResPltt(0);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,
                      FALSE, GX_PTIDENTITY);
    GXSetNumTexGens(1);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, TRUE, GX_SRC_REG, GX_SRC_REG,
                  dLight::getLightMask(), GX_DF_CLAMP, GX_AF_SPOT);
    GXSetNumTevStages(1);
    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE,
                          GX_CH_ALPHA);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC,
                    GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, TRUE,
                    GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, TRUE,
                    GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    void *data;
    u16 width;
    u16 height;
    GXCITexFmt format;
    f32 minLod;
    f32 maxLod;
    u8 mipmap;
    tex.GetTexObjCIParam(&data, &width, &height, &format, &minLod, &maxLod,
                         &mipmap);
    GXTlutObj tlut;
    GXInitTlutObj(&tlut, pltt.GetPlttData(), pltt.GetFmt(),
                  pltt.GetNumEntries());
    GXLoadTlut(&tlut, GX_TLUT0);
    GXTexObj texObj;
    GXInitTexObjCI(&texObj, data, width, height, (GXCITexFmt)format, GX_CLAMP,
                   GX_CLAMP, mipmap, GX_TLUT0);
    GXInitTexObjLOD(&texObj, GX_LINEAR, GX_LINEAR, minLod, maxLod, 0.0f, FALSE,
                    FALSE, GX_ANISO_1);
    GXLoadTexObj(&texObj, GX_TEXMAP0);
    mMtx_c mtx;
    getViewMtx(&mtx);
    PSMTXConcat(mtx, mMtx, mtx);
    GXLoadPosMtxImm(mtx, GX_PNMTX0);
    GXLoadNrmMtxImm(mtx, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
    GXSetCullMode(GX_CULL_NONE);
    GXInvalidateVtxCache();
}

namespace dCountdownFlag {

static inline void sendPoint(u8 i) {
    GXPosition1x8(i);
    GXNormal1x8(i);
    GXTexCoord1x8(i);
}

} // namespace dCountdownFlag

// .text:0x554
void dCountDownFlag_c::drawProc_c::drawCloth() {
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetVtxDesc(GX_VA_NRM, GX_INDEX8);
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);
    GXSetArray(GX_VA_POS, mPos[mBuf], sizeof(mVec3_c));
    GXSetArray(GX_VA_NRM, mNrm[mBuf], sizeof(mVec3_c));
    GXSetArray(GX_VA_TEX0, mTexCoord, sizeof(mTexCoord[0]));
    for (int y = 0; y < CDFLAG_POINT_Y - 1; y++) {
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, CDFLAG_POINT_X * 2);
        int top = y * CDFLAG_POINT_X;
        int bottom = (int)(y + 1) * CDFLAG_POINT_X;
        for (int x = 0; x < CDFLAG_POINT_X; x++) {
            u8 topIndex = top + x;
            u8 bottomIndex = bottom + x;
            sendPoint(bottomIndex);
            sendPoint(topIndex);
        }
        GXEnd();
    }
}

// .text:0x718
void dCountDownFlag_c::drawProc_c::drawOpa() {
    setupGX();
    drawCloth();
}

// .text:0x74C
void dCountDownFlag_c::drawProcCDL_c::init(const mMtx_c *mtx, mAng angle) {
    mAng rot = angle;
    mMtx = *mtx;
    mBuf = 0;
    mWindAngle = 0;
    f32 s = 0.5f + 0.5f * mWindAngle.sin();
    mVec3_c windA = l_windA;
    mVec3_c windB = l_windB;
    mWind = windA * s + windB * (1.0f - s);
    mWind.x = -mWind.x;
    mVec3_c *pos = mPos[0];
    mVec3_c *nrm = mNrm[0];
    mVec3_c *vel = mVel;
    mVec3_c xStep(-CDFLAG_POINT_DIST, 0.0f, 0.0f);
    mVec3_c yStep(0.0f, -CDFLAG_POINT_DIST, 0.0f);
    xStep.rotZ(rot);
    yStep.rotZ(rot);
    for (int y = 0; y < CDFLAG_POINT_Y; y++) {
        for (int x = 0; x < CDFLAG_POINT_X; x++) {
            *pos = xStep * x + yStep * y;
            nrm->set(0.0f, 0.0f, 1.0f);
            vel->set(0.0f, 0.0f, cM::rndRange(-0.01f, 0.01f));
            mTexCoord[y * CDFLAG_POINT_X + x][0] =
                x / (f32)(CDFLAG_POINT_X - 1);
            mTexCoord[y * CDFLAG_POINT_X + x][1] =
                y / (f32)(CDFLAG_POINT_Y - 1);
            pos++;
            nrm++;
            vel++;
        }
    }
    static const u32 cBufSize = sizeof(mPos[0]);
    cLib::memCpy(mPos[1], mPos[0], cBufSize);
    cLib::memCpy(mNrm[1], mNrm[0], cBufSize);
    DCStoreRangeNoSync(mPos, sizeof(mPos));
    DCStoreRangeNoSync(mNrm, sizeof(mNrm));
    for (int i = 0; i < CDFLAG_INIT_STEPS; i++) {
        calc(mtx);
    }
}

namespace dCountdownFlag {

static inline void calcNormal(mVec3_c &out, const mVec3_c &h,
                              const mVec3_c &v) {
    out.set(h.y * v.z - h.z * v.y, h.z * v.x - h.x * v.z,
            h.x * v.y - h.y * v.x);
}

} // namespace dCountdownFlag

// .text:0xA98
void dCountDownFlag_c::drawProcCDL_c::calc(const mMtx_c *mtx) {
    mMtx = *mtx;
    mWindAngle += CDFLAG_WIND_ANGLE_STEP;
    f32 s = 0.5f + 0.5f * mWindAngle.sin();
    mVec3_c windA = l_windA;
    mVec3_c windB = l_windB;
    mWind = windA * s + windB * (1.0f - s);
    int prev = mBuf;
    int cur = prev == 0;
    mVec3_c *prevPos = mPos[prev];
    mVec3_c *prevNrm = mNrm[prev];
    mVec3_c *pos = mPos[cur];
    mVec3_c *nrm = mNrm[cur];
    mWind.x = -mWind.x;
    mBuf = cur;
    static const u32 cBufSize = sizeof(mPos[0]);
    cLib::memCpy(pos, prevPos, cBufSize);
    cLib::memCpy(nrm, prevNrm, cBufSize);
    mVec3_c *vel = mVel;
    for (int y = 0; y < CDFLAG_POINT_Y; y++) {
        for (int x = 0; x < CDFLAG_POINT_X; x++) {
            int i = y * CDFLAG_POINT_X + x;
            const mVec3_c *point = &prevPos[i];
            mVec3_c *velocity = &vel[i];
            mVec3_c *normal = &prevNrm[i];
            mVec3_c force = mVec3_c::Zero;
            if (x == 0) {
                continue;
            }
            int xMax = x != CDFLAG_POINT_X - 1;
            int yMin;
            int yMax;
            if (y == 0) {
                yMin = 0;
                yMax = 1;
            } else if (y == CDFLAG_POINT_Y - 1) {
                yMin = -1;
                yMax = 0;
            } else {
                yMin = -1;
                yMax = 1;
            }
            int top = yMin * CDFLAG_POINT_X;
            for (int dy = yMin; dy <= yMax; dy++) {
                for (int dx = -1; dx <= xMax; dx++) {
                    if (dx != 0 || dy != 0) {
                        const mVec3_c *neighbor =
                            &prevPos[i + dy * CDFLAG_POINT_X + dx];
                        f32 rest;
                        if (dx == 0 || dy == 0) {
                            rest = CDFLAG_POINT_DIST;
                        } else {
                            rest = CDFLAG_POINT_DIST_DIAG;
                        }
                        addSpring(&force, point, neighbor, rest);
                    }
                }
            }
            force.y += -0.125f;
            force += *normal * (mWind.x * normal->x + mWind.y * normal->y +
                                mWind.z * normal->z);
            *velocity += force;
            *velocity *= 0.75f;
            pos[i] += *velocity;
            mVec3_c up = prevPos[i + top];
            mVec3_c down = prevPos[i + yMax * CDFLAG_POINT_X];
            mVec3_c right = prevPos[i + xMax];
            mVec3_c left = prevPos[i - 1];
            mVec3_c v = up - down;
            mVec3_c h = left - right;
            calcNormal(nrm[i], h, v);
            nrm[i].normalizeRS();
        }
    }
    DCStoreRangeNoSync(pos, cBufSize);
    DCStoreRangeNoSync(nrm, cBufSize);
}

// .text:0xFA0
void dCountDownFlag_c::drawProcCDR_c::init(const mMtx_c *mtx, mAng angle) {
    mAng rot = angle;
    mMtx = *mtx;
    mBuf = 0;
    mWindAngle = 0;
    f32 s = 0.5f + 0.5f * -mWindAngle.sin();
    mVec3_c windA = l_windA;
    mVec3_c windB = l_windB;
    mWind = windA * s + windB * (1.0f - s);
    mVec3_c *pos = mPos[0];
    mVec3_c *nrm = mNrm[0];
    mVec3_c *vel = mVel;
    mVec3_c xStep(CDFLAG_POINT_DIST, 0.0f, 0.0f);
    mVec3_c yStep(0.0f, -CDFLAG_POINT_DIST, 0.0f);
    xStep.rotZ(rot);
    yStep.rotZ(rot);
    for (int y = 0; y < CDFLAG_POINT_Y; y++) {
        for (int x = 0; x < CDFLAG_POINT_X; x++) {
            *pos = xStep * x + yStep * y;
            nrm->set(0.0f, 0.0f, 1.0f);
            vel->set(0.0f, 0.0f, cM::rndRange(-0.01f, 0.01f));
            mTexCoord[y * CDFLAG_POINT_X + x][0] =
                x / (f32)(CDFLAG_POINT_X - 1);
            mTexCoord[y * CDFLAG_POINT_X + x][1] =
                y / (f32)(CDFLAG_POINT_Y - 1);
            pos++;
            nrm++;
            vel++;
        }
    }
    static const u32 cBufSize = sizeof(mPos[0]);
    cLib::memCpy(mPos[1], mPos[0], cBufSize);
    cLib::memCpy(mNrm[1], mNrm[0], cBufSize);
    DCStoreRangeNoSync(mPos, sizeof(mPos));
    DCStoreRangeNoSync(mNrm, sizeof(mNrm));
    for (int i = 0; i < CDFLAG_INIT_STEPS; i++) {
        calc(mtx);
    }
}

// .text:0x12EC
void dCountDownFlag_c::drawProcCDR_c::calc(const mMtx_c *mtx) {
    mMtx = *mtx;
    mWindAngle += CDFLAG_WIND_ANGLE_STEP;
    f32 s = 0.5f + 0.5f * -mWindAngle.sin();
    mVec3_c windA = l_windA;
    mVec3_c windB = l_windB;
    mWind = windA * s + windB * (1.0f - s);
    int prev = mBuf;
    int cur = prev == 0;
    mVec3_c *prevPos = mPos[prev];
    mVec3_c *prevNrm = mNrm[prev];
    mVec3_c *pos = mPos[cur];
    mVec3_c *nrm = mNrm[cur];
    mBuf = cur;
    static const u32 cBufSize = sizeof(mPos[0]);
    cLib::memCpy(pos, prevPos, cBufSize);
    cLib::memCpy(nrm, prevNrm, cBufSize);
    mVec3_c *vel = mVel;
    for (int y = 0; y < CDFLAG_POINT_Y; y++) {
        for (int x = 0; x < CDFLAG_POINT_X; x++) {
            int i = y * CDFLAG_POINT_X + x;
            const mVec3_c *point = &prevPos[i];
            mVec3_c *velocity = &vel[i];
            mVec3_c *normal = &prevNrm[i];
            mVec3_c force = mVec3_c::Zero;
            if (x == 0) {
                continue;
            }
            int xMax = x != CDFLAG_POINT_X - 1;
            int yMin;
            int yMax;
            if (y == 0) {
                yMin = 0;
                yMax = 1;
            } else if (y == CDFLAG_POINT_Y - 1) {
                yMin = -1;
                yMax = 0;
            } else {
                yMin = -1;
                yMax = 1;
            }
            int top = yMin * CDFLAG_POINT_X;
            for (int dy = yMin; dy <= yMax; dy++) {
                for (int dx = -1; dx <= xMax; dx++) {
                    if (dx != 0 || dy != 0) {
                        const mVec3_c *neighbor =
                            &prevPos[i + dy * CDFLAG_POINT_X + dx];
                        f32 rest;
                        if (dx == 0 || dy == 0) {
                            rest = CDFLAG_POINT_DIST;
                        } else {
                            rest = CDFLAG_POINT_DIST_DIAG;
                        }
                        addSpring(&force, point, neighbor, rest);
                    }
                }
            }
            force.y += -0.125f;
            force += *normal * (mWind.x * normal->x + mWind.y * normal->y +
                                mWind.z * normal->z);
            *velocity += force;
            *velocity *= 0.75f;
            pos[i] += *velocity;
            mVec3_c up = prevPos[i + top];
            mVec3_c down = prevPos[i + yMax * CDFLAG_POINT_X];
            mVec3_c left = prevPos[i - 1];
            mVec3_c right = prevPos[i + xMax];
            mVec3_c v = up - down;
            mVec3_c h = right - left;
            calcNormal(nrm[i], h, v);
            nrm[i].normalizeRS();
        }
    }
    DCStoreRangeNoSync(pos, cBufSize);
    DCStoreRangeNoSync(nrm, cBufSize);
}

namespace dCountdownFlag {

// The flags' cloth rotation (init).
static mAng l_angleL(CDFLAG_ANGLE);
static mAng l_angleR(-CDFLAG_ANGLE);
// (Unused: d_flag's l_createPos.)
static mVec3_c l_createPos(1915.0f, 192.0f, 536.0f);

} // namespace dCountdownFlag

// .text:0x17F0
dCountDownFlag_c::dCountDownFlag_c() {
}

// .text:0x1928
dCountDownFlag_c::~dCountDownFlag_c() {
}

// .text:0x1A64
int dCountDownFlag_c::create() {
    dSaveTown_c *town = dSaveData_c::getTown();
    if (!mLoader.loadDesignInPlace(&town->mBuilding.mTownFlag,
                                   dHeap::localHeap_p, ORG_DESIGN_WIDTH,
                                   ORG_DESIGN_HEIGHT)) {
        return NOT_READY;
    }
    dActor_c *parent = static_cast<dActor_c *>(getConnectParent());
    if (parent != NULL) {
        mPos = parent->getPos();
        mColArea = dSTR::getColBank()->getColArea(
            dSTR::getBuildingResName(BUILDING_INDEX(BUILDING_COUNTDOWN_BOARD)));
    } else {
        mPos.set(0.0f, 0.0f, 0.0f);
    }
    mVec3_c pos;
    mVec3_c base = mPos;
    mAng tilt = dWorld::curvePositionSimple(&pos, &base);
    mMtx_c mtx;
    PSMTXTrans(mtx, pos.x, pos.y, pos.z);
    mtx.XrotM(tilt);
    mAllocator.attach(dHeap::localHeap_p, 4);
    nw4r::g3d::ResFile file(mLoader.getData());
    file.Init();
    mVec3_c offsetL(l_poleTop.x - CDFLAG_POLE_DIST_X, l_poleTop.y,
                    CDFLAG_POLE_L_Z + l_poleTop.z);
    mMtx_c mtxL;
    mtxL = mtx;
    mtxL.concat(mMtx_c::createTrans(offsetL));
    mProcL.create(&mAllocator, NULL);
    mProcL.mResFile = nw4r::g3d::ResFile(mLoader.getData());
    mProcL.init(&mtxL, l_angleL);
    mVec3_c offsetR(CDFLAG_POLE_DIST_X + l_poleTop.x, l_poleTop.y,
                    CDFLAG_POLE_R_Z + l_poleTop.z);
    mMtx_c mtxR;
    mtxR = mtx;
    mtxR.concat(mMtx_c::createTrans(offsetR));
    mProcR.create(&mAllocator, NULL);
    mProcR.mResFile = nw4r::g3d::ResFile(mLoader.getData());
    mProcR.init(&mtxR, l_angleR);
    return SUCCEEDED;
}

// .text:0x1D80
int dCountDownFlag_c::execute() {
    mVec3_c pos;
    mVec3_c base = mPos;
    mAng tilt = dWorld::curvePositionSimple(&pos, &base);
    mMtx_c mtx;
    PSMTXTrans(mtx, pos.x, pos.y, pos.z);
    mtx.XrotM(tilt);
    mVec3_c offsetL(l_poleTop.x - CDFLAG_POLE_DIST_X, l_poleTop.y,
                    CDFLAG_POLE_L_Z + l_poleTop.z);
    mMtx_c mtxL;
    mtxL = mtx;
    mtxL.concat(mMtx_c::createTrans(offsetL));
    mProcL.calc(&mtxL);
    mVec3_c offsetR(CDFLAG_POLE_DIST_X + l_poleTop.x, l_poleTop.y,
                    CDFLAG_POLE_R_Z + l_poleTop.z);
    mMtx_c mtxR;
    mtxR = mtx;
    mtxR.concat(mMtx_c::createTrans(offsetR));
    mProcR.calc(&mtxR);
    return SUCCEEDED;
}

// .text:0x1F9C
int dCountDownFlag_c::draw() {
    mVec3_c pos = mPos + mColArea->getCenter();
    if (dCamera_c::sInstance->isPastTarget(
            &pos, std::fabs(mColArea->mMax.z - mColArea->mMin.z))) {
        return NOT_READY;
    }
    mProcL.setPriorityDraw(0x7F, 0x7F);
    mProcL.entry();
    mProcR.setPriorityDraw(0x7F, 0x7F);
    mProcR.entry();
    return SUCCEEDED;
}

// .text:0x2064
int dCountDownFlag_c::doDelete() {
    if (!mLoader.release()) {
        return NOT_READY;
    }
    mProcL.remove();
    mProcR.remove();
    return SUCCEEDED;
}
