#include <game/game/d_a_npc_sp_missing_mother.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>

// Katie's mother. Functions are in target .text order.

namespace dANpcSpMissingMother {
// .text 0x5C
BASE_PROFILE(A_NPC_SP_MISSING_MOTHER, dAcNpcSpMissingMother_c);
} // namespace dANpcSpMissingMother

// .text 0x1D4
int dAcNpcSpMissingMother_c::preCreate() {
    clearNetSync();
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mMotherTalk);
    return SUCCEEDED;
}

// .text 0x228
int dAcNpcSpMissingMother_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    static const dState::mode_c<dAcNpcSpMissingMother_c>::Entry modes[] = {
        {"待機", &dAcNpcSpMissingMother_c::initWait,
         &dAcNpcSpMissingMother_c::executeWait},
        {"デモ待機", &dAcNpcSpMissingMother_c::initDemoWait,
         &dAcNpcSpMissingMother_c::executeDemoWait},
        {"会話", &dAcNpcSpMissingMother_c::initTalk,
         &dAcNpcSpMissingMother_c::executeTalk},
        {"うろうろ歩き", &dAcNpcSpMissingMother_c::initWanderWalk,
         &dAcNpcSpMissingMother_c::executeWanderWalk},
        {"非表示", &dAcNpcSpMissingMother_c::initHidden,
         &dAcNpcSpMissingMother_c::executeHidden},
        {"再会デモ", &dAcNpcSpMissingMother_c::initReunion,
         &dAcNpcSpMissingMother_c::executeReunion},
    };
    dState::mode_c<dAcNpcSpMissingMother_c>::initialize(
        "dAcNpcSpMissingMother_c", modes, ARRAY_SIZE(modes), false);

    if (dScene::getCurrentScene() == dScene::SCENE_FIELD) {
        mObjc.setDefWeight(objc_c::WEIGHT_DEFAULT);
    } else if (dScene::getCurrentScene() == dScene::SCENE_DM_CHKP_MISS) {
        changeMode(&dAcNpcSpMissingMother_c::executeReunion);
    } else {
        return FAILED;
    }
    return SUCCEEDED;
}

// .text 0x440
int dAcNpcSpMissingMother_c::doDelete() {
    if (!dAcNpcSp_c::doDelete()) {
        return NOT_READY;
    }
    return SUCCEEDED;
}

// .text 0x46C
int dAcNpcSpMissingMother_c::vtB0() {
    dState::base_c<dAcNpcSpMissingMother_c>::execute();
    return SUCCEEDED;
}

// .text 0x4C0
void dAcNpcSpMissingMother_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeMode(&dAcNpcSpMissingMother_c::executeWait);
        break;
    case DEMO_CMD_TALK:
        mMotherTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeMode(&dAcNpcSpMissingMother_c::executeTalk);
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        changeMode(&dAcNpcSpMissingMother_c::executeWait);
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

// .text 0x8B8
void dAcNpcSpMissingMother_c::initWait() {
    mAction.requestWait(PRIORITY_IDLE);
}

// .text 0x8CC
void dAcNpcSpMissingMother_c::executeWait() {
    changeMode(&dAcNpcSpMissingMother_c::executeWanderWalk);
}

// .text 0xA28
void dAcNpcSpMissingMother_c::initWanderWalk() {
    initWander();
}

// .text 0xA2C
void dAcNpcSpMissingMother_c::executeWanderWalk() {
    if (mViewArea.isOutsideAll(NET_MEMBER_MAX, this)) {
        changeMode(&dAcNpcSpMissingMother_c::executeHidden);
    } else {
        executeWander(NULL);
    }
}

// .text 0xBBC
void dAcNpcSpMissingMother_c::initHidden() {
    mAction.requestWait(PRIORITY_IDLE);
}

// .text 0xBD0
void dAcNpcSpMissingMother_c::executeHidden() {
    if (!mViewArea.isOutsideAll(NET_MEMBER_MAX, this)) {
        changeMode(&dAcNpcSpMissingMother_c::executeWanderWalk);
    }
}

// .text 0xD44
void dAcNpcSpMissingMother_c::initDemoWait() {
    mAction.requestTurn(PRIORITY_DEMO,
                        getAngleYToPlayer(dPlayerMgr::PLAYER_INDEX_CURRENT),
                        l_turnSpeed, 0);
}

// .text 0xDA0
void dAcNpcSpMissingMother_c::executeDemoWait() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO);
    }
}

// .text 0xDD0
void dAcNpcSpMissingMother_c::initTalk() {
    if (!mTalk.isBusy()) {
        mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                          cNpcLookRange);
        mAng angle = getTalkAngle();
        mAng speed = getTalkTurnSpeed();
        mTalk.requestTurnTalk(angle, speed, FALSE);
    }
}

// .text 0xE70
void dAcNpcSpMissingMother_c::executeTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
    }
}

// .text 0xEB0
void dAcNpcSpMissingMother_c::initReunion() {
    mCheckGround = 0;
    mObjc.setDefWeight(objc_c::WEIGHT_IMMOVABLE);
}

// .text 0xEC4
void dAcNpcSpMissingMother_c::executeReunion() {
}

// .text 0xEC8
dAcNpcSpMissingMother_c::talk_c::talk_c(dAcNpcSpMissingMother_c *npc)
    : mpOwner(npc) {
    init();
}

// .text 0xF30
dAcNpcSpMissingMother_c::talk_c::~talk_c() {
}

// .text 0xF88
void dAcNpcSpMissingMother_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
}

// .text 0xF8C
void dAcNpcSpMissingMother_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
}

// .text 0xFD4
void dAcNpcSpMissingMother_c::talk_c::getMsgInfo(msgInfo_s *info) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    info->mLabel = "npc_SP/NPC_maigo_mother";
    BOOL met = player->isFlag0(PRIVATE_FLAG0_LOST_CHILD_MET);
    player->setFlag0(PRIVATE_FLAG0_LOST_CHILD_MET);
    if (!met) {
        if (!player->isFlag0(PRIVATE_FLAG0_LOST_CHILD_RETURNED)) {
            info->mCode = MSG_FIRST_MEETING;
        } else {
            info->mCode = MSG_FIRST_MEETING_AGAIN;
        }
        player->setFlag1(PRIVATE_FLAG1_LOST_MOTHER_MET);
    } else {
        BOOL metToday = player->isFlag1(PRIVATE_FLAG1_LOST_MOTHER_MET);
        player->setFlag1(PRIVATE_FLAG1_LOST_MOTHER_MET);
        if (!metToday) {
            info->mCode = MSG_MEET;
        } else {
            info->mCode = MSG_MEET_AGAIN;
        }
    }
}

// .text 0x10B0
void dAcNpcSpMissingMother_c::talk_c::onMessageStart(int arg) {
    // The player's own town, where Katie may be.
    dSaveTown_c *town = dSaveData_c::getTown();
    setLandName(&town->mTownHost.getPID().land, 1);
}

// .text 0x10F4
int dAcNpcSpMissingMother_c::getEarType() {
    return NPC_EAR_TYPE_MISSING_MOTHER;
}
