#include <game/game/d_a_npc_sp_op_tanukichi.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_def.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>

// Tom Nook in the game opening. Functions are in target .text order.

namespace dANpcSpOpTanukichi {
// .text 0x5C
BASE_PROFILE(A_NPC_SP_OP_TANUKICHI, dAcNpcSpOpTanukichi_c);
} // namespace dANpcSpOpTanukichi

// .text 0x1D4
int dAcNpcSpOpTanukichi_c::preCreate() {
    switch (dSaveData_c::getTown()->mShops.mShop.mStage) {
    case SHOP_STAGE_NOOK_N_GO:
        mParam = SP_NPC_CONV_MASTER;
        break;
    case SHOP_STAGE_NOOKWAY:
        mParam = SP_NPC_SUPER_MASTER;
        break;
    case SHOP_STAGE_NOOKINGTONS:
        mParam = SP_NPC_DEPART_MASTER;
        break;
    }
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mOpTalk);
    return SUCCEEDED;
}

// .text 0x280
int dAcNpcSpOpTanukichi_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    static const dState::mode_c<dAcNpcSpOpTanukichi_c>::Entry modes[] = {
        {"強制会話", &dAcNpcSpOpTanukichi_c::initForcedTalk,
         &dAcNpcSpOpTanukichi_c::executeForcedTalk},
        {"デモ待機", &dAcNpcSpOpTanukichi_c::initDemoWait,
         &dAcNpcSpOpTanukichi_c::executeDemoWait},
        {"プレイヤー移動要求", &dAcNpcSpOpTanukichi_c::initPlayerMoveRequest,
         &dAcNpcSpOpTanukichi_c::executePlayerMoveRequest},
        {"プレイヤー移動完了", &dAcNpcSpOpTanukichi_c::initPlayerMoveDone,
         &dAcNpcSpOpTanukichi_c::executePlayerMoveDone},
        {"会話", &dAcNpcSpOpTanukichi_c::initTalk,
         &dAcNpcSpOpTanukichi_c::executeTalk},
        {"うろうろ歩き", &dAcNpcSpOpTanukichi_c::initWanderWalk,
         &dAcNpcSpOpTanukichi_c::executeWanderWalk},
        {"待機", &dAcNpcSpOpTanukichi_c::initWait,
         &dAcNpcSpOpTanukichi_c::executeWait},
        {"非表示", &dAcNpcSpOpTanukichi_c::initHidden,
         &dAcNpcSpOpTanukichi_c::executeHidden},
    };
    dState::mode_c<dAcNpcSpOpTanukichi_c>::initialize(
        "dAcNpcSpOpTanukichi_c", modes, ARRAY_SIZE(modes), false);
    setDemoFlag8();
    fgMngProc_blockDayChange();
    return SUCCEEDED;
}

// .text 0x340
int dAcNpcSpOpTanukichi_c::doDelete() {
    if (!dAcNpcSp_c::doDelete()) {
        return NOT_READY;
    }
    return SUCCEEDED;
}

// .text 0x36C
int dAcNpcSpOpTanukichi_c::vtB0() {
    dState::base_c<dAcNpcSpOpTanukichi_c>::execute();
    return SUCCEEDED;
}

// .text 0x3C0
void dAcNpcSpOpTanukichi_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeMode(&dAcNpcSpOpTanukichi_c::executeDemoWait);
        break;
    case DEMO_CMD_TALK:
        mOpTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeMode(&dAcNpcSpOpTanukichi_c::executeTalk);
        break;
    case DEMO_CMD_SPEAK:
        mOpTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeMode(&dAcNpcSpOpTanukichi_c::executePlayerMoveRequest);
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        if (dSaveData_c::getTown()->mHomes.getCurrentPlayerHome() == NULL) {
            changeMode(&dAcNpcSpOpTanukichi_c::executeWait);
        } else {
            mObjc.setDefWeight(objc_c::WEIGHT_DEFAULT);
            changeMode(&dAcNpcSpOpTanukichi_c::executeWanderWalk);
        }
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

// .text 0xA50
void dAcNpcSpOpTanukichi_c::initWait() {
    mAction.requestWait(PRIORITY_IDLE);
}

// .text 0xA64
void dAcNpcSpOpTanukichi_c::executeWait() {
}

// .text 0xA68
void dAcNpcSpOpTanukichi_c::initDemoWait() {
    mAction.requestWait(PRIORITY_DEMO);
}

// .text 0xA7C
void dAcNpcSpOpTanukichi_c::executeDemoWait() {
}

// .text 0xA80
void dAcNpcSpOpTanukichi_c::initPlayerMoveRequest() {
}

// .text 0xA84
void dAcNpcSpOpTanukichi_c::executePlayerMoveRequest() {
    mVec3_c *player = dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (player) {
        // Walk the player up to Nook's depth.
        mVec3_c pos;
        pos = *player;
        pos.z = mPos.z;
        if (dPlayerMgr::moveTo(&pos, dPlayerMgr::PLAYER_INDEX_CURRENT,
                               dPlayerMgr::sDefaultMoveSpeed)) {
            changeMode(&dAcNpcSpOpTanukichi_c::executePlayerMoveDone);
        }
    }
}

// .text 0xC2C
void dAcNpcSpOpTanukichi_c::initPlayerMoveDone() {
}

// .text 0xC30
void dAcNpcSpOpTanukichi_c::executePlayerMoveDone() {
    if (!dPlayerMgr::isMovingTo(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        clearDemoFlag8();
        changeMode(&dAcNpcSpOpTanukichi_c::executeTalk);
    }
}

// .text 0xDA4
void dAcNpcSpOpTanukichi_c::initTalk() {
    if (!mTalk.isBusy()) {
        mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                          cNpcLookRange);
        mAng angle = getTalkAngle();
        mAng speed = getTalkTurnSpeed();
        mTalk.requestTurnTalk(angle, speed, FALSE);
    }
}

// .text 0xE44
void dAcNpcSpOpTanukichi_c::executeTalk() {
    dPlayerMgr::isMovingTo(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
    }
}

// .text 0xE8C
void dAcNpcSpOpTanukichi_c::initForcedTalk() {
    dBgm::l_mgr.mStgField.fn_80078408();
    mOpeningBgm = 1;
}

// .text 0xECC
void dAcNpcSpOpTanukichi_c::executeForcedTalk() {
    dDemo::requestSpeak(this, NULL);
}

// .text 0xED4
void dAcNpcSpOpTanukichi_c::initWanderWalk() {
    if (mOpeningBgm) {
        dBgm::l_mgr.mStgField.fn_8007840C();
        mOpeningBgm = 0;
    }
    initWander();
}

// .text 0xF28
void dAcNpcSpOpTanukichi_c::executeWanderWalk() {
    if (mViewArea.isOutsideAll(NET_MEMBER_MAX, this)) {
        changeMode(&dAcNpcSpOpTanukichi_c::executeHidden);
    } else {
        executeWander(NULL);
    }
}

// .text 0x10B8
void dAcNpcSpOpTanukichi_c::initHidden() {
    mAction.requestWait(PRIORITY_IDLE);
}

// .text 0x10CC
void dAcNpcSpOpTanukichi_c::executeHidden() {
    if (!mViewArea.isOutsideAll(NET_MEMBER_MAX, this)) {
        changeMode(&dAcNpcSpOpTanukichi_c::executeWanderWalk);
    }
}

// .text 0x1240
dAcNpcSpOpTanukichi_c::talk_c::talk_c(dAcNpcSpOpTanukichi_c *npc)
    : mpOwner(npc) {
    mMsgCode = 0;
    mDayChangeBlocked = 1;
}

// .text 0x12A4
dAcNpcSpOpTanukichi_c::talk_c::~talk_c() {
}

// .text 0x12FC
void dAcNpcSpOpTanukichi_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgCode = 0;
}

// .text 0x1330
void dAcNpcSpOpTanukichi_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    // The house of the tour the player came out of.
    int homeNo = dHomeList_c::getHomeFromScene(NULL, dScene::getPrevScene());
    dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
    dHome_c *home = homes->getHome(homeNo);
    if (!mTalked) {
        mTalked = 1;
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        if (!player->isFlag0(PRIVATE_FLAG0_OP_NOOK_GREETED)) {
            player->setFlag0(PRIVATE_FLAG0_OP_NOOK_GREETED);
            mMsgCode = MSG_GREETING;
        } else if (home->mOwner.isValid()) {
            mMsgCode = MSG_SOMEONES_HOUSE_2;
        } else if (!dNpcInfo::opTanukichi_c::get()->mViewed[homeNo]) {
            mMsgCode = MSG_FIRST_HOUSE;
        } else {
            mMsgCode = MSG_READY_TO_BUY;
        }
        u8 *viewed = dNpcInfo::opTanukichi_c::get()->mViewed;
        viewed[homeNo] = 1;
    } else if (homes->getCurrentPlayerHome() == NULL) {
        if (home->mOwner.isValid()) {
            mMsgCode = MSG_OTHER_PROPERTIES;
        } else {
            mMsgCode = MSG_TAKE_YOUR_TIME;
        }
    } else {
        mMsgCode = MSG_NONE;
    }
}

// .text 0x1488
void dAcNpcSpOpTanukichi_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_tanukichi_0";
    info->mCode = mMsgCode;
}

// .text 0x14A0
void dAcNpcSpOpTanukichi_c::talk_c::onMessageStart(int arg) {
    switch (getMessageCode()) {
    case MSG_HOUSE_BOUGHT:
        dBgm::l_mgr.mStgField.mute1C();
        dBgm::l_mgr.mStgField.play20();
        break;
    case MSG_LOOK_AROUND:
        dBgm::l_mgr.mStgField.set25();
        break;
    case MSG_SOMEONES_HOUSE:
    case MSG_SOMEONES_HOUSE_2: {
        int homeNo =
            dHomeList_c::getHomeFromScene(NULL, dScene::getPrevScene());
        dPersonalID_c owner =
            dSaveData_c::getTown()->mHomes.getHome(homeNo)->mOwner;
        setPersonalName(&owner, 0);
        break;
    }
    }
}

// .text 0x161C
void dAcNpcSpOpTanukichi_c::talk_c::onMessageEnd(int kind) {
    int code;
    dMsg::Comp_c *ctrl = mpController;
    code = getMessageCode();
    int homeNo = dHomeList_c::getHomeFromScene(NULL, dScene::getPrevScene());
    dHome_c *home = dSaveData_c::getTown()->mHomes.getHome(homeNo);
    switch (code) {
    case MSG_GREETING:
        if (home->mOwner.isValid()) {
            ctrl->setNextMsgCode(MSG_SOMEONES_HOUSE);
        } else {
            ctrl->setNextMsgCode(MSG_HOW_WAS_THE_HOUSE);
        }
        break;
    case MSG_HOUSE_BOUGHT:
        home->setOwnerToCurrent();
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        if (player) {
            player->mCatalog.registerDefaults(
                homeNo & 3); // the house's default furniture set
        }
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setDebtFromHouse();
        break;
    }
}

// .text 0x16F4
void dAcNpcSpOpTanukichi_c::talk_c::onAnswer(int answer) {
}

// .text 0x16F8
void dAcNpcSpOpTanukichi_c::talk_c::preExecute() {
}

// .text 0x16FC (+ a dead blr at 0x1714)
void dAcNpcSpOpTanukichi_c::talk_c::onTalkEnd() {
    if (mDayChangeBlocked) {
        mDayChangeBlocked = 0;
        fgMngProc_unblockDayChange();
    }
}

// .text 0x1718 (+ a dead blr at 0x1730)
void dAcNpcSpOpTanukichi_c::talk_c::onCustomTag(int no) {
    if (no == CUSTOM_PLAY_BGM) {
        dBgm::l_mgr.mStgField.play1C();
    }
}

// .text 0x1734
int dAcNpcSpOpTanukichi_c::getEarType() {
    int type = NPC_EAR_TYPE_NOOK_CRANNY;
    switch (dSaveData_c::getTown()->mShops.mShop.mStage) {
    case SHOP_STAGE_NOOK_N_GO:
        type = NPC_EAR_TYPE_NOOK_N_GO;
        break;
    case SHOP_STAGE_NOOKWAY:
        type = NPC_EAR_TYPE_NOOKWAY;
        break;
    case SHOP_STAGE_NOOKINGTONS:
        type = NPC_EAR_TYPE_NOOKINGTONS;
        break;
    }
    return type;
}
