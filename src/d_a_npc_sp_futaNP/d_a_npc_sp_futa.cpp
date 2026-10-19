#include <game/game/d_a_npc_sp_futa.hpp>
#include <game/cLib/c_math.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_hand_item.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_museum.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_scene_check.hpp>

namespace dANpcSpFuta {
// 0x5C
static BOOL isUnidentifiedFossil(const dItem::Item *item) {
    return item->getKind() == dItem::KIND_BEFORE_FOSSIL;
}

// 0x88
static BOOL entryDebugPage(dAcNpcSpFuta_c *npc) {
    return FALSE;
}

// 0x90
BASE_PROFILE(A_NPC_SP_FUTA, dAcNpcSpFuta_c);

// .bss 0x8, built by __sinit.
struct param_s {
    param_s() : _0(0), mSleepMorph(90.0f) {}
    int _0;
    f32 mSleepMorph;
};
static param_s l_param;

static inline dMuseum_c &getMuseum() {
    return dSaveData_c::getTown()->mMuseum;
}

static inline int countUnidentifiedFossils() {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(isUnidentifiedFossil, NULL);
}
} // namespace dANpcSpFuta

// 0x23C
void dAcNpcSpFuta_c::onDemoCmd(int cmd, u8 member) {
    _1B70 = member;
    int permit = requestPermit(cmd, member, TRUE);
    switch (cmd) {
    case DEMO_CMD_START:
        if (permit == PERMIT_REMOTE_START || permit == PERMIT_LOCAL_START) {
            changeState(STATE_LOOK);
        }
        break;
    case DEMO_CMD_TALK:
        if (permit == PERMIT_REMOTE_TALK) {
            changeState(STATE_REMOTE_TALK);
        } else if (permit == PERMIT_LOCAL_TALK) {
            mTalkFuta.start(dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        }
        break;
    case DEMO_CMD_END:
        if (permit == PERMIT_END_SINGLE) {
            changeState(STATE_IDLE);
        } else if (permit == PERMIT_END_MULTI) {
            changeState(STATE_REMOTE);
        }
        break;
    case CMD_RELEASE_PERMIT:
        if (permit == PERMIT_RELEASED) {
            changeState(STATE_IDLE);
        }
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpFuta {
static const dAcNpcSpFuta_c::State_s l_states[] = {
    {&dAcNpcSpFuta_c::initIdle, &dAcNpcSpFuta_c::executeIdle},
    {&dAcNpcSpFuta_c::initLook, &dAcNpcSpFuta_c::executeLook},
    {&dAcNpcSpFuta_c::initTalk, &dAcNpcSpFuta_c::executeTalk},
    {&dAcNpcSpFuta_c::initRemote, &dAcNpcSpFuta_c::executeRemote},
    {&dAcNpcSpFuta_c::initRemoteTalk, &dAcNpcSpFuta_c::executeRemoteTalk},
};
} // namespace dANpcSpFuta

// 0x358
void dAcNpcSpFuta_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpFuta::l_states[state].mInit)();
}

// 0x390
int dAcNpcSpFuta_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mTalkFuta);
    return SUCCEEDED;
}

// 0x3DC
int dAcNpcSpFuta_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    mSleepTimer = 0;
    if (dScene::getPrevScene() == dScene::SCENE_FIELD) {
        dNpcInfo::futa_c::get()->setGreeted(FALSE);
    }
    if (isLocalOwner()) {
        mDaub.mAsleep = isSleepTime();
        changeState(STATE_IDLE);
        setDaubData(&mDaub, sizeof(mDaub));
    } else {
        changeState(STATE_REMOTE);
        getDaubData(&mDaub, sizeof(mDaub));
    }
    dSceneCheck::fn_80081858(dANpcSpFuta::entryDebugPage(this));
    return SUCCEEDED;
}

// 0x4A8
int dAcNpcSpFuta_c::doDelete() {
    if (!dAcNpcSp_c::doDelete()) {
        return NOT_READY;
    }
    dANpcSpFuta::entryDebugPage(this);
    dSceneCheck::fn_8008185C();
    return SUCCEEDED;
}

// 0x4F4
int dAcNpcSpFuta_c::vtB0() {
    if (isMultiPlay()) {
        getDaubData(&mDaub, sizeof(mDaub));
    }
    if (mState < STATE_COUNT) {
        (this->*dANpcSpFuta::l_states[mState].mExecute)();
    }
    if (!isInUse(NULL) && !mTalk.isBusy()) {
        if (mDaub.mAsleep) {
            mLookAt.setFront(PRIORITY_IDLE, l_lookPitchStep, l_lookYawStep);
        } else {
            mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep, cNpcLookRange);
        }
    }
    return 1;
}

// 0x5E0
void dAcNpcSpFuta_c::initIdle() {
    if (mDaub.mAsleep) {
        fallAsleep();
    } else {
        wakeUp();
    }
}

// 0x5F4
void dAcNpcSpFuta_c::wakeUp() {
    mDaub.mAsleep = 0;
    setDaubData(&mDaub, sizeof(mDaub));
    mAnmSet.setAnm0(HMN_ANM_NML_WAIT1);
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mStep = IDLE_STEP_AWAKE;
    mSleepTimer = SLEEP_WAIT_FRAMES;
}

// 0x664
void dAcNpcSpFuta_c::stepIdleAwake() {
    if (isSleepTime()) {
        if (mLookAt.searchPlayer(NET_MEMBER_NONE, this) != NET_MEMBER_NONE) {
            mSleepTimer = SLEEP_WAIT_FRAMES;
        } else if (mSleepTimer != 0) {
            mSleepTimer--;
        } else {
            fallAsleep();
        }
    } else {
        mSleepTimer = SLEEP_WAIT_FRAMES;
    }
}

// 0x6E8
void dAcNpcSpFuta_c::fallAsleep() {
    mDaub.mAsleep = 1;
    setDaubData(&mDaub, sizeof(mDaub));
    mAnmSet.setAnm0(HMN_ANM_OWL_SLP1);
    mAction.requestAnm(PRIORITY_IDLE, HMN_ANM_OWL_SLP1, 0.0f, dANpcSpFuta::l_param.mSleepMorph,
                       m3d::PLAYMODE_INHERIT, 0);
    mStep = IDLE_STEP_ASLEEP;
}

// 0x768
void dAcNpcSpFuta_c::stepIdleAsleep() {
    if (!isSleepTime()) {
        wakeUp();
    }
    mSleepEffect.create("afm_owl_sleep", &mNodeMtx12, 0);
}

namespace dANpcSpFuta {
static const dAcNpcSpFuta_c::Proc l_idleSteps[] = {
    &dAcNpcSpFuta_c::stepIdleAwake,
    &dAcNpcSpFuta_c::stepIdleAsleep,
};
} // namespace dANpcSpFuta

// 0x7BC
void dAcNpcSpFuta_c::executeIdle() {
    if (mStep < IDLE_STEP_DONE) {
        (this->*dANpcSpFuta::l_idleSteps[mStep])();
    }
}

// 0x7FC
void dAcNpcSpFuta_c::initLook() {
    mAnmSet.setAnm0(HMN_ANM_NML_WAIT1);
    if (mDaub.mAsleep) {
        initLookAsleep();
    } else {
        initLookTurn();
    }
}

// 0x850
void dAcNpcSpFuta_c::initLookAsleep() {
    mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_NML_WAIT1, 0.0f, cNpcMorphFrames, m3d::PLAYMODE_INHERIT, 0);
    mStep = LOOK_STEP_WAKE;
}

// 0x8A8
void dAcNpcSpFuta_c::stepLookWake() {
    if (mAction.mActionId == ACTION_ANM && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        mStep = LOOK_STEP_DONE;
    }
}

// 0x904
void dAcNpcSpFuta_c::initLookTurn() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
    mStep = LOOK_STEP_TURN;
}

// 0x968
void dAcNpcSpFuta_c::stepLookTurn() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        mStep++;
    }
}

namespace dANpcSpFuta {
static const dAcNpcSpFuta_c::Proc l_lookSteps[] = {
    &dAcNpcSpFuta_c::stepLookWake,
    &dAcNpcSpFuta_c::stepLookTurn,
};
} // namespace dANpcSpFuta

// 0x9C8
void dAcNpcSpFuta_c::executeLook() {
    if (mStep < LOOK_STEP_DONE) {
        (this->*dANpcSpFuta::l_lookSteps[mStep])();
    }
}

// 0xA08
void dAcNpcSpFuta_c::initTalk() {
    if (mDaub.mAsleep) {
        mTalk.requestTalk(true, FALSE);
    } else {
        mAng angle = getTalkAngle();
        mAng speed = getTalkTurnSpeed();
        mTalk.requestTurnTalk(angle, speed, FALSE);
    }
    mStep = TALK_STEP_TURN;
    mDaub.mAsleep = 0;
    setDaubData(&mDaub, sizeof(mDaub));
}

// 0xAB0
void dAcNpcSpFuta_c::stepTalkTurn() {
    if (!mTalk.isBusy()) {
        mAction.requestTurn(PRIORITY_DEMO, mAng(0), l_turnSpeed, 0, cNpcMorphFrames);
        mStep++;
    }
}

// 0xB20
void dAcNpcSpFuta_c::stepTalkTurnEnd() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        dDemo::finishTalk(this);
        mStep++;
    }
}

namespace dANpcSpFuta {
static const dAcNpcSpFuta_c::Proc l_talkSteps[] = {
    &dAcNpcSpFuta_c::stepTalkTurn,
    &dAcNpcSpFuta_c::stepTalkTurnEnd,
};
} // namespace dANpcSpFuta

// 0xB70
void dAcNpcSpFuta_c::executeTalk() {
    if (mStep < TALK_STEP_DONE) {
        (this->*dANpcSpFuta::l_talkSteps[mStep])();
    }
}

// 0xBB0
void dAcNpcSpFuta_c::initRemote() {
    if (mDaub.mAsleep) {
        initRemoteAsleep();
    } else {
        initRemoteAwake();
    }
}

// 0xBC4
void dAcNpcSpFuta_c::initRemoteAwake() {
    mAnmSet.setAnm0(HMN_ANM_NML_WAIT1);
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mStep = REMOTE_STEP_AWAKE;
}

// 0xC14
void dAcNpcSpFuta_c::stepRemoteAwake() {
    if (mDaub.mAsleep) {
        initRemoteAsleep();
    }
}

// 0xC28
void dAcNpcSpFuta_c::initRemoteAsleep() {
    mAnmSet.setAnm0(HMN_ANM_OWL_SLP1);
    mAction.requestAnm(PRIORITY_IDLE, HMN_ANM_OWL_SLP1, 0.0f, cNpcMorphFrames, m3d::PLAYMODE_INHERIT, 1);
    mStep = REMOTE_STEP_ASLEEP;
}

// 0xC8C
void dAcNpcSpFuta_c::stepRemoteAsleep() {
    if (!mDaub.mAsleep) {
        initRemoteAwake();
    }
    mSleepEffect.create("afm_owl_sleep", &mNodeMtx12, 0);
}

namespace dANpcSpFuta {
static const dAcNpcSpFuta_c::Proc l_remoteSteps[] = {
    &dAcNpcSpFuta_c::stepRemoteAwake,
    &dAcNpcSpFuta_c::stepRemoteAsleep,
};
} // namespace dANpcSpFuta

// 0xCDC
void dAcNpcSpFuta_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            mAnmSet.setAnm0(HMN_ANM_NML_WAIT1);
            fn_80013474();
            mTalkFuta.start(dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_IDLE);
        }
    }
    if (mStep < REMOTE_STEP_DONE) {
        (this->*dANpcSpFuta::l_remoteSteps[mStep])();
    }
}

// 0xDA4
void dAcNpcSpFuta_c::initRemoteTalk() {}

// 0xDA8
void dAcNpcSpFuta_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_IDLE);
    }
}

// 0xDFC
BOOL dAcNpcSpFuta_c::isSleepTime() {
    dTime_c now = *dTime_c::getCurrent();
    BOOL sleep = FALSE;
    if (now.hour >= SLEEP_START_HOUR && now.hour < SLEEP_END_HOUR) {
        sleep = TRUE;
    }
    return sleep;
}

// 0xE90
void dAcNpcSpFuta_c::turnToPlayer() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
}

// 0xEEC
dAcNpcSpFuta_c::talk_c::talk_c(dAcNpcSpFuta_c *npc) : mpOwner(npc) {
    init();
}

// 0xF58
dAcNpcSpFuta_c::talk_c::~talk_c() {}

// 0xFB0
void dAcNpcSpFuta_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
    mNextOnRequestEnd = NULL;
    mProc = NULL;
    mDonated = 0;
    mAppraise = 0;
    mLabel = LABEL_MAIN;
    mItem = dItem::ITEM_ID_NONE;
}

// 0x1060
void dAcNpcSpFuta_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    if (mpOwner->mDaub.mAsleep) {
        mMsgNo = MSG_ASLEEP;
        return;
    }
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
        if (homes->getHome(homes->findCurrentPlayer()) == NULL) {
            if (!dNpcInfo::futa_c::get()->isGreeted()) {
                mMsgNo = MSG_NEWCOMER;
            } else {
                mMsgNo = MSG_NEWCOMER_AGAIN;
            }
            dNpcInfo::futa_c::get()->setGreeted(TRUE);
            return;
        }
    }
    if (!dNpcInfo::futa_c::get()->isGreeted()) {
        dNpcInfo::futa_c::get()->setGreeted(TRUE);
        if (dANpcSpFuta::getMuseum().isComplete()) {
            mMsgNo = MSG_WELCOME_COMPLETE;
        } else {
            mMsgNo = MSG_WELCOME;
        }
    } else {
        mMsgNo = MSG_WELCOME_AGAIN;
    }
}

// 0x1180
void dAcNpcSpFuta_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_hu-ta";
    info->mCode = mMsgNo;
}

// 0x1198
void dAcNpcSpFuta_c::talk_c::onMessageStart(int arg) {
    switch (mLabel) {
    case LABEL_MAIN:
        switch (mMessageCode) {
        case MSG_ASSIST_AGAIN:
        case MSG_ASSIST_AGAIN_EXHIBITS:
        case MSG_DONATE_MORE:
            mItem = dItem::ITEM_ID_NONE;
            mAppraise = 0;
            break;
        case MSG_APPRAISED:
        case MSG_APPRAISED_2:
        case MSG_APPRAISED_3:
        case MSG_DONATED_BY_ME:
        case MSG_DONATED_BY_OTHER:
        case MSG_DONATED_BY_DELETED:
        case MSG_FOSSIL:
        case MSG_FISH:
        case MSG_INSECT:
        case MSG_PICTURE:
        case MSG_PICTURE_2:
        case MSG_PICTURE_3:
        case MSG_FAKE_PICTURE:
            setItemName(mItem, 0);
            break;
        }
        switch (mMessageCode) {
        case MSG_DONATED_BY_OTHER: {
            dPersonalID_c donor;
            donor.clear();
            dANpcSpFuta::getMuseum().getDonorID(&donor, mItem);
            setPersonalName(&donor, 1);
            break;
        }
        }
        break;
    case LABEL_FOSSIL: // empty cases needed to match
    case LABEL_INSECT:
    case LABEL_FISH:
        break;
    }
}

inline void dAcNpcSpFuta_c::talk_c::donateItem() {
    dANpcSpFuta::getMuseum().donate(mItem);
    dItem::Item none;
    dPlayerMgr::Mgr_c::getCurrentPlayer()->setPocket(&none, getMenuSelSlot(), FALSE);
    mDonated = 1;
}

namespace dANpcSpFuta {
// Arrays, not literals: the target keeps three unpooled copies of the main label here.
static char l_fishLabel[] = "npc_SP/NPC_hu-ta_fish";
static char l_insectLabel[] = "npc_SP/NPC_hu-ta_bug";
static char l_fossilLabel[] = "npc_SP/NPC_hu-ta_fossil";
static char l_fossilEndLabel[] = "npc_SP/NPC_hu-ta";
static char l_insectEndLabel[] = "npc_SP/NPC_hu-ta";
static char l_fishEndLabel[] = "npc_SP/NPC_hu-ta";
} // namespace dANpcSpFuta

// 0x1260
void dAcNpcSpFuta_c::talk_c::onMessageEnd(int kind) {
    switch (mLabel) {
    case LABEL_MAIN:
        switch (mMessageCode) {
        case MSG_ASLEEP:
            mOnMessageEnd = &talk_c::suspend;
            mOnRequestEnd = &talk_c::startTurn;
            if (dAcNpc_c::isPlayerFromThisTown()) {
                dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
                if (homes->getHome(homes->findCurrentPlayer()) == NULL) {
                    if (!dNpcInfo::futa_c::get()->isGreeted()) {
                        mpController->setNextMsgCode(MSG_NEWCOMER_WOKEN);
                    } else {
                        mpController->setNextMsgCode(MSG_NEWCOMER_WOKEN_AGAIN);
                    }
                    dNpcInfo::futa_c::get()->setGreeted(TRUE);
                    break;
                }
            }
            if (!dNpcInfo::futa_c::get()->isGreeted()) {
                dNpcInfo::futa_c::get()->setGreeted(TRUE);
                if (dANpcSpFuta::getMuseum().isComplete()) {
                    mpController->setNextMsgCode(MSG_WOKEN_COMPLETE);
                } else {
                    mpController->setNextMsgCode(MSG_WOKEN);
                }
            } else {
                mpController->setNextMsgCode(MSG_WOKEN_AGAIN);
            }
            break;
        case MSG_WOKEN_COMPLETE:
        case MSG_WOKEN:
        case MSG_WOKEN_AGAIN:
        case MSG_WELCOME_COMPLETE:
        case MSG_WELCOME:
        case MSG_WELCOME_AGAIN:
            if (dAcNpc_c::isPlayerFromThisTown()) {
                if (dANpcSpFuta::getMuseum().hasAnyDonation()) {
                    mpController->setNextMsgCode(MSG_ASSIST_EXHIBITS);
                } else {
                    mpController->setNextMsgCode(MSG_ASSIST);
                }
            }
            break;
        case MSG_WANDER:
            setProgressMsg();
            break;
        case MSG_COLLECTIONS:
            if (!dAcNpc_c::isMultiPlay()) {
                if (dANpcSpFuta::getMuseum().isComplete()) {
                    mpController->setNextMsgCode(MSG_GENEROSITY);
                } else {
                    mpController->setNextMsgCode(MSG_SOLE_STAFF);
                }
            }
            break;
        case MSG_EXHIBITS:
            mOnMessageEnd = &talk_c::openExhibits;
            mOnRequestEnd = &talk_c::endExhibits;
            break;
        case MSG_SHOW_FOSSIL:
            mOnMessageEnd = &talk_c::openFossilMenu;
            mOnRequestEnd = &talk_c::onFossilSelected;
            break;
        case MSG_APPRAISE: {
            mItem = dItem::Item(dItem::ITEM_IDX_AMBER, cM::rndInt(MUSEUM_FOSSIL_NUM), FALSE);
            int slot = getMenuSelSlot();
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setPocket(&mItem, slot, FALSE);
            break;
        }
        case MSG_APPRAISED:
        case MSG_APPRAISED_2:
        case MSG_APPRAISED_3:
        case MSG_FOSSIL:
            if (mAppraise) {
                if (dAcNpc_c::isMultiPlay() || dANpcSpFuta::getMuseum().isDonated(mItem)) {
                    mpController->setNextMsgCode(MSG_RETURN_FOSSIL);
                } else {
                    mpController->setNextMsgCode(MSG_ASK_FOSSIL);
                }
            } else if (dANpcSpFuta::getMuseum().isDonated(mItem)) {
                mpController->setNextMsgCode(MSG_HAVE_FOSSIL_ALREADY);
            } else {
                mpController->setNextMsgCode(MSG_TAKE_FOSSIL);
            }
            break;
        case MSG_RETURN_FOSSIL:
        case MSG_KEEP_FOSSIL:
            if (dANpcSpFuta::countUnidentifiedFossils() > 0) {
                mpController->setNextMsgCode(MSG_MORE_FOSSILS);
            } else {
                mpController->setNextMsgCode(MSG_ASSIST_NEXT);
            }
            mOnMessageEnd = &talk_c::giveBack;
            mOnRequestEnd = &talk_c::endGiveBack;
            break;
        case MSG_NO_MORE_FOSSILS:
            mpController->setNextMsgCode(MSG_ASSIST_NEXT);
            break;
        case MSG_ASK_DONATION:
            mOnMessageEnd = &talk_c::openDonationMenu;
            mOnRequestEnd = &talk_c::onDonationSelected;
            break;
        case MSG_DONATED_BY_ME:
        case MSG_DONATED_BY_OTHER:
        case MSG_DONATED_BY_DELETED:
            switch (mItem.getKind()) {
            case dItem::KIND_FOSSIL:
                mpController->setNextMsgCode(MSG_HAVE_FOSSIL);
                break;
            case dItem::KIND_FISH:
                mpController->setNextMsgCode(MSG_HAVE_FISH);
                break;
            case dItem::KIND_INSECT:
                mpController->setNextMsgCode(MSG_HAVE_INSECT);
                break;
            case dItem::KIND_PICTURE:
                mpController->setNextMsgCode(MSG_HAVE_PICTURE);
                break;
            }
            break;
        case MSG_HAVE_FOSSIL:
        case MSG_HAVE_FISH:
        case MSG_HAVE_INSECT:
        case MSG_HAVE_PICTURE:
        case MSG_HAVE_FOSSIL_ALREADY:
        case MSG_FORGERY_EXPOSED:
        case MSG_KNOWN_FORGERY:
            mpController->setNextMsgCode(MSG_DONATE_MORE);
            mOnMessageEnd = &talk_c::giveBack;
            mOnRequestEnd = &talk_c::endGiveBack;
            break;
        case MSG_NO_DONATION:
            if (mItem.isValid()) {
                mOnMessageEnd = &talk_c::giveBack;
                mOnRequestEnd = &talk_c::endGiveBack;
            }
            break;
        case MSG_FAKE_PICTURE: {
            mItem = dItem::Item(dItem::ITEM_IDX_FORGED_PAINTING_00, dItem::seeker_c::get()->findLike(mItem), FALSE);
            int slot = getMenuSelSlot();
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setPocket(&mItem, slot, FALSE);
            break;
        }
        case MSG_FOSSIL_PARTS:
            if (mAppraise) {
                if (dANpcSpFuta::countUnidentifiedFossils() > 0) {
                    mpController->setNextMsgCode(MSG_MORE_FOSSILS);
                } else {
                    mpController->setNextMsgCode(MSG_ASSIST_NEXT);
                }
            } else {
                mpController->setNextMsgCode(MSG_DONATE_MORE);
            }
            break;
        case MSG_FOSSIL_LECTURE_END:
            if (dANpcSpFuta::getMuseum().isFossilComplete()) {
                mpController->setNextMsgCode(MSG_FOSSILS_COMPLETE);
            } else if (mAppraise) {
                if (dANpcSpFuta::countUnidentifiedFossils() > 0) {
                    mpController->setNextMsgCode(MSG_MORE_FOSSILS);
                } else {
                    mpController->setNextMsgCode(MSG_ASSIST_NEXT);
                }
            } else {
                mpController->setNextMsgCode(MSG_DONATE_MORE);
            }
            break;
        case MSG_FISH_LECTURE_END:
            mOnMessageEnd = &talk_c::receive;
            mOnRequestEnd = &talk_c::endReceive;
            if (dANpcSpFuta::getMuseum().isFishComplete()) {
                mpController->setNextMsgCode(MSG_FISH_COMPLETE);
            } else {
                mpController->setNextMsgCode(MSG_DONATE_MORE);
            }
            break;
        case MSG_INSECT_LECTURE_END:
            mOnMessageEnd = &talk_c::receive;
            mOnRequestEnd = &talk_c::endReceive;
            if (dANpcSpFuta::getMuseum().isInsectComplete()) {
                mpController->setNextMsgCode(MSG_INSECTS_COMPLETE);
            } else {
                mpController->setNextMsgCode(MSG_DONATE_MORE);
            }
            break;
        case MSG_FISH: {
            donateItem();
            mpController->setNextMsgLabel(dANpcSpFuta::l_fishLabel);
            int idx = dANpcSpFuta::getMuseum().getIndex(mItem);
            mpController->setNextMsgCode(idx + 1); // entry 0 of the file is its name
            mLabel = LABEL_FISH;
            break;
        }
        case MSG_INSECT: {
            donateItem();
            mpController->setNextMsgLabel(dANpcSpFuta::l_insectLabel);
            int idx = dANpcSpFuta::getMuseum().getIndex(mItem);
            mpController->setNextMsgCode(idx + 1); // entry 0 of the file is its name
            mLabel = LABEL_INSECT;
            break;
        }
        case MSG_GIVE_FOSSIL:
        case MSG_TAKE_FOSSIL: {
            mOnMessageEnd = &talk_c::receive;
            mOnRequestEnd = &talk_c::endReceive;
            donateItem();
            if (dANpcSpFuta::getMuseum().isFossilSetComplete(mItem)) {
                dANpcSpFuta::getMuseum().getIndex(mItem); // result unused
                mpController->setNextMsgLabel(dANpcSpFuta::l_fossilLabel);
                // BITM::getFossil, written out: the inline loads mpController too early
                const dItem::BITM *bitm = dItem::getBITM(mItem.getId());
                int fossil = dItem::FOSSIL_NONE;
                u32 raw = bitm->m_fossil;
                if (raw < dItem::FOSSIL_COUNT) {
                    fossil = raw;
                }
                mpController->setNextMsgCode(fossil);
                mLabel = LABEL_FOSSIL;
            } else {
                mpController->setNextMsgCode(MSG_FOSSIL_PARTS);
            }
            break;
        }
        case MSG_PICTURE:
        case MSG_PICTURE_2:
        case MSG_PICTURE_3: {
            mOnMessageEnd = &talk_c::receive;
            mOnRequestEnd = &talk_c::endReceive;
            donateItem();
            mpController->setNextMsgCode(MSG_PICTURE_THANKS);
            break;
        }
        case MSG_PICTURE_THANKS:
            if (dANpcSpFuta::getMuseum().isPictureComplete()) {
                mpController->setNextMsgCode(MSG_PICTURES_COMPLETE);
            } else {
                mpController->setNextMsgCode(MSG_DONATE_MORE);
            }
            break;
        case MSG_FOSSILS_COMPLETE:
        case MSG_FISH_COMPLETE:
        case MSG_INSECTS_COMPLETE:
        case MSG_PICTURES_COMPLETE:
            if (dANpcSpFuta::getMuseum().isComplete()) {
                mpController->setNextMsgCode(MSG_ALL_COMPLETE);
            } else {
                mpController->setNextMsgCode(MSG_RELY_ON_YOU);
            }
            break;
        case MSG_ASSIST_NEXT:
            if (dANpcSpFuta::getMuseum().hasAnyDonation()) {
                mpController->setNextMsgCode(MSG_ASSIST_AGAIN_EXHIBITS);
            } else {
                mpController->setNextMsgCode(MSG_ASSIST_AGAIN);
            }
            break;
        }
        break;
    case LABEL_FOSSIL:
        mpController->setNextMsgLabel(dANpcSpFuta::l_fossilEndLabel);
        mpController->setNextMsgCode(MSG_FOSSIL_LECTURE_END);
        mLabel = LABEL_MAIN;
        break;
    case LABEL_INSECT:
        mpController->setNextMsgLabel(dANpcSpFuta::l_insectEndLabel);
        mpController->setNextMsgCode(MSG_INSECT_LECTURE_END);
        mLabel = LABEL_MAIN;
        break;
    case LABEL_FISH:
        mpController->setNextMsgLabel(dANpcSpFuta::l_fishEndLabel);
        mpController->setNextMsgCode(MSG_FISH_LECTURE_END);
        mLabel = LABEL_MAIN;
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// 0x1D94
void dAcNpcSpFuta_c::talk_c::onAnswer(int arg) {
    int answer = mpController->getAnswer();
    switch (mLabel) {
    case LABEL_MAIN:
        switch (mMessageCode) {
        case MSG_ASSIST:
        case MSG_ASSIST_AGAIN:
            switch (answer) {
            case ASSIST_DONATE:
                if (dANpcSpFuta::getMuseum().isComplete()) {
                    mpController->setNextMsgCode(MSG_MUSEUM_COMPLETE);
                } else if (dAcNpc_c::isMultiPlay()) {
                    mpController->setNextMsgCode(MSG_VISITORS);
                } else {
                    mpController->setNextMsgCode(MSG_ASK_DONATION);
                }
                break;
            case ASSIST_CHECK_FOSSIL:
                mAppraise = 1;
                if (dANpcSpFuta::countUnidentifiedFossils() > 0) {
                    mpController->setNextMsgCode(MSG_SHOW_FOSSIL);
                } else {
                    mpController->setNextMsgCode(MSG_NO_FOSSIL);
                }
                break;
            case ASSIST_WHERE:
                mpController->setNextMsgCode(MSG_WHERE);
                break;
            case ASSIST_NEVER_MIND:
                mpController->setNextMsgCode(MSG_WANDER);
                break;
            }
            break;
        case MSG_ASSIST_EXHIBITS:
        case MSG_ASSIST_AGAIN_EXHIBITS:
            switch (answer) {
            case ASSIST_EX_DONATE:
                if (dANpcSpFuta::getMuseum().isComplete()) {
                    mpController->setNextMsgCode(MSG_MUSEUM_COMPLETE);
                } else if (dAcNpc_c::isMultiPlay()) {
                    mpController->setNextMsgCode(MSG_VISITORS);
                } else {
                    mpController->setNextMsgCode(MSG_ASK_DONATION);
                }
                break;
            case ASSIST_EX_CHECK_FOSSIL:
                mAppraise = 1;
                if (dANpcSpFuta::countUnidentifiedFossils() > 0) {
                    mpController->setNextMsgCode(MSG_SHOW_FOSSIL);
                } else {
                    mpController->setNextMsgCode(MSG_NO_FOSSIL);
                }
                break;
            case ASSIST_EX_EXHIBITS:
                mpController->setNextMsgCode(MSG_EXHIBITS);
                break;
            case ASSIST_EX_WHERE:
                mpController->setNextMsgCode(MSG_WHERE);
                break;
            case ASSIST_EX_NEVER_MIND:
                mpController->setNextMsgCode(MSG_WANDER);
                break;
            }
            break;
        case MSG_SOLE_STAFF:
            switch (answer) {
            case ANSWER_YES:
                if (dANpcSpFuta::getMuseum().isComplete()) {
                    mpController->setNextMsgCode(MSG_MUSEUM_COMPLETE);
                } else if (dAcNpc_c::isMultiPlay()) {
                    mpController->setNextMsgCode(MSG_VISITORS);
                } else {
                    mpController->setNextMsgCode(MSG_ASK_DONATION);
                }
                break;
            case ANSWER_NO:
                mpController->setNextMsgCode(MSG_WANDER);
                break;
            }
            break;
        case MSG_DONATE_MORE:
            switch (answer) {
            case ANSWER_YES:
                if (dANpcSpFuta::getMuseum().isComplete()) {
                    mpController->setNextMsgCode(MSG_MUSEUM_COMPLETE);
                } else if (dAcNpc_c::isMultiPlay()) {
                    mpController->setNextMsgCode(MSG_VISITORS);
                } else {
                    mpController->setNextMsgCode(MSG_ASK_DONATION);
                }
                break;
            case ANSWER_NO:
                if (mDonated) {
                    mpController->setNextMsgCode(MSG_THANKS);
                } else {
                    mpController->setNextMsgCode(MSG_BYE);
                }
                break;
            }
            break;
        case MSG_MORE_FOSSILS:
            switch (answer) {
            case ANSWER_YES:
                mpController->setNextMsgCode(MSG_SHOW_FOSSIL);
                break;
            case ANSWER_NO:
                mpController->setNextMsgCode(MSG_NO_MORE_FOSSILS);
                break;
            }
            break;
        }
        break;
    case LABEL_FOSSIL: // empty cases needed to match
    case LABEL_INSECT:
    case LABEL_FISH:
        break;
    }
}

// 0x211C
void dAcNpcSpFuta_c::talk_c::preExecute() {
    if (mProc) {
        (this->*mProc)();
    }
}

// 0x2164
void dAcNpcSpFuta_c::talk_c::onTalkEnd() {}

// 0x2168
void dAcNpcSpFuta_c::talk_c::setProgressMsg() {
    int progress = dANpcSpFuta::getMuseum().getProgress();
    if (progress <= 0) {
        mpController->setNextMsgCode(MSG_PROGRESS_NONE);
    } else if (progress >= PROGRESS_FULL) {
        mpController->setNextMsgCode(MSG_PROGRESS_FULL);
    } else if (progress <= PROGRESS_LOW_MAX) {
        mpController->setNextMsgCode(MSG_PROGRESS_LOW);
    } else if (progress <= PROGRESS_MID_MAX) {
        mpController->setNextMsgCode(MSG_PROGRESS_MID);
    } else {
        mpController->setNextMsgCode(MSG_PROGRESS_HIGH);
    }
}

// 0x220C
void dAcNpcSpFuta_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
        if (mNextOnRequestEnd) {
            mOnRequestEnd = mNextOnRequestEnd;
        }
        mNextOnRequestEnd = NULL;
    }
}

namespace dANpcSpFuta {
// 0x22B4
static BOOL isNotDonatable(void *pocket, int flag) {
    if (flag) {
        return TRUE;
    }
    switch (static_cast<dItem::Item *>(pocket)->getKind()) {
    case dItem::KIND_INSECT:
    case dItem::KIND_FISH:
    case dItem::KIND_BEFORE_FOSSIL:
    case dItem::KIND_FOSSIL:
    case dItem::KIND_PICTURE:
    case dItem::KIND_FAKE_PICTURE_BEFORE:
    case dItem::KIND_FAKE_PICTURE_AFTER:
        return FALSE;
    default:
        return TRUE;
    }
}
} // namespace dANpcSpFuta

// 0x2314
void dAcNpcSpFuta_c::talk_c::openDonationMenu() {
    if (mpController->mNextMsgCode == 0) {
        mpController->setNextMsgCode(MSG_NONE);
    }
    reqSelectItem(dANpcSpFuta::isNotDonatable, ITEM_SEL_MSG_DONATE, MENU_DONATE, TRUE);
}

// 0x237C
void dAcNpcSpFuta_c::talk_c::suspend() {
    setRequest1(0);
}

namespace dANpcSpFuta {
// 0x2384
static BOOL isNotUnidentifiedFossil(void *pocket, int flag) {
    if (flag) {
        return TRUE;
    }
    switch (static_cast<dItem::Item *>(pocket)->getKind()) {
    case dItem::KIND_BEFORE_FOSSIL:
        return FALSE;
    default:
        return TRUE;
    }
}
} // namespace dANpcSpFuta

// 0x23CC
void dAcNpcSpFuta_c::talk_c::openFossilMenu() {
    if (mpController->mNextMsgCode == 0) {
        mpController->setNextMsgCode(MSG_NONE);
    }
    reqSelectItem(dANpcSpFuta::isNotUnidentifiedFossil, ITEM_SEL_MSG_GIVE, MENU_FOSSIL_CHECK, TRUE);
}

// 0x2434
void dAcNpcSpFuta_c::talk_c::openExhibits() {
    if (mpController->mNextMsgCode == 0) {
        mpController->setNextMsgCode(MSG_NONE);
    }
    reqMenu09(0);
}

// 0x248C
void dAcNpcSpFuta_c::talk_c::takeItem() {
    requestItemActEx(dHandItem_c::STATE_KEEP, &mItem, dHandItem_c::KIND_ITEM, dHandItem_c::VARIANT_HAND, 0,
                     HAND_EFFECT_NONE);
}

// 0x24A8
void dAcNpcSpFuta_c::talk_c::giveBack() {
    requestHandActE();
}

// 0x24AC
void dAcNpcSpFuta_c::talk_c::receive() {
    requestHandActD();
}

// 0x24B0
void dAcNpcSpFuta_c::talk_c::onDonationSelected() {
    if (isMenuInvalid()) {
        mItem = dItem::ITEM_ID_NONE;
        mpController->setNextMsgCode(MSG_NO_DONATION);
        reqMsgClose();
    } else {
        int slot = getMenuSelSlot();
        mItem = dPlayerMgr::Mgr_c::getCurrentPlayer()->mPockets[slot];
        dItem::infoBank_c::get()->getBITM(mItem); // result unused
        if (dANpcSpFuta::getMuseum().isDonated(mItem)) {
            switch (dANpcSpFuta::getMuseum().getDonorKind(mItem)) {
            case MUSEUM_DONATED_BY_ME:
                mpController->setNextMsgCode(MSG_DONATED_BY_ME);
                break;
            case MUSEUM_DONATED_BY_OTHER:
                mpController->setNextMsgCode(MSG_DONATED_BY_OTHER);
                break;
            case MUSEUM_DONATED_BY_DELETED:
                mpController->setNextMsgCode(MSG_DONATED_BY_DELETED);
                break;
            }
        } else {
            switch (mItem.getKind()) {
            case dItem::KIND_FOSSIL:
                mpController->setNextMsgCode(MSG_FOSSIL);
                break;
            case dItem::KIND_BEFORE_FOSSIL:
                mpController->setNextMsgCode(MSG_UNIDENTIFIED);
                break;
            case dItem::KIND_INSECT:
                mpController->setNextMsgCode(MSG_INSECT);
                break;
            case dItem::KIND_FISH:
                mpController->setNextMsgCode(MSG_FISH);
                break;
            case dItem::KIND_PICTURE:
                // getController(): the controller is loaded before the rndInt call
                getController()->setNextMsgCode(MSG_PICTURE + cM::rndInt(PICTURE_MSG_NUM));
                break;
            case dItem::KIND_FAKE_PICTURE_BEFORE:
                mpController->setNextMsgCode(MSG_FAKE_PICTURE);
                break;
            case dItem::KIND_FAKE_PICTURE_AFTER:
                mpController->setNextMsgCode(MSG_KNOWN_FORGERY);
                break;
            }
        }
        takeItem();
        mNextOnRequestEnd = &talk_c::closeMsg;
    }
}

// 0x268C
void dAcNpcSpFuta_c::talk_c::onFossilSelected() {
    if (isMenuInvalid()) {
        mpController->setNextMsgCode(MSG_NO_MORE_FOSSILS);
        reqMsgClose();
    } else {
        int slot = getMenuSelSlot();
        mItem = dPlayerMgr::Mgr_c::getCurrentPlayer()->mPockets[slot];
        dItem::infoBank_c::get()->getBITM(mItem); // result unused
        mpController->setNextMsgCode(MSG_APPRAISE);
        takeItem();
        mNextOnRequestEnd = &talk_c::closeMsg;
    }
}

// 0x273C
void dAcNpcSpFuta_c::talk_c::endExhibits() {
    setProgressMsg();
}

// 0x2740
void dAcNpcSpFuta_c::talk_c::closeMsg() {
    reqMsgClose();
}

// 0x2744
void dAcNpcSpFuta_c::talk_c::endGiveBack() {}

// 0x2748
void dAcNpcSpFuta_c::talk_c::endReceive() {}

// 0x274C
void dAcNpcSpFuta_c::talk_c::startTurn() {
    mpOwner->turnToPlayer();
    mProc = &talk_c::execTurn;
}

// 0x2798
void dAcNpcSpFuta_c::talk_c::execTurn() {
    dAcNpcSpFuta_c *owner = mpOwner;
    if (owner->mAction.mActionId == ACTION_TURN && owner->mAction.mCanChange) {
        mProc = NULL;
        reqMsgClose();
    }
}
