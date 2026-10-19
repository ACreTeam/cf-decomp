#include <game/game/d_a_npc_sp_asami.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_a_rmobj_sewing_machine.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_list.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/cLib/c_math.hpp>

// Mabel at the sewing machine. Functions are in target .text order.

namespace dANpcSpAsami {
// .text 0x5C
BASE_PROFILE(A_NPC_SP_ASAMI, dAcNpcSpAsami_c);

// Tuning (.bss 0x8, built by __sinit).
struct param_s {
    param_s() : _0(0), _4(-1), mTurnFrame(0x2FF) {
    }
    u32 _0;
    int _4;
    int mTurnFrame; // talk_c::getTurnFrame
};
static param_s l_param;
} // namespace dANpcSpAsami

// .text 0x1CC
void dAcNpcSpAsami_c::onDemoCmd(int cmd, u8 member) {
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
            mAsamiTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
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

namespace dANpcSpAsami {
static const dAcNpcSpAsami_c::State_s l_states[] = {
    {&dAcNpcSpAsami_c::initIdle, &dAcNpcSpAsami_c::executeIdle},
    {&dAcNpcSpAsami_c::initLook, &dAcNpcSpAsami_c::executeLook},
    {&dAcNpcSpAsami_c::initTalk, &dAcNpcSpAsami_c::executeTalk},
    {&dAcNpcSpAsami_c::initRemote, &dAcNpcSpAsami_c::executeRemote},
    {&dAcNpcSpAsami_c::initRemoteTalk, &dAcNpcSpAsami_c::executeRemoteTalk},
};
} // namespace dANpcSpAsami

// .text 0x2E8
void dAcNpcSpAsami_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpAsami::l_states[state].mInit)();
}

// .text 0x320
int dAcNpcSpAsami_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mAsamiTalk);
    if (!isPlayerFromThisTown() || isMultiPlay()) {
        mAnmSet.setAnm0(HMN_ANM_HGS_SAW1);
        mAnmSet.setAnm1(HMN_ANM_HGS_SAW1);
        mAnmSet.setAnm2(HMN_ANM_HGS_SAW1);
    }
    return 1;
}

// .text 0x3A8
int dAcNpcSpAsami_c::create() {
    if (daRmobjSewingMachine_c::s_instance == NULL) {
        return 0;
    }
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    if (isPlayerFromThisTown() && !isMultiPlay() &&
        !dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
            PRIVATE_FLAG0_DRAMA_SABLE_HEARD) &&
        (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getGracieStep() == 1 ||
         dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getGracieStep() == 8)) {
        mDrama = 1;
    } else {
        mDrama = 0;
    }
    mCheckGround = 0;
    f32 frame = daRmobjSewingMachine_c::s_instance->mFrame;
    while (frame >= 168.0f) {
        frame -= 168.0f;
    }
    mAction.requestAnm(PRIORITY_IDLE, HMN_ANM_HGS_SAW1, frame, cNpcMorphFrames,
                       m3d::PLAYMODE_INHERIT, 0);
    daRmobjSewingMachine_c::s_instance->changeState(
        daRmobjSewingMachine_c::STATE_START);
    mLookAt.setFront(PRIORITY_IDLE, l_lookPitchStep, l_lookYawStep);
    if (isLocalOwner()) {
        changeState(STATE_IDLE);
        mDaub.mTiredFace = 0;
        setDaubData(&mDaub, sizeof(mDaub));
    } else {
        changeState(STATE_REMOTE);
        getDaubData(&mDaub, sizeof(mDaub));
    }
    return 1;
}

// .text 0x534
int dAcNpcSpAsami_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x560
int dAcNpcSpAsami_c::vtB0() {
    if (isMultiPlay()) {
        BOOL surprised = mDaub.mTiredFace;
        getDaubData(&mDaub, sizeof(mDaub));
        if (!isLocalOwner()) {
            if (mDaub.mTiredFace) {
                if (!surprised) {
                    mFace.setTex(&getModel()->mMdl, getTexResFile(), 0, 0x11E,
                                 m3d::PLAYMODE_INHERIT);
                    mFace.setTex(&getModel()->mMdl, getTexResFile(), 1, 0x11F,
                                 m3d::PLAYMODE_INHERIT);
                }
            } else if (surprised) {
                mFace.setTex(&getModel()->mMdl, getTexResFile(), 0, 0,
                             m3d::PLAYMODE_INHERIT);
                mFace.setTex(&getModel()->mMdl, getTexResFile(), 1, 1,
                             m3d::PLAYMODE_INHERIT);
            }
        }
    }
    if (mState < STATE_COUNT) {
        (this->*dANpcSpAsami::l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x734
void dAcNpcSpAsami_c::setTiredFace() {
    if (isLocalOwner()) {
        mDaub.mTiredFace = 1;
        setDaubData(&mDaub, sizeof(mDaub));
        getModel()->setFaceTex(this, 0, "e.4");
        getModel()->setFaceTex(this, 1, "m.3");
    }
}

// .text 0x7D4
void dAcNpcSpAsami_c::resetFace() {
    if (isLocalOwner()) {
        mDaub.mTiredFace = 0;
        setDaubData(&mDaub, sizeof(mDaub));
        getModel()->setFaceTex(this, 0, "e.0");
        getModel()->setFaceTex(this, 1, "m.0");
    }
}

// .text 0x874
void dAcNpcSpAsami_c::initIdle() {
}

// .text 0x878
void dAcNpcSpAsami_c::executeIdle() {
}

// .text 0x87C
void dAcNpcSpAsami_c::initLook() {
    if (!mDrama && isPlayerFromThisTown() && !isMultiPlay()) {
        mAction.requestWait(PRIORITY_DEMO);
    }
    mLookAt.setFront(PRIORITY_IDLE, l_lookPitchStep, l_lookYawStep);
}

// .text 0x8F8
void dAcNpcSpAsami_c::executeLook() {
}

// .text 0x8FC
void dAcNpcSpAsami_c::initTalk() {
    if (mDrama) {
        mTalk.requestTalk(true, FALSE);
    } else if (!isPlayerFromThisTown() || isMultiPlay()) {
        mTalk.requestTalk(true, FALSE);
    } else {
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->getSableTalkDays() < 5) {
            mTalk.requestTalk(true, FALSE);
        } else {
            mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep,
                              l_lookYawStep, cNpcLookRange);
            mAng angle = getTalkAngle();
            mAng speed = getTalkTurnSpeed();
            mTalk.requestTurnTalk(angle, speed, FALSE);
        }
        daRmobjSewingMachine_c::s_instance->changeState(
            daRmobjSewingMachine_c::STATE_STOP);
    }
    mStep = TALK_STEP_TALK;
}

// .text 0xA18
void dAcNpcSpAsami_c::stepTalk() {
    if (!mTalk.isBusy()) {
        if (!isPlayerFromThisTown() || isMultiPlay()) {
            dDemo::finishTalk(this);
            mStep = TALK_STEP_DONE;
        } else {
            mLookAt.setFront(PRIORITY_IDLE, l_lookPitchStep, l_lookYawStep);
            mAction.requestTurn(PRIORITY_DEMO, mAng(0), l_turnSpeed, 0);
            mStep++;
        }
    }
}

// .text 0xAD0
void dAcNpcSpAsami_c::stepTalkTurnEnd() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        if (isPlayerFromThisTown() && !isMultiPlay()) {
            f32 frame = daRmobjSewingMachine_c::s_instance->mFrame;
            while (frame >= 168.0f) {
                frame -= 168.0f;
            }
            mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_HGS_SAW1, frame,
                               cNpcMorphFrames, m3d::PLAYMODE_INHERIT, 0);
            daRmobjSewingMachine_c::s_instance->changeState(
                daRmobjSewingMachine_c::STATE_START);
        }
        dDemo::finishTalk(this);
        mStep++;
    }
}

namespace dANpcSpAsami {
static const dAcNpcSpAsami_c::Proc l_talkSteps[] = {
    &dAcNpcSpAsami_c::stepTalk, &dAcNpcSpAsami_c::stepTalkTurnEnd};
} // namespace dANpcSpAsami

// .text 0xB94
void dAcNpcSpAsami_c::executeTalk() {
    if (mStep < ARRAY_SIZE(dANpcSpAsami::l_talkSteps)) {
        (this->*dANpcSpAsami::l_talkSteps[mStep])();
    }
}

// .text 0xBD4
void dAcNpcSpAsami_c::initRemote() {
}

// .text 0xBD8
void dAcNpcSpAsami_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mAsamiTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_IDLE);
        }
    }
}

// .text 0xC6C
void dAcNpcSpAsami_c::initRemoteTalk() {
}

// .text 0xC70
void dAcNpcSpAsami_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_IDLE);
    }
}

// .text 0xCC4
void dAcNpcSpAsami_c::turnToPlayer() {
    mAction.requestTurn(PRIORITY_DEMO, getAngleYToPlayer(_1B70), l_turnSpeed,
                        0);
    daRmobjSewingMachine_c::s_instance->changeState(
        daRmobjSewingMachine_c::STATE_STOP);
}

// .text 0xD30
dAcNpcSpAsami_c::talk_c::talk_c(dAcNpcSpAsami_c *npc)
    : mpOwner(npc), mDramaHeard(0) {
    init();
}

// .text 0xD98
dAcNpcSpAsami_c::talk_c::~talk_c() {
}

// .text 0xDF0
void dAcNpcSpAsami_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
    mOnPreExecute = NULL;
}

// .text 0xE70
void dAcNpcSpAsami_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    const int & days = dPlayerMgr::Mgr_c::getCurrentPlayer()->getSableTalkDays();
    if (mpOwner->mDrama) {
        if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getGracieStep() == 1) {
            if (mDramaHeard) {
                mMsgNo = MSG_DRAMA_1_AGAIN;
            } else {
                mMsgNo = MSG_DRAMA_1;
            }
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->getGracieStep() ==
                   8) {
            if (mDramaHeard) {
                mMsgNo = MSG_DRAMA_2_AGAIN;
            } else {
                mMsgNo = MSG_DRAMA_2;
            }
        }
    } else if (isPlayerFromThisTown() &&
               dSaveData_c::getTown()->mHomes.getCurrentPlayerHome() == NULL) {
        mMsgNo = MSG_NO_HOUSE;
    } else if (!isPlayerFromThisTown() || isMultiPlay()) {
        mMsgNo = MSG_VISITOR;
    } else if (!dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                   PRIVATE_FLAG0_SABLE_TALKED)) {
        if (days <= 3) {
            mMsgNo = MSG_BUSY;
        } else if (days <= 9) {
            mMsgNo = (days - 4) * 3 + MSG_DAY_4;
        } else if (days == 10) {
            mMsgNo = MSG_PARENTS;
        } else {
            mMsgNo = MSG_FRIEND;
        }
    } else if (days <= 3) {
        mMsgNo = cM::rndInt(5) + MSG_BUSY_AGAIN;
    } else if (days <= 9) {
        mMsgNo = (days - 4) * 3 + cM::rndInt(2) + MSG_DAY_4_AGAIN;
    } else if (days == 10) {
        mMsgNo = cM::rndInt(2) + MSG_PARENTS_AGAIN;
    } else {
        mMsgNo = cM::rndInt(5) + MSG_FRIEND_AGAIN;
    }
    if (!mpOwner->mDrama && isPlayerFromThisTown() && !isMultiPlay()) {
        dDemoActor_c *partner = dNpcList::getSpByKey(dItem::Item((u16)SP_NPC_MABEL));
        if (partner != NULL) {
            setPartner(static_cast<dAcNpc_c *>(partner));
        }
    }
}

// .text 0x10D4
void dAcNpcSpAsami_c::talk_c::getMsgInfo(msgInfo_s *info) {
    if (mpOwner->mDrama) {
        info->mLabel = "npc_SP/NPC_drama2";
        info->mCode = mMsgNo;
    } else {
        info->mLabel = "npc_SP/NPC_asami";
        info->mCode = mMsgNo;
    }
}

// .text 0x1114
void dAcNpcSpAsami_c::talk_c::onMessageStart(int arg) {
}

// .text 0x1118
void dAcNpcSpAsami_c::talk_c::onMessageEnd(int kind) {
    if (mpOwner->mDrama) {
        switch (mMessageCode) {
        case MSG_DRAMA_1:
        case MSG_DRAMA_2:
            mOnMessageEnd = &talk_c::closeMessage;
            mOnRequestEnd = &talk_c::turnToPlayer;
            break;
        case MSG_DRAMA_1 + 1:
        case MSG_DRAMA_2 + 1:
            mDramaHeard = 1;
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                PRIVATE_FLAG0_DRAMA_SABLE_HEARD);
            break;
        }
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0x1210
void dAcNpcSpAsami_c::talk_c::onAnswer(int arg) {
}

// .text 0x1214
void dAcNpcSpAsami_c::talk_c::preExecute() {
    if (mOnPreExecute) {
        (this->*mOnPreExecute)();
    }
}

// .text 0x125C
void dAcNpcSpAsami_c::talk_c::onTalkEnd() {
    if (!mpOwner->mDrama) {
        if (!isPlayerFromThisTown() ||
            dSaveData_c::getTown()->mHomes.getCurrentPlayerHome() != NULL) {
            if (!isPlayerFromThisTown() || isMultiPlay()) {
                mpOwner->resetFace();
            } else {
                dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
                    PRIVATE_FLAG0_SABLE_TALKED);
            }
        }
    }
}

// .text 0x1300
void dAcNpcSpAsami_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x1364
void dAcNpcSpAsami_c::talk_c::setSpeakerFeel(u32 feel) {
    if (!isPlayerFromThisTown() || isMultiPlay()) {
        switch (feel) {
        case FEEL_TIRED:
            mpOwner->setTiredFace();
            break;
        default:
            mpOwner->resetFace();
            break;
        }
    } else {
        dAcNpc_c::recept_c::setSpeakerFeel(feel);
    }
}

// .text 0x13E0
void dAcNpcSpAsami_c::talk_c::closeMessage() {
    setRequest1(0);
}

// .text 0x13E8
void dAcNpcSpAsami_c::talk_c::turnToPlayer() {
    mpOwner->turnToPlayer();
    mOnPreExecute = &talk_c::waitTurn;
}

// .text 0x1434
void dAcNpcSpAsami_c::talk_c::waitTurn() {
    if (mpOwner->mAction.mActionId == ACTION_TURN &&
        mpOwner->mAction.mCanChange) {
        mOnPreExecute = NULL;
        reqMsgClose();
    }
}

// .text 0x1474
int dAcNpcSpAsami_c::talk_c::getTurnFrame() {
    return (s16)dANpcSpAsami::l_param.mTurnFrame;
}

// .text 0x1488
int dAcNpcSpAsami_c::getEarType() {
    return NPC_EAR_TYPE_SABLE;
}
