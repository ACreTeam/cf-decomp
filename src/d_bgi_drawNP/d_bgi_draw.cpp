#include <game/game/d_bgi_draw.hpp>
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_bg_util.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_environment.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_hr.hpp>
#include <game/game/d_model_room.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_building.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_shop_grace.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_scene_check.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_str.hpp>
#include <game/mLib/m_3d/global.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/snd/snd_id.hpp>
#include <cstring>

// Debug parameters (RTTI "dBgiDraw_hostIO_c").
class dBgiDraw_hostIO_c {
  public:
    dBgiDraw_hostIO_c() {
        mWallTransX = mWallTransY = mCarpetTransX = mCarpetTransY = 0.0f;
        mCarpetScaleY = mCarpetScaleX = mWallScaleY = mWallScaleX = 1.0f;
        _2A = _24 = _2C = _26 = 0;
        _2E = _28 = 1;
        mDebugDraw = 0;
        mDrawSndPos = 0;
        _32 = 1;
        mRiverAlpha = 0x40;
        mCullAngle = 0x479;
    }
    virtual ~dBgiDraw_hostIO_c();

    /* 0x04 */ f32 mWallTransX;
    /* 0x08 */ f32 mWallTransY;
    /* 0x0C */ f32 mWallScaleX;
    /* 0x10 */ f32 mWallScaleY;
    /* 0x14 */ f32 mCarpetTransX;
    /* 0x18 */ f32 mCarpetTransY;
    /* 0x1C */ f32 mCarpetScaleX;
    /* 0x20 */ f32 mCarpetScaleY;
    /* 0x24 */ s16 _24;
    /* 0x26 */ s16 _26;
    /* 0x28 */ s16 _28;
    /* 0x2A */ s16 _2A;
    /* 0x2C */ s16 _2C;
    /* 0x2E */ s16 _2E;
    /* 0x30 */ u8 mDebugDraw;
    /* 0x31 */ u8 mDrawSndPos;
    /* 0x32 */ u8 _32;
    /* 0x33 */ u8 mRiverAlpha; // KColor1 alpha of the river material
    /* 0x34 */ int mCullAngle; // camera yaw limit of the culling nodes
}; // size 0x38

namespace dBgiDraw {

// .text:0xCC
void *create() {
    return new dBgiDraw_c;
}

fProf::fBaseProfile_c g_profile_BGI_DRAW = {&create, fProf::BGI_DRAW,
                                            fProf::DRAW_ORDER::BGI_DRAW};

// The wallpaper / carpet materials and the texture / palette slots they
// replace.
static const char *l_wallMat = "m_dummy_wall";
static const char *l_wallTex = "dummy_wall";
static const char *l_wallPltt = "pl_dummy_wall";
static const char *l_carpetMat = "m_dummy_carpet";
static const char *l_carpetTex = "dummy_carpet";
static const char *l_carpetPltt = "pl_dummy_carpet";

// The scenes of the four player houses, then the room type of each house size.
static const u32 l_houseAttrs[] = {
    dScene::SCENE_ATTR_HOUSE0 | dScene::SCENE_ATTR_PLAYER_HOUSE,
    dScene::SCENE_ATTR_HOUSE1 | dScene::SCENE_ATTR_PLAYER_HOUSE,
    dScene::SCENE_ATTR_HOUSE2 | dScene::SCENE_ATTR_PLAYER_HOUSE,
    dScene::SCENE_ATTR_HOUSE3 | dScene::SCENE_ATTR_PLAYER_HOUSE,
    0,
};
static const int l_houseSizeTypes[] = {0, 1, 2, 2, 2};

// Scale x, y and translation x, y of an original design's texture by room type.
static const f32 l_wallSrt[][4] = {
    {2.0f, 4.0f, 0.0f, -0.25f},
    {2.0f, 4.0f, 0.0f, -0.25f},
    {2.0f, 4.0f, 0.0f, -0.25f},
};
static const f32 l_carpetSrt[][4] = {
    {2.0f, 2.0f, 0.0f, -0.5f},
    {3.0f, 3.0f, 0.0f, 0.0f},
    {4.0f, 4.0f, 0.0f, 0.25f},
};

static dBgiDraw_hostIO_c l_HIO;

// .text:0x2E8
static void getTexSrt(f32 *scaleX, f32 *scaleY, f32 *transX, f32 *transY,
                      dItem::Item item, BOOL isWall) {
    *scaleX = 1.0f;
    *scaleY = 1.0f;
    *transX = 0.0f;
    *transY = 0.0f;
    if (isWall) {
        l_HIO.mWallScaleX = 1.0f;
        l_HIO.mWallScaleY = 1.0f;
        l_HIO.mWallTransX = 0.0f;
        l_HIO.mWallTransY = 0.0f;
    } else {
        l_HIO.mCarpetScaleX = 1.0f;
        l_HIO.mCarpetScaleY = 1.0f;
        l_HIO.mCarpetTransX = 0.0f;
        l_HIO.mCarpetTransY = 0.0f;
    }
    int house;
    if (dScene::getSceneAttrTableIndex(dScene::getCurrentScene(), l_houseAttrs,
                                       &house) != -1 &&
        house != -1) {
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
        if (bitm != NULL && bitm->getKind() == dItem::KIND_ORG_WC) {
            int room;
            int home;
            int type = 1;
            home =
                dHomeList_c::getHomeFromScene(&room, dScene::getCurrentScene());
            if (home != -1) {
                u32 size = dSaveData_c::getTown()->mHomes.getHome(home)->mSize;
                switch (room) {
                case 0:
                    if (size < 5) {
                        type = l_houseSizeTypes[size];
                    }
                    break;
                case 1:
                    type = 1;
                    break;
                case 2:
                    type = 2;
                    break;
                }
            }
            const f32(*srt)[4] = isWall ? l_wallSrt : l_carpetSrt;
            if (isWall) {
                *scaleX = l_HIO.mWallScaleX = srt[type][0];
                *scaleY = l_HIO.mWallScaleY = srt[type][1];
                *transX = l_HIO.mWallTransX = srt[type][2];
                *transY = l_HIO.mWallTransY = srt[type][3];
            } else {
                *scaleX = l_HIO.mCarpetScaleX = srt[type][0];
                *scaleY = l_HIO.mCarpetScaleY = srt[type][1];
                *transX = l_HIO.mCarpetTransX = srt[type][2];
                *transY = l_HIO.mCarpetTransY = srt[type][3];
            }
        }
    }
}

} // namespace dBgiDraw

// .text:0x510
BOOL dBgiDraw_c::loader_c::load(dItem::Item item) {
    if (dItem::infoBank_c::get()->getBITM(item)->getKind() ==
        dItem::KIND_ORG_WC) {
        u32 idx = dItem::seeker_c::get()->findLike(item);
        dPrivateData_c *player = dPrivateData_c::getChecked(
            dSaveData_c::getTown()->mPlayers, idx >> 3);
        return loadDesignInPlace(&player->mOrgDesigns.mDesigns[idx & 7],
                                 mpFrmHeap, 0x20, 0x20);
    }
    return loadItem(item, mpFrmHeap);
}

// .text:0x5E4
nw4r::g3d::ResTex dBgiDraw_c::loader_c::getTex() {
    return nw4r::g3d::ResFile(mpData).GetResTex(0);
}

// .text:0x614
nw4r::g3d::ResPltt dBgiDraw_c::loader_c::getPltt() {
    return nw4r::g3d::ResFile(mpData).GetResPltt(0);
}

// .text:0x644
void dBgiDraw_c::loader_c::freeHeap() {
    if (mpFrmHeap != NULL) {
        mpFrmHeap->free(3);
    }
}

// .text:0x65C
nw4r::g3d::ResFile dBgiDraw_c::getResFile() {
    return nw4r::g3d::ResFile(mpData);
}

// .text:0x664
nw4r::g3d::ResMdl dBgiDraw_c::getResMdl() {
    return getResFile().GetResMdl(0);
}

// .text:0x694
nw4r::g3d::ResMat dBgiDraw_c::getResMat(const char *name) {
    return getResMdl().GetResMat(name);
}

// .text:0x6D0
dBgiDraw_c::request_c::request_c() {
    mPending = dItem::ITEM_ID_NONE;
    mCurrent = dItem::ITEM_ID_NONE;
    mFlag = 0;
}

// .text:0x784
void dBgiDraw_c::request_c::init() {
    mLoaded = 0;
    mBuf = 0;
    mCurrent = dItem::ITEM_ID_NONE;
    finish();
}

// .text:0x7A0
bool dBgiDraw_c::request_c::createHeap(EGG::Heap *parent, u32 size) {
    bool ok = true;
    for (u32 i = 0; i < LOADER_NUM; i++) {
        loader_c *loader = getLoader(i);
        if (loader->mpFrmHeap == NULL) {
            loader->mpFrmHeap = mHeap::createFrmHeap(
                size, parent, "dBgiDraw_c::request_c::createHeap::loader", 0x20,
                mHeap::OPT_NONE);
            ok &= loader->mpFrmHeap != NULL;
        }
    }
    return ok;
}

// .text:0x844
void dBgiDraw_c::request_c::destroyHeap() {
    for (u32 i = 0; i < LOADER_NUM; i++) {
        loader_c *loader = getLoader(i);
        if (loader->mpFrmHeap != NULL) {
            mHeap::destroyFrmHeap(loader->mpFrmHeap);
            loader->mpFrmHeap = NULL;
        }
    }
}

// .text:0x8B8
BOOL dBgiDraw_c::request_c::set(dItem::Item item, int wrap, int flag,
                                int playSe) {
    if (item.getId() != dItem::ITEM_ID_NONE) {
        int kind = dItem::infoBank_c::get()->getBITM(item)->getKind();
        mPlaySe = playSe;
        mPending = item;
        if (kind == dItem::KIND_WALL) {
            mWrap = GX_REPEAT;
        } else if (kind == dItem::KIND_CARPET) {
            if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_STORE_ROOM)) {
                mWrap = GX_MIRROR;
            } else {
                mWrap = GX_MIRROR;
            }
        } else {
            mWrap = wrap;
        }
        mFlag = flag;
        return TRUE;
    }
    return FALSE;
}

// .text:0x97C
void dBgiDraw_c::request_c::finish() {
    mCurrent = mPending;
    mPending = dItem::ITEM_ID_NONE;
    mWrap = GX_REPEAT;
    mPlaySe = 0;
}

// .text:0x9A4
bool dBgiDraw_c::request_c::release() {
    bool ok = true;
    for (u32 i = 0; i < LOADER_NUM; i++) {
        ok &= getLoader(i)->release();
    }
    return ok;
}

// .text:0xA18
void dBgiDraw_c::request_c::playSe() {
    if (mPlaySe) {
        ::playSe(SE_DESIGN_CHANGE);
    }
}

// .text:0xA30
void dBgiDraw_c::request_c::next() {
    finish();
    mBuf++;
}

// .text:0xA68
dItem::Item dBgiDraw_c::getWallpaper() const {
    dItem::Item item = mWallReq.getCurrent();
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
    if (bitm != NULL && bitm->getKind() == dItem::KIND_ORG_WC) {
        return dItem::ITEM_ID_NONE;
    }
    return item;
}

// .text:0xAF0
dItem::Item dBgiDraw_c::getCarpet() const {
    dItem::Item item = mCarpetReq.getCurrent();
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
    if (bitm != NULL && bitm->getKind() == dItem::KIND_ORG_WC) {
        return dItem::ITEM_ID_NONE;
    }
    return item;
}

// .text:0xB78
dItem::Item dBgiDraw_c::getCarpetRaw() const {
    return mCarpetReq.getCurrent();
}

namespace dBgiDraw {

// .text:0xB80
dItem::Item getStoreWallpaper() {
    int idx = dScene::getSceneAttrIndex(dScene::getCurrentScene(),
                                        dScene::SCENE_ATTR_STORE_ROOM);
    if (idx != -1) {
        dItem::Item items[] = {dItem::Item(0x315), dItem::Item(0x316),
                               dItem::Item(0x317), dItem::Item(),
                               dItem::Item(0x318)};
        return items[idx];
    }
    return dItem::ITEM_ID_NONE;
}

// .text:0xC28
dItem::Item getSceneWallpaper(int *wrap) {
    int tmp;
    if (wrap == NULL) {
        wrap = &tmp;
    }
    u8 scene = dScene::getCurrentScene();
    dItem::Item item = dBgUtil::getSceneWallpaper(scene, wrap);
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_VILLAGER_HOUSE)) {
        int idx =
            dScene::getSceneAttrIndex(scene, dScene::SCENE_ATTR_VILLAGER_HOUSE);
        dAnimal_c *animal =
            dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
        if (animal != NULL) {
            return *animal->getWall();
        }
        return dItem::Item(0x2BE, 1, TRUE);
    }
    if (dScene::getCurrentScene() == dScene::SCENE_RM_HAPPY_MDL) {
        dSaveTown_c *town = dSaveData_c::getTown();
        return town->mModelRoom.getWallpaper();
    }
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_STORE_ROOM) &&
        item.getId() == dItem::ITEM_ID_NONE) {
        return getStoreWallpaper();
    }
    if (scene == dScene::SCENE_RM_GRACE) {
        return getGraceWallpaper();
    }
    return item;
}

// .text:0xD60
dItem::Item getGraceItem(int kind) {
    dSaveShopGrace_c *grace = &dSaveData_c::getTown()->mShops.mShopGrace;
    for (int i = 0; i < 0x17; i++) {
        dItem::Item item = grace->getItem(i, 0);
        if (dItem::isRealItemId(item.getId()) && kind == item.getKind()) {
            return item;
        }
    }
    dItem::seeker_c::get()->search(kind, 6, NULL);
    return dItem::seeker_c::get()->getNth(0);
}

// .text:0xE3C
dItem::Item getGraceCarpet() {
    return getGraceItem(dItem::KIND_CARPET);
}

// .text:0xE44
dItem::Item getGraceWallpaper() {
    return getGraceItem(dItem::KIND_WALL);
}

// .text:0xE4C
dItem::Item getStoreCarpet() {
    int idx = dScene::getSceneAttrIndex(dScene::getCurrentScene(),
                                        dScene::SCENE_ATTR_STORE_ROOM);
    if (idx != -1) {
        dItem::Item items[] = {dItem::Item(0x36E), dItem::Item(0x36F),
                               dItem::Item(0x370), dItem::Item(),
                               dItem::Item(0x371)};
        return items[idx];
    }
    return dItem::ITEM_ID_NONE;
}

// .text:0xEF4
dItem::Item getSceneCarpet(int *wrap) {
    int tmp;
    if (wrap == NULL) {
        wrap = &tmp;
    }
    u8 scene = dScene::getCurrentScene();
    dItem::Item item = dBgUtil::getSceneCarpet(scene, wrap);
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_VILLAGER_HOUSE)) {
        int idx =
            dScene::getSceneAttrIndex(scene, dScene::SCENE_ATTR_VILLAGER_HOUSE);
        dAnimal_c *animal =
            dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
        if (animal != NULL) {
            return *animal->getCarpet();
        }
        return dItem::Item(0x319, 1, TRUE);
    }
    if (dScene::getCurrentScene() == dScene::SCENE_RM_HAPPY_MDL) {
        dSaveTown_c *town = dSaveData_c::getTown();
        return town->mModelRoom.getCarpet();
    }
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_STORE_ROOM) &&
        item.getId() == dItem::ITEM_ID_NONE) {
        return getStoreCarpet();
    }
    if (scene == dScene::SCENE_RM_GRACE) {
        return getGraceCarpet();
    }
    return item;
}

} // namespace dBgiDraw

// .text:0x102C
u32 dBgiDraw_c::getTexIdx(nw4r::g3d::ResMat mat, const char *name) {
    nw4r::g3d::ResMat m = mat;
    for (u32 i = 0; i < m.GetNumResTexPlttInfo(); i++) {
        nw4r::g3d::ResTexPlttInfo info = m.GetResTexPlttInfo(i);
        if (strcmp(info.ofs_to_ptr<char>(info.ref().nameTex), name) == 0) {
            return i;
        }
    }
    return 0;
}

// .text:0x10E0
void dBgiDraw_c::setTexSrt(nw4r::g3d::ResMat mat, const char *name, f32 scaleX,
                           f32 scaleY, f32 transX, f32 transY) {
    nw4r::g3d::ResTexSrt srt = mat.GetResTexSrt();
    u32 idx = getTexIdx(mat, name);
    if (srt.IsExist(idx)) {
        nw4r::math::MTX34 scale;
        nw4r::math::MTX34 trans;
        PSMTXScale(scale, scaleX, scaleY, scaleX);
        PSMTXTrans(trans, transX, transY, 0.0f);
        PSMTXConcat(scale, trans, scale);
        if (srt.SetEffectMtx(idx, &scale)) {
            srt.ref().flag &= ~(nw4r::g3d::TexSrt::FLAGSET_IDENTITY
                                << idx * nw4r::g3d::TexSrt::NUM_OF_FLAGS);
            srt.ref().flag |= nw4r::g3d::TexSrt::FLAG_ANM_EXISTS
                              << idx * nw4r::g3d::TexSrt::NUM_OF_FLAGS;
        }
    }
}

// .text:0x1238
void dBgiDraw_c::setTex(nw4r::g3d::ResTex tex, nw4r::g3d::ResPltt pltt,
                        int wrapS, int wrapT, const char *matName,
                        const char *texName, const char *plttName, f32 scaleX,
                        f32 scaleY, f32 transX, f32 transY) {
    nw4r::g3d::ResMat mat = getResMat(matName);
    mat.ReleaseTexByName(texName, strlen(texName));
    mat.ReleasePlttByName(plttName, strlen(plttName));
    if (mIsPlayerHouse) {
        nw4r::g3d::ResTexPlttInfo info =
            mat.GetResTexPlttInfo(getTexIdx(mat, texName));
        info.ref().wrap_s = (GXTexWrapMode)wrapS;
        info.ref().wrap_t = (GXTexWrapMode)wrapT;
    }
    mat.BindTexByName(tex, texName);
    mat.BindPlttByName(pltt, plttName);
    setTexSrt(mat, texName, scaleX, scaleY, transX, transY);
}

// .text:0x1384
void dBgiDraw_c::setWallTex(nw4r::g3d::ResTex tex, nw4r::g3d::ResPltt pltt,
                            int wrapS, int wrapT, f32 scaleX, f32 scaleY,
                            f32 transX, f32 transY) {
    setTex(tex, pltt, wrapS, wrapT, dBgiDraw::l_wallMat, dBgiDraw::l_wallTex,
           dBgiDraw::l_wallPltt, scaleX, scaleY, transX, transY);
}

// .text:0x13D0
void dBgiDraw_c::setCarpetTex(nw4r::g3d::ResTex tex, nw4r::g3d::ResPltt pltt,
                              int wrapS, int wrapT, f32 scaleX, f32 scaleY,
                              f32 transX, f32 transY) {
    setTex(tex, pltt, wrapS, wrapT, dBgiDraw::l_carpetMat,
           dBgiDraw::l_carpetTex, dBgiDraw::l_carpetPltt, scaleX, scaleY,
           transX, transY);
}

// .text:0x141C
BOOL dBgiDraw_c::loadRoomItems() {
    int wrap;
    f32 scaleX, scaleY, transX, transY;
    if (!mWallReq.mLoaded) {
        const dItem::Item item = dBgiDraw::getSceneWallpaper(&wrap);
        if (!item.isValid()) {
            mWallReq.mLoaded = 1;
            mWallReq.setCurrent(item);
        } else {
            loader_c *loader = mWallReq.getLoader(0);
            if (loader->load(item)) {
                dBgiDraw::getTexSrt(&scaleX, &scaleY, &transX, &transY, item,
                                    TRUE);
                int wrapT =
                    item.getKind() == dItem::KIND_WALL ? GX_MIRROR : wrap;
                setWallTex(loader->getTex(), loader->getPltt(), wrap, wrapT,
                           scaleX, scaleY, transX, transY);
                mWallReq.mLoaded = 1;
                mWallReq.setCurrent(item);
                mWallFlag = mWallReq.mFlag;
            }
        }
    }
    if (!mCarpetReq.mLoaded) {
        const dItem::Item item = dBgiDraw::getSceneCarpet(&wrap);
        if (!item.isValid()) {
            mCarpetReq.mLoaded = 1;
            mCarpetReq.setCurrent(item);
        } else {
            loader_c *loader = mCarpetReq.getLoader(0);
            if (loader->load(item)) {
                dBgiDraw::getTexSrt(&scaleX, &scaleY, &transX, &transY, item,
                                    FALSE);
                setCarpetTex(loader->getTex(), loader->getPltt(), wrap, wrap,
                             scaleX, scaleY, transX, transY);
                mCarpetReq.mLoaded = 1;
                mCarpetReq.setCurrent(item);
                mCarpetFlag = mCarpetReq.mFlag;
            }
        }
    }
    return mWallReq.mLoaded && mCarpetReq.mLoaded;
}

// .text:0x1660
void dBgiDraw_c::calcRequests() {
    f32 scaleX, scaleY, transX, transY;
    if (mWallReq.mPending.isValid()) {
        dItem::Item item = mWallReq.getPending();
        loader_c *old = mWallReq.getLoader(mWallReq.mBuf);
        loader_c *loader = mWallReq.getLoader(mWallReq.mBuf + 1);
        if (loader->load(item)) {
            dBgUtil::setSceneWallpaper(dScene::getCurrentScene(), item,
                                       mWallReq.mWrap);
            dBgiDraw::getTexSrt(&scaleX, &scaleY, &transX, &transY, item, TRUE);
            int wrapT =
                item.getKind() == dItem::KIND_WALL ? GX_MIRROR : mWallReq.mWrap;
            int wrapS = mWallReq.getWrap();
            setWallTex(loader->getTex(), loader->getPltt(), wrapS, wrapT,
                       scaleX, scaleY, transX, transY);
            int kind = item.getKind();
            if (kind == dItem::KIND_WALL || kind == dItem::KIND_CARPET) {
                mWallReq.playSe();
            }
            while (!old->release()) {
            }
            old->freeHeap();
            mWallReq.next();
            mWallFlag = mWallReq.mFlag;
        }
    } else {
        dItem::Item cur = getWallpaper();
        int wrap;
        dItem::Item item = dBgiDraw::getSceneWallpaper(&wrap);
        if (dItem::isRealItemId(cur.getId()) && cur.isNotSame(item)) {
            dBgUtil::setWallpaper(item, wrap, 0, 0, TRUE);
        }
    }
    if (mCarpetReq.mPending.isValid()) {
        dItem::Item item = mCarpetReq.getPending();
        int wrap;
        loader_c *old = mCarpetReq.getLoader(mCarpetReq.mBuf);
        loader_c *loader = mCarpetReq.getLoader(mCarpetReq.mBuf + 1);
        if (loader->load(item)) {
            dBgUtil::setSceneCarpet(dScene::getCurrentScene(), item,
                                    mCarpetReq.mWrap);
            dBgiDraw::getTexSrt(&scaleX, &scaleY, &transX, &transY, item,
                                FALSE);
            wrap = mCarpetReq.mWrap;
            setCarpetTex(loader->getTex(), loader->getPltt(), wrap, wrap,
                         scaleX, scaleY, transX, transY);
            int kind = item.getKind();
            if (kind == dItem::KIND_WALL || kind == dItem::KIND_CARPET) {
                mCarpetReq.playSe();
            }
            while (!old->release()) {
            }
            old->freeHeap();
            mCarpetReq.next();
            mCarpetFlag = mCarpetReq.mFlag;
        }
    } else {
        dItem::Item cur = getCarpet();
        int wrap;
        dItem::Item item = dBgiDraw::getSceneCarpet(&wrap);
        if (dItem::isRealItemId(cur.getId()) && cur.isNotSame(item)) {
            dBgUtil::setCarpet(item, wrap, 0, 0, TRUE);
        }
    }
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_STORE_ROOM)) {
        static const char *wallMat = "m_dummy_wall";
        static const char *carpetMat = "m_dummy_carpet";
        nw4r::g3d::ResMdl mdl = getResMdl();
        nw4r::g3d::ScnMdl *scnMdl =
            nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(
                nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(
                    mMdl.getScn()));
        {
            int id = m3d::getMatID(mdl, wallMat);
            if (id != -1) {
                nw4r::g3d::ScnMdl::CopiedMatAccess cma(scnMdl, id);
                if (cma.IsValid()) {
                    nw4r::g3d::ResMatTevColor tev =
                        cma.GetResMatTevColor(false);
                    GXColor color;
                    if (tev.GXGetTevKColor(GX_KCOLOR1, &color)) {
                        color.a = mWallFlag ? 0 : 0xFF;
                        tev.GXSetTevKColor(GX_KCOLOR1, color);
                        tev.DCStore(false);
                    }
                } else {
                    nw4r::g3d::ResMatTevColor tev =
                        mdl.GetResMat(wallMat).GetResMatTevColor();
                    GXColor color;
                    if (tev.GXGetTevKColor(GX_KCOLOR1, &color)) {
                        color.a = mWallFlag ? 0 : 0xFF;
                        tev.GXSetTevKColor(GX_KCOLOR1, color);
                        tev.DCStore(false);
                    }
                }
            }
        }
        {
            int id = m3d::getMatID(mdl, carpetMat);
            if (id != -1) {
                nw4r::g3d::ScnMdl::CopiedMatAccess cma(scnMdl, id);
                if (cma.IsValid()) {
                    nw4r::g3d::ResMatTevColor tev =
                        cma.GetResMatTevColor(false);
                    GXColor color;
                    if (tev.GXGetTevKColor(GX_KCOLOR1, &color)) {
                        color.a = mCarpetFlag ? 0 : 0xFF;
                        tev.GXSetTevKColor(GX_KCOLOR1, color);
                        tev.DCStore(false);
                    }
                } else {
                    nw4r::g3d::ResMatTevColor tev =
                        mdl.GetResMat(carpetMat).GetResMatTevColor();
                    GXColor color;
                    if (tev.GXGetTevKColor(GX_KCOLOR1, &color)) {
                        color.a = mCarpetFlag ? 0 : 0xFF;
                        tev.GXSetTevKColor(GX_KCOLOR1, color);
                        tev.DCStore(false);
                    }
                }
            }
        }
    }
}

// .text:0x1D14
bool dBgiDraw_c::releaseRequests() {
    bool ok = true;
    ok &= mWallReq.release();
    ok &= mCarpetReq.release();
    return ok;
}

// .text:0x1D68
void dBgiDraw_c::debugDraw() {
}

// .text:0x1D6C
void dBgiDraw_c::initHeap() {
    if (m_frmHeap_p == NULL) {
        EGG::Heap *parent = dHeap::localHeap_p;
        m_frmHeap_p = mHeap::createFrmHeap(
            0x4000, parent, "dBgiDraw_c::m_frmHeap_p", 0x20, mHeap::OPT_NONE);
        if (m_frmHeap_p != NULL) {
            mAllocator.attach(m_frmHeap_p, 0x20);
            init();
        }
        mWallReq.createHeap(parent, 0x2780);
        mCarpetReq.createHeap(parent, 0x4780);
    }
}

// .text:0x1E10
void dBgiDraw_c::deleteHeap() {
    if (m_frmHeap_p != NULL) {
        mHeap::destroyFrmHeap(m_frmHeap_p);
        m_frmHeap_p = NULL;
    }
    mWallReq.destroyHeap();
    mCarpetReq.destroyHeap();
}

// .text:0x1E64
BOOL dBgiDraw_c::getNodePos(mVec3_c *pos, int blockX, int blockZ,
                            const char *name) {
    if (blockX == 0 && blockZ == 0) {
        if (mMdl.getScn() != NULL) {
            int id = m3d::getNodeID(mMdl.getResMdl(), name);
            if (id != -1) {
                mMtx_c mtx;
                mMdl.getNodeWorldMtx(id, &mtx);
                PSMTXMultVec(mtx, mVec3_c::Zero, *pos);
                return TRUE;
            }
        }
        return FALSE;
    }
    return mBlocks.getNodePos(pos, blockX, blockZ, name);
}

// .text:0x1F24
int dBgiDraw_c::getNodeNum(int blockX, int blockZ) {
    if (blockX == 0 && blockZ == 0) {
        if (mMdl.getScn() != NULL) {
            nw4r::g3d::ResMdl mdl = mMdl.getResMdl();
            if (mdl.IsValid()) {
                return mdl.GetResNodeNumEntries();
            }
        }
        return 0;
    }
    return mBlocks.getNodeNum(blockX, blockZ);
}

// .text:0x1F8C
const char *dBgiDraw_c::getNodeName(u32 idx, int blockX, int blockZ,
                                    mVec3_c *pos) {
    const char *name = NULL;
    if (blockX == 0 && blockZ == 0) {
        if (mMdl.getScn() != NULL) {
            nw4r::g3d::ResMdl mdl = mMdl.getResMdl();
            if (mdl.IsValid()) {
                name = mdl.GetResNode(idx).GetName();
            }
        }
    } else {
        name = mBlocks.getNodeName(idx, blockX, blockZ);
    }
    if (name != NULL) {
        if (pos != NULL) {
            getNodePos(pos, blockX, blockZ, name);
        }
        return name;
    }
    return NULL;
}

// .text:0x2080
BOOL dBgiDraw_c::isReady() {
    if (mMdl.getResMdl().IsValid()) {
        return mBlocks.isReady();
    }
    return FALSE;
}

namespace dBgiDraw {

// Shows the "culling_right" / "culling_left" nodes.
// .text:0x20C8
static void setCullingNodes(nw4r::g3d::ResMdl mdl, bool right, bool left) {
    nw4r::g3d::ResNode node = mdl.GetResNode("culling_right");
    if (node.IsValid()) {
        node.SetVisibility(right);
    }
    node = mdl.GetResNode("culling_left");
    if (node.IsValid()) {
        node.SetVisibility(left);
    }
}

// .text:0x2184
static void setCulling(nw4r::g3d::ResMdl mdl, mAng yaw) {
    s16 angle = yaw;
    int limit;
    if (dScene::getCurrentScene() == dScene::SCENE_RM_BARBER) {
        limit = 4000;
    } else {
        limit = l_HIO.mCullAngle;
    }
    setCullingNodes(mdl, angle <= limit, angle >= -limit);
}

} // namespace dBgiDraw

// .text:0x2210
void dBgiDraw_c::sunNodeCallBack_c::timingA(ulong nodeId,
                                            nw4r::g3d::ChrAnmResult *anmRes,
                                            nw4r::g3d::ResMdl resMdl) {
    nw4r::g3d::ResNode node = resMdl.GetResNode(nodeId);
    mMtx_c mtx;
    anmRes->GetRotTrans(mtx);
    const nw4r::math::VEC3 &trans = node.GetTranslate();
    mtx._03 = trans.x;
    mtx._13 = trans.y;
    mtx._23 = trans.z;
    anmRes->SetRotTrans(mtx);
}

namespace dBgiDraw {

// .text:0x2288
static bool hasField() {
    return dField::getInfo(0) != NULL;
}

} // namespace dBgiDraw

// .text:0x22B8
void dBgiDraw_c::init() {
    dFdBase_c *info = dField::getInfo(0);
    mBgId = 0xB8;
    if (info != NULL) {
        mBgId = info->getBlock(0, 0)->getType();
    }
    mWallReq.init();
    mCarpetReq.init();
}

// .text:0x231C
int dBgiDraw_c::create() {
    mHasField = dBgiDraw::hasField();
    if (!mHasField) {
        return SUCCEEDED;
    }
    initHeap();
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM |
                                   dScene::SCENE_ATTR_CHKP) &&
        !mPackBank.load(dHeap::localHeap_p)) {
        return NOT_READY;
    }
    if (!mRoomAlwaysBank.load(dHeap::localHeap_p)) {
        return NOT_READY;
    }
    if (mBlocks.load(dHeap::localHeap_p)) {
        mIsPlayerHouse =
            dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_PLAYER_HOUSE);
        bool ok = loadModel(dHeap::localHeap_p);
        if (ok) {
            ok &= loadRoomItems();
        }
        if (!ok) {
            return NOT_READY;
        }
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM |
                                       dScene::SCENE_ATTR_CHKP)) {
            getResFile().Release();
            getResFile().Bind(nw4r::g3d::ResFile(mPackBank.getData()));
            getResFile().Bind();
        }
        u8 paramB = dScene::getSceneParamB(dScene::getCurrentScene());
        bool hasClockAnm = paramB != 0;
        mBlocks.create(&mAllocator);
        dBgUtil::setBgiDraw(this);
        nw4r::g3d::ResFile res = getResFile();
        nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
        ulong bufferOption = 0;
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_PLAYER_HOUSE) ||
            dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_STORE_ROOM) ||
            dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_VILLAGER_HOUSE)) {
            bufferOption = 0x108;
        }
        mMdl.create(mdl, &mAllocator, bufferOption, 1, NULL);

        int idx = -1;
        if (res.GetResMdlNumEntries() == 2 && !hasClockAnm) {
            idx = 1;
        } else if (res.GetResMdlNumEntries() == 3) {
            idx = 1;
        }
        if (idx != -1) {
            nw4r::g3d::ResMdl sunMdl = res.GetResMdl((ulong)idx);
            mSunMdl.create(sunMdl, &mAllocator, 0, 1, NULL);
            mSunMdl.setCallback(&mSunNodeCallBack);
            nw4r::g3d::ResFile sunAnm(
                mRoomAlwaysBank.getFile("roomAlways/sunAnm.brres", NULL));
            nw4r::g3d::ResAnmChr anmChr = sunAnm.GetResAnmChr(0);
            nw4r::g3d::ResAnmVis anmVis = sunAnm.GetResAnmVis(0);
            mSunAnmChr.create(sunMdl, anmChr, &mAllocator, NULL);
            mSunAnmChr.mPlayMode = 0;
            mSunMdl.setAnm(mSunAnmChr);
            mSunAnmVis.create(sunMdl, anmVis, &mAllocator, NULL);
            mSunAnmVis.mPlayMode = 0;
            mSunMdl.setAnm(mSunAnmVis);
            if (dScene::getCurrentScene() == dScene::SCENE_RM_MM_INS) {
                mSunAnmSrt.create(sunMdl, res.GetResAnmTexSrt(0), &mAllocator,
                                  NULL, 1);
                mSunAnmSrt.setPlayMode((m3d::playMode_e)0, 0);
                mSunMdl.setAnm(mSunAnmSrt);
            }
        } else {
            dScene::clearSceneCreated();
        }

        idx = -1;
        if (res.GetResMdlNumEntries() == 2 && hasClockAnm) {
            idx = 1;
        } else if (res.GetResMdlNumEntries() == 3) {
            idx = 2;
        }
        if (idx != -1) {
            nw4r::g3d::ResMdl clockMdl = res.GetResMdl((ulong)idx);
            mClockMdl.create(clockMdl, &mAllocator, 0, 1, NULL);
            if (paramB == 2) {
                mClockAnmChr.create(clockMdl, res.GetResAnmChr(0), &mAllocator,
                                    NULL);
                mClockAnmChr.mPlayMode = 0;
                mClockMdl.setAnm(mClockAnmChr);
            }
            mClockNodeCallBack.mHourNode = m3d::getNodeID(clockMdl, "kh_j");
            mClockNodeCallBack.mMinuteNode = m3d::getNodeID(clockMdl, "km_j");
            mClockMdl.setCallback(&mClockNodeCallBack);
            mClockMdl.calc(false);
            if (mClockNodeCallBack.mHourNode != -1) {
                mMtx_c mtx;
                mClockMdl.getNodeWorldMtx(mClockNodeCallBack.mHourNode, &mtx);
                PSMTXMultVec(mtx, mVec3_c::Zero, mSndPos);
            }
        }

        mNodeCallBack.mHourNode = m3d::getNodeID(mdl, "kh_j");
        mNodeCallBack.mMinuteNode = m3d::getNodeID(mdl, "km_j");
        mMdl.setCallback(&mNodeCallBack);
        if (res.HasResAnmChr() && paramB != 2) {
            mAnmChr.create(mdl, res.GetResAnmChr(0), &mAllocator, NULL);
            mAnmChr.mPlayMode = 0;
            mMdl.setAnm(mAnmChr);
        }
        if (res.HasResAnmClr()) {
            mAnmClr.create(mdl, res.GetResAnmClr(0), &mAllocator, NULL, 1);
            mAnmClr.setPlayMode((m3d::playMode_e)0, 0);
            mMdl.setAnm(mAnmClr);
        }
        if (res.HasResAnmTexSrt() &&
            dScene::getCurrentScene() != dScene::SCENE_RM_MM_INS) {
            mAnmSrt.create(mdl, res.GetResAnmTexSrt(0), &mAllocator, NULL, 1);
            mAnmSrt.setPlayMode((m3d::playMode_e)0, 0);
            mMdl.setAnm(mAnmSrt);
        }
        if (res.HasResAnmVis()) {
            mAnmVis.create(mdl, res.GetResAnmVis(0), &mAllocator, NULL);
            mAnmVis.mPlayMode = 0;
            mMdl.setAnm(mAnmVis);
        }
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_PLAYER_HOUSE) ||
            dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_VILLAGER_HOUSE)) {
            nw4r::g3d::ResFile grade(
                mRoomAlwaysBank.getFile("roomAlways/gradeTex.brres", NULL));
            res.Bind(grade);
            grade.Bind(res);
            mAnmPat.create(mdl, grade.GetResAnmTexPat(0), &mAllocator, NULL, 1);
            mAnmPat.setPlayMode((m3d::playMode_e)1, 0);
            mMdl.setAnm(mAnmPat);
            int room;
            int home =
                dHomeList_c::getHomeFromScene(&room, dScene::getCurrentScene());
            f32 frame = 1.0f;
            if (dScene::getCurrentScene() == dScene::SCENE_RM_HAPPY_MDL) {
                frame = dSaveData_c::getTown()->mModelRoom.mRoomType;
            }
            if (home != -1) {
                dHome_c *house = dSaveData_c::getTown()->mHomes.getHome(home);
                if (house != NULL) {
                    int size = house->mSize;
                    frame = size;
                    mAnmPat.setFrame(frame, 0);
                }
            }
            mAnmPat.setFrame(frame, 0);
            mAnmPatFixed = 1;
        } else if (res.HasResAnmTexPat()) {
            mAnmPat.create(mdl, res.GetResAnmTexPat(0), &mAllocator, NULL, 1);
            mAnmPat.setPlayMode((m3d::playMode_e)0, 0);
            mMdl.setAnm(mAnmPat);
        }
        if (dScene::getCurrentScene() == dScene::SCENE_RM_GRACE) {
            mMdl.setPriorityDraw(2, 0x81);
        } else {
            mMdl.setPriorityDraw(2, 9);
        }
        mMdl.calc(false);
        mIsTownBuildingRoom = isTownBuildingRoom();
        dBgUtil::createSigns(res, 0, 0);
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text:0x2C64
int dBgiDraw_c::execute() {
    if (!mHasField) {
        return SUCCEEDED;
    }
    mColor color = getSunColor();
    setSunColor(color);
    calcSun(color);
    calcClock();
    calcRequests();
    if (mMdl.getScn() != NULL) {
        mMdl.calc(false);
    }
    if (mClockAnmChr.IsBound()) {
        mClockAnmChr.play();
    }
    if (mAnmChr.IsBound()) {
        mAnmChr.play();
    }
    if (mAnmVis.IsBound()) {
        mAnmVis.play();
    }
    if (mAnmClr.IsBound()) {
        mAnmClr.play();
    }
    if (mAnmSrt.IsBound()) {
        mAnmSrt.play();
    }
    if (mAnmPat.IsBound() && !mAnmPatFixed) {
        mAnmPat.play();
    }
    mBlocks.execute(&mAllocator);
    return SUCCEEDED;
}

// .text:0x2DE0
int dBgiDraw_c::draw() {
    if (!mHasField) {
        return SUCCEEDED;
    }
    if (mMdl.getScn() != NULL) {
        nw4r::g3d::ResMdl mdl = getResMdl();
        static const char *riverMat = "m_river2_pattern";
        int id = m3d::getMatID(mdl, riverMat);
        if (id != -1) {
            nw4r::g3d::ScnMdl::CopiedMatAccess cma(
                nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(
                    nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(
                        mMdl.getScn())),
                id);
            if (cma.IsValid()) {
                nw4r::g3d::ResMatTevColor tev = cma.GetResMatTevColor(false);
                GXColor color;
                if (tev.GXGetTevKColor(GX_KCOLOR1, &color)) {
                    color.a = dBgiDraw::l_HIO.mRiverAlpha;
                    tev.GXSetTevKColor(GX_KCOLOR1, color);
                    tev.DCStore(false);
                }
            } else {
                nw4r::g3d::ResMatTevColor tev =
                    mdl.GetResMat(riverMat).GetResMatTevColor();
                GXColor color;
                if (tev.GXGetTevKColor(GX_KCOLOR1, &color)) {
                    color.a = dBgiDraw::l_HIO.mRiverAlpha;
                    tev.GXSetTevKColor(GX_KCOLOR1, color);
                    tev.DCStore(false);
                }
            }
        }
        dBgiDraw::setCulling(getResFile().GetResMdl(0),
                             dCamera_c::sInstance->mCurrent.mYaw);
        mMdl.entry();
    }
    mBlocks.draw(&mAllocator);
    if (mSunMdl.getScn() != NULL) {
        mSunMdl.entry();
    }
    if (mClockMdl.getScn() != NULL) {
        mClockMdl.calc(false);
        mClockMdl.entry();
    }
    if (dBgiDraw::l_HIO.mDebugDraw) {
        debugDraw();
    }
    return SUCCEEDED;
}

// .text:0x30C0
int dBgiDraw_c::doDelete() {
    if (!mHasField) {
        return SUCCEEDED;
    }
    dBgUtil::setBgiDraw(NULL);
    if (!mPackBank.unload() || !mRoomAlwaysBank.closeArc() ||
        !dDvd::bank_c::unload(FALSE) || !releaseRequests() ||
        !mBlocks.unload()) {
        return NOT_READY;
    }
    if (mMdl.getScn() != NULL) {
        mMdl.remove();
    }
    if (mAnmChr.IsBound()) {
        mAnmChr.remove();
    }
    if (mAnmVis.IsBound()) {
        mAnmVis.remove();
    }
    if (mAnmClr.IsBound()) {
        mAnmClr.remove();
    }
    if (mAnmSrt.IsBound()) {
        mAnmSrt.remove();
    }
    if (mAnmPat.IsBound()) {
        mAnmPat.remove();
    }
    if (mSunMdl.getScn() != NULL) {
        mSunMdl.remove();
    }
    if (mSunAnmChr.IsBound()) {
        mSunAnmChr.remove();
    }
    if (mSunAnmVis.IsBound()) {
        mSunAnmVis.remove();
    }
    if (mSunAnmSrt.IsBound()) {
        mSunAnmSrt.remove();
    }
    if (mClockMdl.getScn() != NULL) {
        mClockMdl.remove();
    }
    if (mClockAnmChr.IsBound()) {
        mClockAnmChr.remove();
    }
    mBlocks.remove(&mAllocator);
    deleteHeap();
    dHR::updateRate(4);
    return SUCCEEDED;
}

// .text:0x3324
mColor dBgiDraw_c::getSunColor() {
    mColor color;
    dEnvironment_c::calcColor(&color, 8);
    return color;
}

namespace dBgiDraw {

// The sunlight materials' KColor1.
static const char *l_sunMat = "m_sunshine";

} // namespace dBgiDraw

// .text:0x3334
void dBgiDraw_c::setSunColor(const mColor &color) {
    if (mMdl.getScn() == NULL) {
        return;
    }
    mColor col = color;
    nw4r::g3d::ResFile file = getResFile();
    nw4r::g3d::ResMdl mdl = file.GetResMdl(0);
    nw4r::g3d::ScnMdl *scnMdl =
        nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(
            nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(
                mMdl.getScn()));
    static const char *mats[] = {dBgiDraw::l_sunMat, "m_etc"};
    for (u32 i = 0; i < 2; i++) {
        const char *name = mats[i];
        int id = m3d::getMatID(mdl, name);
        if (id == -1) {
            continue;
        }
        nw4r::g3d::ScnMdl::CopiedMatAccess cma(scnMdl, id);
        if (cma.IsValid()) {
            nw4r::g3d::ResMatTevColor tev = cma.GetResMatTevColor(false);
            tev.GXSetTevKColor(GX_KCOLOR1, col);
            tev.DCStore(false);
        } else {
            nw4r::g3d::ResMatTevColor tev =
                mdl.GetResMat(name).GetResMatTevColor();
            tev.GXSetTevKColor(GX_KCOLOR1, col);
            tev.DCStore(false);
        }
    }
}

// .text:0x3584
void dBgiDraw_c::calcSun(const mColor &color) {
    if (mSunMdl.getScn() == NULL) {
        return;
    }
    dTime_c time = *dTime_c::getCurrent();
    u8 hour = time.hour;
    u8 min = time.min;
    u8 sec = time.sec;
    mColor col = color;
    f32 frame = 239.0f * ((f32)(u32)(sec + (min + hour * 60) * 60) / 86400.0f);
    mSunAnmChr.setFrame(frame);
    mSunAnmVis.setFrame(frame);
    if (mSunAnmSrt.IsBound()) {
        mSunAnmSrt.setFrame(frame, 0);
    }
    nw4r::g3d::ResFile file = getResFile();
    nw4r::g3d::ResMdl mdl = file.GetResMdl(1);
    nw4r::g3d::ScnMdl *scnMdl =
        nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(
            nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(
                mSunMdl.getScn()));
    static const char *mats[] = {dBgiDraw::l_sunMat, "m_sunshine_01"};
    for (u32 i = 0; i < 2; i++) {
        const char *name = mats[i];
        int id = m3d::getMatID(mdl, name);
        if (id == -1) {
            continue;
        }
        nw4r::g3d::ScnMdl::CopiedMatAccess cma(scnMdl, id);
        if (cma.IsValid()) {
            nw4r::g3d::ResMatTevColor tev = cma.GetResMatTevColor(false);
            tev.GXSetTevKColor(GX_KCOLOR1, col);
            tev.DCStore(false);
        } else {
            nw4r::g3d::ResMatTevColor tev =
                mdl.GetResMat(name).GetResMatTevColor();
            tev.GXSetTevKColor(GX_KCOLOR1, col);
            tev.DCStore(false);
        }
    }
}

// .text:0x38C8
void dBgiDraw_c::calcClock() {
    if (mClockNodeCallBack.mHourNode == -1) {
        return;
    }
    if (dBgiDraw::l_HIO.mDrawSndPos) {
        mColor color(200, 200, 100, 255);
        dSceneCheck::drawSphere(mSndPos, color, 1, 0, 0, 16.0f);
    }
    mSndObj.setPos(&mSndPos);
    dTime_c time = *dTime_c::getCurrent();
    if (time.sec != mLastSecond) {
        if (mSndStarted) {
            mSndObj.startSound(SE_tokei);
        } else {
            mSndStarted = 1;
        }
        mLastSecond = time.sec;
    }
}

#ifdef MUST_MATCH
// Unreferenced constants left by a stripped user: the sun animations' last
// frame, two counts of 2 (the loaders of each request) and the wallpaper /
// carpet loader heap sizes.
struct bgiDrawConsts_c {
    f32 mSunFrameMax;
    int mWallLoaderNum;
    int mCarpetLoaderNum;
    u32 mWallHeapSize;
    u32 mCarpetHeapSize;
};
#pragma force_active on
extern const bgiDrawConsts_c lbl_107_rodata_AC;
const bgiDrawConsts_c lbl_107_rodata_AC = {239.0f, 2, 2, 0x2780, 0x4780};
#pragma force_active reset
#endif

// .text:0x39EC
void dBgiDraw_c::nodeCallBack_c::timingA(ulong nodeId,
                                         nw4r::g3d::ChrAnmResult *anmRes,
                                         nw4r::g3d::ResMdl resMdl) {
    dTime_c time = *dTime_c::getCurrent();
    int hour = time.hour;
    int min = time.min;
    if (nodeId == mHourNode) {
        s32 minutes = hour % 12;
        minutes *= 60;
        minutes += min;
        mMtx_c mtx;
        anmRes->GetRotTrans(mtx);
        mtx.ZrotM(-(s16)((minutes << 16) / 720));
        anmRes->SetRotTrans(mtx);
    } else if (nodeId == mMinuteNode) {
        mMtx_c mtx;
        anmRes->GetRotTrans(mtx);
        mtx.ZrotM(-(s16)((min << 16) / 60));
        anmRes->SetRotTrans(mtx);
    }
}

// .text:0x3B6C
BOOL dBgiDraw_c::block_c::load(void *heap, int blockX, int blockZ) {
    dFdBase_c *info = dField::getInfo(0);
    if (heap != NULL && info != NULL) {
        char path[0x28];
        mBgId = info->getBlock(blockX, blockZ)->getType();
        mBlockX = blockX;
        mBlockZ = blockZ;
        snprintf(path, sizeof(path), "/BgData/BgModel/%03d.brres", mBgId);
        return mBank.load(path, heap, 0);
    }
    return FALSE;
}

// .text:0x3C20
BOOL dBgiDraw_c::block_c::create(mAllocator_c *allocator) {
    nw4r::g3d::ResFile res(mBank.getData());
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.create(mdl, allocator, 0, 1, NULL);
    if (res.HasResAnmChr()) {
        mAnmChr.create(mdl, res.GetResAnmChr(0), allocator, NULL);
        mAnmChr.mPlayMode = 0;
        mMdl.setAnm(mAnmChr);
    }
    if (res.HasResAnmVis()) {
        mAnmVis.create(mdl, res.GetResAnmVis(0), allocator, NULL);
        mAnmVis.mPlayMode = 0;
        mMdl.setAnm(mAnmVis);
    }
    if (res.HasResAnmClr()) {
        mAnmClr.create(mdl, res.GetResAnmClr(0), allocator, NULL, 1);
        mAnmClr.setPlayMode((m3d::playMode_e)0, 0);
        mMdl.setAnm(mAnmClr);
    }
    if (res.HasResAnmTexPat()) {
        mAnmPat.create(mdl, res.GetResAnmTexPat(0), allocator, NULL, 1);
        mAnmPat.setPlayMode((m3d::playMode_e)0, 0);
        mMdl.setAnm(mAnmPat);
    }
    if (res.HasResAnmTexSrt()) {
        mAnmSrt.create(mdl, res.GetResAnmTexSrt(0), allocator, NULL, 1);
        mAnmSrt.setPlayMode((m3d::playMode_e)0, 0);
        mMdl.setAnm(mAnmSrt);
    }
    mMtx_c mtx;
    PSMTXTrans(mtx, 0.0f, 0.0f, 0.0f);
    mMdl.setLocalMtx(&mtx);
    if (dScene::getCurrentScene() == dScene::SCENE_RM_GRACE) {
        mMdl.setPriorityDraw(2, 0x81);
    } else {
        mMdl.setPriorityDraw(2, 9);
    }
    mMdl.calc(false);
    dBgUtil::createSigns(res, mBlockX, mBlockZ);
    return TRUE;
}

// .text:0x3F04
BOOL dBgiDraw_c::block_c::execute(mAllocator_c *allocator) {
    if (mMdl.getScn() != NULL) {
        mMdl.calc(false);
    }
    if (mAnmChr.IsBound()) {
        mAnmChr.play();
    }
    if (mAnmVis.IsBound()) {
        mAnmVis.play();
    }
    if (mAnmClr.IsBound()) {
        mAnmClr.play();
    }
    if (mAnmSrt.IsBound()) {
        mAnmSrt.play();
    }
    if (mAnmPat.IsBound()) {
        mAnmPat.play();
    }
    return TRUE;
}

// .text:0x3FFC
BOOL dBgiDraw_c::block_c::draw(mAllocator_c *allocator) {
    if (mMdl.getScn() != NULL) {
        dBgiDraw::setCulling(nw4r::g3d::ResFile(mBank.getData()).GetResMdl(0),
                             dCamera_c::sInstance->mCurrent.mYaw);
        nw4r::g3d::ResMdl mdl =
            nw4r::g3d::ResFile(mBank.getData()).GetResMdl(0);
        static const char *riverMat = "m_river2_pattern";
        int id = m3d::getMatID(mdl, riverMat);
        if (id != -1) {
            nw4r::g3d::ScnMdl::CopiedMatAccess cma(
                nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(
                    nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(
                        mMdl.getScn())),
                id);
            if (cma.IsValid()) {
                nw4r::g3d::ResMatTevColor tev = cma.GetResMatTevColor(false);
                GXColor color;
                if (tev.GXGetTevKColor(GX_KCOLOR1, &color)) {
                    color.a = dBgiDraw::l_HIO.mRiverAlpha;
                    tev.GXSetTevKColor(GX_KCOLOR1, color);
                    tev.DCStore(false);
                }
            } else {
                nw4r::g3d::ResMatTevColor tev =
                    mdl.GetResMat(riverMat).GetResMatTevColor();
                GXColor color;
                if (tev.GXGetTevKColor(GX_KCOLOR1, &color)) {
                    color.a = dBgiDraw::l_HIO.mRiverAlpha;
                    tev.GXSetTevKColor(GX_KCOLOR1, color);
                    tev.DCStore(false);
                }
            }
        }

        mMdl.entry();
    }
    return TRUE;
}

// .text:0x4260
void dBgiDraw_c::block_c::remove(mAllocator_c *allocator) {
    if (mMdl.getScn() != NULL) {
        mMdl.remove();
    }
    if (mAnmChr.IsBound()) {
        mAnmChr.remove();
    }
    if (mAnmVis.IsBound()) {
        mAnmVis.remove();
    }
    if (mAnmClr.IsBound()) {
        mAnmClr.remove();
    }
    if (mAnmSrt.IsBound()) {
        mAnmSrt.remove();
    }
    if (mAnmPat.IsBound()) {
        mAnmPat.remove();
    }
}

// .text:0x4358
BOOL dBgiDraw_c::block_c::getNodePos(mVec3_c *pos, const char *name) const {
    if (mMdl.getScn() != NULL) {
        int id = m3d::getNodeID(mMdl.getResMdl(), name);
        if (id != -1) {
            mMtx_c mtx;
            mMdl.getNodeWorldMtx(id, &mtx);
            PSMTXMultVec(mtx, mVec3_c::Zero, *pos);
            return TRUE;
        }
    }
    return FALSE;
}

// .text:0x43F4
int dBgiDraw_c::block_c::getNodeNum() {
    nw4r::g3d::ResMdl mdl = mMdl.getResMdl();
    if (mdl.IsValid()) {
        return mdl.GetResNodeNumEntries();
    }
    return 0;
}

// .text:0x4434
const char *dBgiDraw_c::block_c::getNodeName(u32 idx) {
    if (idx < getNodeNum()) {
        nw4r::g3d::ResMdl mdl = mMdl.getResMdl();
        if (mdl.IsValid()) {
            return mdl.GetResNode(idx).GetName();
        }
        return NULL;
    }
    return NULL;
}

// .text:0x44BC
BOOL dBgiDraw_c::blockMng_c::isValid(int blockX, int blockZ) {
    if (blockX >= 0 && blockX < mNumX && blockZ >= 0 && blockZ < mNumZ) {
        return blockX != 0 || blockZ != 0;
    }
    return FALSE;
}

// .text:0x4500
bool dBgiDraw_c::blockMng_c::isReady() {
    bool ok = true;
    for (int z = 0; z < mNumZ; z++) {
        for (int x = 0; x < mNumX; x++) {
            if (isValid(x, z)) {
                ok &= mBlocks[z][x].mMdl.getScn() != NULL;
            }
        }
    }
    return ok;
}

// .text:0x45B0
BOOL dBgiDraw_c::blockMng_c::load(void *heap) {
    dFdBase_c *info = dField::getInfo(0);
    if (heap != NULL && info != NULL) {
        mNumX = info->getBlockW();
        mNumZ = info->getBlockH();
        bool ok = true;
        for (int z = 0; z < mNumZ; z++) {
            for (int x = 0; x < mNumX; x++) {
                if (isValid(x, z)) {
                    ok &= mBlocks[z][x].load(heap, x, z);
                }
            }
        }
        return ok;
    }
    return FALSE;
}

// .text:0x4698
bool dBgiDraw_c::blockMng_c::unload() {
    bool ok = true;
    for (u32 z = 0; z < mNumZ; z++) {
        for (u32 x = 0; x < mNumX; x++) {
            if (isValid(x, z)) {
                ok &= mBlocks[z][x].mBank.unload(FALSE);
            }
        }
    }
    return ok;
}

// .text:0x4744
bool dBgiDraw_c::blockMng_c::create(mAllocator_c *allocator) {
    bool ok = true;
    for (int z = 0; z < mNumZ; z++) {
        for (int x = 0; x < mNumX; x++) {
            if (isValid(x, z)) {
                ok &= mBlocks[z][x].create(allocator);
            }
        }
    }
    return ok;
}

// .text:0x47F4
bool dBgiDraw_c::blockMng_c::execute(mAllocator_c *allocator) {
    bool ok = true;
    for (int z = 0; z < mNumZ; z++) {
        for (int x = 0; x < mNumX; x++) {
            if (isValid(x, z)) {
                ok &= mBlocks[z][x].execute(allocator);
            }
        }
    }
    return ok;
}

// .text:0x48A4
bool dBgiDraw_c::blockMng_c::draw(mAllocator_c *allocator) {
    bool ok = true;
    for (int z = 0; z < mNumZ; z++) {
        for (int x = 0; x < mNumX; x++) {
            if (isValid(x, z)) {
                ok &= mBlocks[z][x].draw(allocator);
            }
        }
    }
    return ok;
}

// .text:0x4954
void dBgiDraw_c::blockMng_c::remove(mAllocator_c *allocator) {
    for (int z = 0; z < mNumZ; z++) {
        for (int x = 0; x < mNumX; x++) {
            if (isValid(x, z)) {
                mBlocks[z][x].remove(allocator);
            }
        }
    }
}

// .text:0x49EC
BOOL dBgiDraw_c::blockMng_c::getNodePos(mVec3_c *pos, int blockX, int blockZ,
                                        const char *name) {
    if (isValid(blockX, blockZ)) {
        return mBlocks[blockZ][blockX].getNodePos(pos, name);
    }
    return FALSE;
}

// .text:0x4A64
int dBgiDraw_c::blockMng_c::getNodeNum(int blockX, int blockZ) {
    if (isValid(blockX, blockZ)) {
        return mBlocks[blockZ][blockX].getNodeNum();
    }
    return 0;
}

// .text:0x4ACC
const char *dBgiDraw_c::blockMng_c::getNodeName(u32 idx, int blockX,
                                                int blockZ) {
    if (isValid(blockX, blockZ)) {
        return mBlocks[blockZ][blockX].getNodeName(idx);
    }
    return NULL;
}

// The room is a town building whose block is in row 5.
// .text:0x4B4C
BOOL dBgiDraw_c::isTownBuildingRoom() {
    u16 id = dScene::getSceneBuilding(dScene::getCurrentScene());
    dItem::Item tmp(id);
    if (tmp.isExtId()) {
        if (dSTR::getBuildingInfo(tmp.getExtIndex())->mField &
            BUILDING_FIELD_TOWN) {
            dItem::Item building(id);
            int unitX, unitZ;
            if (dSaveBuildingList_c::get()->getPos(&unitX, &unitZ, building,
                                                   1)) {
                int blockX, blockZ;
                dBGCF::unitToBlock(&blockX, &blockZ, unitX, unitZ);
                if (blockZ == 5) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

// .text:0x4C2C
dBgiDraw_hostIO_c::~dBgiDraw_hostIO_c() {
}

// .text:0x4C6C
dBgiDraw_c::~dBgiDraw_c() {
}

namespace dBgiDraw {

// Stripped: an unused function that built an m3d::anmChrPart_c. Its vtable,
// generated in the end-of-unit vtable pass right after the callback classes',
// is what first references m3d::anmChr_c::getType, so that weak copy precedes
// anmVis_c::getType (.text 0x511C / 0x5124). MWCC drops the function itself;
// nothing of it reaches the object.
static void unusedPartAnm() {
    m3d::anmChrPart_c anm;
}

} // namespace dBgiDraw
