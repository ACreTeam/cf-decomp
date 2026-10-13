#include <game/game/d_shadow.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_bg.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_environment.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_world.hpp>
#include <game/mLib/m_mtx.hpp>
#include <lib/egg/gfx/eggDrawHelper.h>
#include <nw4r/g3d/g3d_state.h>
#include <revolution/MTX.h>
#include <lib/MSL/math.h>
#include <stddef.h>

#include <game/game/d_shadow_internal.hpp>

namespace dShadow {
static const char *sTextureNames[TEXTURE_COUNT] = {
    "sh_circle", "sh_default", "sh_treeA", "sh_treeB", "sh_treeC",
    "sh_treeA_cut", "sh_treeB_cut", "sh_treeC_cut", "sh_stone"
};
static dShadow_hostIO_c sHostIO;
static f32 sProjectionScale = 14.5f;
static f32 sSunTime;

} // namespace dShadow

// 801C6D54
fBase_c *dShadow_c::build() { return new dShadow_c; }
inline dShadow_c::dShadow_c() {}
namespace dShadow {
// 801C6F24
static void setupProjection(const mVec3_c *pos, const mMtx_c *view, const Projection_s *projection) {
    GXLoadPosMtxImm(view->mtx, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
    mMtx_c texMtx;
    f32 x = sProjectionScale * (projection->mSizeX / 16.0f);
    f32 z = sProjectionScale * (projection->mSizeZ / 16.0f);
    C_MTXLightOrtho(texMtx, z, -z, -x, x, 0.5f, -0.5f, 0.5f, 0.5f);
    mVec3_c up(0.0f, 0.0f, -1.0f);
    mVec3_c target(0.0f, 0.0f, 0.0f);
    mVec3_c eye(0.0f, 200.0f, 0.0f);
    target += *pos;
    eye += *pos;
    mVec3_c curvedTarget, curvedEye;
    mAng curve = dWorld::curvePositionSimple(&curvedTarget, &target);
    dWorld::curvePositionSimple(&curvedEye, &eye);
    up.rotX(curve);
    s16 angle = -projection->mAngle.mAngle;
    if (angle != 0 && projection->mSizeX != projection->mSizeZ) texMtx.ZrotM(mAng(angle));
    mMtx_c lightView;
    C_MTXLookAt(lightView.mtx, (const Vec *)&curvedEye, (const Vec *)&up, (const Vec *)&curvedTarget);
    PSMTXConcat(texMtx, lightView, texMtx);
    GXSetNumIndStages(0);
    GXSetNumChans(0);
    GXSetCullMode(GX_CULL_BACK);
    GXSetDither(GX_TRUE);
    GXSetTevColor(GX_TEVREG0, projection->mColor);
    GXSetNumTexGens(1);
    GXLoadTexMtxImm(texMtx.mtx, GX_TEXMTX0, GX_MTX_3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetNumTevStages(1);
    GXSetNumIndStages(0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetZCompLoc(GX_FALSE);
    if (projection->mDepthTest) GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    else GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_GREATER, 0);
    GXTexObj tex;
    void *image; u16 width, height; GXTexFmt format; f32 minLod, maxLod; u8 mipmap;
    projection->mTexture.GetTexObjParam(&image, &width, &height, &format, &minLod, &maxLod, &mipmap);
    GXInitTexObj(&tex, image, width, height, format, GX_CLAMP, GX_CLAMP, mipmap);
    GXInitTexObjLOD(&tex, GX_NEAR, GX_NEAR, minLod, maxLod, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(&tex, GX_TEXMAP0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
}
// 801C728C
f32 getDistanceFade(const mVec3_c *pos) {
    if (lbl_8074E9B0) {
        mVec3_c center(lbl_80623FEC);
        f32 distance = center.z - pos->z;
        if (distance > 0.0f) {
            if (distance > 256.0f) return 0.0f;
            return (256.0f - distance) / 256.0f;
        }
    }
    return 1.0f;
}
// 801C72FC
f32 getAlpha(const mVec3_c *pos) {
    f32 alpha = dEnvironment_c::getShadowAlpha();
    if (0.0f != alpha) return alpha * getDistanceFade(pos);
    return 1.0f;
}
}

// 801C7374
BOOL dShadow_c::addRound(const mVec3_c *pos, u32 flags, mAng angle,
                         f32 sizeX, f32 scale, f32 alphaScale, f32 sizeZ, f32 heightScale) {
    if (mRoundCount < dShadow::ROUND_CAPACITY) {
        f32 height = pos->y - dBGCF::getGroundY(pos, 0);
        f32 sizeScale = 1.0f;
        if (!(flags & dShadow::NO_HEIGHT_SCALE)) {
            if (height >= 0.0f) {
                if (height > 192.0f) height = 192.0f;
                sizeScale = scale * ((192.0f - height * heightScale) / 192.0f);
            }
        } else sizeScale *= scale;
        if (0.0f != sizeScale) {
            u8 alpha = u8(int(alphaScale * dShadow::getAlpha(pos)));
            if (u8(alpha) >= 1) {
                dShadow::Round_c &shadow = mRounds[mRoundCount];
                shadow.mPos = *pos;
                shadow.mSizeX = sizeX * sizeScale;
                shadow.mSizeZ = sizeZ * sizeScale;
                shadow.mAlpha = u8(alpha);
                shadow.mAngle = angle;
                shadow.mFlags = flags;
                ++mRoundCount;
                return TRUE;
            }
        }
    }
    return FALSE;
}
// 801C7528
BOOL dShadow_c::addTexture(const mVec3_c *pos, int texture, f32 width, f32 length, f32 texS0, f32 texS1) {
    if (mTextureCount < dShadow::TEXTURE_CAPACITY) {
        u8 alpha = u8(int(dShadow::getAlpha(pos)));
        if (u8(alpha) >= 1) {
            dShadow::Texture_c &shadow = mTextureShadows[mTextureCount];
            shadow.mPos = *pos;
            shadow.mTexture = texture;
            shadow.mWidth = width;
            shadow.mLength = length;
            shadow.mTexS0 = texS0;
            shadow.mTexS1 = texS1;
            shadow.mAlpha = u8(alpha);
            ++mTextureCount;
            return TRUE;
        }
    }
    return FALSE;
}
// 801C7638
void dShadow_c::drawPass(u32 pass) {
    EGG::Graphics::setupGX();
    mShear = -((dShadow::sSunTime - 0.5f) * 2.0f);
    if (mReady) {
        static mMtx_c view ALIGN(32);
        getViewMtx(&view);
        if (mRoundCount) {
            nw4r::g3d::ResTex texture = mTextures[dShadow::CIRCLE];
            for (dShadow::Round_c *p = mRounds; p < mRounds + mRoundCount; ++p) {
                if ((p->mFlags & dShadow::DRAW_OPA) != pass) {
                    mColor color(dShadow::sHostIO.mColor.r, dShadow::sHostIO.mColor.g,
                                 dShadow::sHostIO.mColor.b, p->mAlpha);
                    if (p->mFlags & dShadow::DRAW_QUAD) {
                        mVec3_c pos = p->mPos;
                        pos.y += mHeight;
                        f32 size = p->mSizeX > p->mSizeZ ? p->mSizeX : p->mSizeZ;
                        f32 quadSize = 1.0f + size;
                        if (quadSize >= 1.0f) {
                            dShadow::Projection_s projection;
                            projection.mSizeX = p->mSizeX;
                            projection.mSizeZ = p->mSizeZ;
                            projection.mTexture = texture;
                            projection.mColor = color;
                            projection.mAngle = p->mAngle;
                            projection.mDepthTest = !(p->mFlags & dShadow::NO_DEPTH_TEST);
                            dShadow::setupProjection(&pos, &view, &projection);
                            drawQuad(&pos, quadSize);
                        }
                    } else {
                        dShadow::Projection_s projection;
                        projection.mSizeX = p->mSizeX;
                        projection.mSizeZ = p->mSizeZ;
                        projection.mTexture = texture;
                        projection.mColor = color;
                        projection.mAngle = p->mAngle;
                        projection.mDepthTest = !(p->mFlags & dShadow::NO_DEPTH_TEST);
                        shadowDtcb_c callback(p->mPos, &view, projection, mHeight, mRaiseWater);
                        mVec3_c min = p->mPos;
                        mVec3_c max = p->mPos;
                        min.x -= p->mSizeX; min.z -= p->mSizeX;
                        max.x += p->mSizeX; max.z += p->mSizeX;
                        dBGCF::check(&min, &max, &callback, dBGCF::CHECK_NO_MVBG | dBGCF::CHECK_FLOOR, 2, 0);
                    }
                }
            }
        }
        if (pass != 0 && mTextureCount) {
            GXLoadPosMtxImm(view.mtx, GX_PNMTX0);
            GXSetCurrentMtx(GX_PNMTX0);
            for (dShadow::Texture_c *p = mTextureShadows; p < mTextureShadows + mTextureCount; ++p) {
                mColor color(dShadow::sHostIO.mColor.r, dShadow::sHostIO.mColor.g,
                                     dShadow::sHostIO.mColor.b, p->mAlpha);
                drawTexture(p, color);
            }
        }
    }
    if (pass == 0) { mRoundCount = 0; mTextureCount = 0; }
    nw4r::g3d::G3DState::Invalidate(0x7FF);
}
// 801C7A14
void dShadow_c::drawQuad(const mVec3_c *pos, f32 size) {
    Vec corners[4] = {
        {pos->x + size, pos->y, pos->z - size},
        {pos->x + size, pos->y, pos->z + size},
        {pos->x - size, pos->y, pos->z + size},
        {pos->x - size, pos->y, pos->z - size}
    };
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    for (Vec *p = corners; p < corners + 4; ++p) {
        mVec3_c field(*p), curved;
        dWorld::curvePositionSimple(&curved, &field);
        GXPosition3f32(curved.x, curved.y, curved.z);
    }
}
// 801C7AFC
void dShadow_c::drawTexture(const dShadow::Texture_c *shadow, mColor color) {
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetNumTexGens(1);
    GXSetCullMode(GX_CULL_NONE);
    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    GXSetNumIndStages(0);
    GXSetTevColor(GX_TEVREG0, color);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetZCompLoc(GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_GREATER, 0);
    GXTexObj tex;
    void *image; u16 width, height; GXTexFmt format; f32 minLod, maxLod; u8 mipmap;
    mTextures[shadow->mTexture].GetTexObjParam(&image, &width, &height, &format, &minLod, &maxLod, &mipmap);
    GXInitTexObj(&tex, image, width, height, format, GX_REPEAT, GX_CLAMP, mipmap);
    GXInitTexObjLOD(&tex, GX_NEAR, GX_NEAR, minLod, maxLod, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(&tex, GX_TEXMAP0);
    mVec3_c pos = shadow->mPos;
    pos.y += 1.0f;
    f32 halfWidth = shadow->mWidth * 0.5f;
    f32 length = shadow->mLength;
    f32 endZ = pos.z - length;
    u32 segments = u32(pos.z / 32.0f) - u32((pos.z - length) / 32.0f);
    u32 vertexCount = segments * 4 + 4;
    if (vertexCount < dShadow::VERTEX_CAPACITY) {
        static mVec3_c vertices[dShadow::VERTEX_CAPACITY];
        static f32 texCoords[dShadow::VERTEX_CAPACITY][2];
        mVec3_c left(pos.x - halfWidth, pos.y, pos.z);
        mVec3_c right(pos.x + halfWidth, pos.y, pos.z);
        u32 index = 0;
        for (u32 i = 0; i < u32(segments + 1); ++i) {
            mVec3_c nextLeft = left, nextRight = right;
            if (i == u32(segments)) {
                nextLeft = mVec3_c(pos.x - halfWidth, pos.y, pos.z - length);
                nextRight = mVec3_c(pos.x + halfWidth, pos.y, pos.z - length);
            } else {
                nextLeft.z -= 32.0f;
                nextRight.z -= 32.0f;
                mVec3_c snapped;
                dBGCF::snapToUnit(&snapped, &nextLeft);
                nextLeft.z = snapped.z + 16.0f;
                dBGCF::snapToUnit(&snapped, &nextRight);
                nextRight.z = snapped.z + 16.0f;
            }
            texCoords[index][0] = shadow->mTexS0;
            texCoords[index][1] = (nextLeft.z - endZ) / length;
            vertices[index] = nextLeft;
            vertices[index].x += mShear * (vertices[index].z - pos.z);
            index++;
            texCoords[index][0] = shadow->mTexS0;
            texCoords[index][1] = (left.z - endZ) / length;
            vertices[index] = left;
            vertices[index].x += mShear * (vertices[index].z - pos.z);
            index++;
            texCoords[index][0] = shadow->mTexS1;
            texCoords[index][1] = (right.z - endZ) / length;
            vertices[index] = right;
            vertices[index].x += mShear * (vertices[index].z - pos.z);
            index++;
            texCoords[index][0] = shadow->mTexS1;
            texCoords[index][1] = (nextRight.z - endZ) / length;
            vertices[index] = nextRight;
            vertices[index].x += mShear * (vertices[index].z - pos.z);
            index++;
            left = nextLeft; right = nextRight;
        }
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXBegin(GX_QUADS, GX_VTXFMT0, vertexCount);
        f32 (*uv)[2] = texCoords;
        for (mVec3_c *p = vertices; p < vertices + vertexCount; ++p, ++uv) {
            mVec3_c curved;
            dWorld::curvePosition(&curved, &mVec3_c(*p));
            GXPosition3f32(curved.x, curved.y, curved.z);
            GXTexCoord2f32((*uv)[0], (*uv)[1]);
        }
    }
}
namespace dShadow { static dShadow_c *sInstance; }

// 801C81D8
int dShadow_c::create() {
    EGG::Heap *heap = dHeap::localHeap_p;
    if (!mpProcHeap) {
        mpProcHeap = mHeap::createFrmHeap(0x1400, heap,
            "dShadow_c::::m_frmHeap_p : \x89\x65PROC\x83\x71\x81\x5b\x83\x76", 32, mHeap::OPT_NONE);
        if (mpProcHeap) {
            mAllocator.attach(mpProcHeap, 32);
            m3d::proc_c::create(&mAllocator, NULL);
            switch (dScene::getCurrentScene()) {
            case dScene::SCENE_RM_MM_FISH: setPriorityDraw(6, 2); break;
            case dScene::SCENE_RM_GRACE: setPriorityDraw(6, 0x82); break;
            default: setPriorityDraw(6, 0x12); break;
            }
            updateHeight();
            mRaiseWater = dScene::getCurrentScene() == dScene::SCENE_FIELD ||
                          dScene::getCurrentScene() == dScene::SCENE_DM_TITLE;
        }
    }
    if (mReady) return SUCCEEDED;
    {
        mShear = 0.0f;
        dShadow::sInstance = this;
        void *file = dBG::getAlwaysBank()->getFile("always/shadow.brres", NULL);
        if (file) {
            nw4r::g3d::ResFile res(file);
            const char **names = dShadow::sTextureNames;
            for (int i = 0; i < dShadow::TEXTURE_COUNT; ++names, ++i) mTextures[i] = res.GetResTex(*names);
            mReady = true;
            return SUCCEEDED;
        }
    }
    return SUCCEEDED;
}
// 801C8378
int dShadow_c::execute() {
    updateHeight();
    dTime_c time = *dTime_c::getCurrent();
    u32 minutes = u8(time.min) + u8(u8(time.hour + 6) % 12) * 60;
    dShadow::sSunTime = f32(minutes) / 720.0f;
    return SUCCEEDED;
}
// 801C846C
int dShadow_c::draw() {
    if (mReady) { calc(false); m3d::scnLeaf_c::entry(); }
    return SUCCEEDED;
}
// 801C84B8
int dShadow_c::doDelete() {
    m3d::scnLeaf_c::remove();
    if (mpProcHeap) { mHeap::destroyFrmHeap(mpProcHeap); mpProcHeap = NULL; }
    mRoundCount = 0; mTextureCount = 0; mReady = false;
    dShadow::sInstance = NULL;
    return SUCCEEDED;
}
namespace dShadow {
// 801C8518
BOOL requestRound(const mVec3_c *pos, u32 flags, const mAng *angle,
                  f32 sizeX, f32 scale, f32 alphaScale, f32 sizeZ, f32 heightScale) {
    if (sInstance && scale != 0.0f && alphaScale != 0.0f) {
        static Bounds_c bounds(160.0f, -480.0f, 512.0f);
        f32 z;
        f32 x = f32(__fabs(pos->x - lbl_80623FEC.x));
        z = pos->z - lbl_80623FEC.z;
        bool culled = false;
        if (x > bounds.z || z > bounds.x || z < bounds.y)
            culled = true;
        if (!culled) {
            if (sizeZ == 0.0f) sizeZ = sizeX;
            return sInstance->addRound(pos, flags, *angle, sizeX, scale, alphaScale, sizeZ, heightScale);
        }
    }
    return FALSE;
}
// 801C8628
BOOL requestTexture(const mVec3_c *pos, int texture, f32 width, f32 length, f32 texS0, f32 texS1) {
    if (sInstance) {
        if (texture >= TEXTURE_COUNT) texture = DEFAULT;
        return sInstance->addTexture(pos, texture, width, length, texS0, texS1);
    }
    return FALSE;
}
// 801C865C
f32 getSunTime() { return sSunTime; }
// 801C8664
BOOL fn_801C8664() { return FALSE; }
// 801C866C
BOOL fn_801C866C() { return FALSE; }
}
// 801C8674
void dShadow_c::updateHeight() {
    mHeight = 0.0f;
    switch (dScene::getCurrentScene()) {
    case dScene::SCENE_RM_MM_INS: mHeight = 0.5f; break;
    case dScene::SCENE_RM_GRACE: mHeight = 0.25f; break;
    default:
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) mHeight = dShadow::sHostIO.mHeight;
        else mHeight = 0.25f;
        break;
    }
}
static inline bool isWaterEdge(const mVec3_c &a, const mVec3_c &b, const mVec3_c &c) {
    mVec3_c center = (a + b) + c;
    center *= 0.33f;
    int x, z;
    dBGCF::posToBlock(&x, &z, &center);
    return x == 0 || x == 6;
}

// 801C8704
void shadowDtcb_c::checkFloor(dBGCF::floorList_c *floors) {
    dShadow::setupProjection(&mPos, mpView, &mProjection);
    u32 count = floors->getNum();
    GXBegin(GX_TRIANGLES, GX_VTXFMT0, count * 3);
    for (dBGCF::floor_c *p = floors->mFloors; p < floors->mFloors + count; ++p) {
        mVec3_c curvedA, curvedB, curvedC;
        const mVec3_c &pos1 = p->mPos1;
        mVec3_c a = p->mPos0, b = pos1, c = p->mPos2;
        bool water = mRaiseWater ? isWaterEdge(p->mPos0, pos1, p->mPos2) : false;
        if (water) { a.y += 0.25f; b.y += 0.25f; c.y += 0.25f; }
        else { a.y += mHeight; b.y += mHeight; c.y += mHeight; }
        dWorld::curvePositionSimple(&curvedA, &a);
        dWorld::curvePositionSimple(&curvedB, &b);
        dWorld::curvePositionSimple(&curvedC, &c);
        GXPosition3f32(curvedC.x, curvedC.y, curvedC.z);
        GXPosition3f32(curvedB.x, curvedB.y, curvedB.z);
        GXPosition3f32(curvedA.x, curvedA.y, curvedA.z);
    }
}
#define SHADOW_CHECK(name, condition) typedef char name[(condition) ? 1 : -1]
SHADOW_CHECK(roundSize, sizeof(dShadow::Round_c) == 0x1C);
SHADOW_CHECK(textureSize, sizeof(dShadow::Texture_c) == 0x24);
SHADOW_CHECK(projectionSize, sizeof(dShadow::Projection_s) == 0x14);
SHADOW_CHECK(shadowSize, sizeof(dShadow_c) == 0x2C38);
SHADOW_CHECK(roundOffset, offsetof(dShadow_c, mRounds) == 0x6C);
SHADOW_CHECK(textureOffset, offsetof(dShadow_c, mTextures) == 0x2C14);





