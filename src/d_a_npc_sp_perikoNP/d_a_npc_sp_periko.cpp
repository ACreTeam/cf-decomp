#include <game/game/d_a_npc_sp_periko.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_def.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_building.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_select.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <game/game/d_wifi.hpp>
#include <game/sLib/s_lib.hpp>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <lib/revolution/SC/scapi.h>

// Pelly and Phyllis at the town hall. Functions are in target .text order.

namespace dANpcSpPeriko {
// The two windows (WINDOW_TOWN_HALL, WINDOW_POST_OFFICE): where she stands.
static const nw4r::math::_VEC3 l_windowPos[dAcNpcSpPerikoBase_c::WINDOW_NUM] = {
    {240.0f, 0.0f, 304.0f}, {304.0f, 0.0f, 304.0f}};
// initMove: the turn mode of the run to each window.
static const int l_moveTurnMode[dAcNpcSpPerikoBase_c::WINDOW_NUM] = {2, 1};
// The interaction radius (preCreate folds it into a literal; the external
// linkage keeps the unreferenced object in .rodata).
extern const f32 l_interactRadius = 80.0f;

// initMove / isMoveDone: where she stops, a little to the outside of the
// window, and how close counts as arrived.
static const f32 l_arriveDist = 2.0f * l_8074FDEC;
static const nw4r::math::_VEC3 l_moveTarget[dAcNpcSpPerikoBase_c::WINDOW_NUM] =
    {
        {l_windowPos[0].x - l_arriveDist, l_windowPos[0].y, l_windowPos[0].z},
        {l_windowPos[1].x + l_arriveDist, l_windowPos[1].y, l_windowPos[1].z},
};
// getWindowAt: where the player stands in front of each window.
static const nw4r::math::_VEC3 l_frontPos[dAcNpcSpPerikoBase_c::WINDOW_NUM] = {
    {l_windowPos[0].x, l_windowPos[0].y, l_windowPos[0].z + 64.0f},
    {l_windowPos[1].x, l_windowPos[1].y, l_windowPos[1].z + 64.0f},
};

// The message labels: [talk_c::LABEL_*][mType].
static const char
    *l_msgLabels[dAcNpcSpPerikoBase_c::talk_c::LABEL_NUM]
                [dAcNpcSpPerikoBase_c::TYPE_NUM] = {
                    {"npc_SP/NPC_periko_0", "npc_SP/NPC_periko_0"},
                    {"npc_SP/NPC_periko", "npc_SP/NPC_perimi"},
                    {"npc_SP/NPC_tousho", "npc_SP/NPC_tousho"},
                    {dSvMgr_c::mMessageLabel, dSvMgr_c::mMessageLabel},
                    {sNetErrorMessageLabel, sNetErrorMessageLabel},
                    {"npc_SP/NPC_drama4", "npc_SP/NPC_drama4"},
};

// .text 0x5C, 0x1EC
CUSTOM_BASE_PROFILE(NPC_SP_PERIKO, dAcNpcSpPeriko_c, fProf::A_NPC_SP_PERIKO_05A,
                    fProf::DRAW_ORDER::A_NPC_SP_PERIKO_05A);
CUSTOM_BASE_PROFILE(NPC_SP_PERIMI, dAcNpcSpPerimi_c, fProf::A_NPC_SP_PERIKO_05B,
                    fProf::DRAW_ORDER::A_NPC_SP_PERIKO_05B);
} // namespace dANpcSpPeriko

// .text 0x37C
int dAcNpcSpPerikoBase_c::preCreate() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (!player->isFlag0(PRIVATE_FLAG0_NEWCOMER_DONE)) {
        mType = TYPE_PELLY;
        mWindow = WINDOW_TOWN_HALL;
    } else if (!isMultiPlay() && !mTypeChosen) {
        dTime_c now = *dTime_c::getCurrent();
        if (now.hour >= PELLY_START_HOUR && now.hour < PHYLLIS_START_HOUR) {
            mType = TYPE_PELLY;
            mParam = SP_NPC_POST_GIRL;
        } else {
            mType = TYPE_PHYLLIS;
            mParam = SP_NPC_POST_GIRL2;
        }
        mWindow = WINDOW_TOWN_HALL;
        mTypeChosen = 1;
    }
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    if (player->isFlag0(PRIVATE_FLAG0_NEWCOMER_DONE) && isMultiPlay() &&
        !mTypeChosen) {
        dItem::Item key((u16)SP_NPC_POST_GIRL);
        if (mNpcItem.isSame(key)) {
            mType = TYPE_PELLY;
            mWindow = WINDOW_POST_OFFICE;
            mVec3_c pos(dANpcSpPeriko::l_windowPos[0]);
            mAng3_c angle(0, 0, 0);
            dActor_c::construct(fProf::A_NPC_SP_PERIKO_05B, this,
                                SP_NPC_POST_GIRL2, &pos, &angle);
        } else {
            mType = TYPE_PHYLLIS;
            mWindow = WINDOW_TOWN_HALL;
        }
        mTypeChosen = 1;
    }
    mPos = mVec3_c(dANpcSpPeriko::l_windowPos[mWindow]);
    setInteractionRadius(dANpcSpPeriko::l_interactRadius);
    setRecept(&mWindowTalk);
    return 1;
}

// .text 0x5B8
int dAcNpcSpPerikoBase_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    if (isPelly() && isPlayerFromThisTown() && !isMultiPlay() &&
        !dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
            PRIVATE_FLAG0_DRAMA_PELLY_HEARD) &&
        dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getGracieStep() == 2) {
        mDramaDue = 1;
    } else {
        mDramaDue = 0;
    }
    static const dState::mode_c<dAcNpcSpPerikoBase_c>::Entry modes[] = {
        {"待機", &dAcNpcSpPerikoBase_c::initWait,
         &dAcNpcSpPerikoBase_c::executeWait},
        {"移動", &dAcNpcSpPerikoBase_c::initMove,
         &dAcNpcSpPerikoBase_c::executeMove},
        {"デモ待機", &dAcNpcSpPerikoBase_c::initDemoWait,
         &dAcNpcSpPerikoBase_c::executeDemoWait},
        {"会話", &dAcNpcSpPerikoBase_c::initTalk,
         &dAcNpcSpPerikoBase_c::executeTalk},
        {"初期角度に復帰", &dAcNpcSpPerikoBase_c::initTurnBack,
         &dAcNpcSpPerikoBase_c::executeTurnBack},
        {"三人会話パートナーの振る舞い", &dAcNpcSpPerikoBase_c::initPartner,
         &dAcNpcSpPerikoBase_c::executePartner},
        {"通信：リモート", &dAcNpcSpPerikoBase_c::initRemote,
         &dAcNpcSpPerikoBase_c::executeRemote},
        {"通信：リモート会話", &dAcNpcSpPerikoBase_c::initRemoteTalk,
         &dAcNpcSpPerikoBase_c::executeRemoteTalk},
        {"拍手待ち", &dAcNpcSpPerikoBase_c::initWaitClap,
         &dAcNpcSpPerikoBase_c::executeWaitClap},
        {"拍手", &dAcNpcSpPerikoBase_c::initClap,
         &dAcNpcSpPerikoBase_c::executeClap},
    };
    dState::mode_c<dAcNpcSpPerikoBase_c>::initialize("dAcNpcSpPerikoBase_c",
                                                     modes, ARRAY_SIZE(modes));
    if (isLocalOwner()) {
        changeMode(&dAcNpcSpPerikoBase_c::executeWait);
    } else {
        changeMode(&dAcNpcSpPerikoBase_c::executeRemote);
    }
    if (mType == TYPE_PHYLLIS) {
        f32 frame = getModel()->mAnm.mFrameMax;
        getModel()->mAnm.setFrame(0.3f * frame);
    }
    if (isPelly()) {
        mLookRange = 144.0f;
        mLookAt.mFov = labs(0x2BFF);
        mLookAt.mYaw.mMax = 0x2BFF;
    } else {
        mLookRange = 112.0f;
        mLookAt.mFov = labs(0x27FF);
        mLookAt.mYaw.mMax = 0x27FF;
    }
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      mLookRange);
    return 1;
}

// .text 0xA0C
int dAcNpcSpPerikoBase_c::doDelete() {
    if (dWifi::getThreadResult() == dWifi::THREAD_RESULT_BUSY) {
        return 0;
    }
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0xA5C
int dAcNpcSpPerikoBase_c::vtB0() {
    dState::base_c<dAcNpcSpPerikoBase_c>::execute();
    return 1;
}

// .text 0xAB0
bool dAcNpcSpPerikoBase_c::canInteract(dDemoActor_c *actor) {
    mVec3_c pos(actor->getPos());
    if (!dAcNpcSp_c::canInteract(actor)) {
        return false;
    }
    if (mWindow != getWindowAt(&pos)) {
        return false;
    }
    if (!isCurrentMethod(&dAcNpcSpPerikoBase_c::executeWait) &&
        !isCurrentMethod(&dAcNpcSpPerikoBase_c::executeRemote)) {
        return false;
    }
    return true;
}

// .text 0xBB0
void dAcNpcSpPerikoBase_c::onDemoCmd(int cmd, u8 member) {
    const int originalCmd = cmd;
    _1B70 = member;
    int permit = requestPermit(cmd, member, TRUE);
    switch (cmd) {
    case DEMO_CMD_START:
        if (permit == PERMIT_REMOTE_START || permit == PERMIT_LOCAL_START) {
            changeMode(&dAcNpcSpPerikoBase_c::executeDemoWait);
        }
        break;
    case DEMO_CMD_TALK:
        if (permit == PERMIT_REMOTE_TALK) {
            changeMode(&dAcNpcSpPerikoBase_c::executeRemoteTalk);
        } else if (permit == PERMIT_LOCAL_TALK) {
            mWindowTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeMode(&dAcNpcSpPerikoBase_c::executeTalk);
        }
        break;
    case DEMO_CMD_END:
        if (permit == PERMIT_END_SINGLE) {
            changeMode(&dAcNpcSpPerikoBase_c::executeTurnBack);
        } else if (permit == PERMIT_END_MULTI) {
            changeMode(&dAcNpcSpPerikoBase_c::executeRemote);
        }
        break;
    case CMD_RELEASE_PERMIT:
        if (permit == PERMIT_RELEASED) {
            changeMode(&dAcNpcSpPerikoBase_c::executeTurnBack);
        }
        break;
    }
    dAcNpcSp_c::onDemoCmd(originalCmd, member);
}

// .text 0x135C
BOOL dAcNpcSpPerikoBase_c::isPelly() const {
    return mType == TYPE_PELLY;
}

// .text 0x136C
void dAcNpcSpPerikoBase_c::initWait() {
    mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
}

// .text 0x1380
void dAcNpcSpPerikoBase_c::executeWait() {
    if (!isMultiPlay() && isPelly() &&
        dPlayerMgr::isState(dPlayerMgr::STATE_REACT,
                            dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        changeMode(&dAcNpcSpPerikoBase_c::executeWaitClap);
    } else if (checkWindowChange()) {
        changeMode(&dAcNpcSpPerikoBase_c::executeMove);
    }
}

// .text 0x1644
void dAcNpcSpPerikoBase_c::initMove() {
    moveParam_c param;
    if (isPelly()) {
        param.mSpeed = 1.35f;
        param.mAccel = 0.28f;
        param.mDecel = 0.4f;
    } else {
        param.mSpeed = 0.9f;
        param.mAccel = 0.2f;
        param.mDecel = 0.3f;
    }
    mVec3_c pos(dANpcSpPeriko::l_moveTarget[mWindow]);
    mAction.requestRunFacing(PRIORITY_DEMO, pos, l_runTurnSpeed,
                             dANpcSpPeriko::l_moveTurnMode[mWindow], param,
                             dANpcSpPeriko::l_arriveDist, cNpcMorphFrames);
}

// .text 0x1738
BOOL dAcNpcSpPerikoBase_c::isMoveDone() {
    mVec3_c target(dANpcSpPeriko::l_moveTarget[mWindow]);
    mVec3_c diff = target - mPos;
    return std::fabs(diff.xzLen()) <= dANpcSpPeriko::l_arriveDist;
}

// .text 0x17CC
void dAcNpcSpPerikoBase_c::executeMove() {
    if (checkWindowChange()) {
        if (isMoveDone()) {
            changeMode(&dAcNpcSpPerikoBase_c::executeTurnBack);
        } else {
            changeMode(&dAcNpcSpPerikoBase_c::executeMove);
        }
    } else if (isMoveDone()) {
        changeMode(&dAcNpcSpPerikoBase_c::executeTurnBack);
    }
}

// .text 0x1BAC
void dAcNpcSpPerikoBase_c::initTurnBack() {
    mAng angle(0);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_runTurnSpeed, 0,
                        cNpcMorphFrames);
}

// .text 0x1BF4
void dAcNpcSpPerikoBase_c::executeTurnBack() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        changeMode(&dAcNpcSpPerikoBase_c::executeWait);
    }
}

// .text 0x1D68
void dAcNpcSpPerikoBase_c::initDemoWait() {
}

// .text 0x1D6C
void dAcNpcSpPerikoBase_c::executeDemoWait() {
}

// .text 0x1D70
void dAcNpcSpPerikoBase_c::initTalk() {
    mTalk.requestTalk(true, FALSE);
}

// .text 0x1D80
void dAcNpcSpPerikoBase_c::executeTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
    }
}

// .text 0x1DC0
void dAcNpcSpPerikoBase_c::initRemote() {
}

// .text 0x1DC4
void dAcNpcSpPerikoBase_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mWindowTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeMode(&dAcNpcSpPerikoBase_c::executeTalk);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeMode(&dAcNpcSpPerikoBase_c::executeWait);
        }
    }
}

// .text 0x2098
void dAcNpcSpPerikoBase_c::initRemoteTalk() {
}

// .text 0x209C
void dAcNpcSpPerikoBase_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeMode(&dAcNpcSpPerikoBase_c::executeWait);
    }
}

// .text 0x2218
void dAcNpcSpPerikoBase_c::initWaitClap() {
    mClapTimer = 50;
}

// .text 0x2224
void dAcNpcSpPerikoBase_c::executeWaitClap() {
    if (mClapTimer != 0) {
        mClapTimer--;
    }
    if (mClapTimer == 0) {
        changeMode(&dAcNpcSpPerikoBase_c::executeClap);
    }
}

// .text 0x23A0
void dAcNpcSpPerikoBase_c::initClap() {
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      0.0f);
    mAction.requestEmotion(PRIORITY_IDLE, action_c::EMOTION_CLAP,
                           cNpcMorphFrames);
}

// .text 0x2408
void dAcNpcSpPerikoBase_c::executeClap() {
    if (!dPlayerMgr::isState(dPlayerMgr::STATE_REACT,
                             dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                          mLookRange);
        stopManpu();
        changeMode(&dAcNpcSpPerikoBase_c::executeWait);
    }
}

// .text 0x25A4
void dAcNpcSpPerikoBase_c::vt80() {
    changeMode(&dAcNpcSpPerikoBase_c::executePartner);
}

// .text 0x2700
void dAcNpcSpPerikoBase_c::vt84() {
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      mLookRange);
    changeMode(&dAcNpcSpPerikoBase_c::executeTurnBack);
}

// .text 0x2880
void dAcNpcSpPerikoBase_c::initPartner() {
    initWait();
}

// .text 0x2884
void dAcNpcSpPerikoBase_c::executePartner() {
}

// .text 0x2888
BOOL dAcNpcSpPerikoBase_c::checkWindowChange() {
    if (isMultiPlay()) {
        return FALSE;
    }
    mVec3_c *playerPos =
        dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (playerPos == NULL) {
        return FALSE;
    }
    int window = getWindowAt(playerPos);
    if (window == WINDOW_NONE) {
        return FALSE;
    }
    if (window != mWindow) {
        mWindow = window;
        return TRUE;
    }
    return FALSE;
}

// .text 0x2914
int dAcNpcSpPerikoBase_c::getWindowAt(mVec3_c *pos) const {
    int unitX = worldToUnit(pos->x);
    int unitZ = worldToUnit(pos->z);
    for (int i = 0; i < WINDOW_NUM; i++) {
        mVec3_c front(dANpcSpPeriko::l_frontPos[i]);
        int frontX = worldToUnit(front.x);
        int frontZ = worldToUnit(front.z);
        if (unitX == frontX && unitZ == frontZ) {
            return i;
        }
    }
    return WINDOW_NONE;
}

// .text 0x29F0
dAcNpcSpPerikoBase_c::talk_c::talk_c(dAcNpcSpPerikoBase_c *owner)
    : mpOwner(owner) {
    mMsgNo = 0;
    static const dState::mode_c<talk_c>::Entry modes[] = {
        {"待機", &talk_c::initWait, &talk_c::executeWait},
        {"Wii伝言板書き込み", &talk_c::initWiiBoardPost,
         &talk_c::executeWiiBoardPost},
        {"Wii伝言板書き込み完了チェック", &talk_c::initWiiBoardPostDone,
         &talk_c::executeWiiBoardPostDone},
        {"郵便局の手紙枠の空きチェック", &talk_c::initPostOfficeCheck,
         &talk_c::executePostOfficeCheck},
    };
    dState::mode_c<talk_c>::initialize("dAcNpcSpPerikoBase_c::talk_c", modes,
                                       ARRAY_SIZE(modes));
}

// .text 0x2AC0
BOOL dAcNpcSpPerikoBase_c::talk_c::isPelly() const {
    return mpOwner->isPelly();
}

// .text 0x2AC8
BOOL dAcNpcSpPerikoBase_c::talk_c::isBridgeBuilt() {
    dSaveData_c *save = dSaveData_c::getRaw();
    return save->mMainField.mBridgeBlockX == -1 &&
           save->mMainField.mBridgeBlockZ == -1;
}

// .text 0x2B14
BOOL dAcNpcSpPerikoBase_c::talk_c::hasFountain() {
    dItem::Item key((u16)BUILDING_FOUNTAIN);
    int x, z;
    dSaveData_c *save = dSaveData_c::getRaw();
    return save->mBuilding.mList.getPos(&x, &z, key, 1);
}

// .text 0x2B5C
BOOL dAcNpcSpPerikoBase_c::talk_c::hasWindmill() {
    dItem::Item key((u16)BUILDING_WINDMILL);
    int x, z;
    dSaveData_c *save = dSaveData_c::getRaw();
    return save->mBuilding.mList.getPos(&x, &z, key, 1);
}

// .text 0x2BA4
BOOL dAcNpcSpPerikoBase_c::talk_c::hasLighthouse() {
    dItem::Item key((u16)BUILDING_LIGHTHOUSE);
    int x, z;
    dSaveData_c *save = dSaveData_c::getRaw();
    return save->mBuilding.mList.getPos(&x, &z, key, 1);
}

// .text 0x2BEC
void dAcNpcSpPerikoBase_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mRequestEndMethod = NULL;
}

// .text 0x2C3C
void dAcNpcSpPerikoBase_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (!player->isFlag0(PRIVATE_FLAG0_NEWCOMER_DONE)) {
        if (player->isFlag0(PRIVATE_FLAG0_HOUSE_TOUR)) {
            mMsgNo = MSG_NEWCOMER_WELCOME;
            player->clearFlag0(PRIVATE_FLAG0_HOUSE_TOUR);
        } else {
            dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
            if (homes->getHome(homes->findCurrentPlayer()) == NULL) {
                mMsgNo = MSG_NEWCOMER_AGAIN;
            } else {
                mMsgNo = MSG_NEWCOMER_HOUSE_DONE;
            }
        }
    } else if (mpOwner->isDramaDue()) {
        mMsgNo = MSG_DRAMA_SIGH;
    } else if (isBridgeThanksDue()) {
        mMsgNo = MSG_BRIDGE_THANKS;
        player->setFlag0(PRIVATE_FLAG0_BRIDGE_THANKED);
        player->clearFlag0(PRIVATE_FLAG0_PUBLIC_WORKS_BUILT);
    } else if (isFountainThanksDue()) {
        mMsgNo = MSG_FOUNTAIN_THANKS;
        player->setFlag0(PRIVATE_FLAG0_FOUNTAIN_THANKED);
        player->clearFlag0(PRIVATE_FLAG0_PUBLIC_WORKS_BUILT);
    } else if (isWindmillThanksDue()) {
        mMsgNo = MSG_WINDMILL_THANKS;
        player->setFlag0(PRIVATE_FLAG0_TOWER_THANKED);
        player->clearFlag0(PRIVATE_FLAG0_PUBLIC_WORKS_BUILT);
    } else if (isLighthouseThanksDue()) {
        mMsgNo = MSG_LIGHTHOUSE_THANKS;
        player->setFlag0(PRIVATE_FLAG0_TOWER_THANKED);
        player->clearFlag0(PRIVATE_FLAG0_PUBLIC_WORKS_BUILT);
    } else if (isWindmillThanks2Due()) {
        mMsgNo = MSG_WINDMILL_THANKS2;
        player->setFlag0(PRIVATE_FLAG0_WINDMILL_REBUILT_THANKED);
        player->clearFlag0(PRIVATE_FLAG0_PUBLIC_WORKS_BUILT);
    } else if (isLighthouseThanks2Due()) {
        mMsgNo = MSG_LIGHTHOUSE_THANKS2;
        player->setFlag0(PRIVATE_FLAG0_LIGHTHOUSE_REBUILT_THANKED);
        player->clearFlag0(PRIVATE_FLAG0_PUBLIC_WORKS_BUILT);
    } else {
        mMsgNo = getMenuMsg();
    }
}

// .text 0x2E68
BOOL dAcNpcSpPerikoBase_c::talk_c::isBridgeThanksDue() {
    BOOL due;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    due = FALSE;
    if (player->isFlag0(PRIVATE_FLAG0_PUBLIC_WORKS_BUILT) &&
        isPlayerFromThisTown() && isPelly() && isBridgeBuilt() &&
        player->isFlag0(PRIVATE_FLAG0_FUND_DONATED) &&
        !player->isFlag0(PRIVATE_FLAG0_BRIDGE_THANKED)) {
        due = TRUE;
    }
    return due;
}

// .text 0x2F18
BOOL dAcNpcSpPerikoBase_c::talk_c::isFountainThanksDue() {
    BOOL due;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    due = FALSE;
    if (player->isFlag0(PRIVATE_FLAG0_PUBLIC_WORKS_BUILT) &&
        isPlayerFromThisTown() && isPelly() && hasFountain() &&
        player->isFlag0(PRIVATE_FLAG0_FUND_DONATED) &&
        !player->isFlag0(PRIVATE_FLAG0_FOUNTAIN_THANKED)) {
        due = TRUE;
    }
    return due;
}

// .text 0x2FC8
BOOL dAcNpcSpPerikoBase_c::talk_c::isWindmillThanksDue() {
    BOOL due;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    due = FALSE;
    if (player->isFlag0(PRIVATE_FLAG0_PUBLIC_WORKS_BUILT) &&
        isPlayerFromThisTown() && isPelly() && hasWindmill() &&
        player->isFlag0(PRIVATE_FLAG0_FUND_DONATED) &&
        !player->isFlag0(PRIVATE_FLAG0_TOWER_THANKED)) {
        due = TRUE;
    }
    return due;
}

// .text 0x3078
BOOL dAcNpcSpPerikoBase_c::talk_c::isLighthouseThanksDue() {
    BOOL due;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    due = FALSE;
    if (player->isFlag0(PRIVATE_FLAG0_PUBLIC_WORKS_BUILT) &&
        isPlayerFromThisTown() && isPelly() && hasLighthouse() &&
        player->isFlag0(PRIVATE_FLAG0_FUND_DONATED) &&
        !player->isFlag0(PRIVATE_FLAG0_TOWER_THANKED)) {
        due = TRUE;
    }
    return due;
}

// .text 0x3128
BOOL dAcNpcSpPerikoBase_c::talk_c::isWindmillThanks2Due() {
    BOOL due;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    due = FALSE;
    if (player->isFlag0(PRIVATE_FLAG0_PUBLIC_WORKS_BUILT) &&
        isPlayerFromThisTown() && isPelly() && hasWindmill() &&
        player->isFlag0(PRIVATE_FLAG0_FUND_DONATED) &&
        !player->isFlag0(PRIVATE_FLAG0_WINDMILL_REBUILT_THANKED)) {
        due = TRUE;
    }
    return due;
}

// .text 0x31D8
BOOL dAcNpcSpPerikoBase_c::talk_c::isLighthouseThanks2Due() {
    BOOL due;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    due = FALSE;
    if (player->isFlag0(PRIVATE_FLAG0_PUBLIC_WORKS_BUILT) &&
        isPlayerFromThisTown() && isPelly() && hasLighthouse() &&
        player->isFlag0(PRIVATE_FLAG0_FUND_DONATED) &&
        !player->isFlag0(PRIVATE_FLAG0_LIGHTHOUSE_REBUILT_THANKED)) {
        due = TRUE;
    }
    return due;
}

// .text 0x3288
int dAcNpcSpPerikoBase_c::talk_c::getMenuMsg() {
    int msg = MSG_POST_OFFICE;
    if (mpOwner->mWindow == WINDOW_TOWN_HALL) {
        msg = MSG_TOWN_HALL_MENU;
        if (!isPlayerFromThisTown()) {
            if (isPelly()) {
                msg = MSG_TOWN_HALL_WELCOME;
            } else {
                dTime_c now = *dTime_c::getCurrent();
                if (now.hour >= PELLY_START_HOUR &&
                    now.hour < PHYLLIS_START_HOUR) {
                    msg = MSG_TOWN_HALL_WELCOME;
                } else {
                    msg = MSG_TOWN_HALL_CLOSED;
                }
            }
        }
    }
    return msg;
}

// .text 0x3368
void dAcNpcSpPerikoBase_c::talk_c::getMsgInfo(msgInfo_s *info) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int label = LABEL_OWN;
    if (!player->isFlag0(PRIVATE_FLAG0_NEWCOMER_DONE)) {
        label = LABEL_NEWCOMER;
    } else if (mpOwner->isDramaDue()) {
        label = LABEL_DRAMA;
    }
    info->mLabel = dANpcSpPeriko::l_msgLabels[label][mpOwner->mType];
    info->mCode = mMsgNo;
}

// .text 0x3404
void dAcNpcSpPerikoBase_c::talk_c::preExecute() {
    dState::base_c<talk_c>::execute();
}

// .text 0x3454
void dAcNpcSpPerikoBase_c::talk_c::onTalkEnd() {
}

// .text 0x3458
void dAcNpcSpPerikoBase_c::talk_c::setNextMessage(u16 code, int label) {
    mpController->setNextMsgLabel(
        dANpcSpPeriko::l_msgLabels[label][mpOwner->mType]);
    mpController->setNextMsgCode(code);
}

// .text 0x34C0
void dAcNpcSpPerikoBase_c::talk_c::onMessageStart(int arg) {
}

// .text 0x34C4
void dAcNpcSpPerikoBase_c::talk_c::onMessageEnd(int kind) {
    struct Entry {
        int label;
        int code;
        Method method;
    };
    static const Entry messages[] = {
        {LABEL_DRAMA, MSG_DRAMA_LETTER, &talk_c::endDrama},
        {LABEL_DRAMA, MSG_DRAMA_SPACED_OUT, &talk_c::endDrama},
        {LABEL_NEWCOMER, MSG_NEWCOMER_WELCOME, &talk_c::showVacantHouses},
        {LABEL_OWN, MSG_WHERE_SEND, &talk_c::selectMailDestination},
        {LABEL_OWN, MSG_NOT_RESIDENT_RETURNED, &talk_c::endMailReturned},
        {LABEL_OWN, MSG_ALSO, &talk_c::showFullMailboxes},
        {LABEL_OWN, MSG_MAILBOX_FULL1, &talk_c::endFullMailboxes},
        {LABEL_OWN, MSG_MAILBOX_FULL2, &talk_c::endFullMailboxes},
        {LABEL_OWN, MSG_MAILBOX_FULL3, &talk_c::endFullMailboxes},
        {LABEL_OWN, MSG_MAILBOX_FULL4, &talk_c::endFullMailboxes},
        {LABEL_OWN, MSG_PREPARE_BOARD_POST, &talk_c::startBoardPost},
        {LABEL_OWN, MSG_FRIENDS_ONLY, &talk_c::endFullMailboxes},
        {LABEL_OWN, MSG_VISITOR_LETTER, &talk_c::startPostOfficeCheck},
        {LABEL_OWN, MSG_WC24_OFF_MAIL, &talk_c::endWc24Error},
        {LABEL_OWN, MSG_WC24_OFF_BOARD, &talk_c::endWc24Error},
        {LABEL_OWN, MSG_READ_COMMENTS, &talk_c::readTownComments},
        {LABEL_OWN, MSG_BRIDGE_SURVEY_FIRST, &talk_c::openBridgeMap},
        {LABEL_OWN, MSG_BRIDGE_SURVEY, &talk_c::openBridgeMap},
        {LABEL_OWN, MSG_WHO_PROBLEM, &talk_c::selectFirstVillagers},
        {LABEL_OWN, MSG_WHO_PROBLEM_MORE, &talk_c::selectVillagers},
        {LABEL_OWN, MSG_RUDE, &talk_c::resetHabitWord},
        {LABEL_OWN, MSG_CLOTHES, &talk_c::resetCloth},
        {LABEL_OWN, MSG_LETTERS, &talk_c::clearLetter},
        {LABEL_OWN, MSG_BRIDGE_THANKS, &talk_c::showMenu},
        {LABEL_OWN, MSG_FOUNTAIN_THANKS, &talk_c::showMenu},
        {LABEL_OWN, MSG_LIGHTHOUSE_THANKS, &talk_c::showMenu},
        {LABEL_OWN, MSG_WINDMILL_THANKS, &talk_c::showMenu},
        {LABEL_OWN, MSG_LIGHTHOUSE_THANKS2, &talk_c::showMenu},
        {LABEL_OWN, MSG_WINDMILL_THANKS2, &talk_c::showMenu},
        {LABEL_OWN, MSG_WHICH_ROOM, &talk_c::selectRoom},
        {LABEL_OWN, MSG_ROOM_FIRST_FLOOR, &talk_c::setRoomFirstFloor},
        {LABEL_OWN, MSG_ROOM_SECOND_FLOOR, &talk_c::setRoomSecondFloor},
        {LABEL_OWN, MSG_ROOM_BASEMENT, &talk_c::setRoomBasement},
        {LABEL_OWN, MSG_ROOM_CLEARED, &talk_c::recycleRoom},
        {LABEL_REPORT, MSG_REPORT_BEST, &talk_c::endBestReport},
        {LABEL_REPORT, MSG_REPORT_GOOD, &talk_c::endGoodReport},
        {LABEL_REPORT, MSG_REPORT_D_PLUS, &talk_c::endBadReport},
        {LABEL_REPORT, MSG_REPORT_POOR, &talk_c::endBadReport},
        {LABEL_REPORT, MSG_REPORT_PITS, &talk_c::endBadReport},
        {LABEL_NET_ERROR, NET_ERROR_MSG_RESTRICTED, &talk_c::endWc24Error},
        {LABEL_NET_ERROR, NET_ERROR_MSG_WC24_OFF, &talk_c::endWc24Error},
        {LABEL_NET_ERROR, NET_ERROR_MSG_WC24_NAND, &talk_c::endNetError},
        {LABEL_NET_ERROR, NET_ERROR_MSG_WC24_UPDATE, &talk_c::endNetError},
        {LABEL_NET_ERROR, NET_ERROR_MSG_WC24_NO_INTERNET, &talk_c::endNetError},
        {LABEL_NET_ERROR, NET_ERROR_MSG_WC24_BUSY2, &talk_c::endNetError},
        {LABEL_NET_ERROR, NET_ERROR_MSG_WC24_NAND2, &talk_c::endNetError},
        {LABEL_NET_ERROR, NET_ERROR_MSG_WC24_OTHER, &talk_c::endNetError},
        {LABEL_NET_ERROR, NET_ERROR_MSG_WC24_RESET, &talk_c::lockMessage},
        {LABEL_NET_ERROR, NET_ERROR_MSG_WC24_RESET2, &talk_c::lockMessage},
        {LABEL_NET_ERROR, NET_ERROR_MSG_WC24_BUSY, &talk_c::endBoardError},
        {LABEL_NET_ERROR, NET_ERROR_MSG_WC24_BUSY3, &talk_c::endBoardError},
    };
    int i;
    u16 code = getMessageCode();
    const char *label = getMessageLabel();
    for (i = 0; i < ARRAY_SIZE(messages); i++) {
        if (std::strcmp(label,
                        dANpcSpPeriko::l_msgLabels[messages[i].label]
                                                  [mpOwner->mType]) == 0 &&
            code == messages[i].code) {
            (this->*messages[i].method)();
        }
    }

    // The end requests 0x10..0x14 of each label.
    static Method endRequests[LABEL_NUM][END_REQ_TABLE_NUM] = {
        {&talk_c::openHouseMap, NULL, NULL, NULL, NULL},
        {&talk_c::openTownTuneMenu, &talk_c::openFundMenu,
         &talk_c::openMailSaveMenu, &talk_c::openMailMenu,
         &talk_c::openFutureDateMenu},
        {NULL, NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL, NULL},
    };
    int request = kind - END_REQ_TABLE_FIRST;
    for (i = 0; i < LABEL_NUM; i++) {
        if (request >= 0 &&
            std::strcmp(label, dANpcSpPeriko::l_msgLabels[i][mpOwner->mType]) ==
                0 &&
            endRequests[i][request]) {
            (this->*endRequests[i][request])();
        }
    }

    if (kind == END_REQ_HAND_OVER) {
        if (mpOwner->mWindow == WINDOW_POST_OFFICE) {
            handOverLetters();
        } else {
            handOverGoldenCan();
        }
    }

    // Visitors can't send more letters: say goodbye.
    if (std::strcmp(getMessageLabel(),
                    dANpcSpPeriko::l_msgLabels[LABEL_OWN][mpOwner->mType]) ==
            0 &&
        mpController->mNextMsgCode == MSG_ANY_OTHER_LETTERS &&
        !isPlayerFromThisTown()) {
        setNextMessage(MSG_COME_AGAIN, LABEL_OWN);
    }
}

// .text 0x3794
void dAcNpcSpPerikoBase_c::talk_c::selectMailDestination() {
    dMsg::Comp_c *controller = mpController;
    dSelect::Comp_c *select = &controller->mSelect;
    select->setSysMessage(0, SEL_MSG_THIS_TOWN, 0, NULL, 0, 0);
    int num = 1;
    if (dSaveData_c::getRaw()->isFlag(SAVE_FLAG_BUS_NET_EVENT_DONE)) {
        select->setSysMessage(1, SEL_MSG_OTHER_TOWN, MSG_OTHER_TOWN_MAIL, NULL,
                              0, 0);
        num = 2;
    }
    select->setSysMessage(num, SEL_MSG_FUTURE_SELF, 0, NULL, 0, 0);
    controller->mSelect.mCancel = num + 1;
    select->setSysMessage(num + 1, SEL_MSG_NEVER_MIND_MAIL, MSG_WII_BOARD_END,
                          NULL, 0, 0);
    controller->mSelect.mNum = num + 2;
    controller->openSelect();
}

// .text 0x3880
void dAcNpcSpPerikoBase_c::talk_c::endDrama() {
    mpOwner->clearDrama();
    int msg = getMenuMsg();
    setNextMessage(msg, LABEL_OWN);
}

// .text 0x38CC
void dAcNpcSpPerikoBase_c::talk_c::resetHabitWord() {
    dHabitWord_c habit;
    dHabitWord_c other;
    switch (mpVillager->mID.getLooks(1)) {
    case LOOKS_TYPE_NORMAL:
        dString::loadString(&habit, UNIT_HABIT_NORMAL, "sys_STRING/STR_Unit");
        dString::loadString(&other, UNIT_HABIT_NORMAL2, "sys_STRING/STR_Unit");
        break;
    case LOOKS_TYPE_PEPPY:
        dString::loadString(&habit, UNIT_HABIT_PEPPY, "sys_STRING/STR_Unit");
        dString::loadString(&other, UNIT_HABIT_PEPPY2, "sys_STRING/STR_Unit");
        break;
    case LOOKS_TYPE_SNOOTY:
        dString::loadString(&habit, UNIT_HABIT_SNOOTY, "sys_STRING/STR_Unit");
        dString::loadString(&other, UNIT_HABIT_SNOOTY2, "sys_STRING/STR_Unit");
        break;
    case LOOKS_TYPE_LAZY:
        dString::loadString(&habit, UNIT_HABIT_LAZY, "sys_STRING/STR_Unit");
        dString::loadString(&other, UNIT_HABIT_LAZY2, "sys_STRING/STR_Unit");
        break;
    case LOOKS_TYPE_JOCK:
        dString::loadString(&habit, UNIT_HABIT_JOCK, "sys_STRING/STR_Unit");
        dString::loadString(&other, UNIT_HABIT_JOCK2, "sys_STRING/STR_Unit");
        break;
    case LOOKS_TYPE_CRANKY:
        dString::loadString(&habit, UNIT_HABIT_CRANKY, "sys_STRING/STR_Unit");
        dString::loadString(&other, UNIT_HABIT_CRANKY2, "sys_STRING/STR_Unit");
        break;
    }
    dHabitWord_c current;
    mpVillager->getHabitWord(&current, 10, FALSE);
    if (current.isSame(&habit)) {
        mpVillager->setHabitFromWord(&other, 10);
    } else {
        mpVillager->setHabitFromWord(&habit, 10);
    }
}

// .text 0x3AB8
void dAcNpcSpPerikoBase_c::talk_c::resetCloth() {
    dItem::Item orgCloth((u16)mpVillager->mTemplate.getCloth());
    dItem::Item cloth = mpVillager->mCloth;
    if (mpVillager->isWearingOrgCloth()) {
        mpVillager->setCloth(&orgCloth);
    } else if (cloth.isSame(orgCloth)) {
        if (mpVillager->mID.getGender(1) == GENDER_MALE) {
            dItem::Item shirt(dItem::ITEM_IDX_BLUE_SWEATSUIT);
            mpVillager->setCloth(&shirt);
        } else {
            dItem::Item shirt(dItem::ITEM_IDX_RED_SWEATSUIT);
            mpVillager->setCloth(&shirt);
        }
    } else {
        mpVillager->setCloth(&orgCloth);
    }
}

// .text 0x3B88
void dAcNpcSpPerikoBase_c::talk_c::showMenu() {
    int msg = getMenuMsg();
    setNextMessage(msg, LABEL_OWN);
}

// .text 0x3BC8
void dAcNpcSpPerikoBase_c::talk_c::clearLetter() {
    mpVillager->mLetter.clear();
}

// .text 0x3BD4
void dAcNpcSpPerikoBase_c::talk_c::selectFirstVillagers() {
    mVillagerPage = 0;
    selectVillagers();
}

// .text 0x3BE0
void dAcNpcSpPerikoBase_c::talk_c::selectVillagers() {
    dAnmPersonalID_c *id;
    dMsg::Comp_c *controller = mpController;
    dSelect::Comp_c *select = &controller->mSelect;
    dAnimalBlock_c *animals = &dSaveData_c::getRaw()->mAnimals.mTown;
    int num;
    int idx;
    int first = mVillagerPage;
    int i;
    num = 0;
    idx = 0;
    for (i = 0; i < ANIMAL_NUM; i++) {
        dAnimal_c *animal = animals->getAnimalConst(i);
        if (animal != NULL) {
            id = &animal->mID;
            if (id->isValid()) {
                if (idx >= first && num < VILLAGERS_PER_PAGE) {
                    select->setAnimalName(num, id, MSG_WHAT_PROBLEM, NULL, 0,
                                          1);
                    num++;
                }
                idx++;
            }
        }
    }
    select->setSysMessage(num, SEL_MSG_SOMEONE_ELSE, MSG_WHO_PROBLEM_MORE, NULL,
                          0, 0);
    controller->mSelect.mCancel = num + 1;
    select->setSysMessage(num + 1, SEL_MSG_NEVER_MIND, MSG_NOTHING_THE_MATTER,
                          NULL, 0, 0);
    controller->mSelect.mNum = num + 2;
    controller->openSelect();
}

// .text 0x3CF4
void dAcNpcSpPerikoBase_c::talk_c::showVacantHouses() {
    int vacant = dSaveData_c::getTown()->mHomes.countVacant();
    formatNumber(vacant, 0, 0xC, dScript::NUM_FORMAT_REGION);
    if (vacant == 1) {
        setNextMessage(MSG_NEWCOMER_VACANT_HOUSE, LABEL_NEWCOMER);
    } else {
        setNextMessage(MSG_NEWCOMER_VACANT_HOUSES, LABEL_NEWCOMER);
    }
}

// .text 0x3D7C
void dAcNpcSpPerikoBase_c::talk_c::openBridgeMap() {
    reqMenu1F(0);
    mRequestEndMethod = &talk_c::endBridgeMap;
}

// .text 0x3DC8
void dAcNpcSpPerikoBase_c::talk_c::readTownComments() {
    int rank = fgMngProc_assessLiveTown();
    setNextMessage(rank + MSG_REPORT_BEST, LABEL_REPORT);
}

// .text 0x3E0C
void dAcNpcSpPerikoBase_c::talk_c::endBestReport() {
    s8 perfectDays;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    perfectDays = dSaveData_c::getTown()->mTownInfo.mPerfectDays;
    if (player->isFlag0(PRIVATE_FLAG0_GOLDEN_CAN_RECEIVED)) {
        setNextMessage(MSG_GREAT_PLACE, LABEL_OWN);
    } else if (perfectDays >= 15) {
        dItem::Item can(dItem::ITEM_IDX_GOLDEN_CAN);
        bool picked = player->pickUp(can, FALSE);
        if (picked) {
            player->setFlag0(PRIVATE_FLAG0_GOLDEN_CAN_RECEIVED);
            setNextMessage(MSG_GOLDEN_CAN, LABEL_OWN);
        } else {
            setNextMessage(MSG_LUCKIEST_PELICAN, LABEL_OWN);
        }
    } else {
        setNextMessage(MSG_LUCKIEST_PELICAN, LABEL_OWN);
    }
}

// .text 0x3EFC
void dAcNpcSpPerikoBase_c::talk_c::endGoodReport() {
    setNextMessage(MSG_TOWN_WORK_TOGETHER, LABEL_OWN);
}

// .text 0x3F08
void dAcNpcSpPerikoBase_c::talk_c::endBadReport() {
    setNextMessage(fgMngProc_getWorstBlock()->mReason + MSG_PROBLEM_FIRST,
                   LABEL_OWN);
}

// .text 0x3F4C
void dAcNpcSpPerikoBase_c::talk_c::endNetError() {
    switchToOwnSpeaker();
    switch (mNetErrorFrom) {
    case NET_ERROR_FROM_BOARD_POST:
        setNextMessage(MSG_BOARD_DISPOSED, LABEL_OWN);
        break;
    case NET_ERROR_FROM_WII_BOARD:
        setNextMessage(MSG_WII_BOARD_END, LABEL_OWN);
        break;
    case NET_ERROR_FROM_MAIL:
        setNextMessage(MSG_ANY_OTHER_LETTERS, LABEL_OWN);
        break;
    }
}

// .text 0x3FD4
void dAcNpcSpPerikoBase_c::talk_c::endBoardError() {
    switchToOwnSpeaker();
    switch (mNetErrorFrom) {
    case NET_ERROR_FROM_BOARD_POST:
        setNextMessage(MSG_BOARD_RETRY, LABEL_OWN);
        break;
    case NET_ERROR_FROM_WII_BOARD:
        setNextMessage(MSG_WII_BOARD_END, LABEL_OWN);
        break;
    case NET_ERROR_FROM_MAIL:
        setNextMessage(MSG_ANY_OTHER_LETTERS, LABEL_OWN);
        break;
    }
}

// .text 0x405C
void dAcNpcSpPerikoBase_c::talk_c::lockMessage() {
    mpController->mLock = 1;
}

// .text 0x406C
void dAcNpcSpPerikoBase_c::talk_c::endWc24Error() {
    switchToOwnSpeaker();
    switch (mNetErrorFrom) {
    case NET_ERROR_FROM_BOARD_POST:
        setNextMessage(MSG_BOARD_DISPOSED, LABEL_OWN);
        break;
    case NET_ERROR_FROM_WII_BOARD:
        setNextMessage(MSG_WII_BOARD_END, LABEL_OWN);
        break;
    case NET_ERROR_FROM_MAIL:
        setNextMessage(MSG_ANY_OTHER_LETTERS, LABEL_OWN);
        break;
    }
}

// .text 0x40F4
void dAcNpcSpPerikoBase_c::talk_c::endMailReturned() {
    if (isMenuSel40()) {
        setNextMessage(MSG_ALSO, LABEL_OWN);
    } else {
        setNextMessage(MSG_ANY_OTHER_LETTERS, LABEL_OWN);
    }
}

// .text 0x414C
void dAcNpcSpPerikoBase_c::talk_c::showFullMailboxes() {
    if (isMultiPlay()) {
        if (isMenuSel20()) {
            setNextMessage(MSG_POST_OFFICE_FULL2, LABEL_OWN);
        } else if (isMenuSel40()) {
            setNextMessage(MSG_MAIL_TAKEN, LABEL_OWN);
        } else {
            setNextMessage(MSG_ANY_OTHER_LETTERS, LABEL_OWN);
        }
    } else {
        int full = countMenuSelSlots();
        if (full == 0) {
            setNextMessage(MSG_MAIL_TAKEN, LABEL_OWN);
        } else {
            int idx = 0;
            for (int i = 0; i < PLAYER_NUM; i++) {
                if (isMenuSelSlot(i)) {
                    setPersonalName(&dPlayerMgr::Mgr_c::getPlayer(i)->mPID,
                                    idx);
                    idx++;
                }
            }
            switch (full) {
            case 1:
                setNextMessage(MSG_MAILBOX_FULL1, LABEL_OWN);
                break;
            case 2:
                setNextMessage(MSG_MAILBOX_FULL2, LABEL_OWN);
                break;
            case 3:
                setNextMessage(MSG_MAILBOX_FULL3, LABEL_OWN);
                break;
            case 4:
                setNextMessage(MSG_MAILBOX_FULL4, LABEL_OWN);
                break;
            }
        }
    }
}

// .text 0x42C8
void dAcNpcSpPerikoBase_c::talk_c::endFullMailboxes() {
    if (isMenuSel40()) {
        setNextMessage(MSG_MAIL_TAKEN_REST, LABEL_OWN);
    } else {
        setNextMessage(MSG_ANY_OTHER_LETTERS, LABEL_OWN);
    }
}

// .text 0x4320
void dAcNpcSpPerikoBase_c::talk_c::startBoardPost() {
    dMsg::Comp_c *controller = mpController;
    controller->mLock = 1;
    controller->mWaitIconSe = 1;
    controller->mWaitIcon = 1;
    changeMode(&talk_c::executeWiiBoardPost);
}

// .text 0x4490
void dAcNpcSpPerikoBase_c::talk_c::startPostOfficeCheck() {
    dMsg::Comp_c *controller = mpController;
    controller->mLock = 1;
    controller->mWaitIconSe = 1;
    controller->mWaitIcon = 1;
    changeMode(&talk_c::executePostOfficeCheck);
}

// .text 0x4600
void dAcNpcSpPerikoBase_c::talk_c::handOverLetters() {
    dItem::Item item(dItem::ITEM_IDX_LETTER);
    requestItemAct(&item, 0, 1);
}

// .text 0x4648
void dAcNpcSpPerikoBase_c::talk_c::handOverGoldenCan() {
    dItem::Item item(dItem::ITEM_IDX_GOLDEN_CAN);
    requestItemAct(&item, 0, 1);
}

// .text 0x4690
void dAcNpcSpPerikoBase_c::talk_c::openHouseMap() {
    reqMenu2E(0);
    mRequestEndMethod = &talk_c::endHouseMap;
}

// .text 0x46DC
void dAcNpcSpPerikoBase_c::talk_c::openTownTuneMenu() {
    reqMenu1E(0);
    mRequestEndMethod = &talk_c::endTownTuneMenu;
}

// .text 0x4728
void dAcNpcSpPerikoBase_c::talk_c::openFundMenu() {
    reqMenu20(1);
    mRequestEndMethod = &talk_c::endFund;
}

// .text 0x4774
void dAcNpcSpPerikoBase_c::talk_c::openMailSaveMenu() {
    reqMenu1D(0);
    mRequestEndMethod = &talk_c::endMailSaveMenu;
}

// .text 0x47C0
void dAcNpcSpPerikoBase_c::talk_c::openMailMenu() {
    switch (mMailKind) {
    case MAIL_TOWN:
        reqMenu23(1);
        mRequestEndMethod = &talk_c::endTownMailMenu;
        break;
    case MAIL_OTHER_TOWN:
        reqMenu24(1);
        mRequestEndMethod = &talk_c::endOtherTownMailMenu;
        break;
    case MAIL_FUTURE:
        reqMenu25(1);
        mRequestEndMethod = &talk_c::endFutureMailMenu;
        break;
    }
}

// .text 0x4894
void dAcNpcSpPerikoBase_c::talk_c::openFutureDateMenu() {
    reqMenu26(0);
    mRequestEndMethod = &talk_c::setFutureDateWords;
}

// .text 0x48E0
void dAcNpcSpPerikoBase_c::talk_c::setRoomFirstFloor() {
    mRoom = ROOM_FIRST_FLOOR;
}

// .text 0x48EC
void dAcNpcSpPerikoBase_c::talk_c::setRoomSecondFloor() {
    mRoom = ROOM_SECOND_FLOOR;
}

// .text 0x48F8
void dAcNpcSpPerikoBase_c::talk_c::setRoomBasement() {
    mRoom = ROOM_BASEMENT;
}

// .text 0x4904
void dAcNpcSpPerikoBase_c::talk_c::recycleRoom() {
    dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
    dHome_c *home = homes->getHome(homes->findCurrentPlayer());
    if (home != NULL) {
        switch (mRoom) {
        case ROOM_FIRST_FLOOR:
            home->recycleRoomItems(ROOM_FIRST_FLOOR);
            break;
        case ROOM_SECOND_FLOOR:
            home->recycleRoomItems(ROOM_SECOND_FLOOR);
            break;
        case ROOM_BASEMENT:
            home->recycleRoomItems(ROOM_BASEMENT);
            break;
        }
    }
}

// .text 0x49A4
void dAcNpcSpPerikoBase_c::talk_c::selectRoom() {
    dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
    dHome_c *home = homes->getHome(homes->findCurrentPlayer());
    dMsg::Comp_c *controller = mpController;
    dSelect::Comp_c *select = &controller->mSelect;
    int num = 0;
    if (home != NULL) {
        if (home->hasRoom(ROOM_FIRST_FLOOR)) {
            select->setSysMessage(0, SEL_MSG_FIRST_FLOOR, MSG_ROOM_FIRST_FLOOR,
                                  NULL, 0, 0);
            num = 1;
        }
        if (home->hasRoom(ROOM_SECOND_FLOOR)) {
            select->setSysMessage(num, SEL_MSG_SECOND_FLOOR,
                                  MSG_ROOM_SECOND_FLOOR, NULL, 0, 0);
            num++;
        }
        if (home->hasRoom(ROOM_BASEMENT)) {
            select->setSysMessage(num, SEL_MSG_BASEMENT, MSG_ROOM_BASEMENT,
                                  NULL, 0, 0);
            num++;
        }
    }
    controller->mSelect.mCancel = num;
    select->setSysMessage(num, SEL_MSG_NEVER_MIND_ROOM, MSG_ROOM_NEVER_MIND,
                          NULL, 0, 0);
    controller->mSelect.mNum = num + 1;
    controller->openSelect();
}

// .text 0x4AF4
void dAcNpcSpPerikoBase_c::talk_c::onAnswer(int answer) {
    struct Entry {
        int label;
        int code;
        int answer;
        Method method;
    };
    static const Entry answers[] = {
        {LABEL_DRAMA, MSG_DRAMA_SIGH, 0, &talk_c::setDramaHeard},
        {LABEL_OWN, MSG_TOWN_HALL_MENU, 2, &talk_c::answerTownFund},
        {LABEL_OWN, MSG_TOWN_HALL_MENU_AGAIN, 2, &talk_c::answerTownFund},
        {LABEL_OWN, MSG_CONVERT_WINDMILL, 1, &talk_c::fundConversion},
        {LABEL_OWN, MSG_CONVERT_LIGHTHOUSE, 1, &talk_c::fundConversion},
        {LABEL_OWN, MSG_CONVERT_WINDMILL_VOTE, 0, &talk_c::voteWindmill},
        {LABEL_OWN, MSG_CONVERT_WINDMILL_VOTE, 1, &talk_c::voteLighthouse},
        {LABEL_OWN, MSG_CONVERT_LIGHTHOUSE_VOTE, 0, &talk_c::voteLighthouse},
        {LABEL_OWN, MSG_CONVERT_LIGHTHOUSE_VOTE, 1, &talk_c::voteWindmill},
        {LABEL_OWN, MSG_TOWER_SURVEY_FIRST, 0, &talk_c::voteLighthouse},
        {LABEL_OWN, MSG_TOWER_SURVEY_FIRST, 1, &talk_c::voteWindmill},
        {LABEL_OWN, MSG_TOWER_SURVEY, 0, &talk_c::voteLighthouse},
        {LABEL_OWN, MSG_TOWER_SURVEY, 1, &talk_c::voteWindmill},
        {LABEL_OWN, MSG_NO_MAIL, 0, &talk_c::answerSendMail},
        {LABEL_OWN, MSG_ANY_OTHER_LETTERS, 0, &talk_c::answerSendMail},
        {LABEL_OWN, MSG_POST_OFFICE_MENU, 0, &talk_c::answerSendMail},
        {LABEL_OWN, MSG_POST_OFFICE_MENU, 1, &talk_c::answerWiiBoard},
        {LABEL_OWN, MSG_POST_OFFICE_MENU_AGAIN, 0, &talk_c::answerSendMail},
        {LABEL_OWN, MSG_POST_OFFICE_MENU_AGAIN, 1, &talk_c::answerWiiBoard},
        {LABEL_OWN, MSG_POST_OFFICE_MENU2, 0, &talk_c::answerSendMail},
        {LABEL_OWN, MSG_POST_OFFICE_MENU2, 1, &talk_c::answerWiiBoard},
        {LABEL_OWN, MSG_POST_OFFICE_MENU_AGAIN2, 0, &talk_c::answerSendMail},
        {LABEL_OWN, MSG_POST_OFFICE_MENU_AGAIN2, 1, &talk_c::answerWiiBoard},
        {LABEL_OWN, MSG_WII_BOARD_FRIENDS, 0, &talk_c::answerOwnBoard},
        {LABEL_OWN, MSG_WII_BOARD_FRIENDS, 1, &talk_c::answerFriendBoard},
        {LABEL_OWN, MSG_WII_BOARD_OWN, 0, &talk_c::answerOwnBoard},
        {LABEL_OWN, MSG_WII_BOARD_RULES, 0, &talk_c::answerOwnBoard},
        {LABEL_OWN, MSG_WII_BOARD_RULES, 1, &talk_c::answerFriendBoard},
        {LABEL_OWN, MSG_WII_BOARD_OWN2, 0, &talk_c::answerOwnBoard},
        {LABEL_OWN, MSG_NO_MESSAGE, 0, &talk_c::answerSendMessage},
        {LABEL_OWN, MSG_NO_MESSAGE, 1, &talk_c::answerNoMessage},
        {LABEL_OWN, MSG_ADD_PHOTO, 0, &talk_c::answerAddPhoto},
        {LABEL_OWN, MSG_ADD_PHOTO, 1, &talk_c::answerNoPhoto},
        {LABEL_OWN, MSG_WHERE_SEND, 0, &talk_c::answerMailDestination},
        {LABEL_OWN, MSG_WHERE_SEND, 1, &talk_c::answerMailDestination},
        {LABEL_OWN, MSG_WHERE_SEND, 2, &talk_c::answerMailDestination},
        {LABEL_OWN, MSG_WHO_PROBLEM, 0, &talk_c::selectVillager},
        {LABEL_OWN, MSG_WHO_PROBLEM, 1, &talk_c::selectVillager},
        {LABEL_OWN, MSG_WHO_PROBLEM, 2, &talk_c::selectVillager},
        {LABEL_OWN, MSG_WHO_PROBLEM, 3, &talk_c::selectVillager},
        {LABEL_OWN, MSG_WHO_PROBLEM_MORE, 0, &talk_c::selectVillager},
        {LABEL_OWN, MSG_WHO_PROBLEM_MORE, 1, &talk_c::selectVillager},
        {LABEL_OWN, MSG_WHO_PROBLEM_MORE, 2, &talk_c::selectVillager},
        {LABEL_OWN, MSG_WHO_PROBLEM_MORE, 3, &talk_c::selectVillager},
    };
    u16 code = getMessageCode();
    const char *label = getMessageLabel();
    int cursor = mpController->mSelect.mCursor;
    for (u32 i = 0; i < ARRAY_SIZE(answers); i++) {
        if (std::strcmp(
                label,
                dANpcSpPeriko::l_msgLabels[answers[i].label][mpOwner->mType]) ==
                0 &&
            code == answers[i].code && cursor == answers[i].answer) {
            (this->*answers[i].method)();
        }
    }
}

// .text 0x4BB4
void dAcNpcSpPerikoBase_c::talk_c::setDramaHeard() {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
        PRIVATE_FLAG0_DRAMA_PELLY_HEARD);
}

// .text 0x4BDC
void dAcNpcSpPerikoBase_c::talk_c::selectVillager() {
    dAnimal_c *animal;
    int cursor = mpController->mSelect.mCursor;
    dAnmPersonalID_c *id;
    dAnimalBlock_c *animals = &dSaveData_c::getTown()->mAnimals.mTown;
    int num;
    int idx;
    int first = mVillagerPage;
    int i;
    num = 0;
    idx = 0;
    for (i = 0; i < ANIMAL_NUM; i++) {
        animal = animals->getAnimal(i);
        if (animal != NULL) {
            id = &animal->mID;
            if (id->isValid()) {
                if (idx >= first && num < VILLAGERS_PER_PAGE) {
                    if (num == cursor) {
                        setAnmPersonalName(id, 0);
                        mpVillager = animal;
                        return;
                    }
                    num++;
                }
                idx++;
            }
        }
    }
    if (num == cursor) {
        // "Someone else.": the next page, or back to the first.
        mVillagerPage += VILLAGERS_PER_PAGE;
        if (mVillagerPage >= idx) {
            mVillagerPage = 0;
        }
    }
}

// .text 0x4CC4
void dAcNpcSpPerikoBase_c::talk_c::answerSendMail() {
    if (!isPlayerFromThisTown()) {
        setNextMessage(MSG_VISITOR_LETTER, LABEL_OWN);
    } else {
        setNextMessage(MSG_WHERE_SEND, LABEL_OWN);
    }
}

// .text 0x4D1C
void dAcNpcSpPerikoBase_c::talk_c::answerWiiBoard() {
    if (!isPlayerFromThisTown()) {
        setNextMessage(MSG_VISITOR_NO_BOARD, LABEL_OWN);
    } else if (!dWifi::isLetterDayOver() && !dWifi::isLetterDayOver2()) {
        setNextMessage(MSG_WII_BOARD_DAY_OVER, LABEL_OWN);
    } else if (dSaveData_c::getRaw()->isFlag(SAVE_FLAG_BUS_NET_EVENT_DONE)) {
        setNextMessage(MSG_WII_BOARD_FRIENDS, LABEL_OWN);
    } else {
        setNextMessage(MSG_WII_BOARD_OWN, LABEL_OWN);
    }
}

// .text 0x4DC8
void dAcNpcSpPerikoBase_c::talk_c::answerOwnBoard() {
    if (!dWifi::isLetterDayOver2()) {
        setNextMessage(MSG_WII_BOARD_OTHERS_ONLY, LABEL_OWN);
    } else {
        reqMenu0F(0);
        mRequestEndMethod = &talk_c::endOwnBoardMenu;
    }
}

// .text 0x4E38
void dAcNpcSpPerikoBase_c::talk_c::answerNoMessage() {
    if (dWifi::hasRoster()) {
        dWifi::freeRoster();
    }
}

// .text 0x4E64
void dAcNpcSpPerikoBase_c::talk_c::answerFriendBoard() {
    if (fn_803AD87C()) {
        switchToSystemSpeaker();
        setNextMessage(NET_ERROR_MSG_RESTRICTED, LABEL_NET_ERROR);
        mNetErrorFrom = NET_ERROR_FROM_WII_BOARD;
    } else if (dWifi::getWc24State() != dWifi::WC24_STATE_ON) {
        switchToSystemSpeaker();
        setWc24ErrorMsg();
        mNetErrorFrom = NET_ERROR_FROM_WII_BOARD;
    } else if (!dSaveData_c::getRawExtra()->mHeader.mNetEnabled) {
        setNextMessage(MSG_WC24_OFF_BOARD, LABEL_OWN);
        mNetErrorFrom = NET_ERROR_FROM_WII_BOARD;
    } else if (!dWifi::isLetterDayOver()) {
        setNextMessage(MSG_WII_BOARD_OWN_ONLY, LABEL_OWN);
    } else {
        reqMenu0E(0);
        mRequestEndMethod = &talk_c::endFriendBoardMenu;
    }
}

// .text 0x4F5C
void dAcNpcSpPerikoBase_c::talk_c::answerSendMessage() {
    if (dWifi::hasRoster()) {
        reqMenu0E(0);
        mRequestEndMethod = &talk_c::endFriendBoardMenu;
    } else {
        reqMenu0F(0);
        mRequestEndMethod = &talk_c::endOwnBoardMenu;
    }
}

// .text 0x4FE4
void dAcNpcSpPerikoBase_c::talk_c::answerAddPhoto() {
    reqMenu10(0);
    mRequestEndMethod = &talk_c::endPhotoMenu;
}

// .text 0x5030
void dAcNpcSpPerikoBase_c::talk_c::answerNoPhoto() {
    dMenuMng_c::setupPost(FALSE);
}

// .text 0x5038
void dAcNpcSpPerikoBase_c::talk_c::answerMailDestination() {
    int cursor = mpController->mSelect.mCursor;
    if (dSaveData_c::getRaw()->isFlag(SAVE_FLAG_BUS_NET_EVENT_DONE)) {
        switch (cursor) {
        case 0:
            startTownMail();
            break;
        case 1:
            startOtherTownMail();
            break;
        case 2:
            startFutureMail();
            break;
        }
    } else {
        switch (cursor) {
        case 0:
            startTownMail();
            break;
        case 1:
            startFutureMail();
            break;
        }
    }
}

// .text 0x50F8
void dAcNpcSpPerikoBase_c::talk_c::startTownMail() {
    if (dPostOffice::checkFull()) {
        setNextMessage(MSG_POST_OFFICE_FULL, LABEL_OWN);
    } else {
        setNextMessage(MSG_MAIL_SEND, LABEL_OWN);
        mMailKind = MAIL_TOWN;
    }
}

// .text 0x5158
void dAcNpcSpPerikoBase_c::talk_c::startOtherTownMail() {
    if (fn_803AD87C()) {
        switchToSystemSpeaker();
        setNextMessage(NET_ERROR_MSG_RESTRICTED, LABEL_NET_ERROR);
        mNetErrorFrom = NET_ERROR_FROM_MAIL;
    } else if (dWifi::getWc24State() != dWifi::WC24_STATE_ON) {
        switchToSystemSpeaker();
        setWc24ErrorMsg();
        mNetErrorFrom = NET_ERROR_FROM_MAIL;
    } else if (!dSaveData_c::getRawExtra()->mHeader.mNetEnabled) {
        setNextMessage(MSG_WC24_OFF_MAIL, LABEL_OWN);
        mNetErrorFrom = NET_ERROR_FROM_MAIL;
    } else if (dPostOffice::checkFull()) {
        setNextMessage(MSG_POST_OFFICE_FULL, LABEL_OWN);
    } else if (dPostOffice::isOtherTownMailFull()) {
        setNextMessage(MSG_OTHER_TOWN_ROUTES_FULL, LABEL_OWN);
    } else {
        setNextMessage(MSG_OTHER_TOWN_MAIL, LABEL_OWN);
        mMailKind = MAIL_OTHER_TOWN;
    }
}

// .text 0x5260
void dAcNpcSpPerikoBase_c::talk_c::startFutureMail() {
    setNextMessage(MSG_MAIL_SEND, LABEL_OWN);
    mMailKind = MAIL_FUTURE;
}

// .text 0x529C
void dAcNpcSpPerikoBase_c::talk_c::answerTownFund() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int msg;
    if (hasLighthouse() || hasWindmill()) {
        if (dSaveData_c::getRaw()->isFlag(SAVE_FLAG_PUBLIC_WORKS_VOTE)) {
            if (player->isFlag0(PRIVATE_FLAG0_PUBLIC_WORKS_VOTED)) {
                if (hasLighthouse()) {
                    msg = MSG_CONVERTING_LIGHTHOUSE;
                } else {
                    msg = MSG_CONVERTING_WINDMILL;
                }
            } else {
                startVote();
                if (hasLighthouse()) {
                    msg = MSG_CONVERT_LIGHTHOUSE_VOTE;
                } else {
                    msg = MSG_CONVERT_WINDMILL_VOTE;
                }
            }
        } else if (dSaveData_c::getTown()->mPublicWorksBells >= 1000000) {
            if (hasWindmill()) {
                msg = MSG_CONVERT_WINDMILL;
            } else {
                msg = MSG_CONVERT_LIGHTHOUSE;
            }
        } else {
            msg = getFundMsg();
        }
    } else if (dSaveData_c::getRaw()->isFlag(SAVE_FLAG_PUBLIC_WORKS_VOTE)) {
        if (dSaveTown_c::getPublicWorks() == PUBLIC_WORKS_NONE) {
            if (player->isFlag0(PRIVATE_FLAG0_PUBLIC_WORKS_VOTED)) {
                msg = MSG_BRIDGE_BUILDING;
            } else {
                startVote();
                BOOL heard = player->isFlag0(PRIVATE_FLAG0_BRIDGE_SURVEY_HEARD);
                player->setFlag0(PRIVATE_FLAG0_BRIDGE_SURVEY_HEARD);
                if (heard) {
                    msg = MSG_BRIDGE_SURVEY;
                } else {
                    msg = MSG_BRIDGE_SURVEY_FIRST;
                }
            }
        } else if (dSaveTown_c::getPublicWorks() == PUBLIC_WORKS_BRIDGE) {
            startVote();
            msg = MSG_FOUNTAIN_BUILDING;
        } else if (player->isFlag0(PRIVATE_FLAG0_PUBLIC_WORKS_VOTED)) {
            msg = MSG_TOWER_BUILDING;
        } else {
            startVote();
            BOOL heard = player->isFlag0(PRIVATE_FLAG0_TOWER_SURVEY_HEARD);
            player->setFlag0(PRIVATE_FLAG0_TOWER_SURVEY_HEARD);
            if (heard) {
                msg = MSG_TOWER_SURVEY;
            } else {
                msg = MSG_TOWER_SURVEY_FIRST;
            }
        }
    } else {
        msg = getFundMsg();
    }
    setNextMessage(msg, LABEL_OWN);
}

// .text 0x5530
void dAcNpcSpPerikoBase_c::talk_c::fundConversion() {
    dSaveData_c::getTown()->setFlag(SAVE_FLAG_PUBLIC_WORKS_FUNDED);
}

// .text 0x5558
void dAcNpcSpPerikoBase_c::talk_c::voteWindmill() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    player->setFlag0(PRIVATE_FLAG0_PUBLIC_WORKS_VOTED);
    player->mBuildingVote = dPrivateData_c::BUILDING_VOTE_WINDMILL;
}

// .text 0x5598
void dAcNpcSpPerikoBase_c::talk_c::voteLighthouse() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    player->setFlag0(PRIVATE_FLAG0_PUBLIC_WORKS_VOTED);
    player->mBuildingVote = dPrivateData_c::BUILDING_VOTE_LIGHTHOUSE;
}

// .text 0x55D8
int dAcNpcSpPerikoBase_c::talk_c::getFundMsg() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    BOOL explained = player->isFlag0(PRIVATE_FLAG0_FUND_EXPLAINED);
    player->setFlag0(PRIVATE_FLAG0_FUND_EXPLAINED);
    return MSG_FUND_FIRST + (explained != 0);
}

// .text 0x5634
void dAcNpcSpPerikoBase_c::talk_c::onCustomTag(int no) {
    static const Method tags[] = {
        &talk_c::openTownTuneMenu,   &talk_c::openFundMenu,
        &talk_c::openMailSaveMenu,   &talk_c::openMailMenu,
        &talk_c::openFutureDateMenu,
    };
    (this->*tags[no])();
}

// .text 0x5668
void dAcNpcSpPerikoBase_c::talk_c::onRequestEnd(int kind) {
    if (mRequestEndMethod) {
        Method method = mRequestEndMethod;
        callMessageMethod(method);
        if (method == mRequestEndMethod) {
            mRequestEndMethod = NULL;
        }
    }
}

// .text 0x5700
void dAcNpcSpPerikoBase_c::talk_c::endHouseMap() {
}

// .text 0x5704
void dAcNpcSpPerikoBase_c::talk_c::endFund() {
    if (lbl_8074EA7D != 1) {
        setNextMessage(MSG_FUND_DECLINED, LABEL_OWN);
        reqMsgClose();
    } else {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
            PRIVATE_FLAG0_FUND_DONATED);
        dItem::Item bells(dItem::ITEM_IDX_99000_BELLS); // the money bag
        requestItemActEx(6, &bells, 0, 1, 0, 2);
        mRequestEndMethod = &talk_c::closeMessage;
        // The Bells each public work needs.
        static const int costs[] = {200000, 400000, 1000000, 1000000};
        int works = dSaveTown_c::getPublicWorks();
        if (!dSaveData_c::getRaw()->isFlag(SAVE_FLAG_PUBLIC_WORKS_FUNDED) &&
            dSaveData_c::getTown()->mPublicWorksBells >= costs[works]) {
            if (works == PUBLIC_WORKS_BRIDGE) {
                dSaveData_c::getTown()->setFlag(SAVE_FLAG_PUBLIC_WORKS_FUNDED);
            } else if (works != PUBLIC_WORKS_TOWER) {
                dSaveData_c::getTown()->setFlag(SAVE_FLAG_PUBLIC_WORKS_FUNDED);
            }
        }
        int bellsGiven = fn_80029B20();
        int msg;
        if (sLib::isInRange(bellsGiven, 1, 999) == 1) {
            msg = MSG_FUND_THANKS;
        } else {
            // 0x2F = MSG_FUND_COOPERATION; the literal keeps the ternary an
            // int (an enum on both sides changes the register allocation).
            msg = sLib::isInRange(bellsGiven, 1000, 49999) == 1 ? 0x2F
                                                                : MSG_FUND_PAID;
        }
        setNextMessage(msg, LABEL_OWN);
    }
}

// .text 0x5878
void dAcNpcSpPerikoBase_c::talk_c::endTownTuneMenu() {
    if (lbl_8074EA7D == 1) {
        setNextMessage(MSG_TOWN_TUNE_SAVED, LABEL_OWN);
    } else {
        setNextMessage(MSG_TOWN_TUNE_GIVE_UP, LABEL_OWN);
    }
}

// .text 0x58A0
void dAcNpcSpPerikoBase_c::talk_c::endMailSaveMenu() {
    if (lbl_8074EA7D == 1) {
        setNextMessage(MSG_MAIL_SAVED, LABEL_OWN);
    } else {
        setNextMessage(MSG_MAIL_SAVE_CANCELLED, LABEL_OWN);
    }
}

// .text 0x58C8
void dAcNpcSpPerikoBase_c::talk_c::setFutureDateWords() {
    dTime_c time = dPostOffice::getFutureLetterTime();
    setNumber4(time.year, 1);
    setMonthName(time.month, 3);
    setDayName((u8)time.mday, 4);
}

// .text 0x5980
void dAcNpcSpPerikoBase_c::talk_c::closeMessage() {
    reqMsgClose();
}

// .text 0x5984
void dAcNpcSpPerikoBase_c::talk_c::endTownMailMenu() {
    if (lbl_8074EA7D != 1) {
        setNextMessage(MSG_NO_MAIL, LABEL_OWN);
        reqMsgClose();
    } else {
        dItem::Item letters(dItem::ITEM_IDX_LETTER);
        requestItemActEx(6, &letters, 0, 1, 0, 2);
        mRequestEndMethod = &talk_c::closeMessage;
        if (isMenuSel01()) {
            setNextMessage(MSG_NOT_RESIDENT_RETURNED, LABEL_OWN);
        } else {
            showFullMailboxes();
        }
    }
}

// .text 0x5A44
void dAcNpcSpPerikoBase_c::talk_c::endOtherTownMailMenu() {
    if (lbl_8074EA7D != 1) {
        setNextMessage(MSG_NO_MAIL, LABEL_OWN);
        reqMsgClose();
    } else {
        dItem::Item letters(dItem::ITEM_IDX_LETTER);
        requestItemActEx(6, &letters, 0, 1, 0, 2);
        mRequestEndMethod = &talk_c::closeMessage;
        if (isMenuSel80()) {
            setNextMessage(MSG_FRIENDS_ONLY, LABEL_OWN);
        } else {
            setNextMessage(MSG_OTHER_TOWN_MAIL_TAKEN, LABEL_OWN);
        }
    }
}

// .text 0x5B0C
void dAcNpcSpPerikoBase_c::talk_c::endFutureMailMenu() {
    if (lbl_8074EA7D != 1) {
        setNextMessage(MSG_NO_MAIL, LABEL_OWN);
        reqMsgClose();
    } else {
        dItem::Item letters(dItem::ITEM_IDX_LETTER);
        requestItemActEx(6, &letters, 0, 1, 0, 2);
        mRequestEndMethod = &talk_c::closeMessage;
        if (dPostOffice::hasFutureLetter()) {
            setNextMessage(MSG_FUTURE_LIMIT, LABEL_OWN);
            dTime_c time = dPostOffice::getFutureLetterTime();
            setNumber4(time.year, 1);
            setMonthName(time.month, 3);
            setDayName((u8)time.mday, 4);
        } else {
            setNextMessage(MSG_FUTURE_WHEN, LABEL_OWN);
        }
    }
}

// .text 0x5C64
void dAcNpcSpPerikoBase_c::talk_c::endFriendBoardMenu() {
    if (lbl_8074EA7D == 1) {
        if (dMenuMng_c::isPhotoCaptured()) {
            setNextMessage(MSG_ADD_PHOTO, LABEL_OWN);
        } else {
            dMenuMng_c::setupPost(FALSE);
            setNextMessage(MSG_PREPARE_BOARD_POST, LABEL_OWN);
        }
    } else if (dMenuMng_c::isNetError()) {
        mpOwner->mTalk.mNoMouth = 1;
        clearSpeakerName();
        setNetErrorMsg();
        mNetErrorFrom = NET_ERROR_FROM_WII_BOARD;
    } else {
        setNextMessage(MSG_NO_MESSAGE, LABEL_OWN);
    }
}

// .text 0x5D20
void dAcNpcSpPerikoBase_c::talk_c::endOwnBoardMenu() {
    if (lbl_8074EA7D == 1) {
        if (dMenuMng_c::isPhotoCaptured()) {
            setNextMessage(MSG_ADD_PHOTO, LABEL_OWN);
        } else {
            dMenuMng_c::setupPost(FALSE);
            setNextMessage(MSG_PREPARE_BOARD_POST, LABEL_OWN);
        }
    } else {
        setNextMessage(MSG_NO_MESSAGE, LABEL_OWN);
    }
}

// .text 0x5DA0
void dAcNpcSpPerikoBase_c::talk_c::endPhotoMenu() {
    if (lbl_8074EA7D == 1) {
        dMenuMng_c::setupPost(TRUE);
    } else {
        dMenuMng_c::setupPost(FALSE);
    }
    setNextMessage(MSG_PREPARE_BOARD_POST, LABEL_OWN);
}

// .text 0x5DFC
void dAcNpcSpPerikoBase_c::talk_c::endBridgeMap() {
    u32 voteZ;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (lbl_8074EA7D == 1) {
        player->setFlag0(PRIVATE_FLAG0_PUBLIC_WORKS_VOTED);
        voteZ = getMenuPosZ();
        player->mBridgeVoteX = getMenuPosX();
        player->mBridgeVoteY = voteZ;
        setNextMessage(MSG_BRIDGE_VOTED, LABEL_OWN);
    } else {
        setNextMessage(MSG_BRIDGE_PASS, LABEL_OWN);
    }
}

// .text 0x5E90
void dAcNpcSpPerikoBase_c::talk_c::initWait() {
}

// .text 0x5E94
void dAcNpcSpPerikoBase_c::talk_c::executeWait() {
}

// .text 0x5E98
void dAcNpcSpPerikoBase_c::talk_c::initWiiBoardPost() {
}

// .text 0x5E9C
void dAcNpcSpPerikoBase_c::talk_c::executeWiiBoardPost() {
    if (dMenuMng_c::sendPost()) {
        changeMode(&talk_c::executeWiiBoardPostDone);
    }
}

// .text 0x6004
void dAcNpcSpPerikoBase_c::talk_c::initWiiBoardPostDone() {
}

// .text 0x6008
void dAcNpcSpPerikoBase_c::talk_c::executeWiiBoardPostDone() {
    switch (dWifi::getThreadResult()) {
    case dWifi::THREAD_RESULT_OK: {
        if (dWifi::hasRoster()) {
            dWifi::freeRoster();
        }
        dMenuMng_c::offFlag(2);
        setNextMessage(MSG_BOARD_POSTED, LABEL_OWN);
        dMsg::Comp_c *controller = mpController;
        controller->mLock = 0;
        controller->mWaitIconSe = 0;
        controller->mWaitIcon = 0;
        changeMode(&talk_c::executeWait);
        break;
    }
    case dWifi::THREAD_RESULT_FAILED: {
        if (dWifi::hasRoster()) {
            dWifi::freeRoster();
        }
        dMenuMng_c::offFlag(2);
        switchToSystemSpeaker();
        setNetErrorMsg();
        mNetErrorFrom = NET_ERROR_FROM_BOARD_POST;
        dMsg::Comp_c *controller = mpController;
        controller->mLock = 0;
        controller->mWaitIconSe = 0;
        controller->mWaitIcon = 0;
        changeMode(&talk_c::executeWait);
        break;
    }
    }
}

// .text 0x6328
void dAcNpcSpPerikoBase_c::talk_c::initPostOfficeCheck() {
    if (isNetActive()) {
        dPostOffice::resetSync();
    }
}

// .text 0x6354
void dAcNpcSpPerikoBase_c::talk_c::executePostOfficeCheck() {
    BOOL full = FALSE;
    BOOL done = FALSE;
    if (isNetActive()) {
        switch (dPostOffice::getSyncResult()) {
        case 2:
            full = TRUE;
            done = TRUE;
            break;
        case 3:
            full = FALSE;
            done = TRUE;
            break;
        }
    } else {
        done = TRUE;
        full = dPostOffice::checkFull() != 0;
    }
    if (done) {
        dMsg::Comp_c *controller = mpController;
        controller->mLock = 0;
        controller->mWaitIconSe = 0;
        controller->mWaitIcon = 0;
        if (full) {
            setNextMessage(MSG_POST_OFFICE_FULL, LABEL_OWN);
        } else {
            mMailKind = MAIL_TOWN;
            setNextMessage(MSG_VISITOR_TOWN_ONLY, LABEL_OWN);
        }
        changeMode(&talk_c::executeWait);
    }
}

// .text 0x655C
BOOL dAcNpcSpPerikoBase_c::isDramaDue() {
    return mDramaDue;
}

// .text 0x6564
void dAcNpcSpPerikoBase_c::clearDrama() {
    mDramaDue = 0;
}

// .text 0x6570
int dAcNpcSpPerikoBase_c::getEarType() {
    BOOL pelly = isPelly();
    int type = NPC_EAR_TYPE_PHYLLIS;
    if (pelly) {
        type = NPC_EAR_TYPE_PELLY;
    }
    return type;
}

// .text 0x65A0
void dAcNpcSpPerikoBase_c::talk_c::setWc24ErrorMsg() {
    int error = dWifi::getErrorMessage();
    int code = dWifi::getErrorCode();
    u16 msg = NET_ERROR_MSG_WC24_OTHER;
    if (error <= 9) {
        msg = error + NET_ERROR_MSG_WC24_NAND;
    }
    setNextMessage(msg, LABEL_NET_ERROR);
    mpController->formatNumber(0, code, 6, dScript::NUM_FORMAT_ZERO_PAD);
}

// .text 0x6620
void dAcNpcSpPerikoBase_c::talk_c::setNetErrorMsg() {
    int error = dWifi::getNetErrorMessage();
    int code = dWifi::getNetErrorCode();
    if (error == SV_MSG_NAND_DAMAGED) {
        setNextMessage(SV_MSG_NAND_DAMAGED, LABEL_SAVE);
    } else {
        u16 msg = NET_ERROR_MSG_WC24_OTHER;
        if (error <= 9) {
            msg = error + NET_ERROR_MSG_WC24_NAND;
        }
        setNextMessage(msg, LABEL_NET_ERROR);
        mpController->formatNumber(0, code, 6, dScript::NUM_FORMAT_ZERO_PAD);
    }
}

// .text 0x66BC
void dAcNpcSpPerikoBase_c::talk_c::switchToSystemSpeaker() {
    dString::Word_c speaker((const wchar_t *)getSpeakerName());
    dHmnName::Word_c name;
    if (isPelly()) {
        dItem::Item item((u16)SP_NPC_POST_GIRL);
        dNpc::getNpcName(&name, &item, LANGUAGE_NUM);
    } else {
        dItem::Item item((u16)SP_NPC_POST_GIRL2);
        dNpc::getNpcName(&name, &item, LANGUAGE_NUM);
    }
    if (name.isSame(&speaker)) {
        setRequest1(0);
        mRequestEndMethod = &talk_c::hideSpeaker;
    }
}

// .text 0x6798
void dAcNpcSpPerikoBase_c::talk_c::switchToOwnSpeaker() {
    dString::Word_c speaker((const wchar_t *)getSpeakerName());
    dHmnName::Word_c name;
    if (isPelly()) {
        dItem::Item item((u16)SP_NPC_POST_GIRL);
        dNpc::getNpcName(&name, &item, LANGUAGE_NUM);
    } else {
        dItem::Item item((u16)SP_NPC_POST_GIRL2);
        dNpc::getNpcName(&name, &item, LANGUAGE_NUM);
    }
    if (!name.isSame(&speaker)) {
        setRequest1(0);
        mRequestEndMethod = &talk_c::showSpeaker;
    }
}

// .text 0x6874
void dAcNpcSpPerikoBase_c::talk_c::hideSpeaker() {
    mpOwner->mTalk.mNoMouth = 1;
    clearSpeakerName();
    reqMsgClose();
}

// .text 0x68B4
void dAcNpcSpPerikoBase_c::talk_c::showSpeaker() {
    mpOwner->mTalk.mNoMouth = 0;
    dHmnName::Word_c name;
    if (isPelly()) {
        dItem::Item item((u16)SP_NPC_POST_GIRL);
        dNpc::getNpcName(&name, &item, LANGUAGE_NUM);
    } else {
        dItem::Item item((u16)SP_NPC_POST_GIRL2);
        dNpc::getNpcName(&name, &item, LANGUAGE_NUM);
    }
    setSpeakerName(
        (const u16 *)static_cast<dScript::Word_c &>(name).getBuffer(), 2);
    reqMsgClose();
}
