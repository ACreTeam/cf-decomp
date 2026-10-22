#include <game/game/d_a_npc_sp_tanukichi.hpp>
#include <game/framework/f_manager.hpp>
#include <game/framework/f_profile.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_bg_util.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_bgi_draw.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_letter.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_def.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_shop_layout.hpp>
#include <game/sLib/s_lib.hpp>

// Tom Nook and the twins. Functions are in target .text order.

namespace dANpcSpTanukichi {
// .text 0xCC
static dScene::dSceneChange_c *getSceneChange() {
    return &dScene::gSceneChange;
}

// countPockets filters.
// .text 0xD8
static BOOL isWorkClothes(const dItem::Item *item) {
    return item->isSame(dItem::Item((int)dItem::ITEM_IDX_WORK_UNIFORM));
}

// .text 0x118: seed bags and saplings (the job's planting)
static BOOL isPlantItem(const dItem::Item *item) {
    BOOL ret = FALSE;
    if (item->getKind() == dItem::KIND_SEED ||
        item->getKind() == dItem::KIND_SEEDLING) {
        ret = TRUE;
    }
    return ret;
}

// .text 0x170
static BOOL isPaper(const dItem::Item *item) {
    return item->getKind() == dItem::KIND_PAPER;
}

// .text 0x19C: the tools with a how-to message
static BOOL isBasicTool(const dItem::Item *item) {
    switch (item->getKind()) {
    case dItem::KIND_TIMER:
    case dItem::KIND_FISHINGROD:
    case dItem::KIND_SCOOP:
    case dItem::KIND_AXE:
    case dItem::KIND_WATERING:
    case dItem::KIND_NET:
    case dItem::KIND_PACHINKO:
        return TRUE;
    }
    return FALSE;
}

// .text 0x1EC: the silver tools the store sells
static BOOL isSilverTool(const dItem::Item *item) {
    switch (item->getKind()) {
    case dItem::KIND_SILVER_FISHINGROD:
    case dItem::KIND_SILVER_NET:
    case dItem::KIND_SILVER_PACHINKO:
        return TRUE;
    }
    return FALSE;
}

// .text 0x23C
static BOOL isTool(const dItem::Item *item) {
    if (isBasicTool(item) || isSilverTool(item)) {
        return TRUE;
    }
    return FALSE;
}

} // namespace dANpcSpTanukichi

// .text 0x584: emitted after its first user (the Mamekichi profile)
inline dAcNpcSpTubuMame_c::~dAcNpcSpTubuMame_c() {
}

namespace dANpcSpTanukichi {
// .text 0x28C
CUSTOM_BASE_PROFILE(NPC_SP_TANUKICHI, dAcNpcSpTanukichi_c,
                    fProf::A_NPC_SP_TANUKICHI_05E,
                    fProf::DRAW_ORDER::A_NPC_SP_TANUKICHI_05E);
// .text 0x408
CUSTOM_BASE_PROFILE(NPC_SP_MAMEKICHI, dAcNpcSpMamekichi_c,
                    fProf::A_NPC_SP_TANUKICHI_05F,
                    fProf::DRAW_ORDER::A_NPC_SP_TANUKICHI_05F);
// .text 0x6BC
CUSTOM_BASE_PROFILE(NPC_SP_TUBUKICHI, dAcNpcSpTubukichi_c,
                    fProf::A_NPC_SP_TANUKICHI_060,
                    fProf::DRAW_ORDER::A_NPC_SP_TANUKICHI_060);

// The Nookington's vote of the current player.
static inline void setShopVote(u8 vote) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    player->mShopVote = vote;
}

// How many more Bells the player can carry.
static inline int getBellsRoom(dPrivateData_c *player) {
    return PRIVATE_BELLS_MAX - player->mBells;
}

// The current player's house.
static inline dHome_c *getPlayerHome() {
    dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
    return homes->getHome(homes->findCurrentPlayer());
}
static inline int getPlayerRoofColor() {
    dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
    return homes->getHome(homes->findCurrentPlayer())->mNextRoofColor;
}

// The player has the money (signed >= as a value: subfc / adde).
static inline BOOL canAfford(int price) {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->getPocketMoney() >= price;
}

// The literals become anonymous .data temporaries.
static inline void setupMail(dMail_c *mail, const u16 *kind, const u8 &sender,
                             dPrivateData_c *to, const int &paper) {
    mail->setupSystem(kind, "MAIL_NPC_Tanukichi", &sender, &to->mPID, &paper);
}

// Unreferenced static of the unit (.bss 0x8, built by __sinit).
struct unk_s {
    unk_s() : _0(0), _4(0), _8(0) {
    }
    u32 _0;
    u32 _4;
    u8 _8;
};
static unk_s l_unk;
} // namespace dANpcSpTanukichi

using namespace dANpcSpTanukichi;

// .text 0x838
void dAcNpcSpTanukiBase_c::onDemoCmd(int cmd, u8 member) {
    _1B70 = member;
    int permit = requestPermit(cmd, member, TRUE);
    switch (cmd) {
    case DEMO_CMD_START:
        if (permit == PERMIT_REMOTE_START || permit == PERMIT_LOCAL_START) {
            changeState(STATE_LOOK);
        }
        break;
    case DEMO_CMD_TALK:
        if (!isMultiPlay() || member == NET_MEMBER_NONE) {
            mBaseTalk.mKind = talk_c::KIND_TALK;
        }
        if (permit == PERMIT_REMOTE_TALK) {
            changeState(STATE_REMOTE_TALK);
        } else if (permit == PERMIT_LOCAL_TALK) {
            mBaseTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        }
        break;
    case DEMO_CMD_SPEAK:
        if (isMultiPlay() && member == NET_MEMBER_NONE) {
            mBaseTalk.mKind = talk_c::KIND_ITEM;
        }
        if (permit == PERMIT_REMOTE_TALK) {
            changeState(STATE_REMOTE_TALK);
        } else if (permit == PERMIT_LOCAL_TALK) {
            mBaseTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        }
        break;
    case DEMO_CMD_END:
        if (permit == PERMIT_END_SINGLE) {
            if (isMultiPlay()) {
                changeState(STATE_MULTI_WAIT);
            } else {
                changeState(STATE_WAIT);
            }
        } else if (permit == PERMIT_END_MULTI) {
            changeState(STATE_REMOTE);
        }
        break;
    case CMD_RELEASE_PERMIT:
        if (permit == PERMIT_RELEASED) {
            if (isMultiPlay()) {
                changeState(STATE_MULTI_WAIT);
            } else {
                changeState(STATE_WAIT);
            }
        }
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpTanukichi {
static const dAcNpcSpTanukiBase_c::State_s l_states[] = {
    {&dAcNpcSpTanukiBase_c::initWait, &dAcNpcSpTanukiBase_c::executeWait},
    {&dAcNpcSpTanukiBase_c::initTurn, &dAcNpcSpTanukiBase_c::executeTurn},
    {&dAcNpcSpTanukiBase_c::initApproach,
     &dAcNpcSpTanukiBase_c::executeApproach},
    {&dAcNpcSpTanukiBase_c::initLook, &dAcNpcSpTanukiBase_c::executeLook},
    {&dAcNpcSpTanukiBase_c::initTalk, &dAcNpcSpTanukiBase_c::executeTalk},
    {&dAcNpcSpTanukiBase_c::initGreet, &dAcNpcSpTanukiBase_c::executeGreet},
    {&dAcNpcSpTanukiBase_c::initSeeOff, &dAcNpcSpTanukiBase_c::executeSeeOff},
    {&dAcNpcSpTanukiBase_c::initClosing, &dAcNpcSpTanukiBase_c::executeClosing},
    {&dAcNpcSpTanukiBase_c::initMovePlayer,
     &dAcNpcSpTanukiBase_c::executeMovePlayer},
    {&dAcNpcSpTanukiBase_c::initWatch, &dAcNpcSpTanukiBase_c::executeWatch},
    {&dAcNpcSpTanukiBase_c::initMultiWait,
     &dAcNpcSpTanukiBase_c::executeMultiWait},
    {&dAcNpcSpTanukiBase_c::initRemote, &dAcNpcSpTanukiBase_c::executeRemote},
    {&dAcNpcSpTanukiBase_c::initRemoteTalk,
     &dAcNpcSpTanukiBase_c::executeRemoteTalk},
};
} // namespace dANpcSpTanukichi

// .text 0xA14
void dAcNpcSpTanukiBase_c::changeState(int state) {
    mState = state;
    (this->*l_states[state].mInit)();
}

// .text 0xA4C
int dAcNpcSpTanukiBase_c::preCreate() {
    switch (dScene::getCurrentScene()) {
    case dScene::SCENE_RM_SHOP1:
        mParam = SP_NPC_CONV_MASTER;
        break;
    case dScene::SCENE_RM_SHOP2:
        mParam = SP_NPC_SUPER_MASTER;
        break;
    case dScene::SCENE_RM_SHOP3_F1:
        mParam = SP_NPC_DEPART_MASTER;
        break;
    }
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mBaseTalk);
    return 1;
}

// .text 0xAF4
int dAcNpcSpTanukiBase_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    for (int i = 0; i < STATE_NUM; i++) {
        mStep[i] = STEP_START;
    }
    if (isTanukichi() && isPlayerFromThisTown() && !isMultiPlay() &&
        !dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
            PRIVATE_FLAG0_DRAMA_NOOK_HEARD) &&
        (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getGracieStep() == 3 ||
         dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getGracieStep() == 5)) {
        mDrama = 1;
    } else {
        mDrama = 0;
    }
    mApproached = 0;
    if (isMultiPlay()) {
        if (isLocalOwner()) {
            changeState(STATE_MULTI_WAIT);
        } else {
            changeState(STATE_REMOTE);
        }
        switch (dScene::getCurrentScene()) {
        case dScene::SCENE_RM_SHOP0:
            mPos.set(240.0f, 0.0f, 400.0f);
            break;
        case dScene::SCENE_RM_SHOP1:
            mPos.set(304.0f, 0.0f, 368.0f);
            break;
        case dScene::SCENE_RM_SHOP2:
            mPos.set(240.0f, 0.0f, 336.0f);
            break;
        case dScene::SCENE_RM_SHOP3_F1:
            mPos.set(272.0f, 0.0f, 304.0f);
            break;
        case dScene::SCENE_RM_SHOP3_F2:
            if (isTubukichi()) {
                mPos.set(240.0f, 0.0f, 272.0f);
            } else {
                mPos.set(240.0f, 0.0f, 304.0f);
            }
            break;
        }
        mLastPos = mPos;
    } else {
        mObjc.setDefWeight(objc_c::WEIGHT_DEFAULT);
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_PART_TIME_JOB)) {
            dBGCF::lockNext(1);
            dQuestErrandList_c *errand =
                &dPlayerMgr::Mgr_c::getCurrentPlayer()->mErrand;
            if ((!errand->isActive() ||
                 errand->getKind() == QUEST_KIND_FIRSTJOB_START) &&
                (!dNpcInfo::tanukichi_c::get()->isAllVillagersMet() ||
                 !dPlayerMgr::Mgr_c::getCurrentPlayer()->mErrand.isState(1))) {
                changeState(STATE_GREET);
            } else {
                changeState(STATE_WAIT);
            }
        } else if (isMamekichi()) {
            changeState(STATE_WATCH);
            mSoundPlayed = 1;
        } else if (isTanukichi() &&
                   dScene::getPrevScene() != dScene::SCENE_FIELD) {
            dBGCF::lockNext(0);
            changeState(STATE_WAIT);
            mSoundPlayed = 1;
        } else if (isTubukichi() &&
                   dScene::getPrevScene() != dScene::SCENE_RM_SHOP3_F1) {
            dBGCF::lockNext(0);
            changeState(STATE_WAIT);
            mSoundPlayed = 1;
        } else {
            dBGCF::lockNext(0);
            changeState(STATE_GREET);
        }
    }
    if (isTwin() && dNpcInfo::tanukichi_c::get()->mTwinsGreeted) {
        mSoundPlayed = 1;
    }
    return 1;
}

// .text 0xEF0
int dAcNpcSpTanukiBase_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0xF1C
int dAcNpcSpTanukiBase_c::vtB0() {
    if (mState < STATE_NUM) {
        (this->*l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0xF64
int dAcNpcSpTanukiBase_c::canSpeak(dDemoActor_c *actor) {
    if (isInUse(NULL)) {
        return 0;
    }
    return !mTalk.isBusy();
}

// .text 0xFB4
void dAcNpcSpTanukiBase_c::initWait() {
    if (isTwin()) {
        dAcNpcSpTanukiBase_c *partner =
            (dAcNpcSpTanukiBase_c *)fManager_c::searchBaseByProfName(
                getPartnerProfName(), NULL);
        if (partner->mState == STATE_WATCH) {
            partner->changeState(STATE_WAIT);
        }
    }
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
}

// .text 0x103C
void dAcNpcSpTanukiBase_c::executeWait() {
    if (dPlayerMgr::isSceneTransitionState()) {
        return;
    }
    mVec3_c step =
        getRouteStep(dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT));
    if (!mDrama) {
        f32 moveDist = getMoveDist();
        if (calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL) >
                moveDist &&
            step != mPos) {
            changeState(STATE_APPROACH);
        } else {
            if (isFacingAway()) {
                changeState(STATE_TURN);
            } else if (isNotFacing()) {
                changeState(STATE_TURN);
            }
        }
    }
    if (isWorkerAtExit()) {
        changeState(STATE_SEE_OFF);
    } else if (isSeeOffTime()) {
        changeState(STATE_SEE_OFF);
    } else if (isClosingTime()) {
        changeState(STATE_CLOSING);
    } else if (checkPointedItem()) {
        mBaseTalk.mKind = talk_c::KIND_ITEM;
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0x124C
void dAcNpcSpTanukiBase_c::initTurn() {
    mApproached = 1;
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_IDLE, angle, l_turnSpeed, 0, cNpcMorphFrames);
}

// .text 0x12B0
void dAcNpcSpTanukiBase_c::executeTurn() {
    if (isWorkerAtExit()) {
        changeState(STATE_SEE_OFF);
    } else if (isSeeOffTime()) {
        changeState(STATE_SEE_OFF);
    } else if (isClosingTime()) {
        changeState(STATE_CLOSING);
    } else if (checkPointedItem()) {
        mBaseTalk.mKind = talk_c::KIND_ITEM;
        dDemo::requestSpeak(this, NULL);
    } else {
        mVec3_c playerPos =
            *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
        mVec3_c step = getRouteStep(&playerPos);
        if (dPlayerMgr::isSceneTransitionState()) {
            changeState(STATE_WAIT);
        } else {
            f32 moveDist = getMoveDist();
            if (calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL) >
                    moveDist &&
                step != mPos) {
                changeState(STATE_APPROACH);
            } else if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
                changeState(STATE_WAIT);
            } else {
                mAction.setTargetAngle(getAngleYToPlayer(_1B70));
            }
        }
    }
}

// .text 0x1480
void dAcNpcSpTanukiBase_c::initApproach() {
    mVec3_c pos = *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    mAction.requestWalkFacing(PRIORITY_IDLE, pos, l_walkTurnSpeed, 0,
                              l_moveParamWalk, l_defaultAnmRate,
                              cNpcMorphFrames);
    mApproached = 1;
}

// .text 0x1504
void dAcNpcSpTanukiBase_c::executeApproach() {
    if (dPlayerMgr::isSceneTransitionState()) {
        changeState(STATE_WAIT);
    } else if (isWorkerAtExit()) {
        changeState(STATE_SEE_OFF);
    } else if (isSeeOffTime()) {
        changeState(STATE_SEE_OFF);
    } else if (isClosingTime()) {
        changeState(STATE_CLOSING);
    } else if (checkPointedItem()) {
        mBaseTalk.mKind = talk_c::KIND_ITEM;
        dDemo::requestSpeak(this, NULL);
    } else {
        mVec3_c playerPos =
            *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
        f32 dist = calcPlayerDistXZ(dPlayerMgr::PLAYER_INDEX_CURRENT, NULL);
        mVec3_c step = getRouteStep(&playerPos);
        if (dist > getMoveDist()) {
            if (mAction.mActionId != ACTION_RUN &&
                mAction.mActionId != ACTION_RUN_FACING) {
                moveParam_c param(1.3333334f, 0.4f, 0.53333336f);
                mAction.requestRunFacing(PRIORITY_IDLE, step, l_runTurnSpeed, 0,
                                         param, l_8074FDEC, cNpcMorphFrames);
            }
        } else {
            if (mAction.mActionId != ACTION_WALK &&
                mAction.mActionId != ACTION_WALK_FACING) {
                mAction.requestWalkFacing(PRIORITY_IDLE, step, l_walkTurnSpeed,
                                          0, l_moveParamWalk, l_defaultAnmRate,
                                          cNpcMorphFrames);
            }
        }
        if (dist <= getStopDist() || step == mPos) {
            changeState(STATE_WAIT);
        } else {
            mAction.setTargetPos(step);
        }
    }
}

// .text 0x1774
void dAcNpcSpTanukiBase_c::initLook() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
    mStep[STATE_LOOK] = STEP_START;
}

// .text 0x17D8
void dAcNpcSpTanukiBase_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        ++mStep[STATE_LOOK];
    }
}

namespace dANpcSpTanukichi {
static const dAcNpcSpTanukiBase_c::Proc l_lookSteps[] = {
    &dAcNpcSpTanukiBase_c::stepLook};
} // namespace dANpcSpTanukichi

// .text 0x1838
void dAcNpcSpTanukiBase_c::executeLook() {
    if (mStep[STATE_LOOK] < STEP_DONE) {
        (this->*l_lookSteps[mStep[STATE_LOOK]])();
    }
}

// .text 0x1878
void dAcNpcSpTanukiBase_c::initTalk() {
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, PRIORITY_IDLE);
    mStep[STATE_TALK] = STEP_START;
}

// .text 0x18E4
void dAcNpcSpTanukiBase_c::stepTalk() {
    if (!mTalk.isBusy() && !mBaseTalk.mExitRequested) {
        dDemo::finishTalk(this);
        ++mStep[STATE_TALK];
    }
}

namespace dANpcSpTanukichi {
static const dAcNpcSpTanukiBase_c::Proc l_talkSteps[] = {
    &dAcNpcSpTanukiBase_c::stepTalk};
} // namespace dANpcSpTanukichi

// .text 0x193C
void dAcNpcSpTanukiBase_c::executeTalk() {
    if (mStep[STATE_TALK] < STEP_DONE) {
        (this->*l_talkSteps[mStep[STATE_TALK]])();
    }
    // The wallpaper / carpet preview: unlock the message once it is shown.
    if (mBaseTalk.mRoomPreview && mBaseTalk.getController() != NULL &&
        mBaseTalk.getController()->mLock && dBgUtil::isWallpaperDone() &&
        dBgUtil::isCarpetDone()) {
        mBaseTalk.getController()->mLock = 0;
        mBaseTalk.mRoomPreview = 0;
    }
}

// .text 0x19D4
void dAcNpcSpTanukiBase_c::initGreet() {
    mBaseTalk.mKind = talk_c::KIND_GREET;
}

// .text 0x19E0
void dAcNpcSpTanukiBase_c::executeGreet() {
    if (!dDemo::isType(dDemo::TYPE_ANY) && dDemo::requestSpeak(this, NULL) &&
        isTwin()) {
        ((dAcNpcSpTanukiBase_c *)fManager_c::searchBaseByProfName(
             getPartnerProfName(), NULL))
            ->changeState(STATE_WATCH);
    }
}

// .text 0x1A70
void dAcNpcSpTanukiBase_c::initSeeOff() {
    mBaseTalk.mKind = talk_c::KIND_SEE_OFF;
}

// .text 0x1A7C
void dAcNpcSpTanukiBase_c::executeSeeOff() {
    if (!dDemo::isType(dDemo::TYPE_ANY)) {
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0x1AC0
void dAcNpcSpTanukiBase_c::initClosing() {
    mBaseTalk.mKind = talk_c::KIND_CLOSING;
}

// .text 0x1ACC
void dAcNpcSpTanukiBase_c::executeClosing() {
    if (!dDemo::isType(dDemo::TYPE_ANY) && dDemo::requestSpeak(this, NULL) &&
        isTwin()) {
        ((dAcNpcSpTanukiBase_c *)fManager_c::searchBaseByProfName(
             getPartnerProfName(), NULL))
            ->changeState(STATE_WATCH);
    }
}

// .text 0x1B5C
void dAcNpcSpTanukiBase_c::initMovePlayer() {
    mVec3_c pos = *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    pos.z += 32.0f;
    dPlayerMgr::moveTo(&pos, dPlayerMgr::PLAYER_INDEX_CURRENT,
                       dPlayerMgr::sDefaultMoveSpeed);
}

// .text 0x1BB8
void dAcNpcSpTanukiBase_c::executeMovePlayer() {
    if (!dPlayerMgr::isMovingTo(dPlayerMgr::PLAYER_INDEX_CURRENT) &&
        !mTalk.isBusy()) {
        dDemo::finishTalk(this);
    }
}

// .text 0x1C08
void dAcNpcSpTanukiBase_c::initWatch() {
    if (sLib::distanceAngle(mAngle.y, getAngleYToPlayer(_1B70)) > 0) {
        mAng angle = getAngleYToPlayer(_1B70);
        mAction.requestTurn(PRIORITY_IDLE, angle, l_turnSpeed, 0,
                            cNpcMorphFrames);
    } else {
        mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    }
}

// .text 0x1CA0
void dAcNpcSpTanukiBase_c::executeWatch() {
    if (mAction.mActionId == ACTION_TURN) {
        if (mAction.mCanChange) {
            mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
        } else {
            mAction.setTargetAngle(getAngleYToPlayer(_1B70));
        }
    }
}

// .text 0x1D14
void dAcNpcSpTanukiBase_c::initMultiWait() {
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
}

// .text 0x1D28
void dAcNpcSpTanukiBase_c::executeMultiWait() {
    if (!dPlayerMgr::isSceneTransitionState() && checkPointedItem()) {
        mBaseTalk.mKind = talk_c::KIND_ITEM;
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0x1D80
void dAcNpcSpTanukiBase_c::initRemote() {
}

// .text 0x1D84
void dAcNpcSpTanukiBase_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mBaseTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
            return;
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_MULTI_WAIT);
            return;
        }
    }
    if (!dPlayerMgr::isSceneTransitionState() && checkPointedItem()) {
        mBaseTalk.mKind = talk_c::KIND_ITEM;
        dDemo::requestSpeak(this, NULL);
    }
}

// .text 0x1E4C
void dAcNpcSpTanukiBase_c::initRemoteTalk() {
}

// .text 0x1E50
void dAcNpcSpTanukiBase_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_MULTI_WAIT);
    }
}

// .text 0x1EA4
BOOL dAcNpcSpTanukiBase_c::isSeeOffTime() {
    if (isMultiPlay()) {
        return FALSE;
    }
    if (isTwin() && !isNearerToPlayer()) {
        return FALSE;
    }
    if (dDemo::isType(dDemo::TYPE_ANY)) {
        return FALSE;
    }
    if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
            PRIVATE_FLAG0_PART_TIME_JOB)) {
        return FALSE;
    }
    if (dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    return dBGCF::isNextLocked(
        dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT));
}

// .text 0x1F78
BOOL dAcNpcSpTanukiBase_c::checkPointedItem() {
    if (isMultiPlay()) {
        if (isTubukichi()) {
            return FALSE;
        }
    } else if (isTwin() && !isNearerToPlayer()) {
        return FALSE;
    }
    dItem::Item item;
    int unitX;
    int unitZ;
    if (!getPointedShopItem(&item, &unitX, &unitZ)) {
        return FALSE;
    }
    mBaseTalk.setPointedItem(item, unitX, unitZ);
    if (mBaseTalk.mItem.isValid()) {
        return TRUE;
    }
    return FALSE;
}

// .text 0x2078
BOOL dAcNpcSpTanukiBase_c::isWorkerAtExit() {
    if (isTwin()) {
        return FALSE;
    }
    if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
            PRIVATE_FLAG0_PART_TIME_JOB)) {
        return FALSE;
    }
    if (dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    return dBGCF::isNextLocked(
        dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT));
}

// .text 0x20F4
BOOL dAcNpcSpTanukiBase_c::isClosingTime() {
    if (isMultiPlay()) {
        return FALSE;
    }
    if (!isNearerToPlayer()) {
        return FALSE;
    }
    if (dDemo::isType(dDemo::TYPE_ANY)) {
        return FALSE;
    }
    if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
            PRIVATE_FLAG0_PART_TIME_JOB)) {
        return FALSE;
    }
    return isShopClosed();
}

// .text 0x2190
BOOL dAcNpcSpTanukiBase_c::isShopClosed() {
    return !dSaveData_c::getTown()->mShops.mShop.isOpen();
}

// .text 0x21C4
f32 dAcNpcSpTanukichi_c::getStopDist() {
    if (mApproached) {
        return 51.2f;
    }
    return 80.0f;
}

// .text 0x21E8
f32 dAcNpcSpTubuMame_c::getStopDist() {
    mVec3_c playerPos =
        *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    fBase_c *base =
        fManager_c::searchBaseByProfName(getPartnerProfName(), NULL);
    dAcNpcSpTanukiBase_c *partner = (dAcNpcSpTanukiBase_c *)base;
    if (partner != NULL && partner->mTalk.isBusy()) {
        // Stop just behind the twin who talks.
        mVec3_c partnerPos = partner->getPos();
        f32 dist = 32.0f + PSVECMag(playerPos - partnerPos);
        if (!mApproached && dist < 80.0f) {
            return 80.0f;
        }
        return dist;
    }
    if (!mApproached) {
        return 80.0f;
    }
    if (isNearerToPlayer()) {
        return 51.2f;
    }
    return 51.2f;
}

// .text 0x231C
f32 dAcNpcSpTanukichi_c::getMoveDist() {
    if (mApproached) {
        return 64.32f;
    }
    return 80.0f;
}

// .text 0x2340
f32 dAcNpcSpTubuMame_c::getMoveDist() {
    f32 stopDist = getStopDist();
    f32 dist;
    if (mApproached) {
        if (isNearerToPlayer()) {
            dist = 64.32f;
        } else {
            dist = 64.32f;
        }
    } else {
        dist = 80.0f;
    }
    if (dist < stopDist) {
        dist = stopDist;
    }
    return dist;
}

// .text 0x23CC
BOOL dAcNpcSpTubuMame_c::isNearerToPlayer() {
    mVec3_c playerPos =
        *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    dAcNpcSpTanukiBase_c *partner =
        (dAcNpcSpTanukiBase_c *)fManager_c::searchBaseByProfName(
            getPartnerProfName(), NULL);
    if (partner != NULL) {
        mVec3_c partnerPos = partner->getPos();
        f32 partnerDist = PSVECMag(playerPos - partnerPos);
        f32 dist = PSVECMag(playerPos - mPos);
        if (dist < partnerDist) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text 0x24AC
u16 dAcNpcSpTanukichi_c::getPartnerProfName() {
    return fProf::A_NPC_SP_TANUKICHI_05E;
}

// .text 0x24B4
u16 dAcNpcSpMamekichi_c::getPartnerProfName() {
    return fProf::A_NPC_SP_TANUKICHI_060;
}

// .text 0x24BC
u16 dAcNpcSpTubukichi_c::getPartnerProfName() {
    return fProf::A_NPC_SP_TANUKICHI_05F;
}

// .text 0x24C4
dAcNpcSpTanukiBase_c::talk_c::talk_c(dAcNpcSpTanukiBase_c *npc) : mpOwner(npc) {
    init();
    mItem = dItem::ITEM_ID_NONE;
    mKind = KIND_TALK;
    mUnitX = 0;
    mUnitZ = 0;
    mTurnipPrice = 0;
    mExitRequested = 0;
    mDeliveryReminded = 0;
}

// .text 0x2560
dAcNpcSpTanukiBase_c::talk_c::~talk_c() {
}

// .text 0x25B8
void dAcNpcSpTanukiBase_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgCode = 0;
    mPrice = 0;
    mSellPrice = 0;
    mOnMessageEnd = NULL;
    mOnAnswer = NULL;
    mOnRequestEnd = NULL;
    mRoomPreview = 0;
    mBought = 0;
    mHowToAsked = 0;
    mOrdered = 0;
    mSaleNow = 0;
}

// .text 0x2654
void dAcNpcSpTanukiBase_c::talk_c::start(dActor_c *target) {
    init();
    dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
    dHome_c *home = homes->getHome(homes->findCurrentPlayer());
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    mpActActor = target;
    // The weekday is read from the current time while it is copied (the
    // copy's register lives across getTown); the hour from the copy.
    dTime_c *cur = dTime_c::getCurrent();
    dTime_c now = *cur;
    int wday = cur->wday;
    mTurnipPrice = dSaveData_c::getTown()->mShops.mStalkMarket.getPrice(
        wday, now.hour >= 12);
    mSaleNow = dSaveData_c::getTown()->mShops.mShop.isSaleNow();
    switch (mKind) {
    case KIND_GREET:
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_PART_TIME_JOB)) {
            mMsgCode = getJobGreetMsg();
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                       PRIVATE_FLAG0_TRAVELLING) ||
                   mpOwner->isTwin()) {
            if (dSaveData_c::getTown()->mShops.mShop.isEventToday()) {
                mMsgCode = MSG_WELCOME_POINT_SPECIAL;
            } else if (mSaleNow) {
                mMsgCode = MSG_WELCOME_TIME_SPECIAL;
                dNpcInfo::tanukichi_c *info = dNpcInfo::tanukichi_c::get();
                info->mTimeSpecialTold = 1;
            } else if (dSaveData_c::getTown()->mShops.mShop.hasFrom6()) {
                mMsgCode = MSG_WELCOME_SPECIAL;
            } else {
                mMsgCode = MSG_WELCOME;
            }
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                       PRIVATE_FLAG0_SHOP_OPENED)) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->clearFlag0(
                PRIVATE_FLAG0_SHOP_OPENED);
            switch (dSaveData_c::getTown()->mShops.mShop.mStage) {
            case SHOP_STAGE_NOOK_N_GO:
                if (dSaveData_c::getTown()->mShops.mShop.mBC_6) {
                    mMsgCode = MSG_GRAND_REOPENING_1;
                } else {
                    mMsgCode = MSG_GRAND_OPENING_1;
                }
                break;
            case SHOP_STAGE_NOOKWAY:
                if (dSaveData_c::getTown()->mShops.mShop.mBC_6) {
                    mMsgCode = MSG_GRAND_REOPENING_2;
                } else {
                    mMsgCode = MSG_GRAND_OPENING_2;
                }
                break;
            case SHOP_STAGE_NOOKINGTONS:
                if (dSaveData_c::getTown()->mShops.mShop.mBC_6) {
                    mMsgCode = MSG_GRAND_REOPENING_3;
                } else {
                    mMsgCode = MSG_GRAND_OPENING_3;
                }
                break;
            }
        } else if (home != NULL && player->mDebt <= 0 &&
                   !dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                       PRIVATE_FLAG0_HOUSE_RENOVATING) &&
                   !dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                       PRIVATE_FLAG0_HOUSE_COMPLETE)) {
            // The loan is paid off: offer the next renovation.
            switch (home->getSize()) {
            case 0:
                mMsgCode = MSG_LOAN_PAID_1;
                break;
            case 1:
                mMsgCode = MSG_LOAN_PAID_2;
                break;
            case 2:
                mMsgCode = MSG_LOAN_PAID_3;
                break;
            case 3:
                mMsgCode = MSG_LOAN_PAID_4;
                break;
            case 4:
            default:
                mMsgCode = MSG_LOAN_PAID_5;
                break;
            }
            if (home->getSize() < HOME_SIZE_NUM - 1) {
                home->requestUpgrade();
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                    PRIVATE_FLAG0_HOUSE_RENOVATING);
            } else {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                    PRIVATE_FLAG0_HOUSE_COMPLETE);
                home->_15BB.mBits.mFlagPending = 1;
            }
        } else if (home != NULL && home->getSize() != 0 &&
                   dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                       PRIVATE_FLAG0_HOUSE_RENOVATING) &&
                   !dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                       PRIVATE_FLAG0_HOUSE_COMPLETE) &&
                   home->getSize() == home->getNextSize()) {
            // The renovation is done: the new loan.
            switch (home->getSize()) {
            case 0:
            case 1:
                mMsgCode = MSG_RENOVATED_1;
                break;
            case 2:
                mMsgCode = MSG_RENOVATED_2;
                break;
            case 3:
                mMsgCode = MSG_RENOVATED_3;
                break;
            case 4:
            default:
                mMsgCode = MSG_RENOVATED_4;
                break;
            }
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setDebtFromHouse();
            dPlayerMgr::Mgr_c::getCurrentPlayer()->clearFlag0(
                PRIVATE_FLAG0_HOUSE_RENOVATING);
        } else if (home != NULL && home->_15BB.mBits.mHasFlag &&
                   !dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                       PRIVATE_FLAG0_HOUSE_FLAG_TOLD)) {
            mMsgCode = MSG_FLAG;
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_HOUSE_FLAG_TOLD);
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                       PRIVATE_FLAG0_SHOP_VOTE_DECIDED)) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->clearFlag0(
                PRIVATE_FLAG0_SHOP_VOTE_DECIDED);
            if (dSaveData_c::getTown()->mShops.mShop.mBC_6 &&
                dPlayerMgr::Mgr_c::getCurrentPlayer()->mShopVote !=
                    SHOP_VOTE_NONE) {
                mMsgCode = MSG_STORE_VOTE_THANKS;
            } else {
                mMsgCode = MSG_CLOSED_NOTICE;
            }
        } else {
            if (dSaveData_c::getTown()->mShops.mShop.isEventToday()) {
                mMsgCode = MSG_WELCOME_POINT_SPECIAL;
            } else if (mSaleNow) {
                mMsgCode = MSG_WELCOME_TIME_SPECIAL;
                dNpcInfo::tanukichi_c *info = dNpcInfo::tanukichi_c::get();
                info->mTimeSpecialTold = 1;
            } else if (dSaveData_c::getTown()->mShops.mShop.hasFrom6()) {
                mMsgCode = MSG_WELCOME_SPECIAL;
            } else {
                mMsgCode = MSG_WELCOME;
            }
        }
        break;
    case KIND_CLOSING:
        mMsgCode = MSG_CLOSING;
        break;
    case KIND_SEE_OFF:
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_PART_TIME_JOB)) {
            mMsgCode = getJobSeeOffMsg();
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                       PRIVATE_FLAG0_TRAVELLING) ||
                   mpOwner->isTwin()) {
            mMsgCode = MSG_THANKS_FOR_SHOPPING;
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->mShopVote !=
                   SHOP_VOTE_NONE) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                    PRIVATE_FLAG0_SHOP_VOTE_RESULT)) {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->clearFlag0(
                    PRIVATE_FLAG0_SHOP_VOTE_RESULT);
                switch (dPlayerMgr::Mgr_c::getCurrentPlayer()->mShopVote) {
                case SHOP_VOTE_HOURS:
                    if (dSaveData_c::getTown()->mShops.mShop.mStage ==
                        SHOP_STAGE_NOOK_N_GO) {
                        mMsgCode = MSG_STORE_VOTE_RESULT_KEPT;
                    } else {
                        mMsgCode = MSG_STORE_VOTE_RESULT;
                    }
                    break;
                case SHOP_VOTE_BALANCE:
                    if (dSaveData_c::getTown()->mShops.mShop.mStage ==
                        SHOP_STAGE_NOOKWAY) {
                        mMsgCode = MSG_STORE_VOTE_RESULT_KEPT;
                    } else {
                        mMsgCode = MSG_STORE_VOTE_RESULT;
                    }
                    break;
                case SHOP_VOTE_VARIETY:
                    if (dSaveData_c::getTown()->mShops.mShop.mStage ==
                        SHOP_STAGE_NOOKINGTONS) {
                        mMsgCode = MSG_STORE_VOTE_RESULT_KEPT;
                    } else {
                        mMsgCode = MSG_STORE_VOTE_RESULT;
                    }
                    break;
                default:
                    mMsgCode = MSG_THANKS_FOR_SHOPPING;
                    break;
                }
            } else {
                mMsgCode = MSG_THANKS_FOR_SHOPPING;
            }
        } else {
            if (dSaveData_c::getTown()->mShops.mShop.isVoteCountdown()) {
                mMsgCode = MSG_STORE_VOTE;
            } else {
                mMsgCode = MSG_THANKS_FOR_SHOPPING;
            }
        }
        break;
    case KIND_TALK:
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_PART_TIME_JOB)) {
            mMsgCode = getJobTalkMsg();
        } else if (mpOwner->mDrama) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getGracieStep() ==
                3) {
                mMsgCode = DRAMA_MSG_WORRIED;
            } else {
                mMsgCode = DRAMA_MSG_REGRET;
            }
        } else {
            mMsgCode = MSG_WHAT_CAN_I_DO;
        }
        break;
    case KIND_ITEM:
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_PART_TIME_JOB)) {
            mMsgCode = getJobItemMsg();
        } else {
            if (mSaleNow && !dNpcInfo::tanukichi_c::get()->mTimeSpecialTold) {
                mMsgCode = MSG_TIME_SPECIAL;
                dNpcInfo::tanukichi_c *info = dNpcInfo::tanukichi_c::get();
                info->mTimeSpecialTold = 1;
            } else {
                mMsgCode = getPriceMsg();
            }
            mPrice = mItem.getShopPrice();
            if (mSaleNow) {
                mPrice /= 2;
            }
        }
        break;
    }
}

// .text 0x2EA8
u16 dAcNpcSpTanukiBase_c::talk_c::getPriceMsg() {
    int msg = MSG_PRICE;
    if (isBasicTool(&mItem)) {
        msg = MSG_PRICE_TOOL;
    } else if (isSilverTool(&mItem)) {
        msg = MSG_PRICE_SILVER_TOOL;
    } else {
        switch (mItem.getKind()) {
        case dItem::KIND_WALL:
        case dItem::KIND_CARPET:
            msg = MSG_PRICE_PREVIEW;
            break;
        case dItem::KIND_PAPER:
            msg = MSG_PRICE_PAPER;
            break;
        case dItem::KIND_PAINT:
            if (dAcNpc_c::isPlayerFromThisTown()) {
                msg = MSG_PRICE_ROOF_PAINT;
            } else {
                msg = MSG_ROOF_PAINT_OUT_OF_TOWN;
            }
            break;
        default: {
            const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(mItem);
            if (bitm != NULL && bitm->getFrom() == dItem::FROM_EYE_CATCHER) {
                msg = MSG_PRICE_SPECIAL;
            }
            break;
        }
        }
    }
    return msg;
}

// .text 0x2FB0
int dAcNpcSpTanukiBase_c::talk_c::getJobTalkMsg() {
    dQuestErrandList_c *errand =
        &dPlayerMgr::Mgr_c::getCurrentPlayer()->mErrand;
    int kind = errand->getKind();
    dAnimalBlock_c *animals = &dSaveData_c::getRaw()->mAnimals.mTown;
    BOOL noClothes = FALSE;
    BOOL changed = !dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                       PRIVATE_FLAG0_JOB_SCOLDED) &&
                   dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                       PRIVATE_FLAG0_JOB_CHANGED);
    if (changed) {
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mShirt.isNotSame(
                dItem::Item((int)dItem::ITEM_IDX_WORK_UNIFORM))) {
            noClothes = TRUE;
        }
    }
    if (noClothes) {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
            PRIVATE_FLAG0_JOB_SCOLDED);
        return JOB_MSG_NO_WORK_CLOTHES;
    }
    if (kind == QUEST_KIND_FIRSTJOB_POST_NOTICE) {
        return errand->isState(1) ? (int)JOB_MSG_NOTICE_POSTED
                                  : (int)JOB_MSG_NOTICE_WAIT;
    }
    if (kind == QUEST_KIND_FIRSTJOB_DELIVER_WATERING_CAN) {
        if (errand->isState(1)) {
            return JOB_MSG_WATERING_CAN_DELIVERED;
        }
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countErrandPockets(NULL) ==
            0) {
            return !animals->isErrandSenderResident(NULL)
                       ? (int)JOB_MSG_WATERING_CAN_GIVE_UP
                       : (int)JOB_MSG_WATERING_CAN_LOST;
        }
        return JOB_MSG_DELIVERY_WAIT;
    }
    if (kind == QUEST_KIND_FIRSTJOB_DELIVER_CARPET) {
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_JOB_POCKETS_FULL)) {
            return dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                           0
                       ? (int)JOB_MSG_ROOM_THEN_OTHER_CARPET
                       : (int)JOB_MSG_DROP_STUFF;
        }
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_JOB_HELLO_AGAIN)) {
            return dNpcInfo::tanukichi_c::get()->isAllVillagersMet()
                       ? (int)JOB_MSG_HELLO_THEN_OTHER_CARPET
                       : (int)JOB_MSG_INTRODUCTIONS;
        }
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_JOB_HELLO)) {
            return dNpcInfo::tanukichi_c::get()->isAllVillagersMet()
                       ? (int)JOB_MSG_HELLO_THEN_WATERING_CAN
                       : (int)JOB_MSG_INTRODUCTIONS;
        }
        if (errand->isState(1)) {
            return dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                           0
                       ? (int)JOB_MSG_ROOM_THEN_WATERING_CAN
                       : (int)JOB_MSG_DROP_STUFF;
        }
        if (errand->isState(2)) {
            return JOB_MSG_CARPET_DELIVERED;
        }
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countErrandPockets(NULL) ==
            0) {
            return !animals->isErrandSenderResident(NULL)
                       ? (int)JOB_MSG_CARPET_GIVE_UP_ASK
                       : (int)JOB_MSG_CARPET_LOST;
        }
        return JOB_MSG_DELIVERY_WAIT;
    }
    if (kind == QUEST_KIND_FIRSTJOB_SEND_LETTER) {
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_JOB_HELLO_AGAIN)) {
            return dNpcInfo::tanukichi_c::get()->isAllVillagersMet()
                       ? (int)JOB_MSG_HELLO_THEN_OTHER_LETTER
                       : (int)JOB_MSG_INTRODUCTIONS;
        }
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_JOB_HELLO)) {
            return dNpcInfo::tanukichi_c::get()->isAllVillagersMet()
                       ? (int)JOB_MSG_HELLO_THEN_CARPET
                       : (int)JOB_MSG_INTRODUCTIONS;
        }
        if (errand->isState(1)) {
            return dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                           0
                       ? (int)JOB_MSG_ROOM_THEN_CARPET
                       : (int)JOB_MSG_DROP_STUFF;
        }
        if (!animals->isErrandSenderResident(NULL)) {
            return JOB_MSG_LETTER_LOST;
        }
        if (errand->isState(2) || errand->isState(3)) {
            if (!errand->isState(2)) {
                return JOB_MSG_LETTER_LATE;
            }
            return JOB_MSG_LETTER_SENT;
        }
        return dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(isPaper,
                                                                   NULL)
                   ? (int)JOB_MSG_HOW_TO_LETTER
                   : (int)JOB_MSG_MORE_PAPER;
    }
    if (kind == QUEST_KIND_FIRSTJOB_DELIVER_FTR) {
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_JOB_POCKETS_FULL)) {
            return dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                           0
                       ? (int)JOB_MSG_ROOM_THEN_OTHER_FTR
                       : (int)JOB_MSG_DROP_STUFF;
        }
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_JOB_HELLO_AGAIN)) {
            return dNpcInfo::tanukichi_c::get()->isAllVillagersMet()
                       ? (int)JOB_MSG_HELLO_THEN_OTHER_FTR
                       : (int)JOB_MSG_INTRODUCTIONS;
        }
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_JOB_HELLO)) {
            return dNpcInfo::tanukichi_c::get()->isAllVillagersMet()
                       ? (int)JOB_MSG_HELLO_THEN_LETTER
                       : (int)JOB_MSG_INTRODUCTIONS;
        }
        if (errand->isState(1)) {
            return dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                           0
                       ? (int)JOB_MSG_ROOM_MADE
                       : (int)JOB_MSG_DROP_STUFF;
        }
        if (errand->isState(2)) {
            return JOB_MSG_DELIVERED;
        }
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countErrandPockets(NULL) ==
            0) {
            return !dSaveData_c::getRaw()
                           ->mAnimals.mTown.isErrandSenderResident(NULL)
                       ? (int)JOB_MSG_FTR_GIVE_UP_ASK
                       : (int)JOB_MSG_FTR_LOST;
        }
        if (!mDeliveryReminded) {
            mDeliveryReminded = 1;
            return JOB_MSG_NOT_DELIVERED;
        }
        return JOB_MSG_NOT_DELIVERED_AGAIN;
    }
    if (kind == QUEST_KIND_FIRSTJOB_PLANT_FLOWER) {
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_JOB_HELLO)) {
            return dNpcInfo::tanukichi_c::get()->isAllVillagersMet()
                       ? (int)JOB_MSG_HELLO_THEN_FTR
                       : (int)JOB_MSG_INTRODUCTIONS;
        }
        if (errand->isState(1)) {
            return dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                           0
                       ? (int)JOB_MSG_DROPPED_STUFF
                       : (int)JOB_MSG_DROP_STUFF;
        }
        int num = dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
            isPlantItem, NULL);
        if (num <= 0) {
            return JOB_MSG_PLANTED;
        }
        return num >= JOB_PLANT_ITEM_NUM ? (int)JOB_MSG_HOW_TO_PLANT
                                         : (int)JOB_MSG_PLANT_MORE;
    }
    if (kind == QUEST_KIND_FIRSTJOB_CHANGE_CLOTH) {
        if (errand->isState(1)) {
            return dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                       NULL, NULL) >= JOB_PLANT_ITEM_NUM
                       ? (int)JOB_MSG_DROPPED_STUFF
                       : (int)JOB_MSG_DROP_STUFF;
        }
        if (errand->isState(2)) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                0) {
                return dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                           isWorkClothes, NULL) != 0
                           ? (int)JOB_MSG_ROOM_MADE_CHECK
                           : (int)JOB_MSG_NEW_CLOTHES;
            }
            return dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                       isWorkClothes, NULL) != 0
                       ? (int)JOB_MSG_CLOTHES_NO_ROOM_2
                       : (int)JOB_MSG_LOST_CLOTHES_NO_ROOM;
        }
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment.mShirt.isSame(
                dItem::Item((int)dItem::ITEM_IDX_WORK_UNIFORM))) {
            return JOB_MSG_CLOTHES_ON;
        }
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(isWorkClothes,
                                                                NULL)) {
            return JOB_MSG_NOT_CHANGED;
        }
        return dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >= 0
                   ? (int)JOB_MSG_LOST_CLOTHES
                   : (int)JOB_MSG_LOST_CLOTHES_FULL;
    }
    if (dNpcInfo::tanukichi_c::get()->isAllVillagersMet()) {
        return dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >= 0
                   ? (int)JOB_MSG_DROPPED_STUFF
                   : (int)JOB_MSG_DROP_STUFF;
    }
    return JOB_MSG_HELLO_FIRST;
}

// .text 0x36A0
int dAcNpcSpTanukiBase_c::talk_c::getJobGreetMsg() {
    return dNpcInfo::tanukichi_c::get()->isAllVillagersMet()
               ? JOB_MSG_ALL_MET
               : JOB_MSG_NOT_ALL_MET;
}

// .text 0x36D4
int dAcNpcSpTanukiBase_c::talk_c::getJobSeeOffMsg() {
    return JOB_MSG_SECOND_FLOOR;
}

// .text 0x36DC
int dAcNpcSpTanukiBase_c::talk_c::getJobItemMsg() {
    return JOB_MSG_NOT_FOR_WORKERS;
}

// .text 0x36E4
void dAcNpcSpTanukiBase_c::talk_c::getMsgInfo(msgInfo_s *info) {
    if (mpOwner->isTwin()) {
        info->mLabel = "npc_SP/NPC_twins";
        info->mCode = mMsgCode;
    } else if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                   PRIVATE_FLAG0_PART_TIME_JOB)) {
        info->mLabel = "npc_SP/NPC_tanukichi_1";
        info->mCode = mMsgCode;
    } else if (mpOwner->mDrama && mKind == KIND_TALK) {
        info->mLabel = "npc_SP/NPC_drama6";
        info->mCode = mMsgCode;
    } else {
        info->mLabel = "npc_SP/NPC_tanukichi";
        info->mCode = mMsgCode;
    }
}

// .text 0x37C0
void dAcNpcSpTanukiBase_c::talk_c::onMessageStart(int arg) {
    if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
            PRIVATE_FLAG0_PART_TIME_JOB)) {
        onJobMessageStart(arg);
        return;
    }
    if (mpOwner->mDrama && mKind == KIND_TALK) {
        return;
    }
    if (mpOwner->isTwin()) {
        dItem::Item key((u16)SP_NPC_MAMEDANUKI0);
        setVoiceType(dNpcInfo::getSpVoice(&key));
        getController()->updateVoiceType();
    }
    switch (mMessageCode) {
    case MSG_ANYTHING_ELSE:
        mOrdered = 0;
        break;
    case MSG_CLOSED_NOTICE:
    case MSG_STORE_VOTE_THANKS: {
        // The store's upgrade day.
        dSaveShop_c *shop = &dSaveData_c::getTown()->mShops.mShop;
        int day;
        int month;
        BOOL upgrading;
        if (shop->mUpgrading) {
            const dTime_c &date = shop->mStageDate.get();
            day = date.mday;
            month = date.mon;
            upgrading = TRUE;
        } else {
            upgrading = FALSE;
        }
        if (upgrading) {
            setMonthName(month, 1);
            setDayName((u8)day, 2);
        }
        break;
    }
    case MSG_TURNIP_PRICE:
        setNumber(mTurnipPrice, 0, 3, dScript::NUM_FORMAT_REGION);
        break;
    case MSG_SELL_PRICE:
    case MSG_SELL_PRICE_ALL:
    case MSG_SELL_IMPORTANT:
    case MSG_SELL_IMPORTANT_ONE:
    case MSG_SELL_PRICE_FORGERY:
    case MSG_SELL_PRICE_FORGERY_ALL:
        setNumber(mSellPrice, 3, 7, dScript::NUM_FORMAT_REGION);
        break;
    case MSG_ORDERED: {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->payMoney(mPrice, 0);
        dSaveTown_c *town = dSaveData_c::getTown();
        town->mShops.mShop.sellAndNotify(dScene::getCurrentScene(),
                                         SHOP_SLOT_NONE, 0, 0, mPrice,
                                         !dAcNpc_c::isPlayerFromThisTown());
        dLetter::setItemName(0, mItem);
        static dMail_c l_mail;
        u16 kind = 0;
        switch (dSaveData_c::getTown()->mShops.mShop.mStage) {
        case SHOP_STAGE_CRANNY:
            kind = ORDER_MAIL_CRANNY;
            break;
        case SHOP_STAGE_NOOK_N_GO:
            kind = ORDER_MAIL_NOOK_N_GO;
            break;
        case SHOP_STAGE_NOOKWAY:
            kind = ORDER_MAIL_NOOKWAY;
            break;
        case SHOP_STAGE_NOOKINGTONS:
            kind = ORDER_MAIL_NOOKINGTONS;
            break;
        }
        setupMail(&l_mail, &kind, MAIL_FROM_NOOK,
                  dPlayerMgr::Mgr_c::getCurrentPlayer(),
                  dItem::ITEM_IDX_NOOK_PAPER);
        l_mail.setPresent(mItem.getId(), 0xFF);
        dPostOffice::add(&l_mail);
        int points = mPrice / 100;
        if (points <= 0) {
            points = 1;
        }
        if (dSaveData_c::getTown()->mShops.mShop.isEventToday()) {
            points *= 5;
        }
        dPlayerMgr::Mgr_c::getCurrentPlayer()->addNookPoints(points);
        break;
    }
    case MSG_PRICE:
    case MSG_PRICE_TOOL:
    case MSG_PRICE_PREVIEW:
    case MSG_PRICE_SPECIAL:
    case MSG_PRICE_PAPER:
    case MSG_PRICE_SILVER_TOOL:
    case MSG_ROOF_PAINT_OUT_OF_TOWN:
    case MSG_PRICE_ROOF_PAINT:
        setItemName(mItem, 0);
        setNumber(mPrice, 4, 7, dScript::NUM_FORMAT_REGION);
        break;
    case MSG_PRICE_AFTER_PREVIEW:
        getController()->mLock = 1;
        break;
    case MSG_ROOF_COLOR: {
        getController()->setStringWord(0, getPlayerRoofColor() + 1,
                                       "sys_STRING/STR_RoofColor");
        break;
    }
    }
}

// .text 0x3BE8
void dAcNpcSpTanukiBase_c::talk_c::onJobMessageStart(int arg) {
    dQuestErrandList_c *errand =
        &dPlayerMgr::Mgr_c::getCurrentPlayer()->mErrand;
    switch (mMessageCode) {
    case JOB_MSG_CHANGE_CLOTHES:
        errand->startChangeCloth();
        break;
    case JOB_MSG_CLOTHES_ON:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
            PRIVATE_FLAG0_JOB_CHANGED);
        break;
    case JOB_MSG_PLANT_DONE_ASK:
        errand->startPlantFlower();
        break;
    case JOB_MSG_DELIVER_FTR:
        errand->startDeliverFtr();
        setAnmPersonalName(errand->getSender(), 0);
        break;
    case JOB_MSG_LETTER:
    case JOB_MSG_THINGS_CHANGED:
        errand->startSendLetter();
        setAnmPersonalName(errand->getSender(), 0);
        break;
    case JOB_MSG_HOW_TO_LETTER:
    case JOB_MSG_LETTER_WRONG:
        setAnmPersonalName(errand->getSender(), 0);
        break;
    case JOB_MSG_CARPET:
        errand->startDeliverCarpet();
        setAnmPersonalName(errand->getSender(), 0);
        break;
    case JOB_MSG_WATERING_CAN:
        errand->startDeliverWateringCan();
        setAnmPersonalName(errand->getSender(), 0);
        break;
    case JOB_MSG_WATERING_CAN_DELIVERED:
    case JOB_MSG_NOTICE:
        errand->startPostNotice();
        break;
    case JOB_MSG_DELIVERY_WAIT:
    case JOB_MSG_FTR_LOST:
    case JOB_MSG_FTR_GIVE_UP_ASK:
    case JOB_MSG_FTR_AGAIN:
    case JOB_MSG_LETTER_LOST:
    case JOB_MSG_LETTER_INSTEAD:
    case JOB_MSG_CARPET_LOST:
    case JOB_MSG_CARPET_GIVE_UP_ASK:
    case JOB_MSG_CARPET_AGAIN:
    case JOB_MSG_WATERING_CAN_LOST:
    case JOB_MSG_WATERING_CAN_GIVE_UP:
        setAnmPersonalName(errand->getSender(), 0);
        break;
    }
}

// .text 0x3D40
void dAcNpcSpTanukiBase_c::talk_c::onMessageEnd(int kind) {
    if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
            PRIVATE_FLAG0_PART_TIME_JOB)) {
        onJobMessageEnd(kind);
    } else if (mpOwner->mDrama && mKind == KIND_TALK) {
        switch (mMessageCode) {
        case DRAMA_MSG_BACK_TO_WORK:
        case DRAMA_MSG_BACK_TO_WORK_2:
            // A copy of the label in this function's .data (not merged with
            // getMsgInfo's literal).
            static char label[] = "npc_SP/NPC_tanukichi";
            getController()->setNextMsgLabel(label);
            getController()->setNextMsgCode(MSG_MENU);
            mpOwner->mDrama = 0;
            break;
        case DRAMA_MSG_TOLD:
        case DRAMA_MSG_TOLD_2:
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_DRAMA_NOOK_HEARD);
            break;
        }
    } else {
        switch (mMessageCode) {
        case MSG_SELL_WHAT:
            mOnMessageEnd = &talk_c::selectSellItems;
            mOnRequestEnd = &talk_c::onSellSelected;
            break;
        case MSG_SELL_TOO_MUCH_MONEY: {
            removeMenuSelItems();
            dPlayerMgr::Mgr_c::getCurrentPlayer()->addMoney(mSellPrice);
            dSaveTown_c *town = dSaveData_c::getTown();
            town->mShops.mShop.sellAndNotify(dScene::getCurrentScene(),
                                             SHOP_SLOT_NONE, 0, 0, mSellPrice,
                                             !dAcNpc_c::isPlayerFromThisTown());
            break;
        }
        case MSG_CATALOG:
            mOnMessageEnd = &talk_c::openCatalog;
            mOnRequestEnd = &talk_c::onCatalogSelected;
            break;
        case MSG_SELL_IMPORTANT_ONE:
            break;
        case MSG_ORDERED:
            if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                    PRIVATE_FLAG0_NOOK_POINTS_TOLD)) {
                getController()->setNextMsgCode(MSG_ANYTHING_ELSE);
            } else {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                    PRIVATE_FLAG0_NOOK_POINTS_TOLD);
                getController()->setNextMsgCode(MSG_POINT_SYSTEM);
            }
            break;
        case MSG_FORGED_PAINTING: {
            int num;
            getMenuSelItems(&num);
            if (num <= 1) {
                getController()->setNextMsgCode(MSG_SELL_PRICE_FORGERY);
            } else {
                getController()->setNextMsgCode(MSG_SELL_PRICE_FORGERY_ALL);
            }
            break;
        }
        case MSG_BOUGHT:
            getController()->setNextMsgCode(MSG_BOUGHT_EXACTLY);
            break;
        case MSG_BOUGHT_EXACTLY:
            if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                    PRIVATE_FLAG0_NOOK_POINTS_TOLD)) {
                if (isTool(&mItem) && !mHowToAsked) {
                    getController()->setNextMsgCode(MSG_ASK_HOW_TO);
                }
            } else {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                    PRIVATE_FLAG0_NOOK_POINTS_TOLD);
                getController()->setNextMsgCode(MSG_POINT_SYSTEM);
            }
            break;
        case MSG_POINT_SYSTEM:
            if (mOrdered) {
                getController()->setNextMsgCode(MSG_ANYTHING_ELSE);
            } else if (isTool(&mItem) && !mHowToAsked) {
                getController()->setNextMsgCode(MSG_ASK_HOW_TO);
            }
            break;
        case MSG_HOW_TO_ROD:
        case MSG_HOW_TO_SHOVEL:
        case MSG_HOW_TO_NET:
        case MSG_HOW_TO_WATERING_CAN:
        case MSG_HOW_TO_AXE:
        case MSG_HOW_TO_TIMER:
        case MSG_HOW_TO_SLINGSHOT:
        case MSG_HOW_TO_END:
            if (!mBought) {
                mHowToAsked = 1;
                switch (mItem.getKind()) {
                case dItem::KIND_SILVER_FISHINGROD:
                case dItem::KIND_SILVER_NET:
                case dItem::KIND_SILVER_PACHINKO:
                    getController()->setNextMsgCode(
                        MSG_PRICE_SILVER_TOOL_AGAIN);
                    break;
                default:
                    getController()->setNextMsgCode(MSG_PRICE_AGAIN);
                    break;
                }
            }
            break;
        case MSG_ROOF_PAINT_BOUGHT:
            if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                    PRIVATE_FLAG0_NOOK_POINTS_TOLD)) {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                    PRIVATE_FLAG0_NOOK_POINTS_TOLD);
                getController()->setNextMsgCode(MSG_POINT_SYSTEM);
            }
            break;
        case MSG_TIME_SPECIAL:
        case MSG_TIME_SPECIAL_END:
            getController()->setNextMsgCode(getPriceMsg());
            break;
        case MSG_STORE_VOTE_DECLINED:
            break;
        case MSG_STORE_VOTE_REASON:
            switch (dSaveData_c::getTown()->mShops.mShop.mStage) {
            case SHOP_STAGE_NOOK_N_GO:
                getController()->setNextMsgCode(MSG_STORE_VOTE_HOURS);
                break;
            case SHOP_STAGE_NOOKWAY:
                getController()->setNextMsgCode(MSG_STORE_VOTE_BALANCE);
                break;
            case SHOP_STAGE_NOOKINGTONS:
                getController()->setNextMsgCode(MSG_STORE_VOTE_VARIETY);
                break;
            }
            break;
        case MSG_STORE_VOTE_RESULT:
        case MSG_STORE_VOTE_RESULT_KEPT:
            switch (dSaveData_c::getTown()->mShops.mShop.mStage) {
            case SHOP_STAGE_NOOK_N_GO:
                getController()->setNextMsgCode(MSG_KEEP_SHOPPING_1);
                break;
            case SHOP_STAGE_NOOKWAY:
                getController()->setNextMsgCode(MSG_KEEP_SHOPPING_2);
                break;
            case SHOP_STAGE_NOOKINGTONS:
                getController()->setNextMsgCode(MSG_KEEP_SHOPPING_3);
                break;
            }
            break;
        }
        switch (mKind) {
        case KIND_SEE_OFF:
        case KIND_CLOSING:
            dMsg::Comp_c::mInstance->mBgmKeep = 1;
            break;
        }
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0x42A4
void dAcNpcSpTanukiBase_c::talk_c::onJobMessageEnd(int kind) {
    dQuestErrandList_c *errand =
        &dPlayerMgr::Mgr_c::getCurrentPlayer()->mErrand;
    switch (mMessageCode) {
    case JOB_MSG_START:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
            PRIVATE_FLAG0_NEWCOMER_DONE);
        errand->startFirstJob();
        break;
    case JOB_MSG_CHANGE_CLOTHES:
    case JOB_MSG_LOST_CLOTHES:
    case JOB_MSG_NEW_CLOTHES:
        mItem = dItem::Item((int)dItem::ITEM_IDX_WORK_UNIFORM);
        mOnMessageEnd = &talk_c::handOver;
        mOnRequestEnd = &talk_c::giveItem;
        getController()->setNextMsgCode(JOB_MSG_WAIT_CHANGE);
        break;
    case JOB_MSG_CLOTHES_NO_ROOM:
        errand->setState(1);
        break;
    case JOB_MSG_DROPPED_STUFF:
        if (errand->getKind() == QUEST_KIND_FIRSTJOB_PLANT_FLOWER) {
            errand->resetState(1);
            if (!dNpcInfo::tanukichi_c::get()->isAllVillagersMet()) {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                    PRIVATE_FLAG0_JOB_HELLO);
                getController()->setNextMsgCode(JOB_MSG_SAY_HELLO);
            } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(
                           0) >= 0) {
                getController()->setNextMsgCode(JOB_MSG_DELIVER_FTR);
            } else {
                getController()->setNextMsgCode(JOB_MSG_DELIVER_NO_ROOM);
            }
        } else if (errand->getKind() == QUEST_KIND_FIRSTJOB_CHANGE_CLOTH) {
            errand->resetState(1);
            getController()->setNextMsgCode(JOB_MSG_GIVE_SEEDS);
        } else {
            errand->resetState(1);
            getController()->setNextMsgCode(JOB_MSG_CHANGE_CLOTHES);
        }
        break;
    case JOB_MSG_CLOTHES_ON:
        getController()->setNextMsgCode(JOB_MSG_PLANT);
        break;
    case JOB_MSG_PLANT:
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(NULL, NULL) >=
            JOB_PLANT_ITEM_NUM) {
            startGiveSeeds();
            getController()->setNextMsgCode(JOB_MSG_PLANT_DONE_ASK);
        } else {
            getController()->setNextMsgCode(JOB_MSG_PLANT_NO_ROOM);
        }
        break;
    case JOB_MSG_PLANT_NO_ROOM:
        errand->setState(1);
        break;
    case JOB_MSG_GIVE_SEEDS:
        startGiveSeeds();
        getController()->setNextMsgCode(JOB_MSG_PLANT_DONE_ASK);
        break;
    case JOB_MSG_LOST_CLOTHES_FULL:
        errand->setState(2);
        break;
    case JOB_MSG_PLANTED:
        getController()->setNextMsgCode(JOB_MSG_REAL_PART_TIMER);
        break;
    case JOB_MSG_REAL_PART_TIMER:
        if (!dNpcInfo::tanukichi_c::get()->isAllVillagersMet()) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_JOB_HELLO);
            getController()->setNextMsgCode(JOB_MSG_SAY_HELLO);
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                   0) {
            getController()->setNextMsgCode(JOB_MSG_DELIVER_FTR);
        } else {
            getController()->setNextMsgCode(JOB_MSG_DELIVER_NO_ROOM);
        }
        break;
    case JOB_MSG_DELIVER_NO_ROOM: {
        errand->getKind(); // the result is unused
        errand->setState(1);
        break;
    }
    case JOB_MSG_DELIVER_FTR:
        mItem = *errand->getItem();
        mOnMessageEnd = &talk_c::handOver;
        mOnRequestEnd = &talk_c::giveErrandItem;
        getController()->setNextMsgCode(mMessageCode + 1);
        break;
    case JOB_MSG_DELIVERED:
        getController()->setNextMsgCode(JOB_MSG_NEXT_JOB);
        break;
    case JOB_MSG_NEXT_JOB:
        if (!dNpcInfo::tanukichi_c::get()->isAllVillagersMet()) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_JOB_HELLO);
            getController()->setNextMsgCode(JOB_MSG_SAY_HELLO);
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                   0) {
            getController()->setNextMsgCode(JOB_MSG_LETTER);
        } else {
            getController()->setNextMsgCode(JOB_MSG_DELIVER_NO_ROOM);
        }
        break;
    case JOB_MSG_LETTER:
    case JOB_MSG_GIVE_PAPER:
    case JOB_MSG_MORE_PAPER_2: {
        dItem::Item paper;
        paper = dItem::Item((int)dItem::ITEM_IDX_LINED_PAPER);
        mItem = paper.makeVariant(0);
        mOnMessageEnd = &talk_c::handOver;
        mOnRequestEnd = &talk_c::giveItem;
        getController()->setNextMsgCode(mMessageCode + 1);
        break;
    }
    case JOB_MSG_ROOM_MADE:
        errand->resetState(1);
        if (!dNpcInfo::tanukichi_c::get()->isAllVillagersMet()) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_JOB_HELLO);
            getController()->setNextMsgCode(JOB_MSG_SAY_HELLO);
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                   0) {
            getController()->setNextMsgCode(JOB_MSG_LETTER);
        } else {
            getController()->setNextMsgCode(JOB_MSG_DELIVER_NO_ROOM);
        }
        break;
    case JOB_MSG_LETTER_SENT:
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(isPaper,
                                                                NULL) == 0 &&
            dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >= 0) {
            getController()->setNextMsgCode(JOB_MSG_MORE_PAPER_2);
        } else {
            getController()->setNextMsgCode(JOB_MSG_LETTER_WRONG);
        }
        break;
    case JOB_MSG_LETTER_WRONG:
        errand->resetState(2);
        break;
    case JOB_MSG_LETTER_LATE:
        if (!dNpcInfo::tanukichi_c::get()->isAllVillagersMet()) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_JOB_HELLO);
            getController()->setNextMsgCode(JOB_MSG_SAY_HELLO);
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                   0) {
            getController()->setNextMsgCode(JOB_MSG_CARPET);
        } else {
            getController()->setNextMsgCode(JOB_MSG_CARPET_NO_ROOM);
        }
        break;
    case JOB_MSG_CARPET:
        mItem = *errand->getItem();
        mOnMessageEnd = &talk_c::handOver;
        mOnRequestEnd = &talk_c::giveErrandItem;
        getController()->setNextMsgCode(JOB_MSG_CARPET_GIVEN);
        break;
    case JOB_MSG_CARPET_NO_ROOM: {
        errand->getKind(); // the result is unused
        errand->setState(1);
        break;
    }
    case JOB_MSG_CARPET_DELIVERED:
        if (!dSaveData_c::getRaw()->mAnimals.mTown.hasErrandFromResident(
                NULL) &&
            !dNpcInfo::tanukichi_c::get()->isAllVillagersMet()) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_JOB_HELLO);
            getController()->setNextMsgCode(JOB_MSG_SAY_HELLO);
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                   0) {
            getController()->setNextMsgCode(JOB_MSG_WATERING_CAN);
        } else {
            getController()->setNextMsgCode(JOB_MSG_CARPET_NO_ROOM);
        }
        break;
    case JOB_MSG_WATERING_CAN:
        mItem = *errand->getItem();
        mOnMessageEnd = &talk_c::handOver;
        mOnRequestEnd = &talk_c::giveErrandItem;
        getController()->setNextMsgCode(mMessageCode + 1);
        break;
    case JOB_MSG_NOTICE_POSTED:
        getController()->setNextMsgCode(mMessageCode + 1);
        break;
    case JOB_MSG_LOAN:
        getController()->setNextMsgCode(mMessageCode + 1);
        break;
    case JOB_MSG_NO_WORK_LEFT: {
        dNpcInfo::tanukichi_c *info = dNpcInfo::tanukichi_c::get();
        info->mJobDone = 1;
        errand->cancel();
        dMsg::Comp_c::mInstance->mBgmKeep = 1;
        break;
    }
    case JOB_MSG_SECOND_FLOOR:
        mpOwner->changeState(STATE_MOVE_PLAYER);
        break;
    case JOB_MSG_FTR_LOST:
    case JOB_MSG_FTR_GIVE_UP_ASK:
        for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getPocketFlag(i) ==
                POCKET_FLAG_ERRAND) {
                mItem = dPlayerMgr::Mgr_c::getCurrentPlayer()->mPockets[i];
                mOnMessageEnd = &talk_c::receiveItem;
                mOnRequestEnd = &talk_c::takeErrandItems;
                break;
            }
        }
        getController()->setNextMsgCode(JOB_MSG_FTR_GIVE_UP);
        break;
    case JOB_MSG_FTR_GIVE_UP:
        if (dNpcInfo::tanukichi_c::get()->isAllVillagersMet()) {
            errand->startDeliverFtrAgain();
            mItem = *errand->getItem();
            mOnMessageEnd = &talk_c::handOver;
            mOnRequestEnd = &talk_c::giveErrandItem;
            getController()->setNextMsgCode(JOB_MSG_FTR_AGAIN);
        } else {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_JOB_HELLO_AGAIN);
            getController()->setNextMsgCode(JOB_MSG_SAY_HELLO);
        }
        break;
    case JOB_MSG_LETTER_LOST:
        if (dNpcInfo::tanukichi_c::get()->isAllVillagersMet()) {
            errand->startSendLetter2();
            getController()->setNextMsgCode(JOB_MSG_LETTER_INSTEAD);
        } else {
            getController()->setNextMsgCode(JOB_MSG_DELIVER_OTHER_2);
        }
        break;
    case JOB_MSG_LETTER_INSTEAD:
    case JOB_MSG_THINGS_CHANGED:
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(isPaper,
                                                                NULL) == 0 &&
            dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >= 0) {
            getController()->setNextMsgCode(JOB_MSG_MORE_PAPER_3);
        } else {
            getController()->setNextMsgCode(JOB_MSG_DO_IT_RIGHT);
        }
        break;
    case JOB_MSG_CARPET_LOST:
    case JOB_MSG_CARPET_GIVE_UP_ASK:
        for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getPocketFlag(i) ==
                POCKET_FLAG_ERRAND) {
                mItem = dPlayerMgr::Mgr_c::getCurrentPlayer()->mPockets[i];
                mOnMessageEnd = &talk_c::receiveItem;
                mOnRequestEnd = &talk_c::takeErrandItems;
                break;
            }
        }
        errand->startDeliverCarpetAgain();
        getController()->setNextMsgCode(JOB_MSG_CARPET_GIVE_UP);
        break;
    case JOB_MSG_CARPET_GIVE_UP:
        if (dNpcInfo::tanukichi_c::get()->isAllVillagersMet()) {
            mItem = *errand->getItem();
            mOnMessageEnd = &talk_c::handOver;
            mOnRequestEnd = &talk_c::giveErrandItem;
            getController()->setNextMsgCode(JOB_MSG_CARPET_AGAIN);
        } else {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_JOB_HELLO_AGAIN);
            getController()->setNextMsgCode(JOB_MSG_SAY_HELLO);
        }
        break;
    case JOB_MSG_WATERING_CAN_LOST:
    case JOB_MSG_WATERING_CAN_GIVE_UP:
        for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getPocketFlag(i) ==
                POCKET_FLAG_ERRAND) {
                mItem = dPlayerMgr::Mgr_c::getCurrentPlayer()->mPockets[i];
                mOnMessageEnd = &talk_c::receiveItem;
                mOnRequestEnd = &talk_c::takeErrandItems;
                break;
            }
        }
        getController()->setNextMsgCode(JOB_MSG_FIND_ELSE);
        break;
    case JOB_MSG_MORE_PAPER_3: {
        dItem::Item paper;
        paper = dItem::Item((int)dItem::ITEM_IDX_LINED_PAPER);
        mItem = paper.makeVariant(0);
        mOnMessageEnd = &talk_c::handOver;
        mOnRequestEnd = &talk_c::giveItem;
        getController()->setNextMsgCode(JOB_MSG_DO_IT_RIGHT);
        break;
    }
    case JOB_MSG_DELIVER_OTHER:
        if (errand->getKind() == QUEST_KIND_FIRSTJOB_DELIVER_FTR) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                0) {
                errand->startDeliverFtrAgain();
                mItem = *errand->getItem();
                mOnMessageEnd = &talk_c::handOver;
                mOnRequestEnd = &talk_c::giveErrandItem;
                getController()->setNextMsgCode(JOB_MSG_FTR_AGAIN);
            } else {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                    PRIVATE_FLAG0_JOB_POCKETS_FULL);
                getController()->setNextMsgCode(JOB_MSG_DELIVER_NO_ROOM);
            }
        } else if (errand->getKind() == QUEST_KIND_FIRSTJOB_DELIVER_CARPET) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                0) {
                errand->startDeliverCarpetAgain();
                mItem = *errand->getItem();
                mOnMessageEnd = &talk_c::handOver;
                mOnRequestEnd = &talk_c::giveErrandItem;
                getController()->setNextMsgCode(JOB_MSG_CARPET_AGAIN);
            } else {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                    PRIVATE_FLAG0_JOB_POCKETS_FULL);
                getController()->setNextMsgCode(JOB_MSG_DELIVER_NO_ROOM);
            }
        }
        break;
    case JOB_MSG_DELIVER_OTHER_2:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
            PRIVATE_FLAG0_JOB_HELLO_AGAIN);
        getController()->setNextMsgCode(JOB_MSG_SAY_HELLO);
        break;
    case JOB_MSG_HELLO_THEN_FTR:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->clearFlag0(
            PRIVATE_FLAG0_JOB_HELLO);
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >= 0) {
            getController()->setNextMsgCode(JOB_MSG_DELIVER_FTR);
        } else {
            getController()->setNextMsgCode(JOB_MSG_DELIVER_NO_ROOM);
        }
        break;
    case JOB_MSG_HELLO_THEN_LETTER:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->clearFlag0(
            PRIVATE_FLAG0_JOB_HELLO);
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >= 0) {
            getController()->setNextMsgCode(JOB_MSG_LETTER);
        } else {
            getController()->setNextMsgCode(JOB_MSG_DELIVER_NO_ROOM);
        }
        break;
    case JOB_MSG_HELLO_THEN_OTHER_FTR:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->clearFlag0(
            PRIVATE_FLAG0_JOB_HELLO_AGAIN);
        getController()->setNextMsgCode(JOB_MSG_DELIVER_OTHER);
        break;
    case JOB_MSG_HELLO_THEN_CARPET:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->clearFlag0(
            PRIVATE_FLAG0_JOB_HELLO);
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >= 0) {
            getController()->setNextMsgCode(JOB_MSG_CARPET);
        } else {
            getController()->setNextMsgCode(JOB_MSG_CARPET_NO_ROOM);
        }
        break;
    case JOB_MSG_HELLO_THEN_OTHER_LETTER:
        errand->startSendLetter2();
        dPlayerMgr::Mgr_c::getCurrentPlayer()->clearFlag0(
            PRIVATE_FLAG0_JOB_HELLO_AGAIN);
        getController()->setNextMsgCode(JOB_MSG_THINGS_CHANGED);
        break;
    case JOB_MSG_HELLO_THEN_WATERING_CAN:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->clearFlag0(
            PRIVATE_FLAG0_JOB_HELLO);
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >= 0) {
            getController()->setNextMsgCode(JOB_MSG_WATERING_CAN);
        } else {
            getController()->setNextMsgCode(JOB_MSG_CARPET_NO_ROOM);
        }
        break;
    case JOB_MSG_HELLO_THEN_OTHER_CARPET:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->clearFlag0(
            PRIVATE_FLAG0_JOB_HELLO_AGAIN);
        getController()->setNextMsgCode(JOB_MSG_DELIVER_OTHER);
        break;
    case JOB_MSG_ROOM_THEN_OTHER_FTR:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->clearFlag0(
            PRIVATE_FLAG0_JOB_POCKETS_FULL);
        if (dNpcInfo::tanukichi_c::get()->isAllVillagersMet()) {
            getController()->setNextMsgCode(JOB_MSG_DELIVER_OTHER);
        } else {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_JOB_HELLO_AGAIN);
            getController()->setNextMsgCode(JOB_MSG_SAY_HELLO);
        }
        break;
    case JOB_MSG_ROOM_THEN_CARPET:
        errand->resetState(1);
        if (!dNpcInfo::tanukichi_c::get()->isAllVillagersMet()) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_JOB_HELLO);
            getController()->setNextMsgCode(JOB_MSG_SAY_HELLO);
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                   0) {
            getController()->setNextMsgCode(JOB_MSG_CARPET);
        } else {
            getController()->setNextMsgCode(JOB_MSG_CARPET_NO_ROOM);
        }
        break;
    case JOB_MSG_ROOM_THEN_WATERING_CAN:
        errand->resetState(1);
        if (!dSaveData_c::getRaw()->mAnimals.mTown.hasErrandFromResident(
                NULL) &&
            !dNpcInfo::tanukichi_c::get()->isAllVillagersMet()) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_JOB_HELLO);
            getController()->setNextMsgCode(JOB_MSG_SAY_HELLO);
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                   0) {
            getController()->setNextMsgCode(JOB_MSG_WATERING_CAN);
        } else {
            getController()->setNextMsgCode(JOB_MSG_CARPET_NO_ROOM);
        }
        break;
    case JOB_MSG_ROOM_THEN_OTHER_CARPET:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->clearFlag0(
            PRIVATE_FLAG0_JOB_POCKETS_FULL);
        if (dNpcInfo::tanukichi_c::get()->isAllVillagersMet()) {
            getController()->setNextMsgCode(JOB_MSG_DELIVER_OTHER);
        } else {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_JOB_HELLO_AGAIN);
            getController()->setNextMsgCode(JOB_MSG_SAY_HELLO);
        }
        break;
    }
    switch (mMessageCode) {
    case JOB_MSG_NEW_CLOTHES:
    case JOB_MSG_ROOM_MADE_CHECK:
    case JOB_MSG_CLOTHES_NO_ROOM_2:
        errand->resetState(2);
        break;
    }
}

inline void dAcNpcSpTanukiBase_c::talk_c::onStoreAnswer(int code, int sel) {
    switch (code) {
    case MSG_WHAT_CAN_I_DO:
    case MSG_ANYTHING_ELSE:
    case MSG_ANYTHING_ELSE_2:
    case MSG_NO_SELLING:
    case MSG_SOLD:
    case MSG_SELL_KEPT:
    case MSG_MENU:
        switch (sel) {
        case 0:
            getController()->setNextMsgCode(MSG_SELL_WHAT);
            break;
        case 1:
            if (!dAcNpc_c::isPlayerFromThisTown()) {
                getController()->setNextMsgCode(MSG_ORDER_OUT_OF_TOWN);
            } else if (dPostOffice::isFull() == TRUE) {
                getController()->setNextMsgCode(MSG_ORDERS_FULL);
            } else {
                getController()->setNextMsgCode(MSG_CATALOG);
            }
            break;
        case 2:
            if (dTime_c::getCurrent()->wday == WEEKDAY_SUNDAY) {
                getController()->setNextMsgCode(MSG_TURNIP_SUNDAY);
            } else {
                getController()->setNextMsgCode(MSG_TURNIP_PRICE);
            }
            break;
        case 3:
            getController()->setNextMsgCode(MSG_VERY_GOOD_THEN);
            break;
        }
        break;
    case MSG_SELL_PRICE:
    case MSG_SELL_PRICE_ALL:
    case MSG_SELL_IMPORTANT:
    case MSG_SELL_IMPORTANT_ONE:
    case MSG_SELL_PRICE_FORGERY:
    case MSG_SELL_PRICE_FORGERY_ALL:
        switch (sel) {
        case 0: {
            int num;
            getMenuSelItems(&num);
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->getMoneyRoom(num) <
                mSellPrice) {
                getController()->setNextMsgCode(MSG_SELL_NO_ROOM);
            } else if (mSellPrice >
                       getBellsRoom(dPlayerMgr::Mgr_c::getCurrentPlayer())) {
                getController()->setNextMsgCode(MSG_SELL_TOO_MUCH_MONEY);
            } else {
                removeMenuSelItems();
                dPlayerMgr::Mgr_c::getCurrentPlayer()->addMoney(mSellPrice);
                dSaveTown_c *town = dSaveData_c::getTown();
                town->mShops.mShop.sellAndNotify(
                    dScene::getCurrentScene(), SHOP_SLOT_NONE, 0, 0, mSellPrice,
                    !dAcNpc_c::isPlayerFromThisTown());
            }
            break;
        }
        }
        break;
    case MSG_SELL_NOT_BUYING:
        switch (sel) {
        case 0:
            removeMenuSelItems();
            break;
        }
        break;
    case MSG_PRICE:
    case MSG_PRICE_AGAIN:
    case MSG_PRICE_SPECIAL:
    case MSG_PRICE_SILVER_TOOL_AGAIN:
        switch (sel) {
        case 0:
            buy();
            break;
        case 1:
            getController()->setNextMsgCode(MSG_NOT_BUYING);
            break;
        }
        break;
    case MSG_PRICE_PAPER:
    case MSG_PRICE_PAPER_AGAIN:
        switch (sel) {
        case 0:
            buy();
            break;
        case 1:
            mOnAnswer = &talk_c::openStationery;
            mOnRequestEnd = &talk_c::askPaperAgain;
            break;
        case 2:
            getController()->setNextMsgCode(MSG_NOT_BUYING);
            break;
        }
        break;
    case MSG_PRICE_AFTER_PREVIEW:
        switch (sel) {
        case 0:
            buy();
            break;
        case 1:
            getController()->setNextMsgCode(MSG_NOT_BUYING);
            break;
        }
        getController()->mLock = 1;
        switch (mItem.getKind()) {
        case dItem::KIND_CARPET:
            dBgUtil::setCarpet(dBgiDraw::getStoreCarpet(), 2, 0, 1, 1);
            mRoomPreview = 1;
            dCamera_c::sInstance->mRequestedMode =
                dCamera_c::sInstance->mPreviousMode;
            break;
        case dItem::KIND_WALL:
            dBgUtil::setWallpaper(dBgiDraw::getStoreWallpaper(), 2, 0, 1, 1);
            mRoomPreview = 1;
            dCamera_c::sInstance->mRequestedMode =
                dCamera_c::sInstance->mPreviousMode;
            break;
        }
        break;
    case MSG_PRICE_TOOL:
    case MSG_PRICE_SILVER_TOOL:
        switch (sel) {
        case 0:
            buy();
            break;
        case 1:
            getController()->setNextMsgCode(getHowToMsg(&mItem));
            break;
        case 2:
            getController()->setNextMsgCode(MSG_NOT_BUYING);
            break;
        }
        break;
    case MSG_PRICE_PREVIEW:
        switch (sel) {
        case 0:
            buy();
            break;
        case 1:
            switch (mItem.getKind()) {
            case dItem::KIND_CARPET:
                dBgUtil::setCarpet(mItem, 2, 1, 1, 1);
                mRoomPreview = 1;
                dCamera_c::sInstance->mRequestedMode =
                    dCamera_c::MODE_SHOP_WIDE;
                break;
            case dItem::KIND_WALL:
                dBgUtil::setWallpaper(mItem, 2, 1, 1, 1);
                mRoomPreview = 1;
                dCamera_c::sInstance->mRequestedMode =
                    dCamera_c::MODE_SHOP_CLOSE;
                break;
            }
            getController()->setNextMsgCode(MSG_PRICE_AFTER_PREVIEW);
            break;
        case 2:
            getController()->setNextMsgCode(MSG_NOT_BUYING);
            break;
        }
        break;
    case MSG_ASK_HOW_TO:
        switch (sel) {
        case 0:
            getController()->setNextMsgCode(getHowToMsg(&mItem));
            break;
        case 1:
            getController()->setNextMsgCode(MSG_KNOWS_HOW_TO);
            break;
        }
        break;
    case MSG_PRICE_ROOF_PAINT:
        switch (sel) {
        case 0:
            buy();
            break;
        case 1:
            break;
        }
        break;
    case MSG_ROOF_ASK_1:
    case MSG_ROOF_ASK_2:
    case MSG_ROOF_ASK_3:
    case MSG_ROOF_ASK_4:
    case MSG_ROOF_ASK_5:
        if (sel < ROOF_COLORS_PER_MENU) {
            getPlayerHome()->mNextRoofColor = sel;
        }
        break;
    case MSG_ROOF_COLORS_2:
        if (sel < ROOF_COLORS_PER_MENU) {
            getPlayerHome()->mNextRoofColor = sel + ROOF_COLORS_PER_MENU;
        }
        break;
    case MSG_ROOF_COLORS_3:
        if (sel < ROOF_COLORS_PER_MENU) {
            getPlayerHome()->mNextRoofColor = sel + 2 * ROOF_COLORS_PER_MENU;
        }
        break;
    case MSG_STORE_VOTE_HOURS:
        switch (sel) {
        case 0:
            setShopVote(SHOP_VOTE_HOURS);
            break;
        case 1:
            setShopVote(SHOP_VOTE_BALANCE);
            break;
        case 2:
            setShopVote(SHOP_VOTE_VARIETY);
            break;
        case 3:
            setShopVote(SHOP_VOTE_NOTHING);
            break;
        }
        break;
    case MSG_STORE_VOTE_BALANCE:
        switch (sel) {
        case 0:
            setShopVote(SHOP_VOTE_BALANCE);
            break;
        case 1:
            setShopVote(SHOP_VOTE_VARIETY);
            break;
        case 2:
            setShopVote(SHOP_VOTE_HOURS);
            break;
        case 3:
            setShopVote(SHOP_VOTE_NOTHING);
            break;
        }
        break;
    case MSG_STORE_VOTE_VARIETY:
        switch (sel) {
        case 0:
            setShopVote(SHOP_VOTE_VARIETY);
            break;
        case 1:
            setShopVote(SHOP_VOTE_BALANCE);
            break;
        case 2:
            setShopVote(SHOP_VOTE_HOURS);
            break;
        case 3:
            setShopVote(SHOP_VOTE_NOTHING);
            break;
        }
        break;
    }
}

// Both inline boundaries preserve MWCC's Item temporary order.
inline void dAcNpcSpTanukiBase_c::talk_c::onStoreAnswer(int sel) {
    onStoreAnswer(mMessageCode, sel);
}

// .text 0x51B0
void dAcNpcSpTanukiBase_c::talk_c::onAnswer(int answer) {
    if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
            PRIVATE_FLAG0_PART_TIME_JOB)) {
        onJobAnswer(answer);
    } else if (!(mpOwner->mDrama && mKind == KIND_TALK)) {
        onStoreAnswer(getController()->mSelect.mCursor);
    }
    if (mOnAnswer) {
        (this->*mOnAnswer)();
        mOnAnswer = NULL;
    }
}

// .text 0x5954
void dAcNpcSpTanukiBase_c::talk_c::onJobAnswer(int answer) {
    int sel = getController()->mSelect.mCursor;
    switch (mMessageCode) {
    case JOB_MSG_MORE_PAPER:
        switch (sel) {
        case 0:
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >=
                0) {
                getController()->setNextMsgCode(JOB_MSG_GIVE_PAPER);
            } else {
                getController()->setNextMsgCode(JOB_MSG_PAPER_NO_ROOM);
            }
            break;
        case 1:
            break;
        }
        break;
    }
}

// .text 0x59DC
void dAcNpcSpTanukiBase_c::talk_c::preExecute() {
}

// .text 0x59E0
void dAcNpcSpTanukiBase_c::talk_c::onTalkEnd() {
    if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
            PRIVATE_FLAG0_PART_TIME_JOB)) {
        if (dNpcInfo::tanukichi_c::get()->mJobDone) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->clearFlag0(
                PRIVATE_FLAG0_PART_TIME_JOB);
            getSceneChange()->requestExit(0, 2, 2);
            dPlayerMgr::Mgr_c::getCurrentPlayer()->payDebt(1400);
            mExitRequested = 1;
        }
    } else if (mpOwner->mDrama && mKind == KIND_TALK) {
        mpOwner->mDrama = 0;
    } else if (mKind == KIND_GREET) {
        if (mpOwner->isTanukichi()) {
            dNpcInfo::tanukichi_c *info = dNpcInfo::tanukichi_c::get();
            info->mTwinsGreeted = 0;
        } else {
            dNpcInfo::tanukichi_c *info = dNpcInfo::tanukichi_c::get();
            info->mTwinsGreeted = 1;
        }
    } else if (mKind == KIND_CLOSING) {
        if (dScene::getCurrentScene() == dScene::SCENE_RM_SHOP3_F2) {
            getSceneChange()->requestExit(1, 2, 2);
        } else {
            getSceneChange()->requestExit(0, 2, 2);
        }
        mExitRequested = 1;
    } else if (mKind == KIND_SEE_OFF) {
        getSceneChange()->requestExit(0, 2, 2);
        mExitRequested = 1;
    }
    mKind = KIND_TALK;
    mItem = dItem::ITEM_ID_NONE;
}

// .text 0x5B58
BOOL dAcNpcSpTanukiBase_c::talk_c::buy() {
    int needed = 1;
    if (mItem.getKind() == dItem::KIND_PAINT) {
        needed = 0;
    }
    int free = dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(NULL, NULL);
    if (!canAfford(mPrice)) {
        getController()->setNextMsgCode(MSG_BUY_NO_MONEY);
        return FALSE;
    }
    if (free < needed) {
        getController()->setNextMsgCode(MSG_BUY_NO_ROOM);
        return FALSE;
    }
    dPlayerMgr::Mgr_c::getCurrentPlayer()->payMoney(mPrice, 0);
    if (mItem.getKind() == dItem::KIND_PAINT) {
        int color = dItem::seeker_c::get()->findLike(mItem);
        getPlayerHome()->mNextRoofColor = color;
        getController()->setNextMsgCode(MSG_ROOF_PAINT_BOUGHT);
    } else {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(mItem, 0);
        getController()->setNextMsgCode(MSG_BOUGHT);
    }
    int points = mPrice / 100;
    if (points <= 0) {
        points = 1;
    }
    if (dSaveData_c::getTown()->mShops.mShop.isEventToday()) {
        points *= 5;
    }
    dPlayerMgr::Mgr_c::getCurrentPlayer()->addNookPoints(points);
    if (mItem.getKind() == dItem::KIND_SEED) {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->incNookPurchases();
    }
    int slot = dShopLayout_getSlot(mUnitX, mUnitZ);
    dSaveTown_c *town = dSaveData_c::getTown();
    town->mShops.mShop.sellAndNotify(dScene::getCurrentScene(), (u8)slot,
                                     (u8)mUnitX, (u8)mUnitZ, mPrice,
                                     !dAcNpc_c::isPlayerFromThisTown());
    mBought = 1;
    return TRUE;
}

// .text 0x5D68
u16 dAcNpcSpTanukiBase_c::talk_c::getHowToMsg(dItem::Item *item) {
    switch (item->getKind()) {
    case dItem::KIND_FISHINGROD:
    case dItem::KIND_SILVER_FISHINGROD:
        return MSG_HOW_TO_ROD;
    case dItem::KIND_SCOOP:
        return MSG_HOW_TO_SHOVEL;
    case dItem::KIND_NET:
    case dItem::KIND_SILVER_NET:
        return MSG_HOW_TO_NET;
    case dItem::KIND_WATERING:
        return MSG_HOW_TO_WATERING_CAN;
    case dItem::KIND_AXE:
        return MSG_HOW_TO_AXE;
    case dItem::KIND_TIMER:
        return MSG_HOW_TO_TIMER;
    case dItem::KIND_PACHINKO:
    case dItem::KIND_SILVER_PACHINKO:
        return MSG_HOW_TO_SLINGSHOT;
    }
    return 0;
}

// .text 0x5DEC
void dAcNpcSpTanukiBase_c::talk_c::startGiveSeeds() {
    mItem = dItem::Item((int)dItem::ITEM_IDX_SAPLING);
    mOnMessageEnd = &talk_c::handOver;
    mOnRequestEnd = &talk_c::giveSeeds;
}

// .text 0x5E60
void dAcNpcSpTanukiBase_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x5EC4
void dAcNpcSpTanukiBase_c::talk_c::onCustomTag(int no) {
    if (mpOwner->isTwin()) {
        // The twins finish each other's sentences.
        dItem::Item key;
        switch (no) {
        case 0:
            key = (u16)SP_NPC_MAMEDANUKI1;
            break;
        case 1:
            key = (u16)SP_NPC_MAMEDANUKI0;
            break;
        }
        setVoiceType(dNpcInfo::getSpVoice(&key));
        getController()->updateVoiceType();
    }
}

// .text 0x5F60: the item select filter (TRUE: the item can't be picked)
BOOL dAcNpcSpTanukiBase_c::talk_c::cannotSell(void *pockets, int flag) {
    if (flag != 0) {
        return TRUE;
    }
    const dItem::Item &item = *(const dItem::Item *)pockets;
    if (dItem::isRealItemId(item.mId)) {
        switch (dItem::infoBank_c::get()->getBITM(item)->getKind()) {
        case dItem::KIND_MONEY:
            return TRUE;
        default:
            return FALSE;
        }
    }
    return TRUE;
}

// .text 0x5FFC
void dAcNpcSpTanukiBase_c::talk_c::selectSellItems() {
    if (getController()->mNextMsgCode == 0) {
        getController()->setNextMsgCode(0xFFFF);
    }
    reqSelectItem2(cannotSell, 0x37, 0);
}

// .text 0x6060
void dAcNpcSpTanukiBase_c::talk_c::openCatalog() {
    if (getController()->mNextMsgCode == 0) {
        getController()->setNextMsgCode(0xFFFF);
    }
    reqMenu08(0);
}

// .text 0x60B8
void dAcNpcSpTanukiBase_c::talk_c::openStationery() {
    if (getController()->mNextMsgCode == 0) {
        getController()->setNextMsgCode(0xFFFF);
    }
    static dMail_c l_mail;
    l_mail.clear();
    l_mail.setPaper(&mItem);
    reqMailMenu(&l_mail, 0);
}

// .text 0x6168
void dAcNpcSpTanukiBase_c::talk_c::handOver() {
    requestItemAct(&mItem, 0, 0);
}

// .text 0x6178
void dAcNpcSpTanukiBase_c::talk_c::receiveItem() {
    requestItemActEx(6, &mItem, 0, 0, 0, 2);
}

// .text 0x6194
void dAcNpcSpTanukiBase_c::talk_c::onSellSelected() {
    if (isMenuInvalid()) {
        getController()->setNextMsgCode(MSG_NO_SELLING);
        return;
    }
    int num;
    u16 *items = getMenuSelItems(&num);
    BOOL turnip = FALSE;
    mSellPrice = 0;
    bool important = false;
    BOOL forged = FALSE;
    for (int i = 0; i < num; i++) {
        dItem::Item item = *items;
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
        if (bitm != NULL) {
            switch (bitm->getKind()) {
            case dItem::KIND_KABU:
                turnip = TRUE;
                mSellPrice += mTurnipPrice * item.getPrice();
                break;
            case dItem::KIND_FAKE_PICTURE_BEFORE: {
                dItem::Item forgery((int)dItem::ITEM_IDX_FORGED_PAINTING_00,
                                    dItem::seeker_c::get()->findLike(item),
                                    FALSE);
                mSellPrice += forgery.getPrice() / 4;
                break;
            }
            default:
                mSellPrice += item.getPrice() / 4;
                break;
            }
            switch (bitm->getKind()) {
            case dItem::KIND_SILVER_FISHINGROD:
            case dItem::KIND_GOLD_FISHINGROD:
            case dItem::KIND_SILVER_SCOOP:
            case dItem::KIND_GOLD_SCOOP:
            case dItem::KIND_SILVER_AXE:
            case dItem::KIND_GOLD_AXE:
            case dItem::KIND_SILVER_WATERING:
            case dItem::KIND_GOLD_WATERING:
            case dItem::KIND_SILVER_NET:
            case dItem::KIND_GOLD_NET:
            case dItem::KIND_GOLD_PACHINKO:
            case dItem::KIND_SILVER_PACHINKO:
                important = true;
                break;
            case dItem::KIND_FAKE_PICTURE_BEFORE:
                forged = TRUE;
                break;
            }
            items++;
        }
    }
    if (turnip && dTime_c::getCurrent()->wday == WEEKDAY_SUNDAY) {
        getController()->setNextMsgCode(MSG_WHITE_TURNIP_SUNDAY);
    } else if (important == true) {
        if (num >= 2) {
            getController()->setNextMsgCode(MSG_SELL_IMPORTANT);
        } else {
            getController()->setNextMsgCode(MSG_SELL_IMPORTANT_ONE);
        }
    } else if (forged) {
        // The forged paintings show up as forgeries now.
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        u16 mask = dMenuMng_c::getSelMask();
        for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
            if ((1 << i) & mask) {
                dItem::Item item = player->mPockets[i];
                const dItem::BITM *bitm =
                    dItem::infoBank_c::get()->getBITM(item);
                if (bitm != NULL &&
                    bitm->getKind() == dItem::KIND_FAKE_PICTURE_BEFORE) {
                    dItem::Item forgery((int)dItem::ITEM_IDX_FORGED_PAINTING_00,
                                        dItem::seeker_c::get()->findLike(item),
                                        FALSE);
                    player->setPocket(&forgery, i, FALSE);
                }
            }
        }
        getController()->setNextMsgCode(MSG_FORGED_PAINTING);
    } else if (mSellPrice <= 0) {
        getController()->setNextMsgCode(MSG_SELL_NOT_BUYING);
    } else if (num <= 1) {
        getController()->setNextMsgCode(MSG_SELL_PRICE);
    } else {
        getController()->setNextMsgCode(MSG_SELL_PRICE_ALL);
    }
}

// .text 0x64D8
void dAcNpcSpTanukiBase_c::talk_c::onCatalogSelected() {
    if (isMenuInvalid()) {
        getController()->setNextMsgCode(MSG_NO_ORDER);
        return;
    }
    mItem = getMenuItem();
    mPrice = mItem.getShopPrice();
    if (!canAfford(mPrice)) {
        getController()->setNextMsgCode(MSG_ORDER_NO_MONEY);
    } else {
        getController()->setNextMsgCode(MSG_ORDERED);
        mOrdered = 1;
    }
}

// .text 0x657C
void dAcNpcSpTanukiBase_c::talk_c::askPaperAgain() {
    getController()->setNextMsgCode(MSG_PRICE_PAPER_AGAIN);
}

// .text 0x6588
void dAcNpcSpTanukiBase_c::talk_c::giveItem() {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(mItem, 0);
}

// .text 0x65C0
void dAcNpcSpTanukiBase_c::talk_c::giveErrandItem() {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(mItem, 3);
}

// .text 0x65F8: one random bag of tulips, pansies, cosmos and roses each, and
// three saplings
void dAcNpcSpTanukiBase_c::talk_c::giveSeeds() {
    dItem::Item tulips((int)dItem::ITEM_IDX_RED_TULIP_BAG, cM::rndInt(3),
                       FALSE);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(tulips, 0);
    dItem::Item pansies((int)dItem::ITEM_IDX_WHITE_PANSY_BAG, cM::rndInt(3),
                        FALSE);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(pansies, 0);
    dItem::Item cosmos((int)dItem::ITEM_IDX_WHITE_COSMOS_BAG, cM::rndInt(3),
                       FALSE);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(cosmos, 0);
    dItem::Item roses((int)dItem::ITEM_IDX_RED_ROSE_BAG, cM::rndInt(3), FALSE);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(roses, 0);
    for (int i = 0; i < JOB_SAPLING_NUM; i++) {
        dItem::Item sapling((int)dItem::ITEM_IDX_SAPLING);
        dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(sapling, 0);
    }
}

// .text 0x66F8
void dAcNpcSpTanukiBase_c::talk_c::takeErrandItems() {
    for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getPocketFlag(i) ==
            POCKET_FLAG_ERRAND) {
            dItem::Item none;
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setPocket(&none, i, FALSE);
        }
    }
}

// .text 0x6768
int dAcNpcSpTanukichi_c::getEarType() {
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

// .text 0x67D8
int dAcNpcSpTubuMame_c::getEarType() {
    return NPC_EAR_TYPE_TWINS;
}

// .text 0x67E0
dAcNpcSpTanukichi_c::~dAcNpcSpTanukichi_c() {
}

// .text 0x6918
dAcNpcSpMamekichi_c::~dAcNpcSpMamekichi_c() {
}

// .text 0x6A54
dAcNpcSpTubukichi_c::~dAcNpcSpTubukichi_c() {
}
