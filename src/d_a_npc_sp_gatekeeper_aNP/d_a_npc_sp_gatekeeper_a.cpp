#include <game/game/d_a_npc_sp_gatekeeper_a.hpp>
#include <game/framework/f_profile.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_a_npc_sp_missing_child.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_a_rmobj_checkp.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_ds_dl.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_home_button.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_nand.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_net_dwc.hpp>
#include <game/game/d_net_mgr.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_def.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_npc_list.hpp>
#include <game/game/d_obj_chat.hpp>
#include <game/game/d_obj_telop.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <game/game/d_travel_data.hpp>
#include <game/game/d_wifi.hpp>
#include <game/mLib/m_fader.hpp>
#include <game/sLib/s_lib.hpp>
#include <cstring>
#include <revolution/sc.h>

// Copper the gatekeeper (scaffold). Functions are in target .text order; a
// "// .text:" comment marks every function of 0x5C..0xD0C8 (written or not).
// The template instantiations from 0xD0C8 on are listed at the end of the
// file. See the header for the class layout and the plan of the TU for the
// address -> function map.

namespace dANpcSpGatekeeperA {
// .text:0x5C fn_66_5C (0xC)
static dScene::dSceneChange_c *getSceneChange() {
    return &dScene::gSceneChange;
}
// .text:0x68 fn_66_68 (0x1C): a net error is pending
static BOOL isNetError() {
    return dNetMgr_c::sInstance->mInfo.mError.mState != 0;
}
// .text:0x84 fn_66_84 (0x10): the error code (shown in the NetworkError
// message)
static int getNetErrorCode() {
    return dNetMgr_c::sInstance->mInfo.mError.mDetail;
}
// .text:0x94 fn_66_94 (0x10)
static int getNetErrorKind() {
    return dNetMgr_c::sInstance->mInfo.mError.mKind;
}
// Joined as a guest and the host (member 0) is still connected (inlined).
inline BOOL isHostConnected() {
    dNetMgr_c *mgr = dNetMgr_c::sInstance;
    return mgr->getMyNo() != 0 && mgr->isConnected(0);
}

// Where the gatekeeper walks the player (.rodata 0x0, 0xC, 0x18): inside the
// gate, in front of the gate and outside of it.
static const nw4r::math::_VEC3 l_enterPos = {256.0f, 0.0f, 304.0f};
static const nw4r::math::_VEC3 l_gatePos = {256.0f, 0.0f, 336.0f};
static const nw4r::math::_VEC3 l_exitPos = {256.0f, 0.0f, 48.0f};

// The label of the gatekeeper's messages (.data 0x18; the string is .data 0x0).
const char *l_msgLabel = "NPC_sp/NPC_gatekeeperA";

// .text:0xA4 fn_66_A4 (0x208): construct
BASE_PROFILE(A_NPC_SP_GATEKEEPER_A, dAcNpcSpGkA_c);

// The travel data of a DS trip (.bss 0x8): allocated by create, freed by
// doDelete.
static dTravelData::Data_c *l_travelData;
} // namespace dANpcSpGatekeeperA

// .text:0x2AC fn_66_2AC (0xAC): dAcNpcSpGkA_c::preCreate()
int dAcNpcSpGkA_c::preCreate() {
    if (dScene::getCurrentScene() == dScene::SCENE_DM_CHKP_MISS ||
        dScene::getCurrentScene() == dScene::SCENE_DM_CHKP_CNNCT ||
        dScene::getCurrentScene() == dScene::SCENE_DM_CHKP_DCNNCT ||
        !isMultiPlay()) {
        clearNetSync();
    }
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mTalkGk);
    mAnmSet.setAnm0(HMN_ANM_PLC_WAIT1);
    mAnmSet.setAnm1(HMN_ANM_PLC_WALK1);
    return 1;
}

// .text:0x358 fn_66_358 (0x838): dAcNpcSpGkA_c::create()
int dAcNpcSpGkA_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    static const dState::mode_c<dAcNpcSpGkA_c>::Entry modes[] = {
        {"待機", &dAcNpcSpGkA_c::initWait, &dAcNpcSpGkA_c::executeWait},
        {"デモ待機", &dAcNpcSpGkA_c::initDemoWait,
         &dAcNpcSpGkA_c::executeDemoWait},
        {"会話", &dAcNpcSpGkA_c::initTalk, &dAcNpcSpGkA_c::executeTalk},
        {"初期角度に復帰", &dAcNpcSpGkA_c::initTurnBack,
         &dAcNpcSpGkA_c::executeTurnBack},
        {"処理無し", &dAcNpcSpGkA_c::initNone, &dAcNpcSpGkA_c::executeNone},
        {"デモ：ネットホストの門を開く", &dAcNpcSpGkA_c::initOpenGateNetHst,
         &dAcNpcSpGkA_c::executeOpenGateNetHst},
        {"デモ：ネットホストの門を閉じる", &dAcNpcSpGkA_c::initCloseGateNetHst,
         &dAcNpcSpGkA_c::executeCloseGateNetHst},
        {"デモ：ネットビジターとして通信開始シーケンス",
         &dAcNpcSpGkA_c::initGoOutNetVst, &dAcNpcSpGkA_c::executeGoOutNetVst},
        {"デモ：新規参入受け入れシーケンス", &dAcNpcSpGkA_c::initComeInNet,
         &dAcNpcSpGkA_c::executeComeInNet},
        {"デモ：迷子ネコ：ネットホストの新規参入受け入れシーケンス",
         &dAcNpcSpGkA_c::initReunionNetHst,
         &dAcNpcSpGkA_c::executeReunionNetHst},
        {"デモ：迷子ネコ：ネットビジターの新規参入受け入れシーケンス",
         &dAcNpcSpGkA_c::initReunionNetVst,
         &dAcNpcSpGkA_c::executeReunionNetVst},
        {"デモ：退場（本人）", &dAcNpcSpGkA_c::initRetireNetVst,
         &dAcNpcSpGkA_c::executeRetireNetVst},
        {"デモ：退場（見送り）", &dAcNpcSpGkA_c::initSeeOffNetHst,
         &dAcNpcSpGkA_c::executeSeeOffNetHst},
        {"デモ：退場（見送り）", &dAcNpcSpGkA_c::initSeeOffNetVst,
         &dAcNpcSpGkA_c::executeSeeOffNetVst},
        {"デモ：DSアプリでおでかけ", &dAcNpcSpGkA_c::initGoOutDsAppli,
         &dAcNpcSpGkA_c::executeGoOutDsAppli},
        {"デモ：DSアプリで受け入れ", &dAcNpcSpGkA_c::initComeInDsAppli,
         &dAcNpcSpGkA_c::executeComeInDsAppli},
        {"通信：リモート", &dAcNpcSpGkA_c::initRemote,
         &dAcNpcSpGkA_c::executeRemote},
        {"通信：リモート会話", &dAcNpcSpGkA_c::initRemoteTalk,
         &dAcNpcSpGkA_c::executeRemoteTalk},
        {"デモ：プレイヤ引き戻し", &dAcNpcSpGkA_c::initBringBack,
         &dAcNpcSpGkA_c::executeBringBack},
    };
    dState::mode_c<dAcNpcSpGkA_c>::initialize("dAcNpcSpGkA_c", modes,
                                              ARRAY_SIZE(modes));
    if (dScene::getCurrentScene() == dScene::SCENE_DM_CHKP_CNNCT) {
        if (isNetActive()) {
            changeMode(&dAcNpcSpGkA_c::executeComeInNet);
        } else {
            changeMode(&dAcNpcSpGkA_c::executeComeInDsAppli);
        }
        dDemo::onHoldStart();
    } else if (dScene::getCurrentScene() == dScene::SCENE_DM_CHKP_DCNNCT) {
        if (isPlayerFromThisTown()) {
            changeMode(&dAcNpcSpGkA_c::executeSeeOffNetHst);
        } else {
            changeMode(&dAcNpcSpGkA_c::executeSeeOffNetVst);
        }
        dDemo::onHoldStart();
    } else if (isLocalOwner()) {
        changeMode(&dAcNpcSpGkA_c::executeWait);
    } else {
        changeMode(&dAcNpcSpGkA_c::executeRemote);
    }
    dANpcSpGatekeeperA::l_travelData =
        (dTravelData::Data_c *)dHeap::localHeap_p->alloc(
            sizeof(dTravelData::Data_c), 0x20);
    new (dANpcSpGatekeeperA::l_travelData) dTravelData::Data_c;
    return 1;
}

// .text:0xB90 fn_66_B90 (0xC4): dAcNpcSpGkA_c::doDelete()
int dAcNpcSpGkA_c::doDelete() {
    if (!dNand::isIdle()) {
        return 0;
    }
    if (!dAcNpcSp_c::doDelete()) {
        return 0;
    }
    if (dScene::getCurrentScene() == dScene::SCENE_DM_CHKP_CNNCT) {
        dDemo::offHoldStart();
    } else if (dScene::getCurrentScene() == dScene::SCENE_DM_CHKP_DCNNCT) {
        dDemo::offHoldStart();
    }
    if (dANpcSpGatekeeperA::l_travelData != NULL) {
        dHeap::localHeap_p->free(dANpcSpGatekeeperA::l_travelData);
        dANpcSpGatekeeperA::l_travelData = NULL;
    }
    dHomeButton_c::getInstance()->mFlags &= ~dHomeButton_c::FLAG_SAVING;
    return 1;
}

// .text:0xC54 fn_66_C54 (0x64): dAcNpcSpGkA_c::vtB0()
int dAcNpcSpGkA_c::vtB0() {
    if (mDsDlActive) {
        dDsDl::execute();
    }
    dState::base_c<dAcNpcSpGkA_c>::execute();
    return 1;
}

// .text:0xCB8 fn_66_CB8 (0x8E8): dAcNpcSpGkA_c::onDemoCmd(int cmd, u8 member)
void dAcNpcSpGkA_c::onDemoCmd(int cmd, u8 member) {
    const int originalCmd = cmd;
    _1B70 = member;
    int permit = requestPermit(cmd, member, TRUE);
    switch (cmd) {
    case DEMO_CMD_START:
        if (permit == PERMIT_REMOTE_START || permit == PERMIT_LOCAL_START) {
            changeMode(&dAcNpcSpGkA_c::executeDemoWait);
        }
        break;
    case DEMO_CMD_TALK:
        if (permit == PERMIT_REMOTE_TALK) {
            changeMode(&dAcNpcSpGkA_c::executeRemoteTalk);
        } else if (permit == PERMIT_LOCAL_TALK) {
            mTalkGk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeMode(&dAcNpcSpGkA_c::executeTalk);
        }
        break;
    case DEMO_CMD_SPEAK:
        mTalkGk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeMode(&dAcNpcSpGkA_c::executeTalk);
        break;
    case DEMO_CMD_END:
        if (permit == PERMIT_END_SINGLE) {
            changeMode(&dAcNpcSpGkA_c::executeTurnBack);
        } else if (permit == PERMIT_END_MULTI) {
            changeMode(&dAcNpcSpGkA_c::executeRemote);
        }
        break;
    case CMD_RELEASE_PERMIT:
        if (permit == PERMIT_RELEASED) {
            changeMode(&dAcNpcSpGkA_c::executeTurnBack);
        }
        break;
    }
    dAcNpcSp_c::onDemoCmd(originalCmd, member);
}

// .text:0x15A0 fn_66_15A0 (0x5C): dAcNpcSpGkA_c::canSpeak(dDemoActor_c *actor)
int dAcNpcSpGkA_c::canSpeak(dDemoActor_c *actor) {
    if (actor == NULL) {
        return 0;
    }
    if (isMultiPlay()) {
        return 0;
    }
    return !mTalk.isBusy();
}

// .text:0x15FC fn_66_15FC (0x1D4): dAcNpcSpGkA_c::getSoundId()
int dAcNpcSpGkA_c::getSoundId() {
    if (isCurrentMethod(&dAcNpcSpGkA_c::executeReunionNetVst) ||
        isCurrentMethod(&dAcNpcSpGkA_c::executeReunionNetHst) ||
        isCurrentMethod(&dAcNpcSpGkA_c::executeComeInNet) ||
        isCurrentMethod(&dAcNpcSpGkA_c::executeSeeOffNetHst) ||
        isCurrentMethod(&dAcNpcSpGkA_c::executeSeeOffNetVst) ||
        isCurrentMethod(&dAcNpcSpGkA_c::executeComeInDsAppli)) {
        return 0xFFFF;
    }
    return dNpcInfo::getSpSound(&mNpcItem);
}

// .text:0x17D0 fn_66_17D0 (0x14): dAcNpcSpGkA_c::initWait()
void dAcNpcSpGkA_c::initWait() {
    mAction.requestWait(PRIORITY_IDLE);
}

// .text:0x17E4 fn_66_17E4 (0x58): dAcNpcSpGkA_c::executeWait()
void dAcNpcSpGkA_c::executeWait() {
    if (isPlayerInGate() && !dDemo::isType(dDemo::TYPE_ANY)) {
        mAutoTalk = 1;
        dDemo::requestSpeak(this, NULL);
    }
}

// .text:0x183C fn_66_183C (0x4): dAcNpcSpGkA_c::initDemoWait()
void dAcNpcSpGkA_c::initDemoWait() {
}

// .text:0x1840 fn_66_1840 (0x4): dAcNpcSpGkA_c::executeDemoWait()
void dAcNpcSpGkA_c::executeDemoWait() {
}

// .text:0x1844 fn_66_1844 (0x78): dAcNpcSpGkA_c::initTalk()
void dAcNpcSpGkA_c::initTalk() {
    if (!mTalk.isBusy()) {
        mAng angle = getTalkAngle();
        mAng speed = getTalkTurnSpeed();
        mTalk.requestTurnTalk(angle, speed, FALSE);
    }
}

// .text:0x18BC fn_66_18BC (0x40): dAcNpcSpGkA_c::executeTalk()
void dAcNpcSpGkA_c::executeTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
    }
}

// .text:0x18FC fn_66_18FC (0x48): dAcNpcSpGkA_c::initTurnBack()
void dAcNpcSpGkA_c::initTurnBack() {
    mAng angle(0);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0);
}

// .text:0x1944 fn_66_1944 (0x174): dAcNpcSpGkA_c::executeTurnBack()
void dAcNpcSpGkA_c::executeTurnBack() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        changeMode(&dAcNpcSpGkA_c::executeWait);
    }
}

// .text:0x1AB8 fn_66_1AB8 (0x4): dAcNpcSpGkA_c::initNone()
void dAcNpcSpGkA_c::initNone() {
}

// .text:0x1ABC fn_66_1ABC (0x4): dAcNpcSpGkA_c::executeNone()
void dAcNpcSpGkA_c::executeNone() {
}

// .text:0x1AC0 fn_66_1AC0 (0x4): dAcNpcSpGkA_c::initRemote()
void dAcNpcSpGkA_c::initRemote() {
}

// .text:0x1AC4 fn_66_1AC4 (0x2D4): dAcNpcSpGkA_c::executeRemote()
void dAcNpcSpGkA_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mTalkGk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeMode(&dAcNpcSpGkA_c::executeTalk);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeMode(&dAcNpcSpGkA_c::executeTurnBack);
        }
    }
}

// .text:0x1D98 fn_66_1D98 (0x4): dAcNpcSpGkA_c::initRemoteTalk()
void dAcNpcSpGkA_c::initRemoteTalk() {
}

// .text:0x1D9C fn_66_1D9C (0x17C): dAcNpcSpGkA_c::executeRemoteTalk()
void dAcNpcSpGkA_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeMode(&dAcNpcSpGkA_c::executeTurnBack);
    }
}

// .text:0x1F18 fn_66_1F18 (0x118): dAcNpcSpGkA_c::initOpenGateNetHst()
void dAcNpcSpGkA_c::initOpenGateNetHst() {
    static const stepDemoOpenGateNetHst_c::Entry steps[] = {
        {"会話終了まで待つ",
         (stepDemoOpenGateNetHst_c::Method)&stepDemoOpenGateNetHst_c::
             waitTalkEnd},
        {"門を開く",
         (stepDemoOpenGateNetHst_c::Method)&stepDemoOpenGateNetHst_c::openGate},
        {"親の開始", (stepDemoOpenGateNetHst_c::
                          Method)&stepDemoOpenGateNetHst_c::startParent},
        {"門が開くまで待つ",
         (stepDemoOpenGateNetHst_c::Method)&stepDemoOpenGateNetHst_c::
             waitGateOpen},
        {"会話デモ終了", (stepDemoOpenGateNetHst_c::
                              Method)&stepDemoOpenGateNetHst_c::finishTalkDemo},
    };
    mStepDemoOpenGateNetHst.initialize(
        this, "dAcNpcSpGkA_c::stepDemoOpenGateNetHst_c", steps,
        ARRAY_SIZE(steps));
}

// .text:0x2030 fn_66_2030 (0x48): dAcNpcSpGkA_c::executeOpenGateNetHst()
void dAcNpcSpGkA_c::executeOpenGateNetHst() {
    mStepDemoOpenGateNetHst.execute();
}

// .text:0x2078 fn_66_2078 (0xB0):
// dAcNpcSpGkA_c::stepDemoOpenGateNetHst_c::startParent() - "親の開始"
void dAcNpcSpGkA_c::stepDemoOpenGateNetHst_c::startParent() {
    if (dNet::startHost()) {
        dNpc::takeMovedAnimalToBuffer(1);
        dNpc::initPermitManagers();
        nextStep();
    }
}

// .text:0x2128 fn_66_2128 (0x8):
// dAcNpcSpGkA_c::stepDemoOpenGateNetHst_c::finishTalkDemo() - "会話デモ終了"
void dAcNpcSpGkA_c::stepDemoOpenGateNetHst_c::finishTalkDemo() {
    dDemo::finishTalk(mpOwner);
}

// .text:0x2130 fn_66_2130 (0xFC): dAcNpcSpGkA_c::initCloseGateNetHst()
void dAcNpcSpGkA_c::initCloseGateNetHst() {
    static const stepDemoCloseGateNetHst_c::Entry steps[] = {
        {"会話終了まで待つ",
         (stepDemoCloseGateNetHst_c::Method)&stepDemoCloseGateNetHst_c::
             waitTalkEnd},
        {"門を閉じる", (stepDemoCloseGateNetHst_c::
                            Method)&stepDemoCloseGateNetHst_c::closeGate},
        {"門が閉じるまで待つ",
         (stepDemoCloseGateNetHst_c::Method)&stepDemoCloseGateNetHst_c::
             waitGateClose},
        {"会話デモ終了",
         (stepDemoCloseGateNetHst_c::Method)&stepDemoCloseGateNetHst_c::
             finishTalkDemo},
    };
    mStepDemoCloseGateNetHst.initialize(
        this, "dAcNpcSpGkA_c::stepDemoCloseGateNetHst_c", steps,
        ARRAY_SIZE(steps));
}

// .text:0x222C fn_66_222C (0x48): dAcNpcSpGkA_c::executeCloseGateNetHst()
void dAcNpcSpGkA_c::executeCloseGateNetHst() {
    mStepDemoCloseGateNetHst.execute();
}

// .text:0x2274 fn_66_2274 (0x9C):
// dAcNpcSpGkA_c::stepDemoCloseGateNetHst_c::closeGate() - "門を閉じる"
void dAcNpcSpGkA_c::stepDemoCloseGateNetHst_c::closeGate() {
    dARmobjCheckp::closeGate();
    nextStep();
}

// .text:0x2310 fn_66_2310 (0xA4):
// dAcNpcSpGkA_c::stepDemoCloseGateNetHst_c::waitGateClose() -
// "門が閉じるまで待つ"
void dAcNpcSpGkA_c::stepDemoCloseGateNetHst_c::waitGateClose() {
    if (dARmobjCheckp::isGateClosed()) {
        nextStep();
    }
}

// .text:0x23B4 fn_66_23B4 (0x8):
// dAcNpcSpGkA_c::stepDemoCloseGateNetHst_c::finishTalkDemo() - "会話デモ終了"
void dAcNpcSpGkA_c::stepDemoCloseGateNetHst_c::finishTalkDemo() {
    dDemo::finishTalk(mpOwner);
}

// .text:0x23BC fn_66_23BC (0x328): dAcNpcSpGkA_c::initGoOutNetVst()
void dAcNpcSpGkA_c::initGoOutNetVst() {
    static const stepDemoGoOutNetVst_c::Entry steps[] = {
        {"会話サスペンドまで待つ",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::
             waitTalkSuspend},
        {"手続きメッセージ開始",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::
             startProcedureMsg},
        {"もんばん前を向く",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::faceFront},
        {"門を開く",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::openGate},
        {"門の手前までプレイヤ移動開始",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::
             startPlayerMoveToGate},
        {"門の手前までプレイヤ移動完了",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::
             waitPlayerMoveToGate},
        {"門の方向へプレイヤ回転",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::
             turnPlayerToGate},
        {"もんばん横を向く初期化",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::initFaceSide},
        {"もんばん横を向く",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::faceSide},
        {"もんばん横を向く完了",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::waitFaceSide},
        {"門の方向へプレイヤ回転完了",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::
             waitPlayerTurnToGate},
        {"門が開くまで待つ",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::waitGateOpen},
        {"デモ完了！！",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::demoDone},
        {"会話終了まで待つ",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::waitTalkEnd},
        {"チャット閉じる",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::closeChat},
        {"プレイヤが門の外へ出る開始",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::
             startPlayerExit},
        {"もんばん敬礼初期化",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::initSalute},
        {"もんばん敬礼",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::salute},
        {"門の外のシーンへ",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::toOutsideScene},
        {"終了", (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::finish},
    };
    mStepDemoGoOutNetVst.initialize(
        this, "dAcNpcSpGkA_c::stepDemoGoOutNetVst_c", steps, ARRAY_SIZE(steps));
}

// .text:0x26E4 fn_66_26E4 (0x48): dAcNpcSpGkA_c::executeGoOutNetVst()
void dAcNpcSpGkA_c::executeGoOutNetVst() {
    mStepDemoGoOutNetVst.execute();
}

// .text:0x272C fn_66_272C (0x370):
// dAcNpcSpGkA_c::stepDemoGoOutNetVst_c::startPlayerExit() -
// "プレイヤが門の外へ出る開始"
void dAcNpcSpGkA_c::stepDemoGoOutNetVst_c::startPlayerExit() {
    startExitMove(0);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player->mHost.mFlagB) {
        dItem::Item key((u16)SP_NPC_MISSING_CHILD);
        dAcNpcSpMissingChild_c *child =
            static_cast<dAcNpcSpMissingChild_c *>(dNpcList::getSpByKey(key));
        if (player->mHost.mPID.land == dSaveData_c::getTown()->mLandID) {
            child->startFollowRun();
        } else {
            child->startFall();
        }
    }
    nextStep();
}

// .text:0x2A9C fn_66_2A9C (0x18C):
// dAcNpcSpGkA_c::stepDemoGoOutNetVst_c::toOutsideScene() - "門の外のシーンへ"
void dAcNpcSpGkA_c::stepDemoGoOutNetVst_c::toOutsideScene() {
    dPrivateHost_c *host;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    host = &player->mHost;
    if (host->mFlagB) {
        dNpcInfo::gatekeeperA_c::get()->clear();
        if (host->mPID.land == dSaveData_c::getTown()->mLandID) {
            host->clear();
            player->setFlag0(PRIVATE_FLAG0_LOST_CHILD_RETURNED);
            player->setFlag0(PRIVATE_FLAG0_LOST_CHILD_THANKS);
            player->clearFlag1(PRIVATE_FLAG1_LOST_CHILD_PENDING);
            dScene::requestSceneChange(dScene::SCENE_DM_CHKP_MISS, 0, 0);
        } else {
            dANpcSpGatekeeperA::getSceneChange()->requestExit(0, 0, 0);
        }
    } else {
        dANpcSpGatekeeperA::getSceneChange()->requestExit(0, 0, 0);
    }
    nextStep();
    dObjTelop_c::setNetCheckAll();
}

// .text:0x2C28 fn_66_2C28 (0xBC):
// dAcNpcSpGkA_c::stepDemoGoOutNetVst_c::startProcedureMsg() -
// "手続きメッセージ開始"
void dAcNpcSpGkA_c::stepDemoGoOutNetVst_c::startProcedureMsg() {
    mpOwner->mTalk.mNoMouth = true;
    mpOwner->mTalkGk.clearSpeakerName();
    mpOwner->mTalkGk.reqMsgClose();
    nextStep();
}

// .text:0x2CE4 fn_66_2CE4 (0x29C): dAcNpcSpGkA_c::initReunionNetHst()
void dAcNpcSpGkA_c::initReunionNetHst() {
    static const stepDemoReunionNetHst_c::Entry steps[] = {
        {"受け入れメッセージ開始",
         (stepDemoReunionNetHst_c::Method)&stepDemoReunionNetHst_c::
             startWelcomeMsg},
        {"ロック開始まで待つ",
         (stepDemoReunionNetHst_c::Method)&stepDemoReunionNetHst_c::
             waitLockStart},
        {"初期化処理", (stepDemoReunionNetHst_c::
                            Method)&stepDemoReunionNetHst_c::setupVisitor},
        {"「相手が持っている全追加データ」の受け取りが終わっているか？",
         (stepDemoReunionNetHst_c::Method)&stepDemoReunionNetHst_c::
             waitItemsReceived},
        {"追加データセーブ初期化",
         (stepDemoReunionNetHst_c::Method)&stepDemoReunionNetHst_c::
             initializeItemSave},
        {"追加データセーブ開始",
         (stepDemoReunionNetHst_c::Method)&stepDemoReunionNetHst_c::
             startItemSave},
        {"追加データセーブ終了",
         (stepDemoReunionNetHst_c::Method)&stepDemoReunionNetHst_c::
             waitItemSave},
        {"「全データ」の送信が終わっているか？",
         (stepDemoReunionNetHst_c::Method)&stepDemoReunionNetHst_c::
             waitJoinSent},
        {"全マシンネットデモが終了可能か？",
         (stepDemoReunionNetHst_c::Method)&stepDemoReunionNetHst_c::
             waitJoinDoneAll},
        {"「お出かけデモの終了を送信したい」をセット",
         (stepDemoReunionNetHst_c::Method)&stepDemoReunionNetHst_c::
             sendFinishJoin},
        {"「お出かけデモの終了命令」を送信し終わったか？",
         (stepDemoReunionNetHst_c::Method)&stepDemoReunionNetHst_c::
             waitJoinFinished},
        {"アイコンとロック解除",
         (stepDemoReunionNetHst_c::Method)&stepDemoReunionNetHst_c::
             releaseIconAndLock},
        {"会話終了待ち", (stepDemoReunionNetHst_c::
                              Method)&stepDemoReunionNetHst_c::waitTalkEnd},
        {"終了",
         (stepDemoReunionNetHst_c::Method)&stepDemoReunionNetHst_c::finish},
        {"セーブ失敗",
         (stepDemoReunionNetHst_c::Method)&stepDemoReunionNetHst_c::saveFailed},
    };
    mStepDemoReunionNetHst.initialize(this,
                                      "dAcNpcSpGkA_c::stepDemoReunionNetHst_c",
                                      steps, ARRAY_SIZE(steps));
}

// .text:0x2F80 fn_66_2F80 (0x48): dAcNpcSpGkA_c::executeReunionNetHst()
void dAcNpcSpGkA_c::executeReunionNetHst() {
    mStepDemoReunionNetHst.execute();
}

// .text:0x2FC8 fn_66_2FC8 (0xA4):
// dAcNpcSpGkA_c::stepDemoReunionNetHst_c::setupVisitor() - "初期化処理"
void dAcNpcSpGkA_c::stepDemoReunionNetHst_c::setupVisitor() {
    dSvMgr_c::addVisitor(dSvMgr_c::mCurrentVisitor);
    nextStep();
}

// .text:0x306C fn_66_306C (0x29C): dAcNpcSpGkA_c::initComeInNet()
void dAcNpcSpGkA_c::initComeInNet() {
    static const stepDemoComeInNet_c::Entry steps[] = {
        {"フェードイン終了待ち",
         (stepDemoComeInNet_c::Method)&stepDemoComeInNet_c::waitFadeIn},
        {"受け入れメッセージ開始",
         (stepDemoComeInNet_c::Method)&stepDemoComeInNet_c::startWelcomeMsg},
        {"BGM音量維持",
         (stepDemoComeInNet_c::Method)&stepDemoComeInNet_c::keepBgmVolume},
        {"プレイヤが門の中に入る開始",
         (stepDemoComeInNet_c::Method)&stepDemoComeInNet_c::startPlayerEnter},
        {"プレイヤが指定時間歩いたか？",
         (stepDemoComeInNet_c::Method)&stepDemoComeInNet_c::waitPlayerWalkTime},
        {"もんばん横を向く初期化",
         (stepDemoComeInNet_c::Method)&stepDemoComeInNet_c::initFaceSide},
        {"もんばん横を向く",
         (stepDemoComeInNet_c::Method)&stepDemoComeInNet_c::faceSide},
        {"もんばん横を向く完了",
         (stepDemoComeInNet_c::Method)&stepDemoComeInNet_c::waitFaceSide},
        {"プレイヤ移動完了",
         (stepDemoComeInNet_c::Method)&stepDemoComeInNet_c::playerMoveDone},
        {"デモ完了！！",
         (stepDemoComeInNet_c::Method)&stepDemoComeInNet_c::demoDone},
        {"会話終了待ち",
         (stepDemoComeInNet_c::Method)&stepDemoComeInNet_c::waitTalkEnd},
        {"もんばん敬礼初期化",
         (stepDemoComeInNet_c::Method)&stepDemoComeInNet_c::initSalute},
        {"もんばん敬礼",
         (stepDemoComeInNet_c::Method)&stepDemoComeInNet_c::salute},
        {"元のシーンへ",
         (stepDemoComeInNet_c::Method)&stepDemoComeInNet_c::toPrevScene},
        {"終了", (stepDemoComeInNet_c::Method)&stepDemoComeInNet_c::finish},
    };
    mStepDemoComeInNet.initialize(this, "dAcNpcSpGkA_c::stepDemoComeInNet_c",
                                  steps, ARRAY_SIZE(steps));
}

// .text:0x3308 fn_66_3308 (0x48): dAcNpcSpGkA_c::executeComeInNet()
void dAcNpcSpGkA_c::executeComeInNet() {
    mStepDemoComeInNet.execute();
}

// .text:0x3350 fn_66_3350 (0xA8):
// dAcNpcSpGkA_c::stepDemoComeInNet_c::startPlayerEnter() -
// "プレイヤが門の中に入る開始"
void dAcNpcSpGkA_c::stepDemoComeInNet_c::startPlayerEnter() {
    startEnterMove(dSvMgr_c::mCurrentVisitor);
    nextStep();
    dObjTelop_c::setNetCheckHost();
}

// .text:0x33F8 fn_66_33F8 (0xAC):
// dAcNpcSpGkA_c::stepDemoComeInNet_c::playerMoveDone() - "プレイヤ移動完了"
void dAcNpcSpGkA_c::stepDemoComeInNet_c::playerMoveDone() {
    if (isPlayerMoveDone(dSvMgr_c::mCurrentVisitor)) {
        nextStep();
    }
}

// .text:0x34A4 fn_66_34A4 (0x280): dAcNpcSpGkA_c::initReunionNetVst()
void dAcNpcSpGkA_c::initReunionNetVst() {
    static const stepDemoReunionNetVst_c::Entry steps[] = {
        {"受け入れメッセージ開始",
         (stepDemoReunionNetVst_c::Method)&stepDemoReunionNetVst_c::
             startWelcomeMsg},
        {"ロック開始まで待つ",
         (stepDemoReunionNetVst_c::Method)&stepDemoReunionNetVst_c::
             waitLockStart},
        {"「相手が持っている全追加データ」の受け取りが終わっているか？",
         (stepDemoReunionNetVst_c::Method)&stepDemoReunionNetVst_c::
             waitItemsReceived},
        {"追加データセーブ初期化",
         (stepDemoReunionNetVst_c::Method)&stepDemoReunionNetVst_c::
             initializeItemSave},
        {"追加データセーブ開始",
         (stepDemoReunionNetVst_c::Method)&stepDemoReunionNetVst_c::
             startItemSave},
        {"追加データセーブ終了",
         (stepDemoReunionNetVst_c::Method)&stepDemoReunionNetVst_c::
             waitItemSave},
        {"「全データ」の送信が終わっているか？",
         (stepDemoReunionNetVst_c::Method)&stepDemoReunionNetVst_c::
             waitJoinSent},
        {"「お出かけデモの終了を送信したい」をセット",
         (stepDemoReunionNetVst_c::Method)&stepDemoReunionNetVst_c::
             sendFinishJoin},
        {"「ネットデモの終了命令」が来ていたら",
         (stepDemoReunionNetVst_c::Method)&stepDemoReunionNetVst_c::
             waitJoinAllDone},
        {"アイコンとロック解除",
         (stepDemoReunionNetVst_c::Method)&stepDemoReunionNetVst_c::
             releaseIconAndLock},
        {"会話終了待ち", (stepDemoReunionNetVst_c::
                              Method)&stepDemoReunionNetVst_c::waitTalkEnd},
        {"終了",
         (stepDemoReunionNetVst_c::Method)&stepDemoReunionNetVst_c::finish},
        {"セーブ失敗",
         (stepDemoReunionNetVst_c::Method)&stepDemoReunionNetVst_c::saveFailed},
    };
    mStepDemoReunionNetVst.initialize(this,
                                      "dAcNpcSpGkA_c::stepDemoReunionNetVst_c",
                                      steps, ARRAY_SIZE(steps));
}

// .text:0x3724 fn_66_3724 (0x48): dAcNpcSpGkA_c::executeReunionNetVst()
void dAcNpcSpGkA_c::executeReunionNetVst() {
    mStepDemoReunionNetVst.execute();
}

// .text:0x376C fn_66_376C (0x29C): dAcNpcSpGkA_c::initRetireNetVst()
void dAcNpcSpGkA_c::initRetireNetVst() {
    static const stepDemoRetireNetVst_c::Entry steps[] = {
        {"会話終了待ち",
         (stepDemoRetireNetVst_c::Method)&stepDemoRetireNetVst_c::waitTalkEnd},
        {"チャット閉じる",
         (stepDemoRetireNetVst_c::Method)&stepDemoRetireNetVst_c::closeChat},
        {"もんばん前を向く",
         (stepDemoRetireNetVst_c::Method)&stepDemoRetireNetVst_c::faceFront},
        {"門の手前までプレイヤ移動開始",
         (stepDemoRetireNetVst_c::Method)&stepDemoRetireNetVst_c::
             startPlayerMoveToGate},
        {"門の手前までプレイヤ移動完了",
         (stepDemoRetireNetVst_c::Method)&stepDemoRetireNetVst_c::
             waitPlayerMoveToGate},
        {"門の方向へプレイヤ回転",
         (stepDemoRetireNetVst_c::Method)&stepDemoRetireNetVst_c::
             turnPlayerToGate},
        {"もんばん横を向く初期化",
         (stepDemoRetireNetVst_c::Method)&stepDemoRetireNetVst_c::initFaceSide},
        {"もんばん横を向く",
         (stepDemoRetireNetVst_c::Method)&stepDemoRetireNetVst_c::faceSide},
        {"もんばん横を向く完了",
         (stepDemoRetireNetVst_c::Method)&stepDemoRetireNetVst_c::waitFaceSide},
        {"門の方向へプレイヤ回転完了",
         (stepDemoRetireNetVst_c::Method)&stepDemoRetireNetVst_c::
             waitPlayerTurnToGate},
        {"プレイヤが門の外へ出る開始",
         (stepDemoRetireNetVst_c::Method)&stepDemoRetireNetVst_c::
             startPlayerExit},
        {"もんばん敬礼初期化",
         (stepDemoRetireNetVst_c::Method)&stepDemoRetireNetVst_c::initSalute},
        {"もんばん敬礼",
         (stepDemoRetireNetVst_c::Method)&stepDemoRetireNetVst_c::salute},
        {"セーブシーンへ",
         (stepDemoRetireNetVst_c::Method)&stepDemoRetireNetVst_c::toSaveScene},
        {"終了",
         (stepDemoRetireNetVst_c::Method)&stepDemoRetireNetVst_c::finish},
    };
    mStepDemoRetireNetVst.initialize(this,
                                     "dAcNpcSpGkA_c::stepDemoRetireNetVst_c",
                                     steps, ARRAY_SIZE(steps));
}

// .text:0x3A08 fn_66_3A08 (0x48): dAcNpcSpGkA_c::executeRetireNetVst()
void dAcNpcSpGkA_c::executeRetireNetVst() {
    mStepDemoRetireNetVst.execute();
}

// .text:0x3A50 fn_66_3A50 (0xA0):
// dAcNpcSpGkA_c::stepDemoRetireNetVst_c::startPlayerMoveToGate() -
// "門の手前までプレイヤ移動開始"
void dAcNpcSpGkA_c::stepDemoRetireNetVst_c::startPlayerMoveToGate() {
    startGateMove(dPlayerMgr::PLAYER_INDEX_CURRENT);
    nextStep();
}

// .text:0x3AF0 fn_66_3AF0 (0xA8):
// dAcNpcSpGkA_c::stepDemoRetireNetVst_c::waitPlayerMoveToGate() -
// "門の手前までプレイヤ移動完了"
void dAcNpcSpGkA_c::stepDemoRetireNetVst_c::waitPlayerMoveToGate() {
    if (isPlayerMoveDone(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        nextStep();
    }
}

// .text:0x3B98 fn_66_3B98 (0xA4):
// dAcNpcSpGkA_c::stepDemoRetireNetVst_c::turnPlayerToGate() -
// "門の方向へプレイヤ回転"
void dAcNpcSpGkA_c::stepDemoRetireNetVst_c::turnPlayerToGate() {
    dPlayerMgr::turnTo(-DEG_TO_ANGLE(180), dPlayerMgr::PLAYER_INDEX_CURRENT);
    nextStep();
}

// .text:0x3C3C fn_66_3C3C (0xA8):
// dAcNpcSpGkA_c::stepDemoRetireNetVst_c::waitPlayerTurnToGate() -
// "門の方向へプレイヤ回転完了"
void dAcNpcSpGkA_c::stepDemoRetireNetVst_c::waitPlayerTurnToGate() {
    if (!dPlayerMgr::isTurningTo(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        nextStep();
    }
}

// .text:0x3CE4 fn_66_3CE4 (0xA0):
// dAcNpcSpGkA_c::stepDemoRetireNetVst_c::startPlayerExit() -
// "プレイヤが門の外へ出る開始"
void dAcNpcSpGkA_c::stepDemoRetireNetVst_c::startPlayerExit() {
    startExitMove(dPlayerMgr::PLAYER_INDEX_CURRENT);
    nextStep();
}

// .text:0x3D84 fn_66_3D84 (0xB0):
// dAcNpcSpGkA_c::stepDemoRetireNetVst_c::toSaveScene() - "セーブシーンへ"
void dAcNpcSpGkA_c::stepDemoRetireNetVst_c::toSaveScene() {
    dSvMgr_c::requestSaveRetireNetVst();
    dScene::requestSceneChange(dScene::SCENE_DM_SAVE, 0, 5);
    dNet::fn_800DDE7C();
    nextStep();
}

// .text:0x3E34 fn_66_3E34 (0x3C0): dAcNpcSpGkA_c::initSeeOffNetHst()
void dAcNpcSpGkA_c::initSeeOffNetHst() {
    static const stepDemoSeeOffNetHst_c::Entry steps[] = {
        {"フェードイン終了待ち",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::waitFadeIn},
        {"見送りメッセージ開始",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::
             startSeeOffMsg},
        {"BGM音量維持", (stepDemoSeeOffNetHst_c::
                             Method)&stepDemoSeeOffNetHst_c::keepBgmVolume},
        {"ロック開始まで待つ",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::
             waitLockStart},
        {"セーブ開始まで待つ",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::
             waitSaveStart},
        {"DL BOX生成初期化",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::
             initializeDownloadBox},
        {"DL BOX生成", (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::
                           createDownloadBox},
        {"全体セーブ初期化",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::
             initializeFullSave},
        {"全体セーブ開始", (stepDemoSeeOffNetHst_c::
                                Method)&stepDemoSeeOffNetHst_c::startFullSave},
        {"全体セーブ終了",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::waitFullSave},
        {"セーブ終了を連絡",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::
             notifySaveDone},
        {"デモの終了条件待ち",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::
             waitDisconnectReady},
        {"アイコンとロック解除",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::
             releaseIconAndLock},
        {"会話終了待ち",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::waitTalkEnd},
        {"もんばん横を向く初期化",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::initFaceSide},
        {"もんばん横を向く",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::faceSide},
        {"プレイヤが門の外へ出る開始",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::
             startPlayerExit},
        {"もんばん敬礼初期化",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::initSalute},
        {"もんばん敬礼",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::salute},
        {"デモのロック受付キャンセルクリアを送信",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::
             sendReleaseRequest},
        {"元のシーンへ",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::toPrevScene},
        {"終了",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::finish},
        {"セーブ失敗",
         (stepDemoSeeOffNetHst_c::Method)&stepDemoSeeOffNetHst_c::saveFailed},
    };
    mStepDemoSeeOffNetHst.initialize(this,
                                     "dAcNpcSpGkA_c::stepDemoSeeOffNetHst_c",
                                     steps, ARRAY_SIZE(steps));
}

// .text:0x41F4 fn_66_41F4 (0x48): dAcNpcSpGkA_c::executeSeeOffNetHst()
void dAcNpcSpGkA_c::executeSeeOffNetHst() {
    mStepDemoSeeOffNetHst.execute();
}

// .text:0x423C fn_66_423C (0xB0):
// dAcNpcSpGkA_c::stepDemoSeeOffNetHst_c::waitSaveStart() - "セーブ開始まで待つ"
void dAcNpcSpGkA_c::stepDemoSeeOffNetHst_c::waitSaveStart() {
    if (dNet::isDisconnectStarted()) {
        dSvMgr_c::removeVisitor(dNet::getLeaveMember());
        dNpc::exchangeBufferedMovedAnimal();
        nextStep();
    }
}

// .text:0x42EC fn_66_42EC (0xA8):
// dAcNpcSpGkA_c::stepDemoSeeOffNetHst_c::startPlayerExit() -
// "プレイヤが門の外へ出る開始"
void dAcNpcSpGkA_c::stepDemoSeeOffNetHst_c::startPlayerExit() {
    startExitMove(dNet::getLeaveMember());
    nextStep();
}

// .text:0x4394 fn_66_4394 (0x350): dAcNpcSpGkA_c::initSeeOffNetVst()
void dAcNpcSpGkA_c::initSeeOffNetVst() {
    static const stepDemoSeeOffNetVst_c::Entry steps[] = {
        {"フェードイン終了待ち",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::waitFadeIn},
        {"見送りメッセージ開始",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::
             startSeeOffMsg},
        {"BGM音量維持", (stepDemoSeeOffNetVst_c::
                             Method)&stepDemoSeeOffNetVst_c::keepBgmVolume},
        {"ロック開始まで待つ",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::
             waitLockStart},
        {"セーブ開始まで待つ",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::
             waitSaveStart},
        {"ネットビジターセーブ初期化",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::
             initializeVisitorSave},
        {"ネットビジターセーブ開始",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::
             startVisitorSave},
        {"ネットビジターセーブ終了",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::
             waitVisitorSave},
        {"セーブとデモが終了なら",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::
             notifySaveDone},
        {"デモの終了条件待ち",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::
             waitDisconnectReady},
        {"アイコンとロック解除",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::
             releaseIconAndLock},
        {"会話終了待ち",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::waitTalkEnd},
        {"もんばん横を向く初期化",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::initFaceSide},
        {"もんばん横を向く",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::faceSide},
        {"プレイヤが門の外へ出る開始",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::
             startPlayerExit},
        {"もんばん敬礼初期化",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::initSalute},
        {"もんばん敬礼",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::salute},
        {"デモのロック受付キャンセルクリアを送信",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::
             sendReleaseRequest},
        {"元のシーンへ",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::toPrevScene},
        {"終了",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::finish},
        {"セーブ失敗",
         (stepDemoSeeOffNetVst_c::Method)&stepDemoSeeOffNetVst_c::saveFailed},
    };
    mStepDemoSeeOffNetVst.initialize(this,
                                     "dAcNpcSpGkA_c::stepDemoSeeOffNetVst_c",
                                     steps, ARRAY_SIZE(steps));
}

// .text:0x46E4 fn_66_46E4 (0x48): dAcNpcSpGkA_c::executeSeeOffNetVst()
void dAcNpcSpGkA_c::executeSeeOffNetVst() {
    mStepDemoSeeOffNetVst.execute();
}

// .text:0x472C fn_66_472C (0xA4):
// dAcNpcSpGkA_c::stepDemoSeeOffNetVst_c::waitSaveStart() - "セーブ開始まで待つ"
void dAcNpcSpGkA_c::stepDemoSeeOffNetVst_c::waitSaveStart() {
    if (dNet::isDisconnectStarted()) {
        nextStep();
    }
}

// .text:0x47D0 fn_66_47D0 (0xA8):
// dAcNpcSpGkA_c::stepDemoSeeOffNetVst_c::startPlayerExit() -
// "プレイヤが門の外へ出る開始"
void dAcNpcSpGkA_c::stepDemoSeeOffNetVst_c::startPlayerExit() {
    startExitMove(dNet::getLeaveMember());
    nextStep();
}

// .text:0x4878 fn_66_4878 (0xFC): dAcNpcSpGkA_c::initBringBack()
void dAcNpcSpGkA_c::initBringBack() {
    static const stepDemoBringBack_c::Entry steps[] = {
        {"会話終了まで待つ",
         (stepDemoBringBack_c::Method)&stepDemoBringBack_c::waitTalkEnd},
        {"門の手前までプレイヤ移動開始",
         (stepDemoBringBack_c::Method)&stepDemoBringBack_c::
             startPlayerMoveToGate},
        {"門の手前までプレイヤ移動完了",
         (stepDemoBringBack_c::Method)&stepDemoBringBack_c::
             waitPlayerMoveToGate},
        {"会話終了",
         (stepDemoBringBack_c::Method)&stepDemoBringBack_c::finishTalk},
    };
    mStepDemoBringBack.initialize(this, "dAcNpcSpGkA_c::stepDemoBringBack_c",
                                  steps, ARRAY_SIZE(steps));
}

// .text:0x4974 fn_66_4974 (0x48): dAcNpcSpGkA_c::executeBringBack()
void dAcNpcSpGkA_c::executeBringBack() {
    mStepDemoBringBack.execute();
}

// .text:0x49BC fn_66_49BC (0xA0):
// dAcNpcSpGkA_c::stepDemoBringBack_c::startPlayerMoveToGate() -
// "門の手前までプレイヤ移動開始"
void dAcNpcSpGkA_c::stepDemoBringBack_c::startPlayerMoveToGate() {
    startEnterMove(0);
    nextStep();
}

// .text:0x4A5C fn_66_4A5C (0xA8):
// dAcNpcSpGkA_c::stepDemoBringBack_c::waitPlayerMoveToGate() -
// "門の手前までプレイヤ移動完了"
void dAcNpcSpGkA_c::stepDemoBringBack_c::waitPlayerMoveToGate() {
    if (isPlayerMoveDone(0)) {
        nextStep();
    }
}

// .text:0x4B04 fn_66_4B04 (0x44):
// dAcNpcSpGkA_c::stepDemoBringBack_c::finishTalk() - "会話終了"
void dAcNpcSpGkA_c::stepDemoBringBack_c::finishTalk() {
    if (!mpOwner->mTalk.isBusy()) {
        dDemo::finishTalk(mpOwner);
    }
}

// .text:0x4B48 fn_66_4B48 (0x98): dAcNpcSpGkA_c::isPlayerInGate() - net member
// 4 at z < 256 (provisional)
BOOL dAcNpcSpGkA_c::isPlayerInGate() {
    if (isMultiPlay()) {
        return FALSE;
    }
    if (dDemo::isType(dDemo::TYPE_ANY)) {
        return FALSE;
    }
    if (!isNetActive()) {
        return FALSE;
    }
    mVec3_c *pos = dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (pos == NULL) {
        return FALSE;
    }
    f32 gateZ = 256.0f;
    return !(pos->z >= gateZ);
}

// .text:0x4BE0 fn_66_4BE0 (0x2D4): dAcNpcSpGkA_c::initGoOutDsAppli()
void dAcNpcSpGkA_c::initGoOutDsAppli() {
    static const stepDemoGoOutDsAppli_c::Entry steps[] = {
        {"会話終了待ち",
         (stepDemoGoOutDsAppli_c::Method)&stepDemoGoOutDsAppli_c::waitTalkEnd},
        {"チャット閉じる",
         (stepDemoGoOutDsAppli_c::Method)&stepDemoGoOutDsAppli_c::closeChat},
        {"もんばん前を向く",
         (stepDemoGoOutDsAppli_c::Method)&stepDemoGoOutDsAppli_c::faceFront},
        {"門を開く",
         (stepDemoGoOutDsAppli_c::Method)&stepDemoGoOutDsAppli_c::openGate},
        {"門の手前までプレイヤ移動開始",
         (stepDemoGoOutDsAppli_c::Method)&stepDemoGoOutDsAppli_c::
             startPlayerMoveToGate},
        {"門の手前までプレイヤ移動完了",
         (stepDemoGoOutDsAppli_c::Method)&stepDemoGoOutDsAppli_c::
             waitPlayerMoveToGate},
        {"門の方向へプレイヤ回転",
         (stepDemoGoOutDsAppli_c::Method)&stepDemoGoOutDsAppli_c::
             turnPlayerToGate},
        {"もんばん横を向く初期化",
         (stepDemoGoOutDsAppli_c::Method)&stepDemoGoOutDsAppli_c::initFaceSide},
        {"もんばん横を向く",
         (stepDemoGoOutDsAppli_c::Method)&stepDemoGoOutDsAppli_c::faceSide},
        {"もんばん横を向く完了",
         (stepDemoGoOutDsAppli_c::Method)&stepDemoGoOutDsAppli_c::waitFaceSide},
        {"門の方向へプレイヤ回転完了",
         (stepDemoGoOutDsAppli_c::Method)&stepDemoGoOutDsAppli_c::
             waitPlayerTurnToGate},
        {"門が開くまで待つ",
         (stepDemoGoOutDsAppli_c::Method)&stepDemoGoOutDsAppli_c::waitGateOpen},
        {"プレイヤが門の外へ出る開始",
         (stepDemoGoOutDsAppli_c::Method)&stepDemoGoOutDsAppli_c::
             startPlayerExit},
        {"もんばん敬礼初期化",
         (stepDemoGoOutDsAppli_c::Method)&stepDemoGoOutDsAppli_c::initSalute},
        {"もんばん敬礼",
         (stepDemoGoOutDsAppli_c::Method)&stepDemoGoOutDsAppli_c::salute},
        {"タイトルシーンへ",
         (stepDemoGoOutDsAppli_c::Method)&stepDemoGoOutDsAppli_c::toTitleScene},
        {"終了",
         (stepDemoGoOutDsAppli_c::Method)&stepDemoGoOutDsAppli_c::finish},
    };
    mStepDemoGoOutDsAppli.initialize(this,
                                     "dAcNpcSpGkA_c::stepDemoGoOutDsAppli_c",
                                     steps, ARRAY_SIZE(steps));
}

// .text:0x4EB4 fn_66_4EB4 (0x48): dAcNpcSpGkA_c::executeGoOutDsAppli()
void dAcNpcSpGkA_c::executeGoOutDsAppli() {
    mStepDemoGoOutDsAppli.execute();
}

// .text:0x4EFC fn_66_4EFC (0xA0):
// dAcNpcSpGkA_c::stepDemoGoOutDsAppli_c::startPlayerExit() -
// "プレイヤが門の外へ出る開始"
void dAcNpcSpGkA_c::stepDemoGoOutDsAppli_c::startPlayerExit() {
    startExitMove(0);
    nextStep();
}

// .text:0x4F9C fn_66_4F9C (0x280): dAcNpcSpGkA_c::initComeInDsAppli()
void dAcNpcSpGkA_c::initComeInDsAppli() {
    static const stepDemoComeInDsAppli_c::Entry steps[] = {
        {"フェードイン終了待ち",
         (stepDemoComeInDsAppli_c::Method)&stepDemoComeInDsAppli_c::waitFadeIn},
        {"プレイヤが門の中に入る開始",
         (stepDemoComeInDsAppli_c::Method)&stepDemoComeInDsAppli_c::
             startPlayerEnter},
        {"プレイヤが指定時間歩いたか？",
         (stepDemoComeInDsAppli_c::Method)&stepDemoComeInDsAppli_c::
             waitPlayerWalkTime},
        {"もんばん横を向く初期化",
         (stepDemoComeInDsAppli_c::Method)&stepDemoComeInDsAppli_c::
             initFaceSide},
        {"もんばん横を向く",
         (stepDemoComeInDsAppli_c::Method)&stepDemoComeInDsAppli_c::faceSide},
        {"もんばん横を向く完了",
         (stepDemoComeInDsAppli_c::Method)&stepDemoComeInDsAppli_c::
             waitFaceSide},
        {"プレイヤ移動完了",
         (stepDemoComeInDsAppli_c::Method)&stepDemoComeInDsAppli_c::
             playerMoveDone},
        {"受け入れメッセージ開始",
         (stepDemoComeInDsAppli_c::Method)&stepDemoComeInDsAppli_c::
             startWelcomeMsg},
        {"BGM音量維持", (stepDemoComeInDsAppli_c::
                             Method)&stepDemoComeInDsAppli_c::keepBgmVolume},
        {"もんばん敬礼初期化",
         (stepDemoComeInDsAppli_c::Method)&stepDemoComeInDsAppli_c::initSalute},
        {"もんばん敬礼",
         (stepDemoComeInDsAppli_c::Method)&stepDemoComeInDsAppli_c::salute},
        {"会話終了待ち", (stepDemoComeInDsAppli_c::
                              Method)&stepDemoComeInDsAppli_c::waitTalkEnd},
        {"門の外のシーンへ",
         (stepDemoComeInDsAppli_c::Method)&stepDemoComeInDsAppli_c::
             toOutsideScene},
        {"終了",
         (stepDemoComeInDsAppli_c::Method)&stepDemoComeInDsAppli_c::finish},
    };
    mStepDemoComeInDsAppli.initialize(this,
                                      "dAcNpcSpGkA_c::stepDemoComeInDsAppli_c",
                                      steps, ARRAY_SIZE(steps));
}

// .text:0x521C fn_66_521C (0x48): dAcNpcSpGkA_c::executeComeInDsAppli()
void dAcNpcSpGkA_c::executeComeInDsAppli() {
    mStepDemoComeInDsAppli.execute();
}

// .text:0x5264 fn_66_5264 (0xA0):
// dAcNpcSpGkA_c::stepDemoComeInDsAppli_c::startPlayerEnter() -
// "プレイヤが門の中に入る開始"
void dAcNpcSpGkA_c::stepDemoComeInDsAppli_c::startPlayerEnter() {
    startEnterMove(dPlayerMgr::PLAYER_INDEX_CURRENT);
    nextStep();
}

// .text:0x5304 fn_66_5304 (0xAC):
// dAcNpcSpGkA_c::stepDemoComeInDsAppli_c::toOutsideScene() - "門の外のシーンへ"
void dAcNpcSpGkA_c::stepDemoComeInDsAppli_c::toOutsideScene() {
    dANpcSpGatekeeperA::getSceneChange()->requestExit(0, 0, 0);
    nextStep();
}

// .text:0x53B0 fn_66_53B0 (0xA8):
// dAcNpcSpGkA_c::stepDemoComeInDsAppli_c::playerMoveDone() - "プレイヤ移動完了"
void dAcNpcSpGkA_c::stepDemoComeInDsAppli_c::playerMoveDone() {
    if (isPlayerMoveDone(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        nextStep();
    }
}

// .text:0x5458 fn_66_5458 (0x144): dAcNpcSpGkA_c::talk_c::talk_c(dAcNpcSpGkA_c
// *npc)
dAcNpcSpGkA_c::talk_c::talk_c(dAcNpcSpGkA_c *npc) : mpOwner(npc) {
    static const dState::mode_c<talk_c>::Entry modes[] = {
        {"待機", &talk_c::initWait, &talk_c::executeWait},
        {"WiFiコネクション接続前・セーブ処理",
         &talk_c::initSaveBeforeWiFiConnect,
         &talk_c::executeSaveBeforeWiFiConnect},
        {"WiFiコネクション接続", &talk_c::initWiFiConnect,
         &talk_c::executeWiFiConnect},
        {"WiFiコネクション切断", &talk_c::initWiFiDisconnect,
         &talk_c::executeWiFiDisconnect},
        {"おでかけ開始シーケンス", &talk_c::initGoOutStart,
         &talk_c::executeGoOutStart},
        {"おでかけ終了シーケンス", &talk_c::initGoOutEnd,
         &talk_c::executeGoOutEnd},
        {"DSアプリ開始", &talk_c::initDsAppli, &talk_c::executeDsAppli},
        {"ネットホストの新規参入受け入れシーケンス", &talk_c::initComeInNetHst,
         &talk_c::executeComeInNetHst},
        {"ネットビジターの新規参入受け入れシーケンス",
         &talk_c::initComeInNetVst, &talk_c::executeComeInNetVst},
        {"ネットビジターとして通信開始シーケンス", &talk_c::initGoOutNetVst,
         &talk_c::executeGoOutNetVst},
    };
    init();
    dState::mode_c<talk_c>::initialize("dAcNpcSpGkA_c::talk_c", modes,
                                       ARRAY_SIZE(modes));
}

// .text:0x559C fn_66_559C (0x48): dAcNpcSpGkA_c::talk_c::init()
void dAcNpcSpGkA_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mRequestEndMethod = NULL;
}

// .text:0x55E4 fn_66_55E4 (0x48): dAcNpcSpGkA_c::talk_c::start(dActor_c
// *target)
void dAcNpcSpGkA_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
}

// .text:0x562C fn_66_562C (0x324): dAcNpcSpGkA_c::talk_c::getMsgInfo(msgInfo_s
// *info)
void dAcNpcSpGkA_c::talk_c::getMsgInfo(msgInfo_s *info) {
    mSavedBeforeConnect = 0;
    mLockRefused = 0;
    if (mpOwner->mAutoTalk) {
        mpOwner->mAutoTalk = 0;
        info->mCode = MSG_FILL_OUT_FORMS;
        info->mLabel = dANpcSpGatekeeperA::l_msgLabel;
        return;
    }

    bool comeIn = true;
    bool reunion = true;
    if (!mpOwner->isMode(&dAcNpcSpGkA_c::executeReunionNetVst)) {
        if (!mpOwner->isMode(&dAcNpcSpGkA_c::executeReunionNetHst)) {
            reunion = false;
        }
    }
    if (!reunion) {
        if (!mpOwner->isMode(&dAcNpcSpGkA_c::executeComeInNet)) {
            comeIn = false;
        }
    }

    if (comeIn) {
        clearSpeakerName();
        info->mCode = SV_MSG_VISITOR_CAME;
        info->mLabel = dSvMgr_c::mMessageLabel;
        return;
    }

    bool seeOff = true;
    if (!mpOwner->isMode(&dAcNpcSpGkA_c::executeSeeOffNetHst)) {
        if (!mpOwner->isMode(&dAcNpcSpGkA_c::executeSeeOffNetVst)) {
            seeOff = false;
        }
    }

    if (seeOff) {
        info->mCode = SV_MSG_VISITOR_LEFT;
        info->mLabel = dSvMgr_c::mMessageLabel;
        return;
    }

    if (mpOwner->isMode(&dAcNpcSpGkA_c::executeComeInDsAppli)) {
        info->mCode = MSG_DS_RETURN_HINT;
        info->mLabel = dANpcSpGatekeeperA::l_msgLabel;
        return;
    }

    if (isPlayerFromThisTown()) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        if (player->isFlag0(PRIVATE_FLAG0_GATEKEEPER_MET)) {
            if (player->isFlag0(PRIVATE_FLAG0_PART_TIME_JOB)) {
                info->mCode = MSG_HELLO_PART_TIME_JOB;
            } else {
                dTime_c now = *dTime_c::getCurrent();
                info->mCode = now.getTimeOfDay() + MSG_HELLO_MORNING;
            }
        } else {
            player->setFlag0(PRIVATE_FLAG0_GATEKEEPER_MET);
            info->mCode = MSG_FIRST_MEETING;
        }
    } else if (isNetActive()) {
        info->mCode = MSG_ASK_GO_HOME;
    } else {
        info->mCode = MSG_READY_TO_GO_HOME;
    }
    info->mLabel = dANpcSpGatekeeperA::l_msgLabel;
}

// .text:0x5950 fn_66_5950 (0x50): dAcNpcSpGkA_c::talk_c::preExecute()
void dAcNpcSpGkA_c::talk_c::preExecute() {
    dState::base_c<talk_c>::execute();
}

// .text:0x59A0 fn_66_59A0 (0x4): dAcNpcSpGkA_c::talk_c::onTalkEnd()
void dAcNpcSpGkA_c::talk_c::onTalkEnd() {
}

// .text:0x59A4 fn_66_59A4 (0x4C): dAcNpcSpGkA_c::talk_c::setNextMessage(u16
// code, const char *label)
void dAcNpcSpGkA_c::talk_c::setNextMessage(u16 code, const char *label) {
    mpController->setNextMsgLabel(label);
    mpController->setNextMsgCode(code);
}

// .text:0x59F0 fn_66_59F0 (0x30): dAcNpcSpGkA_c::talk_c::selectNetMsg() - 8
// while the net is active, else 9 (provisional)
int dAcNpcSpGkA_c::talk_c::selectNetMsg() {
    return isNetActive() ? (int)MSG_MENU_GATE_OPEN : (int)MSG_MENU;
}

// .text:0x5A20 fn_66_5A20 (0x148): dAcNpcSpGkA_c::talk_c::getNetErrorMsg() -
// the NetworkError message of the error code
int dAcNpcSpGkA_c::talk_c::getNetErrorMsg() {
    int kind = dANpcSpGatekeeperA::getNetErrorKind();
    int code = dANpcSpGatekeeperA::getNetErrorCode();
    dMsg::Comp_c::mInstance->formatNumber(0, code, 5,
                                          dScript::NUM_FORMAT_ZERO_PAD);
    int msg = kind == dNetDwc::ERROR_PERMANENT ? NET_ERROR_MSG_WFC_RESET
                                               : NET_ERROR_MSG_DISCONNECTED;
    if (sLib::isInRange(code, NET_ERROR_CODE_LOGIN_FIRST,
                        NET_ERROR_CODE_SERVICE_ENDED - 1) ||
        sLib::isInRange(code, NET_ERROR_CODE_SERVICE_ENDED + 1,
                        NET_ERROR_CODE_LOGIN_LAST)) {
        msg = NET_ERROR_MSG_WFC_ERROR;
    } else if (code == NET_ERROR_CODE_SERVER_BUSY ||
               sLib::isInRange(code, NET_ERROR_CODE_BUSY_FIRST,
                               NET_ERROR_CODE_BUSY_LAST)) {
        msg = NET_ERROR_MSG_WFC_BUSY;
    } else if (code == NET_ERROR_CODE_SERVICE_ENDED) {
        msg = NET_ERROR_MSG_WFC_DISCONTINUED;
    } else if (code == NET_ERROR_CODE_NAND_FULL) {
        msg = NET_ERROR_MSG_NAND_FULL;
    } else if (code == NET_ERROR_CODE_NAND_CORRUPTED) {
        msg = NET_ERROR_MSG_NAND_CORRUPTED;
    } else if (code == NET_ERROR_CODE_NO_INTERNET ||
               sLib::isInRange(code, NET_ERROR_CODE_INTERNET_FIRST,
                               NET_ERROR_CODE_INTERNET_LAST)) {
        msg = NET_ERROR_MSG_NO_INTERNET;
    } else if (code == NET_ERROR_CODE_NO_RESPONSE) {
        msg = NET_ERROR_MSG_NO_RESPONSE;
    } else if (code == 0) {
        msg = NET_ERROR_MSG_DISCONNECTED2;
    }
    return msg;
}

// .text:0x5B68 fn_66_5B68 (0x30): dAcNpcSpGkA_c::talk_c::selectNetMsg2() - 0x3D
// while the net is active, else 0x3E (provisional)
int dAcNpcSpGkA_c::talk_c::selectNetMsg2() {
    return isNetActive() ? (int)MSG_MENU_GATE_OPEN2 : (int)MSG_MENU2;
}

// .text:0x5B98 fn_66_5B98 (0xE8): dAcNpcSpGkA_c::talk_c::onMessageStart(int
// arg)
void dAcNpcSpGkA_c::talk_c::onMessageStart(int arg) {
    struct Entry {
        const char *label;
        int code;
        Method method;
    };
    static Entry messages[] = {
        {dANpcSpGatekeeperA::l_msgLabel, MSG_DS_DOWNLOAD,
         &talk_c::startDsAppli},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_FRIEND_CODE,
         &talk_c::setFriendCodeWord},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_FRIEND_CODE_SAVED2,
         &talk_c::setFriendCodeWord},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_FRIEND_CODE_SAVED,
         &talk_c::setFriendCodeWord},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_CLOSE_GATE,
         &talk_c::startWiFiDisconnect},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_GO_OUT_CANCELLED,
         &talk_c::startWiFiDisconnect},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_TOWN_GATE_CLOSED,
         &talk_c::startWiFiDisconnect},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_TOWN_FULL,
         &talk_c::startWiFiDisconnect},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_MENU_END,
         &talk_c::startWiFiDisconnect},
        {dSvMgr_c::mMessageLabel, SV_MSG_VISITOR_CAME,
         &talk_c::setNetPlayerName},
        {dSvMgr_c::mMessageLabel, SV_MSG_VISITOR_LEFT,
         &talk_c::setLeaveMemberName},
#if VERSION == VERSION_RUUK01_01
        {dANpcSpGatekeeperA::l_msgLabel, MSG_DS_RETURN_HINT,
         &talk_c::setActing},
#endif
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

// .text:0x5C80 fn_66_5C80 (0x15C): dAcNpcSpGkA_c::talk_c::startDsAppli() -
// changeMode(MODE_DS_APPLI)
void dAcNpcSpGkA_c::talk_c::startDsAppli() {
    changeMode(&talk_c::executeDsAppli);
}

// .text:0x5DDC fn_66_5DDC (0x15C): dAcNpcSpGkA_c::talk_c::startWiFiDisconnect()
// - changeMode(MODE_WIFI_DISCONNECT)
void dAcNpcSpGkA_c::talk_c::startWiFiDisconnect() {
    changeMode(&talk_c::executeWiFiDisconnect);
}

// .text:0x5F38 fn_66_5F38 (0x3C): dAcNpcSpGkA_c::talk_c::setFriendCodeWord()
void dAcNpcSpGkA_c::talk_c::setFriendCodeWord() {
    formatFriendCode(dWifi::getMyFriendKey());
}

// .text:0x5F74 fn_66_5F74 (0x80): dAcNpcSpGkA_c::talk_c::setNetPlayerName()
void dAcNpcSpGkA_c::talk_c::setNetPlayerName() {
    mActing = 1;
    mpOwner->mTalk.mNoMouth = 1;
    int no = dSvMgr_c::mCurrentVisitor;
    dPrivateData_c *player;
    if (no < 4) {
        player = dPlayerMgr::Mgr_c::getNetPlayer(no);
    } else {
        player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    }
    setPersonalName(&player->mPID, 0);
    setNextMessage(SV_MSG_SAVING_VISIT, dSvMgr_c::mMessageLabel);
}

// .text:0x5FF4 fn_66_5FF4 (0x7C): dAcNpcSpGkA_c::talk_c::setLeaveMemberName()
void dAcNpcSpGkA_c::talk_c::setLeaveMemberName() {
    mActing = 1;
    mpOwner->mTalk.mNoMouth = 1;
    int no = dNet::getLeaveMember();
    dPrivateData_c *player;
    if (no < 4) {
        player = dPlayerMgr::Mgr_c::getNetPlayer(no);
    } else {
        player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    }
    setPersonalName(&player->mPID, 0);
    setNextMessage(SV_MSG_SAVING_VISIT, dSvMgr_c::mMessageLabel);
}

#if VERSION == VERSION_RUUK01_01
void dAcNpcSpGkA_c::talk_c::setActing() {
    mActing = 1;
}
#endif

// .text:0x6070 fn_66_6070 (0x160): dAcNpcSpGkA_c::talk_c::onMessageEnd(int
// kind)
void dAcNpcSpGkA_c::talk_c::onMessageEnd(int kind) {
    struct Entry {
        const char *label;
        int code;
        Method method;
    };
    static Entry messages[] = {
        {dANpcSpGatekeeperA::l_msgLabel, MSG_FIRST_MEETING,
         &talk_c::endFirstMeeting},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_HELLO_MORNING, &talk_c::showMenu},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_HELLO_MORNING + 1,
         &talk_c::showMenu},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_HELLO_MORNING + 2,
         &talk_c::showMenu},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_HELLO_MORNING + 3,
         &talk_c::showMenu},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_ISSUE_FRIEND_CODE,
         &talk_c::startFriendCodeConnect},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_GO_HOME_START,
         &talk_c::startGoOutEnd},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_GO_HOME_BYE, &talk_c::startRetire},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_CONNECTING,
         &talk_c::startWiFiConnect},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_GO_OUT_NO_FRIENDS,
         &talk_c::endGoOutNoFriends},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_ISSUE_FRIEND_CODE2,
         &talk_c::startFriendCodeConnect},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_SEARCH_GATE,
         &talk_c::startGoOutConnect},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_SEARCHING, &talk_c::showTownList},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_TOWN_LIST,
         &talk_c::openFriendMenu},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_SEARCH_GATE_START,
         &talk_c::openFriendMenu},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_GO_OUT_CANCELLED,
         &talk_c::cancelGoOut},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_INQUIRING,
         &talk_c::startGoOutStart},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_SAFE_JOURNEY,
         &talk_c::startGoOutNetVstDemo},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_OPEN_GATE_START,
         &talk_c::startOpenGateConnect},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_OPEN_GATE, &talk_c::startOpenGate},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_OPEN_GATE_HOORAH,
         &talk_c::startOpenGate},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_CLOSE_GATE,
         &talk_c::startCloseGate},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_DS_CANCEL_HINT,
         &talk_c::checkDsConnected},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_DS_JOURNEY,
         &talk_c::startGoOutDsAppliDemo},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_DS_JOURNEY2,
         &talk_c::startGoOutDsAppliDemo},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_TOWN_BUSY, &talk_c::endTownBusy},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_TOWN_NO_RESPONSE,
         &talk_c::endTownBusy},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_TOWN_IN_CONVERSATION,
         &talk_c::endTownBusy},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_TOWN_UNAVAILABLE,
         &talk_c::endTownBusy},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_TOWN_GATE_CLOSED,
         &talk_c::endTownRefused},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_TOWN_FULL,
         &talk_c::endTownRefused},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_MENU_END,
         &talk_c::setDisconnectWaitIcon},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_DS_COMMUNICATING,
         &talk_c::checkDsAppliEnd},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_FILL_OUT_FORMS,
         &talk_c::startBringBack},
        {sNetErrorMessageLabel, NET_ERROR_MSG_RESTRICTED,
         &talk_c::endNetRestricted},
        {dSvMgr_c::mMessageLabel, SV_MSG_SAVING_NET,
         &talk_c::startSaveBeforeWiFiConnect},
        {dSvMgr_c::mMessageLabel, SV_MSG_SAVING_VISIT,
         &talk_c::startGoOutNetVst},
        {dSvMgr_c::mMessageLabel, SV_MSG_VISITOR_CAME, &talk_c::startComeInNet},
        {dSvMgr_c::mMessageLabel, SV_MSG_VISITOR_LEFT,
         &talk_c::endVisitorLeftMsg},
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

// .text:0x61D0 fn_66_61D0 (0xAC):
// dAcNpcSpGkA_c::talk_c::switchToSystemSpeaker() - (request-end hideSpeaker)
void dAcNpcSpGkA_c::talk_c::switchToSystemSpeaker() {
    dString::Word_c speaker((const wchar_t *)getSpeakerName());
    dHmnName::Word_c name;
    dItem::Item item((u16)SP_NPC_GATEKEEPER_A);
    dNpc::getNpcName(&name, &item, LANGUAGE_NUM);
    if (name.isSame(&speaker)) {
        setRequest1(0);
        mRequestEndMethod = &talk_c::hideSpeaker;
    }
}

// .text:0x627C fn_66_627C (0xAC):
// dAcNpcSpGkA_c::talk_c::switchToGatekeeperSpeaker() - (request-end
// showSpeaker)
void dAcNpcSpGkA_c::talk_c::switchToGatekeeperSpeaker() {
    dString::Word_c speaker((const wchar_t *)getSpeakerName());
    dHmnName::Word_c name;
    dItem::Item item((u16)SP_NPC_GATEKEEPER_A);
    dNpc::getNpcName(&name, &item, LANGUAGE_NUM);
    if (!name.isSame(&speaker)) {
        setRequest1(0);
        mRequestEndMethod = &talk_c::showSpeaker;
    }
}

// .text:0x6328 fn_66_6328 (0x2D0): dAcNpcSpGkA_c::talk_c::endMsg6328() -
// provisional
void dAcNpcSpGkA_c::talk_c::startComeInNet() {
    if (mpOwner->isMode(&dAcNpcSpGkA_c::executeComeInNet)) {
        if (dNet::isHost()) {
            changeMode(&talk_c::executeComeInNetHst);
        } else {
            changeMode(&talk_c::executeComeInNetVst);
        }
    }
}

// .text:0x65F8 fn_66_65F8 (0x4): dAcNpcSpGkA_c::talk_c::endMsg65F8() -
// provisional (empty)
void dAcNpcSpGkA_c::talk_c::endVisitorLeftMsg() {
}

// .text:0x65FC fn_66_65FC (0x164): dAcNpcSpGkA_c::talk_c::startBringBack() -
// owner changeMode(MODE_BRING_BACK)
void dAcNpcSpGkA_c::talk_c::startBringBack() {
    mpOwner->changeMode(&dAcNpcSpGkA_c::executeBringBack);
}

// .text:0x6760 fn_66_6760 (0x74): dAcNpcSpGkA_c::talk_c::endMsg6760() -
// provisional
void dAcNpcSpGkA_c::talk_c::endFirstMeeting() {
    if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
            PRIVATE_FLAG0_PART_TIME_JOB)) {
        setNextMessage(MSG_RESIDENT_PROCESSING, dANpcSpGatekeeperA::l_msgLabel);
    } else {
        int msg = selectNetMsg();
        setNextMessage(msg, dANpcSpGatekeeperA::l_msgLabel);
    }
}

// .text:0x67D4 fn_66_67D4 (0x44): dAcNpcSpGkA_c::talk_c::requestNetMsg() -
// provisional
void dAcNpcSpGkA_c::talk_c::showMenu() {
    int msg = selectNetMsg();
    setNextMessage(msg, dANpcSpGatekeeperA::l_msgLabel);
}

// .text:0x6818 fn_66_6818 (0xBC): dAcNpcSpGkA_c::talk_c::checkDsConnected() -
// provisional
void dAcNpcSpGkA_c::talk_c::checkDsConnected() {
    if (isCurrentMethod(&talk_c::executeDsAppli)) {
        if (dDsDl::g_manager.isConnected()) {
            setNextMessage(MSG_DS_COMMUNICATING,
                           dANpcSpGatekeeperA::l_msgLabel);
            switchToSystemSpeaker();
        } else {
            dMsg::Comp_c *controller = mpController;
            controller->mLock = 1;
            controller->mWaitIconSe = 1;
            controller->mWaitIcon = 1;
        }
    } else {
        setNextMessage(MSG_DS_ERROR, dANpcSpGatekeeperA::l_msgLabel);
    }
}

// .text:0x68D4 fn_66_68D4 (0xA4): dAcNpcSpGkA_c::talk_c::endMsg68D4() -
// provisional
void dAcNpcSpGkA_c::talk_c::checkDsAppliEnd() {
    if (isCurrentMethod(&talk_c::executeDsAppli)) {
        dMsg::Comp_c *controller = mpController;
        controller->mLock = 1;
        controller->mWaitIconSe = 1;
        controller->mWaitIcon = 1;
    } else if (mpController->mNextMsgCode == 0) {
        setNextMessage(MSG_DS_ERROR, dANpcSpGatekeeperA::l_msgLabel);
    }
    mpController->mForceNext = 0;
}

// .text:0x6978 fn_66_6978 (0xC): dAcNpcSpGkA_c::talk_c::endMsg6978() -
// provisional
void dAcNpcSpGkA_c::talk_c::startFriendCodeConnect() {
    mConnectPurpose = CONNECT_FRIEND_CODE;
    startConnect();
}

// .text:0x6984 fn_66_6984 (0x64): dAcNpcSpGkA_c::talk_c::endMsg6984() -
// provisional
void dAcNpcSpGkA_c::talk_c::startConnect() {
    switchToSystemSpeaker();
    if (mSavedBeforeConnect) {
        setNextMessage(MSG_CONNECTING, dANpcSpGatekeeperA::l_msgLabel);
    } else {
        setNextMessage(SV_MSG_SAVING_NET, dSvMgr_c::mMessageLabel);
    }
}

// .text:0x69E8 fn_66_69E8 (0x170): dAcNpcSpGkA_c::talk_c::startGoOutEnd() -
// changeMode(MODE_GO_OUT_END)
void dAcNpcSpGkA_c::talk_c::startGoOutEnd() {
    dMsg::Comp_c *controller = mpController;
    controller->mLock = 1;
    controller->mWaitIconSe = 1;
    controller->mWaitIcon = 1;
    changeMode(&talk_c::executeGoOutEnd);
}

// .text:0x6B58 fn_66_6B58 (0x18C): dAcNpcSpGkA_c::talk_c::startRetire() - owner
// changeMode(MODE_RETIRE_NET_VST)
void dAcNpcSpGkA_c::talk_c::startRetire() {
    mpOwner->changeMode(&dAcNpcSpGkA_c::executeRetireNetVst);
    mRequestEndMethod = &talk_c::resetSpeakerFeel2;
}

// .text:0x6CE4 fn_66_6CE4 (0xC): dAcNpcSpGkA_c::talk_c::endMsg6CE4() -
// provisional
void dAcNpcSpGkA_c::talk_c::startGoOutConnect() {
    mConnectPurpose = CONNECT_GO_OUT;
    startConnect();
}

// .text:0x6CF0 fn_66_6CF0 (0x10): dAcNpcSpGkA_c::talk_c::endMsg6CF0() -
// provisional
void dAcNpcSpGkA_c::talk_c::showTownList() {
    setNextMessage(MSG_TOWN_LIST, dANpcSpGatekeeperA::l_msgLabel);
}

// .text:0x6D00 fn_66_6D00 (0x4C): dAcNpcSpGkA_c::talk_c::openFriendMenu() -
// provisional (reqMenu19, end onFriendMenuEnd)
void dAcNpcSpGkA_c::talk_c::openFriendMenu() {
    reqMenu19(0);
    mRequestEndMethod = &talk_c::onFriendMenuEnd;
}

// .text:0x6D4C fn_66_6D4C (0x4C): dAcNpcSpGkA_c::talk_c::endMsg6D4C() -
// provisional
void dAcNpcSpGkA_c::talk_c::cancelGoOut() {
    setDisconnectWaitIcon();
    int msg = selectNetMsg2();
    setNextMessage(msg, dANpcSpGatekeeperA::l_msgLabel);
}

// .text:0x6D98 fn_66_6D98 (0x170): dAcNpcSpGkA_c::talk_c::startGoOutStart() -
// changeMode(MODE_GO_OUT_START)
void dAcNpcSpGkA_c::talk_c::startGoOutStart() {
    dMsg::Comp_c *controller = mpController;
    controller->mLock = 1;
    controller->mWaitIconSe = 1;
    controller->mWaitIcon = 1;
    changeMode(&talk_c::executeGoOutStart);
}

// .text:0x6F08 fn_66_6F08 (0x1C0):
// dAcNpcSpGkA_c::talk_c::startGoOutNetVstDemo() - owner
// changeMode(MODE_GO_OUT_NET_VST)
void dAcNpcSpGkA_c::talk_c::startGoOutNetVstDemo() {
    dBgm::l_mgr.mState.mFlags |= dBgm::State_c::FLAG_10;
    mpOwner->changeMode(&dAcNpcSpGkA_c::executeGoOutNetVst);
    setNextMessage(SV_MSG_SAVING_VISIT, dSvMgr_c::mMessageLabel);
    setRequest1(0);
    mRequestEndMethod = &talk_c::resetSpeakerFeel;
}

// .text:0x70C8 fn_66_70C8 (0x4): dAcNpcSpGkA_c::talk_c::endMsg70C8() -
// provisional
void dAcNpcSpGkA_c::talk_c::endTownBusy() {
    dNetMgr_c::fn_800E55B0();
}

// .text:0x70CC fn_66_70CC (0x4C): dAcNpcSpGkA_c::talk_c::endMsg70CC() -
// provisional
void dAcNpcSpGkA_c::talk_c::endTownRefused() {
    setDisconnectWaitIcon();
    int msg = selectNetMsg2();
    setNextMessage(msg, dANpcSpGatekeeperA::l_msgLabel);
}

// .text:0x7118 fn_66_7118 (0x70):
// dAcNpcSpGkA_c::talk_c::setDisconnectWaitIcon() - provisional
void dAcNpcSpGkA_c::talk_c::setDisconnectWaitIcon() {
    if (isCurrentMethod(&talk_c::executeWiFiDisconnect)) {
        dMsg::Comp_c *controller = mpController;
        controller->mLock = 1;
        controller->mWaitIconSe = 1;
        controller->mWaitIcon = 1;
    }
}

// .text:0x7188 fn_66_7188 (0x170): dAcNpcSpGkA_c::talk_c::startWiFiConnect() -
// changeMode(MODE_WIFI_CONNECT)
void dAcNpcSpGkA_c::talk_c::startWiFiConnect() {
    dMsg::Comp_c *controller = mpController;
    controller->mLock = 1;
    controller->mWaitIconSe = 1;
    controller->mWaitIcon = 1;
    changeMode(&talk_c::executeWiFiConnect);
}

// .text:0x72F8 fn_66_72F8 (0x44): dAcNpcSpGkA_c::talk_c::endMsg72F8() -
// provisional
void dAcNpcSpGkA_c::talk_c::endGoOutNoFriends() {
    int msg = selectNetMsg2();
    setNextMessage(msg, dANpcSpGatekeeperA::l_msgLabel);
}

// .text:0x733C fn_66_733C (0x174):
// dAcNpcSpGkA_c::talk_c::startSaveBeforeWiFiConnect()
void dAcNpcSpGkA_c::talk_c::startSaveBeforeWiFiConnect() {
    dMsg::Comp_c *controller = mpController;
    controller->mLock = 1;
    controller->mWaitIconSe = 0;
    controller->mWaitIcon = 1;
    changeMode(&talk_c::executeSaveBeforeWiFiConnect);
}

// .text:0x74B0 fn_66_74B0 (0x1AC): dAcNpcSpGkA_c::talk_c::startGoOutNetVst()
void dAcNpcSpGkA_c::talk_c::startGoOutNetVst() {
    if (mpOwner->isMode(&dAcNpcSpGkA_c::executeGoOutNetVst)) {
        changeMode(&talk_c::executeGoOutNetVst);
    }
    dMsg::Comp_c *controller = mpController;
    controller->mLock = 1;
    controller->mWaitIconSe = 0;
    controller->mWaitIcon = 1;
}

// .text:0x765C fn_66_765C (0x80): dAcNpcSpGkA_c::talk_c::endMsg765C() -
// provisional
void dAcNpcSpGkA_c::talk_c::endNetRestricted() {
    switchToGatekeeperSpeaker();
    switch (mConnectPurpose) {
    case CONNECT_FRIEND_CODE:
    case CONNECT_OPEN_GATE:
    case CONNECT_FRIEND_CODE_OPEN_GATE:
        setNextMessage(MSG_MENU_AGAIN, dANpcSpGatekeeperA::l_msgLabel);
        break;
    case CONNECT_GO_OUT:
        int msg = selectNetMsg2();
        setNextMessage(msg, dANpcSpGatekeeperA::l_msgLabel);
        break;
    }
}

// .text:0x76DC fn_66_76DC (0x50): dAcNpcSpGkA_c::talk_c::endMsg76DC() -
// provisional
void dAcNpcSpGkA_c::talk_c::startOpenGateConnect() {
    if (dWifi::getMyFriendKey() != 0) {
        mConnectPurpose = CONNECT_OPEN_GATE;
    } else {
        mConnectPurpose = CONNECT_FRIEND_CODE_OPEN_GATE;
    }
    startConnect();
}

// .text:0x772C fn_66_772C (0x164): dAcNpcSpGkA_c::talk_c::startOpenGate() -
// owner changeMode(MODE_OPEN_GATE_NET_HST)
void dAcNpcSpGkA_c::talk_c::startOpenGate() {
    mpOwner->changeMode(&dAcNpcSpGkA_c::executeOpenGateNetHst);
}

// .text:0x7890 fn_66_7890 (0x170): dAcNpcSpGkA_c::talk_c::startCloseGate() -
// owner changeMode(MODE_CLOSE_GATE_NET_HST)
void dAcNpcSpGkA_c::talk_c::startCloseGate() {
    setDisconnectWaitIcon();
    dSvMgr_c::closeGateForSave();
    mpOwner->changeMode(&dAcNpcSpGkA_c::executeCloseGateNetHst);
}

// .text:0x7A00 fn_66_7A00 (0x170):
// dAcNpcSpGkA_c::talk_c::startGoOutDsAppliDemo() - owner
// changeMode(MODE_GO_OUT_DS_APPLI)
void dAcNpcSpGkA_c::talk_c::startGoOutDsAppliDemo() {
    mpController->mBgmKeep = 1;
    mpOwner->changeMode(&dAcNpcSpGkA_c::executeGoOutDsAppli);
}

// .text:0x7B70 fn_66_7B70 (0x138): dAcNpcSpGkA_c::talk_c::onAnswer(int answer)
void dAcNpcSpGkA_c::talk_c::onAnswer(int answer) {
    struct Entry {
        const char *label;
        int code;
        int answer;
        Method method;
    };
    static Entry messages[] = {
        {dANpcSpGatekeeperA::l_msgLabel, MSG_MENU_GATE_OPEN, 0,
         &talk_c::answerGoOut},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_MENU_GATE_OPEN, 1,
         &talk_c::answerCloseGate},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_MENU_GATE_OPEN2, 0,
         &talk_c::answerGoOut},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_MENU_GATE_OPEN2, 1,
         &talk_c::answerCloseGate},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_MENU_AGAIN, 0,
         &talk_c::answerGoOut},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_MENU_AGAIN, 1,
         &talk_c::answerInviteGuests},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_MENU_AGAIN, 2,
         &talk_c::answerFriendCode},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_MENU, 0, &talk_c::answerGoOut},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_MENU, 1,
         &talk_c::answerInviteGuests},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_MENU, 2,
         &talk_c::answerFriendCode},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_MENU2, 0, &talk_c::answerGoOut},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_MENU2, 1,
         &talk_c::answerInviteGuests},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_MENU2, 2,
         &talk_c::answerFriendCode},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_GO_OUT_HOW, 0,
         &talk_c::answerGoOutWiFi},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_GO_OUT_HOW, 2,
         &talk_c::answerBackToMenu},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_GO_OUT_NO_FRIEND_CODE, 1,
         &talk_c::answerBackToMenu},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_GO_OUT_INSTEAD, 0,
         &talk_c::answerGoOutInstead},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_GO_OUT_INSTEAD, 1,
         &talk_c::answerBackToMenu},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_READY_TO_GO_HOME, 1,
         &talk_c::answerRemindGoHome},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_DS_NOT_FOUND, 0,
         &talk_c::answerDsRetry},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_DS_NOT_FOUND, 1,
         &talk_c::answerRemindGoHome},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_DS_ERROR, 0,
         &talk_c::answerDsRetry},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_DS_ERROR, 1,
         &talk_c::answerRemindGoHome},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_DS_ASK_QUIT, 1,
         &talk_c::answerRemindGoHome},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_TOWN_BUSY, 0,
         &talk_c::answerSystemSpeaker},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_TOWN_NO_RESPONSE, 0,
         &talk_c::answerSystemSpeaker},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_TOWN_IN_CONVERSATION, 0,
         &talk_c::answerSystemSpeaker},
        {dANpcSpGatekeeperA::l_msgLabel, MSG_TOWN_UNAVAILABLE, 0,
         &talk_c::answerSystemSpeaker},
    };
    u16 code = getMessageCode();
    const char *label;
    int selected = mpController->mSelect.mCursor;
    Entry *entry = messages;
    label = getMessageLabel();
    for (int i = 0; i < (int)ARRAY_SIZE(messages); i++, entry++) {
        if (std::strcmp(label, entry->label) == 0 && code == entry->code &&
            selected == entry->answer) {
            callMessageMethod(entry->method);
        }
    }
}

// .text:0x7CA8 fn_66_7CA8 (0x4): dAcNpcSpGkA_c::talk_c::answer7CA8() -
// provisional
void dAcNpcSpGkA_c::talk_c::answerSystemSpeaker() {
    switchToSystemSpeaker();
}

// .text:0x7CAC fn_66_7CAC (0x44): dAcNpcSpGkA_c::talk_c::answer7CAC() -
// provisional
void dAcNpcSpGkA_c::talk_c::answerBackToMenu() {
    int msg = selectNetMsg2();
    setNextMessage(msg, dANpcSpGatekeeperA::l_msgLabel);
}

// .text:0x7CF0 fn_66_7CF0 (0x178): dAcNpcSpGkA_c::talk_c::answer7CF0() -
// provisional
void dAcNpcSpGkA_c::talk_c::answerGoOutInstead() {
    dARmobjCheckp::closeGate();
    dSvMgr_c::closeGateForSave();
    dMsg::Comp_c *controller = mpController;
    controller->mLock = 1;
    controller->mWaitIconSe = 1;
    controller->mWaitIcon = 1;
    changeMode(&talk_c::executeWiFiDisconnect);
}

// .text:0x7E68 fn_66_7E68 (0xB0): dAcNpcSpGkA_c::talk_c::answer7E68() -
// provisional
void dAcNpcSpGkA_c::talk_c::answerGoOutWiFi() {
    if (SCCheckPCMessageRestriction()) {
        switchToSystemSpeaker();
        setNextMessage(NET_ERROR_MSG_RESTRICTED, sNetErrorMessageLabel);
    } else if (dWifi::getFriendNum() == 0) {
        setNextMessage(MSG_GO_OUT_NO_FRIENDS, dANpcSpGatekeeperA::l_msgLabel);
    } else if (dWifi::getMyFriendKey() != 0) {
        setNextMessage(MSG_SEARCH_GATE, dANpcSpGatekeeperA::l_msgLabel);
    } else {
        setNextMessage(MSG_GO_OUT_NO_FRIEND_CODE,
                       dANpcSpGatekeeperA::l_msgLabel);
    }
}

// .text:0x7F18 fn_66_7F18 (0x84): dAcNpcSpGkA_c::talk_c::answer7F18() -
// provisional
void dAcNpcSpGkA_c::talk_c::answerGoOut() {
    if (isNetActive()) {
        if (isMultiPlay()) {
            setNextMessage(MSG_CANT_LEAVE, dANpcSpGatekeeperA::l_msgLabel);
        } else {
            setNextMessage(MSG_GO_OUT_INSTEAD, dANpcSpGatekeeperA::l_msgLabel);
        }
    } else {
        setNextMessage(MSG_GO_OUT_HOW, dANpcSpGatekeeperA::l_msgLabel);
    }
}

// .text:0x7F9C fn_66_7F9C (0x8C): dAcNpcSpGkA_c::talk_c::answer7F9C() -
// provisional
void dAcNpcSpGkA_c::talk_c::answerInviteGuests() {
    if (SCCheckPCMessageRestriction()) {
        switchToSystemSpeaker();
        setNextMessage(NET_ERROR_MSG_RESTRICTED, sNetErrorMessageLabel);
    } else if (dWifi::getFriendNum() == 0) {
        setNextMessage(MSG_NO_FRIENDS, dANpcSpGatekeeperA::l_msgLabel);
    } else {
        setNextMessage(MSG_OPEN_GATE_START, dANpcSpGatekeeperA::l_msgLabel);
    }
}

// .text:0x8028 fn_66_8028 (0x8C): dAcNpcSpGkA_c::talk_c::answer8028() -
// provisional
void dAcNpcSpGkA_c::talk_c::answerFriendCode() {
    if (dWifi::getMyFriendKey() != 0) {
        setNextMessage(MSG_FRIEND_CODE, dANpcSpGatekeeperA::l_msgLabel);
    } else if (SCCheckPCMessageRestriction()) {
        switchToSystemSpeaker();
        setNextMessage(NET_ERROR_MSG_RESTRICTED, sNetErrorMessageLabel);
    } else {
        setNextMessage(MSG_NO_FRIEND_CODE, dANpcSpGatekeeperA::l_msgLabel);
    }
}

// .text:0x80B4 fn_66_80B4 (0x60): dAcNpcSpGkA_c::talk_c::answer80B4() -
// provisional
void dAcNpcSpGkA_c::talk_c::answerCloseGate() {
    if (isMultiPlay()) {
        setNextMessage(MSG_CANT_CLOSE_GATE, dANpcSpGatekeeperA::l_msgLabel);
    } else {
        setNextMessage(MSG_CLOSE_GATE, dANpcSpGatekeeperA::l_msgLabel);
    }
}

// .text:0x8114 fn_66_8114 (0x60): dAcNpcSpGkA_c::talk_c::answer8114() -
// provisional
void dAcNpcSpGkA_c::talk_c::answerDsRetry() {
    if (!isPlayerFromThisTown()) {
        setNextMessage(MSG_DS_TURN_ON_VISITOR, dANpcSpGatekeeperA::l_msgLabel);
    } else {
        setNextMessage(MSG_DS_TURN_ON, dANpcSpGatekeeperA::l_msgLabel);
    }
}

// .text:0x8174 fn_66_8174 (0x48): dAcNpcSpGkA_c::talk_c::answer8174() -
// provisional
void dAcNpcSpGkA_c::talk_c::answerRemindGoHome() {
    if (!isPlayerFromThisTown()) {
        setNextMessage(MSG_GO_HOME_REMINDER, dANpcSpGatekeeperA::l_msgLabel);
    }
}

// .text:0x81BC fn_66_81BC (0xF4): dAcNpcSpGkA_c::talk_c::formatFriendCode(u64
// key) - provisional signature
void dAcNpcSpGkA_c::talk_c::formatFriendCode(u64 key) {
    u64 high = key / 100000000;
    u64 mid = (key - high * 100000000) / 10000;
    formatNumber(high, 0, 4, dScript::NUM_FORMAT_ZERO_PAD);
    formatNumber(mid, 1, 4, dScript::NUM_FORMAT_ZERO_PAD);
    formatNumber(key - mid * 10000 - high * 100000000, 2, 4,
                 dScript::NUM_FORMAT_ZERO_PAD);
}

// .text:0x82B0 fn_66_82B0 (0x98): dAcNpcSpGkA_c::talk_c::onRequestEnd(int kind)
void dAcNpcSpGkA_c::talk_c::onRequestEnd(int kind) {
    if (mRequestEndMethod) {
        Method method = mRequestEndMethod;
        callMessageMethod(method);
        if (method == mRequestEndMethod) {
            mRequestEndMethod = NULL;
        }
    }
}

// .text:0x8348 fn_66_8348 (0x40): dAcNpcSpGkA_c::talk_c::hideSpeaker() -
// request-end method of switchToSystemSpeaker
void dAcNpcSpGkA_c::talk_c::hideSpeaker() {
    mpOwner->mTalk.mNoMouth = 1;
    clearSpeakerName();
    reqMsgClose();
}

// .text:0x8388 fn_66_8388 (0x90): dAcNpcSpGkA_c::talk_c::showSpeaker() -
// request-end method of switchToGatekeeperSpeaker
void dAcNpcSpGkA_c::talk_c::showSpeaker() {
    mpOwner->mTalk.mNoMouth = 0;
    dHmnName::Word_c name;
    dItem::Item item((u16)SP_NPC_GATEKEEPER_A);
    dNpc::getNpcName(&name, &item, LANGUAGE_NUM);
    setSpeakerName(
        (const u16 *)static_cast<dScript::Word_c &>(name).getBuffer(), 2);
    reqMsgClose();
}

// .text:0x8418 fn_66_8418 (0xCC): dAcNpcSpGkA_c::talk_c::onFriendMenuEnd() -
// provisional
void dAcNpcSpGkA_c::talk_c::onFriendMenuEnd() {
    if (dANpcSpGatekeeperA::isNetError()) {
        mpOwner->mTalk.mNoMouth = 1;
        clearSpeakerName();
        setNetErrorMsg();
    } else if (isMenuInvalid()) {
        setNextMessage(MSG_GO_OUT_CANCELLED, dANpcSpGatekeeperA::l_msgLabel);
    } else {
        mFriendSlot = dMenuMng_c::getSelSlot();
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        setLandName(&player->mFriends.mFriends[mFriendSlot].mPID.land, 0);
        mpOwner->mTalk.mNoMouth = 1;
        clearSpeakerName();
        setNextMessage(MSG_INQUIRING, dANpcSpGatekeeperA::l_msgLabel);
    }
}

// .text:0x84E4 fn_66_84E4 (0x14): dAcNpcSpGkA_c::talk_c::resetSpeakerFeel() -
// setSpeakerFeel(0)
void dAcNpcSpGkA_c::talk_c::resetSpeakerFeel() {
    setSpeakerFeel(0);
}

// .text:0x84F8 fn_66_84F8 (0x14): dAcNpcSpGkA_c::talk_c::resetSpeakerFeel2() -
// setSpeakerFeel(0)
void dAcNpcSpGkA_c::talk_c::resetSpeakerFeel2() {
    setSpeakerFeel(0);
}

// .text:0x850C fn_66_850C (0x4): dAcNpcSpGkA_c::talk_c::initWait()
void dAcNpcSpGkA_c::talk_c::initWait() {
}

// .text:0x8510 fn_66_8510 (0x4): dAcNpcSpGkA_c::talk_c::executeWait()
void dAcNpcSpGkA_c::talk_c::executeWait() {
}

// .text:0x8514 fn_66_8514 (0x19C):
// dAcNpcSpGkA_c::talk_c::initSaveBeforeWiFiConnect()
void dAcNpcSpGkA_c::talk_c::initSaveBeforeWiFiConnect() {
    static const stepSaveBeforeWiFiConnect_c::Entry steps[] = {
        {"HBMとReset禁止ON",
         (stepSaveBeforeWiFiConnect_c::Method)&stepSaveBeforeWiFiConnect_c::
             disableHomeAndReset},
        {"DL BOX生成初期化",
         (stepSaveBeforeWiFiConnect_c::Method)&stepSaveBeforeWiFiConnect_c::
             initializeDownloadBox},
        {"DL BOX生成",
         (stepSaveBeforeWiFiConnect_c::Method)&stepSaveBeforeWiFiConnect_c::
             createDownloadBox},
        {"全体セーブ初期化",
         (stepSaveBeforeWiFiConnect_c::Method)&stepSaveBeforeWiFiConnect_c::
             initializeFullSave},
        {"全体セーブ開始",
         (stepSaveBeforeWiFiConnect_c::Method)&stepSaveBeforeWiFiConnect_c::
             startFullSave},
        {"全体セーブ終了",
         (stepSaveBeforeWiFiConnect_c::Method)&stepSaveBeforeWiFiConnect_c::
             waitFullSave},
        {"HBMとReset禁止OFF",
         (stepSaveBeforeWiFiConnect_c::Method)&stepSaveBeforeWiFiConnect_c::
             enableHomeAndReset},
        {"終了", (stepSaveBeforeWiFiConnect_c::
                      Method)&stepSaveBeforeWiFiConnect_c::finish},
        {"セーブ失敗", (stepSaveBeforeWiFiConnect_c::
                            Method)&stepSaveBeforeWiFiConnect_c::saveFailed},
    };
    mStepSaveBeforeWiFiConnect.initialize(
        this, "dAcNpcSpGkA_c::talk_c::stepSaveBeforeWiFiConnect_c", steps,
        ARRAY_SIZE(steps));
}

// .text:0x86B0 fn_66_86B0 (0x48):
// dAcNpcSpGkA_c::talk_c::executeSaveBeforeWiFiConnect()
void dAcNpcSpGkA_c::talk_c::executeSaveBeforeWiFiConnect() {
    mStepSaveBeforeWiFiConnect.execute();
}

// .text:0x86F8 fn_66_86F8 (0x1A0):
// dAcNpcSpGkA_c::talk_c::stepSaveBeforeWiFiConnect_c::finish() - "終了"
void dAcNpcSpGkA_c::talk_c::stepSaveBeforeWiFiConnect_c::finish() {
    talk_c *talk = getOwner();
    talk->mSavedBeforeConnect = 1;
    dSvRuntime::releaseMessage(getOwner()->getController());
    getOwner()->setNextMessage(talk_c::MSG_CONNECTING,
                               dANpcSpGatekeeperA::l_msgLabel);
    getOwner()->changeMode(&talk_c::executeWait);
}

// .text:0x8898 fn_66_8898 (0x150): dAcNpcSpGkA_c::talk_c::initWiFiConnect()
void dAcNpcSpGkA_c::talk_c::initWiFiConnect() {
    static const stepWiFiConnect_c::Entry steps[] = {
        {"WiFiコネクションに接続",
         (stepWiFiConnect_c::Method)&stepWiFiConnect_c::connect},
        {"WiFiコネクションに接続確立",
         (stepWiFiConnect_c::Method)&stepWiFiConnect_c::waitConnect},
        {"ネットホストセーブ初期化",
         (stepWiFiConnect_c::Method)&stepWiFiConnect_c::initializeHostSave},
        {"ネットホストセーブ開始",
         (stepWiFiConnect_c::Method)&stepWiFiConnect_c::startHostSave},
        {"ネットホストセーブ終了",
         (stepWiFiConnect_c::Method)&stepWiFiConnect_c::waitHostSave},
        {"終了", (stepWiFiConnect_c::Method)&stepWiFiConnect_c::finish},
        {"セーブ失敗",
         (stepWiFiConnect_c::Method)&stepWiFiConnect_c::saveFailed},
    };
    mStepWiFiConnect.initialize(this,
                                "dAcNpcSpGkA_c::talk_c::stepWiFiConnect_c",
                                steps, ARRAY_SIZE(steps));
}

// .text:0x89E8 fn_66_89E8 (0x48): dAcNpcSpGkA_c::talk_c::executeWiFiConnect()
void dAcNpcSpGkA_c::talk_c::executeWiFiConnect() {
    mStepWiFiConnect.execute();
}

// .text:0x8A30 fn_66_8A30 (0x9C):
// dAcNpcSpGkA_c::talk_c::stepWiFiConnect_c::connect() -
// "WiFiコネクションに接続"
void dAcNpcSpGkA_c::talk_c::stepWiFiConnect_c::connect() {
    dNet::initNetMgr();
    nextStep();
}

// .text:0x8ACC fn_66_8ACC (0x150):
// dAcNpcSpGkA_c::talk_c::stepWiFiConnect_c::waitConnect() -
// "WiFiコネクションに接続確立"
void dAcNpcSpGkA_c::talk_c::stepWiFiConnect_c::waitConnect() {
    if (dANpcSpGatekeeperA::isNetError()) {
        int kind = getOwner()->mConnectPurpose;
        if ((kind == CONNECT_FRIEND_CODE ||
             kind == CONNECT_FRIEND_CODE_OPEN_GATE) &&
            dWifi::getMyFriendKey() != 0) {
            nextStep();
        } else {
            getOwner()->setNetErrorMsg();
        }
    } else if (dNet::openNetConnection()) {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->clearFlag0(
            PRIVATE_FLAG0_PLAYING_UNSAVED);
        nextStep();
    }
}

// .text:0x8C1C fn_66_8C1C (0x5DC):
// dAcNpcSpGkA_c::talk_c::stepWiFiConnect_c::finish() - "終了"
void dAcNpcSpGkA_c::talk_c::stepWiFiConnect_c::finish() {
    getOwner()->switchToGatekeeperSpeaker();
    switch (getOwner()->mConnectPurpose) {
    case CONNECT_FRIEND_CODE:
        getOwner()->setNextMessage(talk_c::MSG_FRIEND_CODE_SAVED2,
                                   dANpcSpGatekeeperA::l_msgLabel);
        getOwner()->changeMode(&talk_c::executeWiFiDisconnect);
        break;
    case CONNECT_GO_OUT:
        dSvRuntime::releaseMessage(getOwner()->getController());
        getOwner()->setNextMessage(talk_c::MSG_TOWN_LIST,
                                   dANpcSpGatekeeperA::l_msgLabel);
        getOwner()->changeMode(&talk_c::executeWait);
        break;
    case CONNECT_OPEN_GATE:
        dSvRuntime::releaseMessage(getOwner()->getController());
        if (isAnyFriendInSession()) {
            getOwner()->setNextMessage(talk_c::MSG_FRIEND_GATE_OPEN,
                                       dANpcSpGatekeeperA::l_msgLabel);
        } else {
            getOwner()->setNextMessage(talk_c::MSG_OPEN_GATE,
                                       dANpcSpGatekeeperA::l_msgLabel);
        }
        getOwner()->changeMode(&talk_c::executeWait);
        break;
    case CONNECT_FRIEND_CODE_OPEN_GATE:
        dSvRuntime::releaseMessage(getOwner()->getController());
        getOwner()->setNextMessage(talk_c::MSG_FRIEND_CODE_SAVED,
                                   dANpcSpGatekeeperA::l_msgLabel);
        getOwner()->changeMode(&talk_c::executeWait);
        break;
    }
}

// .text:0x91F8 fn_66_91F8 (0x4): dAcNpcSpGkA_c::talk_c::initWiFiDisconnect()
void dAcNpcSpGkA_c::talk_c::initWiFiDisconnect() {
}

// .text:0x91FC fn_66_91FC (0x190):
// dAcNpcSpGkA_c::talk_c::executeWiFiDisconnect()
void dAcNpcSpGkA_c::talk_c::executeWiFiDisconnect() {
    if (dNet::disconnectNet(0)) {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
            PRIVATE_FLAG0_PLAYING_UNSAVED);
        dSvRuntime::releaseMessage(mpController);
        mLockRefused = 0;
        changeMode(&talk_c::executeWait);
    }
}

// .text:0x938C fn_66_938C (0x798): dAcNpcSpGkA_c::talk_c::checkNetError(int
// arg)
BOOL dAcNpcSpGkA_c::talk_c::checkNetError(int errorMsgOnly) {
    int code;
    BOOL ret = TRUE;
    dMsg::Comp_c *controller;
    int kind = dNetMgr_c::getErrorKind();
    controller = mpController;
    code = dNetMgr_c::sInstance->mInfo.mError.mDetail;
    switch (kind) {
    case dNetMgr_c::ERROR_KIND_NONE:
        ret = FALSE;
        break;
    case dNetMgr_c::ERROR_KIND_GATE_CLOSED:
        switchToGatekeeperSpeaker();
        setNextMessage(talk_c::MSG_TOWN_GATE_CLOSED,
                       dANpcSpGatekeeperA::l_msgLabel);
        dSvRuntime::releaseMessage(controller);
        changeMode(&talk_c::executeWait);
        break;
    case dNetMgr_c::ERROR_KIND_TOWN_FULL:
        switchToGatekeeperSpeaker();
        setNextMessage(talk_c::MSG_TOWN_FULL, dANpcSpGatekeeperA::l_msgLabel);
        dSvRuntime::releaseMessage(controller);
        changeMode(&talk_c::executeWait);
        break;
    case dNetMgr_c::ERROR_KIND_TOWN_BUSY:
        if (errorMsgOnly) {
            setNetErrorMsg();
        } else {
            switchToGatekeeperSpeaker();
            setNextMessage(talk_c::MSG_TOWN_BUSY,
                           dANpcSpGatekeeperA::l_msgLabel);
            dSvRuntime::releaseMessage(controller);
            changeMode(&talk_c::executeWait);
        }
        break;
    case dNetMgr_c::ERROR_KIND_OTHER:
        if (errorMsgOnly) {
            setNetErrorMsg();
        } else if (code == 0) {
            switchToGatekeeperSpeaker();
            setNextMessage(talk_c::MSG_TOWN_UNAVAILABLE,
                           dANpcSpGatekeeperA::l_msgLabel);
            dSvRuntime::releaseMessage(controller);
            changeMode(&talk_c::executeWait);
        } else if (code == NET_ERROR_CODE_NO_RESPONSE) {
            switchToGatekeeperSpeaker();
            setNextMessage(talk_c::MSG_TOWN_NO_RESPONSE,
                           dANpcSpGatekeeperA::l_msgLabel);
            dSvRuntime::releaseMessage(controller);
            changeMode(&talk_c::executeWait);
        } else {
            setNetErrorMsg();
        }
        break;
    case dNetMgr_c::ERROR_KIND_RESTART:
    case dNetMgr_c::ERROR_KIND_PERMANENT:
        setNetErrorMsg();
        break;
    }
    if (ret) {
        controller->formatNumber(0, code, 5, dScript::NUM_FORMAT_ZERO_PAD);
    }
    return ret;
}

// .text:0x9B24 fn_66_9B24 (0x1D8): dAcNpcSpGkA_c::talk_c::initGoOutStart()
void dAcNpcSpGkA_c::talk_c::initGoOutStart() {
    static const stepConnectNetVst_c::Entry steps[] = {
        {"「子機としてマッチング」を要求",
         (stepConnectNetVst_c::Method)&stepConnectNetVst_c::startGuest},
        {"親機に「デモのロック要求」を送信",
         (stepConnectNetVst_c::Method)&stepConnectNetVst_c::requestConnect},
        {"親機への「デモのロック要求」の結果を取得",
         (stepConnectNetVst_c::Method)&stepConnectNetVst_c::getConnectResult},
        {"NPCセーブ初期化",
         (stepConnectNetVst_c::Method)&stepConnectNetVst_c::initializeNpcSave},
        {"NPCセーブ開始",
         (stepConnectNetVst_c::Method)&stepConnectNetVst_c::startNpcSave},
        {"NPCセーブ終了",
         (stepConnectNetVst_c::Method)&stepConnectNetVst_c::waitNpcSave},
        {"「新規参入時セーブデータ送受信処理」開始前の設定 ",
         (stepConnectNetVst_c::Method)&stepConnectNetVst_c::setupJoinSave},
        {"「新規参入プレイヤー表示のためのデータ」の送信が終わっているか？",
         (stepConnectNetVst_c::Method)&stepConnectNetVst_c::waitJoinPlayerSent},
        {"「新規参入マシンがセーブデータ要求」を送信要求セット",
         (stepConnectNetVst_c::Method)&stepConnectNetVst_c::requestTown},
        {"セーブ失敗",
         (stepConnectNetVst_c::Method)&stepConnectNetVst_c::saveFailed},
    };
    mStepConnectNetVst.initialize(this,
                                  "dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c",
                                  steps, ARRAY_SIZE(steps));
}

// .text:0x9CFC fn_66_9CFC (0x48): dAcNpcSpGkA_c::talk_c::executeGoOutStart()
void dAcNpcSpGkA_c::talk_c::executeGoOutStart() {
    mStepConnectNetVst.execute();
}

// .text:0x9D44 fn_66_9D44 (0x138):
// dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c::startGuest() -
// "「子機としてマッチング」を要求"
void dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c::startGuest() {
    if (!getOwner()->checkNetError(0)) {
        talk_c *talk = getOwner();
        if (talk->mLockRefused) {
            talk->mLockRefused = 0;
            nextStep();
        } else if (dNet::startGuest(talk->mFriendSlot)) {
            nextStep();
        }
    }
}

// .text:0x9E7C fn_66_9E7C (0xB8):
// dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c::requestConnect() -
// "親機に「デモのロック要求」を送信"
void dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c::requestConnect() {
    if (!getOwner()->checkNetError(0) && dNet::requestConnect()) {
        nextStep();
    }
}

// .text:0x9F34 fn_66_9F34 (0x290):
// dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c::getConnectResult() -
// "親機への「デモのロック要求」の結果を取得"
void dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c::getConnectResult() {
    if (!dANpcSpGatekeeperA::isHostConnected()) {
        getOwner()->setNetErrorMsg();
        return;
    }
    int result = dNet::getRequestResult();
    if (result > 0) {
        dItem::Item key((u16)SP_NPC_MISSING_CHILD);
        dAcNpc_c *npc = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(key));
        if (npc != NULL) {
            npc->clearNetSync();
        }
        nextStep();
    } else if (result < 0) {
        getOwner()->switchToGatekeeperSpeaker();
        getOwner()->setNextMessage(talk_c::MSG_TOWN_IN_CONVERSATION,
                                   dANpcSpGatekeeperA::l_msgLabel);
        dSvRuntime::releaseMessage(getOwner()->getController());
        getOwner()->changeMode(&talk_c::executeWait);
        getOwner()->mLockRefused = 1;
    } else {
        getOwner()->checkNetError(1);
    }
}

// .text:0xA1C4 fn_66_A1C4 (0xC0):
// dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c::setupJoinSave() -
// "「新規参入時セーブデータ送受信処理」開始前の設定"
void dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c::setupJoinSave() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (dNpcInfo::gatekeeperA_c::get()->mEscorting) {
        player->mHost.mFlagB = 1;
    }
    dNet::startJoinTransfer();
    nextStep();
}

// .text:0xA284 fn_66_A284 (0xA4):
// dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c::waitJoinPlayerSent() -
// "「新規参入プレイヤー表示のためのデータ」の送信が終わっているか？"
void dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c::waitJoinPlayerSent() {
    if (dNet::isJoinPlayerSent()) {
        nextStep();
    }
}

// .text:0xA328 fn_66_A328 (0x1A8):
// dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c::requestTown() -
// "「新規参入マシンがセーブデータ要求」を送信要求セット"
void dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c::requestTown() {
    if (dNet::requestTown()) {
        getOwner()->switchToGatekeeperSpeaker();
        getOwner()->setNextMessage(talk_c::MSG_SAFE_JOURNEY,
                                   dANpcSpGatekeeperA::l_msgLabel);
        dSvRuntime::releaseMessage(getOwner()->getController());
        getOwner()->changeMode(&talk_c::executeWait);
    }
}

// .text:0xA4D0 fn_66_A4D0 (0xFC): dAcNpcSpGkA_c::talk_c::initGoOutEnd()
void dAcNpcSpGkA_c::talk_c::initGoOutEnd() {
    static const stepDisconnectNetVst_c::Entry steps[] = {
        {"デモのロック",
         (stepDisconnectNetVst_c::Method)&stepDisconnectNetVst_c::
             requestDisconnect},
        {"要求の結果", (stepDisconnectNetVst_c::Method)&stepDisconnectNetVst_c::
                           getRequestResult},
        {"セーブ可能か調べる",
         (stepDisconnectNetVst_c::Method)&stepDisconnectNetVst_c::
             checkSaveReady},
        {"メッセージ終了待ち→デモへ",
         (stepDisconnectNetVst_c::Method)&stepDisconnectNetVst_c::waitMsgEnd},
    };
    mStepDisconnectNetVst.initialize(
        this, "dAcNpcSpGkA_c::talk_c::stepDisconnectNetVst_c", steps,
        ARRAY_SIZE(steps));
}

// .text:0xA5CC fn_66_A5CC (0x48): dAcNpcSpGkA_c::talk_c::executeGoOutEnd()
void dAcNpcSpGkA_c::talk_c::executeGoOutEnd() {
    mStepDisconnectNetVst.execute();
}

// .text:0xA614 fn_66_A614 (0xA4):
// dAcNpcSpGkA_c::talk_c::stepDisconnectNetVst_c::requestDisconnect() -
// "デモのロック"
void dAcNpcSpGkA_c::talk_c::stepDisconnectNetVst_c::requestDisconnect() {
    if (dNet::requestDisconnect()) {
        nextStep();
    }
}

// .text:0xA6B8 fn_66_A6B8 (0x230):
// dAcNpcSpGkA_c::talk_c::stepDisconnectNetVst_c::getRequestResult() -
// "要求の結果"
void dAcNpcSpGkA_c::talk_c::stepDisconnectNetVst_c::getRequestResult() {
    int result = dNet::getRequestResult();
    if (result > 0) {
        getOwner()->mpOwner->clearNetSync();
        dItem::Item key((u16)SP_NPC_GATEKEEPER_B);
        static_cast<dAcNpc_c *>(dNpcList::getSpByKey(key))->clearNetSync();
        dNet::startDisconnect();
        nextStep();
    } else if (result < 0) {
        getOwner()->setNextMessage(talk_c::MSG_BUSY_IN_CONVERSATION,
                                   dANpcSpGatekeeperA::l_msgLabel);
        dSvRuntime::releaseMessage(getOwner()->getController());
        getOwner()->changeMode(&talk_c::executeWait);
    }
}

// .text:0xA8E8 fn_66_A8E8 (0xA4):
// dAcNpcSpGkA_c::talk_c::stepDisconnectNetVst_c::checkSaveReady() -
// "セーブ可能か調べる"
void dAcNpcSpGkA_c::talk_c::stepDisconnectNetVst_c::checkSaveReady() {
    if (dNet::isDisconnectStarted()) {
        nextStep();
    }
}

// .text:0xA98C fn_66_A98C (0x194):
// dAcNpcSpGkA_c::talk_c::stepDisconnectNetVst_c::waitMsgEnd() -
// "メッセージ終了待ち→デモへ"
void dAcNpcSpGkA_c::talk_c::stepDisconnectNetVst_c::waitMsgEnd() {
    getOwner()->setNextMessage(talk_c::MSG_GO_HOME_BYE,
                               dANpcSpGatekeeperA::l_msgLabel);
    dSvRuntime::releaseMessage(getOwner()->getController());
    getOwner()->changeMode(&talk_c::executeWait);
}

// .text:0xAB20 fn_66_AB20 (0x328): dAcNpcSpGkA_c::talk_c::initDsAppli()
void dAcNpcSpGkA_c::talk_c::initDsAppli() {
    static const stepGoOutDsAppli_c::Entry steps[] = {
        {"おでかけアプリ配信",
         (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::sendApp},
        {"接続チェック",
         (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::connect},
        {"送信完了チェック",
         (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::waitSendDone},
        {"ロック開始まで待つ",
         (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::waitLockStart},
        {"HBMとReset禁止ON",
         (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::disableHomeAndReset},
        {"DL BOX生成初期化", (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::
                                 initializeDownloadBox},
        {"DL BOX生成",
         (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::createDownloadBox},
        {"全体セーブ初期化",
         (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::initializeFullSave},
        {"全体セーブ開始",
         (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::startFullSave},
        {"全体セーブ終了",
         (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::waitFullSave},
        {"セーブ完了をDSに通知",
         (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::notifySaveDone},
        {"送信しきったか？",
         (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::waitSent},
        {"村＋その他データセーブ初期化",
         (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::initializeTownSave},
        {"村＋その他データセーブ開始",
         (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::startTownSave},
        {"村＋その他データセーブ終了",
         (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::waitTownSave},
        {"エラーメッセージをセット",
         (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::setErrorMsg},
        {"HBMとReset禁止OFF",
         (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::enableHomeAndReset},
        {"DS通信終了開始",
         (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::startEndDs},
        {"DS通信終了", (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::endDs},
        {"セーブ失敗",
         (stepGoOutDsAppli_c::Method)&stepGoOutDsAppli_c::saveFailed},
    };
    mStepGoOutDsAppli.initialize(this,
                                 "dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c",
                                 steps, ARRAY_SIZE(steps));
}

// .text:0xAE48 fn_66_AE48 (0x48): dAcNpcSpGkA_c::talk_c::executeDsAppli()
void dAcNpcSpGkA_c::talk_c::executeDsAppli() {
    mStepGoOutDsAppli.execute();
}

// .text:0xAE90 fn_66_AE90 (0xF4):
// dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::sendApp() - "おでかけアプリ配信"
void dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::sendApp() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dANpcSpGatekeeperA::l_travelData->prepareDeparture();
    } else {
        dANpcSpGatekeeperA::l_travelData->prepareVisit();
    }
    mTimer = 3 * 60 * 60;
    dDsDl::g_manager.sendToDs(dANpcSpGatekeeperA::l_travelData,
                              sizeof(*dANpcSpGatekeeperA::l_travelData));
    getOwner()->mpOwner->mDsDlActive = 1;
    nextStep();
}

// .text:0xAF84 fn_66_AF84 (0x1A0):
// dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::connect() - "接続チェック"
void dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::connect() {
    dDsDl::Manage_c *manager = &dDsDl::g_manager;
    dMsg::Comp_c *controller = getOwner()->getController();
    if (manager->mError) {
        getOwner()->setNextMessage(talk_c::MSG_DS_NOT_FOUND,
                                   dANpcSpGatekeeperA::l_msgLabel);
        abortSave();
    } else if (manager->isConnected()) {
        if (controller->mLock) {
            dSvRuntime::releaseMessage(controller);
        } else {
            controller->mForceNext = 1;
        }
        getOwner()->setNextMessage(talk_c::MSG_DS_COMMUNICATING,
                                   dANpcSpGatekeeperA::l_msgLabel);
        getOwner()->switchToSystemSpeaker();
        nextStep();
    } else if (controller->mLock) {
        if (mTimer != 0) {
            mTimer--;
        }
        if (mTimer == 0) {
            getOwner()->setNextMessage(talk_c::MSG_DS_NOT_FOUND,
                                       dANpcSpGatekeeperA::l_msgLabel);
            abortSave();
        } else if (dPad::isCancelTriggered()) {
            getOwner()->setNextMessage(talk_c::MSG_DS_ASK_QUIT,
                                       dANpcSpGatekeeperA::l_msgLabel);
            abortSave();
        }
    }
}

// .text:0xB124 fn_66_B124 (0x140):
// dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::waitSendDone() -
// "送信完了チェック"
void dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::waitSendDone() {
    dDsDl::Manage_c *manager = &dDsDl::g_manager;
    if (manager->mError) {
        getOwner()->setNextMessage(talk_c::MSG_DS_ERROR,
                                   dANpcSpGatekeeperA::l_msgLabel);
        abortSave();
    } else if (manager->isReceived()) {
        if (dAcNpc_c::isPlayerFromThisTown()) {
            dANpcSpGatekeeperA::l_travelData->backupDeparture();
        } else {
            dANpcSpGatekeeperA::l_travelData->backupVisit();
        }
        getOwner()->getController()->mLock = 0;
        getOwner()->setNextMessage(SV_MSG_SAVING_VISIT,
                                   dSvMgr_c::mMessageLabel);
        nextStep();
    }
}

// .text:0xB264 fn_66_B264 (0xAC):
// dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::waitLockStart() -
// "ロック開始まで待つ"
void dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::waitLockStart() {
    if (dMsg::Comp_c::mInstance->mLock) {
        nextStep();
    }
}

// .text:0xB310 fn_66_B310 (0x118):
// dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::notifySaveDone() -
// "セーブ完了をDSに通知"
void dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::notifySaveDone() {
    dDsDl::Manage_c *manager = &dDsDl::g_manager;
    if (manager->mError) {
        if (dAcNpc_c::isPlayerFromThisTown()) {
            dANpcSpGatekeeperA::l_travelData->cancelDeparture();
            mCurrentMethod = &stepGoOutDsAppli_c::initializeTownSave;
        } else {
            dANpcSpGatekeeperA::l_travelData->cancelVisit();
            mCurrentMethod = &stepGoOutDsAppli_c::setErrorMsg;
        }
    } else {
        manager->closeSend();
        nextStep();
    }
}

// .text:0xB428 fn_66_B428 (0x140):
// dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::waitSent() - "送信しきったか？"
void dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::waitSent() {
    dDsDl::Manage_c *manager = &dDsDl::g_manager;
    if (manager->isStopping()) {
        if (dAcNpc_c::isPlayerFromThisTown()) {
            getOwner()->setNextMessage(talk_c::MSG_DS_JOURNEY,
                                       dANpcSpGatekeeperA::l_msgLabel);
        } else {
            getOwner()->setNextMessage(talk_c::MSG_DS_JOURNEY2,
                                       dANpcSpGatekeeperA::l_msgLabel);
        }
        abortSave();
    } else if (manager->mError) {
        if (dAcNpc_c::isPlayerFromThisTown()) {
            dANpcSpGatekeeperA::l_travelData->cancelDeparture();
            nextStep();
        } else {
            dANpcSpGatekeeperA::l_travelData->cancelVisit();
            mCurrentMethod = &stepGoOutDsAppli_c::setErrorMsg;
        }
    }
}

// .text:0xB568 fn_66_B568 (0x44):
// dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::setErrorMsg() -
// "エラーメッセージをセット"
void dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::setErrorMsg() {
    getOwner()->setNextMessage(talk_c::MSG_DS_ERROR,
                               dANpcSpGatekeeperA::l_msgLabel);
    abortSave();
}

// .text:0xB5AC fn_66_B5AC (0x20):
// dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::abortSave() - mCurrentMethod =
// enableHomeAndReset (provisional)
void dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::abortSave() {
    mCurrentMethod = &stepGoOutDsAppli_c::enableHomeAndReset;
}

// .text:0xB5CC fn_66_B5CC (0xA4):
// dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::startEndDs() - "DS通信終了開始"
void dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::startEndDs() {
    dDsDl::g_manager.cancel();
    nextStep();
}

// .text:0xB670 fn_66_B670 (0x1B4):
// dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::endDs() - "DS通信終了"
void dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::endDs() {
    dDsDl::Manage_c *manager = &dDsDl::g_manager;
    if (manager->isDone()) {
        getOwner()->mpOwner->mDsDlActive = 0;
        dANpcSpGatekeeperA::l_travelData->clear();
        getOwner()->switchToGatekeeperSpeaker();
        dSvRuntime::releaseMessage(getOwner()->getController());
        getOwner()->changeMode(&talk_c::executeWait);
    }
}

// .text:0xB824 fn_66_B824 (0x280): dAcNpcSpGkA_c::talk_c::initComeInNetHst()
void dAcNpcSpGkA_c::talk_c::initComeInNetHst() {
    static const stepDemoComeInNetHst_c::Entry steps[] = {
        {"ロック開始まで待つ",
         (stepDemoComeInNetHst_c::Method)&stepDemoComeInNetHst_c::
             waitLockStart},
        {"初期化処理",
         (stepDemoComeInNetHst_c::Method)&stepDemoComeInNetHst_c::setupVisitor},
        {"「相手が持っている全追加データ」の受け取りが終わっているか？",
         (stepDemoComeInNetHst_c::Method)&stepDemoComeInNetHst_c::
             waitItemsReceived},
        {"追加データセーブ初期化",
         (stepDemoComeInNetHst_c::Method)&stepDemoComeInNetHst_c::
             initializeItemSave},
        {"追加データセーブ開始",
         (stepDemoComeInNetHst_c::Method)&stepDemoComeInNetHst_c::
             startItemSave},
        {"追加データセーブ終了",
         (stepDemoComeInNetHst_c::Method)&stepDemoComeInNetHst_c::waitItemSave},
        {"「全データ」の送信が終わっているか？",
         (stepDemoComeInNetHst_c::Method)&stepDemoComeInNetHst_c::waitJoinSent},
        {"全マシンネットデモが終了可能か？",
         (stepDemoComeInNetHst_c::Method)&stepDemoComeInNetHst_c::
             waitJoinDoneAll},
        {"デモ自体が完了しているか？",
         (stepDemoComeInNetHst_c::Method)&stepDemoComeInNetHst_c::
             waitDemoComplete},
        {"「お出かけデモの終了を送信したい」をセット",
         (stepDemoComeInNetHst_c::Method)&stepDemoComeInNetHst_c::
             sendFinishJoin},
        {"「お出かけデモの終了命令」を送信し終わったか？",
         (stepDemoComeInNetHst_c::Method)&stepDemoComeInNetHst_c::
             waitJoinFinished},
        {"アイコンとロック解除",
         (stepDemoComeInNetHst_c::Method)&stepDemoComeInNetHst_c::
             releaseIconAndLock},
        {"終了",
         (stepDemoComeInNetHst_c::Method)&stepDemoComeInNetHst_c::finish},
        {"セーブ失敗",
         (stepDemoComeInNetHst_c::Method)&stepDemoComeInNetHst_c::saveFailed},
    };
    mStepDemoComeInNetHst.initialize(
        this, "dAcNpcSpGkA_c::talk_c::stepDemoComeInNetHst_c", steps,
        ARRAY_SIZE(steps));
}

// .text:0xBAA4 fn_66_BAA4 (0x48): dAcNpcSpGkA_c::talk_c::executeComeInNetHst()
void dAcNpcSpGkA_c::talk_c::executeComeInNetHst() {
    mStepDemoComeInNetHst.execute();
}

// .text:0xBAEC fn_66_BAEC (0xA4):
// dAcNpcSpGkA_c::talk_c::stepDemoComeInNetHst_c::setupVisitor() - "初期化処理"
void dAcNpcSpGkA_c::talk_c::stepDemoComeInNetHst_c::setupVisitor() {
    dSvMgr_c::addVisitor(dSvMgr_c::mCurrentVisitor);
    nextStep();
}

// .text:0xBB90 fn_66_BB90 (0x248): dAcNpcSpGkA_c::talk_c::initComeInNetVst()
void dAcNpcSpGkA_c::talk_c::initComeInNetVst() {
    static const stepDemoComeInNetVst_c::Entry steps[] = {
        {"ロック開始まで待つ",
         (stepDemoComeInNetVst_c::Method)&stepDemoComeInNetVst_c::
             waitLockStart},
        {"「相手が持っている全追加データ」の受け取りが終わっているか？",
         (stepDemoComeInNetVst_c::Method)&stepDemoComeInNetVst_c::
             waitItemsReceived},
        {"追加データセーブ初期化",
         (stepDemoComeInNetVst_c::Method)&stepDemoComeInNetVst_c::
             initializeItemSave},
        {"追加データセーブ開始",
         (stepDemoComeInNetVst_c::Method)&stepDemoComeInNetVst_c::
             startItemSave},
        {"追加データセーブ終了",
         (stepDemoComeInNetVst_c::Method)&stepDemoComeInNetVst_c::waitItemSave},
        {"「全データ」の送信が終わっているか？",
         (stepDemoComeInNetVst_c::Method)&stepDemoComeInNetVst_c::waitJoinSent},
        {"デモ自体が完了しているか？",
         (stepDemoComeInNetVst_c::Method)&stepDemoComeInNetVst_c::
             waitDemoComplete},
        {"「お出かけデモの終了を送信したい」をセット",
         (stepDemoComeInNetVst_c::Method)&stepDemoComeInNetVst_c::
             sendFinishJoin},
        {"「ネットデモの終了命令」が来ていたら",
         (stepDemoComeInNetVst_c::Method)&stepDemoComeInNetVst_c::
             waitJoinAllDone},
        {"アイコンとロック解除",
         (stepDemoComeInNetVst_c::Method)&stepDemoComeInNetVst_c::
             releaseIconAndLock},
        {"終了",
         (stepDemoComeInNetVst_c::Method)&stepDemoComeInNetVst_c::finish},
        {"セーブ失敗",
         (stepDemoComeInNetVst_c::Method)&stepDemoComeInNetVst_c::saveFailed},
    };
    mStepDemoComeInNetVst.initialize(
        this, "dAcNpcSpGkA_c::talk_c::stepDemoComeInNetVst_c", steps,
        ARRAY_SIZE(steps));
}

// .text:0xBDD8 fn_66_BDD8 (0x48): dAcNpcSpGkA_c::talk_c::executeComeInNetVst()
void dAcNpcSpGkA_c::talk_c::executeComeInNetVst() {
    mStepDemoComeInNetVst.execute();
}

// .text:0xBE20 fn_66_BE20 (0x350): dAcNpcSpGkA_c::talk_c::initGoOutNetVst()
void dAcNpcSpGkA_c::talk_c::initGoOutNetVst() {
    static const stepDemoGoOutNetVst_c::Entry steps[] = {
        {"ロック開始まで待つ",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::waitLockStart},
        {"「相手が持っている全追加データ」の受け取りが終わっているか？",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::
             waitItemsReceived},
        {"追加データセーブ初期化",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::
             initializeItemSave},
        {"追加データセーブ開始",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::startItemSave},
        {"追加データセーブ終了",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::waitItemSave},
        {"「村データ・プレイヤーデータ」の受け取りが終わっているか？",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::
             waitJoinPlayersReady},
        {"ともだち情報で検索0",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::searchFriend0},
        {"ともだち情報で検索1",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::searchFriend1},
        {"ともだち情報で検索2",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::searchFriend2},
        {"ともだち情報で検索3",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::searchFriend3},
        {"ネットビジターセーブ初期化",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::
             initializeVisitorSave},
        {"ネットビジターセーブ開始",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::
             startVisitorSave},
        {"ネットビジターセーブ終了",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::
             waitVisitorSave},
        {"「全データ」の送信が終わっているか？ ",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::waitJoinSent},
        {"「新規参入時に設定されたコールバック」を呼び出す（参入側） ",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::
             callJoinCallback},
        {"デモ自体が完了しているか？",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::
             waitDemoComplete},
        {"「お出かけデモの終了を送信したい」をセット",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::sendFinishJoin},
        {"「ネットデモの終了命令」が来ていたら",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::
             waitJoinAllDone},
        {"アイコンとロック解除",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::
             releaseIconAndLock},
        {"終了", (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::finish},
        {"セーブ失敗",
         (stepDemoGoOutNetVst_c::Method)&stepDemoGoOutNetVst_c::saveFailed},
    };
    mStepDemoGoOutNetVst.initialize(
        this, "dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c", steps,
        ARRAY_SIZE(steps));
}

// .text:0xC170 fn_66_C170 (0x48): dAcNpcSpGkA_c::talk_c::executeGoOutNetVst()
void dAcNpcSpGkA_c::talk_c::executeGoOutNetVst() {
    mStepDemoGoOutNetVst.execute();
}

// .text:0xC1B8 fn_66_C1B8 (0xF0):
// dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c::searchFriend(int member)
BOOL dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c::searchFriend(int member) {
    if (member == dNet::getMyNo()) {
        return TRUE;
    }
    u32 no = (u8)member;
    BOOL peer = no != dNetMgr_c::sInstance->mInfo.mMyNo &&
                (dNetMgr_c::sInstance->mInfo.mMemberBits & (1 << no));
    if (peer) {
        u64 wiiNumber = dNet::getWiiNumber(member);
        if (wiiNumber != 0) {
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayer(member);
            int idx = dWifi::findUser(&player->mFriends.mUserData);
            if (idx >= 0 || idx == -1) {
                if (idx >= 0) {
                    dWifi::updateFriend(idx, &player->mPID,
                                        player->mFriends.mName, &wiiNumber);
                    return TRUE;
                }
                return TRUE;
            }
            return FALSE;
        }
        return TRUE;
    }
    return TRUE;
}

// .text:0xC2A8 fn_66_C2A8 (0xA8):
// dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c::searchFriend0() -
// "ともだち情報で検索0"
void dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c::searchFriend0() {
    if (searchFriend(0)) {
        nextStep();
    }
}

// .text:0xC350 fn_66_C350 (0xA8):
// dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c::searchFriend1() -
// "ともだち情報で検索1"
void dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c::searchFriend1() {
    if (searchFriend(1)) {
        nextStep();
    }
}

// .text:0xC3F8 fn_66_C3F8 (0xA8):
// dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c::searchFriend2() -
// "ともだち情報で検索2"
void dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c::searchFriend2() {
    if (searchFriend(2)) {
        nextStep();
    }
}

// .text:0xC4A0 fn_66_C4A0 (0xA8):
// dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c::searchFriend3() -
// "ともだち情報で検索3"
void dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c::searchFriend3() {
    if (searchFriend(3)) {
        nextStep();
    }
}

// .text:0xC548 fn_66_C548 (0xAC):
// dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c::waitJoinPlayersReady() -
// "「村データ・プレイヤーデータ」の受け取りが終わっているか？"
void dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c::waitJoinPlayersReady() {
    if (dNet::isJoinPlayersReady()) {
        dSaveData_c::get()->refreshTown();
        nextStep();
    }
}

// .text:0xC5F4 fn_66_C5F4 (0x9C):
// dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c::callJoinCallback() -
// "「新規参入時に設定されたコールバック」を呼び出す（参入側）"
void dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c::callJoinCallback() {
    dNet::clearVisitor();
    nextStep();
}

// .text:0xC690 fn_66_C690 (0x15C): dAcNpcSpGkA_c::endDemo() -
// changeMode(executeNone)
void dAcNpcSpGkA_c::endDemo() {
    changeMode(&dAcNpcSpGkA_c::executeNone);
}

// .text:0xC7EC fn_66_C7EC (0x15C): dAcNpcSpGkA_c::talk_c::endDemo() -
// changeMode(MODE_WAIT)
void dAcNpcSpGkA_c::talk_c::endDemo() {
    changeMode(&talk_c::executeWait);
}

// .text:0xC948 fn_66_C948 (0x15C): dAcNpcSpGkA_c::startReunionNetHst() -
// changeMode(executeReunionNetHst), unreferenced
void dAcNpcSpGkA_c::startReunionNetHst() {
    changeMode(&dAcNpcSpGkA_c::executeReunionNetHst);
}

// .text:0xCAA4 fn_66_CAA4 (0x15C): dAcNpcSpGkA_c::startReunionNetVst() -
// changeMode(executeReunionNetVst), unreferenced
void dAcNpcSpGkA_c::startReunionNetVst() {
    changeMode(&dAcNpcSpGkA_c::executeReunionNetVst);
}

// .text:0xCC00 fn_66_CC00 (0x4C): dAcNpcSpGkA_c::isModeNone() - , unreferenced
BOOL dAcNpcSpGkA_c::isModeNone() {
    return isCurrentMethod(&dAcNpcSpGkA_c::executeNone);
}

// .text:0xCC4C fn_66_CC4C (0x70): dAcNpcSpGkA_c::isAnyFriendInSession() -
// (provisional owner)
BOOL dAcNpcSpGkA_c::isAnyFriendInSession() {
    for (int i = 0; i < FRIEND_NUM; i++) {
        if (dWifi::isFriendValid(i) &&
            dNetMgr_c::sInstance->isFriendInSession(i)) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text:0xCCBC fn_66_CCBC (0x134): dAcNpcSpGkA_c::getName(dHmnName::Word_c
// *name, int len)
void dAcNpcSpGkA_c::getName(dHmnName::Word_c *name, int len) const {
    BOOL noName = isCurrentMethod(&dAcNpcSpGkA_c::executeComeInNet) ||
                  isCurrentMethod(&dAcNpcSpGkA_c::executeSeeOffNetHst) ||
                  isCurrentMethod(&dAcNpcSpGkA_c::executeSeeOffNetVst);
    if (noName) {
        dString::Word_c empty;
        name->copy(&empty, 0);
    } else {
        dAcNpcSp_c::getName(name, len);
    }
}

// .text:0xCDF0 fn_66_CDF0 (0x2D8): dAcNpcSpGkA_c::talk_c::setNetErrorMsg()
void dAcNpcSpGkA_c::talk_c::setNetErrorMsg() {
    switchToSystemSpeaker();
    int msg = getNetErrorMsg();
    setNextMessage(msg, sNetErrorMessageLabel);
    if (dANpcSpGatekeeperA::getNetErrorKind() == dNetDwc::ERROR_PERMANENT) {
        dSvRuntime::releaseMessage(mpController);
        changeMode(&talk_c::executeWait);
    } else {
        dMsg::Comp_c *controller = mpController;
        controller->mLock = 1;
        controller->mWaitIconSe = 1;
        controller->mWaitIcon = 1;
        changeMode(&talk_c::executeWiFiDisconnect);
    }
}

// ---------------------------------------------------------------------------
// .text 0xD0C8..0x1B83C: template instantiations and weak functions (not
// written yet). Owner of every address (instantiation order = reverse class
// completion; members in declaration order; the stepDemoBase_c helpers come in
// a second pass): 0x0D0C8 fn_66_D0C8   0xAC   stepDemoBase_c<talk_c,
// talk_c::stepDemoGoOutNetVst_c>::waitLockStart 0x0D174 fn_66_D174   0xB0
// stepDemoBase_c<talk_c, talk_c::stepDemoGoOutNetVst_c>::releaseIconAndLock
// 0x0D224 fn_66_D224   0x8    stepDemoBase_c<talk_c,
// talk_c::stepDemoGoOutNetVst_c>::finish 0x0D22C fn_66_D22C   0xA4
// stepDemoBase_c<talk_c, talk_c::stepDemoGoOutNetVst_c>::waitItemsReceived
// 0x0D2D0 fn_66_D2D0   0xA4   stepDemoBase_c<talk_c,
// talk_c::stepDemoGoOutNetVst_c>::waitJoinSent 0x0D374 fn_66_D374   0xA4
// stepDemoBase_c<talk_c, talk_c::stepDemoGoOutNetVst_c>::sendFinishJoin 0x0D418
// fn_66_D418   0xB0   stepDemoBase_c<talk_c,
// talk_c::stepDemoGoOutNetVst_c>::waitJoinAllDone 0x0D4C8 fn_66_D4C8   0xB4
// stepDemoBase_c<talk_c, talk_c::stepDemoGoOutNetVst_c>::waitDemoComplete
// 0x0D57C fn_66_D57C   0xAC   stepDemoBase_c<talk_c,
// talk_c::stepDemoComeInNetVst_c>::waitLockStart 0x0D628 fn_66_D628   0xB0
// stepDemoBase_c<talk_c, talk_c::stepDemoComeInNetVst_c>::releaseIconAndLock
// 0x0D6D8 fn_66_D6D8   0x8    stepDemoBase_c<talk_c,
// talk_c::stepDemoComeInNetVst_c>::finish 0x0D6E0 fn_66_D6E0   0xA4
// stepDemoBase_c<talk_c, talk_c::stepDemoComeInNetVst_c>::waitItemsReceived
// 0x0D784 fn_66_D784   0xA4   stepDemoBase_c<talk_c,
// talk_c::stepDemoComeInNetVst_c>::waitJoinSent 0x0D828 fn_66_D828   0xA4
// stepDemoBase_c<talk_c, talk_c::stepDemoComeInNetVst_c>::sendFinishJoin
// 0x0D8CC fn_66_D8CC   0xB0   stepDemoBase_c<talk_c,
// talk_c::stepDemoComeInNetVst_c>::waitJoinAllDone 0x0D97C fn_66_D97C   0xB4
// stepDemoBase_c<talk_c, talk_c::stepDemoComeInNetVst_c>::waitDemoComplete
// 0x0DA30 fn_66_DA30   0xAC   stepDemoBase_c<talk_c,
// talk_c::stepDemoComeInNetHst_c>::waitLockStart 0x0DADC fn_66_DADC   0xB0
// stepDemoBase_c<talk_c, talk_c::stepDemoComeInNetHst_c>::releaseIconAndLock
// 0x0DB8C fn_66_DB8C   0x8    stepDemoBase_c<talk_c,
// talk_c::stepDemoComeInNetHst_c>::finish 0x0DB94 fn_66_DB94   0xA4
// stepDemoBase_c<talk_c, talk_c::stepDemoComeInNetHst_c>::waitItemsReceived
// 0x0DC38 fn_66_DC38   0xA4   stepDemoBase_c<talk_c,
// talk_c::stepDemoComeInNetHst_c>::waitJoinSent 0x0DCDC fn_66_DCDC   0xA4
// stepDemoBase_c<talk_c, talk_c::stepDemoComeInNetHst_c>::waitJoinDoneAll
// 0x0DD80 fn_66_DD80   0xA4   stepDemoBase_c<talk_c,
// talk_c::stepDemoComeInNetHst_c>::sendFinishJoin 0x0DE24 fn_66_DE24   0xB0
// stepDemoBase_c<talk_c, talk_c::stepDemoComeInNetHst_c>::waitJoinFinished
// 0x0DED4 fn_66_DED4   0xB4   stepDemoBase_c<talk_c,
// talk_c::stepDemoComeInNetHst_c>::waitDemoComplete 0x0DF88 fn_66_DF88   0x78
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoBringBack_c>::startEnterMove (helper)
// 0x0E000 fn_66_E000   0x2C   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoBringBack_c>::isPlayerMoveDone (helper) 0x0E02C fn_66_E02C   0xAC
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoBringBack_c>::waitTalkEnd 0x0E0D8
// fn_66_E0D8   0x88   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetVst_c>::startExitMove (helper) 0x0E160 fn_66_E160   0x9C
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetVst_c>::initFaceSide 0x0E1FC
// fn_66_E1FC   0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetVst_c>::faceSide 0x0E2A0 fn_66_E2A0   0x9C
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetVst_c>::initSalute 0x0E33C
// fn_66_E33C   0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetVst_c>::salute 0x0E3E0 fn_66_E3E0   0xC0
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetVst_c>::waitFadeIn 0x0E4A0
// fn_66_E4A0   0xA8   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetVst_c>::keepBgmVolume 0x0E548 fn_66_E548   0xAC
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetVst_c>::waitLockStart 0x0E5F4
// fn_66_E5F4   0xB0   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetVst_c>::releaseIconAndLock 0x0E6A4 fn_66_E6A4   0xAC
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetVst_c>::startSeeOffMsg 0x0E750
// fn_66_E750   0xAC   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetVst_c>::waitTalkEnd 0x0E7FC fn_66_E7FC   0xAC
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetVst_c>::toPrevScene 0x0E8A8
// fn_66_E8A8   0x8    stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetVst_c>::finish 0x0E8B0 fn_66_E8B0   0x9C
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetVst_c>::sendReleaseRequest
// 0x0E94C fn_66_E94C   0x9C   dSvStep_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetVst_c>::notifySaveDone 0x0E9E8 fn_66_E9E8   0xA8
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetVst_c>::waitDisconnectReady
// 0x0EA90 fn_66_EA90   0x88   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetHst_c>::startExitMove (helper) 0x0EB18 fn_66_EB18   0x9C
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetHst_c>::initFaceSide 0x0EBB4
// fn_66_EBB4   0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetHst_c>::faceSide 0x0EC58 fn_66_EC58   0x9C
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetHst_c>::initSalute 0x0ECF4
// fn_66_ECF4   0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetHst_c>::salute 0x0ED98 fn_66_ED98   0xC0
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetHst_c>::waitFadeIn 0x0EE58
// fn_66_EE58   0xA8   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetHst_c>::keepBgmVolume 0x0EF00 fn_66_EF00   0xAC
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetHst_c>::waitLockStart 0x0EFAC
// fn_66_EFAC   0xB0   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetHst_c>::releaseIconAndLock 0x0F05C fn_66_F05C   0xAC
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetHst_c>::startSeeOffMsg 0x0F108
// fn_66_F108   0xAC   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetHst_c>::waitTalkEnd 0x0F1B4 fn_66_F1B4   0xAC
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetHst_c>::toPrevScene 0x0F260
// fn_66_F260   0x8    stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetHst_c>::finish 0x0F268 fn_66_F268   0x9C
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetHst_c>::sendReleaseRequest
// 0x0F304 fn_66_F304   0x9C   dSvStep_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetHst_c>::notifySaveDone 0x0F3A0 fn_66_F3A0   0xA8
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetHst_c>::waitDisconnectReady
// 0x0F448 fn_66_F448   0x78   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInDsAppli_c>::startEnterMove (helper) 0x0F4C0 fn_66_F4C0   0x2C
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoComeInDsAppli_c>::isPlayerMoveDone
// (helper) 0x0F4EC fn_66_F4EC   0x9C   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInDsAppli_c>::initFaceSide 0x0F588 fn_66_F588   0xA4
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoComeInDsAppli_c>::faceSide 0x0F62C
// fn_66_F62C   0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInDsAppli_c>::waitFaceSide 0x0F6D0 fn_66_F6D0   0x9C
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoComeInDsAppli_c>::initSalute 0x0F76C
// fn_66_F76C   0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInDsAppli_c>::salute 0x0F810 fn_66_F810   0xC0
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoComeInDsAppli_c>::waitFadeIn 0x0F8D0
// fn_66_F8D0   0xA8   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInDsAppli_c>::keepBgmVolume 0x0F978 fn_66_F978   0xAC
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoComeInDsAppli_c>::startWelcomeMsg
// 0x0FA24 fn_66_FA24   0xAC   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInDsAppli_c>::waitTalkEnd 0x0FAD0 fn_66_FAD0   0xA4
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoComeInDsAppli_c>::waitPlayerWalkTime
// 0x0FB74 fn_66_FB74   0x8    stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInDsAppli_c>::finish 0x0FB7C fn_66_FB7C   0x78
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoComeInNet_c>::startEnterMove (helper)
// 0x0FBF4 fn_66_FBF4   0x2C   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInNet_c>::isPlayerMoveDone (helper) 0x0FC20 fn_66_FC20   0x9C
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoComeInNet_c>::initFaceSide 0x0FCBC
// fn_66_FCBC   0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInNet_c>::faceSide 0x0FD60 fn_66_FD60   0xA4
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoComeInNet_c>::waitFaceSide 0x0FE04
// fn_66_FE04   0x9C   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInNet_c>::initSalute 0x0FEA0 fn_66_FEA0   0xA4
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoComeInNet_c>::salute 0x0FF44 fn_66_FF44
// 0xC0   stepDemoBase_c<dAcNpcSpGkA_c, stepDemoComeInNet_c>::waitFadeIn 0x10004
// fn_66_10004  0xA8   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInNet_c>::keepBgmVolume 0x100AC fn_66_100AC  0xAC
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoComeInNet_c>::startWelcomeMsg 0x10158
// fn_66_10158  0xAC   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInNet_c>::waitTalkEnd 0x10204 fn_66_10204  0xA4
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoComeInNet_c>::waitPlayerWalkTime
// 0x102A8 fn_66_102A8  0xAC   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInNet_c>::toPrevScene 0x10354 fn_66_10354  0x8
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoComeInNet_c>::finish 0x1035C
// fn_66_1035C  0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInNet_c>::demoDone 0x10400 fn_66_10400  0xAC
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoReunionNetVst_c>::waitLockStart 0x104AC
// fn_66_104AC  0xB0   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoReunionNetVst_c>::releaseIconAndLock 0x1055C fn_66_1055C  0xAC
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoReunionNetVst_c>::startWelcomeMsg
// 0x10608 fn_66_10608  0xAC   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoReunionNetVst_c>::waitTalkEnd 0x106B4 fn_66_106B4  0x8
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoReunionNetVst_c>::finish 0x106BC
// fn_66_106BC  0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoReunionNetVst_c>::waitItemsReceived 0x10760 fn_66_10760  0xA4
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoReunionNetVst_c>::waitJoinSent 0x10804
// fn_66_10804  0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoReunionNetVst_c>::sendFinishJoin 0x108A8 fn_66_108A8  0xB0
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoReunionNetVst_c>::waitJoinAllDone
// 0x10958 fn_66_10958  0xAC   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoReunionNetHst_c>::waitLockStart 0x10A04 fn_66_10A04  0xB0
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoReunionNetHst_c>::releaseIconAndLock
// 0x10AB4 fn_66_10AB4  0xAC   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoReunionNetHst_c>::startWelcomeMsg 0x10B60 fn_66_10B60  0xAC
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoReunionNetHst_c>::waitTalkEnd 0x10C0C
// fn_66_10C0C  0x8    stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoReunionNetHst_c>::finish 0x10C14 fn_66_10C14  0xA4
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoReunionNetHst_c>::waitItemsReceived
// 0x10CB8 fn_66_10CB8  0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoReunionNetHst_c>::waitJoinSent 0x10D5C fn_66_10D5C  0xA4
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoReunionNetHst_c>::waitJoinDoneAll
// 0x10E00 fn_66_10E00  0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoReunionNetHst_c>::sendFinishJoin 0x10EA4 fn_66_10EA4  0xB0
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoReunionNetHst_c>::waitJoinFinished
// 0x10F54 fn_66_10F54  0x88   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutDsAppli_c>::startExitMove (helper) 0x10FDC fn_66_10FDC  0xEC
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutDsAppli_c>::faceFront 0x110C8
// fn_66_110C8  0x9C   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutDsAppli_c>::initFaceSide 0x11164 fn_66_11164  0xA4
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutDsAppli_c>::faceSide 0x11208
// fn_66_11208  0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutDsAppli_c>::waitFaceSide 0x112AC fn_66_112AC  0x9C
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutDsAppli_c>::initSalute 0x11348
// fn_66_11348  0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutDsAppli_c>::salute 0x113EC fn_66_113EC  0xAC
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutDsAppli_c>::waitTalkEnd 0x11498
// fn_66_11498  0x9C   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutDsAppli_c>::closeChat 0x11534 fn_66_11534  0x9C
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutDsAppli_c>::openGate 0x115D0
// fn_66_115D0  0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutDsAppli_c>::waitGateOpen 0x11674 fn_66_11674  0xB0
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutDsAppli_c>::toTitleScene 0x11724
// fn_66_11724  0x8    stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutDsAppli_c>::finish 0x1172C fn_66_1172C  0xA0
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutDsAppli_c>::startPlayerMoveToGate
// 0x117CC fn_66_117CC  0xA8   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutDsAppli_c>::waitPlayerMoveToGate 0x11874 fn_66_11874  0xA4
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutDsAppli_c>::turnPlayerToGate
// 0x11918 fn_66_11918  0xA8   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutDsAppli_c>::waitPlayerTurnToGate 0x119C0 fn_66_119C0  0x88
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoRetireNetVst_c>::startExitMove (helper)
// 0x11A48 fn_66_11A48  0x64   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoRetireNetVst_c>::startGateMove (helper) 0x11AAC fn_66_11AAC  0x2C
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoRetireNetVst_c>::isPlayerMoveDone
// (helper) 0x11AD8 fn_66_11AD8  0xEC   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoRetireNetVst_c>::faceFront 0x11BC4 fn_66_11BC4  0x9C
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoRetireNetVst_c>::initFaceSide 0x11C60
// fn_66_11C60  0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoRetireNetVst_c>::faceSide 0x11D04 fn_66_11D04  0xA4
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoRetireNetVst_c>::waitFaceSide 0x11DA8
// fn_66_11DA8  0x9C   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoRetireNetVst_c>::initSalute 0x11E44 fn_66_11E44  0xA4
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoRetireNetVst_c>::salute 0x11EE8
// fn_66_11EE8  0xAC   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoRetireNetVst_c>::waitTalkEnd 0x11F94 fn_66_11F94  0x9C
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoRetireNetVst_c>::closeChat 0x12030
// fn_66_12030  0x8    stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoRetireNetVst_c>::finish 0x12038 fn_66_12038  0x88
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutNetVst_c>::startExitMove (helper)
// 0x120C0 fn_66_120C0  0xEC   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutNetVst_c>::faceFront 0x121AC fn_66_121AC  0x9C
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutNetVst_c>::initFaceSide 0x12248
// fn_66_12248  0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutNetVst_c>::faceSide 0x122EC fn_66_122EC  0xA4
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutNetVst_c>::waitFaceSide 0x12390
// fn_66_12390  0x9C   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutNetVst_c>::initSalute 0x1242C fn_66_1242C  0xA4
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutNetVst_c>::salute 0x124D0
// fn_66_124D0  0xAC   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutNetVst_c>::waitTalkEnd 0x1257C fn_66_1257C  0x9C
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutNetVst_c>::closeChat 0x12618
// fn_66_12618  0xD4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutNetVst_c>::waitTalkSuspend 0x126EC fn_66_126EC  0x9C
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutNetVst_c>::openGate 0x12788
// fn_66_12788  0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutNetVst_c>::waitGateOpen 0x1282C fn_66_1282C  0x8
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutNetVst_c>::finish 0x12834
// fn_66_12834  0xA0   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutNetVst_c>::startPlayerMoveToGate 0x128D4 fn_66_128D4  0xA8
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutNetVst_c>::waitPlayerMoveToGate
// 0x1297C fn_66_1297C  0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutNetVst_c>::turnPlayerToGate 0x12A20 fn_66_12A20  0xA8
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutNetVst_c>::waitPlayerTurnToGate
// 0x12AC8 fn_66_12AC8  0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutNetVst_c>::demoDone 0x12B6C fn_66_12B6C  0xAC
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoCloseGateNetHst_c>::waitTalkEnd 0x12C18
// fn_66_12C18  0xAC   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoOpenGateNetHst_c>::waitTalkEnd 0x12CC4 fn_66_12CC4  0x9C
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoOpenGateNetHst_c>::openGate 0x12D60
// fn_66_12D60  0xA4   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoOpenGateNetHst_c>::waitGateOpen 0x12E04 fn_66_12E04  0x80
// stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetVst_c>::initSaluteAnm (helper,
// 2nd pass) 0x12E84 fn_66_12E84  0x1D0  stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetVst_c>::updateSaluteAnm (helper, 2nd pass) 0x13054
// fn_66_13054  0xA0   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetVst_c>::initFaceSideAnm (helper, 2nd pass) 0x130F4
// fn_66_130F4  0xA0   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetVst_c>::updateFaceSideAnm (helper, 2nd pass) 0x13194
// fn_66_13194  0x80   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetHst_c>::initSaluteAnm (helper, 2nd pass) 0x13214 fn_66_13214
// 0x1D0  stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetHst_c>::updateSaluteAnm
// (helper, 2nd pass) 0x133E4 fn_66_133E4  0xA0   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetHst_c>::initFaceSideAnm (helper, 2nd pass) 0x13484
// fn_66_13484  0xA0   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetHst_c>::updateFaceSideAnm (helper, 2nd pass) 0x13524
// fn_66_13524  0x80   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInDsAppli_c>::initSaluteAnm (helper, 2nd pass) 0x135A4
// fn_66_135A4  0x1D0  stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInDsAppli_c>::updateSaluteAnm (helper, 2nd pass) 0x13774
// fn_66_13774  0xA0   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInDsAppli_c>::initFaceSideAnm (helper, 2nd pass) 0x13814
// fn_66_13814  0xA0   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInDsAppli_c>::updateFaceSideAnm (helper, 2nd pass) 0x138B4
// fn_66_138B4  0x70   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInDsAppli_c>::isFaceSideAnmDone (helper, 2nd pass) 0x13924
// fn_66_13924  0x2C   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInDsAppli_c>::isWalkTimeUp (helper, 2nd pass) 0x13950 fn_66_13950
// 0x80   stepDemoBase_c<dAcNpcSpGkA_c, stepDemoComeInNet_c>::initSaluteAnm
// (helper, 2nd pass) 0x139D0 fn_66_139D0  0x1D0  stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInNet_c>::updateSaluteAnm (helper, 2nd pass) 0x13BA0 fn_66_13BA0
// 0xA0   stepDemoBase_c<dAcNpcSpGkA_c, stepDemoComeInNet_c>::initFaceSideAnm
// (helper, 2nd pass) 0x13C40 fn_66_13C40  0xA0   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInNet_c>::updateFaceSideAnm (helper, 2nd pass) 0x13CE0
// fn_66_13CE0  0x70   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInNet_c>::isFaceSideAnmDone (helper, 2nd pass) 0x13D50
// fn_66_13D50  0x2C   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoComeInNet_c>::isWalkTimeUp (helper, 2nd pass) 0x13D7C fn_66_13D7C
// 0x80   stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutDsAppli_c>::initSaluteAnm
// (helper, 2nd pass) 0x13DFC fn_66_13DFC  0x1D0  stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutDsAppli_c>::updateSaluteAnm (helper, 2nd pass) 0x13FCC
// fn_66_13FCC  0xA0   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutDsAppli_c>::initFaceSideAnm (helper, 2nd pass) 0x1406C
// fn_66_1406C  0xA0   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutDsAppli_c>::updateFaceSideAnm (helper, 2nd pass) 0x1410C
// fn_66_1410C  0x70   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutDsAppli_c>::isFaceSideAnmDone (helper, 2nd pass) 0x1417C
// fn_66_1417C  0x64   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutDsAppli_c>::startGateMove (helper, 2nd pass) 0x141E0 fn_66_141E0
// 0x2C   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutDsAppli_c>::isPlayerMoveDone (helper, 2nd pass) 0x1420C
// fn_66_1420C  0x80   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoRetireNetVst_c>::initSaluteAnm (helper, 2nd pass) 0x1428C fn_66_1428C
// 0x1D0  stepDemoBase_c<dAcNpcSpGkA_c, stepDemoRetireNetVst_c>::updateSaluteAnm
// (helper, 2nd pass) 0x1445C fn_66_1445C  0xA0   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoRetireNetVst_c>::initFaceSideAnm (helper, 2nd pass) 0x144FC
// fn_66_144FC  0xA0   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoRetireNetVst_c>::updateFaceSideAnm (helper, 2nd pass) 0x1459C
// fn_66_1459C  0x70   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoRetireNetVst_c>::isFaceSideAnmDone (helper, 2nd pass) 0x1460C
// fn_66_1460C  0x80   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutNetVst_c>::initSaluteAnm (helper, 2nd pass) 0x1468C fn_66_1468C
// 0x1D0  stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutNetVst_c>::updateSaluteAnm
// (helper, 2nd pass) 0x1485C fn_66_1485C  0xA0   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutNetVst_c>::initFaceSideAnm (helper, 2nd pass) 0x148FC
// fn_66_148FC  0xA0   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutNetVst_c>::updateFaceSideAnm (helper, 2nd pass) 0x1499C
// fn_66_1499C  0x70   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutNetVst_c>::isFaceSideAnmDone (helper, 2nd pass) 0x14A0C
// fn_66_14A0C  0x64   stepDemoBase_c<dAcNpcSpGkA_c,
// stepDemoGoOutNetVst_c>::startGateMove (helper, 2nd pass) 0x14A70 fn_66_14A70
// 0x2C   stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutNetVst_c>::isPlayerMoveDone
// (helper, 2nd pass) 0x14A9C fn_66_14A9C  0x8    weak dAcNpc_c::getHoldItem
// (header inline copy) 0x14AA4 fn_66_14AA4  0x4    weak dAcNpc_c::vt84 0x14AA8
// fn_66_14AA8  0x4    weak dAcNpc_c::vt80 0x14AAC fn_66_14AAC  0x8    weak
// dAcNpc_c::getTool 0x14AB4 fn_66_14AB4  0x8    weak dAcNpc_c::getCloth 0x14ABC
// fn_66_14ABC  0x8    weak dAcNpc_c::getModel 0x14AC4 fn_66_14AC4  0x2C   weak
// dAcNpcSp_c::getWanderMinDist 0x14AF0 fn_66_14AF0  0x2C   weak
// dAcNpcSp_c::getWanderMaxDist 0x14B1C fn_66_14B1C  0x40
// dState::base_c<dAcNpcSpGkA_c>::~dtor 0x14B5C fn_66_14B5C  0x40
// dState::mode_c<dAcNpcSpGkA_c>::~dtor 0x14B9C fn_66_14B9C  0x40
// dState::base_c<dAcNpcSpGkA_c::stepDemoOpenGateNetHst_c>::~dtor 0x14BDC
// fn_66_14BDC  0x40
// dState::mode_c<dAcNpcSpGkA_c::stepDemoOpenGateNetHst_c>::~dtor 0x14C1C
// fn_66_14C1C  0x40   dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoOpenGateNetHst_c>::~dtor 0x14C5C fn_66_14C5C  0x40
// dState::base_c<dAcNpcSpGkA_c::stepDemoCloseGateNetHst_c>::~dtor 0x14C9C
// fn_66_14C9C  0x40
// dState::mode_c<dAcNpcSpGkA_c::stepDemoCloseGateNetHst_c>::~dtor 0x14CDC
// fn_66_14CDC  0x40   dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoCloseGateNetHst_c>::~dtor 0x14D1C fn_66_14D1C  0x40
// dState::base_c<dAcNpcSpGkA_c::stepDemoGoOutNetVst_c>::~dtor 0x14D5C
// fn_66_14D5C  0x40 dState::mode_c<dAcNpcSpGkA_c::stepDemoGoOutNetVst_c>::~dtor
// 0x14D9C fn_66_14D9C  0x40   dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoGoOutNetVst_c>::~dtor 0x14DDC fn_66_14DDC  0x40
// dState::base_c<dAcNpcSpGkA_c::stepDemoRetireNetVst_c>::~dtor 0x14E1C
// fn_66_14E1C  0x40
// dState::mode_c<dAcNpcSpGkA_c::stepDemoRetireNetVst_c>::~dtor 0x14E5C
// fn_66_14E5C  0x40   dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoRetireNetVst_c>::~dtor 0x14E9C fn_66_14E9C  0x40
// dState::base_c<dAcNpcSpGkA_c::stepDemoGoOutDsAppli_c>::~dtor 0x14EDC
// fn_66_14EDC  0x40
// dState::mode_c<dAcNpcSpGkA_c::stepDemoGoOutDsAppli_c>::~dtor 0x14F1C
// fn_66_14F1C  0x40   dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoGoOutDsAppli_c>::~dtor 0x14F5C fn_66_14F5C  0x40
// dState::base_c<dAcNpcSpGkA_c::stepDemoReunionNetHst_c>::~dtor 0x14F9C
// fn_66_14F9C  0x40
// dState::mode_c<dAcNpcSpGkA_c::stepDemoReunionNetHst_c>::~dtor 0x14FDC
// fn_66_14FDC  0x40   dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoReunionNetHst_c>::~dtor 0x1501C fn_66_1501C  0x40
// dState::base_c<dAcNpcSpGkA_c::stepDemoReunionNetVst_c>::~dtor 0x1505C
// fn_66_1505C  0x40
// dState::mode_c<dAcNpcSpGkA_c::stepDemoReunionNetVst_c>::~dtor 0x1509C
// fn_66_1509C  0x40   dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoReunionNetVst_c>::~dtor 0x150DC fn_66_150DC  0x40
// dState::base_c<dAcNpcSpGkA_c::stepDemoComeInNet_c>::~dtor 0x1511C fn_66_1511C
// 0x40   dState::mode_c<dAcNpcSpGkA_c::stepDemoComeInNet_c>::~dtor 0x1515C
// fn_66_1515C  0x40   dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoComeInNet_c>::~dtor 0x1519C fn_66_1519C  0x40
// dState::base_c<dAcNpcSpGkA_c::stepDemoComeInDsAppli_c>::~dtor 0x151DC
// fn_66_151DC  0x40
// dState::mode_c<dAcNpcSpGkA_c::stepDemoComeInDsAppli_c>::~dtor 0x1521C
// fn_66_1521C  0x40   dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoComeInDsAppli_c>::~dtor 0x1525C fn_66_1525C  0x40
// dState::base_c<dAcNpcSpGkA_c::stepDemoSeeOffNetHst_c>::~dtor 0x1529C
// fn_66_1529C  0x40
// dState::mode_c<dAcNpcSpGkA_c::stepDemoSeeOffNetHst_c>::~dtor 0x152DC
// fn_66_152DC  0x40   dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoSeeOffNetHst_c>::~dtor 0x1531C fn_66_1531C  0x40
// dState::base_c<dAcNpcSpGkA_c::stepDemoSeeOffNetVst_c>::~dtor 0x1535C
// fn_66_1535C  0x40
// dState::mode_c<dAcNpcSpGkA_c::stepDemoSeeOffNetVst_c>::~dtor 0x1539C
// fn_66_1539C  0x40   dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoSeeOffNetVst_c>::~dtor 0x153DC fn_66_153DC  0x40
// dState::base_c<dAcNpcSpGkA_c::stepDemoBringBack_c>::~dtor 0x1541C fn_66_1541C
// 0x40   dState::mode_c<dAcNpcSpGkA_c::stepDemoBringBack_c>::~dtor 0x1545C
// fn_66_1545C  0x40   dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoBringBack_c>::~dtor 0x1549C fn_66_1549C  0x40
// dState::step_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c>::~dtor 0x154DC fn_66_154DC 0x40
// dState::mode_c<dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c>::~dtor 0x1551C
// fn_66_1551C  0x40
// dState::base_c<dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c>::~dtor 0x1555C
// fn_66_1555C  0x40   dState::step_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepDemoComeInNetVst_c>::~dtor 0x1559C fn_66_1559C
// 0x40   dState::mode_c<dAcNpcSpGkA_c::talk_c::stepDemoComeInNetVst_c>::~dtor
// 0x155DC fn_66_155DC  0x40
// dState::base_c<dAcNpcSpGkA_c::talk_c::stepDemoComeInNetVst_c>::~dtor 0x1561C
// fn_66_1561C  0x40   dState::step_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepDemoComeInNetHst_c>::~dtor 0x1565C fn_66_1565C
// 0x40   dState::mode_c<dAcNpcSpGkA_c::talk_c::stepDemoComeInNetHst_c>::~dtor
// 0x1569C fn_66_1569C  0x40
// dState::base_c<dAcNpcSpGkA_c::talk_c::stepDemoComeInNetHst_c>::~dtor 0x156DC
// fn_66_156DC  0x40   dState::step_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c>::~dtor 0x1571C fn_66_1571C  0x40
// dState::mode_c<dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c>::~dtor 0x1575C
// fn_66_1575C  0x40
// dState::base_c<dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c>::~dtor 0x1579C
// fn_66_1579C  0x40   dState::step_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepDisconnectNetVst_c>::~dtor 0x157DC fn_66_157DC
// 0x40   dState::mode_c<dAcNpcSpGkA_c::talk_c::stepDisconnectNetVst_c>::~dtor
// 0x1581C fn_66_1581C  0x40
// dState::base_c<dAcNpcSpGkA_c::talk_c::stepDisconnectNetVst_c>::~dtor 0x1585C
// fn_66_1585C  0x40   dState::step_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c>::~dtor 0x1589C fn_66_1589C  0x40
// dState::mode_c<dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c>::~dtor 0x158DC
// fn_66_158DC  0x40
// dState::base_c<dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c>::~dtor 0x1591C
// fn_66_1591C  0x40   dState::step_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepWiFiConnect_c>::~dtor 0x1595C fn_66_1595C  0x40
// dState::mode_c<dAcNpcSpGkA_c::talk_c::stepWiFiConnect_c>::~dtor 0x1599C
// fn_66_1599C  0x40
// dState::base_c<dAcNpcSpGkA_c::talk_c::stepWiFiConnect_c>::~dtor 0x159DC
// fn_66_159DC  0x40   dState::step_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepSaveBeforeWiFiConnect_c>::~dtor 0x15A1C
// fn_66_15A1C  0x40
// dState::mode_c<dAcNpcSpGkA_c::talk_c::stepSaveBeforeWiFiConnect_c>::~dtor
// 0x15A5C fn_66_15A5C  0x40
// dState::base_c<dAcNpcSpGkA_c::talk_c::stepSaveBeforeWiFiConnect_c>::~dtor
// 0x15A9C fn_66_15A9C  0x40   dState::mode_c<dAcNpcSpGkA_c::talk_c>::~dtor
// 0x15ADC fn_66_15ADC  0x40   dState::base_c<dAcNpcSpGkA_c::talk_c>::~dtor
// 0x15B1C fn_66_15B1C  0x88   dState::step_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c>::getStateName 0x15BA4
// fn_66_15BA4  0x88   dState::step_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepDemoComeInNetVst_c>::getStateName 0x15C2C
// fn_66_15C2C  0x88   dState::step_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepDemoComeInNetHst_c>::getStateName 0x15CB4
// fn_66_15CB4  0x88   dState::step_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c>::getStateName 0x15D3C fn_66_15D3C
// 0x88   dState::step_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepDisconnectNetVst_c>::getStateName 0x15DC4
// fn_66_15DC4  0x88   dState::step_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c>::getStateName 0x15E4C fn_66_15E4C
// 0x88   dState::step_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepWiFiConnect_c>::getStateName 0x15ED4 fn_66_15ED4
// 0x88   dState::step_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepSaveBeforeWiFiConnect_c>::getStateName 0x15F5C
// fn_66_15F5C  0x88   dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoBringBack_c>::getStateName 0x15FE4 fn_66_15FE4  0x88
// dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoSeeOffNetVst_c>::getStateName 0x1606C fn_66_1606C 0x88
// dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoSeeOffNetHst_c>::getStateName 0x160F4 fn_66_160F4 0x88
// dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoComeInDsAppli_c>::getStateName 0x1617C fn_66_1617C
// 0x88   dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoComeInNet_c>::getStateName 0x16204 fn_66_16204  0x88
// dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoReunionNetVst_c>::getStateName 0x1628C fn_66_1628C
// 0x88   dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoReunionNetHst_c>::getStateName 0x16314 fn_66_16314
// 0x88   dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoGoOutDsAppli_c>::getStateName 0x1639C fn_66_1639C 0x88
// dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoRetireNetVst_c>::getStateName 0x16424 fn_66_16424 0x88
// dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoGoOutNetVst_c>::getStateName 0x164AC fn_66_164AC  0x88
// dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoCloseGateNetHst_c>::getStateName 0x16534 fn_66_16534
// 0x88   dState::step_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoOpenGateNetHst_c>::getStateName 0x165BC fn_66_165BC
// 0x88
// dState::mode_c<dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c>::getStateName
// 0x16644 fn_66_16644  0x88
// dState::mode_c<dAcNpcSpGkA_c::talk_c::stepDemoComeInNetVst_c>::getStateName
// 0x166CC fn_66_166CC  0x88
// dState::mode_c<dAcNpcSpGkA_c::talk_c::stepDemoComeInNetHst_c>::getStateName
// 0x16754 fn_66_16754  0x88
// dState::mode_c<dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c>::getStateName
// 0x167DC fn_66_167DC  0x88
// dState::mode_c<dAcNpcSpGkA_c::talk_c::stepDisconnectNetVst_c>::getStateName
// 0x16864 fn_66_16864  0x88
// dState::mode_c<dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c>::getStateName
// 0x168EC fn_66_168EC  0x88
// dState::mode_c<dAcNpcSpGkA_c::talk_c::stepWiFiConnect_c>::getStateName
// 0x16974 fn_66_16974  0x88
// dState::mode_c<dAcNpcSpGkA_c::talk_c::stepSaveBeforeWiFiConnect_c>::getStateName
// 0x169FC fn_66_169FC  0x88 dState::mode_c<dAcNpcSpGkA_c::talk_c>::getStateName
// 0x16A84 fn_66_16A84  0x88
// dState::mode_c<dAcNpcSpGkA_c::stepDemoBringBack_c>::getStateName 0x16B0C
// fn_66_16B0C  0x88
// dState::mode_c<dAcNpcSpGkA_c::stepDemoSeeOffNetVst_c>::getStateName 0x16B94
// fn_66_16B94  0x88
// dState::mode_c<dAcNpcSpGkA_c::stepDemoSeeOffNetHst_c>::getStateName 0x16C1C
// fn_66_16C1C  0x88
// dState::mode_c<dAcNpcSpGkA_c::stepDemoComeInDsAppli_c>::getStateName 0x16CA4
// fn_66_16CA4  0x88
// dState::mode_c<dAcNpcSpGkA_c::stepDemoComeInNet_c>::getStateName 0x16D2C
// fn_66_16D2C  0x88
// dState::mode_c<dAcNpcSpGkA_c::stepDemoReunionNetVst_c>::getStateName 0x16DB4
// fn_66_16DB4  0x88
// dState::mode_c<dAcNpcSpGkA_c::stepDemoReunionNetHst_c>::getStateName 0x16E3C
// fn_66_16E3C  0x88
// dState::mode_c<dAcNpcSpGkA_c::stepDemoGoOutDsAppli_c>::getStateName 0x16EC4
// fn_66_16EC4  0x88
// dState::mode_c<dAcNpcSpGkA_c::stepDemoRetireNetVst_c>::getStateName 0x16F4C
// fn_66_16F4C  0x88
// dState::mode_c<dAcNpcSpGkA_c::stepDemoGoOutNetVst_c>::getStateName 0x16FD4
// fn_66_16FD4  0x88
// dState::mode_c<dAcNpcSpGkA_c::stepDemoCloseGateNetHst_c>::getStateName
// 0x1705C fn_66_1705C  0x88
// dState::mode_c<dAcNpcSpGkA_c::stepDemoOpenGateNetHst_c>::getStateName 0x170E4
// fn_66_170E4  0x88   dState::mode_c<dAcNpcSpGkA_c>::getStateName 0x1716C
// fn_66_1716C  0x8
// dState::base_c<dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c>::getStateName
// 0x17174 fn_66_17174  0x8
// dState::base_c<dAcNpcSpGkA_c::talk_c::stepDemoComeInNetVst_c>::getStateName
// 0x1717C fn_66_1717C  0x8
// dState::base_c<dAcNpcSpGkA_c::talk_c::stepDemoComeInNetHst_c>::getStateName
// 0x17184 fn_66_17184  0x8
// dState::base_c<dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c>::getStateName
// 0x1718C fn_66_1718C  0x8
// dState::base_c<dAcNpcSpGkA_c::talk_c::stepDisconnectNetVst_c>::getStateName
// 0x17194 fn_66_17194  0x8
// dState::base_c<dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c>::getStateName
// 0x1719C fn_66_1719C  0x8
// dState::base_c<dAcNpcSpGkA_c::talk_c::stepWiFiConnect_c>::getStateName
// 0x171A4 fn_66_171A4  0x8
// dState::base_c<dAcNpcSpGkA_c::talk_c::stepSaveBeforeWiFiConnect_c>::getStateName
// 0x171AC fn_66_171AC  0x8 dState::base_c<dAcNpcSpGkA_c::talk_c>::getStateName
// 0x171B4 fn_66_171B4  0x8
// dState::base_c<dAcNpcSpGkA_c::stepDemoBringBack_c>::getStateName 0x171BC
// fn_66_171BC  0x8
// dState::base_c<dAcNpcSpGkA_c::stepDemoSeeOffNetVst_c>::getStateName 0x171C4
// fn_66_171C4  0x8
// dState::base_c<dAcNpcSpGkA_c::stepDemoSeeOffNetHst_c>::getStateName 0x171CC
// fn_66_171CC  0x8
// dState::base_c<dAcNpcSpGkA_c::stepDemoComeInDsAppli_c>::getStateName 0x171D4
// fn_66_171D4  0x8
// dState::base_c<dAcNpcSpGkA_c::stepDemoComeInNet_c>::getStateName 0x171DC
// fn_66_171DC  0x8
// dState::base_c<dAcNpcSpGkA_c::stepDemoReunionNetVst_c>::getStateName 0x171E4
// fn_66_171E4  0x8
// dState::base_c<dAcNpcSpGkA_c::stepDemoReunionNetHst_c>::getStateName 0x171EC
// fn_66_171EC  0x8
// dState::base_c<dAcNpcSpGkA_c::stepDemoGoOutDsAppli_c>::getStateName 0x171F4
// fn_66_171F4  0x8
// dState::base_c<dAcNpcSpGkA_c::stepDemoRetireNetVst_c>::getStateName 0x171FC
// fn_66_171FC  0x8
// dState::base_c<dAcNpcSpGkA_c::stepDemoGoOutNetVst_c>::getStateName 0x17204
// fn_66_17204  0x8
// dState::base_c<dAcNpcSpGkA_c::stepDemoCloseGateNetHst_c>::getStateName
// 0x1720C fn_66_1720C  0x8
// dState::base_c<dAcNpcSpGkA_c::stepDemoOpenGateNetHst_c>::getStateName 0x17214
// fn_66_17214  0x8    dState::base_c<dAcNpcSpGkA_c>::getStateName 0x1721C
// fn_66_1721C  0x40   dSvStep_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoOpenGateNetHst_c>::~dtor 0x1725C fn_66_1725C  0x40
// dSvStep_c<dAcNpcSpGkA_c, dAcNpcSpGkA_c::stepDemoCloseGateNetHst_c>::~dtor
// 0x1729C fn_66_1729C  0x40   dSvStep_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoGoOutNetVst_c>::~dtor 0x172DC fn_66_172DC  0x40
// dSvStep_c<dAcNpcSpGkA_c, dAcNpcSpGkA_c::stepDemoRetireNetVst_c>::~dtor
// 0x1731C fn_66_1731C  0x40   dSvStep_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoGoOutDsAppli_c>::~dtor 0x1735C fn_66_1735C  0x40
// dSvStep_c<dAcNpcSpGkA_c, dAcNpcSpGkA_c::stepDemoReunionNetHst_c>::~dtor
// 0x1739C fn_66_1739C  0x40   dSvStep_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoReunionNetVst_c>::~dtor 0x173DC fn_66_173DC  0x40
// dSvStep_c<dAcNpcSpGkA_c, dAcNpcSpGkA_c::stepDemoComeInNet_c>::~dtor 0x1741C
// fn_66_1741C  0x40   dSvStep_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoComeInDsAppli_c>::~dtor 0x1745C fn_66_1745C  0x40
// dSvStep_c<dAcNpcSpGkA_c, dAcNpcSpGkA_c::stepDemoSeeOffNetHst_c>::~dtor
// 0x1749C fn_66_1749C  0x40   dSvStep_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoSeeOffNetVst_c>::~dtor 0x174DC fn_66_174DC  0x40
// dSvStep_c<dAcNpcSpGkA_c, dAcNpcSpGkA_c::stepDemoBringBack_c>::~dtor 0x1751C
// fn_66_1751C  0x40   dSvStep_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c>::~dtor 0x1755C fn_66_1755C 0x40
// dSvStep_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepDemoComeInNetVst_c>::~dtor 0x1759C fn_66_1759C
// 0x40   dSvStep_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepDemoComeInNetHst_c>::~dtor 0x175DC fn_66_175DC
// 0x40   dSvStep_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c>::~dtor 0x1761C fn_66_1761C  0x40
// dSvStep_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c>::~dtor 0x1765C fn_66_1765C  0x40
// dSvStep_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepWiFiConnect_c>::~dtor 0x1769C fn_66_1769C  0x40
// dSvStep_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepSaveBeforeWiFiConnect_c>::~dtor 0x176DC
// fn_66_176DC  0x9C   dSvStep_c<talk_c,
// talk_c::stepDemoGoOutNetVst_c>::initializeVisitorSave 0x17778 fn_66_17778
// 0xA4   dSvStep_c<talk_c, talk_c::stepDemoGoOutNetVst_c>::startVisitorSave
// 0x1781C fn_66_1781C  0x4    dSvStep_c<talk_c,
// talk_c::stepDemoGoOutNetVst_c>::waitVisitorSave 0x17820 fn_66_17820  0x98
// dSvStep_c<talk_c, talk_c::stepDemoGoOutNetVst_c>::initializeItemSave 0x178B8
// fn_66_178B8  0xA4   dSvStep_c<talk_c,
// talk_c::stepDemoGoOutNetVst_c>::startItemSave 0x1795C fn_66_1795C  0x4
// dSvStep_c<talk_c, talk_c::stepDemoGoOutNetVst_c>::waitItemSave 0x17960
// fn_66_17960  0x4    dSvStep_c<talk_c,
// talk_c::stepDemoGoOutNetVst_c>::saveFailed 0x17964 fn_66_17964  0x98
// dSvStep_c<talk_c, talk_c::stepDemoComeInNetVst_c>::initializeItemSave 0x179FC
// fn_66_179FC  0xA4   dSvStep_c<talk_c,
// talk_c::stepDemoComeInNetVst_c>::startItemSave 0x17AA0 fn_66_17AA0  0x4
// dSvStep_c<talk_c, talk_c::stepDemoComeInNetVst_c>::waitItemSave 0x17AA4
// fn_66_17AA4  0x4    dSvStep_c<talk_c,
// talk_c::stepDemoComeInNetVst_c>::saveFailed 0x17AA8 fn_66_17AA8  0x98
// dSvStep_c<talk_c, talk_c::stepDemoComeInNetHst_c>::initializeItemSave 0x17B40
// fn_66_17B40  0xA4   dSvStep_c<talk_c,
// talk_c::stepDemoComeInNetHst_c>::startItemSave 0x17BE4 fn_66_17BE4  0x4
// dSvStep_c<talk_c, talk_c::stepDemoComeInNetHst_c>::waitItemSave 0x17BE8
// fn_66_17BE8  0x4    dSvStep_c<talk_c,
// talk_c::stepDemoComeInNetHst_c>::saveFailed 0x17BEC fn_66_17BEC  0xA8
// dSvStep_c<talk_c, talk_c::stepGoOutDsAppli_c>::initializeFullSave 0x17C94
// fn_66_17C94  0xA4   dSvStep_c<talk_c,
// talk_c::stepGoOutDsAppli_c>::startFullSave 0x17D38 fn_66_17D38  0x4
// dSvStep_c<talk_c, talk_c::stepGoOutDsAppli_c>::waitFullSave 0x17D3C
// fn_66_17D3C  0x11C  dSvStep_c<talk_c,
// talk_c::stepGoOutDsAppli_c>::initializeDownloadBox 0x17E58 fn_66_17E58  0x1DC
// dSvStep_c<talk_c, talk_c::stepGoOutDsAppli_c>::createDownloadBox 0x18034
// fn_66_18034  0xA0   dSvStep_c<talk_c,
// talk_c::stepGoOutDsAppli_c>::initializeTownSave 0x180D4 fn_66_180D4  0xA4
// dSvStep_c<talk_c, talk_c::stepGoOutDsAppli_c>::startTownSave 0x18178
// fn_66_18178  0x4    dSvStep_c<talk_c,
// talk_c::stepGoOutDsAppli_c>::waitTownSave 0x1817C fn_66_1817C  0xB0
// dSvStep_c<talk_c, talk_c::stepGoOutDsAppli_c>::disableHomeAndReset 0x1822C
// fn_66_1822C  0xB0   dSvStep_c<talk_c,
// talk_c::stepGoOutDsAppli_c>::enableHomeAndReset 0x182DC fn_66_182DC  0x4
// dSvStep_c<talk_c, talk_c::stepGoOutDsAppli_c>::saveFailed 0x182E0 fn_66_182E0
// 0xA0   dSvStep_c<talk_c, talk_c::stepConnectNetVst_c>::initializeNpcSave
// 0x18380 fn_66_18380  0xA4   dSvStep_c<talk_c,
// talk_c::stepConnectNetVst_c>::startNpcSave 0x18424 fn_66_18424  0x4
// dSvStep_c<talk_c, talk_c::stepConnectNetVst_c>::waitNpcSave 0x18428
// fn_66_18428  0x4    dSvStep_c<talk_c,
// talk_c::stepConnectNetVst_c>::saveFailed 0x1842C fn_66_1842C  0x9C
// dSvStep_c<talk_c, talk_c::stepWiFiConnect_c>::initializeHostSave 0x184C8
// fn_66_184C8  0xA4   dSvStep_c<talk_c,
// talk_c::stepWiFiConnect_c>::startHostSave 0x1856C fn_66_1856C  0x4
// dSvStep_c<talk_c, talk_c::stepWiFiConnect_c>::waitHostSave 0x18570
// fn_66_18570  0x4    dSvStep_c<talk_c, talk_c::stepWiFiConnect_c>::saveFailed
// 0x18574 fn_66_18574  0xA8   dSvStep_c<talk_c,
// talk_c::stepSaveBeforeWiFiConnect_c>::initializeFullSave 0x1861C fn_66_1861C
// 0xA4   dSvStep_c<talk_c, talk_c::stepSaveBeforeWiFiConnect_c>::startFullSave
// 0x186C0 fn_66_186C0  0x4    dSvStep_c<talk_c,
// talk_c::stepSaveBeforeWiFiConnect_c>::waitFullSave 0x186C4 fn_66_186C4  0x11C
// dSvStep_c<talk_c, talk_c::stepSaveBeforeWiFiConnect_c>::initializeDownloadBox
// 0x187E0 fn_66_187E0  0x1DC  dSvStep_c<talk_c,
// talk_c::stepSaveBeforeWiFiConnect_c>::createDownloadBox 0x189BC fn_66_189BC
// 0xB0   dSvStep_c<talk_c,
// talk_c::stepSaveBeforeWiFiConnect_c>::disableHomeAndReset 0x18A6C fn_66_18A6C
// 0xB0   dSvStep_c<talk_c,
// talk_c::stepSaveBeforeWiFiConnect_c>::enableHomeAndReset 0x18B1C fn_66_18B1C
// 0x4    dSvStep_c<talk_c, talk_c::stepSaveBeforeWiFiConnect_c>::saveFailed
// 0x18B20 fn_66_18B20  0x9C   dSvStep_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetVst_c>::initializeVisitorSave 0x18BBC fn_66_18BBC  0xA4
// dSvStep_c<dAcNpcSpGkA_c, stepDemoSeeOffNetVst_c>::startVisitorSave 0x18C60
// fn_66_18C60  0x4    dSvStep_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetVst_c>::waitVisitorSave 0x18C64 fn_66_18C64  0x4
// dSvStep_c<dAcNpcSpGkA_c, stepDemoSeeOffNetVst_c>::saveFailed 0x18C68
// fn_66_18C68  0xA8   dSvStep_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetHst_c>::initializeFullSave 0x18D10 fn_66_18D10  0xA4
// dSvStep_c<dAcNpcSpGkA_c, stepDemoSeeOffNetHst_c>::startFullSave 0x18DB4
// fn_66_18DB4  0x4    dSvStep_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetHst_c>::waitFullSave 0x18DB8 fn_66_18DB8  0x11C
// dSvStep_c<dAcNpcSpGkA_c, stepDemoSeeOffNetHst_c>::initializeDownloadBox
// 0x18ED4 fn_66_18ED4  0x1DC  dSvStep_c<dAcNpcSpGkA_c,
// stepDemoSeeOffNetHst_c>::createDownloadBox 0x190B0 fn_66_190B0  0x4
// dSvStep_c<dAcNpcSpGkA_c, stepDemoSeeOffNetHst_c>::saveFailed 0x190B4
// fn_66_190B4  0x98   dSvStep_c<dAcNpcSpGkA_c,
// stepDemoReunionNetVst_c>::initializeItemSave 0x1914C fn_66_1914C  0xA4
// dSvStep_c<dAcNpcSpGkA_c, stepDemoReunionNetVst_c>::startItemSave 0x191F0
// fn_66_191F0  0x4    dSvStep_c<dAcNpcSpGkA_c,
// stepDemoReunionNetVst_c>::waitItemSave 0x191F4 fn_66_191F4  0x4
// dSvStep_c<dAcNpcSpGkA_c, stepDemoReunionNetVst_c>::saveFailed 0x191F8
// fn_66_191F8  0x98   dSvStep_c<dAcNpcSpGkA_c,
// stepDemoReunionNetHst_c>::initializeItemSave 0x19290 fn_66_19290  0xA4
// dSvStep_c<dAcNpcSpGkA_c, stepDemoReunionNetHst_c>::startItemSave 0x19334
// fn_66_19334  0x4    dSvStep_c<dAcNpcSpGkA_c,
// stepDemoReunionNetHst_c>::waitItemSave 0x19338 fn_66_19338  0x4
// dSvStep_c<dAcNpcSpGkA_c, stepDemoReunionNetHst_c>::saveFailed 0x1933C
// fn_66_1933C  0x260  checkSaveResultImpl<talk_c::stepDemoGoOutNetVst_c>
// 0x1959C fn_66_1959C  0x260
// checkSaveResultImpl<talk_c::stepDemoComeInNetVst_c> 0x197FC fn_66_197FC 0x260
// checkSaveResultImpl<talk_c::stepDemoComeInNetHst_c> 0x19A5C fn_66_19A5C 0x260
// checkSaveResultImpl<talk_c::stepGoOutDsAppli_c> 0x19CBC fn_66_19CBC  0x260
// checkSaveResultImpl<talk_c::stepConnectNetVst_c> 0x19F1C fn_66_19F1C  0x260
// checkSaveResultImpl<talk_c::stepWiFiConnect_c> 0x1A17C fn_66_1A17C  0x260
// checkSaveResultImpl<talk_c::stepSaveBeforeWiFiConnect_c> 0x1A3DC fn_66_1A3DC
// 0x260  checkSaveResultImpl<stepDemoSeeOffNetVst_c> 0x1A63C fn_66_1A63C  0x260
// checkSaveResultImpl<stepDemoSeeOffNetHst_c> 0x1A89C fn_66_1A89C  0x260
// checkSaveResultImpl<stepDemoReunionNetVst_c> 0x1AAFC fn_66_1AAFC  0x260
// checkSaveResultImpl<stepDemoReunionNetHst_c> 0x1AD5C fn_66_1AD5C  0x40
// dAcNpcSpGkA_c::stepDemoBase_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoOpenGateNetHst_c>::~dtor 0x1AD9C fn_66_1AD9C  0x40
// dAcNpcSpGkA_c::stepDemoOpenGateNetHst_c::~dtor 0x1ADDC fn_66_1ADDC  0x40
// dAcNpcSpGkA_c::stepDemoBase_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoCloseGateNetHst_c>::~dtor 0x1AE1C fn_66_1AE1C  0x40
// dAcNpcSpGkA_c::stepDemoCloseGateNetHst_c::~dtor 0x1AE5C fn_66_1AE5C  0x40
// dAcNpcSpGkA_c::stepDemoBase_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoGoOutNetVst_c>::~dtor 0x1AE9C fn_66_1AE9C  0x40
// dAcNpcSpGkA_c::stepDemoGoOutNetVst_c::~dtor 0x1AEDC fn_66_1AEDC  0x40
// dAcNpcSpGkA_c::stepDemoBase_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoRetireNetVst_c>::~dtor 0x1AF1C fn_66_1AF1C  0x40
// dAcNpcSpGkA_c::stepDemoRetireNetVst_c::~dtor 0x1AF5C fn_66_1AF5C  0x40
// dAcNpcSpGkA_c::stepDemoBase_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoGoOutDsAppli_c>::~dtor 0x1AF9C fn_66_1AF9C  0x40
// dAcNpcSpGkA_c::stepDemoGoOutDsAppli_c::~dtor 0x1AFDC fn_66_1AFDC  0x40
// dAcNpcSpGkA_c::stepDemoBase_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoReunionNetHst_c>::~dtor 0x1B01C fn_66_1B01C  0x40
// dAcNpcSpGkA_c::stepDemoReunionNetHst_c::~dtor 0x1B05C fn_66_1B05C  0x40
// dAcNpcSpGkA_c::stepDemoBase_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoReunionNetVst_c>::~dtor 0x1B09C fn_66_1B09C  0x40
// dAcNpcSpGkA_c::stepDemoReunionNetVst_c::~dtor 0x1B0DC fn_66_1B0DC  0x40
// dAcNpcSpGkA_c::stepDemoBase_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoComeInNet_c>::~dtor 0x1B11C fn_66_1B11C  0x40
// dAcNpcSpGkA_c::stepDemoComeInNet_c::~dtor 0x1B15C fn_66_1B15C  0x40
// dAcNpcSpGkA_c::stepDemoBase_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoComeInDsAppli_c>::~dtor 0x1B19C fn_66_1B19C  0x40
// dAcNpcSpGkA_c::stepDemoComeInDsAppli_c::~dtor 0x1B1DC fn_66_1B1DC  0x40
// dAcNpcSpGkA_c::stepDemoBase_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoSeeOffNetHst_c>::~dtor 0x1B21C fn_66_1B21C  0x40
// dAcNpcSpGkA_c::stepDemoSeeOffNetHst_c::~dtor 0x1B25C fn_66_1B25C  0x40
// dAcNpcSpGkA_c::stepDemoBase_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoSeeOffNetVst_c>::~dtor 0x1B29C fn_66_1B29C  0x40
// dAcNpcSpGkA_c::stepDemoSeeOffNetVst_c::~dtor 0x1B2DC fn_66_1B2DC  0x40
// dAcNpcSpGkA_c::stepDemoBase_c<dAcNpcSpGkA_c,
// dAcNpcSpGkA_c::stepDemoBringBack_c>::~dtor 0x1B31C fn_66_1B31C  0x40
// dAcNpcSpGkA_c::stepDemoBringBack_c::~dtor 0x1B35C fn_66_1B35C  0x58
// talk_c::~talk_c 0x1B3B4 fn_66_1B3B4  0x40
// dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c::~dtor 0x1B3F4 fn_66_1B3F4  0x40
// dAcNpcSpGkA_c::stepDemoBase_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepDemoGoOutNetVst_c>::~dtor 0x1B434 fn_66_1B434 0x40
// dAcNpcSpGkA_c::talk_c::stepDemoComeInNetVst_c::~dtor 0x1B474 fn_66_1B474 0x40
// dAcNpcSpGkA_c::stepDemoBase_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepDemoComeInNetVst_c>::~dtor 0x1B4B4 fn_66_1B4B4
// 0x40   dAcNpcSpGkA_c::talk_c::stepDemoComeInNetHst_c::~dtor 0x1B4F4
// fn_66_1B4F4  0x40   dAcNpcSpGkA_c::stepDemoBase_c<dAcNpcSpGkA_c::talk_c,
// dAcNpcSpGkA_c::talk_c::stepDemoComeInNetHst_c>::~dtor 0x1B534 fn_66_1B534
// 0x40   dAcNpcSpGkA_c::talk_c::stepGoOutDsAppli_c::~dtor 0x1B574 fn_66_1B574
// 0x40   dAcNpcSpGkA_c::talk_c::stepDisconnectNetVst_c::~dtor 0x1B5B4
// fn_66_1B5B4  0x40   dAcNpcSpGkA_c::talk_c::stepConnectNetVst_c::~dtor 0x1B5F4
// fn_66_1B5F4  0x40   dAcNpcSpGkA_c::talk_c::stepWiFiConnect_c::~dtor 0x1B634
// fn_66_1B634  0x40   dAcNpcSpGkA_c::talk_c::stepSaveBeforeWiFiConnect_c::~dtor
// 0x1B674 fn_66_1B674  0x138  dAcNpcSpGkA_c::~dAcNpcSpGkA_c
// 0x1B7AC fn_66_1B7AC  0x40   dMsgRcptExp_c<dAcNpcSpGkA_c::talk_c>::~dtor
// 0x1B7EC fn_66_1B7EC  0x8    weak fBase_c::createHeap
// 0x1B7F4 fn_66_1B7F4  0x4    weak fBase_c::deleteReady
// 0x1B7F8 fn_66_1B7F8  0x8    weak dMsg::Rcpt_c::isSpeakerItchy
// 0x1B800 fn_66_1B800  0x8    weak dMsg::Rcpt_c::getListenerAnimal
// 0x1B808 fn_66_1B808  0x8    weak dMsg::Rcpt_c::getSpeakerAnimal
// 0x1B810 fn_66_1B810  0x1C   sLib::calcTimer<u32> (weak)
// 0x1B82C fn_66_1B82C  0x8    thunk @236@talk_c::~talk_c
// 0x1B834 fn_66_1B834  0x8    thunk @7292@dAcNpcSpGkA_c::~dAcNpcSpGkA_c

// ---- stepDemoBase_c: the common steps of the gate demos (instantiated per
// step class at the end of the unit) ----

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::initSaluteAnm() {
    dItem::Item key((u16)SP_NPC_GATEKEEPER_A);
    dAcNpc_c *gk = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(key));
    mTimer = cM::rndInt(15) + 15;
    gk->mAction.requestAnm(dAcNpcSpGkA_c::PRIORITY_DEMO, HMN_ANM_MR_PLC1, 0.0f,
                           cNpcMorphFrames, m3d::PLAYMODE_INHERIT, 0);
}

template <class Owner, class Step>
BOOL dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::updateSaluteAnm() {
    dItem::Item keyB((u16)SP_NPC_GATEKEEPER_B);
    dAcNpc_c *gkB = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(keyB));
    dItem::Item keyA((u16)SP_NPC_GATEKEEPER_A);
    dAcNpc_c *gkA = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(keyA));
    if (sLib::calcTimer(&mTimer) == 0 &&
        gkB->getModel()->mAnm.mAnmId != HMN_ANM_MR_PLC1 &&
        gkB->getModel()->mAnm.mAnmId != HMN_ANM_MR_PLC2) {
        gkB->mAction.requestAnm(dAcNpcSpGkA_c::PRIORITY_DEMO, HMN_ANM_MR_PLC1,
                                0.0f, cNpcMorphFrames, m3d::PLAYMODE_INHERIT,
                                0);
    }
    if (gkA->getModel()->mAnm.mAnmId == HMN_ANM_MR_PLC1 &&
        gkA->mAction.mCanChange) {
        gkA->mAction.requestAnm(dAcNpcSpGkA_c::PRIORITY_DEMO, HMN_ANM_MR_PLC2,
                                0.0f, cNpcMorphFrames, m3d::PLAYMODE_INHERIT,
                                0);
    }
    if (gkB->getModel()->mAnm.mAnmId == HMN_ANM_MR_PLC1 &&
        gkB->mAction.mCanChange) {
        gkB->mAction.requestAnm(dAcNpcSpGkA_c::PRIORITY_DEMO, HMN_ANM_MR_PLC2,
                                0.0f, cNpcMorphFrames, m3d::PLAYMODE_INHERIT,
                                0);
    }
    if (gkA->getModel()->mAnm.mAnmId == HMN_ANM_MR_PLC2 &&
        gkB->getModel()->mAnm.mAnmId == HMN_ANM_MR_PLC2) {
        return TRUE;
    }
    return FALSE;
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::initFaceSideAnm() {
    dItem::Item key((u16)SP_NPC_GATEKEEPER_A);
    dAcNpc_c *gk = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(key));
    gk->mAction.requestTurn(dAcNpcSpGkA_c::PRIORITY_DEMO,
                            mAng(-DEG_TO_ANGLE(90)), l_turnSpeed, 0);
    gk->mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
    mTimer = cM::rndInt(15) + 15;
}

template <class Owner, class Step>
BOOL dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::updateFaceSideAnm() {
    if (sLib::calcTimer(&mTimer) != 0) {
        return FALSE;
    }
    dItem::Item key((u16)SP_NPC_GATEKEEPER_B);
    dAcNpc_c *gk = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(key));
    gk->mAction.requestTurn(dAcNpcSpGkA_c::PRIORITY_DEMO,
                            mAng(DEG_TO_ANGLE(90)), l_turnSpeed, 0);
    gk->mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
    return TRUE;
}

template <class Owner, class Step>
BOOL dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::isFaceSideAnmDone() {
    dItem::Item keyA((u16)SP_NPC_GATEKEEPER_A);
    dAcNpc_c *gkA = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(keyA));
    dItem::Item keyB((u16)SP_NPC_GATEKEEPER_B);
    dAcNpc_c *gkB = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(keyB));
    if (gkA->mAction.mCanChange && gkB->mAction.mCanChange) {
        return TRUE;
    }
    return FALSE;
}

template <class Owner, class Step>
BOOL dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::isWalkTimeUp() {
    return sLib::calcTimer(&mWalkTimer) == 0;
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::startExitMove(int member) {
    dCamera_c::sInstance->startGateCloseUp();
    dPlayerMgr::setScriptedMove(TRUE, member);
    mVec3_c pos = dANpcSpGatekeeperA::l_exitPos;
    dPlayerMgr::moveToEscorted(&pos, member, 2.5f);
    mWalkTimer = 60;
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::startEnterMove(int member) {
    dPlayerMgr::setScriptedMove(TRUE, member);
    mVec3_c pos = dANpcSpGatekeeperA::l_enterPos;
    dPlayerMgr::moveTo(&pos, member, dPlayerMgr::sDefaultMoveSpeed);
    mWalkTimer = 60;
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::startGateMove(int member) {
    dPlayerMgr::setScriptedMove(TRUE, member);
    mVec3_c pos = dANpcSpGatekeeperA::l_gatePos;
    dPlayerMgr::moveTo(&pos, member, dPlayerMgr::sDefaultMoveSpeed);
}

template <class Owner, class Step>
BOOL dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::isPlayerMoveDone(int member) {
    return !dPlayerMgr::isMovingTo(member);
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::faceFront() {
    this->mpOwner->mTalk.setKeepCamera(TRUE);
    dCamera_c::sInstance->startGateView();
    this->mpOwner->setDemoFlag8();
    this->mpOwner->mAction.requestTurn(dAcNpcSpGkA_c::PRIORITY_DEMO, mAng(0),
                                       l_turnSpeed, 0);
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::initFaceSide() {
    initFaceSideAnm();
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::faceSide() {
    if (updateFaceSideAnm()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::waitFaceSide() {
    if (isFaceSideAnmDone()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::initSalute() {
    initSaluteAnm();
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::salute() {
    if (updateSaluteAnm()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::waitFadeIn() {
    if (mFader_c::isStatus(mFaderBase_c::HIDDEN)) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::keepBgmVolume() {
    dMsg::Comp_c::mInstance->mBgmKeep = TRUE;
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::waitLockStart() {
    if (dMsg::Comp_c::mInstance->mLock) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::releaseIconAndLock() {
    dMsg::Comp_c *comp = dMsg::Comp_c::mInstance;
    comp->mLock = FALSE;
    comp->mWaitIconSe = 0;
    comp->mWaitIcon = FALSE;
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::startSeeOffMsg() {
    this->mpOwner->mTalk.requestTalk(true, TRUE);
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::startWelcomeMsg() {
    this->mpOwner->mTalk.requestTalk(true, TRUE);
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::waitTalkEnd() {
    if (!this->mpOwner->mTalk.isBusy()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::closeChat() {
    dObjChat::expireAll();
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::waitTalkSuspend() {
    if (dMsg::Comp_c::mInstance->isState(&dMsg::Comp_c::executeSuspend)) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::openGate() {
    dARmobjCheckp::openGate();
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::waitGateOpen() {
    if (dARmobjCheckp::isGateOpen()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::waitPlayerWalkTime() {
    if (isWalkTimeUp()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::toPrevScene() {
    dANpcSpGatekeeperA::getSceneChange()->requestExit(1, 0, 0);
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::toTitleScene() {
    dSaveData_c::get();
    dSvRuntime::fn_8010DED0();
    dScene::requestSceneChange(dScene::SCENE_DM_TITLE, 0, 2);
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::finish() {
    this->mpOwner->endDemo();
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::sendReleaseRequest() {
    dNet::releaseRequest();
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::notifySaveDone() {
    dNet::setDisconnectReady();
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::waitDisconnectReady() {
    if (dNet::isDisconnectReady()) {
        dNet::sendDisconnectGo();
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::waitItemsReceived() {
    if (dNet::isItemsReceived()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::waitJoinSent() {
    if (dNet::isJoinTransferDone()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::waitJoinDoneAll() {
    if (dNet::isJoinDoneAll()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::sendFinishJoin() {
    if (dNet::finishJoin()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::waitJoinAllDone() {
    if (dNet::getJoinAllDone()) {
        dNet::resetJoinDone();
        dNet::clearRequestMember();
        dNet::releaseRequest();
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::waitJoinFinished() {
    if (dNet::isJoinFinished()) {
        dNet::resetJoinDone();
        dNet::clearRequestMember();
        dNet::releaseRequest();
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::waitDemoComplete() {
    dAcNpcSpGkA_c *gk = this->mpOwner->getGk();
    if (gk->mDemoDone) {
        gk->mDemoDone = FALSE;
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::startPlayerMoveToGate() {
    startGateMove(0);
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::waitPlayerMoveToGate() {
    if (isPlayerMoveDone(0)) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::turnPlayerToGate() {
    dPlayerMgr::turnTo(-DEG_TO_ANGLE(180), 0);
    this->nextStep();
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::waitPlayerTurnToGate() {
    if (!dPlayerMgr::isTurningTo(0)) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>::demoDone() {
    this->mpOwner->getGk()->mDemoDone = TRUE;
    this->nextStep();
}
