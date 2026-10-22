#include <game/framework/f_profile_name.hpp>
#include <game/game/d_fg_draw.hpp>
#include <egg/gfxe/eggFogManager.h>
#include <game/game/d_bg_attr.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_fg_obj.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_light.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_shadow.hpp>
#include <game/game/d_world.hpp>
#include <game/mLib/m_3d/global.hpp>
#include <game/mLib/m_sphere.hpp>
#include <new>
#include <nw4r/g3d/g3d_draw1mat1shp.h>
#include <nw4r/g3d/g3d_state.h>
#include <revolution/gx.h>
#include <stdio.h>
#include <string.h>

// Cedar-light registration/release in the foreground-object manager REL.
// These wrappers only call unidentified manager functions; ownership is not yet
// known.
#include <game/game/d_fgobj_manager.hpp>

namespace dFgDraw {
static designLoaders_c sDesigns;
static fgHostIO_c sHostIO;

fProf::fBaseProfile_c g_profile_FG_DRAW = {&create, fProf::FG_DRAW,
                                           fProf::DRAW_ORDER::FG_DRAW};
static const char *sHoleTexture = "tex_hole";
static const char *sSoilPalette = "pl_hole_soil";
static const char *sSandPalette = "pl_hole_sand";
static const char *sCloverTexture = "tex_clover";
static const char *sGrassPalette = "pl_grass";
static const char *sCrackTexture = "tex_crack";
static const char *sSoldOutTextures0 = "tex_JpnJpn_soldout";
static const char *sSoldOutTextures1 = "tex_UsaEng_soldout";
static const char *sSoldOutTextures2 = "tex_UsaFra_soldout";
static const char *sSoldOutTextures3 = "tex_UsaSpn_soldout";
static const char *sSoldOutTextures4 = "tex_EurEng_soldout";
static const char *sSoldOutTextures5 = "tex_EurFra_soldout";
static const char *sSoldOutTextures6 = "tex_EurGer_soldout";
static const char *sSoldOutTextures7 = "tex_EurIta_soldout";
static const char *sSoldOutTextures8 = "tex_EurSpn_soldout";
static const char *sSoldOutTextures9 = "tex_KorKor_soldout";
static const char *sGraceSoldOutTextures0 = "tex_JpnJpn_grace_soldout";
static const char *sGraceSoldOutTextures1 = "tex_UsaEng_grace_soldout";
static const char *sGraceSoldOutTextures2 = "tex_UsaFra_grace_soldout";
static const char *sGraceSoldOutTextures3 = "tex_UsaSpn_grace_soldout";
static const char *sGraceSoldOutTextures4 = "tex_EurEng_grace_soldout";
static const char *sGraceSoldOutTextures5 = "tex_EurFra_grace_soldout";
static const char *sGraceSoldOutTextures6 = "tex_EurGer_grace_soldout";
static const char *sGraceSoldOutTextures7 = "tex_EurIta_grace_soldout";
static const char *sGraceSoldOutTextures8 = "tex_EurSpn_grace_soldout";
static const char *sGraceSoldOutTextures9 = "tex_KorKor_grace_soldout";

static inline bool isCedar(const dItem::Item &item) {
    u16 id = item.mId;
    return id >= dItem::FG_CEDAR_SAPLING && id <= dItem::FG_CEDAR_LIGHTS;
}
static inline bool isPalm(const dItem::Item &item) {
    u16 id = item.mId;
    return id >= dItem::FG_PALM_SAPLING && id <= dItem::FG_PALM_FRUIT;
}
static inline bool isStump(const dItem::Item &item) {
    u16 id = item.mId;
    return id >= dItem::FG_STUMP_FIRST && id <= dItem::FG_STUMP_LAST;
}

static inline bool isStone(const dItem::Item &item) {
    u16 id = item.mId;
    return id >= dItem::FG_STONE_A && id <= dItem::FG_STONE_E;
}
static inline bool isMoneyRock(const dItem::Item &item) {
    u16 id = item.mId;
    return id >= dItem::FG_MONEY_ROCK_FIRST && id <= dItem::FG_MONEY_ROCK_LAST;
}

static inline bool isBroadleaf(u16 id) {
    return id >= dItem::FG_TREE_SAPLING && id <= dItem::FG_TREE_BELLS;
}
static inline bool isMoneyTree(u16 id) {
    return id >= dItem::FG_MONEY_TREE_SAPLING && id <= dItem::FG_MONEY_TREE;
}
static inline bool isWeed(u16 id) {
    return id >= dItem::FG_WEED_A && id <= dItem::FG_WEED_D;
}

// .text 00CC
void *create() {
    return new dFgDraw_c;
}

// .text 0148
BOOL designLoaders_c::load(void *heap) {
    for (u32 i = 0; i < DESIGN_COUNT; i++) {
        dPrivateData_c *player = dPrivateData_c::getChecked(
            dSaveData_c::getTown()->mPlayers, (i >> 3) & 3);
        mLoaders[i].loadDesignInPlace(&player->mOrgDesigns.mDesigns[i & 7],
                                      heap, 32, 32);
    }
    return TRUE;
}

// .text 01D4
BOOL designLoaders_c::release() {
    bool done = false;
    while (!done) {
        done = true;
        for (u32 i = 0; i < DESIGN_COUNT; i++) {
            done &= mLoaders[i].release();
        }
    }
    return TRUE;
}

// .text 025C
int blockMask_c::test(int x, int z) {
    return (mRows[z & 15] >> (x & 15)) & 1;
}
// .text 0274
void blockMask_c::set(int x, int z) {
    mRows[z & 15] |= 1 << (x & 15);
}
// .text 0294
void blockMask_c::clear(int x, int z) {
    mRows[z & 15] &= ~(1 << (x & 15));
}

// .text 02B4
void fieldMask_c::reset() {
    for (u32 z = 0; z < BLOCK_COUNT; z++) {
        for (u32 x = 0; x < BLOCK_COUNT; x++) {
            memset(&mBlocks[z][x], 0, sizeof(blockMask_c));
        }
    }
}

// .text 0330
int fieldMask_c::test(int x, int z) {
    int bx = x >> 4, bz = z >> 4;
    x -= bx << 4;
    z -= bz << 4;
    if (bx >= 0 && bx < BLOCK_COUNT && bz >= 0 && bz < BLOCK_COUNT) {
        return mBlocks[bz][bx].test(x, z);
    }
    return 0;
}

// .text 0380
void fieldMask_c::set(int x, int z) {
    int bx = x >> 4, bz = z >> 4;
    x -= bx << 4;
    z -= bz << 4;
    if (bx >= 0 && bx < BLOCK_COUNT && bz >= 0 && bz < BLOCK_COUNT) {
        mBlocks[bz][bx].set(x, z);
    }
}

// .text 03CC
void fieldMask_c::clear(int x, int z) {
    int bx = x >> 4, bz = z >> 4;
    x -= bx << 4;
    z -= bz << 4;
    if (bx >= 0 && bx < BLOCK_COUNT && bz >= 0 && bz < BLOCK_COUNT) {
        mBlocks[bz][bx].clear(x, z);
    }
}

// .text 0418
int getCullKind(const dItem::Item &item) {
    if (dItem::isRealItemId(item.mId)) {
        return CULL_TEXTURE;
    }
    dItem::FgInfo *info = item.getFgInfo();
    if (info) {
        CullKind kind = CULL_TEXTURE;
        if (info->mTreeStage >= 2) {
            kind = CULL_MODEL;
        }
        return kind;
    }
    return CULL_NONE;
}

// .text 0490
BOOL inModelRange(int x, int z, int unitX, int unitZ, int height,
                  int unitHeight) {
    int above = unitHeight - height;
    if (above < 0) {
        above = 0;
    }
    int dx = unitX - x, dz = unitZ - z;
    return dx >= -13 && dx <= 13 && dz >= -19 && dz - above <= 5;
}
// .text 04D4
BOOL inTextureRange(int x, int z, int unitX, int unitZ, int height,
                    int unitHeight) {
    int above = unitHeight - height;
    if (above < 0) {
        above = 0;
    }
    int dx = unitX - x, dz = unitZ - z;
    return dx >= -13 && dx <= 13 && dz >= -16 && dz - above <= 5;
}
// .text 0518
BOOL inNoRange(int, int, int, int, int, int) {
    return FALSE;
}
// .text 0520
BOOL inRange(int x, int z, int unitX, int unitZ, u8 kind, int height,
             int unitHeight) {
    typedef BOOL (*RangeFunc)(int, int, int, int, int, int);
    static RangeFunc funcs[] = {inNoRange, inTextureRange, inModelRange};
    return funcs[kind](x, z, unitX, unitZ, height, unitHeight);
}

// .text 0540
void setupTexture(const mMtx_c &view, nw4r::g3d::ResTex tex,
                  nw4r::g3d::ResPltt pltt, int width, int height) {
    GXLoadPosMtxImm(view, GX_PNMTX0);
    GXLoadNrmMtxImm(view, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,
                      FALSE, GX_PTIDENTITY);
    GXSetNumTexGens(1);
    GXSetCullMode(GX_CULL_BACK);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, TRUE, GX_SRC_REG, GX_SRC_REG,
                  dLight::getLightMask(), GX_DF_CLAMP, GX_AF_SPOT);
    GXSetNumTevStages(1);
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
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetZMode(TRUE, GX_LEQUAL, FALSE);
    GXSetZCompLoc(FALSE);
    void *data;
    u16 texWidth, texHeight;
    GXCITexFmt format;
    f32 minLod, maxLod;
    u8 mipmap;
    tex.GetTexObjCIParam(&data, &texWidth, &texHeight, &format, &minLod,
                         &maxLod, &mipmap);

    GXTexObj texture;
    GXTlutObj palette;
    GXInitTexObjCI(&texture, data, width == -1 ? texWidth : width,
                   height == -1 ? texHeight : height, (GXCITexFmt)format,
                   GX_CLAMP, GX_CLAMP, mipmap, GX_TLUT0);
    GXInitTlutObj(&palette, pltt.GetPlttData(), pltt.GetFmt(),
                  pltt.GetNumEntries());
    GXLoadTlut(&palette, GX_TLUT0);
    GXInitTexObjLOD(&texture, GX_NEAR, GX_NEAR, minLod, maxLod, 0.0f, FALSE,
                    FALSE, GX_ANISO_1);
    GXLoadTexObj(&texture, GX_TEXMAP0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
}

// .text 0804
void calcTexCoord(mVec3_c *pos, f32 *s, f32 *t, const mVec3_c &origin,
                  mVec3_c scale) {
    mVec3_c delta = *pos - origin;
    *s = 0.5f + delta.x / 32.0f;
    *t = 0.5f + delta.z / 32.0f;
    delta.x *= scale.x;
    delta.y *= scale.y;
    delta.z *= scale.z;
    *pos = origin + delta;
}

// .text 08DC
void entry_c::reset() {
    mKind = DRAW_MODEL;
    mItem = dItem::ITEM_ID_NONE;
    mUnitZ = -1;
    mUnitX = -1;
    mHeight = 0;
    mScale.set(1.0f, 1.0f, 1.0f);
    mRot.set(0, 0, 0);
    mLink.prevObject = NULL;
    mLink.nextObject = NULL;
    mModel = nw4r::g3d::ResMdl(NULL);
    mFruitModel = nw4r::g3d::ResMdl(NULL);
    mTexture = nw4r::g3d::ResTex(NULL);
    mPalette = nw4r::g3d::ResPltt(NULL);
    mTexWidth = -1;
    mTexHeight = -1;
    mFlags = 0;
}
// .text 0948
void entry_c::restoreVisibility() {
    mFlags &= ~HIDDEN;
    dItem::Item item(mItem);
    if (item.isFg()) {
        dItem::FgInfo *info = item.getFgInfo();
        if (info && info->mTreeStage > 0) {
            mFlags |= TREE;
        }
    }
}

// .text 09B4
void entry_c::setupResources() {
    const dItem::Item item(mItem);
    mModel = nw4r::g3d::ResMdl(NULL);
    mFruitModel = nw4r::g3d::ResMdl(NULL);
    nw4r::g3d::ResFile always(dFgObj::getAlwaysBank()->getData());
    mScale.set(1.0f, 1.0f, 1.0f);
    mShadowRadius = 0.0f;
    mShadowScale = 0.0f;
    mShadowKind = dShadow::DEFAULT;
    mRot.set(0, 0, 0);
    mHeight = (int)(mPos.y / 32.0f);
    u32 id = item.getId();
    if (item.isFg()) {
        nw4r::g3d::ResFile season(g_fgObjRes.getData());
        if (id == dItem::FG_HOLE) {
            mKind = DRAW_TEXTURE;
            mTexture = season.GetResTex(sHoleTexture);
            mPalette = season.GetResPltt(sSoilPalette);
            if (fgMngProc_isUnitSpecialGround(mUnitX, mUnitZ)) {
                mPalette = season.GetResPltt(sSandPalette);
            }
        } else if (item.mId == dItem::FG_CLOVER ||
                   item.mId == dItem::FG_LUCKY_CLOVER) {
            mKind = DRAW_TEXTURE;
            mTexture = season.GetResTex(sCloverTexture);
            mPalette = season.GetResPltt(sGrassPalette);
        } else if (id >= dItem::FG_DESIGN_FIRST &&
                   id <= dItem::FG_DESIGN_LAST) {
            mKind = DRAW_TEXTURE;
            int index = (id - dItem::FG_DESIGN_FIRST) & 31;
            mTexture = sDesigns.getTexture(index);
            mPalette = sDesigns.getPalette(index);
        } else {
            dItem::FgInfo *info = item.getFgInfo();
            mKind = DRAW_MODEL;
            char name[19];
            memset(name, 0, sizeof(name));
            getModelName(item, mUnitX, mUnitZ, name);
            mModel = season.GetResMdl(name);
            if (info->mTreeStage > 0) {
                mFlags |= TREE;
            }
            if (dBGCF::isFlat(&mPos)) {
                mShadowRadius = (s8)info->_08[3];
                mShadowScale = 32.0f * (s8)info->_08[4] / 100.0f;
                if (info->mTreeStage > 0) {
                    if (isCedar(item)) {
                        mShadowKind = dShadow::TREE_B;
                    } else if (isPalm(item)) {
                        mShadowKind = dShadow::TREE_C;
                    } else {
                        mShadowKind = dShadow::TREE_A;
                    }
                } else if (isStump(item)) {
                    switch (id) {
                    case dItem::FG_STUMP_S1:
                    case dItem::FG_STUMP_S2:
                    case dItem::FG_STUMP_S3:
                    case dItem::FG_STUMP:
                        mShadowKind = dShadow::TREE_A_CUT;
                        break;
                    case dItem::FG_CEDAR_STUMP_S1:
                    case dItem::FG_CEDAR_STUMP_S2:
                    case dItem::FG_CEDAR_STUMP_S3:
                    case dItem::FG_CEDAR_STUMP:
                        mShadowKind = dShadow::TREE_B_CUT;
                        break;
                    case dItem::FG_PALM_STUMP_S1:
                    case dItem::FG_PALM_STUMP_S2:
                    case dItem::FG_PALM_STUMP_S3:
                    case dItem::FG_PALM_STUMP:
                        mShadowKind = dShadow::TREE_C_CUT;
                        break;
                    }
                } else if (isStone(item) || isMoneyRock(item)) {
                    mShadowKind = dShadow::STONE;
                }
            }
            if ((u16)info->mDropItem != dItem::ITEM_ID_NONE) {
                dItem::Item dropItem((u16)info->mDropItem);
                const dItem::BITM *bitm =
                    dItem::infoBank_c::get()->getBITM(dropItem);
                if (bitm) {
                    mFruitModel = always.GetResMdl(bitm->getFgObjName(TRUE));
                }
            }
        }
    } else {
        dFdBase_c *field = dField::getInfo(0);
        if (!(mFlags & EXPLICIT_POS) && field &&
            field->isBuried(mUnitX, mUnitZ)) {
            nw4r::g3d::ResFile season(g_fgObjRes.getData());
            mKind = DRAW_TEXTURE;
            mTexture = season.GetResTex(sCrackTexture);
            mPalette = season.GetResPltt(sSoilPalette);
            if (fgMngProc_isUnitSpecialGround(mUnitX, mUnitZ)) {
                mPalette = season.GetResPltt(sSandPalette);
            }
        } else {
            if (mFlags & PRESENT) {
                mModel = always.GetResMdl("fg_present_gr");
            } else {
                mModel = always.GetResMdl(
                    dItem::infoBank_c::get()->getBITM(item)->getFgObjName(
                        TRUE));
            }
            if (mItem ==
                dItem::Item(dItem::ITEM_IDX_SOLD_OUT_SIGN_00).getId()) {
                nw4r::g3d::ResTex tex = always.GetResTex(getSoldOutTexture(
                    (dSaveData_c::getTown()->_0735C2 >> 4) & 15));
                nw4r::g3d::ResMat mat = mModel.GetResMat("m_soldout");
                mat.ReleaseTexByName(sSoldOutTextures0,
                                     strlen(sSoldOutTextures0));
                mat.BindTexByName(tex, sSoldOutTextures0);
            } else if (mItem ==
                       dItem::Item(dItem::ITEM_IDX_SOLD_OUT_SIGN_01).getId()) {
                nw4r::g3d::ResTex tex = always.GetResTex(getGraceSoldOutTexture(
                    (dSaveData_c::getTown()->_0735C2 >> 4) & 15));
                nw4r::g3d::ResMat mat = mModel.GetResMat("m_grace_soldout");
                mat.ReleaseTexByName(sGraceSoldOutTextures0,
                                     strlen(sGraceSoldOutTextures0));
                mat.BindTexByName(tex, sGraceSoldOutTextures0);
            }
        }
    }
    if (mKind == DRAW_TEXTURE) {
        dBGCF::groundChk_c ground(mUnitX, mUnitZ, 0, 0, 0);
        if (ground.mUnitAttr == BG_ATTR_SAND ||
            ground.mUnitAttr == BG_ATTR_SAND_SE ||
            ground.mUnitAttr == BG_ATTR_SAND_SW ||
            fgMngProc_isUnitSpecialGround(mUnitX, mUnitZ)) {
            mFlags |= SPECIAL_GROUND;
        }
    }
}

// .text 103C
BOOL entry_c::init(const dItem::Item &item, int x, int z, int kind,
                   BOOL upper) {
    mFlags = 0;
    mUnitX = x;
    mUnitZ = z;
    mItem = item.mId;
    mCullKind = kind;
    mCullRadius = getCullRadius(mCullKind);
    getItemPosition(&mPos, mItem, x, z, upper);
    if (item == dItem::FG_CEDAR_LIGHTS) {
        dFgobjManager::startDecoAnm(x, z, (int)cM::rndInt(240));
    }
    setupResources();
    if (isHidden(mUnitX, mUnitZ)) {
        mFlags |= HIDDEN;
        mFlags &= ~TREE;
    }
    return TRUE;
}

// .text 1144
BOOL entry_c::initAt(u16 id, const mVec3_c &pos, const mVec3_c &scale,
                     BOOL shadow, BOOL present, const mAng3_c &rot) {
    mFlags = 0;
    int x, z;
    dBGCF::posToUnit(&x, &z, &pos);
    mPos = pos;
    mUnitX = x;
    mUnitZ = z;
    mItem = id;
    dItem::Item item(id);
    mCullKind = getCullKind(item);
    mCullRadius = getCullRadius(mCullKind);
    mFlags |= EXPLICIT_POS;
    if (present) {
        if (item.isSame(dItem::Item(dItem::ITEM_IDX_PRESENT))) {
            mFlags |= PRESENT;
        }
    }
    setupResources();
    if (!shadow) {
        mShadowRadius = 0.0f;
    }
    mRot = rot;
    mScale.x *= scale.x;
    mScale.y *= scale.y;
    mScale.z *= scale.z;
    if (present && item.isFg()) {
        mFlags |= HIDDEN;
        mFlags &= ~TREE;
    }
    return TRUE;
}

// .text 12BC
BOOL entry_c::initFlat(u16 id, const mVec3_c &pos, const mVec3_c &scale) {
    initAt(id, pos, scale, FALSE, FALSE, mAng3_c(0, 0, 0));
    mFlags |= FLAT_TEXTURE;
    return TRUE;
}

// .text 1314
BOOL entry_c::initAtUnit(u16 id, const mVec3_c &pos, int x, int z,
                         const mVec3_c &scale) {
    mFlags = 0;
    mPos = pos;
    mUnitX = x;
    mUnitZ = z;
    mItem = id;
    dItem::Item item(id);
    mCullKind = getCullKind(item);
    mCullRadius = getCullRadius(mCullKind);
    mFlags |= EXPLICIT_POS;
    setupResources();
    mShadowRadius = 0.0f;
    mScale.x *= scale.x;
    mScale.y *= scale.y;
    mScale.z *= scale.z;
    return TRUE;
}

// .text 13EC
BOOL entry_c::initWithMtx(u16 id, const mVec3_c &pos, const mMtx_c &mtx) {
    mFlags = 0;
    int x, z;
    dBGCF::posToUnit(&x, &z, &pos);
    mPos = pos;
    mUnitX = x;
    mUnitZ = z;
    mItem = id;
    dItem::Item item(id);
    mCullKind = getCullKind(item);
    mCullRadius = getCullRadius(mCullKind);
    mFlags |= EXPLICIT_POS;
    setupResources();
    mFlags |= EXPLICIT_MTX;
    mMtx = mtx;
    mShadowRadius = 0.0f;
    return TRUE;
}

// .text 152C
BOOL entry_c::isNeeded(int x, int z, int height) {
    if (!inRange(x, z, mUnitX, mUnitZ, mCullKind, height, mHeight)) {
        return FALSE;
    }
    if (!(mFlags & EXPLICIT_POS)) {
        dItem::Item item = getUnitItem(mUnitX, mUnitZ, NULL, NULL, FALSE);
        u16 id = item.mId;
        if ((!dItem::isRealItemId(id) && id >= dItem::FG_COUNT) ||
            mItem != id) {
            return FALSE;
        }
    }
    return TRUE;
}

// .text 1608
dItem::Item getUnitItem(int x, int z, int *height, BOOL *upper,
                        BOOL filterFurniture) {
    const dFdBase_c *field = dField::getInfo(0);
    dItem::Item *item = NULL;
    BOOL layer = TRUE;
    if (field) {
        item = field->getItem(x, z, 1);
        if (!item || *item == dItem::ITEM_ID_NONE) {
            item = field->getItem(x, z, 0);
            layer = FALSE;
        }
    }
    const dItem::Item &value = item ? *item : dItem::Item();
    u16 id = value.mId;
    if (upper) {
        *upper = layer;
    }
    if (filterFurniture && dItem::isRealItemId(id)) {
        const dItem::BITM *bitm =
            dItem::infoBank_c::get()->getBITM(dItem::Item(id));
        int func = bitm->m_ftrFunc;
        if ((static_cast<u32>(func) < 0x41 ? func : 1) != 0) {
            return dItem::Item((u16)dItem::ITEM_ID_NONE);
        }
    }
    if (height) {
        mVec3_c pos;
        dFdBase_c::getUnitCenterPos(&pos, x, z);
        *height = (int)(dBGCF::getUnitY(&pos) / 32.0f);
    }
    return dItem::Item(id);
}

// .text 1784
BOOL entry_c::release() {
    if (mItem == dItem::FG_CEDAR_LIGHTS) {
        dFgobjManager::endDecoAnm(mUnitX, mUnitZ);
    }
    mKind = DRAW_MODEL;
    mItem = dItem::ITEM_ID_NONE;
    mUnitX = mUnitZ = -1;
    return TRUE;
}

// .text 17F0
void entry_c::drawModel(const mMtx_c &view) {
    dCamera_c *camera;
    if (!mModel.IsValid()) {
        return;
    }
    if ((mFlags & TREE) && (camera = (dCamera_c *)dScene::getSceneE848()) &&
        camera->isPastForeground(&mPos, 32.0f)) {
        return;
    }
    mMtx_c model, normal, world;
    mMtx_c viewMtx = view;
    mVec3_c curved;
    mAng angle = dWorld::curvePosition(&curved, &mPos);
    if (mFlags & EXPLICIT_MTX) {
        model = mMtx;
    } else {
        model.trans(curved);
        model.XrotM(angle);
        if (!mRot.isZero()) {
            model.ZXYrotM(mRot);
        }
    }
    world = model;
    normal = model;
    mVec3_c scale = mScale;
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_SHOP_ROOM) &&
        !(mFlags & EXPLICIT_POS)) {
        scale *= 1.2f;
    }
    model.concat(mMtx_c::createScale(scale.x, scale.y, scale.z));
    PSMTXConcat(viewMtx, model, model);
    PSMTXConcat(viewMtx, normal, normal);
    if (!(mFlags & HIDDEN)) {
        nw4r::g3d::ResMat mat = mModel.GetResMat(0);
        nw4r::g3d::ResShp shp = mModel.GetResShp(0);
        nw4r::g3d::G3DState::SetViewPosNrmMtxArray(model, NULL, NULL);
        nw4r::g3d::Draw1Mat1ShpDirectly(mat, shp, model, normal, 0, NULL,
                                        NULL);
    }
    if (hasShadow()) {
        dShadow::requestTexture(&mPos, mShadowKind, mShadowRadius, mShadowScale,
                                0.0f, 1.0f);
    }
    camera = (dCamera_c *)dScene::getSceneE848();
    if (mFruitModel.IsValid() && camera) {
        int kind;
        u32 count;
        if (isCedar(dItem::Item(mItem))) {
            count = 2;
            kind = 1;
        } else if (isPalm(dItem::Item(mItem))) {
            count = 2;
            kind = 2;
        } else {
            count = 3;
            kind = 0;
        }
        nw4r::g3d::ResMat mat = mFruitModel.GetResMat(0);
        nw4r::g3d::ResShp shp = mFruitModel.GetResShp(0);
        for (u32 i = 0; i < count; i++) {
            mVec3_c pos;
            PSMTXMultVec(world.mtx, (const Vec *)getFruitOffset(i, kind),
                         (Vec *)&pos);
            mSphere_c sphere(pos, getCullRadius(CULL_TEXTURE));
            if (camera->mCullFrustum.intersectSphere(&sphere)) {
                model.trans(pos.x, pos.y, pos.z);
                model.XrotM(angle);
                PSMTXConcat(viewMtx, model, model);
                normal = model;
                nw4r::g3d::G3DState::SetViewPosNrmMtxArray(model, NULL, NULL);
                nw4r::g3d::Draw1Mat1ShpDirectly(mat, shp, model, normal, 0,
                                                NULL, NULL);
            }
        }
    }
}

// .text 1DA4
void entry_c::drawTexture(const mMtx_c &view) {
    static mVec3_c corners[] = {
        mVec3_c(16.0f, 0.0f, -16.0f), mVec3_c(16.0f, 0.0f, 16.0f),
        mVec3_c(-16.0f, 0.0f, 16.0f), mVec3_c(-16.0f, 0.0f, -16.0f)};
    if (mTexture.IsValid() && mPalette.IsValid()) {
        if (dBGCF::isFlat(&mPos) || (mFlags & FLAT_TEXTURE)) {
            setupTexture(view, mTexture, mPalette, mTexWidth, mTexHeight);
            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            for (mVec3_c *corner = corners; corner < corners + 4; corner++) {
                mVec3_c pos = mPos + *corner;
                if (mFlags & SPECIAL_GROUND) {
                    pos.y += 1.0f;
                } else if (mItem == dItem::FG_CLOVER ||
                           mItem == dItem::FG_LUCKY_CLOVER) {
                    pos.y += 0.5f;
                } else {
                    pos.y += 0.185f;
                }
                f32 s, t;
                calcTexCoord(&pos, &s, &t, mPos, mScale);
                mVec3_c curved;
                mAng angle = dWorld::curvePositionSimple(&curved, &pos);
                mVec3_c normal(0.0f, 0.98f, 0.0f);
                normal.rotX(angle);
                GXPosition3f32(curved.x, curved.y, curved.z);
                GXNormal3f32(normal.x, normal.y, normal.z);
                GXTexCoord2f32(s, t);
            }
        } else {
            drawUtDtcb_c cb(mPos, mScale, mTexture, mPalette, view, mTexWidth,
                            mTexHeight, isSpecialGround());
            dBGCF::check(&mPos, &mPos, &cb, 1, 1, 0);
        }
    }
}
} // namespace dFgDraw

// .text 2114
drawUtDtcb_c::drawUtDtcb_c(const mVec3_c &pos, mVec3_c scale,
                           nw4r::g3d::ResTex tex, nw4r::g3d::ResPltt pltt,
                           const mMtx_c &view, s16 width, s16 height,
                           bool special)
    : mTexture(NULL), mPalette(NULL) {
    mPos = pos;
    mScale = scale;
    mTexture = tex;
    mPalette = pltt;
    mpView = &view;
    mWidth = width;
    mHeight = height;
    mSpecial = special;
}

namespace dFgDraw {
// .text 2184
BOOL entry_c::draw(const mMtx_c &view, int kind) {
    dCamera_c *camera = (dCamera_c *)dScene::getSceneE848();
    if (mKind == kind) {
        BOOL drawn = FALSE;
        if (camera) {
            mVec3_c curved;
            mVec3_c pos = mPos;
            if (mKind != DRAW_TEXTURE) {
                pos.y += mCullRadius - 9.6f;
            }
            dWorld::curvePosition(&curved, &pos);
            mSphere_c sphere(curved, mCullRadius);
            if (camera->mCullFrustum.intersectSphere(&sphere)) {
                typedef void (entry_c::*DrawFunc)(const mMtx_c &);
                static const DrawFunc funcs[] = {&entry_c::drawModel,
                                                 &entry_c::drawTexture};
                (this->*funcs[mKind])(view);
                drawn = TRUE;
            }
        }
        if (mFlags & EXPLICIT_POS) {
            mUnitX = -127;
            mUnitZ = -127;
        }
        return drawn;
    }
    return FALSE;
}

// .text 22B4
const char *entry_c::getSoldOutTexture(int language) {
    switch (language) {
    case 0:
        return sSoldOutTextures0;
    case 1:
        return sSoldOutTextures1;
    case 2:
        return sSoldOutTextures3;
    case 3:
        return sSoldOutTextures2;
    case 4:
        return sSoldOutTextures4;
    case 5:
        return sSoldOutTextures8;
    case 6:
        return sSoldOutTextures5;
    case 7:
        return sSoldOutTextures7;
    case 8:
        return sSoldOutTextures6;
    case 9:
        return sSoldOutTextures9;
    default:
        return NULL;
    }
}
// .text 2334
const char *entry_c::getGraceSoldOutTexture(int language) {
    switch (language) {
    case 0:
        return sGraceSoldOutTextures0;
    case 1:
        return sGraceSoldOutTextures1;
    case 2:
        return sGraceSoldOutTextures3;
    case 3:
        return sGraceSoldOutTextures2;
    case 4:
        return sGraceSoldOutTextures4;
    case 5:
        return sGraceSoldOutTextures8;
    case 6:
        return sGraceSoldOutTextures5;
    case 7:
        return sGraceSoldOutTextures7;
    case 8:
        return sGraceSoldOutTextures6;
    case 9:
        return sGraceSoldOutTextures9;
    default:
        return NULL;
    }
}

// .text 23B4
BOOL manager_c::add(u16 item, const mVec3_c &pos, const mVec3_c &scale,
                    BOOL shadow, BOOL present, const mAng3_c &rot) {
    entry_c *entry = (entry_c *)nw4r::ut::List_GetPrev(&mFree, NULL);
    if (entry && entry->initAt(item, pos, scale, shadow, present, rot)) {
        nw4r::ut::List_Remove(&mFree, entry);
        insert(&mActive, entry);
        return TRUE;
    }
    return FALSE;
}
// .text 2460
BOOL manager_c::addWithMtx(u16 item, const mVec3_c &pos, const mMtx_c &mtx) {
    entry_c *entry = (entry_c *)nw4r::ut::List_GetPrev(&mFree, NULL);
    if (entry && entry->initWithMtx(item, pos, mtx)) {
        nw4r::ut::List_Remove(&mFree, entry);
        insert(&mActive, entry);
        return TRUE;
    }
    return FALSE;
}
// .text 24F4
BOOL manager_c::addFlat(u16 item, const mVec3_c &pos, const mVec3_c &scale) {
    entry_c *entry = (entry_c *)nw4r::ut::List_GetPrev(&mFree, NULL);
    if (entry && entry->initFlat(item, pos, scale)) {
        nw4r::ut::List_Remove(&mFree, entry);
        insert(&mActive, entry);
        return TRUE;
    }
    return FALSE;
}
// .text 2588
BOOL manager_c::addAtUnit(u16 item, const mVec3_c &pos, int x, int z,
                          const mVec3_c &scale) {
    entry_c *entry = (entry_c *)nw4r::ut::List_GetPrev(&mFree, NULL);
    if (entry && entry->initAtUnit(item, pos, x, z, scale)) {
        nw4r::ut::List_Remove(&mFree, entry);
        insert(&mActive, entry);
        return TRUE;
    }
    return FALSE;
}
// .text 262C
BOOL manager_c::init(entry_c *entries, int count, bool drawFurniture) {
    mDrawFurniture = drawFurniture;
    nw4r::ut::List_Init(&mFree, 0);
    nw4r::ut::List_Init(&mActive, 0);
    if (entries && count) {
        for (entry_c *end = entries + count; entries < end; entries++) {
            nw4r::ut::List_Append(&mFree, entries);
        }
    }
    mUsed.reset();
    return TRUE;
}
// .text 26C8
BOOL manager_c::execute() {
    if (!dField::getInfo(0)) {
        return FALSE;
    }
    int x, z;
    dBGCF::posToUnit(&x, &z, &dCamera_c::sCenter);
    mVec3_c *player = dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    int height;
    if (player) {
        height = (int)(player->y / 32.0f);
    } else {
        height = 0;
    }
    entry_c *entry = (entry_c *)nw4r::ut::List_GetNext(&mActive, NULL);
    while (entry) {
        entry_c *next = (entry_c *)nw4r::ut::List_GetNext(&mActive, entry);
        int unitX = entry->getUnitX(), unitZ = entry->getUnitZ();
        if (!entry->isNeeded(x, z, height) && entry->release()) {
            nw4r::ut::List_Remove(&mActive, entry);
            if (!(entry->mFlags & EXPLICIT_POS)) {
                mUsed.clear(unitX, unitZ);
            }
            nw4r::ut::List_Append(&mFree, entry);
        }
        entry = next;
    }
    for (int dz = -19; dz <= 5; dz++) {
        f32 radius = 13.0f;
        for (int dx = -radius; dx <= radius; dx++) {
            int unitX = x + dx, unitZ = z + dz;
            if (!mUsed.test(unitX, unitZ)) {
                BOOL upper;
                int unitHeight;
                dItem::Item item = getUnitItem(unitX, unitZ, &unitHeight,
                                               &upper, !mDrawFurniture);
                int kind = getCullKind(item);
                if (inRange(x, z, unitX, unitZ, kind, height, unitHeight)) {
                    entry = (entry_c *)nw4r::ut::List_GetPrev(&mFree, NULL);
                    if (entry && entry->init(item, unitX, unitZ, kind, upper)) {
                        nw4r::ut::List_Remove(&mFree, entry);
                        insert(&mActive, entry);
                        mUsed.set(unitX, unitZ);
                    }
                }
            }
        }
    }
    return TRUE;
}
// .text 296C
BOOL manager_c::draw(const mMtx_c &view, int kind) {
    entry_c *entry = (entry_c *)nw4r::ut::List_GetNext(&mActive, NULL);
    while (entry) {
        entry_c *next = (entry_c *)nw4r::ut::List_GetNext(&mActive, entry);
        entry->draw(view, kind);
        entry = next;
    }
    return TRUE;
}
// .text 29E8
BOOL manager_c::release() {
    return TRUE;
}
// .text 29F0
BOOL manager_c::setHidden(int x, int z, BOOL hidden) {
    entry_c *entry = (entry_c *)nw4r::ut::List_GetNext(&mActive, NULL);
    while (entry) {
        entry_c *next = (entry_c *)nw4r::ut::List_GetNext(&mActive, entry);
        if (x == entry->mUnitX && z == entry->mUnitZ) {
            if (hidden) {
                entry->mFlags |= HIDDEN;
            } else {
                entry->restoreVisibility();
            }
            return TRUE;
        }
        entry = next;
    }
    return FALSE;
}
// .text 2AA4
void manager_c::insert(nw4r::ut::List *list, entry_c *entry) {
    int z = entry->mUnitZ;
    entry_c *next = (entry_c *)nw4r::ut::List_GetNext(list, NULL);
    while (next) {
        if (next->mUnitZ >= z) {
            break;
        }
        next = (entry_c *)nw4r::ut::List_GetNext(list, next);
    }
    nw4r::ut::List_Insert(list, next, entry);
}
manager_c sManager;
static hiddenUnit_c sHidden[HIDDEN_CAPACITY];

// .text 2B2C
void add(u16 item, const mVec3_c &pos, const mVec3_c &scale, BOOL shadow,
         BOOL present, const mAng3_c &rot) {
    sManager.add(item, pos, scale, shadow, present, rot);
}
// .text 2B88
BOOL addWithMtx(u16 item, const mVec3_c &pos, const mMtx_c &mtx) {
    return sManager.addWithMtx(item, pos, mtx);
}
// .text 2BA8
BOOL addFlat(u16 item, const mVec3_c &pos, const mVec3_c &scale) {
    return sManager.addFlat(item, pos, scale);
}
// .text 2BC8
BOOL addAtUnit(u16 item, const mVec3_c &pos, int x, int z,
               const mVec3_c &scale) {
    return sManager.addAtUnit(item, pos, x, z, scale);
}
// .text 2BF8
BOOL setHidden(int x, int z, BOOL hidden) {
    return sManager.setHidden(x, z, hidden);
}

// .text 2C18
mVec3_c *getFruitOffset(u32 index, int kind) {
    static mVec3_c broadleaf[] = {mVec3_c(-16.5f, 36.0f, 18.5f),
                                  mVec3_c(18.5f, 40.5f, 18.0f),
                                  mVec3_c(1.0f, 56.5f, 8.5f)};
    static mVec3_c cedar[] = {mVec3_c(-13.0f, 30.0f, 12.0f),
                              mVec3_c(13.0f, 30.0f, 12.0f)};
    static mVec3_c palm[] = {mVec3_c(-6.5f, 36.0f, 8.0f),
                             mVec3_c(5.0f, 32.0f, 11.0f)};
    static mVec3_c *offsets[] = {broadleaf, cedar, palm};
    static const u8 counts[] = {3, 2, 2};
    return index < counts[kind] ? &offsets[kind][index] : &broadleaf[2];
}
// .text 2E40
void getScatterOffset(mVec3_c *pos, int x, int z) {
    static const f32 offsets[] = {6, -4, 0,  0,  -4, 4,  4,  0,  -4, -4, -6,
                                  2, 2,  0,  -2, 4,  6,  -2, 8,  0,  -2, -6,
                                  4, 4,  -2, 6,  4,  -6, 6,  -2, -2, -4};
    u8 index = (x & 3) * 2 + (z & 3) * 8;
    pos->set(offsets[index], 0.0f, offsets[index + 1]);
}
// .text 2E84
void getItemPosition(mVec3_c *pos, u16 id, int x, int z, BOOL upper) {
    dFdBase_c::getUnitCenterPos(pos, x, z);
    if (!upper) {
        if (dScene::isSceneAttr(dScene::getCurrentScene(),
                                dScene::SCENE_ATTR_TOWN)) {
            dItem::Item item(id);
            if (isWeed(id) || item.isShell()) {
                mVec3_c offset;
                getScatterOffset(&offset, x, z);
                pos->x += offset.x;
                pos->z += offset.z;
            }
        }
        if (dBGCF::getAttr(x, z) == BG_ATTR_TAILOR) {
            pos->y = 0.0f;
        } else {
            pos->y = dBGCF::getGroundY(pos, 0);
        }
    } else {
        pos->y = dFTR::getGroundY(x, z);
    }
}

} // namespace dFgDraw

// .text 2F9C
namespace dFgDraw {
static char sBroadleafModelFormat[] = "fg_treeA_%d_%d";
static char sBroadleafMaterialFormat[] = "m_treeA_%d";
static char sBroadleafTextureBaseFormat[] = "tex_treeA_%d";
static char sBroadleafTextureFormat[] = "tex_treeA_%d_%d";
static char sBroadleafPaletteFormat[] = "pl_treeA_%d";
static char sBroadleafPaletteBase[] = "pl_treeA";
static char sBroadleafTopModelFormat[] = "fg_treeA_%d_t_%d";
static char sBroadleafTopMaterialAFormat[] = "m_treeA_%da";
static char sBroadleafTopMaterialBFormat[] = "m_treeA_%db";
static char sBroadleafSaplingModelFormat[] = "fg_treeA_0_%d_a";
static char sBroadleafSaplingTextureFormat[] = "tex_treeA_0_%d";
static char sSaplingMaterialFormat[] = "m_v_%d";
static char sBroadleafSaplingTextureBase[] = "tex_treeA_0";
static char sDeadBroadleafModelFormat[] = "fg_treeA_x_%d";
static char sDeadBroadleafMaterial[] = "m_treeA_x";
static char sDeadBroadleafTextureFormat[] = "tex_treeA_x_%d";
static char sDeadBroadleafTextureBase[] = "tex_treeA_x";
static char sDeadBroadleafSaplingModelFormat[] = "fg_treeA_x_%d_a";
} // namespace dFgDraw

void dFgDraw_c::bindTreeResources() {
    nw4r::g3d::ResFile file(g_fgObjRes.getData());
    nw4r::g3d::ResMdl model(NULL);
    nw4r::g3d::ResMat mat(NULL);
    char modelName[0x1E], matName[0x1E], texName[0x1E], baseTexName[0x1E],
        paletteName[0x1E];
    const char *broadleafPaletteBase;
    const char *saplingTextureBase, *saplingPaletteBase;
    const char *deadTreeTextureBase, *deadTreePaletteBase;
    const char *deadSaplingTextureBase, *deadSaplingPaletteBase;
    for (int stage = 0; stage < 5; stage++) {
        for (int variant = 1; variant <= 2; variant++) {
            broadleafPaletteBase = dFgDraw::sBroadleafPaletteBase;
            sprintf(modelName, dFgDraw::sBroadleafModelFormat, stage, variant);
            model = file.GetResMdl(modelName);
            sprintf(matName, dFgDraw::sBroadleafMaterialFormat, stage);
            mat = model.GetResMat(matName);
            sprintf(baseTexName, dFgDraw::sBroadleafTextureBaseFormat, stage);
            sprintf(texName, dFgDraw::sBroadleafTextureFormat, stage, variant);
            nw4r::g3d::ResTex tex = file.GetResTex(texName);
            if (tex.IsValid()) {
                mat.ReleaseTexByName(baseTexName, strlen(baseTexName));
                mat.BindTexByName(tex, baseTexName);
            }
            sprintf(paletteName, dFgDraw::sBroadleafPaletteFormat, variant);
            nw4r::g3d::ResPltt pltt = file.GetResPltt(paletteName);
            if (pltt.IsValid()) {
                mat.ReleasePlttByName(broadleafPaletteBase,
                                      strlen(broadleafPaletteBase));
                mat.BindPlttByName(pltt, broadleafPaletteBase);
            }
            sprintf(modelName, dFgDraw::sBroadleafTopModelFormat, stage,
                    variant);
            model = file.GetResMdl(modelName);
            if (model.IsValid()) {
                sprintf(matName, dFgDraw::sBroadleafTopMaterialAFormat, stage);
                mat = model.GetResMat(matName);
                if (tex.IsValid()) {
                    mat.ReleaseTexByName(baseTexName, strlen(baseTexName));
                    mat.BindTexByName(tex, baseTexName);
                }
                if (pltt.IsValid()) {
                    mat.ReleasePlttByName(broadleafPaletteBase,
                                          strlen(broadleafPaletteBase));
                    mat.BindPlttByName(pltt, broadleafPaletteBase);
                }
                sprintf(matName, dFgDraw::sBroadleafTopMaterialBFormat, stage);
                mat = model.GetResMat(matName);
                if (tex.IsValid()) {
                    mat.ReleaseTexByName(baseTexName, strlen(baseTexName));
                    mat.BindTexByName(tex, baseTexName);
                }
                if (pltt.IsValid()) {
                    mat.ReleasePlttByName(broadleafPaletteBase,
                                          strlen(broadleafPaletteBase));
                    mat.BindPlttByName(pltt, broadleafPaletteBase);
                }
            }
        }
    }
    for (int variant = 1; variant <= 2; variant++) {
        saplingTextureBase = dFgDraw::sBroadleafSaplingTextureBase;
        saplingPaletteBase = dFgDraw::sBroadleafPaletteBase;
        sprintf(modelName, dFgDraw::sBroadleafSaplingModelFormat, variant);
        model = file.GetResMdl(modelName);
        sprintf(texName, dFgDraw::sBroadleafSaplingTextureFormat, variant);
        nw4r::g3d::ResTex tex = file.GetResTex(texName);
        sprintf(paletteName, dFgDraw::sBroadleafPaletteFormat, variant);
        nw4r::g3d::ResPltt pltt = file.GetResPltt(paletteName);
        for (int part = 0; part <= 1; part++) {
            sprintf(matName, dFgDraw::sSaplingMaterialFormat, part);
            mat = model.GetResMat(matName);
            if (tex.IsValid()) {
                mat.ReleaseTexByName(saplingTextureBase,
                                     strlen(saplingTextureBase));
                mat.BindTexByName(tex, saplingTextureBase);
            }
            if (pltt.IsValid()) {
                mat.ReleasePlttByName(saplingPaletteBase,
                                      strlen(saplingPaletteBase));
                mat.BindPlttByName(pltt, saplingPaletteBase);
            }
        }
    }
    for (int variant = 1; variant <= 2; variant++) {
        deadTreeTextureBase = dFgDraw::sDeadBroadleafTextureBase;
        deadTreePaletteBase = dFgDraw::sBroadleafPaletteBase;
        sprintf(modelName, dFgDraw::sDeadBroadleafModelFormat, variant);
        model = file.GetResMdl(modelName);
        mat = model.GetResMat(dFgDraw::sDeadBroadleafMaterial);
        sprintf(texName, dFgDraw::sDeadBroadleafTextureFormat, variant);
        nw4r::g3d::ResTex tex = file.GetResTex(texName);
        if (tex.IsValid()) {
            mat.ReleaseTexByName(deadTreeTextureBase,
                                 strlen(deadTreeTextureBase));
            mat.BindTexByName(tex, deadTreeTextureBase);
        }
        sprintf(paletteName, dFgDraw::sBroadleafPaletteFormat, variant);
        nw4r::g3d::ResPltt pltt = file.GetResPltt(paletteName);
        if (pltt.IsValid()) {
            mat.ReleasePlttByName(deadTreePaletteBase,
                                  strlen(deadTreePaletteBase));
            mat.BindPlttByName(pltt, deadTreePaletteBase);
        }
    }
    for (int variant = 1; variant <= 2; variant++) {
        deadSaplingTextureBase = dFgDraw::sDeadBroadleafTextureBase;
        deadSaplingPaletteBase = dFgDraw::sBroadleafPaletteBase;
        sprintf(modelName, dFgDraw::sDeadBroadleafSaplingModelFormat, variant);
        model = file.GetResMdl(modelName);
        sprintf(texName, dFgDraw::sDeadBroadleafTextureFormat, variant);
        nw4r::g3d::ResTex tex = file.GetResTex(texName);
        sprintf(paletteName, dFgDraw::sBroadleafPaletteFormat, variant);
        nw4r::g3d::ResPltt pltt = file.GetResPltt(paletteName);
        for (int part = 0; part <= 1; part++) {
            sprintf(matName, dFgDraw::sSaplingMaterialFormat, part);
            mat = model.GetResMat(matName);
            if (tex.IsValid()) {
                mat.ReleaseTexByName(deadSaplingTextureBase,
                                     strlen(deadSaplingTextureBase));
                mat.BindTexByName(tex, deadSaplingTextureBase);
            }
            if (pltt.IsValid()) {
                mat.ReleasePlttByName(deadSaplingPaletteBase,
                                      strlen(deadSaplingPaletteBase));
                mat.BindPlttByName(pltt, deadSaplingPaletteBase);
            }
        }
    }
}

// .text 35A8
void dFgDraw_c::drawOpa() {
    mMtx_c view;
    getViewMtx(&view);
    dFgDraw::sManager.draw(view, dFgDraw::DRAW_MODEL);
    nw4r::g3d::G3DState::Invalidate(0x7FF);
    GXSetChanAmbColor(GX_COLOR0A0, dLight::getAmbColor(0));
    EGG::Fog *fog = m3d::getFogMgr()->mFog[0];
    if (fog && (fog->mFlags & EGG::Fog::FLAG_ENABLE)) {
        GXSetFog(fog->mFogType, fog->GetStartZ(), fog->GetEndZ(),
                 fog->GetNearZ(), fog->GetFarZ(), fog->mColor);
    }
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE,
                          GX_CH_ALPHA);
    dFgDraw::sManager.draw(view, dFgDraw::DRAW_TEXTURE);
}

// .text 36BC
int dFgDraw_c::create() {
    EGG::Heap *heap = dHeap::getFgHeap();
    if (!mParam || g_fgObjRes.load(heap)) {
        mAllocator.attach(heap, 32);
        m3d::proc_c::create(&mAllocator, NULL);
        mpEntries = (dFgDraw::entry_c *)heap->alloc(
            sizeof(dFgDraw::entry_c) * dFgDraw::DRAW_CAPACITY, 32);
        u32 i;
        for (i = 0; i < dFgDraw::DRAW_CAPACITY; i++) {
            new (&mpEntries[i]) dFgDraw::entry_c;
        }
        if (dFgDraw::sManager.init(mpEntries, dFgDraw::DRAW_CAPACITY,
                                   mParam != 0)) {
            setPriorityDraw(5, 127);
        }
        dFgDraw::sDesigns.load(heap);
        for (i = 0; i < dFgDraw::HIDDEN_CAPACITY; i++) {
            dFgDraw::getHiddenUnit(i)->reset();
        }
        if (mParam) {
            bindTreeResources();
        }
        return SUCCEEDED;
    }
    return NOT_READY;
}
// .text 3820
int dFgDraw_c::execute() {
    return SUCCEEDED;
}
// .text 3828
int dFgDraw_c::draw() {
    entry();
    dFgDraw::sManager.execute();
    return SUCCEEDED;
}
// .text 3864
int dFgDraw_c::doDelete() {
    if (g_fgObjRes.unload(0)) {
        dFgDraw::sDesigns.release();
        dFgDraw::sManager.release();
        m3d::scnLeaf_c::remove();
        if (dHeap::fgHeap_p) {
            ((EGG::FrmHeap *)dHeap::fgHeap_p)->free(3);
        }
        mpEntries = NULL;
        return SUCCEEDED;
    }
    return NOT_READY;
}

namespace dFgDraw {
// .text 38F0
void hiddenUnit_c::reset() {
    mX = -1;
    mZ = -1;
    mRefs = 0;
}
// .text 3908
BOOL isHidden(int x, int z) {
    hiddenUnit_c *first = getHiddenUnit(0),
                 *last = getHiddenUnit(HIDDEN_CAPACITY - 1);
    for (; first <= last; first++) {
        if (first->isAt(x, z)) {
            return TRUE;
        }
    }
    return FALSE;
}
// .text 39B8
int hideUnit(int x, int z) {
    hiddenUnit_c *first = getHiddenUnit(0),
                 *last = getHiddenUnit(HIDDEN_CAPACITY - 1);
    int index = 0;
    for (hiddenUnit_c *unit = first; unit <= last; unit++, index++) {
        if (unit->isAt(x, z)) {
            unit->mRefs++;
            return index;
        }
    }
    index = 0;
    for (; first <= last; first++, index++) {
        if (first->mX == -1 && setHidden(x, z, TRUE)) {
            first->mX = x;
            first->mZ = z;
            first->mRefs = 1;
            return index;
        }
    }
    return -1;
}
// .text 3AD4
void showUnit(int slot) {
    if (slot >= 0) {
        hiddenUnit_c *unit = getHiddenUnit(slot);
        unit->mRefs--;
        if (!unit->mRefs) {
            setHidden(unit->mX, unit->mZ, FALSE);
            unit->reset();
        }
    }
}
// .text 3B40
void getModelName(const dItem::Item &item, int x, int z, char *name) {
    strncpy(name, item.getFgInfo()->mName, ITEM_NAME_LEN + 1);
    appendTreeVariant(item, x, z, name);
}
// .text 3BB8
void appendTreeVariant(const dItem::Item &item, int x, int z, char *name) {
    u16 id = item.mId;
    if (isBroadleaf(id) || isMoneyTree(id) || id == dItem::FG_DEAD_SAPLING) {
        if ((x ^ z) & 1) {
            strncat(name, "_2", 2);
        } else {
            strncat(name, "_1", 2);
        }
    }
}
// .text 3C3C
hiddenUnit_c *getHiddenUnit(int slot) {
    return &sHidden[slot];
}
} // namespace dFgDraw

// .text 3C54
void drawUtDtcb_c::checkFloor(dBGCF::floorList_c *floors) {
    dFgDraw::setupTexture(*mpView, mTexture, mPalette, mWidth, mHeight);
    u32 count = floors->mNum;
    GXBegin(GX_TRIANGLES, GX_VTXFMT0, count * 3);
    mVec3_c ca, cb, cc;
    mVec3_c origin = mPos;
    origin.y += 1.0f;
    for (dBGCF::floor_c *floor = floors->mFloors, *end = floor + count;
         floor < end; floor++) {
        mVec3_c a = floor->mPos0, b = floor->mPos1, c = floor->mPos2;
        if (mSpecial) {
            a.y += 1.0f;
            b.y += 1.0f;
            c.y += 1.0f;
        } else {
            const char *texName =
                mTexture.ref().name
                    ? (const char *)mTexture.ptr() + mTexture.ref().name
                    : NULL;
            if (strcmp(texName, dFgDraw::sCloverTexture) == 0) {
                a.y += 0.5f;
                b.y += 0.5f;
                c.y += 0.5f;
            } else {
                a.y += 0.185f;
                b.y += 0.185f;
                c.y += 0.185f;
            }
        }
        f32 sa, sb, sc, ta, tb, tc;
        dFgDraw::calcTexCoord(&a, &sa, &ta, origin, mScale);
        dFgDraw::calcTexCoord(&b, &sb, &tb, origin, mScale);
        dFgDraw::calcTexCoord(&c, &sc, &tc, origin, mScale);
        mAng aa = dWorld::curvePositionSimple(&ca, &a),
             ab = dWorld::curvePositionSimple(&cb, &b),
             ac = dWorld::curvePositionSimple(&cc, &c);
        mVec3_c na(0.0f, 1.0f, 0.0f), nb(0.0f, 1.0f, 0.0f),
            nc(0.0f, 1.0f, 0.0f);
        na.rotX(aa);
        nb.rotX(ab);
        nc.rotX(ac);
        GXPosition3f32(cc.x, cc.y, cc.z);
        GXNormal3f32(nc.x, nc.y, nc.z);
        GXTexCoord2f32(sc, tc);
        GXPosition3f32(cb.x, cb.y, cb.z);
        GXNormal3f32(nb.x, nb.y, nb.z);
        GXTexCoord2f32(sb, tb);
        GXPosition3f32(ca.x, ca.y, ca.z);
        GXNormal3f32(na.x, na.y, na.z);
        GXTexCoord2f32(sa, ta);
    }
}
// .text 4024
fgHostIO_c::~fgHostIO_c() {
}
// .text 4064
dFgDraw_c::~dFgDraw_c() {
}
