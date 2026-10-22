// REL .text 0xCC..0x91CC, .ctors 0x0..0x4, .rodata 0x0..0x1F0, .data 0x0..0x850, .bss 0x8..0x2A8.
#include <game/game/d_m3d.hpp>
#include <game/game/d_a_ftr.hpp>
#include <game/game/d_ftr_manager.hpp>
#include <game/game/d_base.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_bg_util.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_bg_attr.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_environment.hpp>
#include <game/game/d_fg_draw.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_home_room_map.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_npc_list.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_shadow.hpp>
#include <game/game/d_date.hpp>
#include <game/framework/f_profile.hpp>
#include <game/snd/snd_id.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <revolution/mtx.h>
#include <string.h>
#include <game/game/d_a_player.hpp>

enum {
    SCENE_ATTR_NPC_ROOM = dScene::SCENE_ATTR_NPC_HOUSE | dScene::SCENE_ATTR_FTR_ROOM | dScene::SCENE_ATTR_ROOM,
    SCENE_ATTR_SHOP_ROOM = dScene::SCENE_ATTR_SHOP | dScene::SCENE_ATTR_ROOM,
};

// Unused; referencing anmChrPart_c's vtable queues anmChr_c::getType before anmVis_c::getType among the weak
// functions. The original's equivalent is unknown; d_ftr_manager, with the same headers, must not have it.
inline void makePartAnm() {
    m3d::anmChrPart_c anm;
}

struct initState_c {
    int mState;
    u8 mDraw;
};

// 0xCC
namespace dAFtr {
CUSTOM_BASE_PROFILE(A_FTR, daFTR_c, fProf::A_FTR, fProf::DRAW_ORDER::A_FTR);
} // namespace dAFtr

// 0x454
void *daFTR_c::operator new(size_t size) {
    u32 blockSize = dFtrm_c::getPool()->mSize;
    void *p = dFtrm_c::getPool()->alloc();
    if (p == NULL) {
        return NULL;
    }
    memset(p, 0, blockSize);
    return p;
}

// 0x4B4
void daFTR_c::operator delete(void *p) {
    dFtrm_c::getPool()->free(p);
}

// 0x4E8
int daFTR_c::getKind() {
    return mInfo.mKind;
}

// 0x4F4
int daFTR_c::getLayer() {
    return mInfo.mLayer;
}

// 0x500
int daFTR_c::getFtrSize() {
    if (mpBITM != NULL) {
        return mpBITM->getFtrSize();
    }
    return dItem::FTR_SIZE_1x1;
}

// 0x534
int daFTR_c::getItemKind() {
    if (mpBITM != NULL) {
        return mpBITM->getKind();
    }
    return dItem::KIND_NONE;
}

// 0x564
BOOL daFTR_c::canWalkOn() {
    if (mpBITM != NULL) {
        return mpBITM->m_ftrPassThru;
    }
    return FALSE;
}

// 0x584
BOOL daFTR_c::isSyncDisabled() {
    return getKind() == FTR_KIND_HIDDEN || getKind() == FTR_KIND_DUMMY;
}

// 0x5DC
BOOL daFTR_c::isDummy() {
    return getKind() == FTR_KIND_DUMMY;
}

// 0x608
f32 daFTR_c::getTopY() const {
    if (mpBITM != NULL) {
        return mPos.y + (s8)mpBITM->m_ftrHeight;
    }
    return mPos.y;
}

// 0x658
int daFTR_c::getTopKind() {
    if (mpBITM != NULL) {
        return mpBITM->getFtrTable();
    }
    return 0;
}

// 0x688
int daFTR_c::getFtrFunc() {
    if (mpBITM != NULL) {
        return mpBITM->getFtrFunc();
    }
    return 1;
}

// 0x6B8
int daFTR_c::getFtrLamp() {
    if (mpBITM != NULL) {
        return mpBITM->getFtrLamp();
    }
    return 0;
}

// 0x6EC
dItem::Item daFTR_c::getItem() {
    return dItem::Item((u16)mInfo.mItem);
}

// 0x6FC
EGG::FrmHeap *daFTR_c::getHeap() {
    return dFtrm_c::sRecBank.get(&mRecSlot.mIndex);
}

// 0x710
dItem::NpcMsg *daFTR_c::getNpcMsg() {
    const dItem::BITM *bitm = getBITM();
    if (bitm != NULL) {
        int kind = bitm->m_ftrKind;
        return dItem::infoBank_c::get()->getNpcMsg(kind);
    }
    return NULL;
}

// 0x768
u32 daFTR_c::makeInfo(int x, int z, const dItem::Item &item, int rot, int layer, int kind) {
    info_c info;
    info.mX = x;
    info.mZ = z;
    info.mItem = item.getId();
    info.mRot = rot;
    info.mKind = kind;
    info.mLayer = layer;
    return info.mValue;
}

// 0x798
void dFTR::counter_c::clear() {
    mCount = 0;
}

// 0x7A4
void dFTR::counter_c::set(int count) {
    mCount = count;
}

// 0x7AC
BOOL dFTR::counter_c::calc() {
    if (mCount == 0) {
        return TRUE;
    }
    if (mCount != 0) {
        mCount--;
    }
    return FALSE;
}

// 0x7D4
void daFTR_c::initSwitch() {
    BOOL on = TRUE;
    if (getKind() == FTR_KIND_APPEAR || isSyncDisabled()) {
        if (mpBITM != NULL) {
            on = !mpBITM->_188_0;
        }
    } else {
        int x;
        int z;
        if (getUnit(&x, &z, NULL, mAng(0))) {
            u8 scene = dScene::getCurrentScene();
            on = isRoomMapSet(x, z, getLayer(), scene);
        }
    }
    changeSwitch(on, FALSE);
}

// 0x8D4
void daFTR_c::applySwitch() {
    if (!isSyncDisabled()) {
        mSwitch = mSwitchNext;
    }
}

// 0x91C
void daFTR_c::saveSwitch() {
    if (!isSyncDisabled()) {
        int x;
        int z;
        if (getUnit(&x, &z, NULL, mAng(0))) {
            u8 scene = dScene::getCurrentScene();
            if (mProfName == fProf::FTR_ACC) {
                setRoomMap(x, z, getLayer(), TRUE, scene);
            } else {
                setRoomMap(x, z, getLayer(), getSwitch(), scene);
            }
        }
    }
}

// 0xA28
BOOL daFTR_c::getSwitch() {
    return mSwitchNext;
}

// 0xA30
BOOL daFTR_c::isSwitchChanging() {
    return mSwitch != mSwitchNext;
}

// 0xA4C
BOOL daFTR_c::startHold() {
    if (!mSwitchWait) {
        toggleSwitch(TRUE);
        return TRUE;
    }
    return FALSE;
}

// 0xA88
void daFTR_c::toggleSwitch(BOOL keep) {
    if (mEnabled) {
        changeSwitch((mSwitchNext + 1) & 1, keep);
    }
}

// 0xAB8
void daFTR_c::changeSwitch(u32 sw, BOOL keep) {
    if (keep) {
        mSwitchNext = sw;
    } else {
        mSwitch = sw;
        mSwitchNext = sw;
    }
}

// Stripped by the linker; leaves its message in .data.
void checkInfo(const dItem::BITM *info_p) {
    static const char *message = "No info_p";
    if (info_p == NULL) {
        OSReport(message);
    }
}

// 0xAD4
void daFTR_c::loader_c::onLoaded() {
    dItem::resLoader_c::onLoaded();
    nw4r::g3d::ResFile res(getData());
    res.Init();
    res.Bind(nw4r::g3d::ResFile(dFtrm_c::sLoader.getFile("int_common.brres")));
    res.Bind(res);
    if (getDataSize() >= dFtrm_c::sMinResSize) {
        dItem::infoBank_c::get()->getBITM(getItem());
    }
}

// 0xB78
u32 daFTR_c::getAnmFrameMax(u32 idx, int types) {
    u32 anmIdx = idx & 1;
    u32 max = 0;
    nw4r::g3d::ResFile res(mRes.getData());
    if ((types & ANM_CHR) && anmIdx < res.GetResAnmChrNumEntries()) {
        u16 frame = res.GetResAnmChr(anmIdx).GetNumFrame();
        if (frame > max) {
            max = frame;
        }
    }
    if ((types & ANM_CLR) && anmIdx < res.GetResAnmClrNumEntries()) {
        u16 frame = res.GetResAnmClr(anmIdx).GetNumFrame();
        if (frame > max) {
            max = frame;
        }
    }
    if ((types & ANM_SRT) && anmIdx < res.GetResAnmTexSrtNumEntries()) {
        u16 frame = res.GetResAnmTexSrt(anmIdx).GetNumFrame();
        if (frame > max) {
            max = frame;
        }
    }
    if ((types & ANM_PAT) && anmIdx < res.GetResAnmTexPatNumEntries()) {
        u16 frame = res.GetResAnmTexPat(anmIdx).GetNumFrame();
        if (frame > max) {
            max = frame;
        }
    }
    if ((types & ANM_VIS) && anmIdx < res.GetResAnmVisNumEntries()) {
        u16 frame = res.GetResAnmVis(anmIdx).GetNumFrame();
        if (frame > max) {
            max = frame;
        }
    }
    return max;
}

// 0xCC8
void daFTR_c::createAnm(f32 rate, u32 idx, int playMode, u16 frame, int types) {
    u32 anmIdx = idx & 1;
    nw4r::g3d::ResFile res(mRes.getData());
    nw4r::g3d::ResMdl mdl = getResMdl();
    if ((types & ANM_VIS) && anmIdx < res.GetResAnmVisNumEntries()) {
        mAnmVis.create(mdl, res.GetResAnmVis(anmIdx), &mAllocator, NULL);
        mAnmVis.setFrame(frame);
        mAnmVis.mPlayMode = playMode;
        mAnmVis.setRate(rate);
        mMdl.setAnm(mAnmVis);
    }
    if ((types & ANM_CLR) && anmIdx < res.GetResAnmClrNumEntries()) {
        mAnmClr.create(mdl, res.GetResAnmClr(anmIdx), &mAllocator, NULL, 1);
        mAnmClr.setFrame(frame, 0);
        mAnmClr.setPlayMode((m3d::playMode_e)playMode, 0);
        mAnmClr.setRate(rate, 0);
        mMdl.setAnm(mAnmClr);
    }
    if ((types & ANM_SRT) && anmIdx < res.GetResAnmTexSrtNumEntries()) {
        mAnmTexSrt.create(mdl, res.GetResAnmTexSrt(anmIdx), &mAllocator, NULL, 1);
        mAnmTexSrt.setFrame(frame, 0);
        mAnmTexSrt.setPlayMode((m3d::playMode_e)playMode, 0);
        mAnmTexSrt.setRate(rate, 0);
        mMdl.setAnm(mAnmTexSrt);
    }
    if ((types & ANM_PAT) && anmIdx < res.GetResAnmTexPatNumEntries()) {
        mAnmTexPat.create(mdl, res.GetResAnmTexPat(anmIdx), &mAllocator, NULL, 1);
        mAnmTexPat.setFrame(frame, 0);
        mAnmTexPat.setPlayMode((m3d::playMode_e)playMode, 0);
        mAnmTexPat.setRate(rate, 0);
        mMdl.setAnm(mAnmTexPat);
    }
    if ((types & ANM_CHR) && anmIdx < res.GetResAnmChrNumEntries()) {
        mAnmChr.create(mdl, res.GetResAnmChr(anmIdx), &mAllocator, NULL);
        mAnmChr.setFrame(frame);
        mAnmChr.mPlayMode = playMode;
        mAnmChr.setRate(rate);
        mMdl.setAnm(mAnmChr);
    }
}

// 0x1034
void daFTR_c::changeAnm(f32 rate, u32 idx, int playMode, u16 frame, int types) {
    u32 anmIdx = idx & 1;
    nw4r::g3d::ResFile res(mRes.getData());
    if ((types & ANM_VIS) && anmIdx < res.GetResAnmVisNumEntries()) {
        nw4r::g3d::ResAnmVis anm = res.GetResAnmVis(anmIdx);
        u16 start = frame == ANM_FRAME_CURRENT ? mAnmVis.getFrame() : frame;
        mAnmVis.setAnm(mMdl, anm, (m3d::playMode_e)playMode);
        mAnmVis.setFrame(start);
        mAnmVis.setRate(rate);
        mMdl.setAnm(mAnmVis);
    }
    if ((types & ANM_CLR) && anmIdx < res.GetResAnmClrNumEntries()) {
        nw4r::g3d::ResAnmClr anm = res.GetResAnmClr(anmIdx);
        u16 start = frame == ANM_FRAME_CURRENT ? mAnmClr.getFrame(0) : frame;
        mAnmClr.setAnm(mMdl, anm, 0, (m3d::playMode_e)playMode);
        mAnmClr.setFrame(start, 0);
        mAnmClr.setRate(rate, 0);
        mMdl.setAnm(mAnmClr);
    }
    if ((types & ANM_SRT) && anmIdx < res.GetResAnmTexSrtNumEntries()) {
        nw4r::g3d::ResAnmTexSrt anm = res.GetResAnmTexSrt(anmIdx);
        u16 start = frame == ANM_FRAME_CURRENT ? mAnmTexSrt.getFrame(0) : frame;
        mAnmTexSrt.setAnm(mMdl, anm, 0, (m3d::playMode_e)playMode);
        mAnmTexSrt.setFrame(start, 0);
        mAnmTexSrt.setRate(rate, 0);
        mMdl.setAnm(mAnmTexSrt);
    }
    if ((types & ANM_PAT) && anmIdx < res.GetResAnmTexPatNumEntries()) {
        nw4r::g3d::ResAnmTexPat anm = res.GetResAnmTexPat(anmIdx);
        u16 start = frame == ANM_FRAME_CURRENT ? mAnmTexPat.getFrame(0) : frame;
        mAnmTexPat.setAnm(mMdl, anm, 0, (m3d::playMode_e)playMode);
        mAnmTexPat.setFrame(start, 0);
        mAnmTexPat.setRate(rate, 0);
        mMdl.setAnm(mAnmTexPat);
    }
    if ((types & ANM_CHR) && anmIdx < res.GetResAnmChrNumEntries()) {
        nw4r::g3d::ResAnmChr anm = res.GetResAnmChr(anmIdx);
        u16 start = frame == ANM_FRAME_CURRENT ? mAnmChr.getFrame() : frame;
        mAnmChr.setAnm(mMdl, anm, (m3d::playMode_e)playMode);
        mAnmChr.setFrame(start);
        mAnmChr.setRate(rate);
        mMdl.setAnm(mAnmChr);
    }
}

// 0x1464
void daFTR_c::setAnmEnd(f32 rate, u32 idx, int playMode, int types) {
    u32 anmIdx = idx & 1;
    nw4r::g3d::ResFile res(mRes.getData());
    if ((types & ANM_VIS) && anmIdx < res.GetResAnmVisNumEntries()) {
        nw4r::g3d::ResAnmVis anm = res.GetResAnmVis(anmIdx);
        mAnmVis.setAnm(mMdl, anm, (m3d::playMode_e)playMode);
        mAnmVis.setFrame(anm.GetNumFrame() - 1);
        mAnmVis.setRate(rate);
        mMdl.setAnm(mAnmVis);
    }
    if ((types & ANM_CLR) && anmIdx < res.GetResAnmClrNumEntries()) {
        nw4r::g3d::ResAnmClr anm = res.GetResAnmClr(anmIdx);
        mAnmClr.setAnm(mMdl, anm, 0, (m3d::playMode_e)playMode);
        mAnmClr.setFrame(anm.GetNumFrame() - 1, 0);
        mAnmClr.setRate(rate, 0);
        mMdl.setAnm(mAnmClr);
    }
    if ((types & ANM_SRT) && anmIdx < res.GetResAnmTexSrtNumEntries()) {
        nw4r::g3d::ResAnmTexSrt anm = res.GetResAnmTexSrt(anmIdx);
        mAnmTexSrt.setAnm(mMdl, anm, 0, (m3d::playMode_e)playMode);
        mAnmTexSrt.setFrame(anm.GetNumFrame() - 1, 0);
        mAnmTexSrt.setRate(rate, 0);
        mMdl.setAnm(mAnmTexSrt);
    }
    if ((types & ANM_PAT) && anmIdx < res.GetResAnmTexPatNumEntries()) {
        nw4r::g3d::ResAnmTexPat anm = res.GetResAnmTexPat(anmIdx);
        mAnmTexPat.setAnm(mMdl, anm, 0, (m3d::playMode_e)playMode);
        mAnmTexPat.setFrame(anm.GetNumFrame() - 1, 0);
        mAnmTexPat.setRate(rate, 0);
        mMdl.setAnm(mAnmTexPat);
    }
    if ((types & ANM_CHR) && anmIdx < res.GetResAnmChrNumEntries()) {
        nw4r::g3d::ResAnmChr anm = res.GetResAnmChr(anmIdx);
        mAnmChr.setAnm(mMdl, anm, (m3d::playMode_e)playMode);
        mAnmChr.setFrame(anm.GetNumFrame() - 1);
        mAnmChr.setRate(rate);
        mMdl.setAnm(mAnmChr);
    }
}

// 0x17A8
bool daFTR_c::createAnmChr(f32 rate, nw4r::g3d::ResAnmChr anm, int playMode, u32 frame) {
    bool ok = true;
    ok &= mAnmChr.create(getResMdl(), anm, &mAllocator, NULL);
    mAnmChr.setFrame(frame);
    mAnmChr.mPlayMode = playMode;
    mAnmChr.setRate(rate);
    return ok & mMdl.setAnm(mAnmChr);
}

// 0x1890
void daFTR_c::changeAnmChr(f32 rate, nw4r::g3d::ResAnmChr anm, int playMode, u32 frame) {
    u16 start = frame == ANM_FRAME_CURRENT ? mAnmChr.getFrame() : frame;
    mAnmChr.setAnm(mMdl, anm, (m3d::playMode_e)playMode);
    mAnmChr.setFrame(start);
    mAnmChr.setRate(rate);
    mMdl.setAnm(mAnmChr);
}

// 0x1988
void daFTR_c::setAnmChrEnd(f32 rate, nw4r::g3d::ResAnmChr anm, int playMode) {
    mAnmChr.setAnm(mMdl, anm, (m3d::playMode_e)playMode);
    mAnmChr.setFrame(anm.GetNumFrame() - 1);
    mAnmChr.setRate(rate);
    mMdl.setAnm(mAnmChr);
}

// 0x1A44
BOOL daFTR_c::playAnm(f32 rate, int types) {
    bool stop = true;
    bool setRate = rate != -1.0f;
    if ((types & ANM_CHR) && mAnmChr.IsBound()) {
        if (setRate) {
            mAnmChr.setRate(rate);
        }
        mAnmChr.play();
        stop &= mAnmChr.isStop();
    }
    if ((types & ANM_CLR) && mAnmClr.IsBound()) {
        if (setRate) {
            mAnmClr.setRate(rate, 0);
        }
        mAnmClr.play();
        stop &= mAnmClr.isStop(0);
    }
    if ((types & ANM_PAT) && mAnmTexPat.IsBound()) {
        if (setRate) {
            mAnmTexPat.setRate(rate, 0);
        }
        mAnmTexPat.play();
        stop &= mAnmTexPat.isStop(0);
    }
    if ((types & ANM_SRT) && mAnmTexSrt.IsBound()) {
        if (setRate) {
            mAnmTexSrt.setRate(rate, 0);
        }
        mAnmTexSrt.play();
        stop &= mAnmTexSrt.isStop(0);
    }
    if ((types & ANM_VIS) && mAnmVis.IsBound()) {
        if (setRate) {
            mAnmVis.setRate(rate);
        }
        mAnmVis.play();
        stop &= mAnmVis.isStop();
    }
    return stop;
}

// 0x1C80
BOOL daFTR_c::isAnmNum(u32 num, int types) {
    nw4r::g3d::ResFile res(mRes.getData());
    u32 chrNum = res.GetResAnmChrNumEntries();
    u32 clrNum = res.GetResAnmClrNumEntries();
    u32 srtNum = res.GetResAnmTexSrtNumEntries();
    u32 patNum = res.GetResAnmTexPatNumEntries();
    u32 visNum = res.GetResAnmVisNumEntries();
    BOOL ok = TRUE;
    if ((types & ANM_CHR) && chrNum != 0 && chrNum != num) {
        return FALSE;
    }
    if ((types & ANM_CLR) && clrNum != 0 && clrNum != num) {
        ok = FALSE;
    }
    if ((types & ANM_SRT) && srtNum != 0 && srtNum != num) {
        ok = FALSE;
    }
    if ((types & ANM_PAT) && patNum != 0 && patNum != num) {
        ok = FALSE;
    }
    if ((types & ANM_VIS) && visNum != 0 && visNum != num) {
        ok = FALSE;
    }
    return ok;
}

// 0x1D8C
f32 daFTR_c::getAnmFrame() {
    if (mAnmChr.IsBound()) {
        return mAnmChr.getFrame();
    }
    if (mAnmClr.IsBound()) {
        return mAnmClr.getFrame(0);
    }
    if (mAnmTexPat.IsBound()) {
        return mAnmTexPat.getFrame(0);
    }
    if (mAnmTexSrt.IsBound()) {
        return mAnmTexSrt.getFrame(0);
    }
    if (mAnmVis.IsBound()) {
        return mAnmVis.getFrame();
    }
    return 0.0f;
}

// 0x1E54
BOOL daFTR_c::isParentIdle() {
    if (getLayer() == 1) {
        daFTR_c *parent = dFTR::getList()->get(mParent);
        if (parent != NULL) {
            return parent->isIdle();
        }
    }
    return TRUE;
}

// 0x1EC0
BOOL daFTR_c::startRotate(const mAng &angle) {
    mTargetAngle = mAngle.y + angle;
    sendEvent(STATE_ROTATE);
    mSndObj.startSound(SE_rotate);
    return TRUE;
}

// 0x1F1C
BOOL daFTR_c::startPush(const mAng &angle) {
    mVec3_c dir(0.0f, 0.0f, 32.0f);
    dir.rotY(angle);
    mTargetPos = mPos + dir;
    setTargetAngle(angle);
    sendEvent(STATE_PUSH);
    return TRUE;
}

// 0x1FCC
BOOL daFTR_c::startPull(const mAng &angle) {
    mAng back(angle.mAngle + 0x8000);
    mVec3_c dir(0.0f, 0.0f, 32.0f);
    dir.rotY(back);
    mTargetPos = mPos + dir;
    setTargetAngle(angle);
    sendEvent(STATE_PULL);
    return TRUE;
}

// 0x2084
void daFTR_c::startShrink() {
    sendEvent(STATE_SHRINK);
}

// 0x2098
int daFTR_c::sendEvent(int state) {
    static const initProc_t l_initProc[STATE_NUM] = {
        &daFTR_c::initAppear,   &daFTR_c::initWait,   &daFTR_c::initRotate, &daFTR_c::initPush,
        &daFTR_c::initPull,     &daFTR_c::initRemove, &daFTR_c::initShrink, &daFTR_c::initWait,
        &daFTR_c::initHidden,   &daFTR_c::initTalk,   &daFTR_c::initTalkEnd, &daFTR_c::initDelete,
    };
    if ((u32)state < STATE_NUM && (this->*l_initProc[state])()) {
        mState = state;
        return TRUE;
    }
    return FALSE;
}

// 0x2104
void daFTR_c::executeState() {
    static const executeProc_t l_executeProc[STATE_NUM] = {
        &daFTR_c::executeAppear, &daFTR_c::executeWait,   &daFTR_c::executeRotate, &daFTR_c::executePush,
        &daFTR_c::executePull,   &daFTR_c::executeRemove, &daFTR_c::executeShrink, &daFTR_c::executeWait,
        &daFTR_c::executeHidden, &daFTR_c::executeTalk,   &daFTR_c::executeTalkEnd, &daFTR_c::executeDelete,
    };
    if (mState < STATE_NUM) {
        (this->*l_executeProc[mState])();
    }
}

// 0x2144
BOOL daFTR_c::initAppear() {
    static const s16 initialSpeed = 10000;
    mAppearTimer = 0;
    mAppearAngle = 0;
    mAppearSpeed = initialSpeed;
    mAppearAmp = 0.5f;
    mBgScale = 1.0f / 3.0f;
    mScale.x = 0.0f;
    mScale.y = 0.0f;
    mScale.z = 0.0f;
    getCenter(&mEffectPos);
    if (!getLayer()) {
        mEffectPos.y = mPos.y;
    } else {
        mEffectPos.y = dFTR::getGroundY(&mEffectPos);
    }
    mEffectPos.y += 2.0f;
    if (dScene::getCurrentScene() != dScene::SCENE_RM_AUCTION) {
        dFtrm_c::appearItem(getItem().getId(), &mEffectPos);
    }
    return TRUE;
}

// 0x2234
void daFTR_c::executeAppear() {
    static const s16 acceleration = 900;
    if (mAppearTimer < 40) {
        mAppearTimer++;
        mStateTimer = 0;
        if (mAppearTimer == 35) {
            startAppearSe();
        }
    } else if (mAppearTimer < 46) {
        BOOL ready = FALSE;
        if (getResMdl().IsValid()) {
            if (mMdl.getScn() != NULL) {
                ready = TRUE;
            }
        }
        if (ready) {
            mAppearTimer++;
            if (!mFlags.mAppearEffect) {
                mVec3_c center;
                getCenter(&center);
                appearEffect(&center);
                mFlags.mAppearEffect = 1;
            }
        } else if (dScene::getCurrentScene() != dScene::SCENE_RM_AUCTION) {
            mFlags.mIcon = 1;
        }
    } else {
        s16 prev = mAppearAngle;
        mFlags.mDraw = 1;
        mAppearAngle += mAppearSpeed;
        s16 angle = mAppearAngle;
        f32 prevSin = nw4r::math::SinIdx(prev);
        f32 sin = nw4r::math::SinIdx(angle);
        if (prevSin * sin < 0.0f) {
            mAppearSpeed += acceleration;
            mAppearAmp *= 0.8f;
        }
        f32 bounce = mAppearAmp * sin;
        f32 thin = 1.0f - bounce;
        f32 tall = 1.0f + bounce;
        mScale.x = thin;
        mScale.y = tall;
        mScale.z = thin;
        if (mBgScale == 1.0f && mAppearAmp < 0.005f) {
            sendEvent(STATE_WAIT);
        }
        if (!mFlags.mAppearSmoke) {
            mVec3_c center;
            getCenter(&center);
            switch (getFtrSize()) {
            case dItem::FTR_SIZE_1x1:
                dEffect::create("af_itm_appear_S", &center, &mAngle, NULL);
                break;
            case dItem::FTR_SIZE_1x2:
                dEffect::create("af_itm_appear_M", &center, &mAngle, NULL);
                break;
            default:
                dEffect::create("af_itm_appear_L", &center, &mAngle, NULL);
                break;
            }
            mFlags.mAppearSmoke = 1;
        }
        mStateTimer++;
    }
}

// 0x24C0
BOOL daFTR_c::initWait() {
    return TRUE;
}

// 0x24C8
void daFTR_c::executeWait() {}

// 0x24CC
BOOL daFTR_c::initRotate() {
    mTopItems.pickUp(this);
    mAng angle = getTargetAngle() - mAngle.y;
    moveUnits(NULL, angle);
    mRotateWait = dFtrm_c::getRotateWait();
    return TRUE;
}

// 0x2530
void daFTR_c::executeRotate() {
    if (mRotateWait != 0) {
        if (--mRotateWait == 0) {
            BOOL left = (s16)(getTargetAngle() - mAngle.y) < 0;
            onRotateStart(left);
            if (!getLayer() && getTopKind() == dItem::FTR_TABLE_ITEM) {
                dFTR::unitList_c units;
                getUnits(&units, NULL);
                for (u32 i = 0; i < units.mNum; i++) {
                    daFTR_c *top = dFTR::getUnitMap()->get(units.get(i)->x, units.get(i)->z, 1);
                    if (top != NULL) {
                        top->onRotateStart(left);
                    }
                }
            }
        }
    } else if (mRotateWait == 0) {
        BOOL left = (s16)(getTargetAngle() - mAngle.y) < 0;
        onRotate(left);
        if (!getLayer() && getTopKind() == dItem::FTR_TABLE_ITEM) {
            dFTR::unitList_c units;
            getUnits(&units, NULL);
            for (u32 i = 0; i < units.mNum; i++) {
                daFTR_c *top = dFTR::getUnitMap()->get(units.get(i)->x, units.get(i)->z, 1);
                if (top != NULL) {
                    top->onRotate(left);
                }
            }
        }
        mAng angle = mAngle.y;
        s16 step = 0x4000 / dFtrm_c::getRotateSpeed();
        bool done = sLib::chaseAngle(&angle.mAngle, getTargetAngle(), step);
        mAngle.y = angle;
        if (done) {
            clearUnit();
            if (!getLayer() && getTopKind() == dItem::FTR_TABLE_ITEM) {
                dFTR::unitList_c units;
                getUnits(&units, NULL);
                for (u32 i = 0; i < units.mNum; i++) {
                    daFTR_c *top = dFTR::getUnitMap()->get(units.get(i)->x, units.get(i)->z, 1);
                    if (top != NULL) {
                        top->clearUnit();
                    }
                }
            }
            sendEvent(STATE_WAIT);
            mTopItems.put(this);
        }
    }
}

// 0x2888
BOOL daFTR_c::initPush() {
    mVec3_c ofs = mTargetPos - mPos;
    mTopItems.pickUp(this);
    moveUnits(&ofs, mAng(0));
    if (dFtrm_c::sTransAnm.startPush(&mHoldId, &mPos, mTargetAngle)) {
        if (!onMoveStart(TRUE)) {
            startMoveSe();
        }
        return TRUE;
    }
    return FALSE;
}

// 0x2940
void daFTR_c::executePush() {
    mVec3_c pos;
    if (dFtrm_c::sTransAnm.calc(&mHoldId, &pos)) {
        clearUnit();
        if (!getLayer() && getTopKind() == dItem::FTR_TABLE_ITEM) {
            dFTR::unitList_c units;
            getUnits(&units, NULL);
            for (u32 i = 0; i < units.mNum; i++) {
                daFTR_c *top = dFTR::getUnitMap()->get(units.get(i)->x, units.get(i)->z, 1);
                if (top != NULL) {
                    top->clearUnit();
                }
            }
        }
        mPos = mTargetPos;
        sendEvent(STATE_WAIT);
        mTopItems.put(this);
    } else {
        mPos = pos;
    }
}

// 0x2AB0
BOOL daFTR_c::initPull() {
    mVec3_c ofs = mTargetPos - mPos;
    mTopItems.pickUp(this);
    moveUnits(&ofs, mAng(0));
    if (dFtrm_c::sTransAnm.startPull(&mHoldId, &mPos, mTargetAngle)) {
        if (!onMoveStart(FALSE)) {
            startMoveSe();
        }
        return TRUE;
    }
    return FALSE;
}

// 0x2B68
void daFTR_c::executePull() {
    executePush();
}

// 0x2B6C
BOOL daFTR_c::initRemove() {
    onRemove();
    mStateTimer = 0;
    mVec3_c center;
    getCenter(&center);
    dFtrm_c::setEffectPos(&center);
    startVanishSe();
    return TRUE;
}

// 0x2BCC
void daFTR_c::executeRemove() {
    sLib::chase(&mScale.x, 0.0f, 1.0f / 15.0f);
    mScale.y = mScale.z = mScale.x;
    if (mScale.x == 0.0f) {
        deleteRequest();
    }
    if (mStateTimer == 1) {
        mVec3_c center;
        getCenter(&center);
        appearEffect(&center);
    }
    mStateTimer++;
}

// 0x2C64
BOOL daFTR_c::initShrink() {
    return initRemove();
}

// 0x2C68
void daFTR_c::executeShrink() {
    sLib::chase(&mScale.x, 0.0f, 1.0f / 15.0f);
    mScale.y = mScale.z = mScale.x;
    if (mScale.x == 0.0f) {
        sendEvent(STATE_VANISHED);
    } else if (mStateTimer == 1) {
        mVec3_c center;
        getCenter(&center);
        appearEffect(&center);
    }
    mStateTimer++;
}

// 0x2D14
BOOL daFTR_c::initHidden() {
    return TRUE;
}

// 0x2D1C
void daFTR_c::executeHidden() {}

// 0x2D20
BOOL daFTR_c::initTalk() {
    static const s16 messageCode = 0x1D;
    attachDemoActor(this);
    setMessageLabel("sys_MESS/SYS_Player");
    setMessageCode(messageCode);
    dItem::name_c name(getItem());
    if (getLayer() == 1) {
        if (mMsgItem1.getId() == dItem::ITEM_ID_NONE) {
            mMsgItem1 = getItem();
        }
    } else {
        if (mMsgItem0.getId() == dItem::ITEM_ID_NONE) {
            mMsgItem0 = getItem();
        }
    }
    if (dItem::isRealItemId(mMsgItem0.getId())) {
        mpController->setItemName(0, mMsgItem0);
    } else {
        mpController->clearWord(0);
    }
    if (dItem::isRealItemId(mMsgItem1.getId())) {
        mpController->setItemName(1, mMsgItem1);
    } else {
        mpController->clearWord(1);
    }
    mpController->requestStart(0);
    return TRUE;
}

// 0x2EC4
void daFTR_c::executeTalk() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (!comp->isCurrentMethod(&dMsg::Comp_c::executeHidden)) {
            sendEvent(STATE_TALK_END);
        }
    }
}

// 0x2F44
BOOL daFTR_c::initTalkEnd() {
    return TRUE;
}

// 0x2F4C
void daFTR_c::executeTalkEnd() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (comp->isCurrentMethod(&dMsg::Comp_c::executeHidden)) {
            detachDemoActor(this);
            dDemo::finishTalk(this);
        }
    }
}

// 0x2FD4
BOOL daFTR_c::initDelete() {
    deleteRequest();
    return TRUE;
}

// 0x2FF8
void daFTR_c::executeDelete() {}

// 0x2FFC
int daFTR_c::create() {
    initFtr();
    if (loadModel()) {
        createModel();
        mFlags.mCreated = 1;
        if (isSyncDisabled()) {
            dFtrm_c::getCurrent()->mNew = this;
            dFtrm_c::getCurrent()->replace(this);
        }
        if (isDummy()) {
            deleteRequest();
        }
        return SUCCEEDED;
    }
    return NOT_READY;
}

// 0x30A0
int daFTR_c::preExecute() {
    if (dDemoActor_c::preExecute() == FALSE) {
        return NOT_READY;
    }
    if (getLayer() == 1) {
        return NOT_READY;
    }
    return SUCCEEDED;
}

// 0x3100
int daFTR_c::execute() {
    mFlags.mIcon = 0;
    calcSound();
    executeState();
    setMtx(FALSE);
    calcBg();
    if (mState != STATE_APPEAR && mState != STATE_REMOVE) {
        if (mFlags.mToggle) {
            toggleSwitch(TRUE);
            mFlags.mToggle = 0;
        }
        executeHook();
        executeMain();
        speak();
        setDpdBox();
    } else {
        executeIdle();
        if (isSwitchChanging()) {
            toggleSwitch(FALSE);
        }
    }
    if (!isSyncDisabled()) {
        executeLamp();
        applySwitch();
        executeChildren();
    }
    calcSwitchNode();
    return SUCCEEDED;
}

// 0x322C
int daFTR_c::draw() {
    if (mFlags.mIcon) {
        dFgDraw::add(getItem().getId(), mEffectPos, mVec3_c(1.0f, 1.0f, 1.0f), 0, 0, mAng3_c(0, 0, 0));
    }
    if (isDraw()) {
        mVec3_c scale = getScale();
        mMdl.setScale(scale.x, scale.y, scale.z);
        calcMdl(NULL);
        entryMdl();
        drawExtra();
        mTopItems.draw(this);
        drawShadow(&scale);
    }
    return SUCCEEDED;
}

// 0x335C
void daFTR_c::calcMdl(const mMtx_c *mtx) {
    if (mMdl.getScn() != NULL) {
        if (mtx != NULL) {
            mMdl.setLocalMtx(mtx);
        }
        mMdl.calc(false);
    }
}

// 0x33AC
void daFTR_c::entryMdl() {
    if (mMdl.getScn() != NULL) {
        mMdl.entry();
    }
}

// 0x33CC
int daFTR_c::doDelete() {
    if (!mFlags.mRemoved) {
        dFtrm_c::getCurrent()->remove(this);
        if (!deleteRes()) {
            return NOT_READY;
        }
        mFlags.mRemoved = 1;
    }
    if (mRes.release()) {
        deleteLamp();
        if (mState != STATE_REMOVE && mState != STATE_VANISHED) {
            saveSwitch();
        }
        if (mState == STATE_REMOVE || mState == STATE_DELETE) {
            int x;
            int z;
            BOOL found = getUnit(&x, &z, NULL, mAng(0));
            if (dScene::isCurrentSceneAttr(SCENE_ATTR_NPC_ROOM)) {
                removeUnits(FALSE, FALSE);
            } else if (dScene::isCurrentSceneAttr(SCENE_ATTR_SHOP_ROOM)) {
                dFdBase_c *fd = dField::getInfo(0);
                if (fd != NULL) {
                    removeUnits(FALSE, FALSE);
                    u8 scene = dScene::getCurrentScene();
                    if (scene != dScene::SCENE_RM_AUCTION && fd != NULL) {
                        static dItem::Item l_soldOutSign(dItem::ITEM_IDX_SOLD_OUT_SIGN_00);
                        static dItem::Item l_soldOutSignGrace(dItem::ITEM_IDX_SOLD_OUT_SIGN_01);
                        if (found) {
                            if (scene == dScene::SCENE_RM_GRACE) {
                                fd->setItem(&l_soldOutSignGrace, x, z, 0);
                            } else {
                                fd->setItem(&l_soldOutSign, x, z, 0);
                            }
                        }
                    }
                }
            } else {
                removeUnits(TRUE, FALSE);
            }
        }
        mMdl.remove();
        mAnmChr.remove();
        mAnmClr.remove();
        mAnmTexPat.remove();
        mAnmTexSrt.remove();
        mAnmVis.remove();
        removeBg();
        dFtrm_c::sRecBank.release(&mRecSlot.mIndex);
        dFTR::getList()->remove(this);
        return SUCCEEDED;
    }
    return NOT_READY;
}

// 0x36A4
void daFTR_c::deleteReady() {
    mFlags.mDeleteReady = 1;
}

// 0x36B4
void daFTR_c::executeChildren() {
    if (!getLayer()) {
        int handle = dFTR::getList()->find(this);
        for (u32 i = 0; i < dFTR::list_c::MAX; i++) {
            if (i != handle) {
                daFTR_c *ftr = dFTR::getList()->get(i);
                if (ftr != NULL && ftr->getLayer() == 1 && !ftr->mFlags.mDeleteReady &&
                    ftr->mFlags.mCreated && handle == ftr->mParent) {
                    ftr->execute();
                }
            }
        }
    }
}

// 0x3790
void daFTR_c::speak() {
    flags_c flags = mFlags;
    if (flags.mSpeak || flags.mSpeakDemo) {
        if (flags.mSpeakDemo) {
            mFlags.mSpoken = 1;
        }
        clearDemoFlag8();
        dDemo::requestSpeak(this, NULL);
        mFlags.mSpeak = 0;
        mFlags.mSpeakDemo = 0;
    }
}

// 0x3800
void daFTR_c::setDpdBox() {
    int size = getFtrSize();
    f32 sizeZ;
    if (size == dItem::FTR_SIZE_2x2) {
        sizeZ = 64.0f;
    } else {
        sizeZ = 32.0f;
    }
    f32 sizeX;
    if (size == dItem::FTR_SIZE_1x1) {
        sizeX = 32.0f;
    } else {
        sizeX = 64.0f;
    }
    mDpdBox.set(mCenter, sizeX, sizeZ, dFtrm_c::getDpdHeight(), mAngle.y, dObjc::HIT_FTR_ROOM, this, 0);
    mDpdBox.regist();
}

// 0x38B8
int daFTR_c::getWallSlot() {
    int x;
    int z;
    if (getUnit(&x, &z, NULL, mAng(0))) {
        for (u32 i = 0; i < 9; i++) {
            int wallX;
            int wallZ;
            if (dFtrm_c::getWallUnit(&wallX, &wallZ, i)) {
                if (getFtrSize() == dItem::FTR_SIZE_1x2 && wallX == x && wallZ == z - 1) {
                    return i;
                }
                if (wallX == x && wallZ == z) {
                    return i;
                }
            }
        }
    }
    return 0;
}

// 0x39A8
void daFTR_c::executeHook() {}

// 0x39AC
BOOL daFTR_c::initFtr() {
    if (!mFlags.mInited) {
        clearUnit();
        mMgrItem = dFtrm_c::getCurrent()->mItem;
        mParent = -1;
        mInfo.mValue = mParam;
        dItem::Item item = getItem();
        mpBITM = getItemBITM(item);
        dFTR::getList()->add(this);
        mAllocator.attach(getHeap(), 0x20);
        initPos();
        attachParent();
        initSwitch();
        putUnits(getKind() == FTR_KIND_APPEAR, NULL, mAng(0), FALSE);
        addBg();
        u8 sfx = mpBITM->m_ftrSfx;
        mEnabled = TRUE;
        if (dScene::isCurrentSceneAttr(SCENE_ATTR_NPC_ROOM) && !isKindAvailable(sfx)) {
            mEnabled = FALSE;
        }
        mFlags.mInited = 1;
    }
    return TRUE;
}

// 0x3AF4
BOOL daFTR_c::loadModel() {
    bool ok = true;
    EGG::FrmHeap *heap = getHeap();
    dItem::Item item = getItem();
    if (getDemoID() == 0) {
        u16 id = makeDemoID(NULL, mAng(0));
        setDemoID(id);
    }
    if (mpBITM != NULL && mpBITM->m_hasRes) {
        ok &= mRes.load(item, heap);
    }
    ok &= loadRes();
    ok &= createRes();
    if (!mFlags.mSound) {
        setSound(FALSE);
        mFlags.mSound = 1;
    }
    executeState();
    calcBg();
    initShadow();
    return ok;
}

// 0x3C44
nw4r::g3d::ResMdl daFTR_c::getResMdl() {
    nw4r::g3d::ResFile res(mRes.getData());
    if (res.IsValid() && res.GetResMdlNumEntries() >= 1) {
        nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
        if (mdl.IsValid()) {
            return mdl;
        }
    }
    return nw4r::g3d::ResMdl(NULL);
}

// 0x3C9C
BOOL daFTR_c::createModel() {
    nw4r::g3d::ResMdl mdl = getResMdl();
    u32 size;
    mMdl.create(mdl, &mAllocator, 0, 1, &size);
    mSwitchNode.init(mdl, TRUE);
    mNodeCallBack.mHourNode = m3d::getNodeID(mdl, "kh_j");
    mNodeCallBack.mMinNode = m3d::getNodeID(mdl, "km_j");
    mNodeCallBack.mFtrFunc = getFtrFunc();
    mMdl.setCallback(&mNodeCallBack);
    initModel();
    initLamp();
    if (mMdl.getScn() != NULL && isSyncDisabled()) {
        int lightObj = dBookLight_c::sLightObjIdx;
        mMdl.setLightSetIdxAll(1);
        mMdl.setEnvMapRefAll(1, lightObj);
    }
    setDemoFlag8();
    return TRUE;
}

// 0x3DDC
void daFTR_c::initPos() {
    static const initState_c l_initState[] = {
        {STATE_WAIT, TRUE},
        {STATE_APPEAR, FALSE},
        {STATE_HIDDEN, FALSE},
        {STATE_HIDDEN, FALSE},
    };
    int kind = getKind();
    int state = l_initState[kind].mState;
    mScale.x = mScale.y = mScale.z = mBgScale = 1.0f;
    mAngle.y = mInfo.mRot << 14;
    mFlags.mDraw = 0;
    if (l_initState[kind].mDraw) {
        mFlags.mDraw = 1;
    }
    dFdBase_c::getUnitCenterPos(&mPos, mInfo.mX, mInfo.mZ);
    if (!getLayer()) {
        if (dBGCF::getAttr(mInfo.mX, mInfo.mZ) == 0x67) {
            mPos.y = 0.0f;
        } else {
            mPos.y = dBGCF::getUnitBaseY(&mPos);
        }
    } else {
        mPos.y = dFTR::getGroundY(&mPos);
    }
    adjustPos();
    setMtx(TRUE);
    if (isGyroid() && kind == FTR_KIND_PLACED) {
        u8 scene = dScene::getCurrentScene();
        int x = worldToUnit(mPos.x);
        int z = worldToUnit(mPos.z);
        mSwitchParam = getRoomGyroid(x, z, scene);
    }
    sendEvent(state);
}

// 0x3F74
void daFTR_c::onHit(const dBGCF::wall_c *wall, int ownerId, f32 dist) {
    dActor_c *player = dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
    dActor_c *owner = (dActor_c *)ownerId;
    if (owner != NULL && owner == player && mState == STATE_WAIT) {
        s16 wallAngle = cM::atan2s(wall->mNormal.x, wall->mNormal.z);
        int pushAngle = wallAngle + 0x8000;
        int diff = sLib::distanceAngle(pushAngle, owner->getAngle().y.mAngle);
        s16 maxDiff = dFtrm_c::getPushAngle();
        mVec3_c last = owner->getLastPos();
        dBGC::vecXZ_c from(last.x, last.z);
        dBGC::vecXZ_c to(last.x + wall->mNormal.x, last.z + wall->mNormal.z);
        dBGC::vecXZ_c cross;
        if (diff < maxDiff && wall->cross(&cross, from, to)) {
            f32 margin = dFtrm_c::getPushDist();
            f32 ofs = EGG::Math<f32>::sqrt(wall->mStart.getSquareDistance(cross));
            f32 len = EGG::Math<f32>::sqrt(wall->mStart.getSquareDistance(wall->mEnd));
            if (ofs >= margin && ofs <= len - margin && wall->checkInside(cross)) {
                dBGC::vecXZ_c center = wall->getCenter();
                mVec3_c centerPos(center.x + dist * wall->mNormal.x, 0.0f, center.z + dist * wall->mNormal.z);
                mVec3_c pushPos = centerPos;
                if (len > 48.0f) {
                    int edge = 16;
                    if (ofs < edge) {
                        dBGC::vecXZ_c dir = wall->mEnd - wall->mStart;
                        dir.normalize();
                        f32 scaledX, scaledZ, x, z;
                        scaledX = dir.x * edge;
                        x = scaledX + wall->mStart.x;
                        scaledZ = dir.z * edge;
                        z = scaledZ + wall->mStart.z;
                        cross.x = x;
                        cross.z = z;
                    } else {
                        edge = 48;
                        if (ofs > edge) {
                            dBGC::vecXZ_c dir = wall->mEnd - wall->mStart;
                            dir.normalize();
                            f32 scaledX, scaledZ, x, z;
                            scaledX = dir.x * edge;
                            x = scaledX + wall->mStart.x;
                            scaledZ = dir.z * edge;
                            z = scaledZ + wall->mStart.z;
                            cross.x = x;
                            cross.z = z;
                        }
                    }
                    pushPos.set(cross.x + dist * wall->mNormal.x, 0.0f, cross.z + dist * wall->mNormal.z);
                }
                pushPlayer(owner);
                int handle = dFTR::getList()->find(this);
                s16 angle = wallAngle - mAngle.y;
                if (getLayer() == 1) {
                    mVec3_c pos;
                    getCenter(&pos);
                    daFTR_c *base = dFTR::getUnitMap()->get(&pos, 0);
                    if (base != NULL) {
                        angle = wallAngle - base->mAngle.y - getParentAngle();
                    }
                }
                u8 side = dFtrm_c::getDir(angle);
                dFTR::getHoldList()->set(handle, dist, owner->getLastPos(), owner->getPos(), centerPos, pushPos, pushAngle,
                                         side);
            }
        }
    }
}

// 0x43A8
void daFTR_c::pushPlayer(dActor_c *player) {
    if (getFtrSize() == dItem::FTR_SIZE_1x2) {
        static mVec3_c l_pushOfs[2] = {mVec3_c(0.0f, 0.0f, 0.0f), mVec3_c(-32.0f, 0.0f, 0.0f)};
        static mVec3_c l_side[2] = {mVec3_c(0.0f, 0.0f, 0.0f), mVec3_c(32.0f, 0.0f, 0.0f)};
        static mVec3_c l_shift(32.0f, 0.0f, 0.0f);
        mMtx_c rot;
        rot.YrotS(mAngle.y);
        mVec3_c shift;
        PSMTXMultVec(rot, l_shift, shift);
        mVec3_c side;
        multMtx(&side, &l_side[0]);
        mVec3_c diff0 = side - player->getPos();
        f32 dist0 = EGG::Math<f32>::sqrt(diff0.x * diff0.x + diff0.z * diff0.z);
        multMtx(&side, &l_side[1]);
        mVec3_c diff1 = side - player->getPos();
        f32 dist1 = EGG::Math<f32>::sqrt(diff1.x * diff1.x + diff1.z * diff1.z);
        if (dist0 < dist1) {
            if (mPushOfs != l_pushOfs[0]) {
                mPushOfs = l_pushOfs[0];
                mPos -= shift;
            }
        } else {
            if (mPushOfs != l_pushOfs[1]) {
                mPushOfs = l_pushOfs[1];
                mPos += shift;
            }
        }
    }
}

// 0x46CC
void daFTR_c::addBg() {
    if (!isSyncDisabled() && !canWalkOn()) {
        mVec3_c scale(mBgScale, mBgScale, mBgScale);
        mVec3_c center;
        getCenter(&center);
        mVec3_c size;
        getBgSize(&size);
        dBGCF::getMvbgList()->add(size.x, size.z, 32.0f, this, center, mAngle.y, &scale);
    }
}

// 0x4798
void daFTR_c::calcBg() {
    sLib::chase(&mBgScale, 1.0f, 1.0f / 60.0f);
    if (!isSyncDisabled() && !canWalkOn() && mActive) {
        mVec3_c scale(mBgScale, mBgScale, mBgScale);
        mVec3_c center;
        getCenter(&center);
        dBGCF::getMvbgList()->calc(this, center, mAngle.y, scale);
    }
}

// 0x4870
void daFTR_c::removeBg() {
    if (!isSyncDisabled() && !canWalkOn()) {
        dBGCF::getMvbgList()->remove(this);
    }
}

// 0x48E4
void daFTR_c::calcMtx(mMtx_c *mtx, const mVec3_c *ofs, mAng angle, BOOL target) {
    if (mParent != -1) {
        daFTR_c *parent = dFTR::getList()->get(mParent);
        mMtx_c parentMtx;
        parent->calcMtx(&parentMtx, ofs, angle, FALSE);
        mMtx_c trans;
        const mVec3_c *ofsPos = getParentPos();
        trans.trans(ofsPos->x, ofsPos->y, ofsPos->z);
        PSMTXConcat(parentMtx, trans, parentMtx);
        parentMtx.YrotM(getParentAngle());
        *mtx = parentMtx;
    } else {
        mVec3_c pos = mPos;
        if (ofs != NULL) {
            pos += *ofs;
        }
        mAng rot(mAngle.y.mAngle + angle.mAngle);
        if (target) {
            switch (mState) {
            case STATE_PUSH:
            case STATE_PULL:
                pos = mTargetPos;
                break;
            case STATE_ROTATE:
                rot = getTargetAngle();
                break;
            }
        }
        mtx->trans(pos.x, pos.y, pos.z);
        mtx->YrotM(rot);
        mMtx_c trans;
        trans.trans(mPushOfs.x, mPushOfs.y, mPushOfs.z);
        PSMTXConcat(*mtx, trans, *mtx);
    }
}

// 0x4AD8
void daFTR_c::setMtx(BOOL noMdl) {
    if (!isSyncDisabled()) {
        mMtx_c mtx;
        calcMtx(&mtx, NULL, mAng(0), FALSE);
        mMtx = mtx;
        if (getFtrSize() == dItem::FTR_SIZE_1x2) {
            mMtx_c trans;
            trans.trans(16.0f, 0.0f, 0.0f);
            PSMTXConcat(mtx, trans, mtx);
        }
        if (!noMdl) {
            mMdl.setLocalMtx(&mtx);
        }
    }
}

static mVec3_c l_corner1x1[4] = {
    mVec3_c(-16.0f, 0.0f, -16.0f),
    mVec3_c(-16.0f, 0.0f, 16.0f),
    mVec3_c(16.0f, 0.0f, 16.0f),
    mVec3_c(16.0f, 0.0f, -16.0f),
};
static mVec3_c l_corner2x1[4] = {
    mVec3_c(-16.0f, 0.0f, -16.0f),
    mVec3_c(-16.0f, 0.0f, 16.0f),
    mVec3_c(48.0f, 0.0f, 16.0f),
    mVec3_c(48.0f, 0.0f, -16.0f),
};
static mVec3_c l_corner2x2[4] = {
    mVec3_c(-32.0f, 0.0f, -32.0f),
    mVec3_c(-32.0f, 0.0f, 32.0f),
    mVec3_c(32.0f, 0.0f, 32.0f),
    mVec3_c(32.0f, 0.0f, -32.0f),
};
static mVec3_c *l_corners[] = {l_corner1x1, l_corner2x1, l_corner2x2};
static mVec3_c l_bgSize[3] = {
    mVec3_c(32.0f, 32.0f, 32.0f),
    mVec3_c(64.0f, 32.0f, 32.0f),
    mVec3_c(64.0f, 32.0f, 64.0f),
};

// 0x4BF4
void daFTR_c::getBgSize(mVec3_c *size) {
    *size = l_bgSize[getFtrSize() & 3];
}

// 0x4C48
void daFTR_c::multMtx(mVec3_c *out, const mVec3_c *in) {
    PSMTXMultVec(mMtx, *in, *out);
}

// 0x4C5C
void daFTR_c::getCorner(mVec3_c *out, u32 i) {
    int size = getFtrSize();
    multMtx(out, &l_corners[size & 3][i & 3]);
}

// 0x4CCC
void daFTR_c::getCenter(mVec3_c *out) {
    out->set(0.0f, 0.0f, 0.0f);
    for (u32 i = 0; i < 4; i++) {
        mVec3_c corner;
        getCorner(&corner, i);
        *out += corner;
    }
    *out /= 4.0f;
}

const f32 daFTR_c::sFreeRadius = 8.0f;

// 0x4D98
void daFTR_c::clearUnit() {
    mUnitX = mUnitZ = -1;
}

// 0x4DA8
BOOL daFTR_c::setUnit(const mVec3_c *ofs, mAng angle) {
    int x;
    int z;
    if (getUnit(&x, &z, ofs, angle)) {
        mUnitX = x;
        mUnitZ = z;
        return TRUE;
    }
    return FALSE;
}

// 0x4E1C
BOOL daFTR_c::getUnit(int *x, int *z, const mVec3_c *ofs, mAng angle) {
    if (mUnitX != -1 && mUnitZ != -1) {
        *x = mUnitX;
        *z = mUnitZ;
        return TRUE;
    }
    dFTR::unitList_c units;
    getUnits(&units, ofs, angle, NULL);
    dFdBase_c *fd;
    int layer = getLayer();
    fd = dField::getInfo(0);
    for (u32 i = 0; i < units.mNum; i++) {
        dItem::Item *item = fd->getItem(units.get(i)->x, units.get(i)->z, layer);
        if (item != NULL) {
            const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
            if (bitm != NULL && bitm->getFtrFunc() != dItem::FTR_FUNC_NONE) {
                *x = units.get(i)->x;
                *z = units.get(i)->z;
                return TRUE;
            }
        }
    }
    for (u32 i = 0; i < units.mNum; i++) {
        fd->getItem(units.get(i)->x, units.get(i)->z, layer);
    }
    getFtrSize();
    return FALSE;
}

// 0x4FDC
void daFTR_c::getUnits(dFTR::unitList_c *list, const mVec3_c *ofs, mAng angle, mMtx_c *mtx) {
    mVec3_c pos = mPos;
    if (list != NULL) {
        list->mNum = 0;
    }
    if (ofs != NULL) {
        pos += *ofs;
    }
    static mVec3_c l_center(0.0f, 0.0f, 0.0f);
    static mVec3_c l_center2x1[2] = {mVec3_c(0.0f, 0.0f, 0.0f), mVec3_c(32.0f, 0.0f, 0.0f)};
    int size = getFtrSize();
    mMtx_c localMtx;
    if (mtx == NULL) {
        mtx = &localMtx;
    }
    calcMtx(mtx, ofs, angle, TRUE);
    mVec3_c center;
    switch (size) {
    case dItem::FTR_SIZE_2x2: {
        pos.x -= 16.0f;
        pos.z -= 16.0f;
        int x = worldToUnit(pos.x);
        int z = worldToUnit(pos.z);
        list->add(x, z);
        list->add(x + 1, z);
        list->add(x, z + 1);
        list->add(x + 1, z + 1);
        break;
    }
    case dItem::FTR_SIZE_1x2: {
        static const u32 unitCount = 2;
        for (u32 i = 0; i < unitCount; i++) {
            PSMTXMultVec(*mtx, l_center2x1[i], center);
            int x = worldToUnit(center.x);
            int z = worldToUnit(center.z);
            list->add(x, z);
        }
        break;
    }
    default: {
        static const u32 unitCount = 1;
        for (u32 i = 0; i < unitCount; i++) {
            PSMTXMultVec(*mtx, l_center, center);
            int x = worldToUnit(center.x);
            int z = worldToUnit(center.z);
            list->add(x, z);
        }
        break;
    }
    }
}

// 0x5290
dItem::Item daFTR_c::getDirItem(const mAng &angle) const {
    dItem::Item item((u16)mInfo.mItem);
    return item.makeDir(dFtrm_c::getDir(angle));
}

// 0x52E4
void daFTR_c::putUnits(bool send, const mVec3_c *ofs, mAng angle, BOOL setDemo) {
    if (!isSyncDisabled()) {
        int layer = getLayer();
        dFTR::unitList_c units;
        mMtx_c mtx;
        getUnits(&units, ofs, angle, &mtx);
        dItem::Item item;
        if (layer == 0) {
            item = getDirItem(mAng(mAngle.y + angle));
        } else {
            static mVec3_c l_front(0.0f, 0.0f, 0.0f);
            static mVec3_c l_back(0.0f, 0.0f, 1.0f);
            mVec3_c front;
            PSMTXMultVec(mtx, l_front, front);
            mVec3_c back;
            PSMTXMultVec(mtx, l_back, back);
            item = getDirItem(mAng(cM::atan2s(back.x - front.x, back.z - front.z)));
        }
        BOOL gyroid = isGyroid();
        int param = getSwitchParam();
        for (u32 i = 0; i < units.mNum; i++) {
            int x = units.get(i)->x;
            int z = units.get(i)->z;
            if (i == 0) {
                dFTR::putFtr(dScene::getCurrentScene(), x, z, layer,
                             getSwitch(), gyroid, param, dItem::Item(item),
                             send);
            }
            dFTR::getUnitMap()->set(this, x, z, layer);
        }
        if (setDemo) {
            u16 id = makeDemoID(ofs, angle);
            if (id != getDemoID()) {
                setDemoID(id);
            }
        }
    }
}

// 0x5598
void daFTR_c::removeUnits(BOOL send, BOOL upper) {
    if (!isSyncDisabled()) {
        int layer = getLayer();
        dFdBase_c *fd = dField::getInfo(0);
        if (fd != NULL) {
            dFTR::unitList_c units;
            mMtx_c mtx;
            getUnits(&units, NULL, mAng(0), &mtx);
            for (u32 i = 0; i < units.mNum; i++) {
                int x = units.get(i)->x;
                int z = units.get(i)->z;
                dItem::Item *item = fd->getItem(x, z, getLayer());
                if (item != NULL && item->hasFtrFunc()) {
                    dFTR::removeFtr(dScene::getCurrentScene(), x, z,
                                    (u8)getLayer(), getSwitch(),
                                    dItem::Item(*item), upper, send);
                }
                dFTR::unitMap_c *map = dFTR::getUnitMap();
                map->remove(this, x, z, layer);
                if (upper && !layer && map->getHandle(x, z, 1) != -1) {
                    map->remove(this, x, z, 1);
                }
            }
        }
    }
}

// 0x5770
dItem::Item daFTR_c::getPutItem() {
    if (getFtrFunc() == dItem::FTR_FUNC_MY_DESIGN) {
        return dItem::Item();
    }
    return dItem::Item((u16)mInfo.mItem);
}

// 0x57DC
void daFTR_c::moveUnits(const mVec3_c *ofs, mAng angle) {
    dFTR::unitList_c units;
    getUnits(&units, NULL);
    dFTR::ftrList_c tops(units, getLayer());
    removeUnits(TRUE, TRUE);
    putUnits(true, ofs, angle, TRUE);
    for (u32 i = 0; i < tops.mNum; i++) {
        daFTR_c *top = tops.get(i);
        if (top != NULL) {
            top->putUnits(true, ofs, angle, TRUE);
            top->setUnit(ofs, angle);
        }
    }
    setUnit(ofs, angle);
}

// 0x5908
BOOL daFTR_c::isNear(const mVec3_c *pos, const mVec3_c *other, f32 radius, f32 otherRadius) {
    mVec3_c diff = *other - *pos;
    return EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z) < radius + otherRadius;
}

// 0x5980
BOOL daFTR_c::isFree(const mVec3_c *pos, BOOL npc, f32 playerRadius, f32 npcRadius, f32 playerSize, f32 npcSize) {
    dActor_c *me = dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (me != NULL) {
        for (u32 i = 0; i < dPlayerMgr::PLAYER_INDEX_COUNT; i++) {
            dActor_c *player = dPlayerMgr::getPlayerActor(i);
            if (player != NULL && me != player && isNear(pos, player->getPosP(), playerRadius, playerSize)) {
                return FALSE;
            }
        }
        if (npc) {
            for (int i = 0; i < dNpcList::countNml(); i++) {
                dActor_c *actor = dNpcList::getNml(i);
                if (actor != NULL && isNear(pos, actor->getPosP(), npcRadius, npcSize)) {
                    return FALSE;
                }
            }
            if (getRoomFromScene(dScene::getCurrentScene()) != HOME_SCENE_LOFT) {
                for (int i = 0; i < dNpcList::countSp(); i++) {
                    dActor_c *actor = dNpcList::getSp(i);
                    if (actor != NULL && isNear(pos, actor->getPosP(), npcRadius, npcSize)) {
                        return FALSE;
                    }
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}

// 0x5B10
BOOL daFTR_c::hasTopItem() {
    dFdBase_c *fd = dField::getInfo(0);
    if (fd != NULL) {
        dFTR::unitList_c units;
        getUnits(&units, NULL);
        if (!getLayer()) {
            if (mpBITM != NULL && mpBITM->getFtrTable() == dItem::FTR_TABLE_ITEM) {
                for (u32 i = 0; i < units.mNum; i++) {
                    dFTR::unitList_c::unit_c *unit = units.get(i);
                    dItem::Item *item = fd->getItem(unit->x, unit->z, 1);
                    if (item != NULL && item->getId() != dItem::ITEM_ID_NONE) {
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}

// 0x5C20
BOOL daFTR_c::canRotate(const mAng &angle) {
    return tryMove(NULL, angle);
}

// 0x5C5C
BOOL daFTR_c::canMove(const mVec3_c *dir) {
    return tryMove(dir, mAng(0));
}

// 0x5C94
BOOL daFTR_c::tryMove(const mVec3_c *dir, mAng angle) {
    if (mState != STATE_WAIT) {
        return FALSE;
    }
    dScene::getCurrentScene();
    if (mSwitchWait) {
        return FALSE;
    }
    if (!dScene::isCurrentSceneMyHouse()) {
        return FALSE;
    }
    dFdBase_c *fd = dField::getInfo(0);
    dActor_c *player = dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (getLayer() == 1) {
        return FALSE;
    }
    if (fd == NULL || player == NULL) {
        return FALSE;
    }
    if (!mTopItems.canGet(this)) {
        return FALSE;
    }
    mVec3_c playerPos = player->getPos();
    int playerX = worldToUnit(playerPos.x);
    int playerZ = worldToUnit(playerPos.z);
    mVec3_c nextPos = playerPos;
    if (dir != NULL) {
        nextPos += *dir;
    }
    int nextX = worldToUnit(nextPos.x);
    int nextZ = worldToUnit(nextPos.z);
    daFTR_c *next = dFTR::getUnitMap()->get(nextX, nextZ, 0);
    dFTR::unitList_c units;
    getUnits(&units, NULL);
    for (u32 i = 0; i < units.mNum; i++) {
        int x = units.get(i)->x;
        int z = units.get(i)->z;
        dItem::Item *item = fd->getItem(x, z, 1);
        if (item != NULL && item->getId() != dItem::ITEM_ID_NONE && !dItem::isRealItemId(item->getId()) && !item->isUsable()) {
            return FALSE;
        }
        daFTR_c *top = dFTR::getUnitMap()->get(x, z, 1);
        if (top != NULL && top->mState != STATE_WAIT) {
            return FALSE;
        }
    }
    removeUnits(FALSE, FALSE);
    dFTR::unitList_c dest;
    getUnits(&dest, dir, angle, NULL);
    dFTR::unitList_c half;
    getUnits(&half, dir, mAng(angle.mAngle >> 1), NULL);
    for (u32 i = 0; i < dest.mNum; i++) {
        int x = dest.get(i)->x;
        int z = dest.get(i)->z;
        dItem::Item *item = fd->getItem(x, z, 0);
        if (dir == NULL && playerX == x && playerZ == z) {
            putUnits(false);
            return FALSE;
        }
        if (dir == NULL) {
            dItem::Item *halfItem = fd->getItem(half.get(i)->x, half.get(i)->z, 0);
            if (halfItem != NULL && halfItem->getId() != dItem::ITEM_ID_NONE) {
                putUnits(false);
                return FALSE;
            }
        }
        if (item != NULL && item->getId() != dItem::ITEM_ID_NONE) {
            putUnits(false);
            return FALSE;
        }
        if (!dBGCF::canPutItem(x, z)) {
            putUnits(false);
            return FALSE;
        }
        if (angle.mAngle == 0) {
            if ((next != NULL && !next->canWalkOn() && next != this) || dBGCF::getOpenNext(&nextPos) != -1 ||
                dBGCF::getGroundY(&nextPos, 0) != dBGCF::getGroundY(&playerPos, 0) ||
                dFTR::getGroundY(&nextPos) != dBGCF::getGroundY(&nextPos, 0)) {
                putUnits(false);
                return FALSE;
            }
        }
    }
    putUnits(false);
    return TRUE;
}

// 0x61DC
u16 daFTR_c::makeDemoID(const mVec3_c *ofs, mAng angle) {
    int x;
    int z;
    int room;
    int home = dHomeList_c::getHomeFromScene(&room, dScene::getCurrentScene());
    if (isSyncDisabled() || (home != -1 && room == HOME_SCENE_LOFT)) {
        demoIdA_u id;
        id.mHandle = dFTR::getList()->find(this);
        id.mScene = dScene::getCurrentScene();
        return id.mValue;
    }
    if (getUnit(&x, &z, ofs, angle)) {
        demoIdB_u id;
        id.mX = x;
        id.mZ = z;
        id.mLayer = getLayer();
        id.mScene = dScene::getCurrentScene();
        return id.mValue;
    }
    return 0;
}

// 0x6310
BOOL daFTR_c::getParentOfs(int *handle, mVec3_c *ofs, mAng *angle, int x, int z, mAng ownAngle) {
    daFTR_c *parent = dFTR::getUnitMap()->get(x, z, 0);
    int parentHandle = dFTR::getList()->find(parent);
    if (parent != NULL && parentHandle != -1) {
        mVec3_c pos;
        dFdBase_c::getUnitCenterPos(&pos, x, z);
        pos.y = parent->getTopY();
        mMtx_c mtx;
        parent->calcMtx(&mtx, NULL, mAng(0), FALSE);
        PSMTXInverse(mtx, mtx);
        mVec3_c local;
        PSMTXMultVec(mtx, pos, local);
        *handle = parentHandle;
        *ofs = local;
        *angle = ownAngle - parent->mAngle.y;
        return TRUE;
    }
    return FALSE;
}

// 0x6428
mVec3_c *daFTR_c::getParentPos() {
    return &mParentOfs;
}

// 0x6430
mAng daFTR_c::getParentAngle() {
    return mParentAngle;
}

// 0x643C
BOOL daFTR_c::attachParent() {
    return attachParent(mInfo.mX, mInfo.mZ, mAngle.y);
}

// 0x6474
BOOL daFTR_c::attachParent(int x, int z, mAng angle) {
    if (getLayer() == 1) {
        int handle;
        mVec3_c ofs;
        mAng ofsAngle;
        if (getParentOfs(&handle, &ofs, &ofsAngle, x, z, angle)) {
            return setParent(handle, &ofs, ofsAngle);
        }
    }
    return FALSE;
}

// 0x6528
BOOL daFTR_c::setParent(int handle, const mVec3_c *ofs, mAng angle) {
    if (getLayer() == 1 && handle >= 0 && handle < dFTR::list_c::MAX) {
        mParent = handle;
        mParentOfs = *ofs;
        mParentAngle = angle;
        return TRUE;
    }
    return FALSE;
}

// 0x65CC
void daFTR_c::topItem_c::init() {
    mItem = dItem::Item();
    mPos.set(0.0f, 0.0f, 0.0f);
}

// 0x65F0
BOOL daFTR_c::topItem_c::set(dItem::Item item, const mVec3_c &pos) {
    if (isEmpty()) {
        mItem = item;
        mPos = pos;
        return TRUE;
    }
    return FALSE;
}

// 0x667C
void daFTR_c::topItem_c::draw(daFTR_c *ftr) {
    if (!isEmpty()) {
        mMtx_c mtx;
        ftr->mMdl.getLocalMtx(&mtx);
        f32 ofsX = ftr->getFtrSize() == dItem::FTR_SIZE_1x2 ? -16.0f : 0.0f;
        mMtx_c trans;
        trans.trans(ofsX + mPos.x, mPos.y, mPos.z);
        PSMTXConcat(mtx, trans, mtx);
        mVec3_c zero(0.0f, 0.0f, 0.0f);
        mVec3_c pos;
        PSMTXMultVec(mtx, zero, pos);
        dFgDraw::add(getItem().getId(), pos, mVec3_c(1.0f, 1.0f, 1.0f), 0, 0, mAng3_c(0, 0, 0));
    }
}

// 0x6798
void daFTR_c::topItemList_c::init() {
    for (u32 i = 0; i < 4; i++) {
        get(i)->init();
    }
}

// 0x67E8
void daFTR_c::topItemList_c::draw(daFTR_c *ftr) {
    for (u32 i = 0; i < 4; i++) {
        get(i)->draw(ftr);
    }
}

// 0x6848
BOOL daFTR_c::topItemList_c::add(dItem::Item item, const mVec3_c &pos) {
    for (u32 i = 0; i < 4; i++) {
        topItem_c *slot = get(i);
        if (slot->isEmpty()) {
            slot->set(item, pos);
            return TRUE;
        }
    }
    return FALSE;
}

// 0x68E0
BOOL daFTR_c::topItemList_c::canGet(daFTR_c *ftr) {
    dFTR::unitList_c units;
    ftr->getUnits(&units, NULL);
    dFdBase_c *fd = dField::getInfo(0);
    if (fd != NULL) {
        for (u32 i = 0; i < units.mNum; i++) {
            int x = units.get(i)->x;
            int z = units.get(i)->z;
            dItem::Item *item = fd->getItem(x, z, 1);
            if (item != NULL && !item->isUsable() && !dItem::isRealItemId(item->getId()) &&
                item->getId() != dItem::ITEM_ID_NONE) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

// 0x69E8
BOOL daFTR_c::topItemList_c::pickUp(daFTR_c *ftr) {
    dFTR::unitList_c units;
    bool ok = true;
    ftr->getUnits(&units, NULL);
    dFdBase_c *fd = dField::getInfo(0);
    if (fd != NULL && canGet(ftr)) {
        init();
        mMtx_c mtx;
        ftr->calcMtx(&mtx, NULL, mAng(0), FALSE);
        PSMTXInverse(mtx, mtx);
        for (u32 i = 0; i < units.mNum; i++) {
            int x = units.get(i)->x;
            int z = units.get(i)->z;
            dItem::Item *item = fd->getItem(x, z, 1);
            if (item != NULL && item->hasNoFtrFunc()) {
                mVec3_c pos;
                dFdBase_c::getUnitCenterPos(&pos, x, z);
                pos.y = ftr->getTopY();
                mVec3_c local;
                PSMTXMultVec(mtx, pos, local);
                ok &= add(*item, local);
            }
        }
        if (!ok) {
            init();
        }
        return ok;
    }
    return FALSE;
}

// 0x6B7C
void daFTR_c::topItemList_c::put(daFTR_c *ftr) {
    if (dField::getInfo(0) != NULL) {
        for (u32 i = 0; i < 4; i++) {
            topItem_c *slot = get(i);
            if (!slot->isEmpty()) {
                mVec3_c zero(0.0f, 0.0f, 0.0f);
                f32 ofsX = ftr->getFtrSize() == dItem::FTR_SIZE_1x2 ? -16.0f : 0.0f;
                mMtx_c mtx;
                ftr->mMdl.getLocalMtx(&mtx);
                mMtx_c trans;
                trans.trans(ofsX + slot->mPos.x, slot->mPos.y, slot->mPos.z);
                PSMTXConcat(mtx, trans, mtx);
                mVec3_c pos;
                PSMTXMultVec(mtx, zero, pos);
                int x = worldToUnit(pos.x);
                int z = worldToUnit(pos.z);
                dItem::Item item = slot->getItem();
                dFTR::setUnitItem(dScene::getCurrentScene(), x, z, &item, TRUE);
            }
        }
        init();
    }
}

// 0x6CEC
BOOL dFTR::unitList_c::add(int x, int z) {
    if (mNum < 4) {
        mUnit[mNum].x = x;
        mUnit[mNum].z = z;
        mNum++;
        return TRUE;
    }
    return FALSE;
}

// 0x6D30
dFTR::unitList_c::unit_c *dFTR::unitList_c::get(u32 i) {
    return &mUnit[i & 3];
}

// 0x6D40
BOOL daFTR_c::setSwitch(int sw, int param) {
    mSwitchParam = param;
    mSwitchWait = FALSE;
    return FALSE;
}

// 0x6D54
int daFTR_c::getSwitchBit(int sw) {
    return 0;
}

// 0x6D5C
void daFTR_c::nodeCallBack_c::timingA(ulong nodeId, nw4r::g3d::ChrAnmResult *anmRes, nw4r::g3d::ResMdl resMdl) {
    dTime_c now = *dTime_c::getCurrent();
    int min = now.min;
    int hour = now.hour;
    if (mFtrFunc == dItem::FTR_FUNC_WARDROBE || mFtrFunc == dItem::FTR_FUNC_ARMOIRE || mFtrFunc == dItem::FTR_FUNC_GYROID) {
        nw4r::g3d::ResNode node = resMdl.GetResNode(nodeId);
        mMtx_c mtx;
        anmRes->GetRotTrans(mtx);
        mVec3_c trans(node.ref().translate);
        mtx.m[0][3] = trans.x;
        mtx.m[1][3] = trans.y;
        mtx.m[2][3] = trans.z;
        anmRes->SetRotTrans(mtx);
    }
    if (nodeId == mHourNode) {
        int mins = hour % 12;
        mins *= 60;
        mins += min;
        mMtx_c mtx;
        anmRes->GetRotTrans(mtx);
        mtx.ZrotM(mAng(-(s16)((mins << 16) / 720)));
        anmRes->SetRotTrans(mtx);
    } else if (nodeId == mMinNode) {
        mMtx_c mtx;
        anmRes->GetRotTrans(mtx);
        mtx.ZrotM(mAng(-(s16)((min << 16) / 60)));
        anmRes->SetRotTrans(mtx);
    }
}

// 0x6F38
void dFTR::ftrList_c::clear() {
    mNum = 0;
    mFtr[0] = NULL;
    mFtr[1] = NULL;
    mFtr[2] = NULL;
    mFtr[3] = NULL;
}

// 0x6F54
void dFTR::ftrList_c::set(unitList_c &units, BOOL layer) {
    clear();
    if (!layer) {
        for (u32 i = 0; i < units.mNum; i++) {
            daFTR_c *top = getUnitMap()->get(units.get(i)->x, units.get(i)->z, 1);
            if (top != NULL) {
                add(top);
            }
        }
    }
}

// 0x6FF8
dFTR::ftrList_c::ftrList_c(unitList_c &units, BOOL layer) {
    set(units, layer);
}

// 0x7028
BOOL dFTR::ftrList_c::add(daFTR_c *ftr) {
    if (ftr == NULL) {
        return FALSE;
    }
    if (mNum >= 4) {
        return FALSE;
    }
    for (u32 i = 0; i < mNum; i++) {
        if (mFtr[i] == ftr) {
            return FALSE;
        }
    }
    mFtr[mNum++] = ftr;
    return TRUE;
}

// 0x7098
void daFTR_c::appearEffect(const mVec3_c *pos) {
    dEffect::create("af_itm_appear_SS", pos, NULL, NULL);
}

// 0x70AC
void daFTR_c::setSound(bool on) {
    BOOL seq = FALSE;
    nw4r::g3d::ResFile res(mRes.getData());
    if (res.IsValid()) {
        const void *bfs = res.GetExternal("a.bfs");
        const void *brseq = res.GetExternal("a.brseq");
        if (bfs != NULL || brseq != NULL) {
            seq = TRUE;
            mSndFtr.setupSeq((const SoundObjFurniture::SeqData *)bfs, brseq, on);
        }
    }
    if (!seq) {
        mSndFtr.setup(mpBITM->m_ftrSfx, on);
    }
}

// 0x716C
void daFTR_c::startAppearSe() {
    mSndObj.startSound(SE_kagu_open);
}

// 0x7178
void daFTR_c::startVanishSe() {
    mSndObj.startSound(SE_kagu_close);
}

// 0x7184
BOOL daFTR_c::updateSound() {
    return TRUE;
}

// 0x718C
BOOL daFTR_c::turnOnSound() {
    if (mFlags.mCreated) {
        if (mEnabled) {
            mSndFtr.turnOn();
        }
        return TRUE;
    }
    setSound(TRUE);
    return TRUE;
}

// 0x71DC
BOOL daFTR_c::turnOffSound() {
    if (mFlags.mCreated) {
        if (mEnabled) {
            mSndFtr.turnOff();
        }
        return TRUE;
    }
    return FALSE;
}

// 0x7224
BOOL daFTR_c::startMoveSe() {
    int se = getMoveSe();
    if (se != -1) {
        mSndObj.startSound(se);
        return TRUE;
    }
    return FALSE;
}

// 0x7270
int daFTR_c::getMoveSe() {
    dItem::Item carpet = dBgUtil::getCarpetRaw();
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(carpet);
    if (bitm != NULL) {
        int attr = bitm->m_carpetSfx;
        return dBGCF::getFtrSe(static_cast<u32>(attr) < 0x86 ? static_cast<bgAttr_e>(attr) : BG_ATTR_NONE);
    }
    return -1;
}

// 0x72D4
BOOL daFTR_c::isLampOn(daFTR_c *ftr) {
    if (ftr != NULL) {
        const dItem::BITM *bitm = ftr->getBITM();
        if (bitm != NULL) {
            int lamp = bitm->getFtrLamp();
            BOOL on = FALSE;
            if (lamp != 0 && ftr->mFlags.mLamp && !ftr->isSyncDisabled()) {
                on = TRUE;
            }
            return on;
        }
    }
    return FALSE;
}

// 0x7384
BOOL daFTR_c::isMusicFtr(daFTR_c *ftr) {
    if (ftr != NULL) {
        return ftr->mProfName == fProf::FTR_AUDIO;
    }
    return FALSE;
}

// 0x73A8
BOOL daFTR_c::isMusicOn(daFTR_c *ftr) {
    if (isMusicFtr(ftr)) {
        return ftr->getSwitch();
    }
    return FALSE;
}

// 0x73F8
BOOL daFTR_c::isTvOn() {
    u16 prof = mProfName;
    if (prof == fProf::FTR_TV || prof == fProf::FTR_TV_F) {
        return getSwitch();
    }
    return FALSE;
}

// 0x7428
bool daFTR_c::isSeatFtr(daFTR_c *ftr) {
    BOOL match = ftr->mProfName == fProf::FTR_CHAIR || ftr->mProfName == fProf::FTR_BED;
    return match;
}

// 0x7454
BOOL daFTR_c::isGyroidFunc() {
    return getFtrFunc() == dItem::FTR_FUNC_GYROID;
}

// 0x748C
BOOL daFTR_c::isLit() {
    return isLampOn(this);
}

// 0x7490
BOOL daFTR_c::isMusic() {
    return isMusicFtr(this);
}

// 0x7494
BOOL daFTR_c::isSofa() {
    return (u32)(getFtrFunc() - dItem::FTR_FUNC_SOFA) <= dItem::FTR_FUNC_SOFA_SEAT_ANIM - dItem::FTR_FUNC_SOFA;
}

// 0x74DC
BOOL daFTR_c::isChair() {
    return (u32)(getFtrFunc() - dItem::FTR_FUNC_CHAIR) <= dItem::FTR_FUNC_CHAIR_SEAT_ANIM - dItem::FTR_FUNC_CHAIR;
}

// 0x7524
BOOL daFTR_c::isRightyDesk() {
    return (u32)(getFtrFunc() - dItem::FTR_FUNC_RIGHTY_DESK) <=
           dItem::FTR_FUNC_RIGHTY_DESK_SEAT_ANIM - dItem::FTR_FUNC_RIGHTY_DESK;
}

// 0x756C
BOOL daFTR_c::isLeftyDesk() {
    return (u32)(getFtrFunc() - dItem::FTR_FUNC_LEFTY_DESK) <=
           dItem::FTR_FUNC_LEFTY_DESK_SEAT_ANIM - dItem::FTR_FUNC_LEFTY_DESK;
}

// 0x75B4
bool daFTR_c::isSeat() {
    return isSeatFtr(this);
}

// 0x75B8
BOOL daFTR_c::isGyroid() {
    return isGyroidFunc();
}

// 0x75BC
daFTR_c::lamp_c::lamp_c() : mMat(NULL) {}

// 0x75F4
BOOL daFTR_c::lamp_c::init(nw4r::g3d::ResMdl mdl, const char *matName, BOOL on) {
    if (mdl.IsValid()) {
        nw4r::g3d::ResMat mat = mdl.GetResMat(matName);
        if (mat.IsValid()) {
            light_c::set(on);
            mMat = mat;
            return TRUE;
        }
    }
    return FALSE;
}

// 0x7670
BOOL daFTR_c::lamp_c::set(BOOL on, BOOL fade, BOOL flicker) {
    return light_c::set(on, fade, flicker, 0.1f);
}

// 0x767C
void daFTR_c::lamp_c::update() {
    light_c::update();
    if (mMat.IsValid()) {
        f32 value = mValue;
        u8 alpha = value * 255.0f;
        nw4r::ut::Color color;
        color.r = 0xFF;
        color.g = 0xFF;
        color.b = 0xFF;
        color.a = alpha;
        nw4r::g3d::ResMatTevColor tev = mMat.GetResMatTevColor();
        tev.GXSetTevColor(GX_TEVREG2, color);
        tev.DCStore(false);
    }
}

static const char *l_lampMatName[] = {"mn_m", "tv_m", "wa_m", "wb_m", "fr_m", NULL};

// 0x774C
BOOL daFTR_c::lampList_c::init(nw4r::g3d::ResMdl mdl, BOOL on) {
    mActive = false;
    for (u32 i = 0; i < ARRAY_SIZE(mLamp); i++) {
        mActive = mActive | mLamp[i].init(mdl, l_lampMatName[i], on);
    }
    return mActive;
}

// 0x77E8
BOOL daFTR_c::lampList_c::set(BOOL on, BOOL fade, BOOL flicker) {
    bool changed = false;
    if (mActive) {
        for (lamp_c *lamp = mLamp; lamp < mLamp + ARRAY_SIZE(mLamp); lamp++) {
            changed = changed | lamp->set(on, fade, flicker);
        }
    }
    return changed;
}

// 0x7870
void daFTR_c::lampList_c::update() {
    if (mActive) {
        for (lamp_c *lamp = mLamp; lamp < mLamp + ARRAY_SIZE(mLamp); lamp++) {
            lamp->update();
        }
    }
}

// 0x78C8
f32 daFTR_c::lampList_c::getValue() {
    if (mActive) {
        for (lamp_c *lamp = mLamp; lamp < mLamp + ARRAY_SIZE(mLamp); lamp++) {
            if (lamp->mMat.IsValid()) {
                return lamp->mValue;
            }
        }
    }
    return 1.0f;
}

// 0x791C
void daFTR_c::calcSwitchNode() {
    nw4r::g3d::ResMdl mdl = getResMdl();
    if (mdl.IsValid()) {
        u32 num = mdl.GetResNodeNumEntries();
        for (u32 i = 0; i < num; i++) {
            if (mSwitchNode.isNode(i)) {
                nw4r::g3d::ResNode node = mdl.GetResNode(i);
                BOOL on = mSwitchNode.isOn();
                if (node.IsValid()) {
                    if (on) {
                        node.ptr()->flags |= nw4r::g3d::ResNodeData::FLAG_VISIBLE;
                    } else {
                        node.ptr()->flags &= ~nw4r::g3d::ResNodeData::FLAG_VISIBLE;
                    }
                }
            }
        }
    }
}

static const char *l_switchNodeName[] = {"v", "v1", "v2", "v3"};

// 0x79F0
void daFTR_c::switchNode_c::init(nw4r::g3d::ResMdl mdl, BOOL on) {
    static const u32 nodeCount = ARRAY_SIZE(l_switchNodeName);
    if (mdl.IsValid()) {
        for (u32 i = 0; i < nodeCount; i++) {
            mNode[i] = m3d::getNodeID(mdl, l_switchNodeName[i]);
        }
        set(on);
    }
}

// 0x7A7C
void daFTR_c::switchNode_c::set(BOOL on) {
    mOn = on;
}

// 0x7A84
BOOL daFTR_c::switchNode_c::isNode(u32 nodeId) {
    for (int i = 0; i < ARRAY_SIZE(mNode); i++) {
        if (nodeId == mNode[i]) {
            return TRUE;
        }
    }
    return FALSE;
}

// 0x7AEC
BOOL daFTR_c::switchNode_c::isOn() {
    return mOn;
}

// 0x7AF4
BOOL daFTR_c::setRoomLight(int lamp) {
    if (lamp == 2) {
        dSubLightRoom_c::sLightMode = 0;
        dSubLightRoom_c::sLightOn = TRUE;
        dSubLightRoom_c::sLight.set(TRUE, TRUE, FALSE, 0.1f);
        dFTR::requestRoomFlag(dScene::getCurrentScene(), TRUE);
        return TRUE;
    }
    dSubLightRoom_c::sLightMode = 1;
    dSubLightRoom_c::sLightOn = TRUE;
    dSubLightRoom_c::sLight.set(TRUE, TRUE, TRUE, 0.1f);
    dFTR::requestRoomFlag(dScene::getCurrentScene(), TRUE);
    return TRUE;
}

// 0x7BB0
void daFTR_c::initLamp() {
    mFlags.mLamp = 1;
    if (isLit()) {
        if (dScene::isCurrentSceneAttr(SCENE_ATTR_NPC_ROOM)) {
            int idx = dScene::getSceneAttrIndex(dScene::getCurrentScene(), SCENE_ATTR_NPC_ROOM);
            dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
            if (animal != NULL && (int)animal->mPlace == 1) {
                changeSwitch(TRUE, FALSE);
            } else {
                changeSwitch(FALSE, FALSE);
            }
        }
        mLamp.init(getResMdl(), "lp_m", getSwitch());
        if (dFtrm_c::isLampScene()) {
            u32 num = dFTR::countSwitch(isLampOn, 1);
            if (dScene::isCurrentSceneAttr(SCENE_ATTR_NPC_ROOM) && !dSubLightRoom_c::sLightOn) {
                changeSwitch(FALSE, TRUE);
            }
            if (getSwitch() && num == 1) {
                if (getKind() != FTR_KIND_PLACED) {
                    if (setRoomLight(getFtrLamp())) {
                        dFTR::requestRoomFlag(dScene::getCurrentScene(), TRUE);
                        dFtrm_c::sLampOn = TRUE;
                    }
                } else {
                    dSubLightRoom_c::sLightOn = TRUE;
                    dFtrm_c::sLampOn = TRUE;
                }
            }
        }
    }
}

// 0x7D9C
void daFTR_c::executeLamp() {
    if (isLit() && dFtrm_c::isLampScene()) {
        BOOL flicker = getFtrLamp() == 1;
        if (mLamp.set(getSwitch(), TRUE, flicker)) {
            u32 num = dFTR::countSwitch(isLampOn, 1);
            if (getSwitch()) {
                if (num == 1 && setRoomLight(getFtrLamp())) {
                    dFTR::requestRoomFlag(dScene::getCurrentScene(), TRUE);
                    dFtrm_c::sLampOn = TRUE;
                }
            } else if (num == 0) {
                BOOL mode = dSubLightRoom_c::sLightMode == 1;
                dSubLightRoom_c::sLightOn = FALSE;
                dSubLightRoom_c::sLight.set(FALSE, TRUE, mode, 0.1f);
                dFTR::requestRoomFlag(dScene::getCurrentScene(), FALSE);
                dFtrm_c::sLampOn = FALSE;
            }
        }
    }
    mLamp.update();
}

// 0x7EF8
void daFTR_c::deleteLamp() {
    if (isLit() && dFtrm_c::isLampScene() && getSwitch() && dFTR::countSwitch(isLampOn, 1) == 1) {
        BOOL mode = dSubLightRoom_c::sLightMode == 1;
        dSubLightRoom_c::sLightOn = FALSE;
        dSubLightRoom_c::sLight.set(FALSE, TRUE, mode, 0.1f);
        if (mState == STATE_REMOVE) {
            dFTR::requestRoomFlag(dScene::getCurrentScene(), FALSE);
        }
    }
}

// 0x7FC4
f32 daFTR_c::getCatalogScale() const {
    static const f32 defaultScale = 1.0f;
    u8 scale = defaultScale * 100;
    if (mpBITM != NULL) {
        scale = (s8)mpBITM->m_catalogScale;
    }
    return (int)(u8)scale / 100.0f;
}

// 0x8014
f32 daFTR_c::getHeight() const {
    int height = 0;
    if (mpBITM != NULL) {
        height = (s8)mpBITM->_170;
    }
    return height;
}

// 0x8058
void daFTR_c::adjustPos() {
    if (getFtrSize() == dItem::FTR_SIZE_2x2) {
        mPos.x += 16.0f;
        mPos.z += 16.0f;
    }
}

// 0x80AC
void daFTR_c::onMessageStart(int arg) {
    if (dScene::isCurrentSceneModelRoom() && mMsgItem2.getId() != dItem::ITEM_ID_NONE && mMessageCode == 0x16) {
        mpController->setItemName(1, mMsgItem2);
    }
}

// 0x8114
void daFTR_c::onMessageEnd(int kind) {
    if (dScene::isCurrentSceneModelRoom() && mMsgItem2.getId() != dItem::ITEM_ID_NONE && mMessageCode == 0x1D) {
        mpController->setNextMsgCode(0x16);
    }
}

// 0x8178
mVec3_c *daFTR_c::getDemoPosition() {
    if (dScene::isCurrentSceneModelRoom()) {
        if (mDemoPos.isZero()) {
            if (mCenter.isZero()) {
                return dDemoActor_c::getDemoPosition();
            }
            return &mCenter;
        }
        return &mDemoPos;
    }
    return dDemoActor_c::getDemoPosition();
}

// 0x8240
void daFTR_c::onDemoCmd(int cmd, u8 member) {
    demoCmd(cmd);
}

// 0x8244
BOOL daFTR_c::demoCmd(int cmd) {
    if (dScene::isCurrentSceneModelRoom()) {
        switch (cmd) {
        case DEMO_CMD_SPEAK:
            sendEvent(STATE_TALK);
            break;
        case DEMO_CMD_END: {
            sendEvent(STATE_WAIT);
            int category = getCategoryQ5(getItem());
            int func = getFtrFunc();
            if (category != 3 && func != dItem::FTR_FUNC_DRESSER && func != dItem::FTR_FUNC_WARDROBE &&
                func != dItem::FTR_FUNC_ARMOIRE && func != dItem::FTR_FUNC_CLOSET && func != dItem::FTR_FUNC_COOLER &&
                func != dItem::FTR_FUNC_ATM && func != dItem::FTR_FUNC_AUDIO_UNUSED && func != dItem::FTR_FUNC_AUDIO &&
                func != dItem::FTR_FUNC_AUDIO_ON_OFF_ANIM && category != 8 && category != 1 &&
                getFtrFunc() != dItem::FTR_FUNC_PHONE) {
                if (getLayer() == 1) {
                    int x;
                    int z;
                    if (getUnit(&x, &z, NULL, mAng(0))) {
                        daFTR_c *base = dFTR::getUnitMap()->get(x, z, 0);
                        if (base != NULL && !mFlags.mSpoken) {
                            base->mFlags.mToggle = 1;
                        }
                    }
                }
                if (!isSpoken()) {
                    mFlags.mToggle = 1;
                }
                mFlags.mSpoken = 0;
            }
            mDemoPos.set(0.0f, 0.0f, 0.0f);
            mMsgItem0 = dItem::Item();
            mMsgItem1 = dItem::Item();
            break;
        }
        }
        return TRUE;
    }
    return FALSE;
}

// 0x8468
void daFTR_c::calcSound() {
    getCenter(&mCenter);
    mSndObj.setPos(&mCenter);
    if (mFlags.mCreated && mState != STATE_APPEAR) {
        mSndFtr.update(&mCenter, getAnmFrame());
    }
}

static const int l_noShadowItem[] = {
    dItem::ITEM_IDX_NOT_USED_FTR_04, dItem::ITEM_IDX_NOT_USED_FTR_05, dItem::ITEM_IDX_NOT_USED_FTR_06,
    dItem::ITEM_IDX_NOT_USED_FTR_07, dItem::ITEM_IDX_NOT_USED_FTR_08,
};
static const int l_noShadowKind[] = {dItem::KIND_SHELL};

// 0x84D0
void daFTR_c::initShadow() {
    static const u32 itemCount = ARRAY_SIZE(l_noShadowItem);
    static const u32 kindCount = ARRAY_SIZE(l_noShadowKind);
    dItem::Item item = getItem();
    int kind = getItemKind();
    mFlags.mShadow = 1;
    for (const int *index = l_noShadowItem; index < l_noShadowItem + itemCount; index++) {
        if (item.isSame(dItem::Item(*index))) {
            mFlags.mShadow = 0;
            return;
        }
    }
    for (const int *noShadow = l_noShadowKind; noShadow < l_noShadowKind + kindCount; noShadow++) {
        if (*noShadow == kind) {
            mFlags.mShadow = 0;
            return;
        }
    }
}

// 0x85D4
void daFTR_c::drawShadow(const mVec3_c *scale) {
    static const f32 shadowScale = 1.0f;
    if (mFlags.mShadow && !getLayer()) {
        f32 sizeX;
        f32 sizeZ;
        switch (getFtrSize()) {
        case dItem::FTR_SIZE_1x1:
            sizeX = sizeZ = 16.0f;
            break;
        case dItem::FTR_SIZE_1x2:
            sizeX = 32.0f;
            sizeZ = 16.0f;
            break;
        default:
            sizeX = sizeZ = 32.0f;
            break;
        }
        mAng angle = getAngleY();
        dShadow::requestRound(&mCenter, 2, &angle, sizeX, scale->x, shadowScale, sizeZ, shadowScale);
    }
}

