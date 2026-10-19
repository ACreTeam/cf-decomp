#include <game/game/d_a_npc_sp_totakeke.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_a_rmobj_mirrorball.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_environment.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_obj_staffroll.hpp>
#include <game/game/d_obj_telop.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_visitor_npc.hpp>
#include <game/game/d_scene_check.hpp>
#include <game/game/d_string.hpp>

// K.K. Slider at the Roost. Functions are in target .text order.

namespace dANpcSpTotakeke {
// .text 0x5C: the debug page hook (empty in this build).
static BOOL entryDebugPage(dAcNpcSpTotakeke_c *npc) {
    return FALSE;
}

// .text 0x64
BASE_PROFILE(A_NPC_SP_TOTAKEKE, dAcNpcSpTotakeke_c);

// The pitch limit of his look (.bss 0x8, built by __sinit).
static mAng l_lookPitchMax(0x6000);

// Tuning values (.bss 0x10, built by __sinit).
struct param_s {
    param_s() : _00(0), mSongDelay(50) {
        _1C = 0;
        mLookHeight = 64.0f;
        mLookPitchStep = l_lookPitchStep;
        mLookYawStep = l_lookYawStep;
        mMorph = 36.0f;
    }
    /* 0x00 */ int _00;
    /* 0x04 */ int mSongDelay;  // frames from the song's end to stopShow
    /* 0x08 */ f32 mLookHeight; // the height he looks at the requester
    /* 0x0C */ int mLookPitchStep;
    /* 0x10 */ int mLookYawStep;
    /* 0x14 */ f32 mMorph; // body animation blend frames
    /* 0x18 */ int _18;
    /* 0x1C */ u8 _1C;
}; // size 0x20
static param_s l_param;
} // namespace dANpcSpTotakeke

// .text 0x1E0
void dAcNpcSpTotakeke_c::onDemoCmd(int cmd, u8 member) {
    _1B70 = member;
    int owner = NET_MEMBER_MAX;
    // Matching workaround: volatile preserves the target's two permit-player
    // reloads. The original source qualifier is unknown.
    volatile int player = NET_MEMBER_MAX;
    if (!getPermitPlayers(&owner, const_cast<int *>(&player))) {
        player = NET_MEMBER_MAX;
    }
    BOOL permitted = FALSE;
    if (!isInUse(NULL) ||
        (dNet::getMyNo() == player && member == NET_MEMBER_MAX) ||
        player == member || (isLocalOwner() && member == NET_MEMBER_MAX)) {
        permitted = TRUE;
    }
    if (permitted) {
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
                mTotakekeTalk.start(dPlayerMgr::getNpcTargetActor(
                    dPlayerMgr::PLAYER_INDEX_CURRENT));
                changeState(STATE_TALK);
            }
            break;
        case DEMO_CMD_SPEAK:
            if (permit == PERMIT_REMOTE_TALK) {
                changeState(STATE_REMOTE_TALK);
            } else if (permit == PERMIT_LOCAL_TALK) {
                mTotakekeTalk.start(dPlayerMgr::getNpcTargetActor(
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
    } else if (member == NET_MEMBER_MAX) {
        switch (cmd) {
        case DEMO_CMD_START:
            changeState(STATE_LOOK);
            break;
        case DEMO_CMD_TALK:
            break;
        case DEMO_CMD_SPEAK:
            mTotakekeTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
            break;
        case DEMO_CMD_END:
            if (dNet::isSceneOwner()) {
                changeState(STATE_REMOTE_TALK);
            } else {
                changeState(STATE_REMOTE);
            }
            break;
        case CMD_RELEASE_PERMIT:
            break;
        }
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpTotakeke {
static const dAcNpcSpTotakeke_c::State_s l_states[] = {
    {&dAcNpcSpTotakeke_c::initIdle, &dAcNpcSpTotakeke_c::executeIdle},
    {&dAcNpcSpTotakeke_c::initLook, &dAcNpcSpTotakeke_c::executeLook},
    {&dAcNpcSpTotakeke_c::initTalk, &dAcNpcSpTotakeke_c::executeTalk},
    {&dAcNpcSpTotakeke_c::initRemote, &dAcNpcSpTotakeke_c::executeRemote},
    {&dAcNpcSpTotakeke_c::initRemoteTalk,
     &dAcNpcSpTotakeke_c::executeRemoteTalk}};
} // namespace dANpcSpTotakeke

// .text 0x460
void dAcNpcSpTotakeke_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpTotakeke::l_states[state].mInit)();
}

// .text 0x498
int dAcNpcSpTotakeke_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setInteractionRadius(80.0f);
    setRecept(&mTotakekeTalk);
    mAnmSet.setAnm0(HMN_ANM_END_WAIT1);
    mAnmSet.setAnm1(HMN_ANM_END_WAIT1);
    mAnmSet.setAnm2(HMN_ANM_END_WAIT1);
    return 1;
}

// .text 0x518
int dAcNpcSpTotakeke_c::create() {
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    mCheckGround = 0;
    mPos.y = 2.0f;
    mStartTimer = 0;
    mEndTimer = 0;
    mShowOn = 0;
    mSongOn = 0;
    mListen = 1;
    mStaffroll = 0;
    mEndingReq = 0;
    mTelopLocked = 0;
    mAcceptMoveReq = 0;
    dNpcInfo::totakeke_c::get()->mReceived = 0;
    dNpcInfo::totakeke_c::get()->mPlayReq = 0;
    if (isLocalOwner()) {
        changeState(STATE_IDLE);
        if (!dEvent::isVisitorHere(VISITOR_NPC_SATURDAY)) {
            mDaub.mActive = 0;
        } else {
            mDaub.mActive = 1;
        }
        mDaub.mPos = 0;
        mDaub.mPlaying = 0;
        mDaub.mSong = 0;
        setDaubData(&mDaub, sizeof(mDaub));
    } else {
        changeState(STATE_REMOTE);
        getDaubData(&mDaub, sizeof(mDaub));
        if (mDaub.mPlaying && mDaub.mSong) {
            startShow(0);
            dFixFarSwitchLight_c *light = dFixFarSwitchLight_c::sInstance;
            if (light) {
                light->setOn(0);
            }
            playSong(mDaub.mSong, mDaub.mPos);
        } else {
            resetShow();
        }
    }
    if (mDaub.mActive) {
        mIsActive = 1;
    } else {
        mIsActive = 0;
    }
    if (mIsActive) {
        dActor_c::construct(fProf::A_RMOBJ_MIRRORBALL, this, 0, NULL, NULL);
    }
    mLookAt.mPitch.mMax = dANpcSpTotakeke::l_lookPitchMax;
    dSceneCheck::fn_80081858(dANpcSpTotakeke::entryDebugPage(this));
    return 1;
}

// .text 0x710
int dAcNpcSpTotakeke_c::doDelete() {
    dANpcSpTotakeke::entryDebugPage(this);
    dSceneCheck::fn_8008185C();
    dBgm::l_mgr.mStgCafe.stopKK();
    resetShow();
    dBgm::l_mgr.mStgCafe.unmuteCafe();
    dObjTelop_c::unlock();
    if (dNpcInfo::totakeke_c::get()->mIsRequester) {
        dNpcInfo::totakeke_c::get()->stop();
    }
    if (!dAcNpcSp_c::doDelete()) {
        return 0;
    }
    return 1;
}

// .text 0x7A0
int dAcNpcSpTotakeke_c::vtB0() {
    if (isMultiPlay()) {
        getDaubData(&mDaub, sizeof(mDaub));
    }
    if (isOnStage()) {
        setDemoFlag8();
    } else {
        clearDemoFlag8();
    }
    if (mStartTimer > 0) {
        if (--mStartTimer == 0) {
            if (dNpcInfo::totakeke_c::get()->getSong().isValid()) {
                dItem::Item song = dNpcInfo::totakeke_c::get()->getSong();
                playSong(dItem::seeker_c::get()->findLike(song) + 1, 0);
            } else {
                mStartTimer = 1;
            }
        }
    } else if (dBgm::l_mgr.mStgCafe.isKKPlaying()) {
        if (dBgm::l_mgr.mStgCafe.isKKStopped()) {
            if (!mTelopLocked || dObjStaffroll::isEndingDone()) {
                if (mStaffroll) {
                    dObjStaffroll::stop();
                    mStaffroll = 0;
                }
                if (mEndTimer > 0) {
                    if (--mEndTimer == 0) {
                        stopShow();
                    }
                }
            }
        }
    } else if (!mSongOn && mShowOn) {
        resetShow();
        if (!dNpcInfo::totakeke_c::get()->mInAudience) {
            dBgm::l_mgr.mStgCafe.unmuteCafe();
        }
    }

    if (dBgm::l_mgr.mStgCafe.isKKPlaying()) {
        if (dBgm::l_mgr.mStgCafe.isKKStopped() ||
            dBgm::l_mgr.mStgCafe.getKKInfo3() >= 1) {
            if (mStaffroll) {
                dObjStaffroll::stop();
                mStaffroll = 0;
            }
            if (getModel()->mAnm.mAnmId == HMN_ANM_END_WAIT2) {
                mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_END_WAIT2, 0.0f,
                                   dANpcSpTotakeke::l_param.mMorph,
                                   m3d::PLAYMODE_INHERIT, 0);
            }
            mLookAt.setFront(PRIORITY_DEMO,
                             dANpcSpTotakeke::l_param.mLookPitchStep,
                             dANpcSpTotakeke::l_param.mLookYawStep);
        } else {
            if (dNpcInfo::totakeke_c::get()->mInAudience &&
                dBgm::l_mgr.mStgCafe.getKKInfo3() == 0 && !mStaffroll) {
                dObjStaffroll::start();
                mStaffroll = 1;
            }
            if (getModel()->mAnm.mAnmId == HMN_ANM_END_BEAT3 ||
                getModel()->mAnm.mAnmId == HMN_ANM_END_BEAT4) {
                f32 frame = dBgm::l_mgr.mStgCafe.getKKInfo8();
                getModel()->setFrame(frame, FALSE);
            } else {
                switch (dBgm::l_mgr.mStgCafe.getKKInfo0()) {
                case 3:
                    mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_END_BEAT3, 0.0f,
                                       dANpcSpTotakeke::l_param.mMorph,
                                       m3d::PLAYMODE_INHERIT, 0);
                    break;
                case 4:
                    mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_END_BEAT4, 0.0f,
                                       dANpcSpTotakeke::l_param.mMorph,
                                       m3d::PLAYMODE_INHERIT, 0);
                    break;
                default:
                    mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_END_WAIT1, 0.0f,
                                       dANpcSpTotakeke::l_param.mMorph,
                                       m3d::PLAYMODE_INHERIT, 0);
                    break;
                }
            }
            switch (dBgm::l_mgr.mStgCafe.getKKInfo2()) {
            case 0:
                mLookAt.setFront(PRIORITY_DEMO,
                                 dANpcSpTotakeke::l_param.mLookPitchStep,
                                 dANpcSpTotakeke::l_param.mLookYawStep);
                break;
            case 1:
                if (dPlayerMgr::getPlayerActorRaw(
                        dNpcInfo::totakeke_c::get()->mRequesterNo)) {
                    dAcPlayer_c *requester = dPlayerMgr::getPlayerActorRaw(
                        dNpcInfo::totakeke_c::get()->mRequesterNo);
                    mVec3_c pos(requester->mPos);
                    pos.y = dANpcSpTotakeke::l_param.mLookHeight;
                    mLookAt.setPos(PRIORITY_DEMO, pos, false,
                                   dANpcSpTotakeke::l_param.mLookPitchStep,
                                   dANpcSpTotakeke::l_param.mLookYawStep, 0.0f);
                }
                break;
            default:
                mLookAt.setFront(PRIORITY_DEMO,
                                 dANpcSpTotakeke::l_param.mLookPitchStep,
                                 dANpcSpTotakeke::l_param.mLookYawStep);
                break;
            }
        }
        switch (dBgm::l_mgr.mStgCafe.getKKInfo1()) {
        case 1:
            getModel()->setFaceTex(this, 1, "m.1");
            break;
        case 2:
            getModel()->setFaceTex(this, 1, "m.2");
            break;
        case 3:
            getModel()->setFaceTex(this, 1, "m.3");
            break;
        case 4:
            getModel()->setFaceTex(this, 1, "m.4");
            break;
        case 5:
            getModel()->setFaceTex(this, 1, "m.5");
            break;
        default:
            getModel()->setFaceTex(this, 1, "m.0");
            break;
        }
        daRmobjMirrorBall_c *ball = daRmobjMirrorBall_c::sInstance;
        if (ball) {
            if (dBgm::l_mgr.mStgCafe.getKKInfo5() == 1) {
                ball->mLight1On = 1;
            } else {
                ball->mLight1On = 0;
            }
        }
        if (mEndingReq == 1 && dBgm::l_mgr.mStgCafe.getKKInfo4() == 2) {
            dObjStaffroll::startEnding();
            mEndingReq = 0;
        }
    } else {
        int owner = NET_MEMBER_MAX;
        int player = NET_MEMBER_MAX;
        if (!getPermitPlayers(&owner, &player)) {
            player = NET_MEMBER_MAX;
        }
        if (mTalk.isBusy()) {
            mLookAt.setPlayer(PRIORITY_DEMO, false,
                              dANpcSpTotakeke::l_param.mLookPitchStep,
                              dANpcSpTotakeke::l_param.mLookYawStep, 128.0f);
        } else if (player < NET_MEMBER_MAX) {
            mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_END_WAIT1, 0.0f,
                               dANpcSpTotakeke::l_param.mMorph,
                               m3d::PLAYMODE_INHERIT, 0);
            mLookAt.setPlayerNo(PRIORITY_DEMO, player, false,
                                dANpcSpTotakeke::l_param.mLookPitchStep,
                                dANpcSpTotakeke::l_param.mLookYawStep, 128.0f);
        } else {
            mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_END_ENSOU1, 0.0f,
                               dANpcSpTotakeke::l_param.mMorph,
                               m3d::PLAYMODE_INHERIT, 0);
            mLookAt.setFront(PRIORITY_DEMO,
                             dANpcSpTotakeke::l_param.mLookPitchStep,
                             dANpcSpTotakeke::l_param.mLookYawStep);
        }
        daRmobjMirrorBall_c *ball = daRmobjMirrorBall_c::sInstance;
        if (ball) {
            ball->mLight1On = 0;
        }
    }

    if (dNpcInfo::totakeke_c::get()->mIsRequester && isLocalOwner()) {
        if (mDaub.mPlaying && dBgm::l_mgr.mStgCafe.isKKPlaying() &&
            dBgm::l_mgr.mStgCafe.fn_80079F30()) {
            mDaub.mPos = dBgm::l_mgr.mStgCafe.fn_80079F30();
        } else if (mDaub.mPos) {
            mDaub.mPos = 0;
            mDaub.mPlaying = 0;
            mDaub.mSong = 0;
        }
        setDaubData(&mDaub, sizeof(mDaub));
    }
    if (mState < STATE_COUNT) {
        (this->*dANpcSpTotakeke::l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x1024
bool dAcNpcSpTotakeke_c::canInteract(dDemoActor_c *actor) {
    if (dNpcInfo::totakeke_c::get()->isPlaying()) {
        return false;
    }
    if (dBgm::l_mgr.mStgCafe.isKKPlaying()) {
        return false;
    }
    return dAcNpcSp_c::canInteract(actor);
}

// .text 0x109C
int dAcNpcSpTotakeke_c::canSpeak(dDemoActor_c *actor) {
    return TRUE;
}

// .text 0x10A4
void dAcNpcSpTotakeke_c::startShow(u8 listen) {
    mListen = listen;
    dBgm::l_mgr.mState.mFlags |= dBgm::State_c::FLAG_KK_SHOW;
    dBgm::l_mgr.mStgCafe.muteCafe();
    dFixFarSwitchLight_c *light = dFixFarSwitchLight_c::sInstance;
    if (light) {
        light->mOn = 0;
    }
    mShowOn = 1;
    mStartTimer = 120;
    mEndTimer = 0;
}

// .text 0x111C
void dAcNpcSpTotakeke_c::playSong(u8 song, int pos) {
    if (mListen) {
        dPlayerMgr::listenToKK();
        mEndingReq = 1;
        mTelopLocked = 1;
        dObjTelop_c::lock();
    }
    dBgm::l_mgr.mStgCafe.playKK(song, pos, mListen);
    if (dNpcInfo::totakeke_c::get()->mIsRequester && isLocalOwner()) {
        mDaub.mPos = 0;
        mDaub.mPlaying = 1;
        mDaub.mSong = song;
        setDaubData(&mDaub, sizeof(mDaub));
        if (isMultiPlay()) {
            dNet::beginPacket();
            dNet::sendPacket(NET_PACKET_NPC_EVENT_PLAY, NET_MEMBER_MAX);
        }
    }
    mSongOn = 1;
    mStartTimer = 0;
    mEndTimer = dANpcSpTotakeke::l_param.mSongDelay;
}

// .text 0x121C
void dAcNpcSpTotakeke_c::stopShow() {
    dBgm::l_mgr.mStgCafe.stopKK();
    if (dNpcInfo::totakeke_c::get()->mIsRequester && isLocalOwner()) {
        mDaub.mPos = 0;
        mDaub.mPlaying = 0;
        mDaub.mSong = 0;
        setDaubData(&mDaub, sizeof(mDaub));
    }
    dFixFarSwitchLight_c *light = dFixFarSwitchLight_c::sInstance;
    if (light) {
        light->mOn = 1;
    }
    mSongOn = 0;
    mStartTimer = 0;
    mEndTimer = 0;
    dObjTelop_c::unlock();
    mTelopLocked = 0;
}

// .text 0x12D0
void dAcNpcSpTotakeke_c::resetShow() {
    mListen = 1;
    dBgm::l_mgr.mState.mFlags &= ~dBgm::State_c::FLAG_KK_SHOW;
    mShowOn = 0;
    mStartTimer = 0;
    mEndTimer = 0;
}

// .text 0x1300
void dAcNpcSpTotakeke_c::initIdle() {
}

// Speaks to the player who asked for the song once he is seated, starts the
// show another console asked for, and speaks to a player of another console
// who receives the song.
inline void dAcNpcSpTotakeke_c::checkSongRequest() {
    if (mIsActive && isOnStage() && !mTalk.isBusy() &&
        dNpcInfo::totakeke_c::get()->mIsRequester &&
        dNpcInfo::totakeke_c::get()->isPlaying() &&
        dNpcInfo::totakeke_c::get()->mRequestKind <
            dNpcInfo::totakeke_c::REQUEST_NONE) {
        dDemo::requestSpeak(this, NULL);
    } else if (dNpcInfo::totakeke_c::get()->mPlayReq) {
        dNpcInfo::totakeke_c::get()->clearPlayReq();
        if (!mShowOn) {
            startShow(0);
        }
    } else if (dNpcInfo::totakeke_c::get()->mReceived) {
        if (isOnStage() && !mTalk.isBusy() &&
            dNpcInfo::totakeke_c::get()->isPlaying() &&
            dNpcInfo::totakeke_c::get()->mRequestKind <
                dNpcInfo::totakeke_c::REQUEST_NONE) {
            dDemo::requestSpeak(this, NULL);
        } else {
            dNpcInfo::totakeke_c::get()->clearReceived();
        }
    }
}

// .text 0x1304
void dAcNpcSpTotakeke_c::executeIdle() {
    checkSongRequest();
}

// .text 0x1438
void dAcNpcSpTotakeke_c::initLook() {
}

// .text 0x143C
void dAcNpcSpTotakeke_c::executeLook() {
}

// .text 0x1440
void dAcNpcSpTotakeke_c::initTalk() {
    mTalk.requestTalk(false, FALSE);
    mStep = TALK_STEP_WAIT;
}

// .text 0x1480
void dAcNpcSpTotakeke_c::stepTalk() {
    if (!mTalk.isBusy()) {
        if (mTotakekeTalk.mKind == talk_c::KIND_SONG) {
            clearDemoFlag8();
            dCamera_c::sInstance->mRequestedMode =
                dCamera_c::sInstance->mDefaultMode;
        }
        dDemo::finishTalk(this);
        mStep++;
    }
}

namespace dANpcSpTotakeke {
static const dAcNpcSpTotakeke_c::Proc l_talkSteps[] = {
    &dAcNpcSpTotakeke_c::stepTalk};
} // namespace dANpcSpTotakeke

// .text 0x14F0
void dAcNpcSpTotakeke_c::executeTalk() {
    if (mStep < TALK_STEP_DONE) {
        (this->*dANpcSpTotakeke::l_talkSteps[mStep])();
    }
}

// .text 0x1530
void dAcNpcSpTotakeke_c::initRemote() {
}

// .text 0x1534
void dAcNpcSpTotakeke_c::executeRemote() {
    if (isNetSync() && isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mTotakekeTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_IDLE);
        }
    }
    checkSongRequest();
}

// .text 0x16E8
void dAcNpcSpTotakeke_c::initRemoteTalk() {
}

// .text 0x16EC
void dAcNpcSpTotakeke_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeState(STATE_IDLE);
    }
    checkSongRequest();
}

// .text 0x1850
BOOL dAcNpcSpTotakeke_c::isOnStage() {
    return dNpcInfo::totakeke_c::get()->isOnStage();
}

// .text 0x1874
dAcNpcSpTotakeke_c::talk_c::talk_c(dAcNpcSpTotakeke_c *npc) : mpOwner(npc) {
    init();
    mKind = KIND_TALK;
}

// .text 0x18DC
dAcNpcSpTotakeke_c::talk_c::~talk_c() {
}

// .text 0x1934
void dAcNpcSpTotakeke_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgCode = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
    mOnPreExecute = NULL;
    mNoTalked = 0;
}

namespace dANpcSpTotakeke {
// One of K.K.'s secret songs (only played on a request by title).
static inline bool isSecretSong(const dItem::Item &song) {
    return song.getFrom() == dItem::FROM_MUSIC_SECRET;
}
} // namespace dANpcSpTotakeke

// .text 0x19B8
void dAcNpcSpTotakeke_c::talk_c::start(dActor_c *target) {
    dNpcInfo::totakeke_c::get()->clearReceived();
    init();
    if (dNpcInfo::totakeke_c::get()->isPlaying()) {
        mKind = KIND_SONG;
        if (isMultiPlay() && !dNpcInfo::totakeke_c::get()->mIsRequester) {
            dNet::beginPacket();
            int requester = dNpcInfo::totakeke_c::get()->mRequesterNo;
            dNet::sendPacket(NET_PACKET_NPC_EVENT_JOIN, requester);
        }
    } else {
        mKind = KIND_TALK;
    }
    mpActActor = target;
    switch (mKind) {
    case KIND_TALK:
        if (!dEvent::isVisitorHere(VISITOR_NPC_SATURDAY)) {
            mMsgCode = MSG_DONE_TODAY;
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                       PRIVATE_FLAG0_PART_TIME_JOB)) {
            if (dNpcInfo::totakeke_c::get()->mWorkWarned) {
                mMsgCode = MSG_WORKING_AGAIN;
            } else {
                mMsgCode = MSG_WORKING;
                dNpcInfo::totakeke_c::get()->setWorkWarned();
            }
            mNoTalked = 1;
        } else if (dTime_c::getCurrent()->hour < 20) {
            mMsgCode = MSG_TOO_EARLY;
            mNoTalked = 1;
        } else if (!dNpcInfo::totakeke_c::get()->isMet()) {
            if (!dNpcInfo::totakeke_c::get()->isTalked()) {
                if (isPlayerFromThisTown()) {
                    mMsgCode = MSG_FIRST_MEETING;
                } else {
                    mMsgCode = MSG_VISITOR_FIRST_MEETING;
                }
            } else if (isPlayerFromThisTown()) {
                mMsgCode = MSG_FIRST_MEETING_AGAIN;
            } else if (dNpcInfo::totakeke_c::get()->isRequested()) {
                mMsgCode = MSG_FIRST_MEETING_REQUESTED;
            } else {
                mMsgCode = MSG_FIRST_MEETING_AGAIN;
            }
        } else if (!isPlayerFromThisTown()) {
            if (!dNpcInfo::totakeke_c::get()->isTalked()) {
                mMsgCode = MSG_VISITOR_MEET;
            } else if (!dNpcInfo::totakeke_c::get()->isRequested()) {
                mMsgCode = MSG_VISITOR_ASK;
            } else {
                mMsgCode = MSG_ENCORE;
            }
        } else {
            if (!dNpcInfo::totakeke_c::get()->isTalked()) {
                mMsgCode = MSG_ASK_SONG;
            } else if (!dNpcInfo::totakeke_c::get()->isRequested()) {
                mMsgCode = MSG_ASK_SONG_AGAIN;
            } else {
                mMsgCode = MSG_ENCORE;
            }
        }
        break;
    case KIND_SONG:
        dNpcInfo::totakeke_c::get()->sendSong();
        dCamera_c::sInstance->mRequestedMode = dCamera_c::MODE_CAFE;
        switch (dNpcInfo::totakeke_c::get()->mRequestKind) {
        case dNpcInfo::totakeke_c::REQUEST_GOOD:
            mMsgCode = MSG_MOOD_SONG;
            break;
        case dNpcInfo::totakeke_c::REQUEST_GRUMPY:
            mMsgCode = MSG_MOOD_SONG + 1;
            break;
        case dNpcInfo::totakeke_c::REQUEST_LAZY:
            mMsgCode = MSG_MOOD_SONG + 2;
            break;
        case dNpcInfo::totakeke_c::REQUEST_SAD:
            mMsgCode = MSG_MOOD_SONG + 3;
            break;
        case dNpcInfo::totakeke_c::REQUEST_HARD_TO_SAY:
            mMsgCode = MSG_MOOD_SONG + 4;
            break;
        case dNpcInfo::totakeke_c::REQUEST_PICK:
            mMsgCode = MSG_PICK_SONG;
            break;
        case dNpcInfo::totakeke_c::REQUEST_FIRST:
            mMsgCode = MSG_FIRST_SONG;
            break;
        case dNpcInfo::totakeke_c::REQUEST_TITLE:
            if (dANpcSpTotakeke::isSecretSong(
                    dNpcInfo::totakeke_c::get()->getSong())) {
                mMsgCode = MSG_SECRET_SONG;
            } else {
                mMsgCode = MSG_TITLE_SONG;
            }
            break;
        case dNpcInfo::totakeke_c::REQUEST_NONE:
            break;
        }
        break;
    }
}

// .text 0x1CF8
void dAcNpcSpTotakeke_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_totakeke";
    info->mCode = mMsgCode;
}

// .text 0x1D10
void dAcNpcSpTotakeke_c::talk_c::onMessageStart(int arg) {
    switch (mMessageCode) {
    case MSG_FIRST_SONG:
    case MSG_PICK_SONG:
    case MSG_TITLE_SONG:
    case MSG_MOOD_SONG:
    case MSG_MOOD_SONG + 1:
    case MSG_MOOD_SONG + 2:
    case MSG_MOOD_SONG + 3:
    case MSG_MOOD_SONG + 4:
    case MSG_SECRET_SONG:
        if (dNpcInfo::totakeke_c::get()->mUnknownTitle) {
            dString::Word_c title;
            title.set(dNpcInfo::totakeke_c::get()->mRequestTitle, 0);
            mpController->setWord(0, &title);
        } else {
            setItemName(dNpcInfo::totakeke_c::get()->getSong(), 0);
        }
        setPersonalName(&dPlayerMgr::Mgr_c::getNetPlayer(
                             dNpcInfo::totakeke_c::get()->mRequesterNo)
                             ->mPID,
                        1);
        break;
    case MSG_UNKNOWN_TITLE:
    case MSG_UNKNOWN_TITLE_AUDIENCE:
        if (dNpcInfo::totakeke_c::get()->mUnknownTitle) {
            dString::Word_c title;
            title.set(dNpcInfo::totakeke_c::get()->mRequestTitle, 0);
            mpController->setWord(0, &title);
        }
        break;
    case MSG_SECRET_HEARD:
        if (dNpcInfo::totakeke_c::get()->getSong().isValid()) {
            setItemName(dNpcInfo::totakeke_c::get()->getSong(), 0);
        }
        break;
    }
}

// .text 0x1EA8
void dAcNpcSpTotakeke_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_FIRST_PLAY:
        dNpcInfo::totakeke_c::get()->start(dNpcInfo::totakeke_c::REQUEST_FIRST,
                                           dItem::ITEM_ID_NONE, NULL);
        break;
    case MSG_NAME_SONG:
        mOnMessageEnd = &talk_c::openTitleInput;
        mOnRequestEnd = &talk_c::onTitleInput;
        break;
    case MSG_PICK:
        dNpcInfo::totakeke_c::get()->start(dNpcInfo::totakeke_c::REQUEST_PICK,
                                           dItem::ITEM_ID_NONE, NULL);
        break;
    case MSG_FIRST_SONG:
    case MSG_PICK_SONG:
    case MSG_TITLE_SONG:
    case MSG_MOOD_SONG:
    case MSG_MOOD_SONG + 1:
    case MSG_MOOD_SONG + 2:
    case MSG_MOOD_SONG + 3:
    case MSG_MOOD_SONG + 4:
    case MSG_SECRET_SONG:
        mOnMessageEnd = &talk_c::sitDown;
        mOnRequestEnd = &talk_c::startSong2;
        break;
    case MSG_FIRST_GIFT:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(
            PRIVATE_FLAG0_KK_FIRST_SONG);
        break;
    case MSG_COME_AGAIN:
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(PRIVATE_FLAG0_KK_MET);
    case MSG_GIFT:
    case MSG_GIFT_NO_ROOM:
    case MSG_UNKNOWN_TITLE:
    case MSG_UNKNOWN_TITLE_AUDIENCE:
    case MSG_VISITOR_THANKS:
    case MSG_VISITOR_NO_GIFT:
    case MSG_AUDIENCE_THANKS:
    case MSG_GIFT_GIVEN:
        if (isMultiPlay() && dNpcInfo::totakeke_c::get()->mIsRequester) {
            mpController->setNextMsgCode(dMsg::MSG_CODE_INVALID);
            dMsg::Comp_c *controller = mpController;
            controller->mLock = 1;
            controller->mWaitIconSe = 0;
            controller->mWaitIcon = 1;
        }
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0x2058
void dAcNpcSpTotakeke_c::talk_c::onAnswer(int arg) {
    u8 answer = mpController->mSelect.mCursor;
    switch (mMessageCode) {
    case MSG_ASK_MOOD:
        switch (answer) {
        case dNpcInfo::totakeke_c::REQUEST_GOOD:
            dNpcInfo::totakeke_c::get()->start(
                dNpcInfo::totakeke_c::REQUEST_GOOD, dItem::ITEM_ID_NONE, NULL);
            break;
        case dNpcInfo::totakeke_c::REQUEST_GRUMPY:
            dNpcInfo::totakeke_c::get()->start(
                dNpcInfo::totakeke_c::REQUEST_GRUMPY, dItem::ITEM_ID_NONE,
                NULL);
            break;
        case dNpcInfo::totakeke_c::REQUEST_LAZY:
            dNpcInfo::totakeke_c::get()->start(
                dNpcInfo::totakeke_c::REQUEST_LAZY, dItem::ITEM_ID_NONE, NULL);
            break;
        case dNpcInfo::totakeke_c::REQUEST_SAD:
            dNpcInfo::totakeke_c::get()->start(
                dNpcInfo::totakeke_c::REQUEST_SAD, dItem::ITEM_ID_NONE, NULL);
            break;
        case dNpcInfo::totakeke_c::REQUEST_HARD_TO_SAY:
            dNpcInfo::totakeke_c::get()->start(
                dNpcInfo::totakeke_c::REQUEST_HARD_TO_SAY, dItem::ITEM_ID_NONE,
                NULL);
            break;
        }
        break;
    }
}

// .text 0x2154
void dAcNpcSpTotakeke_c::talk_c::preExecute() {
    if (mpController->mLock && dNpcInfo::totakeke_c::get()->mAudienceDoneNum >=
                                   dNpcInfo::totakeke_c::get()->mAudienceNum) {
        mpController->setNextMsgCode(dMsg::MSG_CODE_NONE);
        dMsg::Comp_c *controller = mpController;
        controller->mLock = 0;
        controller->mWaitIconSe = 0;
        controller->mWaitIcon = 0;
    }
    if (mOnPreExecute) {
        (this->*mOnPreExecute)();
    }
}

// .text 0x21EC
void dAcNpcSpTotakeke_c::talk_c::onTalkEnd() {
    if (!mNoTalked) {
        if (mKind == KIND_TALK) {
            dNpcInfo::totakeke_c::get()->setTalked();
        } else if (mKind == KIND_SONG) {
            dNpcInfo::totakeke_c::get()->stop();
            mpOwner->resetShow();
            dBgm::l_mgr.mStgCafe.unmuteCafe();
        }
    }
}

// .text 0x2260
void dAcNpcSpTotakeke_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x22C4
void dAcNpcSpTotakeke_c::talk_c::openTitleInput() {
    if (mpController->mNextMsgCode == dMsg::MSG_CODE_NONE) {
        mpController->setNextMsgCode(dMsg::MSG_CODE_INVALID);
    }
    reqMenu2D(0);
}

// .text 0x231C
void dAcNpcSpTotakeke_c::talk_c::onTitleInput() {
    if (isMenuInvalid()) {
        dNpcInfo::totakeke_c::get()->start(dNpcInfo::totakeke_c::REQUEST_PICK,
                                           dItem::ITEM_ID_NONE, NULL);
        mpController->setNextMsgCode(MSG_TITLE_CANCEL);
    } else {
        void *title = getMenuWork();
        u16 found = fn_80029CE0();
        dNpcInfo::totakeke_c::get()->start(dNpcInfo::totakeke_c::REQUEST_TITLE,
                                           found, title);
        dItem::Item song(fn_80029CE0());
        if (song.isValid()) {
            int idx = dItem::seeker_c::get()->findLike(song);
            if (song.getFrom() == dItem::FROM_MUSIC_SECRET) {
                if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag3(idx)) {
                    dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag3(idx);
                    mpController->setNextMsgCode(MSG_SECRET_HEARD);
                } else {
                    mpController->setNextMsgCode(MSG_TITLE_OK);
                }
            } else {
                mpController->setNextMsgCode(MSG_TITLE_OK);
            }
        } else {
            mpController->setNextMsgCode(MSG_TITLE_OK);
        }
    }
}

// .text 0x2444
void dAcNpcSpTotakeke_c::talk_c::startSong() {
    mOnPreExecute = &talk_c::waitSong;
    mpOwner->startShow(1);
}

// .text 0x246C
void dAcNpcSpTotakeke_c::talk_c::waitSong() {
    if (!mpOwner->mShowOn) {
        reqMsgClose();
        mOnRequestEnd = &talk_c::afterSong;
        mOnPreExecute = NULL;
    }
}

// .text 0x24E0
void dAcNpcSpTotakeke_c::talk_c::sitDown() {
    if (mpController->mNextMsgCode == dMsg::MSG_CODE_NONE) {
        mpController->setNextMsgCode(dMsg::MSG_CODE_INVALID);
    }
    setRequest1(0);
}

// .text 0x2538
void dAcNpcSpTotakeke_c::talk_c::startSong2() {
    startSong();
}

// .text 0x253C
void dAcNpcSpTotakeke_c::talk_c::afterSong() {
    switch (dNpcInfo::totakeke_c::get()->mRequestKind) {
    case dNpcInfo::totakeke_c::REQUEST_TITLE:
        if (dNpcInfo::totakeke_c::get()->mUnknownTitle) {
            if (dNpcInfo::totakeke_c::get()->mHadAudience) {
                mpController->setNextMsgCode(MSG_UNKNOWN_TITLE_AUDIENCE);
            } else {
                mpController->setNextMsgCode(MSG_UNKNOWN_TITLE);
            }
            break;
        }
    case dNpcInfo::totakeke_c::REQUEST_GOOD:
    case dNpcInfo::totakeke_c::REQUEST_GRUMPY:
    case dNpcInfo::totakeke_c::REQUEST_LAZY:
    case dNpcInfo::totakeke_c::REQUEST_SAD:
    case dNpcInfo::totakeke_c::REQUEST_HARD_TO_SAY:
    case dNpcInfo::totakeke_c::REQUEST_PICK:
        if (dNpcInfo::totakeke_c::get()->mHadAudience) {
            mpController->setNextMsgCode(MSG_AUDIENCE_THANKS);
        } else if (!isPlayerFromThisTown()) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                    PRIVATE_FLAG1_KK_SONG_GIVEN)) {
                mpController->setNextMsgCode(MSG_VISITOR_THANKS);
            } else {
                mpController->setNextMsgCode(MSG_VISITOR_NO_GIFT);
            }
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                       PRIVATE_FLAG1_KK_SONG_GIVEN)) {
            mpController->setNextMsgCode(MSG_GIFT_GIVEN);
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                       NULL, NULL) > 0) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
                PRIVATE_FLAG1_KK_SONG_GIVEN);
            mpController->setNextMsgCode(MSG_GIFT);
            dItem::Item song = dNpcInfo::totakeke_c::get()->getSong();
            dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(&song, FALSE);
            int idx = dItem::seeker_c::get()->findLike(song);
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag2(idx);
        } else {
            mpController->setNextMsgCode(MSG_GIFT_NO_ROOM);
        }
        break;
    case dNpcInfo::totakeke_c::REQUEST_FIRST:
        if (dNpcInfo::totakeke_c::get()->mHadAudience) {
            mpController->setNextMsgCode(MSG_AUDIENCE_THANKS);
        } else if (!isPlayerFromThisTown()) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
                    PRIVATE_FLAG0_KK_FIRST_SONG)) {
                mpController->setNextMsgCode(MSG_VISITOR_THANKS);
            } else {
                mpController->setNextMsgCode(MSG_VISITOR_NO_GIFT);
            }
        } else if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                       NULL, NULL) > 0) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
                PRIVATE_FLAG1_KK_SONG_GIVEN);
            mpController->setNextMsgCode(MSG_FIRST_GIFT);
            dItem::Item song = dNpcInfo::totakeke_c::get()->getSong();
            dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(&song, FALSE);
            int idx = dItem::seeker_c::get()->findLike(song);
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag2(idx);
        } else {
            mpController->setNextMsgCode(MSG_FIRST_NO_ROOM);
        }
        break;
    case dNpcInfo::totakeke_c::REQUEST_NONE:
        break;
    }
}

// .text 0x27C0
int dAcNpcSpTotakeke_c::getEarType() {
    return NPC_EAR_TYPE_KK_SLIDER;
}
