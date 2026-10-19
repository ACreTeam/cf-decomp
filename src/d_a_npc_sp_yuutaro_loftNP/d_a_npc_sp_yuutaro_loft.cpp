#include <game/game/d_a_npc_sp_yuutaro_loft.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <game/snd/snd_id.hpp>

// Wisp in the attic. Functions are in target .text order.

namespace dANpcSpYuutaroLoft {
// .text 0x5C
BASE_PROFILE(A_NPC_SP_YUUTARO_LOFT, dAcNpcSpYuutaroLoft_c);
} // namespace dANpcSpYuutaroLoft

// .text 0x1D0
void dAcNpcSpYuutaroLoft_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeState(STATE_LOOK);
        break;
    case DEMO_CMD_TALK:
    case DEMO_CMD_SPEAK:
        mWispTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        changeState(STATE_WAIT);
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

// .text 0x278
int dAcNpcSpYuutaroLoft_c::canSpeak(dDemoActor_c *actor) {
    return TRUE;
}

namespace dANpcSpYuutaroLoft {
static const dAcNpcSpYuutaroLoft_c::State_s l_states[] = {
    {&dAcNpcSpYuutaroLoft_c::initWait, &dAcNpcSpYuutaroLoft_c::executeWait},
    {&dAcNpcSpYuutaroLoft_c::initLook, &dAcNpcSpYuutaroLoft_c::executeLook},
    {&dAcNpcSpYuutaroLoft_c::initTalk, &dAcNpcSpYuutaroLoft_c::executeTalk}};
} // namespace dANpcSpYuutaroLoft

// .text 0x280: the magic lamp in pocket lampSlot was used (d_menu_item00):
// stand next to the player in the attic and speak to him.
void dAcNpcSpYuutaroLoft_c::summon(int lampSlot) {
    mAngle.y = 0;
    mAngle3D.y = 0;
    mVec3_c *playerPos =
        dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (playerPos) {
        nw4r::math::VEC3 pos;
        if (worldToUnit(playerPos->x) >= 8) {
            dFdBase_c::getUnitGroundPos(&pos, 6, 13);
            pos.x += 8.0f;
        } else {
            dFdBase_c::getUnitGroundPos(&pos, 9, 13);
            pos.x -= 8.0f;
        }
        mPos = pos;
        mLastPos = pos;
        mSummoned = 1;
        mLampSlot = lampSlot;
    }
}

// .text 0x374
void dAcNpcSpYuutaroLoft_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpYuutaroLoft::l_states[state].mInit)();
}

// .text 0x3AC
int dAcNpcSpYuutaroLoft_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mWispTalk);
    return SUCCEEDED;
}

// .text 0x3F8: only in the player's own house.
int dAcNpcSpYuutaroLoft_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    if (!isPlayerFromThisTown()) {
        return FAILED;
    }
    if (!dScene::isCurrentSceneMyHouse()) {
        return FAILED;
    }
    changeState(STATE_WAIT);
    mFootstep.mEnable = 0;
    mLampSlot = -1;
    return SUCCEEDED;
}

// .text 0x47C
int dAcNpcSpYuutaroLoft_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x4A8
int dAcNpcSpYuutaroLoft_c::vtB0() {
    if (mState < STATE_NUM) {
        (this->*dANpcSpYuutaroLoft::l_states[mState].mExecute)();
        if (mSummoned) {
            dDemo::requestSpeak(this, NULL);
            if (isNetActive()) {
                setDemoFlag8();
            } else {
                clearDemoFlag8();
            }
        }
    }
    return 1;
}

// .text 0x534
void dAcNpcSpYuutaroLoft_c::initWait() {
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mObjc.mType &= ~dObjc::MASK_DEFAULT;
    mIsActive = 0;
    mDrawShadow = 0;
}

// .text 0x590
void dAcNpcSpYuutaroLoft_c::executeWait() {
}

// .text 0x594
void dAcNpcSpYuutaroLoft_c::initLook() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
    mStep = LOOK_STEP_TURN;
}

// .text 0x5F8
void dAcNpcSpYuutaroLoft_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        ++mStep;
    }
}

namespace dANpcSpYuutaroLoft {
static const dAcNpcSpYuutaroLoft_c::Proc l_lookSteps[] = {
    &dAcNpcSpYuutaroLoft_c::stepLook};
} // namespace dANpcSpYuutaroLoft

// .text 0x658
void dAcNpcSpYuutaroLoft_c::executeLook() {
    if (mStep < LOOK_STEP_NUM) {
        (this->*dANpcSpYuutaroLoft::l_lookSteps[mStep])();
    }
}

// .text 0x698
void dAcNpcSpYuutaroLoft_c::initTalk() {
    mSummoned = 0;
    if (isNetActive()) {
        mStep = TALK_STEP_NET_TALK;
        mTalk.requestTalk(true, FALSE);
    } else {
        mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_GST_APPEAR1, 0.0f, 0.0f,
                           m3d::PLAYMODE_INHERIT, 0);
        mStep = TALK_STEP_APPEAR;
        mAudioObj.startSound(SE_YuutarouAppear);
    }
}

// .text 0x738
void dAcNpcSpYuutaroLoft_c::stepTalkAppear() {
    if (!mIsActive) {
        mIsActive = 1;
        mDrawShadow = 1;
        _1B68 = 0.0f;
    } else {
        _1B68 = getBodyScale();
        if (getModel()->mAnm.checkFrame(56.0f)) {
            mAng3_c angle(0, mAngle.y.mAngle, 0);
            dEffect::create("af_gst_apear", &mPos, &angle, NULL);
        }
    }
    if (mAction.mActionId == ACTION_ANM && mAction.mCanChange) {
        turnTalk(getTalkAngle(), FALSE);
        mStep = TALK_STEP_TALK;
        _1B68 = 1.0f;
    }
}

// .text 0x8D8
void dAcNpcSpYuutaroLoft_c::stepTalk() {
    if (!mTalk.isBusy()) {
        mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_GST_DISAPPEAR1, 0.0f, 15.0f,
                           m3d::PLAYMODE_INHERIT, 0);
        mStep = TALK_STEP_DISAPPEAR;
    }
}

// .text 0x940
void dAcNpcSpYuutaroLoft_c::stepTalkDisappear() {
    _1B68 = getBodyScale();
    if (getModel()->mAnm.checkFrame(85.0f)) {
        mAng3_c angle(0, mAngle.y.mAngle, 0);
        dEffect::create("af_gst_disappear", &mPos, &angle, NULL);
    }
    if (getModel()->mAnm.checkFrame(31.0f)) {
        mAudioObj.startSound(SE_YuutarouDisappear);
    }
    if (mAction.mActionId == ACTION_ANM && mAction.mCanChange) {
        dDemo::finishTalk(this);
        mStep = TALK_STEP_NUM;
        mIsActive = 0;
        mDrawShadow = 0;
    }
}

// .text 0xAC0
void dAcNpcSpYuutaroLoft_c::stepNetTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
        mStep = TALK_STEP_NUM;
    }
}

namespace dANpcSpYuutaroLoft {
static const dAcNpcSpYuutaroLoft_c::Proc l_talkSteps[] = {
    &dAcNpcSpYuutaroLoft_c::stepTalkAppear, &dAcNpcSpYuutaroLoft_c::stepTalk,
    &dAcNpcSpYuutaroLoft_c::stepTalkDisappear,
    &dAcNpcSpYuutaroLoft_c::stepNetTalk};
} // namespace dANpcSpYuutaroLoft

// .text 0xB08
void dAcNpcSpYuutaroLoft_c::executeTalk() {
    if (mStep < TALK_STEP_NUM) {
        (this->*dANpcSpYuutaroLoft::l_talkSteps[mStep])();
    }
}

namespace dANpcSpYuutaroLoft {
// .text 0xB48: pocket filter, the magic lamp.
static BOOL isMagicLamp(const dItem::Item *item) {
    return item->isSame(dItem::Item(dItem::ITEM_IDX_MAGIC_LAMP));
}
} // namespace dANpcSpYuutaroLoft

// .text 0xB88: the wish is granted: empty the rubbed lamp's pocket (else the
// first pocket with a magic lamp).
void dAcNpcSpYuutaroLoft_c::talk_c::takeLamp() {
    int slot;
    BOOL search;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    search = TRUE;
    slot = mpOwner->mLampSlot;
    if (slot != -1) {
        if (dItem::Item(dItem::ITEM_IDX_MAGIC_LAMP) ==
            player->mPockets[slot].getId()) {
            search = FALSE;
        }
    }
    if (search) {
        u16 mask;
        player->countPockets(dANpcSpYuutaroLoft::isMagicLamp, &mask);
        for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
            if (mask & (1 << i)) {
                player->clearPocket(i);
                break;
            }
        }
    } else {
        player->clearPocket(slot);
    }
}

// .text 0xC8C
dAcNpcSpYuutaroLoft_c::talk_c::talk_c(dAcNpcSpYuutaroLoft_c *npc)
    : mpOwner(npc) {
    init();
}

// .text 0xCEC
dAcNpcSpYuutaroLoft_c::talk_c::~talk_c() {
}

// .text 0xD44
void dAcNpcSpYuutaroLoft_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageStart = NULL;
    mOnMessageEnd = NULL;
    mOnAnswer = NULL;
    mOnRequestEnd = NULL;
}

// .text 0xDDC
void dAcNpcSpYuutaroLoft_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    if (isNetActive()) {
        if (dNet::getMemberNum() > 1) {
            mMsgNo = MSG_NET_GUESTS;
        } else {
            mMsgNo = MSG_NET_ALONE;
        }
    } else if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
                   PRIVATE_FLAG0_WISP_SUMMONED)) {
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
            PRIVATE_FLAG0_WISP_SUMMONED);
        mMsgNo = MSG_FIRST_APPEAR;
    } else {
        mMsgNo = MSG_APPEAR;
    }
}

// .text 0xE88
void dAcNpcSpYuutaroLoft_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_yutarou";
    info->mCode = mMsgNo;
}

// .text 0xEA0
void dAcNpcSpYuutaroLoft_c::talk_c::onMessageStart(int arg) {
    if (mOnMessageStart) {
        (this->*mOnMessageStart)();
        mOnMessageStart = NULL;
    }
}

// .text 0xF04
void dAcNpcSpYuutaroLoft_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_FIRST_APPEAR:
        mpController->setNextMsgCode(MSG_FIRST_GREET);
        break;
    case MSG_FIRST_GREET:
    case MSG_WEED_NONE:
    case MSG_ROACH_NONE:
    case MSG_NO_ITEM:
    case MSG_SKIP:
        mpController->setNextMsgCode(MSG_ASK_WISH);
        break;
    case MSG_STUFF:
        mpController->setNextMsgCode(MSG_ASK_ITEM);
        break;
    case MSG_GIVE_FTR:
    case MSG_GIVE_CAP:
    case MSG_GIVE_ACC:
    case MSG_GIVE_CLOTH:
        mpController->setNextMsgCode(MSG_TAKE_LAMP);
        break;
    case MSG_WEED_DONE:
    case MSG_ROACH_DONE:
    case MSG_NO_WISH:
    case MSG_TAKE_LAMP:
        mpController->setNextMsgCode(MSG_LEAVE);
        break;
    case MSG_APPEAR:
        mpController->setNextMsgCode(MSG_SKIP);
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
    }
}

// .text 0xFD0
void dAcNpcSpYuutaroLoft_c::talk_c::onAnswer(int arg) {
    u8 answer = mpController->getAnswer();
    if (mMessageCode == MSG_ASK_ITEM) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        int kind;
        int from;
        switch (answer) {
        case ITEM_ANSWER_FTR:
            kind = dItem::KIND_FTR;
            from = dItem::FROM_GROUP_ABC_ALL;
            break;
        case ITEM_ANSWER_CAP:
            kind = dItem::KIND_CAP;
            from = dItem::FROM_TAILOR;
            break;
        case ITEM_ANSWER_ACC:
            kind = dItem::KIND_ACC;
            from = dItem::FROM_TAILOR;
            break;
        case ITEM_ANSWER_CLOTH:
            kind = dItem::KIND_CLOTH;
            from = dItem::FROM_GROUP_ABC_ALL;
            break;
        case ITEM_ANSWER_NONE:
            mpController->setNextMsgCode(MSG_NO_ITEM);
            return;
        }
        dItemSel::Range_c range(kind, from);
        dItemSel::Filter_c filter(&player->mCatalog, NULL,
                                  dItemSel::EXCLUDE_NOT_FOR_SALE);
        dItem::Item item;
        dItemSel::select(&item, 1, &range, 1, &filter, NULL, 0, NULL);
        takeLamp();
        player->pickUp(&item, FALSE);
        setItemName(item, 0);
        switch (kind) {
        case dItem::KIND_FTR:
            mpController->setNextMsgCode(MSG_GIVE_FTR);
            break;
        case dItem::KIND_CAP:
            mpController->setNextMsgCode(MSG_GIVE_CAP);
            break;
        case dItem::KIND_ACC:
            mpController->setNextMsgCode(MSG_GIVE_ACC);
            break;
        case dItem::KIND_CLOTH:
            mpController->setNextMsgCode(MSG_GIVE_CLOTH);
            break;
        }
    } else {
        switch (answer) {
        case WISH_WEED:
            if (fgMngProc_getWeedNum() == 0) {
                mpController->setNextMsgCode(MSG_WEED_NONE);
            } else {
                takeLamp();
                dSaveData_c::getTown()->setFlag(SAVE_FLAG_WISP_WEEDING);
                mpController->setNextMsgCode(MSG_WEED_DONE);
            }
            break;
        case WISH_ROACH: {
            dHome_c *home =
                dSaveData_c::getTown()->mHomes.getCurrentPlayerHome();
            if (!home->hasFlags()) {
                mpController->setNextMsgCode(MSG_ROACH_NONE);
            } else {
                takeLamp();
                mpController->setNextMsgCode(MSG_ROACH_DONE);
                home->clearFlags();
            }
            break;
        }
        case WISH_STUFF:
            mpController->setNextMsgCode(MSG_STUFF);
            break;
        case WISH_NONE:
            mpController->setNextMsgCode(MSG_NO_WISH);
            break;
        }
    }
    if (mOnAnswer) {
        (this->*mOnAnswer)();
        mOnAnswer = NULL;
    }
}

// .text 0x128C
void dAcNpcSpYuutaroLoft_c::talk_c::preExecute() {
}

// .text 0x1290
void dAcNpcSpYuutaroLoft_c::talk_c::onTalkEnd() {
}

// .text 0x1294
void dAcNpcSpYuutaroLoft_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}
