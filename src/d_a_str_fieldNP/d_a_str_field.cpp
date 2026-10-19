// The town's structures (REL d_a_str_fieldNP). See
// include/game/game/d_a_str_field.hpp.
// REL .text 0xCC..0x5108, .ctors 0x0..0x4, .rodata 0x0..0x27C, .data
// 0x0..0x27D8, .bss 0x8..0x84.
#include <game/game/d_a_str_field.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_bbs_bird.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_dsn.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_flag.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_def.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_npc_list.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_save_building.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_shop.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_str_manager.hpp>
#include <game/sLib/s_lib.hpp>
#include <lib/egg/math/eggMath.h>

// Debug parameters (the HIO tool's values): the mailbox door's sound frames.
class daStrPost_hostIO_c {
  public:
    daStrPost_hostIO_c() {
        mOpenSeFrame = 15;
        mCloseSeFrame = 20;
    }
    virtual ~daStrPost_hostIO_c() {
    }

    /* 0x04 */ s16 mOpenSeFrame;  // POST_STATE_OPEN: the frame - 5
    /* 0x06 */ s16 mCloseSeFrame; // POST_STATE_CLOSE: frames before the end
}; // size 0x8

// ---------------------------------------------------------------------------
// The gate (daStrChkp_c, profile STR_CHKP)
// ---------------------------------------------------------------------------

// .text:0xCC
namespace dAStrField {
BASE_PROFILE(STR_CHKP, daStrChkp_c)
} // namespace dAStrField

static const char *l_gateFadeMdl[GATE_TYPE_NUM] = {
    "obj_fade_model_c0",
    "obj_fade_model_c1",
    "obj_fade_model_c2",
};

// .text:0x170: the gate's flag shows the town flag's design.
void daStrChkp_c::beforeCreateMdl() {
    dSaveTown_c *town = dSaveData_c::getTown();
    if (!mFlagLoader.loadDesignInPlace(
            &town->mBuilding.mTownFlag, dHeap::strHeap_p, ORG_DESIGN_WIDTH,
            ORG_DESIGN_HEIGHT * ORG_DESIGN_TEX_NUM)) {
        return;
    }
    nw4r::g3d::ResFile file(dSTR::getBrresBank()->getBrres(
        dSTR::getBuildingResName(BUILDING_INDEX(BUILDING_GATE))));
    file.Release();
    file.Bind(nw4r::g3d::ResFile(mFlagLoader.getData()));
    file.Bind(file);
    f32 offsetY = dStrm_c::isUseHostIO() ? dStrm_c::getOffsetY() : 0.0f;
    nw4r::math::MTX34 mtx;
    calcMtx(offsetY + getModelOffsetY(), 0.0f, &mtx);
    static const u32 cGateTypeNum = GATE_TYPE_NUM;
    u32 gate = dSaveData_c::getRaw()->mBuilding.getGateType();
    static mVec3_c l_flagPos[cGateTypeNum] = {
        mVec3_c(123.8f, 77.38f, 21.079f),
        mVec3_c(139.8f, 77.38f, 14.079f),
        mVec3_c(139.8f, 77.37f, 10.966f),
    };
    mVec3_c pos;
    dAStr::toFieldPos(&pos, &l_flagPos[gate < cGateTypeNum ? gate : 0], &mtx);
    dFlag_c::construct(&pos, dFlag_c::KIND_TOWN, this);
}

// .text:0x370
void daStrChkp_c::afterCreateMdl() {
    if (mDoorAnmChr.IsBound()) {
        mDoorAnmChr.mPlayMode = m3d::FORWARD_LOOP;
    }
}

// .text:0x3B0
int daStrChkp_c::execute() {
    if (mDoorAnmChr.IsBound()) {
        mDoorAnmChr.play();
    }
    return SUCCEEDED;
}

// .text:0x400
int daStrChkp_c::doDelete() {
    return mFlagLoader.release();
}

// .text:0x408
const char *daStrChkp_c::getFadeMdlName() {
    static const u32 cGateTypeNum = GATE_TYPE_NUM;
    u32 gate = dSaveData_c::getRaw()->mBuilding.getGateType();
    if (gate < cGateTypeNum) {
        return l_gateFadeMdl[gate];
    }
    return l_gateFadeMdl[0];
}

// ---------------------------------------------------------------------------
// A player's house (daStrPl_c, profile STR_PL)
// ---------------------------------------------------------------------------

// .text:0x470
namespace dAStrField {
BASE_PROFILE(STR_PL, daStrPl_c)
} // namespace dAStrField

// .text:0x524
void daStrPl_c::afterCreateMdl() {
    nw4r::g3d::ResFile common(
        dSTR::getBrresBank()->getBrres("obj_myhome_common"));
    nw4r::g3d::ResAnmClr roof = common.GetResAnmClr("roof_color");
    mRoofAnm.create(mResFile.GetResMdl(0), roof, dStrm_c::getAllocator(), NULL,
                    1);
    mRoofAnm.setAnm(mMdl, roof, 0, m3d::PLAYMODE_INHERIT);
    u32 homeNo = (mBuildingIdx - BUILDING_INDEX(BUILDING_PLAYER_HOUSE_0)) & 3;
    dHome_c *home = dSaveData_c::getTown()->mHomes.getHome(homeNo);
    if ((int)home->mSize == HOME_SIZE_NUM - 1) {
        mHasFlag = home->_15BB.mBits.mHasFlag;
        nw4r::g3d::ResAnmVis vis = mResFile.GetResAnmVis(0);
        mFlagAnm.create(mResFile.GetResMdl(0), vis, dStrm_c::getAllocator(),
                        NULL);
        mFlagAnm.setFrame(mHasFlag);
        mMdl.setAnm(mFlagAnm);
        if (mHasFlag) {
            dFlag_c::construct(&mPos, dFlag_c::KIND_HOME + homeNo, this);
        }
    }
    mRoofAnm.setFrame(home->mRoofColor % HOME_ROOF_COLOR_NUM, 0);
    mMdl.setAnm(mRoofAnm);
    dSaveTown_c *town = dSaveData_c::getTown();
    mIsCurrent = town->mHomes.findCurrentPlayer() == getHouseNo();
    mVec3_c mailbox = mPos;
    mailbox.x -= 32.0f;
    mailbox.z += 32.0f;
    dActor_c::construct(fProf::STR_POST, this,
                        BUILDING_INDEX(BUILDING_MAILBOX_0) + getHouseNo(),
                        &mailbox, NULL);
}

// .text:0x7C0
int daStrPl_c::execute() {
    dSaveTown_c *town = dSaveData_c::getTown();
    mIsCurrent = town->mHomes.findCurrentPlayer() == getHouseNo();
    return SUCCEEDED;
}

// .text:0x82C
int daStrPl_c::getCheckMsg() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player != NULL && player->isFlag0(PRIVATE_FLAG0_HOUSE_TOUR)) {
        dSaveTown_c *town = dSaveData_c::getTown();
        if (!town->mHomes.getHome(getHouseNo())->mOwner.isValid()) {
            return DOOR_MSG_FOR_SALE;
        }
    }
    return DOOR_MSG_NONE;
}

// .text:0x8B4
int daStrPl_c::doDelete() {
    mRoofAnm.remove();
    return SUCCEEDED;
}

// .text:0x8E4
int daStrPl_c::getHouseNo() {
    return dSaveBuildingList_c::getPlayerHouseNo(mBuildingIdx);
}

// .text:0x8EC: the mailbox is drawn with the house.
void daStrPl_c::drawMdl() {
    daSTR_c::drawMdl();
    u16 id = BUILDING_MAILBOX_0 + getHouseNo();
    daSTR_c *mailbox = dSTR::getAcList()->searchBuilding(id, TRUE);
    if (mailbox != NULL && !mailbox->isDeleteRequested()) {
        mailbox->drawMdl();
    }
}

// .text:0x960
BOOL daStrPl_c::isWindowLit() {
    dSaveTown_c *town = dSaveData_c::getTown();
    dHome_c *home = town->mHomes.getHome(getHouseNo());
    if (home != NULL) {
        return home->hasRoom456();
    }
    return FALSE;
}

static daStrPost_hostIO_c l_postHostIO;

// ---------------------------------------------------------------------------
// A player's mailbox (daStrPost_c, profile STR_POST)
// ---------------------------------------------------------------------------

// .text:0x9C4
namespace dAStrField {
BASE_PROFILE(STR_POST, daStrPost_c)
} // namespace dAStrField

// .text:0xA84
BOOL daStrPost_c::setupAnm() {
    mAllocator_c *allocator = dStrm_c::getAllocator();
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl(0);
    mNewMailAnm = mResFile.GetResAnmChr("obj_post_newmail");
    mOpenAnm = mResFile.GetResAnmChr("obj_post_open");
    mBlinkClr = mResFile.GetResAnmClr("obj_post_blink");
    BOOL chr = mDoorAnmChr.create(mdl, mOpenAnm, allocator, NULL);
    if (chr) {
        mDoorAnmChr.mPlayMode = m3d::FORWARD_ONCE;
        mMdl.setAnm(mDoorAnmChr);
    }
    BOOL clr = mBlinkAnm.create(mdl, mBlinkClr, allocator, NULL, 1);
    if (clr) {
        mBlinkAnm.setPlayMode(m3d::FORWARD_ONCE, 0);
        mMdl.setAnm(mBlinkAnm);
    }
    return chr && clr;
}

// .text:0xBEC
void daStrPost_c::afterCreateMdl() {
    setupAnm();
    mHouseNo = mParam - BUILDING_INDEX(BUILDING_MAILBOX_0);
    mIsCurrent = mHouseNo == dSaveData_c::getTown()->mHomes.findCurrentPlayer();
    mDemoPos = mPos;
    setState(STATE_WAIT);
    mObjc.set(this, 16.0f, 41.6f, dObjc::HIT_STR, 0, 0, 0);
    if (mIsCurrent) {
        if (hasNewMail()) {
            setDoorState(POST_STATE_MAIL);
        } else {
            setDoorState(POST_STATE_EMPTY);
        }
    } else if (dNet::isSessionActive() && dNet::getMemberNum() >= 2) {
        u8 state = getNetState();
        setDoorState(state);
    } else {
        setDoorState(POST_STATE_EMPTY);
    }
}

// .text:0xD44
int daStrPost_c::execute() {
    mIsCurrent = mHouseNo == dSaveData_c::getTown()->mHomes.findCurrentPlayer();
    mObjc.regist();
    mSnd.setPos(&mPos);
    executeState();
    return SUCCEEDED;
}

// .text:0xDB0
int daStrPost_c::doDelete() {
    mBlinkAnm.remove();
    return SUCCEEDED;
}

// .text:0xDE0
bool daStrPost_c::canInteract(dDemoActor_c *actor) {
    static const f32 cTalkRange = 35.2f;
    clearDemoState();
    if (mIsCurrent && mDoorState == POST_STATE_MAIL && actor != NULL) {
        mVec3_c diff = mPos - actor->getPos();
        if (EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z) <
                cTalkRange &&
            sLib::distanceAngle(mAngle.y.mAngle + DEG_TO_ANGLE(180),
                                actor->getAngle().y) < DEG_TO_ANGLE(45) &&
            actor->getPos().z > 16.0f + mPos.z) {
            return true;
        }
    }
    return false;
}

// .text:0xEB8
void daStrPost_c::onDemoCmd(int cmd, u8) {
    switch (cmd) {
    case DEMO_CMD_TALK:
    case DEMO_CMD_SPEAK:
        setState(STATE_OPEN_BOX);
        break;
    case DEMO_CMD_END:
        setState(STATE_WAIT);
        break;
    }
}

// .text:0xEE8
BOOL daStrPost_c::hasNewMail() {
    if (mIsCurrent) {
        return dPostOffice::hasMail();
    }
    return FALSE;
}

static BOOL (
    daStrPost_c::*const l_postDoorInit[daStrPost_c::POST_STATE_NUM])() = {
    &daStrPost_c::initEmpty,    &daStrPost_c::initNewMail,
    &daStrPost_c::initMail,     &daStrPost_c::initOpen,
    &daStrPost_c::initOpened,   &daStrPost_c::initClose,
    &daStrPost_c::initFlagDown,
};

static void (
    daStrPost_c::*const l_postDoorExecute[daStrPost_c::POST_STATE_NUM])() = {
    &daStrPost_c::executeEmpty,    &daStrPost_c::executeNewMail,
    &daStrPost_c::executeMail,     &daStrPost_c::executeOpen,
    &daStrPost_c::executeOpened,   &daStrPost_c::executeClose,
    &daStrPost_c::executeFlagDown,
};

// .text:0xF00
BOOL daStrPost_c::setDoorState(int state) {
    if ((u32)state < POST_STATE_NUM && (this->*l_postDoorInit[state])()) {
        sendNetState(state);
        mDoorState = state;
        return TRUE;
    }
    return FALSE;
}

// .text:0xF78
void daStrPost_c::executeDoorState() {
    if (mDoorState < POST_STATE_NUM) {
        (this->*l_postDoorExecute[mDoorState])();
    }
    if (mBlinkAnm.checkFrame(60.0f, 0)) {
        mSnd.startSound(SE_MailAlert);
    }
    if (!mIsCurrent && dNet::isSessionActive() && dNet::getMemberNum() >= 2) {
        u8 state = getNetState();
        if (mDoorState == POST_STATE_EMPTY && state == POST_STATE_MAIL) {
            state = POST_STATE_NEW_MAIL;
        }
        if (!(mDoorState == POST_STATE_NEW_MAIL && state == POST_STATE_MAIL) &&
            mDoorState != state) {
            setDoorState(state);
        }
    }
}

// .text:0x1068
BOOL daStrPost_c::initEmpty() {
    mDoorAnmChr.setAnm(mMdl, mNewMailAnm, m3d::FORWARD_ONCE);
    mDoorAnmChr.setFrame(0.0f);
    mDoorAnmChr.setRate(1.0f);
    mMdl.setAnm(mDoorAnmChr);
    mBlinkAnm.setAnm(mMdl, mBlinkClr, 0, m3d::FORWARD_ONCE);
    mBlinkAnm.setFrame(0.0f, 0);
    mBlinkAnm.setRate(1.0f, 0);
    mMdl.setAnm(mBlinkAnm);
    return TRUE;
}

// .text:0x1150
void daStrPost_c::executeEmpty() {
    if (hasNewMail()) {
        setDoorState(POST_STATE_NEW_MAIL);
    }
}

// .text:0x119C: the flag goes up.
BOOL daStrPost_c::initNewMail() {
    mDoorAnmChr.setAnm(mMdl, mNewMailAnm, m3d::FORWARD_ONCE);
    mDoorAnmChr.setFrame(0.0f);
    mDoorAnmChr.setRate(1.0f);
    mMdl.setAnm(mDoorAnmChr);
    mSnd.startSound(SE_MailBoxLeverUp);
    return TRUE;
}

// .text:0x1228
void daStrPost_c::executeNewMail() {
    if (mDoorAnmChr.isStop()) {
        setDoorState(POST_STATE_MAIL);
    } else {
        mDoorAnmChr.play();
    }
}

// .text:0x1290
BOOL daStrPost_c::initMail() {
    mDoorAnmChr.setAnm(mMdl, mNewMailAnm, m3d::FORWARD_ONCE);
    mDoorAnmChr.setFrame(mNewMailAnm.ref().info.numFrame - 1);
    mDoorAnmChr.setRate(1.0f);
    mMdl.setAnm(mDoorAnmChr);
    mBlinkAnm.setAnm(mMdl, mBlinkClr, 0, m3d::FORWARD_LOOP);
    mBlinkAnm.setFrame(0.0f, 0);
    mBlinkAnm.setRate(1.0f, 0);
    mMdl.setAnm(mBlinkAnm);
    return TRUE;
}

// .text:0x1398
void daStrPost_c::executeMail() {
    mBlinkAnm.play();
}

// .text:0x13A8
BOOL daStrPost_c::initOpen() {
    mDoorAnmChr.setAnm(mMdl, mOpenAnm, m3d::FORWARD_ONCE);
    mDoorAnmChr.setFrame(0.0f);
    mDoorAnmChr.setRate(1.0f);
    mMdl.setAnm(mDoorAnmChr);
    return TRUE;
}

// .text:0x1428
void daStrPost_c::executeOpen() {
    if (mDoorAnmChr.checkFrame(l_postHostIO.mOpenSeFrame + 5)) {
        mSnd.startSound(SE_MailBoxOpen);
    }
    mDoorAnmChr.play();
    mBlinkAnm.play();
    if (mIsCurrent && mDoorAnmChr.isStop()) {
        setDoorState(POST_STATE_OPENED);
    }
}

// .text:0x14F8
BOOL daStrPost_c::initOpened() {
    mDoorAnmChr.setAnm(mMdl, mOpenAnm, m3d::FORWARD_ONCE);
    mDoorAnmChr.setFrame(mOpenAnm.ref().info.numFrame - 1);
    mDoorAnmChr.setRate(1.0f);
    mMdl.setAnm(mDoorAnmChr);
    return TRUE;
}

// .text:0x159C
void daStrPost_c::executeOpened() {
    mBlinkAnm.play();
}

// .text:0x15AC
BOOL daStrPost_c::initClose() {
    mDoorAnmChr.setAnm(mMdl, mOpenAnm, m3d::REVERSE_ONCE);
    mDoorAnmChr.setFrame(mOpenAnm.ref().info.numFrame - 1);
    mDoorAnmChr.setRate(1.0f);
    mMdl.setAnm(mDoorAnmChr);
    return TRUE;
}

// .text:0x1650
void daStrPost_c::executeClose() {
    if (mDoorAnmChr.checkFrame(mOpenAnm.ref().info.numFrame -
                               l_postHostIO.mCloseSeFrame)) {
        mSnd.startSound(SE_MailBoxClose);
    }
    if (mIsCurrent && mDoorAnmChr.isStop()) {
        if (!hasNewMail()) {
            setDoorState(POST_STATE_FLAG_DOWN);
        } else {
            setDoorState(POST_STATE_MAIL);
        }
    } else {
        mDoorAnmChr.play();
    }
    mBlinkAnm.play();
}

// .text:0x1758: the flag goes down.
BOOL daStrPost_c::initFlagDown() {
    mDoorAnmChr.setAnm(mMdl, mNewMailAnm, m3d::REVERSE_ONCE);
    mDoorAnmChr.setFrame(mNewMailAnm.ref().info.numFrame - 1);
    mDoorAnmChr.setRate(1.0f);
    mMdl.setAnm(mDoorAnmChr);
    return TRUE;
}

// .text:0x17FC
void daStrPost_c::executeFlagDown() {
    if (mDoorAnmChr.checkFrame(2.0f)) {
        mSnd.startSound(SE_MailBoxLeverDown);
    }
    if (mIsCurrent && mDoorAnmChr.isStop()) {
        setDoorState(POST_STATE_EMPTY);
    } else {
        mDoorAnmChr.play();
    }
    mBlinkAnm.play();
}

static BOOL (daStrPost_c::*const l_postStateInit[daStrPost_c::STATE_NUM])() = {
    &daStrPost_c::initWait,     &daStrPost_c::initOpenBox,
    &daStrPost_c::initMenu,     &daStrPost_c::initMenuWait,
    &daStrPost_c::initCloseBox,
};

static void (
    daStrPost_c::*const l_postStateExecute[daStrPost_c::STATE_NUM])() = {
    &daStrPost_c::executeWait,     &daStrPost_c::executeOpenBox,
    &daStrPost_c::executeMenu,     &daStrPost_c::executeMenuWait,
    &daStrPost_c::executeCloseBox,
};

// .text:0x18A8
BOOL daStrPost_c::setState(int state) {
    if (state < STATE_NUM && (this->*l_postStateInit[state])()) {
        mPostState = state;
        return TRUE;
    }
    return FALSE;
}

// .text:0x1914
void daStrPost_c::executeState() {
    if (mPostState < STATE_NUM) {
        (this->*l_postStateExecute[mPostState])();
    }
}

// .text:0x1954
BOOL daStrPost_c::initWait() {
    return TRUE;
}

// .text:0x195C
void daStrPost_c::executeWait() {
}

// .text:0x1960
BOOL daStrPost_c::initOpenBox() {
    dAcPlayer_c *player =
        dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    mVec3_c pos(mPos.x, player->getPos().y, mPos.z + 32.0f);
    if (dPlayerMgr::openMailbox(pos.x, pos.z)) {
        setDoorState(POST_STATE_OPEN);
        return TRUE;
    }
    return FALSE;
}

// .text:0x19F4
void daStrPost_c::executeOpenBox() {
    if (mDoorState == POST_STATE_OPENED) {
        setState(STATE_MENU);
    }
}

// .text:0x1A0C
BOOL daStrPost_c::initMenu() {
    return dMenuMng_c::openMenu(dMenuMng_c::MENU_MAILBOX) != 0;
}

// .text:0x1A3C
void daStrPost_c::executeMenu() {
    setState(STATE_MENU_WAIT);
}

// .text:0x1A44
BOOL daStrPost_c::initMenuWait() {
    return TRUE;
}

// .text:0x1A4C
void daStrPost_c::executeMenuWait() {
    if (dMenuMng_c::isMenuEnd() && dPlayerMgr::closeMailbox()) {
        setState(STATE_CLOSE_BOX);
    }
}

// .text:0x1A98
BOOL daStrPost_c::initCloseBox() {
    setDoorState(POST_STATE_CLOSE);
    return TRUE;
}

// .text:0x1ACC
void daStrPost_c::executeCloseBox() {
    if (mDoorState == POST_STATE_EMPTY || mDoorState == POST_STATE_MAIL) {
        dDemo::finishTalk(this);
    }
}

// .text:0x1AE8: tells the other players (the house's NET_REC_MAILBOX record).
void daStrPost_c::sendNetState(int state) {
    u8 record = state;
    if (mIsCurrent) {
        dNet::setRecord(NET_REC_MAILBOX + (mHouseNo & 3), &record, 0);
    }
}

// .text:0x1B2C
u8 daStrPost_c::getNetState() {
    if (!mIsCurrent) {
        u8 *state = (u8 *)dNet::getRecord(NET_REC_MAILBOX + (mHouseNo & 3));
        if (state != NULL) {
            return *state;
        }
    }
    return POST_STATE_EMPTY;
}

// ---------------------------------------------------------------------------
// The construction site (daStrReserve_c, profile STR_RESERVE)
// ---------------------------------------------------------------------------

// .text:0x1B78
namespace dAStrField {
BASE_PROFILE(STR_RESERVE, daStrReserve_c)
} // namespace dAStrField

// .text:0x1BD4
void daStrReserve_c::afterCreateMdl() {
    mSiteNo = dStrm_c::sBuildSiteNo;
    dFdBase_c *field = getCurrentField();
    dItem::Item site = BUILDING_BUILD_SITE;
    field->setItem(&site, &mPos, 0);
    mDemoPos = mPos;
}

// .text:0x1C44
u16 daStrReserve_c::getDemoCode() const {
    return (mUnitZ & 0x7F) << 9 | (mUnitX & 0x7F) << 2;
}

// .text:0x1C58
void daStrReserve_c::onTalkStart(int kind, dMsg::Comp_c *comp) {
    setMessageLabel("sys_MESS/SYS_Board");
    setMessageCode(getBoardMsg());
}

// .text:0x1CA4: one of the construction signs' messages, by the site.
int daStrReserve_c::getBoardMsg() {
    return (u32)(dSaveData_c::getTown()->getLandID()->mId + mSiteNo) %
               BOARD_MSG_NUM +
           BOARD_MSG_FIRST;
}

// .text:0x1CFC
void daStrReserve_c::onMessageStart(int arg) {
    dItem::Item fruit = dSaveData_c::getTown()->mTownInfo.mFruit;
    dMsg::Comp_c *comp = mpController;
    comp->setItemName(1, fruit);
    comp->setStringWord(2,
                        (dSaveData_c::getTown()->mShops.mShop.mStage & 3) +
                            SHOP_NAME_UNIT_MSG,
                        "sys_STRING/STR_Unit");
}

// ---------------------------------------------------------------------------
// A villager's house (daStrNpc_c, profile STR_NPC)
// ---------------------------------------------------------------------------

// .text:0x1D74
namespace dAStrField {
BASE_PROFILE(STR_NPC, daStrNpc_c)
} // namespace dAStrField

// .text:0x1E10
void daStrNpc_c::afterCreateMdl() {
    nw4r::g3d::ResFile common(dSTR::getBrresBank()->getBrres("obj_npc_common"));
    nw4r::g3d::ResAnmTexPat pat;
    switch (dSTR::getTownRegion()) {
    case CONSOLE_REGION_NA:
        pat = common.GetResAnmTexPat("obj_house_usa");
        break;
    case CONSOLE_REGION_EU:
        pat = common.GetResAnmTexPat("obj_house_eur");
        break;
    default:
        pat = common.GetResAnmTexPat("obj_house_jpn");
        break;
    }
    mHouseAnm.create(mResFile.GetResMdl(0), pat, dStrm_c::getAllocator(), NULL,
                     1);
    mHouseAnm.setAnm(mMdl, pat, 0, m3d::PLAYMODE_INHERIT);
    mHouseAnm.setFrame(getHouseType(), 0);
    mMdl.setAnm(mHouseAnm);
    mVec3_c plate = mPos;
    plate.x -= 38.4f;
    plate.z += 32.0f;
    dActor_c::construct(fProf::A_NAMEPLATE, this, getNpcNo(), &plate, NULL);
}

// .text:0x1FB8
int daStrNpc_c::getNpcNo() {
    if (dSaveBuildingList_c::isNpcHouse(mBuildingIdx)) {
        return dSaveBuildingList_c::getNpcHouseNo(mBuildingIdx);
    }
    return 0;
}

// .text:0x2000
int daStrNpc_c::getHouseType() {
    u32 type[2];
    dSTR::getNpcHouseType(type, getNpcNo());
    return type[1];
}

// .text:0x2030
void daStrNpc_c::drawMdl() {
    daSTR_c::drawMdl();
}

// .text:0x2034
int daStrNpc_c::execute() {
    return SUCCEEDED;
}

// .text:0x203C
int daStrNpc_c::doDelete() {
    mHouseAnm.remove();
    return SUCCEEDED;
}

// .text:0x206C
void daStrNpc_c::onTalkStart(int kind, dMsg::Comp_c *comp) {
    dSaveTown_c *town = dSaveData_c::getTown();
    dAnimal_c *animal = town->mAnimals.mTown.getAnimal(getNpcNo());
    if (animal != NULL) {
        dItem::Item key = animal->getKey();
        dHmnName::Word_c name;
        if (dNpc::getNpcName(&name, &key, LANGUAGE_NUM)) {
            comp->setWord(0, &name);
        }
    }
}

// .text:0x2118
int daStrNpc_c::getCheckMsg() {
    // By the villager's personality (dAnmPersonalID_c::getLooks).
    static const u8 l_asleepMsg[] = {
        DOOR_MSG_NPC_ASLEEP + 3, DOOR_MSG_NPC_ASLEEP + 4,
        DOOR_MSG_NPC_ASLEEP + 5, DOOR_MSG_NPC_ASLEEP,
        DOOR_MSG_NPC_ASLEEP + 1, DOOR_MSG_NPC_ASLEEP + 2,
    };
    static const u8 l_cleaningMsg[] = {
        DOOR_MSG_NPC_CLEANING + 3, DOOR_MSG_NPC_CLEANING + 4,
        DOOR_MSG_NPC_CLEANING + 5, DOOR_MSG_NPC_CLEANING,
        DOOR_MSG_NPC_CLEANING + 1, DOOR_MSG_NPC_CLEANING + 2,
    };
    static const u8 l_sickMsg[] = {
        DOOR_MSG_NPC_SICK + 3, DOOR_MSG_NPC_SICK + 4, DOOR_MSG_NPC_SICK + 5,
        DOOR_MSG_NPC_SICK,     DOOR_MSG_NPC_SICK + 1, DOOR_MSG_NPC_SICK + 2,
    };
    static const u8 l_outMsg[] = {
        DOOR_MSG_NPC_OUT + 3, DOOR_MSG_NPC_OUT + 4, DOOR_MSG_NPC_OUT + 5,
        DOOR_MSG_NPC_OUT,     DOOR_MSG_NPC_OUT + 1, DOOR_MSG_NPC_OUT + 2,
    };
    dAnimalBlock_c *block;
    int no = getNpcNo();
    block = &dSaveData_c::getTown()->mAnimals.mTown;
    dAnimal_c *animal = block->getAnimal(no);
    if (animal != NULL) {
        u8 looks = animal->mID.getLooks(1);
        int place = animal->mPlace;
        if (!animal->isLostItemRequestDone(NULL, TRUE)) {
            return DOOR_MSG_LOCKED;
        }
        if (place == dAnimal_c::PLACE_ASLEEP) {
            return l_asleepMsg[looks];
        }
        if (block->isAppointmentSoon(no)) {
            return l_cleaningMsg[looks];
        }
        if (animal->isSickForVisitors()) {
            return l_sickMsg[looks];
        }
        if (place != dAnimal_c::PLACE_HOME) {
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
            if (!dStrm_c::isToyDay() ||
                (player != NULL &&
                 player->isFlag0(PRIVATE_FLAG0_PART_TIME_JOB))) {
                return l_outMsg[looks];
            }
        }
    }
    return DOOR_MSG_NONE;
}

// .text:0x2254: Toy Day: the lights are on while the villager is home.
BOOL daStrNpc_c::isNight() {
    if (dStrm_c::isToyDay()) {
        dSaveTown_c *town = dSaveData_c::getTown();
        dAnimal_c *animal = town->mAnimals.mTown.getAnimal(getNpcNo());
        if (animal != NULL && (int)animal->mPlace != dAnimal_c::PLACE_HOME) {
            return FALSE;
        }
    }
    return TRUE;
}

// .text:0x22D0
BOOL daStrNpc_c::isWindowLit() {
    int msg = getCheckMsg();
    return isNight() &&
           (msg == DOOR_MSG_NONE ||
            (msg >= DOOR_MSG_NPC_CLEANING &&
             msg <= DOOR_MSG_NPC_CLEANING + DOOR_MSG_NPC_NUM - 1) ||
            (msg >= DOOR_MSG_NPC_SICK &&
             msg <= DOOR_MSG_NPC_SICK + DOOR_MSG_NPC_NUM - 1));
}

// ---------------------------------------------------------------------------
// Nook's store (daStrShop_c, profile STR_SHOP)
// ---------------------------------------------------------------------------

// .text:0x2364
namespace dAStrField {
BASE_PROFILE(STR_SHOP, daStrShop_c)
} // namespace dAStrField

// .text:0x23C0
void daStrShop_c::beforeCreateMdl() {
    mStage = dSaveData_c::getTown()->mShops.mShop.mStage & 3;
}

// .text:0x23FC: Nook is out on the opening event (A_NPC_SP_OP_TANUKICHI).
BOOL daStrShop_c::isOpNookHere() {
    int num = dNpcList::countSp();
    for (int i = 0; i < num; i++) {
        dDemoActor_c *npc = dNpcList::getSp(i);
        if (npc != NULL && npc->mProfName == fProf::A_NPC_SP_OP_TANUKICHI) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text:0x246C
int daStrShop_c::getCheckMsg() {
    dHomeList_c *homes;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player != NULL && player->isFlag0(PRIVATE_FLAG0_HOUSE_TOUR)) {
        return DOOR_MSG_NOOK_OUT;
    }
    if (isOpNookHere()) {
        return DOOR_MSG_NOOK_OUT;
    }
    if (player != NULL && !player->isFlag0(PRIVATE_FLAG0_TRAVELLING)) {
        homes = &dSaveData_c::getTown()->mHomes;
        if (homes->getHome(homes->findCurrentPlayer()) == NULL &&
            (!dNet::isSessionActive() || dNet::getMemberNum() < 2)) {
            return DOOR_MSG_NOOK_OUT;
        }
    }
    if (player != NULL && player->isFlag0(PRIVATE_FLAG0_PART_TIME_JOB)) {
        return DOOR_MSG_NONE;
    }
    if (dSaveData_c::getTown()->mShops.mShop.isUpgradingOnLastDay()) {
        return DOOR_MSG_SHOP_REMODELING;
    }
    if (dSaveData_c::getTown()->mShops.mShop.isOpen() == FALSE) {
        return DOOR_MSG_SHOP_CLOSED + (mStage & 3);
    }
    return DOOR_MSG_NONE;
}

// .text:0x25B4
int daStrShop_c::getDoorType() const {
    return mStage == SHOP_STAGE_CRANNY ? DOOR_AUTO : DOOR_HINGED;
}

// .text:0x25CC
int daStrShop_c::getSceneOffset() {
    return mStage & 3;
}

// ---------------------------------------------------------------------------
// The bulletin board (daStrBbs_c, profile STR_BBS)
// ---------------------------------------------------------------------------

// .text:0x25D8
namespace dAStrField {
BASE_PROFILE(STR_BBS, daStrBbs_c)
} // namespace dAStrField

// .text:0x2634
void daStrBbs_c::afterCreateMdl() {
    static const f32 cBoardHeight = 57.6f;
    setState(STATE_WAIT);
    // Not passed on: the bird is created at mPos.
    mVec3_c birdPos = mPos;
    birdPos.x -= 16.0f;
    birdPos.y += 49.5f;
    dActor_c::construct(fProf::A_BBS_BIRD, this, 0, &mPos, NULL);
    mVec3_c bottomL(mPos.x - 48.0f, mPos.y, 16.0f + mPos.z);
    mVec3_c bottomR(16.0f + mPos.x, mPos.y, 16.0f + mPos.z);
    mVec3_c topL = bottomL;
    mVec3_c topR = bottomR;
    topL.y += cBoardHeight;
    topR.y += cBoardHeight;
    mVec3_c center = bottomL + bottomR;
    center *= 0.5f;
    mDpdQuad.set(topL, bottomL, bottomR, topR, dObjc::HIT_STR, this, &center,
                 0);
    mMdl.setPriorityDraw(2, 0x7F);
}

// .text:0x2794
int daStrBbs_c::execute() {
    executeState();
    mDpdQuad.regist();
    return SUCCEEDED;
}

// .text:0x27CC
int daStrBbs_c::doDelete() {
    return SUCCEEDED;
}

// .text:0x27D4
void daStrBbs_c::drawMdl() {
    daSTR_c::drawMdl();
    daBbsBird_c::s_strDrawn = TRUE;
}

// .text:0x2800
bool daStrBbs_c::canInteract(dDemoActor_c *actor) {
    if (actor != NULL &&
        sLib::distanceAngle(mAngle.y.mAngle + DEG_TO_ANGLE(180),
                            actor->getAngle().y) < DEG_TO_ANGLE(45)) {
        clearDemoState();
        return mInFront;
    }
    return false;
}

// .text:0x2864
void daStrBbs_c::onDemoCmd(int cmd, u8) {
    switch (cmd) {
    case DEMO_CMD_TALK:
        setState(STATE_MENU);
        break;
    case DEMO_CMD_END:
        setState(STATE_WAIT);
        break;
    }
}

static BOOL (daStrBbs_c::*const l_bbsStateInit[daStrBbs_c::STATE_NUM])() = {
    &daStrBbs_c::initWait,
    &daStrBbs_c::initMenu,
    &daStrBbs_c::initMenuWait,
};

static void (daStrBbs_c::*const l_bbsStateExecute[daStrBbs_c::STATE_NUM])() = {
    &daStrBbs_c::executeWait,
    &daStrBbs_c::executeMenu,
    &daStrBbs_c::executeMenuWait,
};

// .text:0x288C
BOOL daStrBbs_c::setState(int state) {
    if (state < STATE_NUM && (this->*l_bbsStateInit[state])()) {
        mBbsState = state;
        return TRUE;
    }
    return FALSE;
}

// .text:0x28F8
void daStrBbs_c::executeState() {
    if (mBbsState < STATE_NUM) {
        (this->*l_bbsStateExecute[mBbsState])();
    }
}

// .text:0x2938
BOOL daStrBbs_c::initWait() {
    return TRUE;
}

// .text:0x2940: the talk spot follows the player along the board.
void daStrBbs_c::executeWait() {
    static const f32 cRightEnd = 6.4f;
    dAcPlayer_c *player =
        dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (player != NULL) {
        f32 left = mPos.x - 38.4f;
        f32 right = cRightEnd + mPos.x;
        const f32 &x = player->getPos().x;
        mVec3_c pos = mPos;
        if (x < left) {
            pos.x = left;
            mInFront = FALSE;
        } else if (x > right) {
            pos.x = right;
            mInFront = FALSE;
        } else {
            pos.x = x;
            mInFront = TRUE;
        }
        mDemoPos = pos;
    }
}

// .text:0x2A04
BOOL daStrBbs_c::initMenu() {
    return dMenuMng_c::openMenu(dMenuMng_c::MENU_BBS) != 0;
}

// .text:0x2A34
void daStrBbs_c::executeMenu() {
    setState(STATE_MENU_WAIT);
}

// .text:0x2A3C
BOOL daStrBbs_c::initMenuWait() {
    return TRUE;
}

// .text:0x2A44
void daStrBbs_c::executeMenuWait() {
    if (dMenuMng_c::isMenuEnd()) {
        dDemo::finishTalk(this);
    }
}

// ---------------------------------------------------------------------------
// The museum (daStrMuseum_c, profile STR_MUSEUM)
// ---------------------------------------------------------------------------

// .text:0x2A80
namespace dAStrField {
BASE_PROFILE(STR_MUSEUM, daStrMuseum_c)
} // namespace dAStrField

static const char *l_museumFadeMdl = "obj_fade_model_m";

// .text:0x2ADC
int daStrMuseum_c::getCheckMsg() {
    return DOOR_MSG_NONE;
}

// .text:0x2AE4
const char *daStrMuseum_c::getFadeMdlName() {
    return l_museumFadeMdl;
}

// ---------------------------------------------------------------------------
// The tailor (daStrTailor_c, profile STR_TAILOR)
// ---------------------------------------------------------------------------

// .text:0x2AF0
namespace dAStrField {
BASE_PROFILE(STR_TAILOR, daStrTailor_c)
} // namespace dAStrField

// .text:0x2B4C: open from 8:00 to 23:00.
int daStrTailor_c::getCheckMsg() {
    dTime_c now = *dTime_c::getCurrent();
    if (now.hour >= OPEN_HOUR && now.hour < CLOSE_HOUR) {
        return DOOR_MSG_NONE;
    }
    return DOOR_MSG_TAILOR_CLOSED;
}

// ---------------------------------------------------------------------------
// The town hall (daStrOffice_c, profile STR_OFFICE)
// ---------------------------------------------------------------------------

// .text:0x2BD8
namespace dAStrField {
BASE_PROFILE(STR_OFFICE, daStrOffice_c)
} // namespace dAStrField

// .text:0x2C34
int daStrOffice_c::getCheckMsg() {
    return DOOR_MSG_NONE;
}

// ---------------------------------------------------------------------------
// The tent (daStrTent_c, profile STR_TENT)
// ---------------------------------------------------------------------------

// .text:0x2C3C
namespace dAStrField {
BASE_PROFILE(STR_TENT, daStrTent_c)
} // namespace dAStrField

// ---------------------------------------------------------------------------
// The harvest festival's tent (daStrHarvest_c, profile STR_HARVEST)
// ---------------------------------------------------------------------------

// .text:0x2C98
namespace dAStrField {
BASE_PROFILE(STR_HARVEST, daStrHarvest_c)
} // namespace dAStrField

// ---------------------------------------------------------------------------
// The lighthouse (daStrLightHs_c, profile STR_LIGHT_HS)
// ---------------------------------------------------------------------------

// .text:0x2CF4
namespace dAStrField {
BASE_PROFILE(STR_LIGHT_HS, daStrLightHs_c)
} // namespace dAStrField

// .text:0x2D88
void daStrLightHs_c::afterCreateMdl() {
    mAllocator_c *allocator = dStrm_c::getAllocator();
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl(0);
    nw4r::g3d::ResAnmChr anm(NULL);
    if (getDoorAnm(&anm, 0)) {
        mDoorAnmChr.create(mdl, anm, allocator, NULL);
        mDoorAnmChr.mPlayMode = m3d::FORWARD_LOOP;
        mMdl.setAnm(mDoorAnmChr);
    }
    if (mResFile.GetResAnmClrNumEntries() != 0) {
        mLightAnm.create(mdl, mResFile.GetResAnmClr(0), allocator, NULL, 1);
        mLightAnm.setPlayMode(m3d::FORWARD_LOOP, 0);
        mMdl.setAnm(mLightAnm);
    }
}

// .text:0x2EC8: the light turns while the windows are lit; off, it turns on
// to its start.
int daStrLightHs_c::execute() {
    if (isWindowLit()) {
        if (mDoorAnmChr.IsBound()) {
            mDoorAnmChr.play();
        }
        if (mLightAnm.IsBound()) {
            mLightAnm.play();
        }
    } else {
        if (mDoorAnmChr.IsBound() && mDoorAnmChr.getFrame() != 0.0f) {
            mDoorAnmChr.play();
        }
        if (mLightAnm.IsBound() && mLightAnm.getFrame(0) != 0.0f) {
            mLightAnm.play();
        }
    }
    return SUCCEEDED;
}

// .text:0x2FD4
int daStrLightHs_c::doDelete() {
    if (mDoorAnmChr.IsBound()) {
        mDoorAnmChr.remove();
    }
    if (mLightAnm.IsBound()) {
        mLightAnm.remove();
    }
    return SUCCEEDED;
}

// ---------------------------------------------------------------------------
// The windmill (daStrWindmill_c, profile STR_WINDMILL)
// ---------------------------------------------------------------------------

// .text:0x3048
namespace dAStrField {
BASE_PROFILE(STR_WINDMILL, daStrWindmill_c)
} // namespace dAStrField

// .text:0x30A4
void daStrWindmill_c::afterCreateMdl() {
    mAllocator_c *allocator = dStrm_c::getAllocator();
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl(0);
    nw4r::g3d::ResAnmChr anm(NULL);
    if (getDoorAnm(&anm, 0)) {
        mDoorAnmChr.create(mdl, anm, allocator, NULL);
        mDoorAnmChr.mPlayMode = m3d::FORWARD_LOOP;
        mMdl.setAnm(mDoorAnmChr);
    }
}

// .text:0x317C
int daStrWindmill_c::execute() {
    if (mDoorAnmChr.IsBound()) {
        mDoorAnmChr.play();
    }
    return SUCCEEDED;
}

// .text:0x31CC
int daStrWindmill_c::doDelete() {
    if (mDoorAnmChr.IsBound()) {
        mDoorAnmChr.remove();
    }
    return SUCCEEDED;
}

// ---------------------------------------------------------------------------
// The crashed UFO (daStrUFO_c, profile STR_UFO)
// ---------------------------------------------------------------------------

// .text:0x321C
namespace dAStrField {
BASE_PROFILE(STR_UFO, daStrUFO_c)
} // namespace dAStrField

// .text:0x3278
void daStrUFO_c::afterCreateMdl() {
    mAllocator_c *allocator = dStrm_c::getAllocator();
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl(0);
    mDraw = FALSE;
    nw4r::g3d::ResAnmChr anm(NULL);
    if (getDoorAnm(&anm, 0)) {
        mDoorAnmChr.create(mdl, anm, allocator, NULL);
        mDoorAnmChr.mPlayMode = m3d::FORWARD_LOOP;
        mMdl.setAnm(mDoorAnmChr);
    }
}

// .text:0x3354
int daStrUFO_c::execute() {
    mDraw = dSaveData_c::getRaw()->isFlag(SAVE_FLAG_UFO_STR_DRAW);
    dSaveData_c::getTown()->clearFlag(SAVE_FLAG_UFO_STR_DRAW);
    if (mDoorAnmChr.IsBound()) {
        mDoorAnmChr.play();
    }
    return SUCCEEDED;
}

// .text:0x33C0
int daStrUFO_c::doDelete() {
    if (mDoorAnmChr.IsBound()) {
        mDoorAnmChr.remove();
    }
    return SUCCEEDED;
}

// ---------------------------------------------------------------------------
// The fountain (daStrFountain_c, profile STR_FOUNTAIN)
// ---------------------------------------------------------------------------

// .text:0x3410
namespace dAStrField {
BASE_PROFILE(STR_FOUNTAIN, daStrFountain_c)
} // namespace dAStrField

// .text:0x3524: speaks with the goddess's voice.
int daStrFountain_c::getVoiceMode() {
    dItem::Item goddess = SP_NPC_GODDESS;
    int voice = mVoiceType;
    bool same = dNpcInfo::getSpVoice(&goddess) == voice;
    return !same;
}

// .text:0x3574
void daStrFountain_c::afterCreateMdl() {
    mAllocator_c *allocator = dStrm_c::getAllocator();
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl(0);
    dItem::Item goddess = SP_NPC_GODDESS;
    mVoiceType = dNpcInfo::getSpVoice(&goddess);
    if (mResFile.GetResAnmClrNumEntries() != 0) {
        mAnmClr.create(mdl, mResFile.GetResAnmClr(0), allocator, NULL, 1);
        mAnmClr.setPlayMode(m3d::FORWARD_LOOP, 0);
        mMdl.setAnm(mAnmClr);
    }
    if (mResFile.GetResAnmTexPatNumEntries() != 0) {
        mAnmTexPat.create(mdl, mResFile.GetResAnmTexPat(0), allocator, NULL, 1);
        mAnmTexPat.setPlayMode(m3d::FORWARD_LOOP, 0);
        mMdl.setAnm(mAnmTexPat);
    }
    if (mResFile.GetResAnmTexSrtNumEntries() != 0) {
        mAnmTexSrt.create(mdl, mResFile.GetResAnmTexSrt(0), allocator, NULL, 1);
        mAnmTexSrt.setPlayMode(m3d::FORWARD_LOOP, 0);
        mMdl.setAnm(mAnmTexSrt);
    }
}

// .text:0x3728
int daStrFountain_c::execute() {
    if (!isCulled()) {
        mEffect.create("af_obj_fountain", &mPos, NULL, NULL, 0);
    } else {
        static mVec3_c l_culledPos(0.0f, 0.0f, 0.0f);
        mEffect.create("af_obj_fountain", &l_culledPos, NULL, NULL, 0);
    }
    if (dPlayerMgr::isAxeInFountain() && !isGoddessHere()) {
        clearDemoState();
        setDemoFlag8();
        dDemo::requestSpeak(this, NULL);
    }
    mVec3_c sndPos = mPos;
    sndPos.x += 16.0f;
    sndPos.z += 16.0f;
    mSnd.setPos(&sndPos);
    mSnd.holdSound(SE_LEV_funsui_mura);
    executeState();
    if (mAnmClr.IsBound()) {
        mAnmClr.play();
    }
    if (mAnmTexPat.IsBound()) {
        mAnmTexPat.play();
    }
    if (mAnmTexSrt.IsBound()) {
        mAnmTexSrt.play();
    }
    return SUCCEEDED;
}

// .text:0x38DC
int daStrFountain_c::doDelete() {
    if (mAnmClr.IsBound()) {
        mAnmClr.remove();
    }
    if (mAnmTexPat.IsBound()) {
        mAnmTexPat.remove();
    }
    if (mAnmTexSrt.IsBound()) {
        mAnmTexSrt.remove();
    }
    return SUCCEEDED;
}

// .text:0x3974
BOOL daStrFountain_c::isGoddessHere() const {
    int num = dNpcList::countSp();
    for (int i = 0; i < num; i++) {
        dDemoActor_c *npc = dNpcList::getSp(i);
        if (npc != NULL && npc->mProfName == fProf::A_NPC_SP_GODDESS) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text:0x39E4
void daStrFountain_c::onDemoCmd(int cmd, u8) {
    switch (cmd) {
    case DEMO_CMD_TALK:
    case DEMO_CMD_SPEAK:
        setState(STATE_ITEM);
        break;
    case DEMO_CMD_END:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
            PRIVATE_FLAG1_FOUNTAIN_AXE);
        setState(STATE_WAIT);
        dCamera_c::sInstance->mRequestedMode =
            dCamera_c::sInstance->mDefaultMode;
        break;
    }
}

static BOOL (daStrFountain_c::*const
                 l_fountainStateInit[daStrFountain_c::STATE_NUM])() = {
    &daStrFountain_c::initWait,     &daStrFountain_c::initItem,
    &daStrFountain_c::initItemWait, &daStrFountain_c::initTalk,
    &daStrFountain_c::initTalkEnd,
};

static void (daStrFountain_c::*const
                 l_fountainStateExecute[daStrFountain_c::STATE_NUM])() = {
    &daStrFountain_c::executeWait,     &daStrFountain_c::executeItem,
    &daStrFountain_c::executeItemWait, &daStrFountain_c::executeTalk,
    &daStrFountain_c::executeTalkEnd,
};

// .text:0x3A60
BOOL daStrFountain_c::setState(int state) {
    if (state < STATE_NUM && (this->*l_fountainStateInit[state])()) {
        mFountainState = state;
        return TRUE;
    }
    return FALSE;
}

// .text:0x3ACC
void daStrFountain_c::executeState() {
    if (mFountainState < STATE_NUM) {
        (this->*l_fountainStateExecute[mFountainState])();
    }
}

// .text:0x3B0C
BOOL daStrFountain_c::initWait() {
    return TRUE;
}

// .text:0x3B14
void daStrFountain_c::executeWait() {
}

// .text:0x3B18
BOOL daStrFountain_c::initItem() {
    return dPlayerMgr::receiveGoddessItem();
}

// .text:0x3B1C
void daStrFountain_c::executeItem() {
    setState(STATE_ITEM_WAIT);
}

// .text:0x3B24
BOOL daStrFountain_c::initItemWait() {
    return TRUE;
}

// .text:0x3B2C
void daStrFountain_c::executeItemWait() {
    if (dPlayerMgr::isWaitingForDemo(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        setState(STATE_TALK);
    }
}

// .text:0x3B70
BOOL daStrFountain_c::initTalk() {
    attachDemoActor(this);
    setMessageLabel("npc_SP/NPC_goddess");
    setMessageCode(MSG_GODDESS_AWAY);
    dHmnName::Word_c word;
    dItem::Item goddess = SP_NPC_GODDESS;
    dNpc::getNpcName(&word, &goddess, LANGUAGE_NUM);
    setSpeakerName(
        (const u16 *)static_cast<dScript::Word_c &>(word).getBuffer(), 1);
    mpController->requestStart(0);
    return TRUE;
}

// .text:0x3C30
void daStrFountain_c::executeTalk() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (!comp->isState(&dMsg::Comp_c::executeHidden)) {
            setState(STATE_TALK_END);
        }
    }
}

// .text:0x3CA4
BOOL daStrFountain_c::initTalkEnd() {
    return TRUE;
}

// .text:0x3CAC
void daStrFountain_c::executeTalkEnd() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (comp->isState(&dMsg::Comp_c::executeHidden)) {
            detachDemoActor(this);
            dDemo::finishTalk(this);
        }
    }
}

// ---------------------------------------------------------------------------
// The carnival's stand (daStrCurnival_c, profile STR_CURNIVAL)
// ---------------------------------------------------------------------------

// .text:0x3D34
namespace dAStrField {
BASE_PROFILE(STR_CURNIVAL, daStrCurnival_c)
} // namespace dAStrField

// .text:0x3D90
void daStrCurnival_c::afterCreateMdl() {
}

// .text:0x3D94
int daStrCurnival_c::execute() {
    return SUCCEEDED;
}

// .text:0x3D9C
int daStrCurnival_c::doDelete() {
    return SUCCEEDED;
}
