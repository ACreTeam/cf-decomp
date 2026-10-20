// The bus stop and the bus (REL d_a_str_commonNP). See
// include/game/game/d_a_str_common.hpp.
// REL .text 0xCC..0x3CB4, .ctors 0x0..0x4, .rodata 0x0..0x364, .data
// 0x0..0x7D8, .bss 0x8..0xBC.
#include <game/game/d_a_str_common.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_npc.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_a_snowman.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_def.hpp>
#include <game/game/d_npc_list.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_save_building.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_str_manager.hpp>
#include <game/game/d_weather.hpp>
#include <game/game/d_world.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/sLib/s_lib.hpp>
#include <lib/egg/math/eggMath.h>

// The player faces the stop's sign (canInteract).
#define SIGN_TALK_ANGLE 0x1200
// The player faces the arriving bus before looking at it (STATE_CALL).
#define BUS_LOOK_ANGLE 0x200
// The wipers' phase step per frame (executeDoorState).
#define WIPER_SPEED 0x220
// requestBusState / setBusState: keep the animation's frame.
#define BUS_FRAME_KEEP -1

// Debug parameters (the HIO tool's values).
class hostIO_c {
  public:
    hostIO_c() {
        mWalkWait = 60;
        mOpenWait = 60;
        mGetOffWait = 5;
        _06 = 0;
        mDepartWait = 1;
        _07 = 0;
        mSceneWait = 119;
        mArriveFrame = 120;
    }

    /* 0x00 */ s16 mWalkWait;   // STATE_WALK: frames before the player walks
    /* 0x02 */ s16 mOpenWait;   // STATE_CALL: frames after the door opened
    /* 0x04 */ s16 mGetOffWait; // STATE_GET_OFF: frames after the door opened
    /* 0x06 */ u8 _06;
    /* 0x07 */ u8 _07;
    /* 0x08 */ u8 mDepartWait;  // frames before the bus starts
    /* 0x09 */ u8 mSceneWait;   // STATE_DEPART: frames before the scene change
    /* 0x0A */ u8 mArriveFrame; // the arrive frame the player gets off at

    virtual ~hostIO_c() {
    }
}; // size 0x10

// Defined before the bus code (see notes/TUs/d_a_str_common.txt).
extern u8 l_getOff;
extern hostIO_c l_hostIO;

// .text:0xCC
namespace dAStrCommon {
BASE_PROFILE(STR_BUS_STOP, daStrBusStop_c)

const f32 cBusWallScaleMin = 0.8f;
} // namespace dAStrCommon

// .text:0x5DC
void daStrBusStop_c::updateBusWall() {
    mVec3_c min;
    mVec3_c max;
    if (dSTR::getBusStopBox(&min, &max)) {
        mVec3_c center = (min + max) * 0.5f;
        center.y = min.y;
        if (!mBusWall.mActive) {
            mVec3_c size = max - min;
            dBGCF::getMvbgList()->add(size.x, size.z, size.y, &mBusWall, center,
                                      mAng(0), NULL);
        } else {
            mVec3_c scale(mBusWallScale, mBusWallScale, mBusWallScale);
            dBGCF::getMvbgList()->calc(&mBusWall, center, mAng(0), scale);
        }
    }
}

// .text:0x6C0
void daStrBusStop_c::addWalls() {
    static const f32 cWallHeight = 20.8f;
    if (dScene::getCurrentScene() == dScene::SCENE_FIELD) {
        mBusWallOn = FALSE;
        mBusWallScale = dAStrCommon::cBusWallScaleMin;
        mVec3_c backPos = mPos;
        backPos.z -= 16.0f;
        dBGCF::getMvbgList()->add(32.0f, 0.0f, cWallHeight, &mBackWall,
                                  backPos);
        mVec3_c sidePos = mPos;
        sidePos.x += 16.0f;
        dBGCF::getMvbgList()->add(0.0f, 32.0f, cWallHeight, &mSideWall,
                                  sidePos);
        updateBusWall();
    }
}

// .text:0x7B4
void daStrBusStop_c::executeWalls() {
    if (dScene::getCurrentScene() == dScene::SCENE_FIELD) {
        if (mBusWallOn) {
            sLib::chase(&mBusWallScale, 1.0f, 0.01f);
            updateBusWall();
        } else {
            sLib::chase(&mBusWallScale, dAStrCommon::cBusWallScaleMin, 0.01f);
            if (mBusWall.mActive) {
                dBGCF::getMvbgList()->remove(&mBusWall);
            }
        }
    }
}

// .text:0x848
void daStrBusStop_c::removeWalls() {
    if (dScene::getCurrentScene() == dScene::SCENE_FIELD) {
        if (mBackWall.mActive) {
            dBGCF::getMvbgList()->remove(&mBackWall);
        }
        if (mSideWall.mActive) {
            dBGCF::getMvbgList()->remove(&mSideWall);
        }
        if (mBusWall.mActive) {
            dBGCF::getMvbgList()->remove(&mBusWall);
        }
    }
}

// .text:0x8C4
int daStrBusStop_c::getVoiceMode() {
    return mVoiceType != 0;
}

// .text:0x8D8
void daStrBusStop_c::drawMdl() {
    daSTR_c::drawMdl();
}

// .text:0x8DC
void daStrBusStop_c::onAnswer(int) {
    dMsg::Comp_c *comp = mpController;
    int answer = comp != NULL && comp->getSelect() != NULL
                     ? comp->getSelect()->mCursor
                     : -1;
    switch (answer) {
    case ANSWER_RIDE:
        dStrm_c::sDoorEntered = TRUE;
        setState(STATE_WALK);
        break;
    case ANSWER_CANCEL:
        break;
    }
}

// .text:0x938
void daStrBusStop_c::afterCreateMdl() {
    dFdBase_c *field = getCurrentField();
    if (field != NULL) {
        dItem::Item unit = dItem::ITEM_ID_STR_UNIT;
        field->setItem(&unit, mUnitX, mUnitZ, 0);
    }
    mMdl.setPriorityDraw(2, 9);
    mAng angle = dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)
                     ? DEG_TO_ANGLE(180)
                     : 0;
    addWalls();
    mVec3_c acpos = mPos;
    acpos.x -= 96.0f;
    acpos.y -= 16.0f;
    mAcPos.set(this, acpos, 19.2f, 64.0f, dObjc::TYPE_BUS_STOP, dObjc::TYPE_NPC,
               0xFF, 0);
    mVec3_c offset(-32.0f, 0.0f, -40.0f);
    offset.rotY(angle);
    mVec3_c busPos = mPos;
    busPos.x += offset.x;
    busPos.y += offset.y;
    busPos.z += offset.z;
    busPos.y = dBGCF::getUnitBaseY(&busPos);
    BOOL town = dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN);
    u32 param = BUILDING_INDEX(BUILDING_CITY_BUS);
    if (town) {
        param = BUILDING_INDEX(BUILDING_BUS);
    }
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)) {
        busPos.z += 4.0f;
    }
    dActor_c::construct(fProf::STR_BUS, this, param, &busPos, NULL);
    sInstance = this;
    if (dSTR::getBusInfo()->mType != 0) {
        setState(STATE_ARRIVE);
    }
}

// .text:0xB08
int daStrBusStop_c::execute() {
    u8 scene = dScene::getCurrentScene();
    if (scene == dScene::SCENE_FIELD || scene == dScene::SCENE_DM_TITLE) {
        mAcPos.regist();
    }
    executeState();
    executeWalls();
    return SUCCEEDED;
}

// .text:0xB60
int daStrBusStop_c::doDelete() {
    removeWalls();
    if (mHoldStart && dDemo::isHoldStart()) {
        dDemo::offHoldStart();
        mHoldStart = FALSE;
        stopIntroBgm();
    }
    sInstance = NULL;
    return SUCCEEDED;
}

// .text:0xBC8
bool daStrBusStop_c::canInteract(dDemoActor_c *actor) {
    static const f32 cTalkRange = 35.2f;
    clearDemoState();
    clearDemoFlag8();
    if (mStopState == STATE_WAIT && actor != NULL) {
        mVec3_c diff = mPos - actor->getPos();
        if (EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z) <
            cTalkRange) {
            int unitX;
            int unitZ;
            dBGCF::posToUnit(&unitX, &unitZ, &actor->getPos());
            if (unitZ == mUnitZ) {
                s16 angle =
                    dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)
                        ? DEG_TO_ANGLE(180)
                        : 0;
                s16 face = angle - DEG_TO_ANGLE(90);
                if (sLib::distanceAngle(face + DEG_TO_ANGLE(180),
                                        actor->getAngle().y) <
                    SIGN_TALK_ANGLE) {
                    return true;
                }
            }
        }
    }
    return false;
}

// .text:0xCC8
void daStrBusStop_c::onDemoCmd(int cmd, u8) {
    switch (cmd) {
    case DEMO_CMD_TALK:
        setState(STATE_TALK);
        break;
    case DEMO_CMD_END:
        setState(STATE_WAIT);
        break;
    }
}

static BOOL (
    daStrBusStop_c::*const l_stopStateInit[daStrBusStop_c::STATE_NUM])() = {
    &daStrBusStop_c::initWait,         &daStrBusStop_c::initTalk,
    &daStrBusStop_c::initTalkEnd,      &daStrBusStop_c::initWalk,
    &daStrBusStop_c::initCall,         &daStrBusStop_c::initPutAway,
    &daStrBusStop_c::initBoard,        &daStrBusStop_c::initBoarded,
    &daStrBusStop_c::initDepart,       &daStrBusStop_c::initArrive,
    &daStrBusStop_c::initGetOff,       &daStrBusStop_c::initLeave,
    &daStrBusStop_c::initGuideTalk,    &daStrBusStop_c::initGuideTurn,
    &daStrBusStop_c::initGuideTalkEnd, &daStrBusStop_c::initDriveAway,
    &daStrBusStop_c::initMapTipTurn,   &daStrBusStop_c::initMapTip,
    &daStrBusStop_c::initMapTipEnd,
};

static void (
    daStrBusStop_c::*const l_stopStateExecute[daStrBusStop_c::STATE_NUM])() = {
    &daStrBusStop_c::executeWait,         &daStrBusStop_c::executeTalk,
    &daStrBusStop_c::executeTalkEnd,      &daStrBusStop_c::executeWalk,
    &daStrBusStop_c::executeCall,         &daStrBusStop_c::executePutAway,
    &daStrBusStop_c::executeBoard,        &daStrBusStop_c::executeBoarded,
    &daStrBusStop_c::executeDepart,       &daStrBusStop_c::executeArrive,
    &daStrBusStop_c::executeGetOff,       &daStrBusStop_c::executeLeave,
    &daStrBusStop_c::executeGuideTalk,    &daStrBusStop_c::executeGuideTurn,
    &daStrBusStop_c::executeGuideTalkEnd, &daStrBusStop_c::executeDriveAway,
    &daStrBusStop_c::executeMapTipTurn,   &daStrBusStop_c::executeMapTip,
    &daStrBusStop_c::executeMapTipEnd,
};

// .text:0xCF0
BOOL daStrBusStop_c::setState(int state) {
    if (state < STATE_NUM && (this->*l_stopStateInit[state])()) {
        mStopState = state;
        return TRUE;
    }
    return FALSE;
}

// .text:0xD5C
void daStrBusStop_c::executeState() {
    if (mStopState < STATE_NUM) {
        (this->*l_stopStateExecute[mStopState])();
    }
}

// .text:0xD9C
BOOL daStrBusStop_c::initWait() {
    return TRUE;
}

// .text:0xDA4
void daStrBusStop_c::executeWait() {
}

// .text:0xDA8
BOOL daStrBusStop_c::initTalk() {
    attachDemoActor(this);
    setMessageLabel("npc_SP/NPC_obj");
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player->isFlag0(PRIVATE_FLAG0_PART_TIME_JOB) ||
        dNet::isSessionActive() || player->isFlag0(PRIVATE_FLAG0_TRAVELLING)) {
        setMessageCode(MSG_NOT_COMING);
    } else if (!checkNpc()) {
        setMessageCode(MSG_ROUTE_BLOCKED);
        mBusWallOn = FALSE;
    } else if (!checkSnowBall()) {
        setMessageCode(MSG_SNOWBALL);
        mBusWallOn = FALSE;
    } else if (dScene::getCurrentScene() == dScene::SCENE_FIELD) {
        setMessageCode(MSG_TO_CITY);
        mBusWallOn = TRUE;
    } else {
        setMessageCode(MSG_TO_TOWN);
        mBusWallOn = FALSE;
    }
    mpController->requestStart(0);
    return TRUE;
}

// .text:0xED8
void daStrBusStop_c::executeTalk() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (!comp->isState(&dMsg::Comp_c::executeHidden)) {
            setState(STATE_TALK_END);
        }
    }
}

// .text:0xF4C
BOOL daStrBusStop_c::initTalkEnd() {
    return TRUE;
}

// .text:0xF54
void daStrBusStop_c::executeTalkEnd() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (comp->isState(&dMsg::Comp_c::executeHidden)) {
            detachDemoActor(this);
            dDemo::finishTalk(this);
            mBusWallOn = FALSE;
        }
    }
}

// .text:0xFE4
BOOL daStrBusStop_c::initWalk() {
    mWalkWait = 0;
    dAcPlayer_c *player =
        dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (player != NULL) {
        s16 angle = dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)
                        ? DEG_TO_ANGLE(180)
                        : 0;
        mVec3_c offset(-32.0f, 0.0f, 0.0f);
        offset.rotY(angle);
        mVec3_c target = mPos + offset;
        mVec3_c diff = target - player->getPos();
        if (EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z) >= 2.0f) {
            if (dPlayerMgr::moveTo(&target, dPlayerMgr::PLAYER_INDEX_CURRENT,
                                   dPlayerMgr::sDefaultMoveSpeed)) {
                setDemoFlag8();
                mWalkStep = WALK_CHECK_TOOL;
                return TRUE;
            }
            return FALSE;
        }
    }
    return TRUE;
}

// .text:0x1110
void daStrBusStop_c::executeWalk() {
    if (mWalkWait < l_hostIO.mWalkWait) {
        mWalkWait++;
    } else if (mWalkWait == l_hostIO.mWalkWait &&
               !dPlayerMgr::isMovingTo(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        switch (mWalkStep) {
        case WALK_CHECK_TOOL: {
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
            dItem::Item held =
                player != NULL ? player->mEquipment.mHeld : dItem::Item();
            int kind = held.getKind();
            if (kind == dItem::KIND_ORG_UMB || kind == dItem::KIND_UMBRELLA) {
                mWalkStep = WALK_DONE;
            } else {
                dPlayerMgr::putAwayToolForTalk(0);
                mWalkStep = WALK_PUT_AWAY;
            }
            break;
        }
        case WALK_PUT_AWAY:
            if (dPlayerMgr::canFinishToolAction()) {
                mWalkStep = WALK_DONE;
            }
            break;
        case WALK_DONE: {
            BOOL show = FALSE;
            if (dSaveData_c::getRaw()->isBusNetEventPending() &&
                dScene::getCurrentScene() != dScene::SCENE_TOWN) {
                show = TRUE;
            }
            nw4r::g3d::ResNode node =
                daStrBus_c::getResFile().GetResMdl(0).GetResNode("disp_xct");
            node.SetVisibility(show);
            setState(STATE_CALL);
            break;
        }
        }
    }
}

// .text:0x12B8
BOOL daStrBusStop_c::initCall() {
    if (daStrBus_c::requestBusState(daStrBus_c::BUS_STATE_ARRIVE,
                                    BUS_FRAME_KEEP)) {
        l_getOff = FALSE;
        mOpenWait = 0;
        setDemoFlag8();
        if (dPlayerMgr::turnToInFrames(
                dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)
                    ? DEG_TO_ANGLE(180)
                    : 0,
                800, dPlayerMgr::PLAYER_INDEX_CURRENT)) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text:0x1350
void daStrBusStop_c::executeCall() {
    s16 angle = dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)
                    ? DEG_TO_ANGLE(180)
                    : 0;
    if (sLib::distanceAngle(
            angle, dPlayerMgr::getYaw(dPlayerMgr::PLAYER_INDEX_CURRENT)) <
        BUS_LOOK_ANGLE) {
        dPlayerMgr::lookAt(daStrBus_c::getMainPos(TRUE), 0x7F, FALSE, 0xFF);
    }
    if (daStrBus_c::getBusState() == daStrBus_c::BUS_STATE_ARRIVED) {
        dPlayerMgr::lookAt(NULL, 0x7F, FALSE, 0xFF);
        daStrBus_c::requestBusState(daStrBus_c::BUS_STATE_OPEN, BUS_FRAME_KEEP);
    } else if (daStrBus_c::getBusState() == daStrBus_c::BUS_STATE_OPENED) {
        if (mOpenWait < l_hostIO.mOpenWait) {
            mOpenWait++;
        }
        if (mOpenWait == l_hostIO.mOpenWait) {
            setState(STATE_PUT_AWAY);
        }
    }
}

// .text:0x1460
BOOL daStrBusStop_c::initPutAway() {
    dPlayerMgr::putAwayToolForTalk(0);
    return TRUE;
}

// .text:0x1488
void daStrBusStop_c::executePutAway() {
    if (dPlayerMgr::canFinishToolAction()) {
        setState(STATE_BOARD);
    }
}

// .text:0x14C8
BOOL daStrBusStop_c::initBoard() {
    if (dPlayerMgr::boardBus(
            dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)
                ? DEG_TO_ANGLE(180)
                : 0) &&
        daStrBus_c::requestBusState(daStrBus_c::BUS_STATE_PLAYER_IN,
                                    BUS_FRAME_KEEP)) {
        return TRUE;
    }
    return FALSE;
}

// .text:0x1534
void daStrBusStop_c::executeBoard() {
    if (dPlayerMgr::isOnBus()) {
        if (daStrBus_c::getBusState() == daStrBus_c::BUS_STATE_OPENED) {
            daStrBus_c::requestBusState(daStrBus_c::BUS_STATE_CLOSE,
                                        BUS_FRAME_KEEP);
        } else if (daStrBus_c::getBusState() == daStrBus_c::BUS_STATE_CLOSED) {
            setState(STATE_BOARDED);
        }
    }
}

// .text:0x15A4
BOOL daStrBusStop_c::initBoarded() {
    return TRUE;
}

// .text:0x15AC
void daStrBusStop_c::executeBoarded() {
    setState(STATE_DEPART);
}

// .text:0x15B4
BOOL daStrBusStop_c::initDepart() {
    mDepartWait = l_hostIO.mDepartWait;
    daStrBus_c::startBusSound(SE_BusOut);
    return TRUE;
}

// .text:0x15EC
void daStrBusStop_c::executeDepart() {
    if (mDepartWait != 0) {
        mDepartWait--;
    }
    if (mDepartWait == 0) {
        if (daStrBus_c::getBusState() == daStrBus_c::BUS_STATE_CLOSED) {
            dPlayerMgr::rideBus();
            daStrBus_c::requestBusState(daStrBus_c::BUS_STATE_DEPART,
                                        BUS_FRAME_KEEP);
            mSceneWait = l_hostIO.mSceneWait;
        } else if (daStrBus_c::getBusState() == daStrBus_c::BUS_STATE_DEPART) {
            if (mSceneWait >= 0) {
                mSceneWait--;
            }
            if (mSceneWait == 0) {
                mBusWallOn = FALSE;
                if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)) {
                    dScene::requestSceneChange(dScene::SCENE_DM_BUS_TO_TOWN, 4,
                                               0);
                } else {
                    dScene::requestSceneChange(dScene::SCENE_DM_BUS_TO_LAND, 3,
                                               0);
                }
            }
        } else if (daStrBus_c::getBusState() == daStrBus_c::BUS_STATE_GONE) {
            setState(STATE_TALK_END);
        }
    }
}

// The player arrives in the city for the first time (no control until the
// camera is ready).
static inline BOOL isFirstCityVisit() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    BOOL first = FALSE;
    if (dScene::getCurrentScene() == dScene::SCENE_TOWN && player != NULL &&
        !player->isFlag0(PRIVATE_FLAG0_CITY_VISITED)) {
        first = TRUE;
    }
    return first;
}

// .text:0x16F4
BOOL daStrBusStop_c::initArrive() {
    if (!dDemo::isHoldStart()) {
        mHoldStart = TRUE;
        dDemo::onHoldStart();
    }
    return TRUE;
}

// .text:0x1738
void daStrBusStop_c::executeArrive() {
    if (isFirstCityVisit()) {
        if (dCamera_c::sInstance != NULL &&
            dCamera_c::sInstance->isControllable()) {
            mFirstCity = TRUE;
            setState(STATE_GET_OFF);
        }
    } else {
        setState(STATE_GET_OFF);
    }
}

// .text:0x17F4
BOOL daStrBusStop_c::initGetOff() {
    BOOL first;
    s16 angle = dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)
                    ? DEG_TO_ANGLE(180)
                    : 0;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    first = FALSE;
    if (dScene::getCurrentScene() == dScene::SCENE_TOWN && player != NULL &&
        !player->isFlag0(PRIVATE_FLAG0_CITY_VISITED)) {
        first = TRUE;
    }
    f32 frame = first ? 0.0f : l_hostIO.mArriveFrame;
    if (dScene::getCurrentScene() != dScene::SCENE_DM_TITLE &&
        dPlayerMgr::getOffBus(angle, frame) &&
        daStrBus_c::requestBusState(daStrBus_c::BUS_STATE_ARRIVE, frame)) {
        l_getOff = TRUE;
        mIntroRide = dSTR::getBusInfo()->mType == 1;
        dSTR::busInfo_c *info = dSTR::getBusInfo();
        info->mType = 0;
        dSTR::getBusInfo(); // result not used
        nw4r::g3d::ResNode node =
            daStrBus_c::getResFile().GetResMdl(0).GetResNode("disp_xct");
        node.SetVisibility(dSTR::getBusInfo()->mShowDisp);
        playIntroBgm();
        mGetOffWait = 0;
        if (mFirstCity) {
            l_getOff = FALSE;
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_CITY_VISITED);
        }
        return TRUE;
    }
    return FALSE;
}

// .text:0x1A08
void daStrBusStop_c::executeGetOff() {
    if (daStrBus_c::getBusState() == daStrBus_c::BUS_STATE_ARRIVED &&
        dPlayerMgr::isOffBus()) {
        daStrBus_c::requestBusState(daStrBus_c::BUS_STATE_OPEN, BUS_FRAME_KEEP);
    } else if (daStrBus_c::getBusState() == daStrBus_c::BUS_STATE_OPENED) {
        if (mGetOffWait < l_hostIO.mGetOffWait) {
            mGetOffWait++;
        }
        if (mGetOffWait == l_hostIO.mGetOffWait) {
            setState(STATE_LEAVE);
        }
    } else if (daStrBus_c::getBusState() == daStrBus_c::BUS_STATE_ARRIVE) {
        if (mFirstCity && dPlayerMgr::enableCurrentActor()) {
            mFirstCity = FALSE;
        }
    }
}

// .text:0x1AE4
BOOL daStrBusStop_c::initLeave() {
    if (daStrBus_c::requestBusState(daStrBus_c::BUS_STATE_PLAYER_OUT,
                                    BUS_FRAME_KEEP) &&
        dPlayerMgr::leaveBus()) {
        return TRUE;
    }
    return FALSE;
}

// .text:0x1B2C
void daStrBusStop_c::executeLeave() {
    if (dPlayerMgr::isWaitingForDemo(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        if (mIntroRide) {
            setState(STATE_GUIDE_TALK);
        } else {
            setState(STATE_DRIVE_AWAY);
        }
    }
}

// .text:0x1B8C
BOOL daStrBusStop_c::initGuideTalk() {
    dMsg::Comp_c *comp = dMsg::Comp_c::mInstance;
    if (comp != NULL) {
        setMessageLabel("sys_MESS/SYS_Player");
        setMessageCode(MSG_GO_TO_TOWN_HALL);
        comp->attachActor(this);
        dHmnName::Word_c word;
        dItem::Item guide = SP_NPC_GUIDE;
        dNpc::getNpcName(&word, &guide, LANGUAGE_NUM);
        setSpeakerName(
            (const u16 *)static_cast<dScript::Word_c &>(word).getBuffer(), 2);
        mVoiceType = 0;
        comp->requestStart(0);
        mTalkWait = 10;
        return TRUE;
    }
    return FALSE;
}

// .text:0x1C7C
void daStrBusStop_c::executeGuideTalk() {
    if (mTalkWait != 0) {
        mTalkWait--;
    }
    if (mTalkWait == 0) {
        setState(STATE_GUIDE_TURN);
    }
}

// .text:0x1CA8
BOOL daStrBusStop_c::initGuideTurn() {
    return dPlayerMgr::turnToInFrames(-0x6C00, 2000,
                                      dPlayerMgr::PLAYER_INDEX_CURRENT);
}

// .text:0x1CB8
void daStrBusStop_c::executeGuideTurn() {
    dMsg::Comp_c *comp = dMsg::Comp_c::mInstance;
    if (comp != NULL) {
        if (!comp->isState(&dMsg::Comp_c::executeHidden)) {
            if (!dPlayerMgr::isTurningTo(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
                setState(STATE_GUIDE_TALK_END);
            }
        }
    }
}

// .text:0x1D40
BOOL daStrBusStop_c::initGuideTalkEnd() {
    mTalkWait = 60;
    return TRUE;
}

// .text:0x1D50
void daStrBusStop_c::executeGuideTalkEnd() {
    dMsg::Comp_c *comp = dMsg::Comp_c::mInstance;
    if (comp != NULL) {
        if (comp->isState(&dMsg::Comp_c::executeHidden)) {
            if (mTalkWait != 0) {
                mTalkWait--;
            }
            if (mTalkWait == 0 &&
                dPlayerMgr::turnToInFrames(0x7FFF, 300,
                                           dPlayerMgr::PLAYER_INDEX_CURRENT) &&
                setState(STATE_DRIVE_AWAY)) {
                comp->detachActor();
                mVoiceType = 13;
            }
        }
    }
}

// .text:0x1E20
BOOL daStrBusStop_c::initDriveAway() {
    mDepartWait = l_hostIO.mDepartWait;
    return TRUE;
}

// .text:0x1E38
void daStrBusStop_c::executeDriveAway() {
    if (daStrBus_c::getBusState() == daStrBus_c::BUS_STATE_OPENED) {
        daStrBus_c::requestBusState(daStrBus_c::BUS_STATE_CLOSE,
                                    BUS_FRAME_KEEP);
    } else if (daStrBus_c::getBusState() == daStrBus_c::BUS_STATE_CLOSED) {
        if (mDepartWait == l_hostIO.mDepartWait) {
            daStrBus_c::startBusSound(SE_BusOut);
        }
        if (mDepartWait != 0) {
            mDepartWait--;
        }
        if (mDepartWait == 0) {
            daStrBus_c::requestBusState(daStrBus_c::BUS_STATE_DEPART,
                                        BUS_FRAME_KEEP);
        }
    } else if (daStrBus_c::getBusState() == daStrBus_c::BUS_STATE_DEPART) {
        if (mIntroRide) {
            dPlayerMgr::lookAt(daStrBus_c::getMainPos(TRUE), 0x7F, FALSE, 0xFF);
        }
    } else if (daStrBus_c::getBusState() == daStrBus_c::BUS_STATE_GONE) {
        if (mIntroRide) {
            setState(STATE_MAP_TIP_TURN);
        } else if (setState(STATE_WAIT) && dDemo::isHoldStart()) {
            dDemo::offHoldStart();
            mHoldStart = FALSE;
            daStrBus_c::resetCount();
            stopIntroBgm();
        }
    }
}

// .text:0x1F7C
BOOL daStrBusStop_c::initMapTipTurn() {
    dPlayerMgr::lookAt(NULL, 600, FALSE, 0xFF);
    dPlayerMgr::turnToInFrames(0, 1100, dPlayerMgr::PLAYER_INDEX_CURRENT);
    return TRUE;
}

// .text:0x1FC0
void daStrBusStop_c::executeMapTipTurn() {
    if (!dPlayerMgr::isTurningTo(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        setState(STATE_MAP_TIP);
    }
}

// .text:0x2004
BOOL daStrBusStop_c::initMapTip() {
    dMsg::Comp_c *comp = dMsg::Comp_c::mInstance;
    if (comp != NULL) {
        setMessageLabel("sys_MESS/SYS_Player");
        setMessageCode(MSG_MAP_TIP);
        comp->attachActor(this);
        dHmnName::Word_c word;
        setSpeakerName(
            (const u16 *)static_cast<dScript::Word_c &>(word).getBuffer(), 0);
        comp->requestStart(0);
        return TRUE;
    }
    return FALSE;
}

// .text:0x20C8
void daStrBusStop_c::executeMapTip() {
    dMsg::Comp_c *comp = dMsg::Comp_c::mInstance;
    if (comp != NULL) {
        if (!comp->isState(&dMsg::Comp_c::executeHidden)) {
            setState(STATE_MAP_TIP_END);
        }
    }
}

// .text:0x2140
BOOL daStrBusStop_c::initMapTipEnd() {
    return TRUE;
}

// .text:0x2148
void daStrBusStop_c::executeMapTipEnd() {
    dMsg::Comp_c *comp = dMsg::Comp_c::mInstance;
    if (comp != NULL) {
        if (comp->isState(&dMsg::Comp_c::executeHidden)) {
            if (setState(STATE_WAIT)) {
                comp->detachActor();
                if (dDemo::isHoldStart()) {
                    dDemo::offHoldStart();
                    daStrBus_c::resetCount();
                    stopIntroBgm();
                }
            }
        }
    }
}

// .text:0x21F4: no snowball is on the bus stop's ground.
BOOL daStrBusStop_c::checkSnowBall() {
    for (int i = 0; i < daSnowMan_c::BALL_NUM; i++) {
        daSnowMan_c *ball = daSnowMan_c::sBall[i];
        if (ball != NULL) {
            int unitX;
            int unitZ;
            dBGCF::posToUnit(&unitX, &unitZ, &ball->getPos());
            if (dBGCF::getAttr(unitX, unitZ) == BG_ATTR_STONE_BUS) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

// .text:0x2278: no villager or special npc stands on the bus stop's ground.
BOOL daStrBusStop_c::checkNpc() {
    for (int i = 0; i < dNpcList::countNml(); i++) {
        dAcNpc_c *npc = static_cast<dAcNpc_c *>(dNpcList::getNml(i));
        if (npc != NULL && npc->mIsActive) {
            int unitX;
            int unitZ;
            dBGCF::posToUnit(&unitX, &unitZ, &npc->getPos());
            if (dBGCF::getAttr(unitX, unitZ) == BG_ATTR_STONE_BUS) {
                return FALSE;
            }
        }
    }
    for (int i = 0; i < dNpcList::countSp(); i++) {
        dAcNpc_c *npc = static_cast<dAcNpc_c *>(dNpcList::getSp(i));
        if (npc != NULL && npc->mIsActive) {
            int unitX;
            int unitZ;
            dBGCF::posToUnit(&unitX, &unitZ, &npc->getPos());
            if (dBGCF::getAttr(unitX, unitZ) == BG_ATTR_STONE_BUS) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

// .text:0x2368: the opening ride's music.
void daStrBusStop_c::playIntroBgm() {
    if (mIntroRide) {
        dBgm::l_mgr.mStgField.play18();
    }
}

// .text:0x2388
void daStrBusStop_c::stopIntroBgm() {
    if (mIntroRide) {
        dBgm::l_mgr.mStgField.stop18();
    }
}

// The player gets off the bus (the arrive sound).
u8 l_getOff;
hostIO_c l_hostIO;
daStrBusStop_c *daStrBusStop_c::sInstance;
daStrBus_c *daStrBus_c::sInstance;

// .text:0x23A8
namespace dAStrCommon {
BASE_PROFILE(STR_BUS, daStrBus_c)
} // namespace dAStrCommon

static const char *l_anmArrive = "obj_bus_arrive";
static const char *l_anmOpen = "obj_bus_open";
static const char *l_anmClose = "obj_bus_close";
static const char *l_anmDeparture = "obj_bus_departure";
static const char *l_anmPlayerIn = "obj_bus_plyin";
static const char *l_anmPlayerOut = "obj_bus_plyout";

// .text:0x2494
void daStrBus_c::afterCreateMdl() {
    sInstance = this;
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)) {
        mAngle.y.mAngle += DEG_TO_ANGLE(180);
    }
    mMdl.setPriorityDraw(2, 9);
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl(0);
    mDoorAnmChr.create(mdl, mResFile.GetResAnmChr(l_anmOpen),
                       dStrm_c::getAllocator(), NULL);
    mDoorAnmChr.mPlayMode = m3d::FORWARD_ONCE;
    mMdl.setAnm(mDoorAnmChr);
    mNodeCallBack.mWiperL0 = m3d::getNodeID(mdl, "j_wiper_L0");
    mNodeCallBack.mWiperR0 = m3d::getNodeID(mdl, "j_wiper_R0");
    mNodeCallBack.mWiperL1 = m3d::getNodeID(mdl, "j_wiper_L1");
    mNodeCallBack.mWiperR1 = m3d::getNodeID(mdl, "j_wiper_R1");
    mMdl.setCallback(&mNodeCallBack);
}

// .text:0x25E8
int daStrBus_c::execute() {
    const mVec3_c *pos = getMainPos(TRUE);
    mSnd.setPos(pos);
    mWiperSnd.setPos(getWiperPos(TRUE));
    if (!isDraw()) {
        dSTR::gArea.clear();
    } else {
        mVec3_c min(-128.0f, -48.0f, -48.0f);
        mVec3_c max(32.0f, 25.6f, 48.0f);
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)) {
            min.rotY(DEG_TO_ANGLE(180));
            max.rotY(DEG_TO_ANGLE(180));
            mVec3_c rotMin = max;
            mVec3_c rotMax = min;
            // Rotation swaps the horizontal bounds, but not their heights.
            rotMin.y = min.y;
            rotMax.y = max.y;
            mVec3_c translatedMin = *pos + rotMin;
            mVec3_c areaMin = translatedMin;
            mVec3_c translatedMax = *pos + rotMax;
            mVec3_c areaMax = translatedMax;
            dSTR::gArea.set(areaMin, areaMax);
        } else {
            mVec3_c translatedMin = *pos + min;
            mVec3_c areaMin = translatedMin;
            mVec3_c translatedMax = *pos + max;
            mVec3_c areaMax = translatedMax;
            dSTR::gArea.set(areaMin, areaMax);
        }
    }
    if (isDraw()) {
        // empty in the target (isDraw is called and its result dropped)
    }
    if (isDraw() && mNodeCallBack.isWiping()) {
        f32 prev = mNodeCallBack.mPrevAngle.sin();
        f32 cur = mNodeCallBack.mAngle.sin();
        if (prev <= 0.0f && cur > 0.0f) {
            mWiperSnd.startSound(SE_wiper1);
        } else if (prev > 0.0f && cur <= 0.0f) {
            mWiperSnd.startSound(SE_wiper2);
        }
    }
    return SUCCEEDED;
}

// .text:0x28B8
int daStrBus_c::doDelete() {
    sInstance = NULL;
    dSTR::gArea.clear();
    return SUCCEEDED;
}

// .text:0x28F0
void daStrBus_c::drawMdl() {
    daSTR_c::drawMdl();
}

// .text:0x28F4
mMtx_c *daStrBus_c::getShadowMtx() {
    static mMtx_c mtx;
    const mVec3_c *pos = getMainPos(TRUE);
    if (!pos->isZero()) {
        dActor_c::makeMtx(&mtx, pos, 0);
        return &mtx;
    }
    return NULL;
}

// .text:0x2998
nw4r::math::VEC3 *daStrBus_c::getShadowScale() {
    static mVec3_c scale(-1.0f, 1.0f, 1.0f);
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)) {
        return &scale;
    }
    return NULL;
}

// .text:0x2A24
BOOL daStrBus_c::nodeCallBack_c::isWiping() {
    dWeather_c *weather = dWeather_c::sInstance;
    if (weather != NULL && (dWeather_c::isRainKind(weather->mWeather) ||
                            dWeather_c::isSnowKind(weather->mWeather))) {
        return TRUE;
    }
    return FALSE;
}

// .text:0x2A88
void daStrBus_c::nodeCallBack_c::timingA(ulong nodeId,
                                         nw4r::g3d::ChrAnmResult *anmRes,
                                         nw4r::g3d::ResMdl) {
    if (isWiping()) {
        f32 cos = mAngle.cos();
        if (nodeId == mWiperR0 || nodeId == mWiperL0) {
            mAng angle = 4095.0f * cos;
            mMtx_c mtx;
            anmRes->GetRotTrans(mtx);
            mtx.YrotM(angle);
            anmRes->SetRotTrans(mtx);
        } else if (nodeId == mWiperR1 || nodeId == mWiperL1) {
            mAng angle = 2730.0f * cos;
            mMtx_c mtx;
            anmRes->GetRotTrans(mtx);
            mtx.YrotM(angle);
            anmRes->SetRotTrans(mtx);
        }
    }
}

// .text:0x2BA8
BOOL daStrBus_c::requestBusState(u32 state, int frame) {
    if (sInstance != NULL) {
        return sInstance->setBusState(state, frame);
    }
    return FALSE;
}

// .text:0x2BD0
u8 daStrBus_c::getBusState() {
    if (sInstance != NULL) {
        return sInstance->mDoorState;
    }
    return BUS_STATE_HIDDEN;
}

// .text:0x2BF0
BOOL daStrBus_c::isDraw() {
    return mDoorState != BUS_STATE_HIDDEN && mDoorState != BUS_STATE_GONE;
}

// .text:0x2C10
const mVec3_c *daStrBus_c::getNodePos(const char *name, BOOL field) {
    static mVec3_c pos(0.0f, 0.0f, 0.0f);
    if (mMdl.getScn() != NULL) {
        int id = m3d::getNodeID(mMdl.getResMdl(), name);
        if (id != -1) {
            mMdl.calc(false);
            mMtx_c mtx;
            mMdl.getNodeWorldMtx(id, &mtx);
            PSMTXMultVec(mtx, mVec3_c::Zero, pos);
            if (field) {
                mVec3_c fieldPos;
                dWorld::toFieldPosition(&fieldPos, &pos);
                pos = fieldPos;
            }
        }
    }
    return &pos;
}

// .text:0x2D30
void daStrBus_c::getNodeMtx(mMtx_c *out, const char *name) {
    static mVec3_c pos(0.0f, 0.0f, 0.0f);
    if (mMdl.getScn() != NULL) {
        int id = m3d::getNodeID(mMdl.getResMdl(), name);
        if (id != -1) {
            mMdl.calc(false);
            mMtx_c mtx;
            mMdl.getNodeWorldMtx(id, &mtx);
            *out = mtx;
        }
    }
}

// .text:0x2E68
void daStrBus_c::getMainMtx(mMtx_c *mtx) {
    if (sInstance != NULL) {
        sInstance->getNodeMtx(mtx, "j_main");
    }
}

// .text:0x2E8C
const mVec3_c *daStrBus_c::getMainPos(BOOL field) {
    if (sInstance != NULL) {
        return sInstance->getNodePos("j_main", field);
    }
    return &mVec3_c::Zero;
}

// .text:0x2EB8
const mVec3_c *daStrBus_c::getWiperPos(BOOL field) {
    if (sInstance != NULL) {
        return sInstance->getNodePos("j_wiper_L0", field);
    }
    return &mVec3_c::Zero;
}

// .text:0x2EE4
void daStrBus_c::nextStateAtAnmEnd(u32 state) {
    if (mDoorAnmChr.isStop()) {
        setBusState(state, BUS_FRAME_KEEP);
    } else {
        mDoorAnmChr.play();
    }
}

// .text:0x2F50
BOOL daStrBus_c::setBusAnm(const char *name, m3d::playMode_e playMode,
                           BOOL atEnd) {
    nw4r::g3d::ResAnmChr anm = mResFile.GetResAnmChr(name);
    if (anm.IsValid()) {
        mDoorAnmChr.setAnm(mMdl, anm, playMode);
        mMdl.setAnm(mDoorAnmChr);
        if (atEnd) {
            mDoorAnmChr.setFrame(anm.ref().info.numFrame - 1);
        }
        return TRUE;
    }
    return FALSE;
}

static BOOL (daStrBus_c::*const l_busStateInit[daStrBus_c::BUS_STATE_NUM])() = {
    &daStrBus_c::initHidden,    &daStrBus_c::initArrive,
    &daStrBus_c::initArrived,   &daStrBus_c::initOpen,
    &daStrBus_c::initOpened,    &daStrBus_c::initPlayerIn,
    &daStrBus_c::initPlayerOut, &daStrBus_c::initClose,
    &daStrBus_c::initClosed,    &daStrBus_c::initDepart,
    &daStrBus_c::initGone,
};

static void (
    daStrBus_c::*const l_busStateExecute[daStrBus_c::BUS_STATE_NUM])() = {
    &daStrBus_c::executeHidden,    &daStrBus_c::executeArrive,
    &daStrBus_c::executeArrived,   &daStrBus_c::executeOpen,
    &daStrBus_c::executeOpened,    &daStrBus_c::executePlayerIn,
    &daStrBus_c::executePlayerOut, &daStrBus_c::executeClose,
    &daStrBus_c::executeClosed,    &daStrBus_c::executeDepart,
    &daStrBus_c::executeGone,
};

// .text:0x3020
BOOL daStrBus_c::setBusState(u32 state, int frame) {
    if (state < BUS_STATE_NUM && (this->*l_busStateInit[state])()) {
        if (frame != BUS_FRAME_KEEP) {
            mDoorAnmChr.setFrame(frame);
        }
        mDoorState = state;
        return TRUE;
    }
    return FALSE;
}

// .text:0x30C8
void daStrBus_c::executeDoorState() {
    if (mDoorState < BUS_STATE_NUM) {
        (this->*l_busStateExecute[mDoorState])();
    }
    mNodeCallBack.mPrevAngle.mAngle = mNodeCallBack.mAngle.mAngle;
    mNodeCallBack.mAngle.mAngle += WIPER_SPEED;
}

// .text:0x3124
BOOL daStrBus_c::initHidden() {
    return TRUE;
}

// .text:0x312C
void daStrBus_c::executeHidden() {
}

// .text:0x3130
BOOL daStrBus_c::initArrive() {
    return setBusAnm(l_anmArrive, m3d::FORWARD_ONCE, FALSE);
}

static inline BOOL isRaining() {
    return dWeather_c::sInstance != NULL
               ? dWeather_c::isRainKind(dWeather_c::sInstance->mWeather)
               : FALSE;
}

// .text:0x3144
void daStrBus_c::executeArrive() {
    static const u32 cArriveSeEnd = 105;
    // Not used (the hold sound ends at the HIO's frame); kept for .rodata.
    static const u32 cHoldSeEnd = 87;
    u16 frameNum = mResFile.GetResAnmChr(l_anmArrive).ref().info.numFrame;
    if (mDoorAnmChr.checkFrame(l_hostIO.mArriveFrame)) {
        mMtx_c mtx;
        getMainMtx(&mtx);
        mArriveEffect.create(
            isRaining() ? "af_obj_bus_arrive_rain" : "af_obj_bus_arrive", &mtx);
    } else if (mDoorAnmChr.getFrame() >= l_hostIO.mArriveFrame) {
        mMtx_c mtx;
        getMainMtx(&mtx);
        mArriveEffect.followEffect(&mtx);
    }
    if (mDoorAnmChr.getFrame() < dSTR::getHostIO()->_48) {
        if (l_getOff) {
            mSnd.holdSound(SE_BusCome_Short);
        } else {
            mSnd.holdSound(SE_BusCome);
        }
    }
    if (mDoorAnmChr.checkFrame(frameNum - cArriveSeEnd)) {
        mSnd.startSound(SE_BusStop);
    }
    nextStateAtAnmEnd(BUS_STATE_ARRIVED);
}

// .text:0x330C
BOOL daStrBus_c::initArrived() {
    return setBusAnm(l_anmArrive, m3d::FORWARD_ONCE, TRUE);
}

// .text:0x3320
void daStrBus_c::executeArrived() {
}

// .text:0x3324
BOOL daStrBus_c::initOpen() {
    if (setBusAnm(l_anmOpen, m3d::FORWARD_ONCE, FALSE)) {
        mOpenWait = 45;
        return TRUE;
    }
    return FALSE;
}

// .text:0x337C
void daStrBus_c::executeOpen() {
    if (mOpenWait != 0) {
        mOpenWait--;
    }
    if (mOpenWait == 1) {
        mSnd.startSound(SE_BusDoorOpen);
    } else if (mOpenWait == 0) {
        nextStateAtAnmEnd(BUS_STATE_OPENED);
    }
}

// .text:0x33BC
BOOL daStrBus_c::initOpened() {
    return setBusAnm(l_anmOpen, m3d::FORWARD_ONCE, TRUE);
}

// .text:0x33D0
void daStrBus_c::executeOpened() {
}

// .text:0x33D4
BOOL daStrBus_c::initPlayerIn() {
    return setBusAnm(l_anmPlayerIn, m3d::FORWARD_ONCE, FALSE);
}

// .text:0x33E8
void daStrBus_c::executePlayerIn() {
    nextStateAtAnmEnd(BUS_STATE_OPENED);
}

// .text:0x33F0
BOOL daStrBus_c::initPlayerOut() {
    return setBusAnm(l_anmPlayerOut, m3d::FORWARD_ONCE, FALSE);
}

// .text:0x3404
void daStrBus_c::executePlayerOut() {
    nextStateAtAnmEnd(BUS_STATE_OPENED);
}

// .text:0x340C
BOOL daStrBus_c::initClose() {
    mSnd.startSound(SE_BusDoorCloseSlide);
    return setBusAnm(l_anmClose, m3d::FORWARD_ONCE, FALSE);
}

// .text:0x3458
void daStrBus_c::executeClose() {
    mResFile.GetResAnmChr(l_anmClose);
    if (mDoorAnmChr.checkFrame(78.0f)) {
        mSnd.startSound(SE_BusDoorClose);
    }
    nextStateAtAnmEnd(BUS_STATE_CLOSED);
}

// .text:0x34C0
BOOL daStrBus_c::initClosed() {
    return TRUE;
}

// .text:0x34C8
void daStrBus_c::executeClosed() {
}

// .text:0x34CC
BOOL daStrBus_c::initDepart() {
    if (setBusAnm(l_anmDeparture, m3d::FORWARD_ONCE, FALSE)) {
        mMtx_c mtx;
        getMainMtx(&mtx);
        if (mDepartEffect.create(isRaining() ? "af_obj_bus_departure_rain"
                                             : "af_obj_bus_depature",
                                 &mtx)) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text:0x3588
void daStrBus_c::executeDepart() {
    mMtx_c mtx;
    getMainMtx(&mtx);
    mDepartEffect.followEffect(&mtx);
    nextStateAtAnmEnd(BUS_STATE_GONE);
}

// .text:0x35D0
BOOL daStrBus_c::initGone() {
    return TRUE;
}

// .text:0x35D8
void daStrBus_c::executeGone() {
}

// .text:0x35DC
void daStrBus_c::startBusSound(u32 id) {
    if (sInstance != NULL) {
        sInstance->mSnd.startSound(id);
    }
}

// .text:0x35FC
void daStrBus_c::resetCount() {
    dSTR::resetCount();
}
