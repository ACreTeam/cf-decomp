#include <game/game/d_bgo_draw.hpp>
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_bg.hpp>
#include <game/game/d_bg_attr.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_environment.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_snd_obj.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_world.hpp>
#include <game/mLib/m_3d/global.hpp>
#include <game/mLib/m_color.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/snd/snd_id.hpp>
#include <cmath>
#include <game/game/d_a_player.hpp>

// Debug parameters (RTTI "dBgoDraw_HIO_c").
class dBgoDraw_HIO_c {
  public:
    dBgoDraw_HIO_c() {
        mRiverScaleX = 1.0f;
        mRiverScaleY = 0.68f;
        mRiverTransY = 498.0f;
        mFixRiverTrans = 0;
        mOceanScaleX = 1.0f;
        mOceanScaleY = 1.0f;
        mOceanTransY = 0.0f;
        mOceanAlpha1 = 0;
        mOceanAlpha2 = 0x10;
        _2C = 0;
        _2E = 0;
        _30 = 0;
        mSeaR = 0x18;
        mSeaG = 0x78;
        mSeaB = 0x9C;
        mSeaA = 0x32;
        mUpdateZ = 1;
        mSeaRate = 0x46;
        mGrassTransX = 0.21875f;
        mGrassTransY = 0.8125f;
        mStoneTransX = 0.109375f;
        mStoneTransY = 0.40875f;
        _42 = 1;
        _43 = 1;
        _44 = 0;
        mRiverFalloff = -0.01f;
    }
    virtual ~dBgoDraw_HIO_c();

    /* 0x04 */ f32 mGrassTransX;
    /* 0x08 */ f32 mGrassTransY;
    /* 0x0C */ f32 mStoneTransX;
    /* 0x10 */ f32 mStoneTransY;
    /* 0x14 */ f32 mRiverTransY;
    /* 0x18 */ f32 mRiverScaleX;
    /* 0x1C */ f32 mRiverScaleY;
    /* 0x20 */ f32 mOceanTransY;
    /* 0x24 */ f32 mOceanScaleX;
    /* 0x28 */ f32 mOceanScaleY;
    /* 0x2C */ s16 _2C;
    /* 0x2E */ s16 _2E;
    /* 0x30 */ s16 _30;
    /* 0x32 */ s16 mSeaR; // the sea colour mixed into the time-of-day colour
    /* 0x34 */ s16 mSeaG;
    /* 0x36 */ s16 mSeaB;
    /* 0x38 */ s16 mSeaA;
    /* 0x3A */ s16 mOceanAlpha1; // m_ocean KColor1 alpha
    /* 0x3C */ s16 mOceanAlpha2; // m_ocean KColor2 alpha
    /* 0x3E */ s16 mSeaRate;     // percentage of the sea colour
    /* 0x40 */ u8
        mFixRiverTrans;     // use mRiverTransY instead of the camera-based one
    /* 0x41 */ u8 mUpdateZ; // river z update; draw priority by the camera area
    /* 0x42 */ u8 _42;
    /* 0x43 */ u8 _43;
    /* 0x44 */ u8 _44;
    /* 0x48 */ f32
        mRiverFalloff; // river texture scale change per unit of camera distance
}; // size 0x4C

namespace dBgoDraw {

// .text:0xCC
void *create() {
    return new dBgoDraw_c;
}

fProf::fBaseProfile_c g_profile_BGO_DRAW = {&create, fProf::BGO_DRAW,
                                            fProf::DRAW_ORDER::BGO_DRAW};

// The river texture scale (by the camera distance).
static f32 l_riverScale = 1.0f;
static f32 l_unused = 0.0f;

// The grass / soil and stone materials, scrolled by the debug translation.
static const char *l_grassMats[] = {"m_grass", "m_soil"};
static const char *l_stoneMats[] = {"m_soil_stone", "m_stone"};

static dBgoDraw_HIO_c l_HIO;

// Counts the frames of the town's bg animations.
class frameCounter_c {
  public:
    frameCounter_c() : mFrame(0) {
    }

    u32 getMod(u32 num);

    /* 0x0 */ u32 mFrame;
};

// .text:0x140
u32 frameCounter_c::getMod(u32 num) {
    return mFrame % (num != 0 ? num : 1);
}

static frameCounter_c l_frame;
// A second counter nothing here reads (its user was stripped).
static frameCounter_c l_unusedFrame;

// The river texture's y translation (by the camera).
static f32 l_riverTransY;

// The sea colour (the time-of-day colour mixed with the debug sea colour).
static mColor l_seaColor(0xFF, 0xFF, 0xFF, 0xFF);

} // namespace dBgoDraw

// .text:0x164
void dBgoDraw_c::block_c::setMaterials(u32 blockX, u32 blockZ, u32 flags) {
    static const int l_riverSrtIdx = 3;
    static const int l_oceanSrtIdx = 3;
    static const int l_grassMatNum = 2;
    static const int l_grassSrtIdx = 1;
    static const int l_stoneMatNum = 2;
    static const int l_stoneSrtIdx = 1;
    if (flags == 0) {
        return;
    }
    dBG::bkLoader_c *loader = dBG::getMdlBank()->getLoader(blockX, blockZ);
    if (loader == NULL) {
        return;
    }
    nw4r::g3d::ResFile res(loader->getData());
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    nw4r::g3d::ResMat river = mdl.GetResMat("m_river2_pattern");
    if (river.IsValid()) {
        river.ref().flag &= ~0x80000000;
        river.DCStore(false);
        if (flags & 4) {
            nw4r::g3d::ResMatPix pix = river.GetResMatPix();
            GXBool test;
            GXCompare compare;
            GXBool update;
            if (pix.GXGetZMode(&test, &compare, &update)) {
                pix.GXSetZMode(test, compare, dBgoDraw::l_HIO.mUpdateZ != 0);
                pix.DCStore(false);
            }
        }
        if (flags & 1) {
            nw4r::g3d::ResTexSrt srt = river.GetResTexSrt();
            if (srt.IsExist(l_riverSrtIdx)) {
                mMtx_c mtx = mMtx_c::createScale(
                    dBgoDraw::l_HIO.mRiverScaleX * dBgoDraw::l_riverScale,
                    dBgoDraw::l_HIO.mRiverScaleY * dBgoDraw::l_riverScale,
                    1.0f);
                if (dBgoDraw::l_HIO.mFixRiverTrans) {
                    mtx.concat(mMtx_c::createTrans(
                        0.0f, dBgoDraw::l_HIO.mRiverTransY, 0.0f));
                } else {
                    if (dScene::getCurrentScene() == dScene::SCENE_DM_TITLE) {
                        dBgoDraw::l_riverTransY = 470.0f;
                    }
                    mtx.concat(mMtx_c::createTrans(
                        0.0f, dBgoDraw::l_riverTransY, 0.0f));
                }
                if (srt.SetEffectMtx(l_riverSrtIdx, mtx)) {
                    srt.ref().flag &=
                        ~(nw4r::g3d::TexSrt::FLAGSET_IDENTITY
                          << l_riverSrtIdx * nw4r::g3d::TexSrt::NUM_OF_FLAGS);
                    srt.ref().flag |=
                        nw4r::g3d::TexSrt::FLAG_ANM_EXISTS
                        << l_riverSrtIdx * nw4r::g3d::TexSrt::NUM_OF_FLAGS;
                }
            }
        }
        if (flags & 2) {
            nw4r::g3d::ResMatTevColor tev = river.GetResMatTevColor();
            tev.GXSetTevKColor(GX_KCOLOR1, dBgoDraw::l_seaColor);
            tev.DCStore(false);
        }
    }
    nw4r::g3d::ResMat ocean = mdl.GetResMat("m_ocean");
    if (ocean.IsValid()) {
        ocean.ref().flag &= ~0x80000000;
        ocean.DCStore(false);
        if (flags & 1) {
            nw4r::g3d::ResTexSrt srt = ocean.GetResTexSrt();
            if (srt.IsExist(l_oceanSrtIdx)) {
                mMtx_c mtx =
                    mMtx_c::createScale(dBgoDraw::l_HIO.mOceanScaleX,
                                        dBgoDraw::l_HIO.mOceanScaleY, 1.0f);
                mtx.concat(mMtx_c::createTrans(
                    0.0f, dBgoDraw::l_HIO.mOceanTransY, 0.0f));
                if (srt.SetEffectMtx(l_oceanSrtIdx, mtx)) {
                    srt.ref().flag &=
                        ~(nw4r::g3d::TexSrt::FLAGSET_IDENTITY
                          << l_oceanSrtIdx * nw4r::g3d::TexSrt::NUM_OF_FLAGS);
                    srt.ref().flag |=
                        nw4r::g3d::TexSrt::FLAG_ANM_EXISTS
                        << l_oceanSrtIdx * nw4r::g3d::TexSrt::NUM_OF_FLAGS;
                }
            }
        }
        if (flags & 2) {
            nw4r::g3d::ResMatTevColor tev = ocean.GetResMatTevColor();
            mColor color1;
            mColor color2;
            bool changed = false;
            if (tev.GXGetTevKColor(GX_KCOLOR1, &color1)) {
                color1.a = dBgoDraw::l_HIO.mOceanAlpha1;
                tev.GXSetTevKColor(GX_KCOLOR1, color1);
                changed = true;
            }
            if (tev.GXGetTevKColor(GX_KCOLOR2, &color2)) {
                color2.a = dBgoDraw::l_HIO.mOceanAlpha2;
                tev.GXSetTevKColor(GX_KCOLOR2, color2);
                changed = true;
            }
            if (changed) {
                tev.DCStore(false);
            }
        }
    }
    mMtx_c mtx;
    mtx.trans(dBgoDraw::l_HIO.mGrassTransX, dBgoDraw::l_HIO.mGrassTransY, 0.0f);
    for (u32 i = 0; i < l_grassMatNum; i++) {
        nw4r::g3d::ResMat mat = mdl.GetResMat(dBgoDraw::l_grassMats[i]);
        if (mat.IsValid()) {
            nw4r::g3d::ResTexSrt srt = mat.GetResTexSrt();
            if (srt.IsExist(l_grassSrtIdx) &&
                srt.SetEffectMtx(l_grassSrtIdx, mtx)) {
                srt.ref().flag &=
                    ~(nw4r::g3d::TexSrt::FLAGSET_IDENTITY
                      << l_grassSrtIdx * nw4r::g3d::TexSrt::NUM_OF_FLAGS);
                srt.ref().flag |=
                    nw4r::g3d::TexSrt::FLAG_ANM_EXISTS
                    << l_grassSrtIdx * nw4r::g3d::TexSrt::NUM_OF_FLAGS;
            }
        }
    }
    mtx.trans(dBgoDraw::l_HIO.mStoneTransX, dBgoDraw::l_HIO.mStoneTransY, 0.0f);
    for (u32 i = 0; i < l_stoneMatNum; i++) {
        nw4r::g3d::ResMat mat = mdl.GetResMat(dBgoDraw::l_stoneMats[i]);
        if (mat.IsValid()) {
            nw4r::g3d::ResTexSrt srt = mat.GetResTexSrt();
            if (srt.IsExist(l_stoneSrtIdx) &&
                srt.SetEffectMtx(l_stoneSrtIdx, mtx)) {
                srt.ref().flag &=
                    ~(nw4r::g3d::TexSrt::FLAGSET_IDENTITY
                      << l_stoneSrtIdx * nw4r::g3d::TexSrt::NUM_OF_FLAGS);
                srt.ref().flag |=
                    nw4r::g3d::TexSrt::FLAG_ANM_EXISTS
                    << l_stoneSrtIdx * nw4r::g3d::TexSrt::NUM_OF_FLAGS;
            }
        }
    }
}

namespace dBgoDraw {

// A material and the pack's animation that goes on it.
struct anmName_s {
    const char *mMat;
    const char *mAnm;
};

} // namespace dBgoDraw

// .text:0x704
void dBgoDraw_c::block_c::create(u32 blockX, u32 blockZ,
                                 nw4r::g3d::ResFile pack,
                                 mAllocator_c *allocator, int init) {
    static const dBgoDraw::anmName_s srtAnms[] = {
        {"m_river2_pattern", "New_anim_river"},
        {"m_wave_white", "New_anim_river"},
    };
    static const dBgoDraw::anmName_s clrAnms[] = {
        {"m_wave_white", "New_anim_river"},
    };
    static const int l_srtAnmNum = ARRAY_SIZE(srtAnms);
    static const int l_clrAnmNum = ARRAY_SIZE(clrAnms);
    dBG::bkLoader_c *loader = dBG::getMdlBank()->getLoader(blockX, blockZ);
    dFdBase_c *info = dField::getInfo(0);
    if (loader != NULL && info != NULL) {
        nw4r::g3d::ResFile res(loader->getData());
        nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
        if (!mdl.IsValid()) {
            info->getBlock(blockX, blockZ);
        }
        f32 x = blockX * lbl_80750520;
        info->getBlock(blockX, blockZ);
        res.Bind(pack);
        mMdl.create(mdl, allocator, 0, 1, NULL);
        for (const dBgoDraw::anmName_s *anm = srtAnms;
             anm < srtAnms + l_srtAnmNum; anm++) {
            if (m3d::getMatID(mdl, anm->mMat) >= 0) {
                nw4r::g3d::ResAnmTexSrt srt = pack.GetResAnmTexSrt(anm->mAnm);
                if (srt.IsValid()) {
                    if (mAnmSrt.create(mdl, srt, allocator, NULL, 1)) {
                        mAnmFrameNum = srt.GetNumFrame();
                        mMdl.setAnm(mAnmSrt);
                    }
                }
                break;
            }
        }
        for (const dBgoDraw::anmName_s *anm = clrAnms;
             anm < clrAnms + l_clrAnmNum; anm++) {
            if (m3d::getMatID(mdl, anm->mMat) >= 0) {
                nw4r::g3d::ResAnmClr clr = pack.GetResAnmClr(anm->mAnm);
                if (clr.IsValid()) {
                    if (mAnmClr.create(mdl, clr, allocator, NULL, 1)) {
                        mMdl.setAnm(mAnmClr);
                    }
                }
                break;
            }
        }
        mMtx_c mtx;
        PSMTXTrans(mtx, x, 0.0f, 0.0f);
        mtx.XrotM(mAng(blockZ * dWorld::getAngle()));
        mMdl.setLocalMtx(&mtx);
        mState = STATE_EXECUTE;
        setMaterials(blockX, blockZ, 7);
        mIsEdge = blockX == 0 || blockZ == 0 || blockX == BLOCK_X_NUM - 1;
        if (mIsEdge) {
            if (blockZ == 0) {
                mPrioOpa = 3;
            } else {
                mPrioOpa = 4;
            }
            mPrioXlu = 9;
        } else {
            mPrioOpa = 2;
            mPrioXlu = 9;
        }
        mMdl.setPriorityDraw(mPrioOpa, mPrioXlu);
        for (u32 i = 0; i < SOUND_NUM; i++) {
            mVec3_c pos;
            switch (getSoundPoint(&pos.x, &pos.z, blockX, blockZ, i)) {
            case SOUND_POINT_FALL:
                pos.y = dBGCF::getGroundY(&pos, FALSE);
                mpSound[mSoundNum] = new (dHeap::localHeap_p, 4) dSndObjFall_c;
                if (mpSound[mSoundNum] != NULL) {
                    mSoundPos[mSoundNum] = pos;
                    mSoundId[mSoundNum] = SE_LEV_taki;
                    mSoundNum++;
                }
                break;
            case SOUND_POINT_POND:
                pos.y = dBGCF::getGroundY(&pos, FALSE);
                mpSound[mSoundNum] = new (dHeap::localHeap_p, 4) dSndObjPond_c;
                if (mpSound[mSoundNum] != NULL) {
                    mSoundPos[mSoundNum] = pos;
                    mSoundId[mSoundNum] = SE_LEV_ike;
                    mSoundNum++;
                }
                break;
            case SOUND_POINT_RIVER:
                pos.y = dBGCF::getGroundY(&pos, FALSE);
                mpSound[mSoundNum] = new (dHeap::localHeap_p, 4) dSndObjRiver_c;
                if (mpSound[mSoundNum] != NULL) {
                    mSoundPos[mSoundNum] = pos;
                    mSoundId[mSoundNum] = 0;
                    mSoundNum++;
                }
                break;
            }
        }
    }
}

// .text:0xC30
void dBgoDraw_c::block_c::execute(u32 blockX, u32 blockZ,
                                  nw4r::g3d::ResFile pack,
                                  mAllocator_c *allocator, int init) {
    setMaterials(blockX, blockZ, 7);
    f32 frame = dBgoDraw::l_frame.getMod(mAnmFrameNum);
    if (mAnmSrt.IsBound()) {
        mAnmSrt.setFrame(frame, 0);
    }
    if (mAnmClr.IsBound()) {
        mAnmClr.setFrame(frame, 0);
    }
    if (init == 0) {
        mVec3_c *pos = mSoundPos;
        int i = 0;
        for (u32 *id = mSoundId; id < &mSoundId[mSoundNum]; pos++, i++, id++) {
            switch (*id) {
            case SE_LEV_taki: {
                dSndObjFall_c *sound = (dSndObjFall_c *)mpSound[i];
                if (sound != NULL) {
                    sound->setPos(pos);
                    sound->holdSound(SE_LEV_taki);
                }
                break;
            }
            case SE_LEV_ike: {
                dSndObjPond_c *sound = (dSndObjPond_c *)mpSound[i];
                if (sound != NULL) {
                    sound->setPos(pos);
                    sound->holdSound(SE_LEV_ike);
                }
                break;
            }
            case 0: {
                dSndObjRiver_c *sound = (dSndObjRiver_c *)mpSound[i];
                if (sound != NULL) {
                    sound->setPos(pos);
                }
                break;
            }
            }
        }
    }
}

// .text:0xDB0
void dBgoDraw_c::block_c::proc(u32 blockX, u32 blockZ, nw4r::g3d::ResFile pack,
                               mAllocator_c *allocator, int init) {
    static void (block_c::*const procs[])(u32, u32, nw4r::g3d::ResFile,
                                          mAllocator_c *, int) = {
        &block_c::create,
        &block_c::execute,
    };
    if (mState < STATE_NUM) {
        (this->*procs[mState])(blockX, blockZ, pack, allocator, init);
    }
}

// .text:0xE00
void dBgoDraw_c::block_c::draw(u32 blockX, u32 blockZ,
                               nw4r::g3d::ResFile pack) {
    if (mState == STATE_CREATE) {
        return;
    }
    if (dBgoDraw::l_HIO.mUpdateZ) {
        dBG::area_c area;
        area.setView();
        mPrioXlu = area.getPriority(blockX, blockZ, &dCamera_c::sCenter);
    } else {
        mPrioXlu = 9;
    }
    mMdl.setPriorityDraw(mPrioOpa, mPrioXlu);
    mMdl.entry();
}

// .text:0xEB4
void dBgoDraw_c::block_c::remove(u32 blockX, u32 blockZ,
                                 nw4r::g3d::ResFile pack) {
    if (mState != STATE_CREATE) {
        mMdl.remove();
        mAnmSrt.remove();
        mAnmClr.remove();
    }
    for (u32 i = 0; i < SOUND_NUM; i++) {
        if (mpSound[i] != NULL) {
            switch (mSoundId[i]) {
            case SE_LEV_taki:
                delete (dSndObjFall_c *)mpSound[i];
                break;
            case SE_LEV_ike:
                delete (dSndObjPond_c *)mpSound[i];
                break;
            default:
                delete (dSndObjRiver_c *)mpSound[i];
                break;
            }
            mpSound[i] = NULL;
        }
    }
}

// .text:0xF94
int dBgoDraw_c::block_c::getSoundPoint(f32 *x, f32 *z, u32 blockX, u32 blockZ,
                                       u32 idx) {
    dBG::bkLoader_c *loader = dBG::getMdlBank()->getLoader(blockX, blockZ);
    if (loader != NULL) {
        dBG::bsd_c *bsd = loader->getBsd();
        if (bsd != NULL) {
            return bsd->getWorldPoint(x, z, blockX, blockZ, idx);
        }
    }
    return 0xFFFF;
}

// .text:0x101C
void dBgoDraw_c::blockMng_c::proc(nw4r::g3d::ResFile pack,
                                  mAllocator_c *allocator, int init) {
    dFdBase_c *info = dField::getInfo(0);
    if (info != NULL) {
        for (u32 z = 0; z < info->getBlockH(); z++) {
            for (u32 x = 0; x < info->getBlockW(); x++) {
                mBlocks[z][x].proc(x, z, pack, allocator, init);
            }
        }
    }
}

// .text:0x10CC
void dBgoDraw_c::blockMng_c::draw(nw4r::g3d::ResFile pack) {
    dBG::area_c area;
    area.setView();
    for (u32 z = 0; z < BLOCK_Z_NUM; z++) {
        for (u32 x = 0; x < BLOCK_X_NUM; x++) {
            if (area.isIn(x, z)) {
                mBlocks[z][x].draw(x, z, pack);
            }
        }
    }
}

// .text:0x1168
void dBgoDraw_c::blockMng_c::remove(nw4r::g3d::ResFile pack) {
    for (u32 z = 0; z < BLOCK_Z_NUM; z++) {
        for (u32 x = 0; x < BLOCK_X_NUM; x++) {
            mBlocks[z][x].remove(x, z, pack);
        }
    }
}

// .text:0x11E4
int dBgoDraw_c::create() {
    dFTR::enableHold();
    initHeap(dHeap::localHeap_p);
    bool ok = true;
    ok &= dBG::getPackBank()->load(dHeap::localHeap_p);
    if (ok) {
        dBG::mdlBank_c *bank = dBG::getMdlBank();
        if (bank->create(dHeap::localHeap_p)) {
            ok &= bank->load(dBG::bkLoader_c::KIND_FIRST);
        }
    }
    if (ok) {
        calcSeaColor();
        mBlocks.proc(nw4r::g3d::ResFile(dBG::getPackBank()->getData()),
                     &mAllocator, TRUE);
        initSeaZ();
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text:0x12C0
int dBgoDraw_c::execute() {
    dBG::mdlBank_c *bank = dBG::getMdlBank();
    calcSeaColor();
    calcGrassTrans();
    bank->load(dBG::bkLoader_c::KIND_SECOND);
    mBlocks.proc(nw4r::g3d::ResFile(dBG::getPackBank()->getData()), &mAllocator,
                 FALSE);
    dBgoDraw::l_frame.mFrame++;
    calcBgY();
    calcSeaDist();
    return SUCCEEDED;
}

// .text:0x135C
int dBgoDraw_c::draw() {
    mBlocks.draw(nw4r::g3d::ResFile(dBG::getPackBank()->getData()));
    return SUCCEEDED;
}

// .text:0x13A0
int dBgoDraw_c::doDelete() {
    bool ok = true;
    ok &= dBG::getMdlBank()->destroy();
    ok &= dBG::getPackBank()->unload();
    if (ok) {
        mBlocks.remove(nw4r::g3d::ResFile(dBG::getPackBank()->getData()));
        if (m_g3dHeap_p != NULL) {
            mHeap::destroyFrmHeap(m_g3dHeap_p);
            m_g3dHeap_p = NULL;
        }
        getScene();
        dFTR::enableHold();
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text:0x1440
BOOL dBgoDraw_c::initHeap(EGG::Heap *parent) {
    if (m_g3dHeap_p != NULL) {
        return TRUE;
    }
    BOOL ok = TRUE;
    if (m_g3dHeap_p == NULL) {
        m_g3dHeap_p = mHeap::createFrmHeap(
            0x19000, parent, "dBgoDraw_c::m_g3dHeap_p : BG g3d用バッファ", 0x20,
            mHeap::OPT_NONE);
        if (m_g3dHeap_p != NULL) {
            mAllocator.attach(m_g3dHeap_p, 0x20);
        }
        ok = m_g3dHeap_p != NULL;
    }
    return ok;
}

// The bg y level: a 600-frame wave.
// .text:0x14CC
void dBgoDraw_c::calcBgY() {
    f32 t = dBgoDraw::l_frame.getMod(600);
    f32 y;
    if (t < 200.0f) {
        y = 1.0f + -0.005f * t;
    } else if (t < 300.0f) {
        y = 0.01f * (t - 200.0f);
    } else if (t < 500.0f) {
        y = 1.0f + -0.005f * (t - 300.0f);
    } else {
        y = 0.01f * (t - 500.0f);
    }
    dBGCF::setBgY(y);
}

// .text:0x1590
void dBgoDraw_c::calcSeaColor() {
    mColor color;
    dEnvironment_c::calcColor(&color, 5);
    f32 rate = dBgoDraw::l_HIO.mSeaRate;
    f32 inv = 100.0f - rate;
    u8 r = (color.r * inv + dBgoDraw::l_HIO.mSeaR * rate) / 100.0f;
    u8 g = (color.g * inv + dBgoDraw::l_HIO.mSeaG * rate) / 100.0f;
    u8 b = (color.b * inv + dBgoDraw::l_HIO.mSeaB * rate) / 100.0f;
    dBgoDraw::l_seaColor.a = dBgoDraw::l_HIO.mSeaA;
    dBgoDraw::l_seaColor.r = r;
    dBgoDraw::l_seaColor.g = g;
    dBgoDraw::l_seaColor.b = b;
}

// The river texture follows the horizon on screen and shrinks with the camera
// distance. .text:0x16E0
void dBgoDraw_c::calcGrassTrans() {
    static const f32 l_screenYMin = 0.198f;
    static const f32 l_screenYMax = 0.7311f;
    static const f32 l_baseDist = 406.0f;
    static const f32 l_falloffDist = 416.0f;
    static const f32 l_minDist = 230.0f;
    static const f32 l_maxTransY = 470.0f;
    dCamera_c *camera = dCamera_c::sInstance;
    if (camera == NULL) {
        return;
    }
    mVec3_c pos;
    dAcPlayer_c *player = dPlayerMgr::getPlayerActor(4);
    if (player != NULL) {
        pos = player->getPos();
    } else {
        dWorld::toFieldPosition(&pos, &camera->getTargetPos());
    }
    f32 z = dWorld::angleToZ(dWorld::getPlayerHorizonAngle(NULL));
    mVec3_c horizon(0.0f, dBGCF::getGroundY(&pos, FALSE), z);
    mVec3_c curved;
    dWorld::curvePositionSimple(&curved, &horizon);
    f32 screenX, screenY;
    dCamera::worldToScreen(camera, &screenX, &screenY, &curved);
    dBgoDraw::l_riverTransY =
        dBgoDraw::l_HIO.mRiverTransY -
        (dBgoDraw::l_HIO.mRiverTransY - 40.0f) *
            ((screenY - l_screenYMin) / (l_screenYMax - l_screenYMin));
    mVec3_c dir = camera->getEyePos() - camera->getTargetPos();
    f32 dist = PSVECMag(dir);
    if (dist < l_minDist) {
        dist = l_minDist;
    }
    dBgoDraw::l_riverScale = dist / l_baseDist;
    if (dist > l_falloffDist) {
        dBgoDraw::l_riverTransY *=
            dBgoDraw::l_riverScale *
            (1.0f + (dist - 416.0f) * dBgoDraw::l_HIO.mRiverFalloff);
    } else {
        dBgoDraw::l_riverTransY *= dBgoDraw::l_riverScale;
    }
    if (dBgoDraw::l_riverTransY > l_maxTransY) {
        dBgoDraw::l_riverTransY = l_maxTransY;
    }
}

// The z of the sea's edge for each unit column.
// .text:0x18C0
void dBgoDraw_c::initSeaZ() {
    u8 scene = dScene::getCurrentScene();
    if (scene != dScene::SCENE_FIELD && scene != dScene::SCENE_DM_TITLE) {
        return;
    }
    for (u32 x = 0; x < SEA_UNIT_NUM; x++) {
        mSeaZ[x] = 3600.0f;
        for (u32 z = 0; z < 0x10; z++) {
            u32 unitZ = z + 0x50;
            dBGCF::groundChk_c chk(x, unitZ, 0, 0, 0);
            if (chk.mWater == BG_WATER_SEA) {
                mSeaZ[x] = 16.0f + 32.0f * unitZ;
                break;
            }
        }
    }
}

// .text:0x19D4
void dBgoDraw_c::calcSeaDist() {
    u8 scene = dScene::getCurrentScene();
    if (scene != dScene::SCENE_FIELD && scene != dScene::SCENE_DM_TITLE) {
        return;
    }
    mVec3_c pos;
    bool ok;
    dAcPlayer_c *player = dPlayerMgr::getPlayerActor(4);
    if (player != NULL) {
        pos = player->getPos();
        ok = true;
    } else {
        pos = dCamera_c::sCenter;
        ok = true;
    }
    if (ok) {
        int unitX, unitZ;
        dBGCF::posToUnit(&unitX, &unitZ, &pos);
        nw4r::math::VEC3 center;
        dFdBase_c::getUnitCenterPos(&center, unitX, unitZ);
        f32 t;
        f32 seaZ;
        if (pos.x >= center.x) {
            t = (pos.x - center.x) / 32.0f;
            seaZ = (t * getSeaZ(unitX + 1)) + ((1.0f - t) * getSeaZ(unitX));
        } else {
            t = (pos.x - (center.x - 32.0f)) / 32.0f;
            seaZ = (t * getSeaZ(unitX)) + ((1.0f - t) * getSeaZ(unitX - 1));
        }
        mVec3_c sea(pos.x, pos.y, seaZ);
        sndSetSeaDist(std::fabs(seaZ - pos.z));
    }
}

// .text:0x1B7C
u8 dBgoDraw_c::getScene() {
    return dScene::getCurrentScene();
}

// .text:0x1B80
f32 dBgoDraw_c::getSeaZ(u32 unitX) {
    u8 scene = dScene::getCurrentScene();
    if ((scene == dScene::SCENE_FIELD || scene == dScene::SCENE_DM_TITLE) &&
        unitX < SEA_UNIT_NUM) {
        return mSeaZ[unitX];
    }
    return 3600.0f;
}

// .text:0x1BE8
dBgoDraw_HIO_c::~dBgoDraw_HIO_c() {
}

// .text:0x1C28
dBgoDraw_c::~dBgoDraw_c() {
}
