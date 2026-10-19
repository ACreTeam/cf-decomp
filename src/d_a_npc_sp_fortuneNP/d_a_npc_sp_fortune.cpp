#include <game/game/d_a_npc_sp_fortune.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene_check.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_sv_activity.hpp>
#include <game/game/d_sv_star_sign.hpp>
#include <game/game/d_sv_villager_move.hpp>
#include <game/cLib/c_math.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/snd/snd_id.hpp>
#include <nw4r/math.h>

// Katrina the fortune teller. Functions are in target .text order.

namespace dANpcSpFortune {
static inline dPrivateData_c *getPlayer() {
    return dPlayerMgr::Mgr_c::getCurrentPlayer();
}
static inline dPrivateData_c *getPlayerRaw() {
    return dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
}
// One of the five readings from base on.
static inline void setFortuneMsg(dMsg::Comp_c *controller, int base) {
    controller->setNextMsgCode(base + cM::rndInt(5));
}
// Her visitor's news for the friend towns (dSvVillagerMove).
static inline void setPlayerValue() {
    dSaveTown_c *town = dSaveData_c::getTown();
    town->mVillagerMove.setPlayerValue(dPlayerMgr::getCurrentPlayerSlot(), 3);
}
// The town's monthly charm (jinx).
static inline dSvActivity_c *getActivity() {
    dSaveTown_c *town = dSaveData_c::getTown();
    return &town->mActivity;
}
// The current player has the price in the pockets.
static inline BOOL canPay(int price) {
    return getPlayer()->getPocketMoney() >= price;
}

// .text 0x5C: the debug page hook (empty in this build).
static BOOL entryDebugPage(dAcNpcSpFortune_c *npc) {
    return FALSE;
}

// .text 0x64
BASE_PROFILE(A_NPC_SP_FORTUNE, dAcNpcSpFortune_c);

// The parameters of the unit (.bss 0x8, built by __sinit).
struct param_s {
    param_s() {
        _00 = 0;
        mPrice = 100;
        mDebugDrop = 0;
        mDropDelay = 60;
        mFallSndDelay = 2;
        mBusyTime = 240;
        mCallDist = 16.0f;
    }

    /* 0x00 */ int _00;
    /* 0x04 */ int mPrice;        // a fortune or a charm costs 100 Bells
    /* 0x08 */ int mDebugDrop;    // Drop_e: drop this all the time (debug)
    /* 0x0C */ int mDropDelay;    // startDrop .. it falls
    /* 0x10 */ int mFallSndDelay; // .. SE_FallObj
    /* 0x14 */ int mBusyTime;     // startDrop .. the talk goes on
    /* 0x18 */ f32 mCallDist; // farther away in x: she calls the player over
}; // size 0x1C

static param_s l_param;
} // namespace dANpcSpFortune

// .text 0x3DC
void dAcNpcSpFortune_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeState(STATE_LOOK);
        break;
    case DEMO_CMD_TALK:
        mFortuneTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case DEMO_CMD_SPEAK:
        mFortuneTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        changeState(STATE_IDLE);
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpFortune {
static const dAcNpcSpFortune_c::State_s l_states[] = {
    {&dAcNpcSpFortune_c::initIdle, &dAcNpcSpFortune_c::executeIdle},
    {&dAcNpcSpFortune_c::initLook, &dAcNpcSpFortune_c::executeLook},
    {&dAcNpcSpFortune_c::initTalk, &dAcNpcSpFortune_c::executeTalk},
};
} // namespace dANpcSpFortune

// .text 0x4A8
void dAcNpcSpFortune_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpFortune::l_states[state].mInit)();
}

// .text 0x4E0
int dAcNpcSpFortune_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mFortuneTalk);
    return 1;
}

// .text 0x52C
int dAcNpcSpFortune_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    bool created = true;
    created &= mBucketMdl.create("obj_fortune_bucket", 0);
    created &= mCanMdl.create("obj_fortune_ittocan", 0);
    created &= mTaraiMdl.create("obj_fortune_tarai", 0);
    if (!created) {
        return 0;
    }
    _1C8C = 0;
    mLookStep = 0;
    mTalkStep = 0;
    mDropKind = DROP_NONE;
    mHitTimer = 0;
    mDropTimer = 0;
    mFallSndTimer = 0;
    mBusyTimer = 0;
    mRollTimer = 0;
    setInteractionRadius(80.0f);
    dSceneCheck::fn_80081858(dANpcSpFortune::entryDebugPage(this));
    return 1;
}

// .text 0x628
int dAcNpcSpFortune_c::doDelete() {
    bool removed = true;
    removed &= mBucketMdl.remove();
    removed &= mCanMdl.remove();
    removed &= mTaraiMdl.remove();
    if (!removed) {
        return 0;
    }
    if (!dAcNpcSp_c::doDelete()) {
        return 0;
    }
    dANpcSpFortune::entryDebugPage(this);
    dSceneCheck::fn_8008185C();
    return 1;
}

// .text 0x6C8
int dAcNpcSpFortune_c::vtB0() {
    if (mState < STATE_COUNT) {
        (this->*dANpcSpFortune::l_states[mState].mExecute)();
    }
    if (dANpcSpFortune::l_param.mDebugDrop != DROP_NONE &&
        mDropKind != dANpcSpFortune::l_param.mDebugDrop) {
        startDrop(dANpcSpFortune::l_param.mDebugDrop);
    }
    if (mDropTimer > 0) {
        mDropTimer--;
    }
    if (mDropTimer <= 0) {
        if (mHitTimer > 0 && --mHitTimer == 0) {
            dPlayerMgr::startFortuneHit();
            switch (mDropKind) {
            case DROP_NONE:
                break;
            case DROP_BUCKET:
                mSnd.startSound(SE_BucketHitHead);
                break;
            case DROP_CAN:
                mSnd.startSound(SE_KanHitHead);
                break;
            case DROP_TARAI:
                mSnd.startSound(SE_TaraiHitHead);
                break;
            }
            mRollTimer = 44;
        }
        if (mRollTimer > 0 && --mRollTimer == 0) {
            switch (mDropKind) {
            case DROP_NONE:
                break;
            case DROP_BUCKET:
                mSnd.startSound(SE_BucketKorogaru);
                break;
            case DROP_CAN:
                mSnd.startSound(SE_KanKorogaru);
                break;
            case DROP_TARAI:
                mSnd.startSound(SE_TaraiKorogaru);
                break;
            }
        }
        mVec3_c pos;
        pos = mPos;
        switch (mDropKind) {
        case DROP_NONE:
            break;
        case DROP_BUCKET:
            mBucketMdl.play();
            pos = *mBucketMdl.getNodePos(1);
            break;
        case DROP_CAN:
            mCanMdl.play();
            pos = *mCanMdl.getNodePos(1);
            break;
        case DROP_TARAI:
            mTaraiMdl.play();
            pos = *mTaraiMdl.getNodePos(1);
            break;
        }
        if (mDropKind > DROP_NONE) {
            mSnd.setPos(&pos);
        }
        if (mFallSndTimer > 0 && --mFallSndTimer <= 0) {
            mSnd.startSound(SE_FallObj);
        }
    }
    if (mBusyTimer > 0) {
        mBusyTimer--;
    }
    return 1;
}

// .text 0x970
int dAcNpcSpFortune_c::draw() {
    if (mDropTimer <= 0) {
        switch (mDropKind) {
        case DROP_NONE:
            break;
        case DROP_BUCKET:
            mBucketMdl.entry();
            break;
        case DROP_CAN:
            mCanMdl.entry();
            break;
        case DROP_TARAI:
            mTaraiMdl.entry();
            break;
        }
    }
    return dAcNpc_c::draw();
}

// .text 0x9F8
void dAcNpcSpFortune_c::initIdle() {
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
}

// .text 0xA0C
void dAcNpcSpFortune_c::executeIdle() {
}

// .text 0xA10
void dAcNpcSpFortune_c::initLook() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
    mLookStep = 0;
}

// .text 0xA74
void dAcNpcSpFortune_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        mLookStep++;
    }
}

namespace dANpcSpFortune {
static const dAcNpcSpFortune_c::Proc l_lookSteps[] = {
    &dAcNpcSpFortune_c::stepLook};
} // namespace dANpcSpFortune

// .text 0xAD4
void dAcNpcSpFortune_c::executeLook() {
    if (mLookStep < ARRAY_SIZE(dANpcSpFortune::l_lookSteps)) {
        (this->*dANpcSpFortune::l_lookSteps[mLookStep])();
    }
}

// .text 0xB14
void dAcNpcSpFortune_c::initTalk() {
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, FALSE);
    mTalkStep = 0;
}

// .text 0xB80: once the talk is over, turn back to the front.
void dAcNpcSpFortune_c::stepTalk() {
    if (!mTalk.isBusy()) {
        mAng angle(0);
        mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0,
                            cNpcMorphFrames);
        mTalkStep++;
    }
}

// .text 0xBF0
void dAcNpcSpFortune_c::stepTalkEnd() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        dDemo::finishTalk(this);
        mTalkStep++;
    }
}

namespace dANpcSpFortune {
static const dAcNpcSpFortune_c::Proc l_talkSteps[] = {
    &dAcNpcSpFortune_c::stepTalk, &dAcNpcSpFortune_c::stepTalkEnd};
} // namespace dANpcSpFortune

// .text 0xC40
void dAcNpcSpFortune_c::executeTalk() {
    if (mTalkStep < ARRAY_SIZE(dANpcSpFortune::l_talkSteps)) {
        (this->*dANpcSpFortune::l_talkSteps[mTalkStep])();
    }
}

// .text 0xC80: drops the object on the current player's head.
void dAcNpcSpFortune_c::startDrop(int kind) {
    mDropKind = kind;
    mMtx_c mtx;
    mtx.trans(*dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT));
    mtx.YrotM(dPlayerMgr::getYaw(dPlayerMgr::PLAYER_INDEX_CURRENT));
    switch (mDropKind) {
    case DROP_NONE:
        break;
    case DROP_BUCKET:
        mBucketMdl.setAnm("obj_fortune_bucket_0", m3d::FORWARD_ONCE, 0.0f);
        mBucketMdl.mMdl.setLocalMtx(&mtx);
        break;
    case DROP_CAN:
        mCanMdl.setAnm("obj_fortune_ittocan_0", m3d::FORWARD_ONCE, 0.0f);
        mCanMdl.mMdl.setLocalMtx(&mtx);
        break;
    case DROP_TARAI:
        mTaraiMdl.setAnm("obj_fortune_tarai_0", m3d::FORWARD_ONCE, 0.0f);
        mTaraiMdl.mMdl.setLocalMtx(&mtx);
        break;
    }
    mDropTimer = dANpcSpFortune::l_param.mDropDelay;
    mFallSndTimer = dANpcSpFortune::l_param.mFallSndDelay;
    mHitTimer = 30;
    mBusyTimer = dANpcSpFortune::l_param.mBusyTime;
}

// .text 0xDBC
BOOL dAcNpcSpFortune_c::isDropDone() {
    return mBusyTimer <= 0;
}

// .text 0xDD0
dAcNpcSpFortune_c::talk_c::talk_c(dAcNpcSpFortune_c *npc) : mpOwner(npc) {
    init();
    mGreeted = 0;
    mPaid = 0;
    mCharmPaid = 0;
    mFaint = 0;
}

// .text 0xE44
dAcNpcSpFortune_c::talk_c::~talk_c() {
}

// .text 0xE9C
void dAcNpcSpFortune_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgCode = 0;
    mNextMsg = 0;
    mOnMessageEnd = NULL;
    mOnAnswer = NULL;
    mOnRequestEnd = NULL;
    mCallOver = 0;
    mBgmMuted = 0;
}

// .text 0xF28
void dAcNpcSpFortune_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    if (!mPaid && !mCharmPaid) {
        dAcNpcSpFortune_c *owner = mpOwner;
        if (nw4r::math::FAbs(
                dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT)->x -
                owner->mPos.x) > dANpcSpFortune::l_param.mCallDist) {
            mMsgCode = MSG_COME_CLOSER;
            mCallOver = 1;
            return;
        }
    }
    if (!dANpcSpFortune::getPlayerRaw()->isFlag0(PRIVATE_FLAG0_KATRINA_MET)) {
        mMsgCode = MSG_FIRST_MEETING;
    } else if (!mGreeted) {
        mMsgCode = MSG_WELCOME_BACK;
    } else if (mPaid) {
        mMsgCode = MSG_AFTER_FORTUNE;
    } else if (mCharmPaid) {
        mMsgCode = MSG_AFTER_CHARM;
    } else {
        mMsgCode = MSG_HELLO;
    }
}

// .text 0x103C
void dAcNpcSpFortune_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_hakkemie";
    info->mCode = mMsgCode;
}

// .text 0x1054
void dAcNpcSpFortune_c::talk_c::onMessageStart(int arg) {
    if (mFaint) {
        dPlayerMgr::startFortuneFaint();
    }
    switch (mMessageCode) {
    case MSG_FORTUNE_SEEING:
    case MSG_FORTUNE_BIRTH_SIGN:
    case MSG_CHARM_SEEING:
        if (mBgmMuted == TRUE) {
            dBgm::l_mgr.mStgFortune.unmute2();
            mBgmMuted = 0;
        }
        break;
    case MSG_CHARM_CONDITION + JINX_COND_PULL_WEEDS:
    case MSG_CHARM_CONDITION + JINX_COND_PLANT_TREES:
    case MSG_CHARM_CONDITION + JINX_COND_PLANT_FLOWERS:
        formatNumber(dANpcSpFortune::getActivity()->mJinx.mValue, 0, 2,
                     dScript::NUM_FORMAT_REGION);
        break;
    case MSG_CHARM_CONDITION + JINX_COND_CATCH_BUG: {
        dItem::Item bug(dANpcSpFortune::getActivity()->mJinx.mValue +
                        dItem::ITEM_IDX_COMMON_BUTTERFLY);
        setItemName(bug, 2);
        break;
    }
    case MSG_CHARM_CONDITION + JINX_COND_CATCH_FISH: {
        dItem::Item fish(dANpcSpFortune::getActivity()->mJinx.mValue +
                         dItem::ITEM_IDX_BITTERLING);
        setItemName(fish, 2);
        break;
    }
    }

    int num = 0;
    for (int i = 0; i < dSvStarSign::SIGN_NUM; i++) {
        if (dSaveData_c::getRaw()->getStarSigns()->isValid(i)) {
            num++;
        }
    }
    int sign = dSaveData_c::getRaw()->getStarSigns()->getRandom(-1);
    int sign2 = dSaveData_c::getRaw()->getStarSigns()->getRandom(sign);
    int rnd = -1;
    if (sign >= 0) {
        dString::Word_c word(
            dSaveData_c::getRaw()->getStarSigns()->get(sign)->getName());
        mpController->setWord(0, &word);
    } else {
        rnd = cM::rndInt(12);
        mpController->setStringWord(0, rnd + 1, "sys_STRING/STR_Constellation");
    }
    if (sign2 >= 0) {
        dString::Word_c word(
            dSaveData_c::getRaw()->getStarSigns()->get(sign)->getName());
        mpController->setWord(1, &word);
    } else {
        int rnd2 = cM::rndInt(11);
        if (rnd2 >= rnd) {
            rnd2++;
        }
        mpController->setStringWord(1, rnd2 + 1,
                                    "sys_STRING/STR_Constellation");
    }
    setMonthName(dANpcSpFortune::getActivity()->mMonth, 3);
}

namespace dANpcSpFortune {
// .text 0x1308: the villagers want to talk more to a lucky player.
static void applyFortune(int fortune) {
    if (fortune == FORTUNE_LOVE || fortune == FORTUNE_FRIENDSHIP ||
        fortune == FORTUNE_ITEM) {
        dPrivateData_c *player = getPlayer();
        for (int i = 0; i < ANIMAL_NUM; i++) {
            dAnimal_c *animal =
                dSaveData_c::getTown()->mAnimals.mTown.getAnimal(i);
            if (animal != NULL && animal->mID.isValid()) {
                if (fortune == FORTUNE_LOVE &&
                    animal->mID.getGender(1) ==
                        player->mPID.player.getGender()) {
                    continue;
                }
                if (fortune == FORTUNE_FRIENDSHIP &&
                    animal->mID.getGender(1) !=
                        player->mPID.player.getGender()) {
                    continue;
                }
                dNpc::getEntryByAnimal(&animal->mID)->mTalkWeight.twice();
            }
        }
    }
}
} // namespace dANpcSpFortune

// .text 0x13F0
void dAcNpcSpFortune_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_FORTUNE_TRANCE:
        mpOwner->startRandomDrop();
        mpController->setNextMsgCode(dMsg::MSG_CODE_INVALID);
        mpController->mLock = 1;
        mNextMsg = MSG_FORTUNE_SEEING;
        break;
    case MSG_FORTUNE_SEEING: {
        dPrivateData_c *player = dANpcSpFortune::getPlayer();
        int birthSign = dTime_c::getStarSign(player->mBirthday.mMonth,
                                             player->mBirthday.mDay);
        dTime_c now = *dTime_c::getCurrent();
        if (birthSign == dTime_c::getStarSign((u8)now.month, (u8)now.mday)) {
            mpController->setNextMsgCode(MSG_FORTUNE_BIRTH_SIGN);
        } else {
            int fortune;
            int rnd = cM::rndInt(100);
            if (rnd < 45) {
                fortune = FORTUNE_NONE;
                dANpcSpFortune::setFortuneMsg(mpController, MSG_FORTUNE_NONE);
            } else if (rnd < 50) {
                fortune = FORTUNE_BAD;
                dANpcSpFortune::setFortuneMsg(mpController, MSG_FORTUNE_BAD);
            } else if (rnd < 63) {
                fortune = FORTUNE_LOVE;
                dANpcSpFortune::setFortuneMsg(mpController, MSG_FORTUNE_LOVE);
            } else if (rnd < 76) {
                fortune = FORTUNE_FRIENDSHIP;
                dANpcSpFortune::setFortuneMsg(mpController,
                                              MSG_FORTUNE_FRIENDSHIP);
            } else if (rnd < 90) {
                fortune = FORTUNE_MONEY;
                dANpcSpFortune::setFortuneMsg(mpController, MSG_FORTUNE_MONEY);
            } else {
                fortune = FORTUNE_ITEM;
                dANpcSpFortune::setFortuneMsg(mpController, MSG_FORTUNE_ITEM);
            }
            player->mFortune = fortune;
            dANpcSpFortune::applyFortune(fortune);
        }
        mFaint = 1;
        break;
    }
    case MSG_FORTUNE_BIRTH_SIGN: {
        int fortune;
        int rnd = cM::rndInt(100);
        if (rnd < 55) {
            fortune = FORTUNE_NONE;
            dANpcSpFortune::setFortuneMsg(mpController, MSG_FORTUNE_NONE);
        } else if (rnd < 65) {
            fortune = FORTUNE_BAD;
            dANpcSpFortune::setFortuneMsg(mpController, MSG_FORTUNE_BAD);
        } else if (rnd < 75) {
            fortune = FORTUNE_LOVE;
            dANpcSpFortune::setFortuneMsg(mpController, MSG_FORTUNE_LOVE);
        } else if (rnd < 85) {
            fortune = FORTUNE_FRIENDSHIP;
            dANpcSpFortune::setFortuneMsg(mpController, MSG_FORTUNE_FRIENDSHIP);
        } else if (rnd < 95) {
            fortune = FORTUNE_MONEY;
            dANpcSpFortune::setFortuneMsg(mpController, MSG_FORTUNE_MONEY);
        } else {
            fortune = FORTUNE_ITEM;
            dANpcSpFortune::setFortuneMsg(mpController, MSG_FORTUNE_ITEM);
        }
        dANpcSpFortune::getPlayer()->mFortune = fortune;
        dANpcSpFortune::applyFortune(fortune);
        break;
    }
    case MSG_CHARM_TRANCE:
        mpOwner->startRandomDrop();
        mpController->setNextMsgCode(dMsg::MSG_CODE_INVALID);
        mpController->mLock = 1;
        mNextMsg = MSG_CHARM_SEEING;
        break;
    case MSG_CHARM_SEEING:
        mpController->setNextMsgCode(
            MSG_CHARM_OUTCOME + dANpcSpFortune::getActivity()->mJinx.mOutcome);
        mFaint = 1;
        break;
    case MSG_CHARM_TOLD:
        mpController->setNextMsgCode(
            MSG_CHARM_OUTCOME + dANpcSpFortune::getActivity()->mJinx.mOutcome);
        break;
    case MSG_FORTUNE_PAY:
        mPaid = 1;
        dANpcSpFortune::getPlayer()->setFortuneWeek();
        dANpcSpFortune::getPlayer()->payMoney(dANpcSpFortune::l_param.mPrice,
                                              FALSE);
        mpController->setNextMsgCode(MSG_FORTUNE_BYE);
        dANpcSpFortune::setPlayerValue();
        break;
    case MSG_CHARM_NOT_YOU:
        if (dANpcSpFortune::getPlayerRaw()->isFlag0(PRIVATE_FLAG0_CHARM_TOLD)) {
            mpController->setNextMsgCode(MSG_CHARM_FREE);
        } else {
            mpController->setNextMsgCode(MSG_CHARM_PAY);
        }
        break;
    case MSG_CHARM_PAY:
        mCharmPaid = 1;
        dANpcSpFortune::getPlayer()->payMoney(dANpcSpFortune::l_param.mPrice,
                                              FALSE);
        mpController->setNextMsgCode(MSG_CHARM_BYE);
        dANpcSpFortune::setPlayerValue();
        dANpcSpFortune::getPlayer()->setFlag0(PRIVATE_FLAG0_CHARM_TOLD);
        break;
    case MSG_CHARM_FREE:
        mpController->setNextMsgCode(MSG_CHARM_BYE);
        dANpcSpFortune::setPlayerValue();
        break;
    }
    if (mMessageCode >= MSG_CHARM_OUTCOME &&
        mMessageCode < MSG_CHARM_OUTCOME + JINX_OUTCOME_NUM) {
        mpController->setNextMsgCode(
            MSG_CHARM_CONDITION +
            dANpcSpFortune::getActivity()->mJinx.mCondition);
    }
    if (mMessageCode >= MSG_CHARM_CONDITION &&
        mMessageCode < MSG_CHARM_CONDITION + JINX_COND_NUM) {
        mpController->setNextMsgCode(MSG_CHARM_NOT_YOU);
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0x19FC
void dAcNpcSpFortune_c::talk_c::onAnswer(int arg) {
    int answer = mpController->getAnswer();
    switch (mMessageCode) {
    case MSG_INTRO:
    case MSG_WELCOME_BACK:
    case MSG_HELLO:
        switch (answer) {
        case ANSWER_FORTUNE:
            if (!dANpcSpFortune::getPlayer()->isNewFortuneWeek()) {
                mpController->setNextMsgCode(MSG_TOO_SOON);
            } else if (!dANpcSpFortune::canPay(
                           dANpcSpFortune::l_param.mPrice)) {
                mpController->setNextMsgCode(MSG_NO_MONEY);
            } else {
                mpController->setNextMsgCode(MSG_FORTUNE_TRANCE);
            }
            break;
        case ANSWER_CHARM:
            if (dANpcSpFortune::getPlayerRaw()->isFlag0(
                    PRIVATE_FLAG0_CHARM_TOLD)) {
                mpController->setNextMsgCode(MSG_CHARM_TOLD);
            } else if (!dANpcSpFortune::canPay(
                           dANpcSpFortune::l_param.mPrice)) {
                mpController->setNextMsgCode(MSG_NO_MONEY);
            } else {
                mpController->setNextMsgCode(MSG_CHARM_TRANCE);
            }
            break;
        case ANSWER_NOTHING:
            break;
        }
        break;
    }
    if (mOnAnswer) {
        (this->*mOnAnswer)();
        mOnAnswer = NULL;
    }
}

// .text 0x1B74: the trance message waits for the object to fall.
void dAcNpcSpFortune_c::talk_c::preExecute() {
    if (mpController->mLock && mpOwner->isDropDone()) {
        mpController->setNextMsgCode(mNextMsg);
        mpController->mLock = 0;
        mpController->mForceNext = 1;
    }
}

// .text 0x1BE0
void dAcNpcSpFortune_c::talk_c::onTalkEnd() {
    if (!mCallOver) {
        dANpcSpFortune::getPlayer()->setFlag0(PRIVATE_FLAG0_KATRINA_MET);
        mGreeted = 1;
    }
}

// .text 0x1C28: tag 0 (the trance) mutes the BGM.
void dAcNpcSpFortune_c::talk_c::onCustomTag(int no) {
    switch (no) {
    case TAG_TRANCE:
        if (!mBgmMuted) {
            dBgm::l_mgr.mStgFortune.mute();
            mBgmMuted = 1;
        }
        break;
    }
}

// .text 0x1C80
void dAcNpcSpFortune_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x1CE4
int dAcNpcSpFortune_c::getEarType() {
    return NPC_EAR_TYPE_KATRINA;
}

#ifdef MUST_MATCH
// Matching-only emission order (the d_a_npc_sp_cafe / d_a_str convention).
// There is no evidence for this helper in the original source. MWCC drops
// the unused function; its reference puts anmChr_c::getType before
// anmVis_c::getType among the retained weak inlines.
static void orderFortuneWeakInlines(m3d::anmChr_c *anm) {
    anm->m3d::anmChr_c::getType();
}
#endif
