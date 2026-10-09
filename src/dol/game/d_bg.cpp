// Background (field model) resources, namespace dBG. .text 800684A4..80069D2C, .ctors,
// .data 804A5238..804A5508, .bss 805746E0..80575EB0, .sdata 80749BE8..80749C40,
// .sbss 8074E1D8..8074E1E0, .sdata2 807503D0..807503F8. See include/game/game/d_bg.hpp and
// notes/d_bg.txt.
#include <game/game/d_bg.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_footmark.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_camera.hpp>
#include <game/mLib/m_color.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/mLib/m_vec.hpp>
#include <nw4r/g3d/res/g3d_resmdl.h>
#include <nw4r/g3d/res/g3d_resmat.h>
#include <nw4r/g3d/res/g3d_restex.h>
#include <revolution/MTX.h>
#include <cstdio>
#include <cstring>

namespace dBG {

// 8074E1D8: a byte cleared by the __sinit before the banks are built; nothing reads it. Like
// d_footmark's l_flag and d_item's s_debugFlag (whose constructor also registers it for debugging).
class flag_c {
public:
    flag_c() : mFlag(0) {}

    u8 mFlag;
};
static flag_c l_flag;

// The block model materials whose texture matrix places the block's part of the field textures.
static const char *l_texMtxMats[] = {"m_grass", "m_soil", "m_under_grass"};

// 800684A4
int bsd_c::getNum() const {
    if (mpData != NULL) {
        return mpData->mNum;
    }
    return 0;
}

// 800684C0
int bsd_c::getPoint(f32 *x, f32 *z, u32 idx) const {
    if (idx < getNum()) {
        const point_s *point = &mpData->mPoints[idx];
        *x = point->mX;
        *z = point->mZ;
        return point->mId;
    }
    return bkLoader_c::ID_NONE;
}

// 80068574
int bsd_c::getWorldPoint(f32 *x, f32 *z, int blockX, int blockZ, u32 idx) const {
    mVec3_c base;
    f32 pointX, pointZ;
    base.x = (int)(f32)(blockX * mFI_UNIT_BASE_SIZE) * UT_X_NUM;
    base.z = (int)(f32)(blockZ * mFI_UNIT_BASE_SIZE) * UT_Z_NUM;
    int id = getPoint(&pointX, &pointZ, idx);
    if (id != bkLoader_c::ID_NONE) {
        *x = pointX + base.x;
        *z = pointZ + base.z;
    }
    return id;
}

// The grass colour (TEV konstant colour 1 of "m_grass") for each season (getCurrentSeasonBres).
static mColor getGrassColor(u32 season) {
    static const mColor sColors[] = {
        mColor(0x00, 0x82, 0x40, 0xFF), mColor(0x00, 0x7B, 0x4C, 0xFF), mColor(0x00, 0x70, 0x57, 0xFF),
        mColor(0x47, 0x7A, 0x3E, 0xFF), mColor(0x7C, 0x77, 0x42, 0xFF), mColor(0x85, 0x67, 0x64, 0xFF),
        mColor(0x85, 0x61, 0x63, 0xFF), mColor(0x7B, 0x61, 0x57, 0xFF), mColor(0x61, 0x59, 0x58, 0xFF),
        mColor(0x78, 0xB0, 0xB8, 0xFF),
    };
    if (season < 10) {
        return sColors[season];
    }
    return sColors[0];
}

// 800688E0
void bkLoader_c::init() {
    mKind = KIND_NONE;
    mId = ID_NONE;
    mBlockX = 0;
    mBlockZ = 0;
    _68 = -1;
    mRequested = FALSE;
}

// 8006890C
BOOL bkLoader_c::set(int id, u32 kind, int blockX, int blockZ, u8 variant) {
    if (mId == ID_NONE) {
        mKind = kind;
        mId = id;
        mBlockX = blockX;
        mBlockZ = blockZ;
        mVariant = variant;
        return TRUE;
    }
    return FALSE;
}

// 80068940
void bkLoader_c::setTexMtx(nw4r::g3d::ResFile res, int blockX, int blockZ) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    for (u32 i = 0; i < 3; i++) {
        nw4r::g3d::ResMat mat = mdl.GetResMat(l_texMtxMats[i]);
        if (mat.IsValid()) {
            nw4r::g3d::ResTexSrt srt = mat.GetResTexSrt();
            if (srt.IsExist(0)) {
                f32 x = blockX;
                f32 z = blockZ;
                nw4r::math::MTX34 scale;
                nw4r::math::MTX34 trans;
                PSMTXScale(scale, 0.125f, 0.125f, 0.125f);
                PSMTXTrans(trans, x, z, 0.0f);
                PSMTXConcat(scale, trans, scale);
                if (srt.SetEffectMtx(0, &scale)) {
                    srt.ref().flag &= ~nw4r::g3d::TexSrt::FLAGSET_IDENTITY;
                    srt.ref().flag |= nw4r::g3d::TexSrt::FLAG_ANM_EXISTS;
                }
            }
        }
    }
}

// 80068AA8
void bkLoader_c::onLoaded() {
    nw4r::g3d::ResFile res(mpData);
    res.Init();
    res.Release();
    res.Bind();
    u32 season = getCurrentSeasonBres();
    if (season < 10) {
        nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
        if (mdl.IsValid()) {
            nw4r::g3d::ResMat mat = mdl.GetResMat("m_grass");
            if (mat.IsValid()) {
                mColor color = getGrassColor(season);
                nw4r::g3d::ResMatTevColor tev = mat.GetResMatTevColor();
                tev.GXSetTevKColor(GX_KCOLOR1, color);
                tev.EndEdit();
            }
        }
    }
    setTexMtx(res, mBlockX, mBlockZ);
}

// 80068BD0
BOOL bkLoader_c::load(void *heap, u32 kind, bkLoader_c *same) {
    if (kind == KIND_NONE) {
        kind = mKind;
    }
    if (mKind == KIND_NONE) {
        return TRUE;
    }
    if (kind != mKind) {
        return TRUE;
    }
    if (getData() != NULL) {
        return TRUE;
    }
    char path[0x28];
    snprintf(path, sizeof(path), "/BgData/BgModel/%03d_%d.brres", mId, mVariant & 1);
    mRequested = TRUE;
    return bank_c::load(path, heap, 0);
}

// 80068C90
BOOL bkLoader_c::unload() {
    if (bank_c::unload(FALSE)) {
        init();
        return TRUE;
    }
    return FALSE;
}

// 80068CDC
bsd_c *bkLoader_c::getBsd() {
    nw4r::g3d::ResFile res(mpData);
    if (res.IsValid() && res.HasExternal()) {
        void *data = res.GetExternal("sound.bsd");
        if (data != NULL) {
            mBsd.mpData = (bsd_c::data_s *)data;
            return &mBsd;
        }
    }
    return NULL;
}

// 80068D4C
void mdlBank_c::init() {
    m_heap_p = NULL;
    mBlockW = 0;
    mBlockH = 0;
    mAllLoaded = FALSE;
}

// 80068D64
bkLoader_c *mdlBank_c::find(int id, u32 variant) {
    for (int z = 0; z < mBlockH; z++) {
        for (int x = 0; x < mBlockW; x++) {
            bkLoader_c *loader = &mLoaders[z][x];
            if (id == loader->mId && variant == (loader->mVariant & 1) && loader->mRequested) {
                return loader;
            }
        }
    }
    return NULL;
}

// 80068DD8
BOOL mdlBank_c::create(EGG::Heap *heap) {
    if (m_heap_p != NULL) {
        return TRUE;
    }
    BOOL demo = isCurrentSceneAttr(SCENE_ATTR_DEMO);
    area_c area;
    area.set(getScenePlayerPos());
    m_heap_p = mHeap::createFrmHeap(0x1C8000, heap, "dBG::mdlBank_c::m_heap_p : ＢＧモデルヒープ", 0x20, mHeap::OPT_NONE);
    dFdBase_c *fd = fn_80190C44(0);
    if (m_heap_p != NULL && fd != NULL) {
        mBlockW = fd->mBlockW;
        mBlockH = fd->mBlockH;
        for (int z = 0; z < mBlockH; z++) {
            for (int x = 0; x < mBlockW; x++) {
                dFdBlock_c *block = fd->getBlock(x, z);
                if (block != NULL) {
                    int type = block->getType();
                    BOOL first = area.isIn(x, z) || demo;
                    mLoaders[z][x].set(type, first ? bkLoader_c::KIND_FIRST : bkLoader_c::KIND_SECOND, x, z,
                                       block->mFlag);
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}

// 80068F58
BOOL mdlBank_c::load(u32 kind) {
    bool ok = true;
    for (int z = 0; z < mBlockH; z++) {
        for (int x = 0; x < mBlockW; x++) {
            bkLoader_c *loader = &mLoaders[z][x];
            ok &= loader->load(m_heap_p, kind, find(loader->mId, loader->mVariant & 1));
        }
    }
    if (ok && !mAllLoaded) {
        bool done = true;
        for (int z = 0; z < mBlockH; z++) {
            for (int x = 0; x < mBlockW; x++) {
                done &= mLoaders[z][x].isLoaded();
            }
        }
        if (done) {
            onAllLoaded();
            mAllLoaded = TRUE;
        }
    }
    return ok;
}

// 80069208
void mdlBank_c::onAllLoaded() {}

// 8006920C
BOOL mdlBank_c::destroy() {
    bool ok = true;
    for (int z = 0; z < mBlockH; z++) {
        for (int x = 0; x < mBlockW; x++) {
            ok &= mLoaders[z][x].unload();
        }
    }
    if (ok) {
        if (m_heap_p != NULL) {
            mHeap::destroyFrmHeap(m_heap_p);
            m_heap_p = NULL;
        }
        init();
    }
    return ok;
}

// 800692C4
bkLoader_c *mdlBank_c::getLoader(u32 blockX, u32 blockZ) {
    if (blockX < mBlockW && blockZ < mBlockH) {
        bkLoader_c *loader = &mLoaders[blockZ][blockX];
        if (loader->isLoaded()) {
            return loader;
        }
        return NULL;
    }
    return NULL;
}

// 80069308
BOOL mdlBank_c::isReady() {
    if (isCurrentSceneAttr(SCENE_ATTR_OUTDOOR | SCENE_ATTR_MY_TOWN)) {
        return mAllLoaded;
    }
    return TRUE;
}

static const char *l_grassPath = "/BgData/Grass/grass_first.brres";

// 8006934C
BOOL grassBank_c::load(void *heap) {
    return bank_c::load(l_grassPath, heap, 0);
}

// 8006935C
u8 *grassBank_c::getTexImage(int type) {
    if (mpData != NULL) {
        nw4r::g3d::ResFile res(mpData);
        char name[0x1E];
        snprintf(name, sizeof(name), "%03d", type);
        nw4r::g3d::ResTex tex = res.GetResTex(name);
        return (u8 *)&tex.ref() + tex.ref().toTexData;
    }
    return NULL;
}

// 800693BC
packBank_c::packBank_c() : mSeason(-1) {}

// 80069400
BOOL packBank_c::load(void *heap) {
    if (mSeason == -1) {
        mSeason = getCurrentSeasonBres();
    }
    if (mSeason == -1) {
        mSeason = 0;
    }
    char path[0x32];
    dSaveTown_c *town = dSaveData_c::getTown();
    u8 grassType = town->mMainField.mGrassType % 3u;
    snprintf(path, sizeof(path), "/BgData/Pack/pat%dseason%02d.brres", grassType, mSeason);
    if (bank_c::load(path, heap, 0)) {
        return TRUE;
    }
    return FALSE;
}

// 800694C8
BOOL packBank_c::unload() {
    if (bank_c::unload(FALSE)) {
        mSeason = -1;
        return TRUE;
    }
    return FALSE;
}

// 80069514
void packBank_c::onLoaded() {
    brresBank_c::onLoaded();
    nw4r::g3d::ResFile res(mpData);
    nw4r::g3d::ResTex tex = res.GetResTex("grassBrd");
    tex.ref().width = dFootmark::TEX_SIZE;
    tex.ref().height = dFootmark::TEX_SIZE;
    u8 *pixels = dFootmark::getEditor()->getPixels();
    tex.ref().toTexData = pixels - (u8 *)&tex.ref();
    tex.DCStore(false);
    tex.Init();
}

static const char *l_roomAlwaysPath = "/BgData/Always/roomAlways.arc";

// 800695BC
BOOL roomAlwaysBank_c::load(void *heap) {
    return bank_c::load(l_roomAlwaysPath, heap, 0);
}

// 800695CC
void roomAlwaysBank_c::onLoaded() {
    arcBank_c::onLoaded();
    u32 gradeSize;
    nw4r::g3d::ResFile grade(getFile("roomAlways/gradeTex.brres", &gradeSize));
    grade.Init();
    grade.Bind(grade);
    u32 sunSize;
    nw4r::g3d::ResFile sun(getFile("roomAlways/sunAnm.brres", &sunSize));
    sun.Init();
    sun.Bind(sun);
}

static const char *l_alwaysPath = "/BgData/Always/always.arc";

// 80069670
BOOL alwaysBank_c::load(void *heap) {
    return bank_c::load(l_alwaysPath, heap, 0);
}

// 80069680
BOOL alwaysBank_c::copyBlockCol(void *dst, int type) {
    dBGCF::unitDat_c *src = getBlockCol(type);
    if (src != NULL) {
        memcpy(dst, src, UT_TOTAL_NUM * sizeof(dBGCF::unitDat_c));
        return TRUE;
    }
    return FALSE;
}

// 800696D4
dBGCF::unitDat_c *alwaysBank_c::getBlockCol(int type) {
    u32 size;
    u8 *data = (u8 *)getFile("always/blockCol.bin", &size);
    if (data != NULL && type < 241) {
        data += type * (UT_TOTAL_NUM * sizeof(dBGCF::unitDat_c));
    }
    return (dBGCF::unitDat_c *)data;
}

// 80069724
void alwaysBank_c::onLoaded() {
    arcBank_c::onLoaded();
    u32 size;
    nw4r::g3d::ResFile shadow(getFile("always/shadow.brres", &size));
    shadow.Init();
    shadow.Bind(shadow);
}

// 80069794
void area_c::set(const mVec3_c *center) {
    f32 halfX = 4.0f / 3.0f * (0.7f * lbl_80750520);
    f32 z = center->z;
    f32 y = center->y;
    f32 x = center->x;
    mVec3_c min(x - halfX, y, z - 1.33f * lbl_80750524);
    mVec3_c max(min);
    max.x = x + halfX;
    max.z = z + 0.49f * lbl_80750524;
    dBGCF::posToBlock(&mMinX, &mMinZ, &min);
    dBGCF::posToBlock(&mMaxX, &mMaxZ, &max);
}

// 80069834
void area_c::setView() {
    mVec3_c center(*getScenePlayerPos());
    center = lbl_80623FEC;
    set(&center);
}

// 8006989C
BOOL area_c::isIn(int blockX, int blockZ) const {
    BOOL in = FALSE;
    if (blockX >= mMinX && blockX <= mMaxX && blockZ >= mMinZ && blockZ <= mMaxZ) {
        in = TRUE;
    }
    return in;
}

// 800698DC
u8 area_c::getPriority(int blockX, int blockZ, const mVec3_c *pos) const {
    int posX, posZ;
    dBGCF::posToBlock(&posX, &posZ, pos);
    if (blockZ == BLOCK_Z_NUM - 1) {
        return 17;
    }
    if (posX == blockX) {
        return 13 - (blockZ - mMaxZ);
    }
    return 9 - (blockZ - mMaxZ);
}

const f32 cGroundY = 56.0f;

static mdlBank_c l_mdlBank;
static alwaysBank_c l_alwaysBank;
static packBank_c l_packBank;

// 8006996C
mdlBank_c *getMdlBank() {
    return &l_mdlBank;
}

// 80069978
alwaysBank_c *getAlwaysBank() {
    return &l_alwaysBank;
}

// 80069984
packBank_c *getPackBank() {
    return &l_packBank;
}

// 80069990
roomAlwaysBank_c *getRoomAlwaysBank() {
    return NULL;
}

// 80069998
bkLoader_c::~bkLoader_c() {}

// 800699F8
packBank_c::~packBank_c() {}

// 80069A58
alwaysBank_c::~alwaysBank_c() {}

// 80069AB8
roomAlwaysBank_c::~roomAlwaysBank_c() {}

} // namespace dBG
