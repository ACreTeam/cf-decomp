#include <game/game/d_a_npc_sp_missing_child.hpp>
#include <game/framework/f_manager.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_npc_sp_gatekeeper_a.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_bg_attr.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_ground_attr.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_def.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_npc_list.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/mLib/m_fader.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/snd/snd_id.hpp>
#include <cstring>

// Katie, the lost kitten. Functions are in target .text order; the
// stepDemoBase_c template members are defined at the end of the file (the
// target instantiates them after the other code).

namespace dANpcSpMissingChild {
// .text 0x5C
static dScene::dSceneChange_c *getSceneChange() {
    return &dScene::gSceneChange;
}

// Plain aggregate vectors keep the scene positions in read-only data, without
// the dynamic initialization required by file-scope mVec3_c objects.
// Positions in the checkpoint scene (the reunion demo):
// - l_playerWalkPos: where the player is walked to (the second entry is not
//   used);
// - l_startPos: Katie at the gate; l_walkPos: where she walks to (walkToGate);
// - l_motherSidePos / l_motherPos: where she runs to (runToMotherSide /
//   runToMother); l_motherRunPos: where her mother runs to;
// - l_sidePos: Katie next to her mother; l_exitPos / l_motherExitPos: where
//   they leave.
static const nw4r::math::_VEC3 l_playerWalkPos[2] = {
    {256.0f, 0.0f, 304.0f}, {272.0f, 0.0f, 320.0f},
};
static const nw4r::math::_VEC3 l_startPos = {240.0f, 0.0f, 0.0f};
static const nw4r::math::_VEC3 l_walkPos = {240.0f, 0.0f, 256.0f};
static const nw4r::math::_VEC3 l_motherSidePos = {224.0f, 0.0f, 288.0f};
static const nw4r::math::_VEC3 l_motherPos = {224.0f, 0.0f, 336.0f};
static const nw4r::math::_VEC3 l_motherRunPos = {240.0f, 0.0f, 368.0f};
static const nw4r::math::_VEC3 l_sidePos = {256.0f, 0.0f, 352.0f};
static const nw4r::math::_VEC3 l_exitPos = {256.0f, 0.0f, 528.0f};
static const nw4r::math::_VEC3 l_motherExitPos = {240.0f, 0.0f, 528.0f};

// The label of Katie's messages (.data 0x18; the string is .data 0x0).
const char *l_msgLabel = "NPC_sp/NPC_maigo_girl";

// .text 0x68
BASE_PROFILE(A_NPC_SP_MISSING_CHILD, dAcNpcSpMissingChild_c);
} // namespace dANpcSpMissingChild

// .text 0x218
int dAcNpcSpMissingChild_c::preCreate() {
    if (dScene::getCurrentScene() == dScene::SCENE_DM_CHKP_MISS) {
        clearNetSync();
    }
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mChildTalk);
    return SUCCEEDED;
}

// .text 0x280
int dAcNpcSpMissingChild_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    static const dState::mode_c<dAcNpcSpMissingChild_c>::Entry modes[] = {
        {"待機", &dAcNpcSpMissingChild_c::initWait,
         &dAcNpcSpMissingChild_c::executeWait},
        {"プレイヤ生成待機", &dAcNpcSpMissingChild_c::initWaitPlayer,
         &dAcNpcSpMissingChild_c::executeWaitPlayer},
        {"回転", &dAcNpcSpMissingChild_c::initTurn,
         &dAcNpcSpMissingChild_c::executeTurn},
        {"デモ待機", &dAcNpcSpMissingChild_c::initDemoWait,
         &dAcNpcSpMissingChild_c::executeDemoWait},
        {"会話", &dAcNpcSpMissingChild_c::initTalk,
         &dAcNpcSpMissingChild_c::executeTalk},
        {"プレーヤー追従歩き", &dAcNpcSpMissingChild_c::initFollowWalk,
         &dAcNpcSpMissingChild_c::executeFollowWalk},
        {"プレーヤー追従走り", &dAcNpcSpMissingChild_c::initFollowRun,
         &dAcNpcSpMissingChild_c::executeFollowRun},
        {"泣き", &dAcNpcSpMissingChild_c::initCry,
         &dAcNpcSpMissingChild_c::executeCry},
        {"べそかき", &dAcNpcSpMissingChild_c::initSob,
         &dAcNpcSpMissingChild_c::executeSob},
        {"転ぶ", &dAcNpcSpMissingChild_c::initFall,
         &dAcNpcSpMissingChild_c::executeFall},
        {"門から出てくる", &dAcNpcSpMissingChild_c::initComeOutOfGate,
         &dAcNpcSpMissingChild_c::executeComeOutOfGate},
        {"処理なし", &dAcNpcSpMissingChild_c::initNone,
         &dAcNpcSpMissingChild_c::executeNone},
        {"再会デモ", &dAcNpcSpMissingChild_c::initReunion,
         &dAcNpcSpMissingChild_c::executeReunion},
        {"再会デモ（通信）", &dAcNpcSpMissingChild_c::initReunionNet,
         &dAcNpcSpMissingChild_c::executeReunionNet},
    };
    dState::mode_c<dAcNpcSpMissingChild_c>::initialize(
        "dAcNpcSpMissingChild_c", modes, ARRAY_SIZE(modes), false);

    if (dScene::getCurrentScene() == dScene::SCENE_FIELD) {
        if (dNpcInfo::gatekeeperA_c::get()->mEscorting) {
            if (dScene::getPrevScene() == dScene::SCENE_RM_CHKP) {
                changeMode(&dAcNpcSpMissingChild_c::executeWaitPlayer);
            } else {
                mChildTalk.mMsgKind = talk_c::MSG_KIND_FOLLOW;
                changeMode(&dAcNpcSpMissingChild_c::executeCry);
            }
        } else {
            if (dNpcInfo::gatekeeperA_c::get()->mAtGate) {
                return FAILED;
            }
            mChildTalk.mMsgKind = talk_c::MSG_KIND_LOST;
            changeMode(&dAcNpcSpMissingChild_c::executeCry);
        }
    } else if (dScene::getCurrentScene() == dScene::SCENE_DM_CHKP_MISS) {
        mPos = mVec3_c(dANpcSpMissingChild::l_startPos);
        mAngle.set(0, 0, 0);
        mAngle3D.set(0, 0, 0);
        if (dNet::isSessionActive() &&
            dSvMgr_c::mCurrentVisitor != NET_MEMBER_NONE) {
            changeMode(&dAcNpcSpMissingChild_c::executeReunionNet);
        } else {
            changeMode(&dAcNpcSpMissingChild_c::executeReunion);
        }
        mObjc.setDefWeight(objc_c::WEIGHT_IMMOVABLE);
        mChildTalk.mMsgKind = talk_c::MSG_KIND_REUNION;
        dDemo::onHoldStart();
    } else {
        if (dNpcInfo::gatekeeperA_c::get()->mEscorting) {
            mChildTalk.mMsgKind = talk_c::MSG_KIND_LOST;
            changeMode(&dAcNpcSpMissingChild_c::executeWait);
        } else if (dNpcInfo::gatekeeperA_c::get()->mAtGate) {
            mVec3_c pos = mVec3_c::Zero;
            mAng3_c angle = mAng3_c::Zero;
            dItem::Item key = SP_NPC_MISSING_CHILD;
            BOOL notFound = !dNpcMng_c::getDaubPos(&pos, &angle, &key);
            if (notFound) {
                pos.set(304.0f, 0.0f, 384.0f);
            }
            mPos = pos;
            mAngle.y = angle.y;
            mAngle3D.y = angle.y;
            mChildTalk.mMsgKind = talk_c::MSG_KIND_LOST;
            changeMode(&dAcNpcSpMissingChild_c::executeCry);
        } else {
            return FAILED;
        }
    }

    if (dScene::getCurrentScene() != dScene::SCENE_DM_CHKP_MISS &&
        isNetActive()) {
        dNpcInfo::gatekeeperA_c::get()->clear();
        return FAILED;
    }
    if (dScene::getCurrentScene() == dScene::SCENE_RM_CHKP) {
        dNpcInfo::gatekeeperA_c::get()->setAtGate(1);
    } else {
        dNpcInfo::gatekeeperA_c::get()->setAtGate(0);
    }
    return SUCCEEDED;
}

// .text 0xD78
int dAcNpcSpMissingChild_c::doDelete() {
    if (!dAcNpcSp_c::doDelete()) {
        return NOT_READY;
    }
    if (dScene::getCurrentScene() == dScene::SCENE_DM_CHKP_MISS) {
        dDemo::offHoldStart();
    }
    return SUCCEEDED;
}

// .text 0xDC0
int dAcNpcSpMissingChild_c::vtB0() {
    dState::base_c<dAcNpcSpMissingChild_c>::execute();
    return SUCCEEDED;
}

// .text 0xE14
void dAcNpcSpMissingChild_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeMode(&dAcNpcSpMissingChild_c::executeDemoWait);
        break;
    case DEMO_CMD_TALK:
        mChildTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeMode(&dAcNpcSpMissingChild_c::executeTalk);
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        mChildTalk.mMsgKind = talk_c::MSG_KIND_LOST;
        if (dNpcInfo::gatekeeperA_c::get()->mEscorting) {
            changeMode(&dAcNpcSpMissingChild_c::executeWait);
        } else {
            changeMode(&dAcNpcSpMissingChild_c::executeSob);
        }
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

// .text 0x134C
void dAcNpcSpMissingChild_c::initWaitPlayer() {
    mIsActive = 0;
}

// .text 0x1358
void dAcNpcSpMissingChild_c::executeWaitPlayer() {
    mVec3_c *playerPos =
        dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (playerPos != NULL) {
        mIsActive = 1;
        mVec3_c pos = *playerPos;
        pos.x -= 16.0f;
        pos.z -= 80.0f;
        mPos = pos;
        mLastPos = pos;
        mAngle.y = 0;
        mAngle3D.y = 0;
        mChildTalk.mMsgKind = talk_c::MSG_KIND_LOST;
        changeMode(&dAcNpcSpMissingChild_c::executeComeOutOfGate);
    }
}

// .text 0x1524
void dAcNpcSpMissingChild_c::initWait() {
    mObjc.setDefWeight(objc_c::WEIGHT_DEFAULT);
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
    mAnmSet.setAnm0(HMN_ANM_NML_WAIT1);
    mAction.requestWait(PRIORITY_IDLE);
}

// .text 0x15A0
void dAcNpcSpMissingChild_c::executeWait() {
    if (calcPlayerDistXZ(0, NULL) >= 64.0f) {
        changeMode(&dAcNpcSpMissingChild_c::executeFollowWalk);
    } else if (sLib::distanceAngle(mAngle.y, getAngleYToPlayer(0)) > DEG_TO_ANGLE(45)) {
        changeMode(&dAcNpcSpMissingChild_c::executeTurn);
    }
}

// .text 0x1868
void dAcNpcSpMissingChild_c::initTurn() {
    mObjc.setDefWeight(objc_c::WEIGHT_DEFAULT);
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
    mAction.requestTurn(PRIORITY_IDLE, getAngleYToPlayer(0), l_turnSpeed, 0);
}

// .text 0x18FC
void dAcNpcSpMissingChild_c::executeTurn() {
    if (calcPlayerDistXZ(0, NULL) >= 64.0f) {
        changeMode(&dAcNpcSpMissingChild_c::executeFollowWalk);
    }
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        changeMode(&dAcNpcSpMissingChild_c::executeWait);
    } else {
        mAction.setTargetAngle(getAngleYToPlayer(0));
    }
}

// .text 0x1BD8
void dAcNpcSpMissingChild_c::initDemoWait() {
    mAnmSet.setAnm0(HMN_ANM_NML_WAIT1);
    mAction.requestTurn(PRIORITY_DEMO, getAngleYToPlayer(0), l_turnSpeed, 0);
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
    stopManpu();
}

// .text 0x1C74
void dAcNpcSpMissingChild_c::executeDemoWait() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO);
    }
}

// .text 0x1CA4
void dAcNpcSpMissingChild_c::initTalk() {
    mAnmSet.setAnm0(HMN_ANM_NML_WAIT1);
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
    stopManpu();
    if (!mTalk.isBusy()) {
        mAng angle = getTalkAngle();
        mAng speed = getTalkTurnSpeed();
        mTalk.requestTurnTalk(angle, speed, FALSE);
    }
}

// .text 0x1D58
void dAcNpcSpMissingChild_c::executeTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
    }
}

// .text 0x1D98
void dAcNpcSpMissingChild_c::initFollowWalk() {
    mObjc.setDefWeight(objc_c::WEIGHT_DEFAULT);
    mVec3_c pos(mPos);
    mVec3_c *playerPos = dPlayerMgr::getPosition(0);
    if (playerPos != NULL) {
        pos = *playerPos;
    }
    mAction.requestWalk(PRIORITY_IDLE, pos, l_defaultAnmRate, l_walkTurnSpeed, 0, l_moveParamWalk,
        cNpcMorphFrames);
}

// .text 0x1E40
void dAcNpcSpMissingChild_c::executeFollowWalk() {
    f32 dist = calcPlayerDistXZ(0, NULL);
    if (dist < 32.0f) {
        changeMode(&dAcNpcSpMissingChild_c::executeWait);
    } else if (dist >= 96.0f) {
        changeMode(&dAcNpcSpMissingChild_c::executeFollowRun);
    } else {
        action_c *action;
        mVec3_c *playerPos;
        playerPos = dPlayerMgr::getPosition(0);
        action = &mAction;
        if (!action->isArrived(this, TRUE)) {
            mVec3_c pos;
            switch (getSidePos(&pos)) {
            case SIDE_POS_CLEAR:
                if (playerPos != NULL) {
                    action->setTargetPos(*playerPos);
                } else {
                    changeMode(&dAcNpcSpMissingChild_c::executeWait);
                }
                break;
            case SIDE_POS_BLOCKED:
                if (dScene::getCurrentScene() == dScene::SCENE_FIELD) {
                    changeMode(&dAcNpcSpMissingChild_c::executeCry);
                } else {
                    changeMode(&dAcNpcSpMissingChild_c::executeWait);
                }
                break;
            case SIDE_POS_DETOUR:
                action->setGoalPos(pos);
                break;
            }
        } else if (action->isDetour()) {
            action->resetGoal();
        }
    }
}

// .text 0x2524
void dAcNpcSpMissingChild_c::initFollowRun() {
    mObjc.setDefWeight(objc_c::WEIGHT_DEFAULT);
    mVec3_c pos(mPos);
    mVec3_c *playerPos = dPlayerMgr::getPosition(0);
    if (playerPos != NULL) {
        pos = *playerPos;
    }
    moveParam_c param(1.1f, 0.26f, 0.4f);
    mAction.requestRun(PRIORITY_IDLE, pos, l_runTurnSpeed, 0, param, l_8074FDEC,
                       cNpcMorphFrames);
}

// .text 0x25F0
void dAcNpcSpMissingChild_c::executeFollowRun() {
    f32 dist = calcPlayerDistXZ(0, NULL);
    if (dist < 64.0f) {
        changeMode(&dAcNpcSpMissingChild_c::executeFollowWalk);
    } else if (dScene::getCurrentScene() == dScene::SCENE_FIELD &&
               dist >= 128.0f) {
        changeMode(&dAcNpcSpMissingChild_c::executeFall);
    } else {
        action_c *action;
        mVec3_c *playerPos;
        playerPos = dPlayerMgr::getPosition(0);
        action = &mAction;
        if (!action->isArrived(this, TRUE)) {
            mVec3_c pos;
            switch (getSidePos(&pos)) {
            case SIDE_POS_CLEAR:
                if (playerPos != NULL) {
                    action->setTargetPos(*playerPos);
                } else {
                    changeMode(&dAcNpcSpMissingChild_c::executeWait);
                }
                break;
            case SIDE_POS_BLOCKED:
                if (dScene::getCurrentScene() == dScene::SCENE_FIELD) {
                    changeMode(&dAcNpcSpMissingChild_c::executeFall);
                } else {
                    changeMode(&dAcNpcSpMissingChild_c::executeWait);
                }
                break;
            case SIDE_POS_DETOUR:
                action->setGoalPos(pos);
                break;
            }
        } else if (action->isDetour()) {
            action->resetGoal();
        }
    }
}

// .text 0x2CF4
void dAcNpcSpMissingChild_c::initCry() {
    mObjc.setDefWeight(objc_c::WEIGHT_DEFAULT);
    mLookAt.setFront(PRIORITY_IDLE, l_lookPitchStep, l_lookYawStep);
    mAction.requestEmotion(PRIORITY_IDLE, action_c::EMOTION_CRY, cNpcMorphFrames);
}

// .text 0x2D5C
void dAcNpcSpMissingChild_c::executeCry() {
}

// .text 0x2D60
void dAcNpcSpMissingChild_c::initSob() {
    mObjc.setDefWeight(objc_c::WEIGHT_DEFAULT);
    mLookAt.setFront(PRIORITY_IDLE, l_lookPitchStep, l_lookYawStep);
    mAction.requestEmotion(PRIORITY_IDLE, action_c::EMOTION_SOB, cNpcMorphFrames);
}

// .text 0x2DC8
void dAcNpcSpMissingChild_c::executeSob() {
}

// .text 0x2DCC
void dAcNpcSpMissingChild_c::initFall() {
    mObjc.setDefWeight(objc_c::WEIGHT_IMMOVABLE);
    dNpcInfo::gatekeeperA_c::get()->setEscorting(0);
    mChildTalk.mMsgKind = talk_c::MSG_KIND_FELL;
    static const stepFall_c::Entry steps[] = {
        {"転びアニメ初期化", (stepFall_c::Method)&stepFall_c::initFallAnm},
        {"転びアニメ終了待ち", (stepFall_c::Method)&stepFall_c::waitFallAnm},
        {"起き上がりアニメ初期化",
         (stepFall_c::Method)&stepFall_c::initGetUpAnm},
        {"起き上がりアニメ終了待ち",
         (stepFall_c::Method)&stepFall_c::waitGetUpAnm},
    };
    mStepFall.initialize(this, "dAcNpcSpMissingChild_c::stepFall_c", steps,
                         ARRAY_SIZE(steps), false);
}

// .text 0x2EE8
void dAcNpcSpMissingChild_c::executeFall() {
    mStepFall.execute();
}

// .text 0x2F30
void dAcNpcSpMissingChild_c::stepFall_c::initFallAnm() {
    getOwner()->mAction.requestAnm(
        PRIORITY_IDLE, HMN_ANM_TUMBLE_DOWNA1, 0.0f, cNpcMorphFrames,
        m3d::PLAYMODE_INHERIT, FALSE);
    nextStep();
    mAng angle = getOwner()->getAngleY();
    getOwner()->calcFrontPos(&mFallPos, angle, 16.0f);
    mVec3_c diff = mFallPos - getOwner()->getPos();
    mFallSpeed =
        EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z) / 37.0f;
}

// .text 0x3058
void dAcNpcSpMissingChild_c::stepFall_c::waitFallAnm() {
    cLib::chasePosXZ(getOwner()->getPosPtr(), mFallPos, mFallSpeed);
    getOwner()->setLastPos();
    if (5.0f == getOwner()->getModel()->mAnm.getFrame()) {
        u32 kind = dPlayerMgr::getSurfaceEffectKind(getOwner()->getPosP());
        if (kind == dPlayerMgr::SURFACE_EFFECT_RAIN) {
            getOwner()->mAudioObj.startSound(SE_Kokeru_rain);
        } else {
            getOwner()->mAudioObj.startSound(SE_Kokeru);
        }
    }
    if (20.0f == getOwner()->getModel()->mAnm.getFrame()) {
        static const char *effects[] = {"afi_ply_koke_nml", "af_ply_koke_water",
                                        "af_ply_koke_water", "af_ply_koke_snow",
                                        "afi_ply_koke_sand"};
        dPlayerMgr::createSurfaceEffects(
            effects, getOwner()->getPosP(), &getOwner()->getAngle(),
            "af_ply_koke_water", "af_ply_koke_snow");
        dBGCF::groundChk_c ground(getOwner()->getPosP(), 0, 0, 0);
        if (ground.mAttr == BG_ATTR_SAND || ground.mWater == BG_WATER_SEA ||
            dGroundAttr::getSnowWear(getOwner()->getPosP())) {
            dEffect::create("af_npc_maigo_kokeato", getOwner()->getPosP(),
                            &getOwner()->getAngle(), NULL);
        }
    }
    if (getOwner()->getModel()->mAnm.mAnmId == HMN_ANM_TUMBLE_DOWNA1 &&
        getOwner()->mAction.mCanChange) {
        nextStep();
    }
}

// .text 0x3274
void dAcNpcSpMissingChild_c::stepFall_c::initGetUpAnm() {
    getOwner()->mAction.requestAnm(
        PRIORITY_IDLE, HMN_ANM_TUMBLE_UPA1, 0.0f, cNpcMorphFrames,
        m3d::PLAYMODE_INHERIT, FALSE);
    nextStep();
}

// .text 0x3338
void dAcNpcSpMissingChild_c::stepFall_c::waitGetUpAnm() {
    if (getOwner()->getModel()->mAnm.mAnmId == HMN_ANM_TUMBLE_UPA1 &&
        getOwner()->mAction.mCanChange) {
        getOwner()->changeMode(&dAcNpcSpMissingChild_c::executeCry);
    }
}

// .text 0x34D0
void dAcNpcSpMissingChild_c::initComeOutOfGate() {
    mObjc.setDefWeight(objc_c::WEIGHT_IMMOVABLE);
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
    mCheckGround = 0;
    static const stepGate_c::Entry steps[] = {
        {"歩き1開始", (stepGate_c::Method)&stepGate_c::startWalk1},
        {"歩き1終了", (stepGate_c::Method)&stepGate_c::waitWalk1},
        {"歩き2開始", (stepGate_c::Method)&stepGate_c::startWalk2},
        {"歩き2終了", (stepGate_c::Method)&stepGate_c::waitWalk2},
    };
    mStepGate.initialize(this, "dAcNpcSpMissingChild_c::stepGate_c", steps,
                         ARRAY_SIZE(steps), false);
}

// .text 0x360C
void dAcNpcSpMissingChild_c::executeComeOutOfGate() {
    mStepGate.execute();
}

// .text 0x3654
void dAcNpcSpMissingChild_c::stepGate_c::startWalk1() {
    mVec3_c target;
    target = getOwner()->getPos();
    target.z += 32.0;
    getOwner()->mAction.requestWalk(PRIORITY_IDLE, target, l_defaultAnmRate, l_walkTurnSpeed, 0,
        l_moveParamWalk, cNpcMorphFrames);
    nextStep();
}

// .text 0x3750
void dAcNpcSpMissingChild_c::stepGate_c::waitWalk1() {
    if (getOwner()->mAction.isArrived(getOwner(), TRUE)) {
        nextStep();
    }
}

// .text 0x3800
void dAcNpcSpMissingChild_c::stepGate_c::startWalk2() {
    mVec3_c target;
    target = getOwner()->getPos();
    target.x -= 16.0;
    target.z += 48.0;
    getOwner()->mAction.requestWalk(PRIORITY_IDLE, target, l_defaultAnmRate, l_walkTurnSpeed, 0,
        l_moveParamWalk, cNpcMorphFrames);
    nextStep();
}

// .text 0x3910
void dAcNpcSpMissingChild_c::stepGate_c::waitWalk2() {
    if (getOwner()->mAction.isArrived(getOwner(), TRUE)) {
        getOwner()->mCheckGround = 1;
        getOwner()->changeMode(&dAcNpcSpMissingChild_c::executeWait);
    }
}

// .text 0x3A9C
void dAcNpcSpMissingChild_c::initNone() {
}

// .text 0x3AA0
void dAcNpcSpMissingChild_c::executeNone() {
}

// .text 0x3AA4
void dAcNpcSpMissingChild_c::initReunion() {
    mCheckGround = 0;
    static const stepDemoReunion_c::Entry steps[] = {
        {"フェードイン終了待ち",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::waitFadeIn},
        {"歩き開始", (stepDemoReunion_c::Method)&stepDemoReunion_c::startWalk},
        {"歩き終了", (stepDemoReunion_c::Method)&stepDemoReunion_c::waitWalk},
        {"プレイヤ歩き終了",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::waitPlayerWalk},
        {"会話開始", (stepDemoReunion_c::Method)&stepDemoReunion_c::startTalk},
        {"会話サスペンドまで待つ",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::waitTalkSuspend},
        {"再会サイド移動開始",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::startSideMove},
        {"再会サイド移動終了",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::waitSideMove},
        {"再会開始",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::startReunion},
        {"プレイヤ回転",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::turnPlayer},
        {"子ネコが母ネコの方向へ回転",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::turnChildToMother},
        {"母ネコが子ネコの方向へ回転",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::turnMotherToChild},
        {"再会終了",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::finishReunion},
        {"会話再開",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::restartTalk},
        {"会話終了", (stepDemoReunion_c::Method)&stepDemoReunion_c::finishTalk},
        {"母ネコの側方向へ回転",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::turnToMotherSide},
        {"母ネコの側方向へ回転終了",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::waitTurnToMotherSide},
        {"母ネコの側方向へ移動",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::moveToMotherSide},
        {"母ネコ回転終了",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::waitMotherTurn},
        {"母ネコの側方向へ移動終了",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::waitMoveToMotherSide},
        {"出口方向へ回転",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::turnToExit},
        {"出口方向へ回転終了",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::waitTurnToExit},
        {"出口方向へ移動",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::moveToExit},
        {"子ネコ笑顔",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::smileChild},
        {"母ネコ笑顔",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::smileMother},
        {"笑顔終了", (stepDemoReunion_c::Method)&stepDemoReunion_c::waitSmile},
        {"シーン切り替え待ち終了",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::waitSceneChange},
        {"門の外のシーンへ",
         (stepDemoReunion_c::Method)&stepDemoReunion_c::changeScene},
        {"終了", (stepDemoReunion_c::Method)&stepDemoReunion_c::finish},
    };
    mStepDemoReunion.initialize(this,
                                "dAcNpcSpMissingChild_c::stepDemoReunion_c",
                                steps, ARRAY_SIZE(steps), false);
}

// .text 0x3F30
void dAcNpcSpMissingChild_c::executeReunion() {
    mStepDemoReunion.execute();
}

// .text 0x3F78
void dAcNpcSpMissingChild_c::initReunionNet() {
    mCheckGround = 0;
    static const stepDemoReunionNet_c::Entry steps[] = {
        {"フェードイン終了待ち",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::waitFadeIn},
        {"もんばんにセーブを任せる",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::
             startGatekeeperSave},
        {"もんばんにセーブおわった？",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::
             waitGatekeeperSave},
        {"歩き開始",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::startWalk},
        {"歩き終了",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::waitWalk},
        {"プレイヤ歩き終了",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::waitPlayerWalk},
        {"会話開始",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::startTalk},
        {"会話サスペンドまで待つ",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::waitTalkSuspend},
        {"再会サイド移動開始",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::startSideMove},
        {"再会サイド移動終了",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::waitSideMove},
        {"再会開始",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::startReunion},
        {"プレイヤ回転",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::turnPlayer},
        {"子ネコが母ネコの方向へ回転",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::
             turnChildToMother},
        {"母ネコが子ネコの方向へ回転",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::
             turnMotherToChild},
        {"再会終了",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::finishReunion},
        {"会話再開",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::restartTalk},
        {"会話終了",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::finishTalk},
        {"母ネコの側方向へ回転",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::turnToMotherSide},
        {"母ネコの側方向へ回転終了",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::
             waitTurnToMotherSide},
        {"母ネコの側方向へ移動",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::moveToMotherSide},
        {"母ネコ回転終了",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::waitMotherTurn},
        {"母ネコの側方向へ移動終了",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::
             waitMoveToMotherSide},
        {"出口方向へ回転",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::turnToExit},
        {"出口方向へ回転終了",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::waitTurnToExit},
        {"出口方向へ移動",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::moveToExit},
        {"子ネコ笑顔",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::smileChild},
        {"母ネコ笑顔",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::smileMother},
        {"笑顔終了",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::waitSmile},
        {"シーン切り替え待ち終了",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::waitSceneChange},
        {"元のシーンへ",
         (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::changeScene},
        {"終了", (stepDemoReunionNet_c::Method)&stepDemoReunionNet_c::finish},
    };
    mStepDemoReunionNet.initialize(
        this, "dAcNpcSpMissingChild_c::stepDemoReunionNet_c", steps,
        ARRAY_SIZE(steps), false);
}

// .text 0x443C
void dAcNpcSpMissingChild_c::executeReunionNet() {
    mStepDemoReunionNet.execute();
}

// .text 0x4484
void dAcNpcSpMissingChild_c::walkToGate() {
    moveParam_c param(0.6666667f, 0.26666668f, 0.4f);
    mVec3_c pos(dANpcSpMissingChild::l_walkPos);
    mAction.requestWalk(PRIORITY_DEMO, pos, l_defaultAnmRate, l_walkTurnSpeed, 0, param,
        cNpcMorphFrames);
}

// .text 0x450C
dAcNpcSpMissingChild_c::talk_c::talk_c(dAcNpcSpMissingChild_c *npc)
    : mpOwner(npc) {
    init();
}

// .text 0x4574
dAcNpcSpMissingChild_c::talk_c::~talk_c() {
}

// .text 0x45CC
void dAcNpcSpMissingChild_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mRequestEndMethod = NULL;
}

// .text 0x4614
void dAcNpcSpMissingChild_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
}

// .text 0x465C
void dAcNpcSpMissingChild_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = dANpcSpMissingChild::l_msgLabel;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    switch (mMsgKind) {
    case MSG_KIND_LOST: {
        BOOL met = player->isFlag0(PRIVATE_FLAG0_LOST_CHILD_MET);
        player->setFlag0(PRIVATE_FLAG0_LOST_CHILD_MET);
        if (!met) {
            if (!player->isFlag0(PRIVATE_FLAG0_LOST_CHILD_RETURNED)) {
                info->mCode = MSG_FIRST_MEETING;
            } else {
                info->mCode = MSG_FIRST_MEETING_AGAIN;
            }
            player->setFlag1(PRIVATE_FLAG1_LOST_CHILD_PENDING);
        } else {
            BOOL pending = player->isFlag1(PRIVATE_FLAG1_LOST_CHILD_PENDING);
            player->setFlag1(PRIVATE_FLAG1_LOST_CHILD_PENDING);
            if (!pending) {
                info->mCode = MSG_MEET;
            } else {
                info->mCode = MSG_MEET_AGAIN;
            }
        }
        break;
    }
    case MSG_KIND_FELL:
        info->mCode = MSG_FELL;
        break;
    case MSG_KIND_FOLLOW:
        info->mCode = MSG_FOLLOW;
        break;
    case MSG_KIND_REUNION:
        info->mCode = MSG_REUNION;
        break;
    }
}

// .text 0x478C
void dAcNpcSpMissingChild_c::talk_c::onMessageStart(int arg) {
    // The town of the player she was met with.
    dSaveTown_c *town = dSaveData_c::getTown();
    setLandName(&town->mTownHost.getPID().land, 0);
}

// .text 0x47D0
void dAcNpcSpMissingChild_c::talk_c::onMessageEnd(int kind) {
    struct Entry {
        const char *label;
        int code;
        Method method;
    };
    static Entry messages[] = {
        {dANpcSpMissingChild::l_msgLabel, MSG_TAKE_HOME, &talk_c::startEscort},
        {dANpcSpMissingChild::l_msgLabel, MSG_REFUSE, &talk_c::stopEscort},
        {dANpcSpMissingChild::l_msgLabel, MSG_REUNION, &talk_c::endReunion},
    };
    u16 code = getMessageCode();
    const char *label;
    Entry *entry = messages;
    label = getMessageLabel();
    for (int i = 0; i < (int)ARRAY_SIZE(messages); i++, entry++) {
        if (std::strcmp(label, entry->label) == 0 && code == entry->code) {
            callMessageMethod(entry->method);
        }
    }
}

// .text 0x4890
void dAcNpcSpMissingChild_c::talk_c::startEscort() {
    dNpcInfo::gatekeeperA_c::get()->setEscorting(1);
}

// .text 0x48B8
void dAcNpcSpMissingChild_c::talk_c::stopEscort() {
    dNpcInfo::gatekeeperA_c::get()->setEscorting(0);
}

// .text 0x48E0
void dAcNpcSpMissingChild_c::talk_c::endReunion() {
    setRequest1(0);
}

// .text 0x48E8
void dAcNpcSpMissingChild_c::talk_c::onAnswer(int answer) {
}

// .text 0x48EC
void dAcNpcSpMissingChild_c::talk_c::preExecute() {
}

// .text 0x48F0
void dAcNpcSpMissingChild_c::talk_c::onTalkEnd() {
}

// .text 0x48F4
void dAcNpcSpMissingChild_c::talk_c::onRequestEnd(int kind) {
    if (mRequestEndMethod) {
        Method method = mRequestEndMethod;
        callMessageMethod(method);
        if (method == mRequestEndMethod) {
            mRequestEndMethod = NULL;
        }
    }
}

// .text 0x498C
void dAcNpcSpMissingChild_c::calcFrontPos(mVec3_c *pos, const mAng &angle,
                                          f32 dist) {
    mVec3_c base(mPos);
    pos->x = base.x + dist * angle.sin();
    pos->z = base.z + dist * angle.cos();
    mVec3_c ground(pos->x, base.y, pos->z);
    pos->y = dBGCF::getGroundY(&ground, 0);
}

// .text 0x4A6C
bool dAcNpcSpMissingChild_c::canInteract(dDemoActor_c *actor) {
    if (!dAcNpc_c::canInteract(actor)) {
        return false;
    }
    if (mTalk.isBusy()) {
        return false;
    }
    if (isCurrentMethod(&dAcNpcSpMissingChild_c::executeFall) == true) {
        return false;
    }
    return isCurrentMethod(&dAcNpcSpMissingChild_c::executeComeOutOfGate) !=
           true;
}

// .text 0x4B40
void dAcNpcSpMissingChild_c::stepDemoReunionNet_c::startGatekeeperSave() {
    dAcNpcSpGkA_c *gk = (dAcNpcSpGkA_c *)fManager_c::searchBaseByProfName(
        fProf::A_NPC_SP_GATEKEEPER_A, NULL);
    if (dNet::isHost()) {
        gk->startReunionNetHst();
    } else {
        gk->startReunionNetVst();
    }
    nextStep();
}

// .text 0x4C08
void dAcNpcSpMissingChild_c::stepDemoReunionNet_c::waitGatekeeperSave() {
    dAcNpcSpGkA_c *gk = (dAcNpcSpGkA_c *)fManager_c::searchBaseByProfName(
        fProf::A_NPC_SP_GATEKEEPER_A, NULL);
    if (gk->isModeNone()) {
        nextStep();
    }
}

// .text 0x4CB8
int dAcNpcSpMissingChild_c::getEarType() {
    return NPC_EAR_TYPE_MISSING_CHILD;
}

// .text 0x4CC0
void dAcNpcSpMissingChild_c::runToMotherSide() {
    const f32 speed = 1.35f;
    const f32 accel = 0.28f;
    const f32 decel = 0.4f;
    moveParam_c param;
    param.mSpeed = speed;
    param.mAccel = accel;
    param.mDecel = decel;
    mAction.requestRun(PRIORITY_DEMO,
                       mVec3_c(dANpcSpMissingChild::l_motherSidePos),
                       l_runTurnSpeed, 0, param, l_8074FDEC, cNpcMorphFrames);
}

// .text 0x4D6C
void dAcNpcSpMissingChild_c::runToMother() {
    const f32 speed = 1.35f;
    const f32 accel = 0.28f;
    const f32 decel = 0.4f;
    moveParam_c param;
    param.mSpeed = speed;
    param.mAccel = accel;
    param.mDecel = decel;
    mAction.requestRun(PRIORITY_DEMO, mVec3_c(dANpcSpMissingChild::l_motherPos),
                       l_runTurnSpeed, 0, param, l_8074FDEC, cNpcMorphFrames);
}

// ---- stepDemoBase_c (instantiated for stepDemoReunionNet_c at 0x4E18, for
// stepDemoReunion_c at 0x68BC) ----

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::startWalk() {
    int playerNo = getPlayerNo();
    dPlayerMgr::setScriptedMove(TRUE, playerNo);
    mVec3_c pos(dANpcSpMissingChild::l_playerWalkPos[0]);
    dPlayerMgr::moveTo(&pos, playerNo, dPlayerMgr::sDefaultMoveSpeed);
    this->getOwner()->walkToGate();
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::waitWalk() {
    if (this->getOwner()->mAction.mCanChange) {
        this->getOwner()->mLookAt.setFront(PRIORITY_IDLE, l_lookPitchStep,
                                           l_lookYawStep);
        this->getOwner()->mAction.requestWait(PRIORITY_DEMO);
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::waitPlayerWalk() {
    if (!dPlayerMgr::isMovingTo(getPlayerNo())) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::waitFadeIn() {
    if (mFader_c::isStatus(mFaderBase_c::HIDDEN)) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::finish() {
    this->mpOwner->endDemo();
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::startTalk() {
    this->getOwner()->mTalk.requestTalk(true, TRUE);
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::waitTalkSuspend() {
    if (dMsg::Comp_c::mInstance->isState(&dMsg::Comp_c::executeSuspend)) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::restartTalk() {
    dItem::Item key((u16)SP_NPC_MISSING_MOTHER);
    dAcNpc_c *mother = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(key));
    if (mother != NULL) {
        this->getOwner()->mChildTalk.setPartner(mother);
    }
    this->getOwner()->mChildTalk.reqMsgClose();
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::startSideMove() {
    dItem::Item key((u16)SP_NPC_MISSING_MOTHER);
    dAcNpc_c *mother = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(key));
    this->getOwner()->mLookAt.setActor(PRIORITY_DEMO, mother, true,
                                       l_lookPitchStep, l_lookYawStep,
                                       cNpcLookRange);
    this->getOwner()->runToMotherSide();
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::waitSideMove() {
    if (this->getOwner()->mAction.mCanChange) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::startReunion() {
    this->getOwner()->runToMother();
    dItem::Item key((u16)SP_NPC_MISSING_MOTHER);
    dAcNpc_c *mother = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(key));
    mVec3_c pos(dANpcSpMissingChild::l_motherRunPos);
    mother->mAction.requestRun(PRIORITY_DEMO, pos, l_runTurnSpeed, 0,
                               l_moveParamRun, l_8074FDEC, cNpcMorphFrames);
    mother->mLookAt.setActor(PRIORITY_DEMO, this->getOwner(), true,
                             l_lookPitchStep, l_lookYawStep, cNpcLookRange);
    dCamera_c::sInstance->mRequestedMode = dCamera_c::MODE_FIXED_VIEW_20;
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::turnChildToMother() {
    dItem::Item key((u16)SP_NPC_MISSING_MOTHER);
    dAcNpc_c *mother = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(key));
    if (this->getOwner()->mAction.mCanChange) {
        this->getOwner()->mAction.requestTurn(
            PRIORITY_DEMO,
            dActor_c::targetAngleY(this->getOwner()->getPosP(),
                                   mother->getPosP()),
            l_turnSpeed, 0);
        this->getOwner()->mLookAt.setActor(PRIORITY_DEMO, mother, true,
                                           l_lookPitchStep, l_lookYawStep,
                                           cNpcLookRange);
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::turnMotherToChild() {
    dItem::Item key((u16)SP_NPC_MISSING_MOTHER);
    dAcNpc_c *mother = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(key));
    if (mother->mAction.mCanChange) {
        mother->mAction.requestTurn(
            PRIORITY_DEMO,
            dActor_c::targetAngleY(mother->getPosP(),
                                   this->getOwner()->getPosP()),
            l_turnSpeed, 0);
        mother->mLookAt.setActor(PRIORITY_DEMO, this->getOwner(), true,
                                 l_lookPitchStep, l_lookYawStep, cNpcLookRange);
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::turnPlayer() {
    int playerNo = getPlayerNo();
    dPlayerMgr::getNpcTargetActor(playerNo);
    dItem::Item key((u16)SP_NPC_MISSING_MOTHER);
    dAcNpc_c *mother = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(key));
    dPlayerMgr::turnTo(dActor_c::targetAngleY(dPlayerMgr::getPosition(playerNo),
                                              mother->getPosP()),
                       playerNo);
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::finishReunion() {
    dItem::Item key((u16)SP_NPC_MISSING_MOTHER);
    dAcNpc_c *mother = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(key));
    if (this->getOwner()->mAction.mCanChange && mother->mAction.mCanChange) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::finishTalk() {
    if (!this->getOwner()->mTalk.isBusy()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::turnToMotherSide() {
    dItem::Item key((u16)SP_NPC_MISSING_MOTHER);
    dAcNpc_c *mother = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(key));
    mVec3_c exit(dANpcSpMissingChild::l_motherExitPos);
    mother->mAction.requestTurn(
        PRIORITY_DEMO, dActor_c::targetAngleY(mother->getPosP(), &exit),
        l_turnSpeed, 0);
    mVec3_c side(dANpcSpMissingChild::l_sidePos);
    this->getOwner()->mAction.requestTurn(
        PRIORITY_DEMO,
        dActor_c::targetAngleY(this->getOwner()->getPosP(), &side), l_turnSpeed,
        0);
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner,
                                            Step>::waitTurnToMotherSide() {
    if (this->getOwner()->mAction.mCanChange) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::moveToMotherSide() {
    dPlayerMgr::turnTo(0, getPlayerNo());
    mVec3_c side(dANpcSpMissingChild::l_sidePos);
    this->getOwner()->mAction.requestWalk(PRIORITY_DEMO, side, l_defaultAnmRate, l_walkTurnSpeed, 0,
        l_moveParamWalk, cNpcMorphFrames);
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::waitMotherTurn() {
    dItem::Item key((u16)SP_NPC_MISSING_MOTHER);
    if (static_cast<dAcNpc_c *>(dNpcList::getSpByKey(key))
            ->mAction.mCanChange) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner,
                                            Step>::waitMoveToMotherSide() {
    if (this->getOwner()->mAction.mCanChange) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::turnToExit() {
    mVec3_c exit(dANpcSpMissingChild::l_exitPos);
    this->getOwner()->mAction.requestTurn(
        PRIORITY_DEMO,
        dActor_c::targetAngleY(this->getOwner()->getPosP(), &exit), l_turnSpeed,
        0);
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::waitTurnToExit() {
    if (this->getOwner()->mAction.mCanChange) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::moveToExit() {
    dItem::Item key((u16)SP_NPC_MISSING_MOTHER);
    dAcNpc_c *mother = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(key));
    mother->mLookAt.mYaw.mMax = ANGLE_360_DIV(16);
    this->getOwner()->mLookAt.setActor(PRIORITY_DEMO, mother, true,
                                       l_lookPitchStep, l_lookYawStep,
                                       cNpcLookRange);
    this->getOwner()->mLookAt.mYaw.mMax = ANGLE_360_DIV(16);
    mVec3_c motherExit(dANpcSpMissingChild::l_motherExitPos);
    mother->mAction.requestWalk(PRIORITY_DEMO, motherExit, l_defaultAnmRate, l_walkTurnSpeed, 0,
        l_moveParamWalk, cNpcMorphFrames);
    mVec3_c exit(dANpcSpMissingChild::l_exitPos);
    this->getOwner()->mAction.requestWalk(PRIORITY_DEMO, exit, l_defaultAnmRate, l_walkTurnSpeed, 0,
        l_moveParamWalk, cNpcMorphFrames);
    mTimer = REUNION_EXIT_WALK_FRAMES;
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::smileChild() {
    if (mTimer != 0) {
        mTimer--;
    }
    if (mTimer == 0) {
        dAcNpcSpMissingChild_c *owner = this->getOwner();
        owner->getModel()->setFaceTex(owner, 0, "e.5");
        mTimer = REUNION_SMILE_DELAY_FRAMES;
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::smileMother() {
    if (mTimer != 0) {
        mTimer--;
    }
    if (mTimer == 0) {
        dItem::Item key((u16)SP_NPC_MISSING_MOTHER);
        dAcNpc_c *mother = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(key));
        mother->getModel()->setFaceTex(mother, 0, "e.5");
        mTimer = REUNION_SMILE_HOLD_FRAMES;
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::waitSmile() {
    if (mTimer != 0) {
        mTimer--;
    }
    if (mTimer == 0) {
        dItem::Item key((u16)SP_NPC_MISSING_MOTHER);
        dAcNpc_c *mother = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(key));
        mother->getModel()->setFaceTex(mother, 0, "e.0");
        mVec3_c exit(dANpcSpMissingChild::l_motherExitPos);
        mother->mLookAt.setPos(PRIORITY_DEMO, exit, true, l_lookPitchStep,
                               l_lookYawStep, cNpcLookRange);
        dAcNpcSpMissingChild_c *owner = this->getOwner();
        owner->getModel()->setFaceTex(owner, 0, "e.0");
        mTimer = REUNION_SCENE_CHANGE_DELAY_FRAMES;
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::waitSceneChange() {
    if (mTimer != 0) {
        mTimer--;
    }
    if (mTimer == 0) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::changeScene() {
    dANpcSpMissingChild::getSceneChange()->requestExit(Step::EXIT_IDX, 0, 0);
    this->nextStep();
}

template <class Owner, class Step>
int dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>::getPlayerNo() {
    int no = dSvMgr_c::mCurrentVisitor;
    if (no == NET_MEMBER_NONE) {
        no = dNet::getMyNo();
    }
    return no;
}
