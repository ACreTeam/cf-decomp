// The flags (REL d_flagNP). See include/game/game/d_flag.hpp.
// REL .text 0xCC..0x17E8, .ctors 0x0..0x4, .rodata 0x0..0x7C, .data
// 0x0..0x110, .bss 0x8..0x24.
#include <game/game/d_flag.hpp>
#include <game/framework/f_profile.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_a_str.hpp>
#include <game/game/d_base.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_dsn.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_light.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_world.hpp>
#include <game/mLib/m_color.hpp>
#include <game/mLib/m_mtx.hpp>
#include <lib/egg/gfx/eggDrawHelper.h>
#include <revolution/GX.h>
#include <revolution/OS.h>

// The distance between two neighbouring points of the cloth.
#define FLAG_POINT_DIST 8.0f
#define FLAG_POINT_DIST_DIAG 11.313708f // FLAG_POINT_DIST * sqrt(2)

// A house flag's pole top, relative to the house's position.
#define FLAG_HOME_POLE_X 33.5f
#define FLAG_HOME_POLE_Y 78.0f
#define FLAG_HOME_POLE_Z 32.0f

// .text:0xCC
namespace dFlag {
BASE_PROFILE(FLAG, dFlag_c);
} // namespace dFlag

// The wind: blends between these two by sin(mWindAngle).
static const nw4r::math::_VEC3 l_windB = {0.6f, 0.0f, 0.0f};
static const nw4r::math::_VEC3 l_windA = {0.6f, 0.0f, 0.0f};

// .text:0xFC
void dFlag_c::drawProc_c::init(const mMtx_c *mtx, const mAng &angle) {
    mMtx = *mtx;
    mBuf = 0;
    mWindAngle = 0;
    f32 s = mWindAngle.sin();
    mVec3_c windA = l_windA;
    mVec3_c windB = l_windB;
    mWind = windA * s + windB * (1.0f - s);
    mVec3_c *pos = mPos[0];
    mVec3_c *nrm = mNrm[0];
    mVec3_c *vel = mVel;
    mVec3_c xStep(FLAG_POINT_DIST, 0.0f, 0.0f);
    mVec3_c yStep(0.0f, -FLAG_POINT_DIST, 0.0f);
    xStep.rotZ(angle);
    yStep.rotZ(angle);
    for (int y = 0; y < FLAG_POINT_Y; y++) {
        for (int x = 0; x < FLAG_POINT_X; x++) {
            *pos = xStep * x + yStep * y;
            nrm->set(0.0f, 0.0f, 1.0f);
            vel->set(0.0f, 0.0f, cM::rndRange(-0.01f, 0.01f));
            mTexCoord[y * FLAG_POINT_X + x][0] = x / (f32)(FLAG_POINT_X - 1);
            mTexCoord[y * FLAG_POINT_X + x][1] = y / (f32)(FLAG_POINT_Y - 1);
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
}

// .text:0x414: the spring between two points (rest: its length).
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
    f *= 0.3f * k;
    *force += f;
}

static inline void calcNormal(mVec3_c &out, const mVec3_c &h,
                              const mVec3_c &v) {
    out.set(h.y * v.z - h.z * v.y, h.z * v.x - h.x * v.z,
            h.x * v.y - h.y * v.x);
}

// .text:0x550: one step of the cloth (copy: take the town flag's).
void dFlag_c::drawProc_c::calc(const mMtx_c *mtx, BOOL copy) {
    mMtx = *mtx;
    int prev = mBuf;
    int cur = prev == 0;
    mBuf = cur;
    if (copy) {
        static const u32 cCopySize = sizeof(mPos[0]);
        dFlag_c *town = s_townFlag;
        mVec3_c *townPos = town->mProc.mPos[town->mProc.mBuf];
        mVec3_c *townNrm = town->mProc.mNrm[town->mProc.mBuf];
        mVec3_c *pos = mPos[cur];
        mVec3_c *nrm = mNrm[cur];
        cLib::memCpy(pos, townPos, cCopySize);
        cLib::memCpy(nrm, townNrm, cCopySize);
        DCStoreRangeNoSync(pos, cCopySize);
        DCStoreRangeNoSync(nrm, cCopySize);
        return;
    }
    mVec3_c *prevPos = mPos[prev];
    mVec3_c *prevNrm = mNrm[prev];
    mVec3_c *pos = mPos[cur];
    mVec3_c *nrm = mNrm[cur];
    f32 s = mWindAngle.sin();
    mVec3_c windA = l_windA;
    mVec3_c windB = l_windB;
    mWind = windA * s + windB * (1.0f - s);
    static const u32 cBufSize = sizeof(mPos[0]);
    cLib::memCpy(pos, prevPos, cBufSize);
    cLib::memCpy(nrm, prevNrm, cBufSize);
    mVec3_c *vel = mVel;
    for (int y = 0; y < FLAG_POINT_Y; y++) {
        for (int x = 0; x < FLAG_POINT_X; x++) {
            int i = y * FLAG_POINT_X + x;
            const mVec3_c *point = &prevPos[i];
            mVec3_c *velocity = &vel[i];
            mVec3_c *normal = &prevNrm[i];
            mVec3_c force = mVec3_c::Zero;
            if (x == 0) {
                continue;
            }
            int xMax = x != FLAG_POINT_X - 1;
            int yMin;
            int yMax;
            if (y == 0) {
                yMin = 0;
                yMax = 1;
            } else if (y == FLAG_POINT_Y - 1) {
                yMin = -1;
                yMax = 0;
            } else {
                yMin = -1;
                yMax = 1;
            }
            int top = yMin * FLAG_POINT_X;
            for (int dy = yMin; dy <= yMax; dy++) {
                for (int dx = -1; dx <= xMax; dx++) {
                    if (dx != 0 || dy != 0) {
                        const mVec3_c *neighbor =
                            &prevPos[i + dy * FLAG_POINT_X + dx];
                        f32 rest;
                        if (dx == 0 || dy == 0) {
                            rest = FLAG_POINT_DIST;
                        } else {
                            rest = FLAG_POINT_DIST_DIAG;
                        }
                        addSpring(&force, point, neighbor, rest);
                    }
                }
            }
            force.y += -0.05f;
            force += *normal * (mWind.x * normal->x + mWind.y * normal->y +
                                mWind.z * normal->z);
            *velocity += force;
            *velocity *= 0.8f;
            pos[i] += *velocity;
            mVec3_c up = prevPos[i + top];
            mVec3_c down = prevPos[i + yMax * FLAG_POINT_X];
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

// .text:0xAA8
void dFlag_c::drawProc_c::setupGX() {
    static const GXColor cMatColor = {0xFF, 0xFF, 0xFF, 0xFF};
    EGG::Graphics::setupGX();
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZMode(TRUE, GX_LEQUAL, TRUE);
    GXSetZCompLoc(TRUE);
    GXColor matColor = *const_cast<GXColor *>(&cMatColor);
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
    GXInitTexObjCI(&texObj, data, width, height, (GXTexFmt)format, GX_CLAMP,
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

static inline void sendPoint(u8 i) {
    GXPosition1x8(i);
    GXNormal1x8(i);
    GXTexCoord1x8(i);
}

// .text:0xDC4
void dFlag_c::drawProc_c::drawCloth() {
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
    for (int y = 0; y < FLAG_POINT_Y - 1; y++) {
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, FLAG_POINT_X * 2);
        int top = y * FLAG_POINT_X;
        int bottom = (int)(y + 1) * FLAG_POINT_X;
        for (int x = 0; x < FLAG_POINT_X; x++) {
            u8 topIndex = top + x;
            u8 bottomIndex = bottom + x;
            sendPoint(bottomIndex);
            sendPoint(topIndex);
        }
    }
}

// .text:0xF88
void dFlag_c::drawProc_c::drawOpa() {
    setupGX();
    drawCloth();
}

// The next flag's position and kind (construct).
static mVec3_c l_createPos(1915.0f, 192.0f, 536.0f);
static int l_createKind = -1;

dFlag_c *dFlag_c::s_townFlag;

// .text:0xFBC
dFlag_c::dFlag_c() : mColArea(NULL), mPos(l_createPos), mKind(l_createKind) {
}

// .text:0x10C0
dFlag_c::~dFlag_c() {
}

// .text:0x119C
fBase_c *dFlag_c::construct(const mVec3_c *pos, int kind, fBase_c *parent) {
    l_createPos = *pos;
    l_createKind = kind;
    return dBase_c::createBase(fProf::FLAG, parent, 0, 0);
}

// .text:0x11DC
int dFlag_c::create() {
    int kind = mKind;
    if (kind == KIND_TOWN) {
        dSaveTown_c *town = dSaveData_c::getTown();
        if (!mLoader.loadDesignInPlace(&town->mBuilding.mTownFlag,
                                       dHeap::localHeap_p, ORG_DESIGN_WIDTH,
                                       ORG_DESIGN_HEIGHT)) {
            return NOT_READY;
        }
    } else {
        if (!mLoader.loadDesignInPlace(
                dSaveData_c::getTown()->mHomes.getHome(kind - KIND_HOME),
                dHeap::localHeap_p, ORG_DESIGN_WIDTH, ORG_DESIGN_HEIGHT)) {
            return NOT_READY;
        }
        if (s_townFlag == NULL) {
            return NOT_READY;
        }
    }
    mVec3_c pos;
    mAng tilt = dWorld::curvePositionSimple(&pos, &mPos);
    mMtx_c mtx;
    PSMTXTrans(mtx, pos.x, pos.y, pos.z);
    mtx.XrotM(tilt);
    if (mKind != KIND_TOWN) {
        mMtx_c offset;
        PSMTXTrans(offset, FLAG_HOME_POLE_X, FLAG_HOME_POLE_Y,
                   FLAG_HOME_POLE_Z);
        PSMTXConcat(mtx, offset, mtx);
        mColArea = static_cast<daSTR_c *>(getConnectParent())->mColArea;
    }
    mAllocator.attach(dHeap::localHeap_p, 4);
    mProc.create(&mAllocator, NULL);
    nw4r::g3d::ResFile file(mLoader.getData());
    file.Init();
    mProc.mResFile = nw4r::g3d::ResFile(mLoader.getData());
    mProc.init(&mtx, mAng(0));
    if (mKind == KIND_TOWN) {
        s_townFlag = this;
    }
    return SUCCEEDED;
}

// .text:0x1394
int dFlag_c::execute() {
    mVec3_c pos;
    mAng tilt = dWorld::curvePositionSimple(&pos, &mPos);
    mMtx_c mtx;
    PSMTXTrans(mtx, pos.x, pos.y, pos.z);
    mtx.XrotM(tilt);
    BOOL copy = FALSE;
    if (mKind != KIND_TOWN) {
        mMtx_c offset;
        PSMTXTrans(offset, FLAG_HOME_POLE_X, FLAG_HOME_POLE_Y,
                   FLAG_HOME_POLE_Z);
        PSMTXConcat(mtx, offset, mtx);
        copy = TRUE;
    }
    mProc.calc(&mtx, copy);
    return SUCCEEDED;
}

// .text:0x1450
int dFlag_c::draw() {
    if (mColArea != NULL) {
        mVec3_c pos = mPos + mColArea->getCenter();
        if (dCamera_c::sInstance->isPastTarget(
                &pos, std::fabs(mColArea->mMax.z - mColArea->mMin.z))) {
            return NOT_READY;
        }
    }
    mProc.setPriorityDraw(0x7F, 0x7F);
    mProc.entry();
    return SUCCEEDED;
}

// .text:0x14FC
int dFlag_c::doDelete() {
    if (!mLoader.release()) {
        return NOT_READY;
    }
    mProc.remove();
    if (mKind == KIND_TOWN) {
        s_townFlag = NULL;
    }
    return SUCCEEDED;
}
