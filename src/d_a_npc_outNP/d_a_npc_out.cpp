#include <game/game/d_npc_mng.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_play_util.hpp>
#include <game/game/d_a_str.hpp>
// Inferred names for exports from the structure and field-object RELs.
namespace dAStr {
extern const float cNpcDoorOffset;
mVec3_c getNpcDoorPos(daSTR_c *, const mVec3_c *, int, float);
void requestNpcEnter(daSTR_c *);
void requestNpcExit(daSTR_c *);
} // namespace dAStr
#include <game/game/d_fgobj_manager.hpp>

#include <game/game/d_field_assessment.hpp>
extern "C" int abs(int);

#include <game/game/d_bgm.hpp>

#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_insect_info.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_world.hpp>

#include <game/game/d_npc_list.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_player_mgr.hpp>

#include <game/game/d_a_npc_out.hpp>
#include <game/game/d_a_npc.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/cLib/c_lib.hpp>
#include <cstdlib>
#include <nw4r/ut/ut_algorithm.h>
#include <cmath>
#include <game/snd/snd_id.hpp>

extern "C" long labs(long);

namespace dANpcOut {
// Inferred from the behavior handlers and the shared body animation table.
enum Animation_e {
    ANM_WALK = 5,
    ANM_LEAVE_HOUSE = 7,
    ANM_TOOL_CHANGE = 11,
    ANM_UMBRELLA = 114,
    ANM_PITFALL_STUMBLE = 127,
    ANM_PITFALL_FALL = 128,
    ANM_PITFALL_STRUGGLE = 129,
    ANM_PITFALL_ESCAPE = 130,
    ANM_PITFALL_ESCAPE_WITH_TOOL = 131,
    ANM_APPROACH_NOTICE = 207,
    ANM_APPROACH_FEMALE = 208,
    ANM_APPROACH_FEMALE_LOOP = 209,
    ANM_APPROACH_MALE = 210,
    ANM_APPROACH_MALE_LOOP = 211,
    ANM_NET_HIT_SEEN = 212,
    ANM_NET_HIT_UNSEEN = 213,
    ANM_MASK_CHANGE = 250,
    ANM_WATCH_FIREWORKS = 327
};
enum Priority_e { PRIORITY_IDLE, PRIORITY_REACTION };
enum CollisionWeight_e {
    COLLISION_WEIGHT_NORMAL = 0x80,
    COLLISION_WEIGHT_FIXED = 0xFF
};
enum ToolChangeStep_e {
    TOOL_TAKE_OUT,
    TOOL_SCALE_OUT,
    TOOL_OPEN_UMBRELLA,
    TOOL_FINISH_OPEN_UMBRELLA,
    TOOL_PUT_AWAY,
    TOOL_TAKE_NEXT,
    TOOL_CLOSE_UMBRELLA,
    TOOL_FINISH_CHANGE,
    TOOL_LOCAL_TAKE_OUT,
    TOOL_LOCAL_SCALE_OUT,
    TOOL_LOCAL_OPEN_UMBRELLA,
    TOOL_LOCAL_FINISH_OPEN_UMBRELLA,
    TOOL_LOCAL_PUT_AWAY,
    TOOL_LOCAL_TAKE_NEXT,
    TOOL_LOCAL_CLOSE_UMBRELLA,
    TOOL_LOCAL_FINISH_CHANGE
};

mAng sInsectLookFov(0x37FF), sFishLookFov(0x37FF);
static inline float horizontalDistance(const mVec3_c &a, const mVec3_c &b) {
    mVec3_c v = a - b;
    return EGG::Mathf::sqrt(v.x * v.x + v.z * v.z);
}
} // namespace dANpcOut

// ACTOR FUNCTIONS
// .text CC
namespace dANpcOut {
BASE_PROFILE(A_NPC_OUT, dAcNpcOut_c);
// Behavior settings retained as class data, including folded scalar uses.
const float action_c::cObserveChance = 50.0f;
const u32 action_c::cObserveTimeRange = 600;
const u32 action_c::cObserveTimeBase = 600;
const float action_c::cInsectAcquireDistance = 96.0f;
const float action_c::cInsectTrackDistance = 128.0f;
const float action_c::cFishAcquireDistance = 96.0f;
const float action_c::cFishTrackDistance = 128.0f;
const float action_c::cLookAroundChance = 20.0f;
const u32 action_c::cLookAroundTimeBase = 120;
const u32 action_c::cLookAroundTimeRange = 120;
const u32 action_c::cLookFrontTimeBase = 240;
const u32 action_c::cLookFrontTimeRange = 240;
const action_c::procPair_s sActionPairs[dANpcOut::action_c::ACTION_COUNT] = {
    {&action_c::enterWander, &action_c::executeWander},
    {&action_c::enterOffscreen, &action_c::executeOffscreen},
    {&action_c::enterRemoteHouseWait, &action_c::executeRemoteHouseWait},
    {&action_c::enterAfterTalk, &action_c::executeAfterTalk},
    {&action_c::enterFacePlayer, &action_c::executeFacePlayer},
    {&action_c::enterHideSeekInvite, &action_c::executeHideSeekInvite},
    {&action_c::enterHideSeekStandby, &action_c::executeHideSeekStandby},
    {&action_c::enterHideSeekWait, &action_c::executeHideSeekWait},
    {&action_c::enterHideSeekPeek, &action_c::executeHideSeekPeek},
    {&action_c::enterHideSeekPlay, &action_c::executeHideSeekPlay},
    {&action_c::enterHideSeekWin, &action_c::executeHideSeekWin},
    {&action_c::enterHideSeekLose, &action_c::executeHideSeekLose},
    {&action_c::enterHideSeekFinish, &action_c::executeHideSeekFinish},
    {&action_c::enterRemoteHideWait, &action_c::executeRemoteHideWait},
    {&action_c::enterRemoteHidePeek, &action_c::executeRemoteHidePeek},
    {&action_c::enterHideSeekFacePlayer, &action_c::executeHideSeekFacePlayer},
    {&action_c::enterHideSeekAfterTalk, &action_c::executeHideSeekAfterTalk},
    {&action_c::enterRemoteIdle, &action_c::executeRemoteIdle},
    {&action_c::enterRemoteClothWait, &action_c::executeRemoteClothWait},
    {&action_c::enterToolChange, &action_c::executeToolChange},
    {&action_c::enterRemoteToolChange, &action_c::executeRemoteToolChange},
    {&action_c::enterMaskChange, &action_c::executeMaskChange},
    {&action_c::enterRemoteMaskChange, &action_c::executeRemoteMaskChange},
    {&action_c::enterHalloweenChase, &action_c::executeHalloweenChase},
    {&action_c::enterRemoteHalloweenWait,
     &action_c::executeRemoteHalloweenWait},
    {&action_c::enterNpcChat, &action_c::executeNpcChat},
    {&action_c::enterNpcChatPartner, &action_c::executeNpcChatPartner},
    {&action_c::enterNpcChatAfterTalk, &action_c::executeNpcChatAfterTalk},
    {&action_c::enterNpcChatBusy, &action_c::executeNpcChatBusy},
    {&action_c::enterNpcChatReserved, &action_c::executeNpcChatReserved},
    {&action_c::enterEnterHouse, &action_c::executeEnterHouse},
    {&action_c::enterInHouse, &action_c::executeInHouse},
    {&action_c::enterLeaveHouse, &action_c::executeLeaveHouse},
    {&action_c::enterBirthdayVisit, &action_c::executeBirthdayVisit},
    {&action_c::enterAfterBirthday, &action_c::executeAfterBirthday},
    {&action_c::enterVisitWait, &action_c::executeVisitWait},
    {&action_c::enterPitfall, &action_c::executePitfall},
    {&action_c::enterPitfallTalkReserved,
     &action_c::executePitfallTalkReserved},
    {&action_c::enterAfterPitfallTalk, &action_c::executeAfterPitfallTalk},
    {&action_c::enterNetHit, &action_c::executeNetHit},
    {&action_c::enterNetHitTalkReserved, &action_c::executeNetHitTalkReserved},
    {&action_c::enterAfterNetHitTalk, &action_c::executeAfterNetHitTalk},
    {&action_c::enterItchy, &action_c::executeItchy},
    {&action_c::enterPushed, &action_c::executePushed},
    {&action_c::enterAfterPushedTalk, &action_c::executeAfterPushedTalk},
    {&action_c::enterApproach, &action_c::executeApproach},
    {&action_c::enterAfterApproachTalk, &action_c::executeAfterApproachTalk},
    {&action_c::enterGreeting, &action_c::executeGreeting},
    {&action_c::enterWatchFireworks, &action_c::executeWatchFireworks},
    {&action_c::enterRemoteWatchFireworks,
     &action_c::executeRemoteWatchFireworks},
    {&action_c::enterRequestCatch, &action_c::executeRequestCatch},
    {&action_c::enterRemoteClothChange, &action_c::executeRemoteClothChange},
};
const u32 action_c::cVisitWaitTimeBase = 120;
const u32 action_c::cVisitWaitTimeRange = 600;
const float action_c::cApproachNoticeDistance = 48.0f;
const float action_c::cApproachHomeDistance = 127.0f;
const float action_c::cApproachPursueDistance = 48.0f;
const float action_c::cApproachTalkDistance = 58.0f;
const u32 action_c::cApproachWaitFrames = 600;
const u32 action_c::cApproachLimitFrames = 600;
const u32 action_c::cApproachTurnDistance = 48;
const float action_c::cHalloweenAcquireDistance = 96.0f;
const float action_c::cHalloweenTakeDistance = 38.0f;
const float action_c::cHalloweenTrackDistance = 128.0f;
const float action_c::cHalloweenAskDistance = 40.0f;
const float action_c::cHalloweenTalkDistance = 40.0f;
const u32 action_c::cHalloweenWaitFrames = 120;
const float action_c::cRemoteHalloweenTrackDistance = 128.0f;
} // namespace dANpcOut

// .text 2A8
bool dAcNpcOut_c::canInteract(dDemoActor_c *actor) {
    return dAcNpcNml_c::canInteract(actor) != 0;
}

// .text 2D4
int dAcNpcOut_c::canSpeech(dDemoActor_c *actor) {
    if (!actor) {
        return 0;
    }
    if (!mIsActive) {
        return 0;
    }
    if (isMultiPlay()) {
        return 0;
    }
    if (mBusy) {
        return 0;
    }
    if (isInUse(NULL)) {
        return 0;
    }
    if (mTalk.isBusy()) {
        return 0;
    }
    if (mpChatPartner) {
        return 0;
    }
    if (mOutAction.mAction) {
        return 0;
    }
    if (mHitByNet) {
        return 0;
    }
    // Dispatch through the base interface so the override is emitted with the vtable.
    if (static_cast<dAcNpcNml_c *>(this)->vtCC()) {
        return 0;
    }
    if (mMode) {
        return 0;
    }
    if (!mpEntry || mpEntry->mFlags.mFlag11) {
        return 0;
    }
    int mode = mpEntry->mMoodTimer.getMode();
    if (mode != 0 && mode != 1) {
        return 0;
    }
    dAnimal_c *animal = mpAnimal;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (!animal || !player) {
        return 0;
    }
    dAnimalMemory_c *memory = animal->findMemory2(&player->mPID);
    if (!memory || !memory->mPlayer.isValid()) {
        return 0;
    }
    return 1;
}

// .text 484
void dAcNpcOut_c::onDemoCmd(int cmd, u8 member) {
    _1B70 = member;
    int permit = requestPermit(cmd, member, TRUE);
    switch (cmd) {
    case DEMO_CMD_START:
        if (permit == PERMIT_REMOTE_START) {
            mOutAction.setAction(this, dANpcOut::action_c::ACTION_FACE_PLAYER);
        } else if (permit == PERMIT_LOCAL_START) {
            if (dDemo::isType(dDemo::TYPE_SPEECH)) {
                mOutAction.setAction(this,
                                     dANpcOut::action_c::ACTION_FACE_PLAYER);
            } else if (mOutAction.mRequestedAction <
                       dANpcOut::action_c::ACTION_COUNT) {
                mOutAction.setAction(this, mOutAction.mRequestedAction);
            } else {
                mOutAction.setAction(this,
                                     dANpcOut::action_c::ACTION_FACE_PLAYER);
            }
            mOutAction.mRequestedAction = dANpcOut::action_c::ACTION_COUNT;
        }
        break;
    case DEMO_CMD_TALK:
    case DEMO_CMD_SPEAK:
        if (permit == PERMIT_REMOTE_TALK) {
            mOutAction.setAction(this,
                                 dANpcOut::action_c::ACTION_REMOTE_CLOTH_WAIT);
        } else if (permit == PERMIT_LOCAL_TALK) {
            mOutAction.startTalk(this);
        }
        break;
    case DEMO_CMD_SPEECH:
        mOutAction.setAction(this, dANpcOut::action_c::ACTION_GREETING);
        mOutAction.mAfterTalkAction = dANpcOut::action_c::ACTION_COUNT;
        break;
    case DEMO_CMD_END:
        if (permit == PERMIT_END_SINGLE) {
            mOutAction.saveDefaultAction(this, FALSE);
            mOutAction.restoreAction(this);
        } else if (permit == PERMIT_END_MULTI) {
            mOutAction.saveHouseAction(this);
            mOutAction.restoreAction(this);
        }
        break;
    case CMD_RELEASE_PERMIT:
        if (permit == PERMIT_RELEASED) {
            if (!dNet::isSceneOwner()) {
                mOutAction.saveDefaultAction(this, FALSE);
            } else {
                mOutAction.saveHouseAction(this);
            }
            mOutAction.restoreAction(this);
        }
        break;
    }
    dAcNpc_c::onDemoCmd(cmd, member);
}

// .text 65C
int dAcNpcOut_c::preCreate() {
    if (!dAcNpcNml_c::preCreate()) {
        return 0;
    }
    mMode = MODE_NORMAL;
    mTool.create(this);
    mBirthdayHost = 0;
    return 1;
}

// .text 6BC
static inline dFdBase_c *getOutdoorField() {
    dFdBase_c *field = dField::getInfo(FD_ID_CURRENT);
    return field;
}

int dAcNpcOut_c::create() {
    dFdBase_c *field;
    if (!dAcNpcNml_c::create()) {
        return 0;
    }
    if (!mTool.loadNext(this)) {
        return 0;
    }
    mRoute.init();
    clearNetHitCount();
    mHitByNet = 0;
    mApproachCooldown = 0;
    mChatOutcomePending = 0;
    mFlea.init(this);
    mTalkWeightTimer = TALK_WEIGHT_INTERVAL;
    mPlayerMoved = 0;
    mTargetPos = mVec3_c::Zero;
    mLostItemComplete = 0;
    if (!isMultiPlay()) {
        field = getOutdoorField();
        if (field) {
            const dItem::Item *item = field->getItem(&mPos, 0);
            if (item) {
                bool hole = false;
                u32 id = item->getId();
                if (id >= dItem::FG_HOLE && id <= dItem::FG_HOLE) {
                    hole = true;
                }
                if (hole) {
                    dItem::Item empty;
                    field->setItem(&empty, &mPos, 0);
                }
            }
        }
    }
    dAnimal_c *animal = mpAnimal;
    if (animal && dScene::getPrevScene() == dScene::SCENE_DM_PL_SEL &&
        !dNet::isSessionActive() && !dNpcMng_c::_8074EACF) {
        dSaveTown_c *town = dSaveData_c::getTown();
        mBirthdayHost = town->mAnimals.mTown.isBirthdayHost(
            &animal->mID, dPlayerMgr::getCurrentPlayerSlot(), 1, 0, 1);
    }
    if (mBirthdayHost) {
        const dItem::Item &next = mTool.mNextItem;
        if (next.isValid()) {
            dItem::Item item = next;
            const dItem::BITM *info = dItem::infoBank_c::get()->getBITM(item);
            if (info && info->getKind() == dItem::KIND_UMBRELLA) {
                takeInitialTool();
            }
        }
        mOutAction.setAction(this, dANpcOut::action_c::ACTION_BIRTHDAY_VISIT);
    } else {
        takeInitialTool();
        animal = mpAnimal;
        nw4r::g3d::ResFile special(dNpcMng_c::getSpecialModel());
        if (special.IsValid()) {
            nw4r::g3d::ResMdl model = special.GetResMdl(0);
            int waitAnm = mAnmSet.mWaitAnm;
            mAlternateModel.create(model, waitAnm, &mAllocator, 3, 0);
        }
        if (animal) {
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
            BOOL newPlayer = player ? player->isFlag0(13) : FALSE;
            dSaveData_c *save = dSaveData_c::getRaw();
            dAnimalEventState_c *event = &animal->mEvent;
            int eventId = save->mAnimals.mTown.mEventId;
            if (eventId == EVENT_HALLOWEEN && event->isInEvent() &&
                event->isFlag(0) && !newPlayer &&
                (isLocalOwner() || !isDaubMaskChange())) {
                changeMask();
                if (isLocalOwner() && isDaubMaskChange()) {
                    offDaubMaskChange();
                }
            }
        }
        if (isLocalOwner()) {
            mOutAction.saveDefaultAction(this, TRUE);
            mOutAction.restoreAction(this);
        } else {
            mOutAction.saveHouseAction(this);
            mOutAction.restoreAction(this);
        }
    }
    return 1;
}

// .text A18
int dAcNpcOut_c::doDelete() {
    if (!mTool.remove()) {
        return 0;
    }
    if (m_heap_p) {
        mAlternateModel.remove();
    }
    sendPendingPlace();
    releasePendingUnit();
    dANpcOut::insectLink_s *insect = &mInsectLink;
    if (insect->mpInsect) {
        dInsectField::removeNpcLink(insect->mpInsect, insect);
    }
    dFishLink_c *fish = &mFishLink;
    if (fish->mFish) {
        fish->mFish->removeLink(fish);
    }
    if (!dAcNpcNml_c::doDelete()) {
        return 0;
    }
    return 1;
}

// .text AC0
int dAcNpcOut_c::vtB0() {
    if (mApproachCooldown) {
        --mApproachCooldown;
    }
    mOutAction.execute(this);
    mMood.execute(this);
    mFlea.execute(this);
    dNpc::entry_c *entry = mpEntry;
    if (entry) {
        if (mViewArea0.isOutside(&mPos)) {
            const dNpc::entry_c *snapshot = entry;
            if (snapshot->mFlags.mTalked) {
                entry->mFlags.mTalked = 0;
            }
            if (snapshot->mFlags.mEventTalked) {
                entry->mFlags.mEventTalked = 0;
            }
            if (snapshot->mFlags.mFlag11) {
                entry->mFlags.mFlag11 = 0;
            }
            if (snapshot->mFlags.mPoisonDone) {
                entry->mFlags.mPoisonDone = 0;
            }
        }
        if (mViewArea1.isOutside(&mPos)) {
            dNpc::msgMemorySecond_c *message = &entry->mRememberedMessage;
            if (message->isValid()) {
                message->clear();
            }
            entry->mDialogueCache.clear();
        }
    }
    if (mHitByNet) {
        mFlea.clear();
    }
    mHitByNet = 0;
    return 1;
}

// .text C14
BOOL dAcNpcOut_c::isAppointmentTime(int from, int to) const {
    if (mBirthdayHost) {
        return FALSE;
    }
    dAnimal_c *animal = mpAnimal;
    dAnimalBlock_c *block = &dSaveData_c::getRaw()->mAnimals.mTown;
    if (animal && !dNet::isSessionActive() &&
        block->isAppointmentAnimal(&animal->mID)) {
        dQuestPlayerItem_c *quest = &block->mAppointment;
        dQuestBase_c *base = &quest->mBase;
        if (base->getKind() == QUEST_KIND_APPOINTMENT_0 &&
            base->getState() != 2 &&
            quest->isPastMeetTime(*dTime_c::getCurrent(), from) &&
            !quest->isPastMeetTime(*dTime_c::getCurrent(), to)) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text D0C
BOOL dAcNpcOut_c::isHomeAppointmentTime() const {
    return isAppointmentTime(-60, 30) != 0;
}

// .text D40
BOOL dAcNpcOut_c::isOutdoorAppointmentTime() const {
    return isAppointmentTime(-60, 30) != 0;
}

// .text D74
const dItem::Item *dAcNpcOut_c::getHoldItem() {
    dAnimal_c *animal = mpAnimal;
    if (animal == NULL) {
        return NULL;
    }
    return &animal->mHeldItem;
}

// .text D90
BOOL dAcNpcOut_c::takeInitialTool() {
    toolBase_c *tool = &mTool;
    if (tool->isNextReady()) {
        model_c *model = getModel();
        tool->change(1.0f, this);
        int anm = tool->getAnmId();
        if (anm != HMN_ANM_NUM) {
            model->setRarmAnm(anm, m3d::PLAYMODE_INHERIT, 0.0f, 1.0f, 1.0f);
        }
        return TRUE;
    }
    return FALSE;
}

// .text E4C
BOOL dAcNpcOut_c::syncTool() {
    if (!mpAnimal) {
        return FALSE;
    }
    const dItem::Item *item = &mpAnimal->mHeldItem;
    toolBase_c *tool = &mTool;
    if (tool->mItem.isNotSame(*item)) {
        model_c *model = getModel();
        if (item->mId != dItem::ITEM_ID_NONE) {
            if (tool->isNextLoaded(this)) {
                tool->change(1.0f, this);
                int anm = tool->getAnmId();
                if (anm != HMN_ANM_NUM) {
                    model->setRarmAnm(anm, m3d::PLAYMODE_INHERIT, 0.0f, 1.0f,
                                      1.0f);
                }
                return TRUE;
            }
        } else {
            if (model->mRarmAnm.mAnmId != HMN_ANM_NUM) {
                model->stopRarmAnm(1.0f);
            }
            tool->putAway();
            return TRUE;
        }
    }
    return FALSE;
}

// .text F70
BOOL dAcNpcOut_c::updateHeldItem() {
    if (!mpAnimal) {
        return FALSE;
    }
    if (mpAnimal->mHeldItem.isNotSame(mTool.mItem)) {
        return FALSE;
    }
    if (dSaveData_c::getTown()->mAnimals.mTown.updateHeldItem(getNpcIdx())) {
        offDaubToolChanged();
        return TRUE;
    }
    return FALSE;
}

// .text 1010
int dAcNpcOut_c::getEarType() {
    if (mMode == MODE_MASKED && mAlternateModel.mCreated) {
        return NPC_EAR_TYPE_NONE;
    }
    return dAcNpcNml_c::getEarType();
}

// .text 1038
dAcNpc_c::model_c *dAcNpcOut_c::getModel() {
    if (mMode == MODE_MASKED && mAlternateModel.mCreated) {
        return &mAlternateModel;
    }
    return &mModel;
}

// .text 1060
dAcNpc_c::model_c *dAcNpcOut_c::getAlternateModel() {
    if (mMode == MODE_NORMAL && mAlternateModel.mCreated) {
        return &mAlternateModel;
    }
    return &mModel;
}

// .text 1088
BOOL dAcNpcOut_c::changeMask() {
    model_c *model = getModel(), *alternate = getAlternateModel();
    if (!model->copyAnm(alternate)) {
        return FALSE;
    }
    mMode = mMode == MODE_NORMAL;
    if (!mMode) {
        mFace.reset(&getModel()->mMdl);
        model = getModel();
        if (model->mAnm.mAnmId != HMN_ANM_NUM) {
            mFace.setAnmTex(this, model->mAnm.mAnmId, m3d::PLAYMODE_INHERIT);
        }
    } else {
        mFace.mDisable = 1;
    }
    return TRUE;
}

// .text 1164
BOOL dAcNpcOut_c::needsMaskChange() {
    if (!mpAnimal) {
        return FALSE;
    }
    if (mpAnimal->mHeldItem.isNotSame(mTool.mItem)) {
        return FALSE;
    }
    if (mTool.mItem.mId != dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    dSaveTown_c *town = dSaveData_c::getTown();
    return town->mAnimals.mTown.updateHalloweenFlag(getNpcIdx());
}

// .text 11FC
BOOL dAcNpcOut_c::canStartNpcChat() {
    if (dNet::isSessionActive()) {
        return FALSE;
    }
    if (mOutAction.mCooldown) {
        return FALSE;
    }
    dAnimal_c *animal = mpAnimal;
    if (!animal) {
        return FALSE;
    }
    u8 scene = dScene::getCurrentScene();
    if (dScene::isSceneAttr(scene, 5) && !dScene::isSceneAttr(scene, 32)) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        if (!player) {
            return FALSE;
        }
        if (!animal->findMemory2(&player->mPID)) {
            return FALSE;
        }
    }
    static dQuestEvent_e event0 = EVENT_HARVEST_FESTIVAL;
    static dQuestEvent_e event1 = EVENT_HALLOWEEN;
    static dQuestEvent_e event2 = EVENT_COUNTDOWN;
    static dQuestEvent_e event3 = EVENT_FISHING_TOURNEY;
    static dQuestEvent_e event4 = EVENT_BUG_OFF;
    static dQuestEvent_e event5 = EVENT_FIREWORKS;
    if (dEvent::isActive(event0) || dEvent::isActive(event1) ||
        dEvent::isActive(event2) || dEvent::isActive(event3) ||
        dEvent::isActive(event4) || dEvent::isActive(event5)) {
        return FALSE;
    }
    if ((u32)dSaveData_c::getRaw()->mAnimals.mTown.getHiderIdx(0) < 10) {
        return FALSE;
    }
    if (!animal->isLostItemRequestDone(NULL, 1)) {
        return FALSE;
    }
    if (!canInteract(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT))) {
        return FALSE;
    }
    if (mOutAction.mAction) {
        return FALSE;
    }
    return mpChatPartner == NULL;
}

// .text 13E0
int dAcNpcOut_c::startNpcChat(dAcNpc_c *partner) {
    if (!canStartNpcChat()) {
        return 0;
    }
    float heightDifference = mPos.y - partner->getPosP()->y;
    if (__fabsf(mPos.y - partner->getPosP()->y) > 32.0f) {
        return 0;
    }
    mpChatPartner = partner;
    mOutAction.setAction(this, dANpcOut::action_c::ACTION_NPC_CHAT_PARTNER);
    return 1;
}

// .text 1468
int dAcNpcOut_c::endNpcChat() {
    if (mpChatPartner) {
        stopEmotionManpu();
        mOutAction.saveDefaultAction(this, FALSE);
        mOutAction.restoreAction(this);
        mpChatPartner = NULL;
        mOutAction.startCooldown();
        return 1;
    }
    return 0;
}

// .text 14D8
void dAcNpcOut_c::stopEmotionManpu() {
    if (mAction.mActionId != dAcNpc_c::ACTION_EMOTION) {
        return;
    }
    if (!mAction.mEmotion) {
        return;
    }
    stopManpu();
}

// .text 14F8
void dAcNpcOut_c::vt80() {
    mOutAction.setAction(this, dANpcOut::action_c::ACTION_NPC_CHAT_BUSY);
}

// .text 1508
void dAcNpcOut_c::vt84() {
    mMood.mTimerPending = 1;
    mOutAction.saveDefaultAction(this, FALSE);
    mOutAction.restoreAction(this);
    mpChatPartner = NULL;
    mOutAction.startCooldown();
}

// .text 1564
BOOL dAcNpcOut_c::wantsToGoHome() const {
    dAnimal_c *animal = mpAnimal;
    if (!animal) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player && player->isFlag0(13)) {
        return FALSE;
    }
    static dQuestEvent_e event = EVENT_TOY_DAY;
    if (dEvent::isActive(event)) {
        return FALSE;
    }
    if (!animal->isLostItemRequestDone(player, 1)) {
        return FALSE;
    }
    if (animal->isSleepTime(NULL)) {
        return TRUE;
    }
    if (isHomeAppointmentTime()) {
        return TRUE;
    }
    if (!dNet::isSessionActive()) {
        if (animal->isQuestStarted()) {
            return TRUE;
        }
        if (animal->countNewItems() == 4) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text 1688
BOOL dAcNpcOut_c::hasHouseVisitor() const {
    const dAnimal_c *animal = mpAnimal;
    if (!animal || !animal->mID.isValid()) {
        return FALSE;
    }
    u32 index =
        dSaveData_c::getRaw()->mAnimals.mTown.getAnimalIdx(&animal->mID);
    if (index >= 10) {
        return FALSE;
    }
    u8 scene = index + 17;
    if (!isMultiPlay()) {
        return FALSE;
    }
    int me = dNet::getMyNo();
    for (int i = 0; i < 4; ++i) {
        if (i != me && dNet::isMember(i) &&
            scene == (u8)dNet::getMemberScene(i)) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text 1770
BOOL dAcNpcOut_c::canLeaveHouse() const {
    dAnimal_c *animal = mpAnimal;
    if (!animal) {
        return FALSE;
    }
    if (wantsToGoHome()) {
        return FALSE;
    }
    if (isOutdoorAppointmentTime()) {
        return FALSE;
    }
    if (animal->isPlaceChangeTimeSet()) {
        if (dNet::isSessionActive()) {
            return FALSE;
        }
        if (!animal->isPlaceChangeTimeReached(NULL)) {
            return FALSE;
        }
    }
    if (hasHouseVisitor()) {
        return FALSE;
    }
    return TRUE;
}

// .text 183C
BOOL dAcNpcOut_c::shouldChangeSpot() const {
    dAnimal_c *animal = mpAnimal;
    if (!animal) {
        return FALSE;
    }
    dAnimalSpot_c *spot = &animal->mSpot;
    if (!spot->isValid()) {
        return TRUE;
    }
    dSaveData_c::getRaw();
    int type = spot->mType;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player && player->isFlag0(13)) {
        return type != 2;
    }
    if (type == 2) {
        return TRUE;
    }
    if (!animal->isLostItemRequestDone(NULL, 1)) {
        return type != 3;
    }
    if (type == 3) {
        return TRUE;
    }
    BOOL goHome = wantsToGoHome();
    if (type != 1 && goHome) {
        return TRUE;
    }
    if (type == 1 && !goHome) {
        return TRUE;
    }
    if (type == 0 && (int)spot->mGroup != 1 && spot->hasTime()) {
        dTime_c time = spot->getTime();
        dTime_c *now = dTime_c::getCurrent();
        if (dTime_c::isSameOrAfter(*now, time)) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text 1AB0
BOOL dAcNpcOut_c::wantsTownHallSpot() {
    bool inPlaza;
    dAnimal_c *animal = mpAnimal;
    if (!animal || !animal->mID.isValid()) {
        return FALSE;
    }
    if (!animal->mEvent.isInEvent()) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player && player->isFlag0(13)) {
        return FALSE;
    }
    if (!animal->isLostItemRequestDone(player, 1)) {
        return FALSE;
    }
    if (wantsToGoHome()) {
        return FALSE;
    }
    dAnimalBlock_c *block = &dSaveData_c::getRaw()->mAnimals.mTown;
    int id = block->mEventId;
    dTime_c *now = dTime_c::getCurrent();
    bool activeNow, lateHour, lateMinute;
    bool ongoing = false, preparing = false;
    if (id == 19) {
        dQuestEvent_e event = dQuestEvent_e(id);
        ongoing = dEvent::isOngoing(event) != 0;
        preparing = false;
        if (!ongoing) {
            lateMinute = false;
            lateHour = false;
            activeNow = false;
            dQuestEvent_e active = dQuestEvent_e(id);
            if (dEvent::isActive(active)) {
                dQuestEvent_e over = dQuestEvent_e(id);
                if (!dEvent::isOver(over)) {
                    activeNow = true;
                }
            }
            if (activeNow) {
                if (now->hour == 22) {
                    lateHour = true;
                }
            }
            if (lateHour) {
                if (now->min >= 30) {
                    lateMinute = true;
                }
            }
            if (lateMinute) {
                preparing = true;
            }
        }
    } else if (id == 16) {
        dQuestEvent_e event = dQuestEvent_e(id);
        ongoing = dEvent::isOngoing(event) != 0;
        preparing = false;
        if (!ongoing) {
            activeNow = false;
            lateHour = false;
            lateMinute = false;
            dQuestEvent_e active = dQuestEvent_e(id);
            if (dEvent::isActive(active)) {
                dQuestEvent_e over = dQuestEvent_e(id);
                if (!dEvent::isOver(over)) {
                    lateMinute = true;
                }
            }
            if (lateMinute) {
                if (now->hour >= 18) {
                    lateHour = true;
                }
            }
            if (lateHour) {
                if (now->min >= 30) {
                    activeNow = true;
                }
            }
            if (activeNow) {
                preparing = true;
            }
        }
    }
    if (ongoing || preparing) {
        u32 total =
            (u32)(0.1f +
                  0.4f * (float)(u32)(block->countByOutdoorState(0) * 10));
        u32 max = (total + 5) / 10;
        if (!max) {
            return FALSE;
        }
        dItem::Item plaza((u16)0xD013);
        if (block->countAnimalsNearItem(&plaza, 2) < max &&
            cM::rndF(100.0f) < 60.0f) {
            return TRUE;
        }
        inPlaza = false;
        if (preparing) {
            dItem::Item ownPlaza((u16)0xD013);
            if (animal->isItemInMyBlock(&ownPlaza)) {
                inPlaza = true;
            }
        }
        if (inPlaza) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text 1DD4
BOOL dAcNpcOut_c::chooseSpot() {
    dAnimal_c *animal = mpAnimal;
    if (!animal) {
        return FALSE;
    }
    dAnimalBlock_c *block = &dSaveData_c::getRaw()->mAnimals.mTown;
    dAnimalSpot_c *spot = &animal->mSpot;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player && player->isFlag0(13) && spot->setHomeSpot2(animal, block)) {
        return TRUE;
    }
    if (!animal->isLostItemRequestDone(NULL, 1) &&
        spot->setHomeSpot3(animal, block)) {
        return TRUE;
    }
    if (wantsToGoHome() && spot->setHomeSpot1(animal, block)) {
        return TRUE;
    }
    if (wantsTownHallSpot() && spot->setTownHallSpot(animal, block)) {
        return TRUE;
    }
    return spot->pickWishSpot(animal, block);
}

// .text 1F1C
BOOL dAcNpcOut_c::searchSpotRoute() {
    dAnimal_c *animal = mpAnimal;
    if (!animal) {
        return FALSE;
    }
    dAnimalSpot_c *spot = &animal->mSpot;
    if (spot->isValid() == 0) {
        return FALSE;
    }
    return mRoute.search(spot->mX, spot->mZ, &mPos) != 0;
}

// .text 1F9C
BOOL dAcNpcOut_c::isInSpotBlock() {
    dAnimal_c *animal = mpAnimal;
    if (!animal) {
        return FALSE;
    }
    dAnimalSpot_c *spot = &animal->mSpot;
    if (!spot->isValid()) {
        return FALSE;
    }
    if ((int)spot->mGroup != 1) {
        return TRUE;
    }
    return dANpcOut::route_c::isInsideBlock(spot->mX, spot->mZ, &mPos) != 0;
}

// .text 202C
BOOL dAcNpcOut_c::finishSpotRoute() {
    dAnimal_c *animal = mpAnimal;
    if (!animal) {
        return FALSE;
    }
    dAnimalSpot_c *spot = &animal->mSpot;
    if (!spot->isValid()) {
        return FALSE;
    }
    if ((int)spot->mGroup == 1 && isInSpotBlock()) {
        s64 time = 0x7FFFFFFFFFFFFFFFLL;
        spot->mGroup = 0;
        if (spot->mType == 0) {
            spot->setTime(dTime_c::getCurrent(), 5);
            if (spot->hasTime()) {
                time = spot->getTimeRaw();
            }
        }
        dNpc::sendAnimalSpotTime(getNpcIdx(), 0, &time);
        return TRUE;
    }
    return FALSE;
}

// .text 2128
BOOL dAcNpcOut_c::isAtHomeSpot() {
    dAnimal_c *animal = mpAnimal;
    if (!animal) {
        return FALSE;
    }
    dAnimalSpot_c *spot = &animal->mSpot;
    if (!spot->isValid()) {
        return FALSE;
    }
    if ((int)spot->mType == 1 && (int)spot->mGroup != 1) {
        return TRUE;
    }
    return FALSE;
}

// .text 21A0
BOOL dAcNpcOut_c::getHouseUnit(int *x, int *z) {
    if (mpAnimal == NULL) {
        return FALSE;
    }
    dAnimal_c *animal = mpAnimal;
    return dSaveData_c::getRaw()->mAnimals.mTown.getHouseBlockPosById(
               x, z, &animal->mID) != 0;
}

// .text 2218
BOOL dAcNpcOut_c::getHousePos(mVec3_c *pos) {
    int x = 0, z = 0;
    if (getHouseUnit(&x, &z)) {
        dFdBase_c::getUnitCenterPos(pos, x, z);
        return TRUE;
    }
    return FALSE;
}

// .text 227C
BOOL dAcNpcOut_c::shouldGoHome() {
    if (!mIsActive) {
        return FALSE;
    }
    if (dNet::isSessionActive()) {
        return FALSE;
    }
    if (!isAtHomeSpot()) {
        return FALSE;
    }
    if (!wantsToGoHome()) {
        return FALSE;
    }
    int x = 0, z = 0;
    if (!getHouseUnit(&x, &z)) {
        return FALSE;
    }
    int ux = worldToUnit(mPos.x), uz = worldToUnit(mPos.z);
    if (ux == x && uz == z + 1) {
        return TRUE;
    }
    return FALSE;
}

// .text 2378
BOOL dAcNpcOut_c::updateSpotRoute(BOOL checkDistance) {
    dAnimal_c *animal = mpAnimal;
    if (!animal) {
        return FALSE;
    }
    dAnimalSpot_c *spot = &animal->mSpot;
    if (!spot->isValid()) {
        return FALSE;
    }
    if ((int)spot->mGroup != 1) {
        return FALSE;
    }
    dANpcOut::route_c *route = &mRoute;
    bool search = !route->isDestination(spot->mX, spot->mZ);
    if (checkDistance && !search) {
        u32 count = route->countNodes();
        if (count - 1 < 100) {
            dANpcOut::route_c::node_s *node = route->getNode(count - 1);
            if (node) {
                if (count == 1) {
                    mVec3_c pos = mVec3_c::Zero;
                    dFdBase_c::getUnitCenterPos(&pos, node->mX, node->mZ);
                    if (dANpcOut::horizontalDistance(pos, mPos) > l_8074FE10) {
                        search = true;
                    }
                } else {
                    dANpcOut::route_c::node_s *next = route->getNode(count - 2);
                    if (next) {
                        int x = node->mX, z = node->mZ;
                        int unitX = worldToUnit(mPos.x),
                            unitZ = worldToUnit(mPos.z);
                        int dir = dANpcOut::route_c::getDirTo(next->mX,
                                                              next->mZ, x, z);
                        if (dir) {
                            mVec3_c pos = mVec3_c::Zero;
                            dFdBase_c::getUnitCenterPos(&pos, x, z);
                            if (dANpcOut::horizontalDistance(pos, mPos) >
                                l_8074FE10) {
                                search = true;
                                while (x != next->mX || z != next->mZ) {
                                    if (!dANpcOut::route_c::advanceUnit(&x, &z,
                                                                        dir)) {
                                        break;
                                    }
                                    if (x == unitX && z == unitZ) {
                                        continue;
                                    }
                                    dFdBase_c::getUnitCenterPos(&pos, x, z);
                                    if (dANpcOut::horizontalDistance(
                                            pos, mPos) <= l_8074FE10) {
                                        search = false;
                                        break;
                                    }
                                }
                            }
                        } else {
                            search = true;
                        }
                    } else {
                        search = true;
                    }
                }
            } else {
                search = true;
            }
        } else {
            search = true;
        }
    }
    if (search) {
        return route->search(spot->mX, spot->mZ, &mPos);
    }
    return FALSE;
}

// .text 2684
BOOL dAcNpcOut_c::isPlayerNearHouse() {
    const mVec3_c *player =
        dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (!player) {
        return FALSE;
    }
    int x = 0, z = 0;
    if (!getHouseUnit(&x, &z)) {
        return FALSE;
    }
    mVec3_c pos = mVec3_c::Zero;
    dFdBase_c::getUnitCenterPos(&pos, x, z + 2);
    mVec3_c diff = pos - *player;
    if (EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z) <= 64.0f) {
        return TRUE;
    }
    return FALSE;
}

// .text 2778
void dAcNpcOut_c::setPendingPlace(int place) {
    mPendingPlace = place;
}

// .text 2780
void dAcNpcOut_c::clearPendingPlace() {
    setPendingPlace(PENDING_PLACE_NONE);
}

// .text 2788
void dAcNpcOut_c::sendPendingPlace() {
    if (!dNet::isSessionActive() && (u32)mPendingPlace < PENDING_PLACE_NONE) {
        if (mpAnimal) {
            mpAnimal->mPlace = mPendingPlace;
            dNpc::sendAnimalPlace(getNpcIdx(), mPendingPlace);
        }
        clearPendingPlace();
    }
}

// .text 27F0
void dAcNpcOut_c::setPendingUnit(int x, int z) {
    if (!hasPendingUnit() || !isValidPendingUnit(x, z)) {
        mPendingUnitX = x;
        mPendingUnitZ = z;
    }
}

// .text 285C
void dAcNpcOut_c::clearPendingUnit() {
    setPendingUnit(-1, -1);
}

// .text 2868
BOOL dAcNpcOut_c::isValidPendingUnit(int x, int z) const {
    if (x == -1 || z == -1) {
        return FALSE;
    }
    return TRUE;
}

// .text 2888
bool dAcNpcOut_c::hasPendingUnit() const {
    int x = mPendingUnitX;
    int z = mPendingUnitZ;
    return isValidPendingUnit(x, z) != 0;
}

// .text 28BC
void dAcNpcOut_c::releasePendingUnit() {
    if (!isMultiPlay() && hasPendingUnit()) {
        dFgobjManager::requestPitfallExit(dNet::getMyNo(), mPendingUnitX,
                                          mPendingUnitZ);
        clearPendingUnit();
    }
}

// .text 2918
BOOL dAcNpcOut_c::checkLostItemPickup() {
    if (isMultiPlay()) {
        return FALSE;
    }
    int x = worldToUnit(mPos.x), z = worldToUnit(mPos.z);
    int record = dFgobjManager::requestNpcPitfall((s8)getNpcIdx(), x, z);
    if (record == -1) {
        return FALSE;
    }
    int state = dFgobjManager::getRequestResult(record);
    dFgobjManager::clearRequest(record);
    return state == 1;
}

// .text 29CC
int dAcNpcOut_c::isInPitfall() {
    return mOutAction.mInPitfall != 0;
}

// .text 29E0
int dAcNpcOut_c::vt68(const dItem::Item *item) {
    if (isMultiPlay()) {
        return 2;
    }
    if (dHmnToolBank_c::getItemToolType(item) == HMN_TOOL_NET) {
        mHitByNet = 1;
        return isItchy() != 0;
    }
    return 2;
}

// .text 2A68
void dAcNpcOut_c::clearNetHitCount() {
    mNetHitCount = 0;
}

// .text 2A74
void dAcNpcOut_c::addNetHitCount() {
    dAnimal_c *animal;
    bool known = false;
    animal = mpAnimal;
    if (animal) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
        if (player) {
            if (player->isFlag0(13)) {
                known = false;
            } else {
                dAnimalMemory_c *memory = animal->findMemory2(&player->mPID);
                if (memory && memory->mPlayer.isValid()) {
                    known = true;
                }
            }
        }
    }
    ++mNetHitCount;
    if (mNetHitCount > 3) {
        mNetHitCount = 3;
    }
    if (!known && mNetHitCount == 3) {
        mNetHitCount = 2;
    }
}

// .text 2B50
u8 dAcNpcOut_c::getNetHitCount() {
    return mNetHitCount;
}

// .text 2B58
BOOL dAcNpcOut_c::isNetHitLimit() {
    return getNetHitCount() >= 3;
}

// .text 2B94
void dAcNpcOut_c::clearIdleTimer() {
    mIdleTimer = 0;
}

// .text 2BA0
void dAcNpcOut_c::addIdleTimer() {
    if (mIdleTimer < 300) {
        ++mIdleTimer;
    }
}

// .text 2BB8
void dAcNpcOut_c::subtractIdleTimer() {
    if (mIdleTimer >= 2) {
        mIdleTimer -= 2;
    }
}

// .text 2BD0
BOOL dAcNpcOut_c::isIdleTimerExpired() {
    return mIdleTimer >= 300;
}

// .text 2BF0
int dAcNpcOut_c::isItchy() {
    return mFlea.mState == dANpcOut::flea_c::STATE_ENABLED;
}

// .text 2C00
int dAcNpcOut_c::vtC8() {
    return mChatOutcomePending;
}

// .text 2C08
BOOL dAcNpcOut_c::isPlayerPushing() {
    if (isMultiPlay()) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    if (!player || player->isFlag0(13)) {
        return FALSE;
    }
    if (!mpAnimal) {
        return FALSE;
    }
    dAnimalMemory_c *memory = mpAnimal->findMemory2(&player->mPID);
    if (!memory || !memory->mPlayer.isValid()) {
        return FALSE;
    }
    dAcNpc_c::objc_c *objc = &mObjc;
    if (!objc->mHitType || !objc->isHit(0x80000001)) {
        return FALSE;
    }
    dActor_c *actor =
        dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (!actor || actor->getSpeedF() == 0.0f) {
        return FALSE;
    }
    const mVec3_c *pos = actor->getPosP();
    const mAng3_c *angle = &actor->getAngle3D();
    if (!pos || !angle) {
        return FALSE;
    }
    s16 target = dActor_c::targetAngleY(pos, &mPos);
    return labs((s16)(target - angle->y.mAngle)) < 0x31C7;
}

// .text 2D54
void dAcNpcOut_c::updateIdleTimer() {
    bool pushed = false;
    if (!isMultiPlay() && mIsActive && !mTalk.isBusy() && isPlayerPushing()) {
        pushed = true;
    }
    if (pushed) {
        addIdleTimer();
    } else {
        subtractIdleTimer();
    }
}

// .text 2DE0
dNpc::talkWeight_c *dAcNpcOut_c::getTalkWeight() {
    if (mpEntry) {
        return &mpEntry->mTalkWeight;
    }
    return NULL;
}

// .text 2DFC
const dNpc::talkWeight_c *dAcNpcOut_c::getTalkWeight() const {
    if (mpEntry) {
        return &mpEntry->mTalkWeight;
    }
    return NULL;
}

// .text 2E18
bool dAcNpcOut_c::canChangeTalkWeight(int delta) const {
    const dPersonalID_c *pid;
    dAnimal_c *animal;
    if (isMultiPlay()) {
        return false;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (!player) {
        return false;
    }
    if (player->isFlag0(13)) {
        return false;
    }
    pid = &player->mPID;
    if (!pid->isValid() || !pid->isFromTown()) {
        return false;
    }
    animal = mpAnimal;
    if (!animal) {
        return false;
    }
    if (!mpEntry) {
        return false;
    }
    if (delta >= 0) {
        int mode = mpEntry->mMoodTimer.getMode();
        if (mode != 0 && mode != 1) {
            return false;
        }
    }
    dAnimalMemory_c *memory = animal->findMemory2(pid);
    if (!memory || !memory->mPlayer.isValid()) {
        return false;
    }
    return getTalkWeight() != NULL;
}

// .text 2F5C
void dAcNpcOut_c::addTalkWeight(int delta) {
    if (canChangeTalkWeight(delta)) {
        dNpc::talkWeight_c *weight = getTalkWeight();
        if (weight) {
            weight->add(delta);
            mTalkWeightTimer = TALK_WEIGHT_INTERVAL;
            mPlayerMoved = 0;
            const mVec3_c *pos =
                dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
            if (pos) {
                mTargetPos = *pos;
            }
        }
    }
}

// .text 2FEC
void dAcNpcOut_c::halveTalkWeight() {
    if (canChangeTalkWeight(-1)) {
        dNpc::talkWeight_c *weight = getTalkWeight();
        if (weight) {
            weight->half();
            mTalkWeightTimer = TALK_WEIGHT_INTERVAL;
            mPlayerMoved = 0;
            const mVec3_c *pos =
                dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
            if (pos) {
                mTargetPos = *pos;
            }
        }
    }
}

// .text 3070
BOOL dAcNpcOut_c::updateTalkWeight() {
    if (!canChangeTalkWeight(0)) {
        return FALSE;
    }
    if (!mPlayerMoved) {
        const mVec3_c *pos =
            dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
        if (pos && mTargetPos != *pos) {
            mPlayerMoved = 1;
        }
    }
    if (!mPlayerMoved) {
        return FALSE;
    }
    if (mTalkWeightTimer) {
        --mTalkWeightTimer;
    }
    if (mTalkWeightTimer) {
        return FALSE;
    }
    {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        if (!player) {
            return FALSE;
        }
        dNpc::talkWeight_c *weight = getTalkWeight();
        if (!weight) {
            return FALSE;
        }
        if (!mpAnimal) {
            return FALSE;
        }
        weight->add(
            dNpc::talkWeight_c::calcFriendshipValue(2, 0, mpAnimal, player));
        mTalkWeightTimer = TALK_WEIGHT_INTERVAL;
        mPlayerMoved = 0;
        const mVec3_c *pos =
            dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
        if (pos) {
            mTargetPos = *pos;
        }
        return TRUE;
    }
}

// .text 3200
BOOL dAcNpcOut_c::wantsToApproach() {
    if (isMultiPlay()) {
        return FALSE;
    }
    if (!canChangeTalkWeight(0)) {
        return FALSE;
    }
    dNpc::talkWeight_c *weight = getTalkWeight();
    if (weight == NULL) {
        return FALSE;
    }
    if (weight->mWeight < 32) {
        return FALSE;
    }
    dAnimal_c *animal = mpAnimal;
    if (!animal) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    if (!player) {
        return FALSE;
    }
    dAnimalMemory_c *memory;
    if (player->isFlag0(2) && !player->mMiiMask &&
        (memory = animal->findMemory2(&player->mPID)) &&
        memory->mPlayer.isValid() && !memory->mFlags.mBeeFaceTalked) {
        return FALSE;
    }
    if (!animal->isLostItemRequestDone(player, 0)) {
        return FALSE;
    }
    if (dSaveData_c::getRaw()->mAnimals.mTown.getHiderSlot(&animal->mID) != 3) {
        return FALSE;
    }
    static dQuestEvent_e events[] = {
        EVENT_HARVEST_FESTIVAL, EVENT_HALLOWEEN, EVENT_COUNTDOWN,
        EVENT_FISHING_TOURNEY,  EVENT_BUG_OFF,   EVENT_FIREWORKS};
    u32 i = 0;
    dQuestEvent_e *currentEvent = events;
    for (; i < 6; ++i, ++currentEvent) {
        dQuestEvent_e event = *currentEvent;
        if (dEvent::isOngoing(event)) {
            return FALSE;
        }
    }
    const mVec3_c *pos =
        dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (!pos) {
        return FALSE;
    }
    float maxDistance = 127.0f;
    mVec3_c diff = *pos - mPos;
    if (std::fabs(EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z)) >
        maxDistance) {
        return FALSE;
    }
    if (std::fabs(mPos.y - pos->y) >= 48.0f) {
        return FALSE;
    }
    return mLookAt.isLooking() != 0;
}

// .text 345C
void dAcNpcOut_c::clearTalkWeight() {
    dNpc::talkWeight_c *weight = getTalkWeight();
    if (weight) {
        weight->clear();
    }
    mTalkWeightTimer = TALK_WEIGHT_INTERVAL;
    mPlayerMoved = 0;
    const mVec3_c *pos =
        dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (pos) {
        mTargetPos = *pos;
    }
}

// .text 34CC
int dAcNpcOut_c::getGreetingAnm() {
    dAnimal_c *animal = mpAnimal;
    int anm = 202;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int mode = mpEntry ? mpEntry->mMoodTimer.getMode() : 0;
    u8 looks = animal ? (u8)animal->mID.getLooks(1) : 6;
    if (looks >= 6) {
        looks = 0;
    }
    static const int normal[] = {203, 202, 202, 203, 202, 202},
                     friendly[] = {204, 205, 206, 204, 205, 206};
    switch (mode) {
    case 1:
        anm = normal[looks];
        break;
    case 0:
    default:
        if (animal && player) {
            dAnimalMemory_c *memory = animal->findMemory2(&player->mPID);
            if (memory && memory->mPlayer.isValid()) {
                switch (memory->getFriendshipLevel()) {
                case 1:
                    anm = friendly[looks];
                    break;
                default:
                    anm = normal[looks];
                    break;
                }
            }
        }
    }
    return anm;
}

// .text 35F0
int dAcNpcOut_c::getGreetingSound(int anm) {
    int sound = SE_npc_gre1;
    switch (anm) {
    case 203:
        sound = SE_npc_gre2;
        break;
    case 204:
        sound = SE_npc_cld1;
        break;
    case 205:
        sound = SE_npc_cld2;
        break;
    case 206:
        sound = SE_npc_cld3;
        break;
    }
    return sound;
}

// .text 3640
BOOL dAcNpcOut_c::canWatchFireworks() {
    dAnimalBlock_c *block;
    dAnimal_c *animal = mpAnimal;
    if (!animal || !animal->mID.isValid()) {
        return FALSE;
    }
    if (!animal->mEvent.isInEvent()) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player && player->isFlag0(13)) {
        return FALSE;
    }
    if (!animal->isLostItemRequestDone(player, 1)) {
        return FALSE;
    }
    block = &dSaveData_c::getRaw()->mAnimals.mTown;
    int mode = mpEntry ? mpEntry->mMoodTimer.getMode() : 5;
    if (mode != 0 && mode != 1) {
        return FALSE;
    }
    int id = block->mEventId;
    dTime_c *now = dTime_c::getCurrent();
    bool watch = false;
    if (id == 19) {
        dQuestEvent_e event = dQuestEvent_e(id);
        bool ongoing = false;
        if (dEvent::isOngoing(event)) {
            bool validMinute;
            bool validHour = false;
            if (now->hour < 6) {
                validMinute = false;
                if (now->hour != 0 || now->min >= 2) {
                    validMinute = true;
                }
                if (validMinute) {
                    validHour = true;
                }
            }
            if (validHour) {
                ongoing = true;
            }
        }
        if (ongoing) {
            watch = true;
        }
    } else if (id == 16) {
        dQuestEvent_e event = dQuestEvent_e(id);
        if (dEvent::isOngoing(event)) {
            watch = true;
        }
    }
    if (watch) {
        mVec3_c pos = mPos;
        pos.z -= 32.0f;
        dFdBase_c::snapToUnitCenter(&pos, &pos);
        if (dBGCF::getGroundY(&pos, 0) - mPos.y <= 48.0f) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text 3858
BOOL dAcNpcOut_c::isTurnAngleWide(const mAng *angle) {
    return labs(angle->mAngle) > 0x5000;
}

// .text 3890
mAng dAcNpcOut_c::randomTurnAngle(const mAng *angle) {
    int result = cM::rndInt(0x2FFF) + 0x5001;
    if (isTurnAngleWide(angle)) {
        if (angle->mAngle >= 0) {
            result = (s16)result - ((s16)result * 2);
        }
    } else if (angle->mAngle < 0) {
        result = -(s16)result;
    }
    return mAng(result);
}

// .text 3910
BOOL dAcNpcOut_c::isInsectVisible(void *insect, const mAng *fov,
                                  float distance) {
    if (!insect) {
        return FALSE;
    }
    const mVec3_c *pos = &static_cast<dPlayActor_c *>(insect)->mPos;
    if (std::fabs(pos->y - mPos.y) > 48.0f) {
        return FALSE;
    }
    return lookAt_c::canSee(this, *pos, *fov, distance) != 0;
}

// .text 397C
BOOL dAcNpcOut_c::findInsectCallback(dPlayActor_c *actor, void *arg) {
    dAcNpcOut_c *npc = static_cast<dAcNpcOut_c *>(arg);
    if (!npc) {
        return FALSE;
    }
    if (!actor) {
        return FALSE;
    }
    void *insect = actor;
    mAng angle = dANpcOut::sInsectLookFov;
    if (!npc->isInsectVisible(insect, &angle,
                              dANpcOut::action_c::cInsectAcquireDistance)) {
        return FALSE;
    }
    dANpcOut::insectLink_s *link = &npc->mInsectLink;
    BOOL replace = link->mpInsect ? cM::rndInt(3) == 0 : TRUE;
    if (replace) {
        if (link->mpInsect) {
            dInsectField::removeNpcLink(link->mpInsect, link);
        }
        if (actor) {
            dInsectField::addNpcLink(insect, link);
        }
    }
    return TRUE;
}

// .text 3A5C
BOOL dAcNpcOut_c::findInsect() {
    BOOL found =
        g_playActorMng
            ? g_playActorMng->forEachActiveActor(findInsectCallback, this)
            : FALSE;
    if (!found) {
        dANpcOut::insectLink_s *link = &mInsectLink;
        if (link->mpInsect) {
            dInsectField::removeNpcLink(link->mpInsect, link);
        }
    }
    return mInsectLink.mpInsect != NULL;
}

// .text 3AD8
BOOL dAcNpcOut_c::isFishVisible(dFishFldShadow_c *fish, const mAng *fov,
                                float distance) {
    if (!fish) {
        return FALSE;
    }
    const mVec3_c *pos = &fish->mPos;
    if (std::fabs(pos->y - mPos.y) > 48.0f) {
        return FALSE;
    }
    return lookAt_c::canSee(this, *pos, *fov, distance) != 0;
}

// .text 3B44
BOOL dAcNpcOut_c::findFishCallback(dPlayActor_c *actor, void *arg) {
    dAcNpcOut_c *npc = static_cast<dAcNpcOut_c *>(arg);
    if (!npc) {
        return FALSE;
    }
    if (!actor) {
        return FALSE;
    }
    dFishFldShadow_c *fish = reinterpret_cast<dFishFldShadow_c *>(actor);
    mAng angle = dANpcOut::sFishLookFov;
    if (!npc->isFishVisible(fish, &angle,
                            dANpcOut::action_c::cFishAcquireDistance)) {
        return FALSE;
    }
    dFishLink_c *link = &npc->mFishLink;
    BOOL replace = link->mFish ? cM::rndInt(3) == 0 : TRUE;
    if (replace) {
        if (link->mFish) {
            link->mFish->removeLink(link);
        }
        if (fish) {
            fish->addLink(link);
        }
    }
    return TRUE;
}

// .text 3C24
BOOL dAcNpcOut_c::findFish() {
    BOOL found = g_fishField ? reinterpret_cast<dPlayActorMng_c *>(g_fishField)
                                   ->findPlayerIf(findFishCallback, this)
                             : FALSE;
    if (!found) {
        dFishLink_c *link = &mFishLink;
        if (link->mFish) {
            link->mFish->removeLink(link);
        }
    }
    return mFishLink.mFish != NULL;
}

// .text 3CA0
namespace dANpcOut {
route_c::route_c() {
}
} // namespace dANpcOut

// .text 3CA4
namespace dANpcOut {
route_c::~route_c() {
}
} // namespace dANpcOut

namespace dANpcOut {
// Names inferred from the route-node stack and field unit coordinates.
// .text 3CE4
void route_c::init() {
    clearNodes();
    mDestX = INVALID_UNIT_COORD;
    mDestZ = INVALID_UNIT_COORD;
    clearSegment();
    setNextUnit(INVALID_UNIT_COORD, INVALID_UNIT_COORD);
}
// .text 3D34
BOOL route_c::isValid() const {
    if (mDestX == INVALID_UNIT_COORD || mDestZ == INVALID_UNIT_COORD ||
        countNodes() == 0) {
        return FALSE;
    }
    return TRUE;
}
// .text 3D80
BOOL route_c::isDestination(int x, int z) const {
    if (!isValid() || mDestX != x || mDestZ != z) {
        return FALSE;
    }
    return TRUE;
}
// .text 3DF0
void route_c::node_s::clear() {
    mX = 0;
    mZ = 0;
    mTriedDirs = 0;
}
// .text 3E04
void route_c::node_s::copy(const node_s *other) {
    cLib::memCpy(this, other, sizeof(node_s));
}
// .text 3E0C
BOOL route_c::isValidUnit(int x, int z) {
    if (x <= 0 || z <= 0) {
        return FALSE;
    }
    return TRUE;
}
// .text 3E2C
BOOL route_c::node_s::isValid() const {
    return isValidUnit(mX, mZ);
}
// .text 3E3C
void route_c::clearNodes(u32 count) {
    u32 i = 0;
    // Keep the loop-bound copy distinct for MWCC register allocation.
    const u32 *const bound = &count;
    u32 n = *bound;
    node_s *node = mNodes;
    for (; i < n; ++i, ++node) {
        node->clear();
    }
}

// .text 3E98
void route_c::clearNodes() {
    clearNodes(NODE_COUNT);
}
// .text 3EA0
route_c::node_s *route_c::getNode(u32 index) {
    if (index < NODE_COUNT) {
        return &mNodes[index];
    }
    return NULL;
}
// .text 3EC0
const route_c::node_s *route_c::getNode(u32 index) const {
    if (index < NODE_COUNT) {
        return &mNodes[index];
    }
    return NULL;
}
// .text 3EE0
BOOL route_c::hasSegment() const {
    if (!isValid()) {
        return FALSE;
    }
    if (!isValidUnit(mSegmentX1, mSegmentZ1) ||
        !isValidUnit(mSegmentX0, mSegmentZ0)) {
        return FALSE;
    }
    return TRUE;
}
// .text 3F50
void route_c::setSegment(int x0, int z0, int x1, int z1) {
    mSegmentX0 = x0;
    mSegmentZ0 = z0;
    mSegmentX1 = x1;
    mSegmentZ1 = z1;
}
// .text 3F64
void route_c::clearSegment() {
    setSegment(INVALID_UNIT_COORD, INVALID_UNIT_COORD, INVALID_UNIT_COORD,
               INVALID_UNIT_COORD);
}
// .text 3F78
BOOL route_c::isSegmentEnd(int x, int z) const {
    if (!hasSegment()) {
        return FALSE;
    }
    if ((x == mSegmentX0 && z == mSegmentZ0) ||
        (x == mSegmentX1 && z == mSegmentZ1)) {
        return TRUE;
    }
    return FALSE;
}
// .text 4008
void route_c::setNextUnit(int x, int z) {
    mNextX = x;
    mNextZ = z;
}
// .text 4014
BOOL route_c::hasNextUnit() const {
    return isValidUnit(mNextX, mNextZ) != 0;
}
// .text 404C
int route_c::getRouteDirs(int x, int z, dFdBase_c *field) {
    if (!field) {
        field = dField::getInfo(FD_ID_CURRENT);
    }
    if (!field) {
        return 0;
    }
    return field->getRouteDirs(x, z);
}
// .text 40B4
int route_c::findStartUnit(int *outX, int *outZ, u32 x, u32 z, u32 radius) {
    u32 maxX, maxZ, minX;
    dFdBase_c *field = dField::getInfo(FD_ID_CURRENT);
    if (!field) {
        return 0;
    }
    u32 r = 0;
    mVec3_c pos = mVec3_c::Zero;
    dFdBase_c::getUnitCenterPos(&pos, x, z);
    f32 height = dBGCF::getGroundY(&pos, 0);
    maxX = x;
    maxZ = z;
    for (; r < radius; ++maxX, ++maxZ, ++r) {
        u32 uz = z - r;
        minX = x - r;
        for (; uz <= maxZ; ++uz) {
            for (u32 ux = minX; ux <= maxX; ++ux) {
                int dirs = getRouteDirs(ux, uz, field);
                if (dirs) {
                    mVec3_c candidate = mVec3_c::Zero;
                    dFdBase_c::getUnitCenterPos(&candidate, ux, uz);
                    if (std::fabs(dBGCF::getGroundY(&candidate, 0) - height) <
                        48.0f) {
                        *outX = ux;
                        *outZ = uz;
                        return dirs;
                    }
                }
            }
        }
    }
    return 0;
}
// .text 4240
int route_c::findStartPos(int *outX, int *outZ, const mVec3_c *pos,
                          u32 radius) {
    dFdBase_c *field = dField::getInfo(FD_ID_CURRENT);
    if (!field) {
        return 0;
    }
    int x = worldToUnit(pos->x), z = worldToUnit(pos->z);
    return findStartUnit(outX, outZ, x, z, radius);
}
// .text 42DC
BOOL route_c::pushFront(const node_s *node) {
    if (countNodes() >= NODE_COUNT) {
        return FALSE;
    }
    if (node->isValid()) {
        node_s *dst = &mNodes[NODE_COUNT - 1];
        for (int i = NODE_COUNT - 1; i > 0; --i, --dst) {
            dst->copy((node_s *)nw4r::ut::AddOffsetToPtr(mNodes, i * 3) - 1);
        }
        mNodes[0].copy(node);
        return TRUE;
    }
    return FALSE;
}
// .text 4378
BOOL route_c::popFront(node_s *out, u32 count) {
    if (!count) {
        return FALSE;
    }
    if (count > NODE_COUNT) {
        count = NODE_COUNT;
    }
    if (out) {
        out->copy(&mNodes[count - 1]);
    }
    node_s *dst = mNodes;
    int i = 0;
    int remaining = NODE_COUNT - count;
    for (; i < remaining; ++dst, ++i) {
        dst->copy(&mNodes[i + count]);
    }
    dst = &mNodes[remaining];
    for (; (u32)remaining < NODE_COUNT; ++dst, ++remaining) {
        dst->clear();
    }
    if (out && out->isValid()) {
        return TRUE;
    }
    return FALSE;
}
// .text 4470
BOOL route_c::popBack(node_s *out, u32 count) {
    if (!count) {
        return FALSE;
    }
    node_s *node;
    u32 size = countNodes();
    if (count > size) {
        count = size;
    }
    count = size - count;
    if (count >= NODE_COUNT) {
        return FALSE;
    }
    if (out) {
        out->copy(&mNodes[count]);
    }
    node = &mNodes[count];
    for (; count < size; ++node, ++count) {
        node->clear();
    }
    if (out && out->isValid()) {
        return TRUE;
    }
    return FALSE;
}
// .text 4558
int route_c::findNode(int x, int z) const {
    if (!isValidUnit(x, z)) {
        return -1;
    }
    const node_s *node = getNode(0);
    for (int i = 0; i < NODE_COUNT; ++i, ++node) {
        if (x == node->mX && z == node->mZ) {
            return i;
        }
    }
    return -1;
}
// .text 468C
u32 route_c::countNodes() const {
    const node_s *node = getNode(0);
    u32 count = 0;
    for (u32 i = 0; i < NODE_COUNT; ++i, ++node) {
        if (!node->isValid()) {
            break;
        }
        ++count;
    }
    return count;
}
// .text 46FC
int route_c::chooseDir(int dirs, int excluded, int x, int z, int destX,
                       int destZ) {
    int dir = DIR_NONE;
    int available = dirs & ~excluded;
    if (!available) {
        return 0;
    }
    int dx = destX - x, dz = destZ - z;
    if (labs(dx) >= labs(dz)) {
        if (dx > 0) {
            if ((available >> 3) & 1) {
                dir = DIR_EAST;
            }
        } else if (dx < 0) {
            if ((available >> 1) & 1) {
                dir = DIR_WEST;
            }
        }
        if (!dir) {
            if (dz > 0) {
                if ((available >> 2) & 1) {
                    dir = DIR_SOUTH;
                }
            } else if (dz < 0) {
                if (available & 1) {
                    dir = DIR_NORTH;
                }
            }
        }
    } else {
        if (dz > 0) {
            if ((available >> 2) & 1) {
                dir = DIR_SOUTH;
            }
        } else if (dz < 0) {
            if (available & 1) {
                dir = DIR_NORTH;
            }
        }
        if (!dir) {
            if (dx > 0) {
                if ((available >> 3) & 1) {
                    dir = DIR_EAST;
                }
            } else if (dx < 0) {
                if ((available >> 1) & 1) {
                    dir = DIR_WEST;
                }
            }
        }
    }
    if (!dir) {
        u32 count = 0;
        for (int i = 0; i < DIRECTION_COUNT; ++i) {
            if ((available >> i) & 1) {
                ++count;
                f32 chance = 100.0f / count;
                if (cM::rndF(100.0f) < chance) {
                    dir = DIR_NORTH << i;
                }
            }
        }
    }
    return dir;
}
// .text 48B4
int route_c::oppositeDir(int dir) {
    if (dAcNpc_c::countDirBits(dir) != 1) {
        return 0;
    }
    int opposite = dir << 2;
    if (!(opposite & DIR_ALL)) {
        opposite >>= DIRECTION_COUNT;
    }
    return opposite;
}
// .text 4900
BOOL route_c::isDeadEnd(int dirs, int incoming) {
    int opposite = oppositeDir(incoming);
    if (opposite) {
        dirs &= ~opposite;
    }
    if (!dirs) {
        return TRUE;
    }
    return FALSE;
}
// .text 4950
BOOL route_c::isJunction(int dirs, int incoming) {
    int opposite = oppositeDir(incoming);
    if (opposite) {
        dirs &= ~opposite;
        if (dirs == incoming) {
            return FALSE;
        }
    }
    return TRUE;
}
// .text 49AC
int route_c::getDirTo(int x, int z, int destX, int destZ) {
    if (destX != x && destZ != z) {
        return DIR_NONE;
    }
    if (destX == x) {
        if (destZ > z) {
            return DIR_NORTH;
        }
        if (destZ < z) {
            return DIR_SOUTH;
        }
    } else if (destZ == z) {
        if (destX > x) {
            return DIR_WEST;
        }
        if (destX < x) {
            return DIR_EAST;
        }
    }
    return DIR_NONE;
}
struct unitOffset_s {
    int x, z;
};
// .text 4A10
BOOL route_c::offsetUnit(int *outX, int *outZ, int x, int z, u32 dir) {
    static unitOffset_s sUnitOffsets[DIRECTION_COUNT] = {
        {0, -1}, {-1, 0}, {0, 1}, {1, 0}};

    if (dir < DIRECTION_COUNT) {
        x += sUnitOffsets[dir].x;
        z += sUnitOffsets[dir].z;
        *outX = x;
        *outZ = z;
        return TRUE;
    }
    return FALSE;
}
// .text 4A50
int route_c::advanceUnit(int *x, int *z, int dir) {
    if (dAcNpc_c::countDirBits(dir) != 1) {
        return 0;
    }
    u32 index = dAcNpc_c::getSingleDir(dir);
    if (offsetUnit(x, z, *x, *z, index)) {
        return getRouteDirs(*x, *z, NULL);
    }
    return 0;
}
// .text 4AE8
int route_c::backtrack(int *x, int *z, int destX, int destZ, dFdBase_c *field) {
    if (!field) {
        return 0;
    }
    node_s node;
    int result = 0;
    node.clear();
    getNode(0);
    while (popFront(&node, 1)) {
        int dirs = getRouteDirs(node.mX, node.mZ, field);
        int dir =
            chooseDir(dirs, node.mTriedDirs, node.mX, node.mZ, destX, destZ);
        if (dir) {
            result = dir;
            *x = node.mX;
            *z = node.mZ;
            node.mTriedDirs |= dir;
            if (!pushFront(&node)) {
                return 0;
            }
            break;
        }
        node.clear();
    }
    return result;
}
// .text 4BE8
BOOL route_c::isInsideBlock(int x, int z, int unitX, int unitZ) {
    int blockX, blockZ;
    int localX = unitX - ((blockX = unitX >> 4) << 4);
    int localZ = unitZ - ((blockZ = unitZ >> 4) << 4);
    int destBlockZ = z >> 4, destBlockX = x >> 4;
    if (blockX == destBlockX && blockZ == destBlockZ && localX > 0 &&
        localX < 15 && localZ > 0 && localZ < 15) {
        return TRUE;
    }
    return FALSE;
}
// .text 4C48
BOOL route_c::isInsideBlock(int x, int z, const mVec3_c *pos) {
    float px = pos->x;
    float pz = pos->z;
    return isInsideBlock(x, z, (int)px >> 5, (int)pz >> 5);
}
// .text 4C90
BOOL route_c::search(int destX, int destZ, const mVec3_c *pos) {
    route_c *route = this;
    clearNodes();
    dFdBase_c *field = dField::getInfo(FD_ID_CURRENT);
    if (!field) {
        return FALSE;
    }
    int startX = 0, startZ = 0;
    int dirs = findStartPos(&startX, &startZ, pos, 3);
    int index;
    int nextDirs;
    int dir;
    if (!dirs) {
        return FALSE;
    }
    dir = chooseDir(dirs, 0, startX, startZ, destX, destZ);
    if (!dir) {
        return FALSE;
    }
    node_s node;
    node.clear();
    node.mX = startX;
    node.mZ = startZ;
    node.mTriedDirs = dir;
    pushFront(&node);
    int x = startX, z = startZ;
    for (;;) {
        nextDirs = advanceUnit(&x, &z, dir);
        if (isInsideBlock(destX, destZ, x, z)) {
            node.clear();
            node.mX = x;
            node.mZ = z;
            if (pushFront(&node)) {
                mDestX = destX;
                mDestZ = destZ;
                return TRUE;
            }
            clearNodes();
            return FALSE;
        }
        if (isDeadEnd(nextDirs, dir)) {
            node_s unused;
            unused.clear();
            int tmpdir = backtrack(&x, &z, destX, destZ, field);
            if (tmpdir) {
                dir = tmpdir;
            } else {
                return FALSE;
            }
        } else if (isJunction(nextDirs, dir)) {
            index = route->findNode(x, z);
            if ((u32)index < NODE_COUNT) {
                int opposite = oppositeDir(dir);
                if (opposite) {
                    getNode(index)->mTriedDirs |= opposite;
                }
                int tmpdir = backtrack(&x, &z, destX, destZ, field);
                if (tmpdir) {
                    dir = tmpdir;
                } else {
                    return FALSE;
                }
            } else {
                int opposite = oppositeDir(dir);
                dir = chooseDir(nextDirs, opposite, x, z, destX, destZ);
                if (dir) {
                    node.clear();
                    node.mX = x;
                    node.mZ = z;
                    node.mTriedDirs = dir | opposite;

                    // Retain the route copy for MWCC call-argument scheduling.
                    route = this;
                    if (!route->pushFront(&node)) {
                        return FALSE;
                    }
                } else {
                    return FALSE;
                }
            }
        }
    }
}
// .text 4F50
int route_c::findRouteIndex(int x, int z) {
    if (!isValid()) {
        return -1;
    }
    if (!getRouteDirs(x, z, NULL)) {
        return -1;
    }
    u32 count = countNodes();
    if (count == 1) {
        node_s *node = getNode(0);
        if (node && node->mX == x && node->mZ == z) {
            return 0;
        }
    } else {
        int dir;
        for (int i = count - 1; i > 0; --i) {
            node_s *a = getNode(i), *b = getNode(i - 1);
            if (!a || !b) {
                break;
            }
            dir = getDirTo(b->mX, b->mZ, a->mX, a->mZ);
            if (dir) {
                int ux = a->mX, uz = a->mZ;
                if (ux == x && uz == z) {
                    return i;
                }
                if (x == b->mX && z == b->mZ) {
                    return i - 1;
                }
                while (ux != b->mX || uz != b->mZ) {
                    int dirs = advanceUnit(&ux, &uz, dir);
                    if (ux == x && uz == z) {
                        return i;
                    }
                    if (!dirs) {
                        break;
                    }
                }
            }
        }
    }
    return -1;
}
// .text 5108
void route_c::trimToPos(const mVec3_c *pos) {
    if (!isValid()) {
        return;
    }
    float px = pos->x;
    float pz = pos->z;
    int x = (int)px >> 5, z = (int)pz >> 5;
    int index = findRouteIndex(x, z);
    if (isSegmentEnd(x, z)) {
        int other;
        if (x != mSegmentX1 || z != mSegmentZ1) {
            other = findRouteIndex(mSegmentX1, mSegmentZ1);
        } else {
            other = findRouteIndex(mSegmentX0, mSegmentZ0);
        }
        if ((u32)other < NODE_COUNT &&
            ((u32)index >= NODE_COUNT || other < index)) {
            index = other;
        }
    }
    if ((u32)index < NODE_COUNT) {
        int last = countNodes() - 1;
        if (index < last) {
            popBack(NULL, last - index);
        }
    }
}
// .text 522C
void route_c::trimToNearbyPos(const mVec3_c *pos) {
    int z;
    int x;
    float px;
    float pz;
    if (!isValid()) {
        return;
    }
    px = pos->x;
    pz = pos->z;
    x = (int)px >> 5;
    z = (int)pz >> 5;
    u32 best = -1;
    for (int uz = z; uz <= z; ++uz) {
        for (int ux = x; ux <= x; ++ux) {
            u32 index = findRouteIndex(ux, uz);
            if (index < NODE_COUNT &&
                (best >= NODE_COUNT || (int)index < (int)best)) {
                best = index;
            }
        }
    }
    if (best < NODE_COUNT) {
        u32 last = countNodes() - 1;
        if (best < last) {
            popBack(NULL, last - best);
        }
    }
}

typedef char CheckNodeSize[sizeof(route_c::node_s) == 3 ? 1 : -1];
typedef char CheckRouteSize[sizeof(route_c) == 0x14C ? 1 : -1];
} // namespace dANpcOut

namespace dANpcOut {
extern const float cPitfallMoveSpeed;
extern const float cPitfallMoveAccel;
dAcNpc_c::moveParam_c sPitfallMove(cPitfallMoveSpeed, cPitfallMoveAccel,
                                 cPitfallMoveAccel);

// .text 5318
action_c::action_c() {
    mBlockedFrames = 0;
    mTimer2 = 0;
    mLookTimer = 0;
    mAction = ACTION_COUNT;
    mpProcs = NULL;
    mStep = 0;
    mTimer0 = 0;
    mTimer1 = 0;
    mCooldown = 0;
    mpTalkSet = NULL;
    mRequestedAction = ACTION_COUNT;
    mAfterTalkAction = ACTION_COUNT;
    mSavedAction = ACTION_WANDER;
    mInPitfall = 0;
}

// .text 535C
action_c::~action_c() {
}

// .text 539C
void action_c::request(u32 action, u32 afterTalk,
                       const dAcNpcNml_c::talk_c::procSet_s *talk) {
    mRequestedAction = action;
    mAfterTalkAction = afterTalk;
    mpTalkSet = talk;
}

// .text 53AC
void action_c::setAction(dAcNpcOut_c *npc, u32 action) {
    if (action < ACTION_COUNT) {
        mAction = action;
        mpProcs = &sActionPairs[action];
        mStep = 0;
        if (mpProcs) {
            (this->*mpProcs->enter)(npc);
        }
    }
}

// .text 53FC
void action_c::execute(dAcNpcOut_c *npc) {
    if (mTimer0) {
        --mTimer0;
    }
    if (mTimer1) {
        --mTimer1;
    }
    if (mCooldown) {
        --mCooldown;
    }
    if (mTimer2) {
        --mTimer2;
    }
    if (mpProcs) {
        (this->*mpProcs->execute)(npc);
    }
}

// .text 5480
void action_c::startTalk(dAcNpcOut_c *npc) {
    u32 afterTalk = mAfterTalkAction;
    dAcNpcNml_c::talk_c *talk;
    const dAcNpcNml_c::talk_c::procSet_s *procs = mpTalkSet;
    if (dDemo::isType(dDemo::TYPE_SPEAK) &&
        npc->mMode == dAcNpcOut_c::MODE_MASKED) {
        procs = dAcNpcNml_c::talk_c::getProcSet(
            dAcNpcNml_c::talk_c::TALK_PROC_CANDY_ASK);
    }
    talk = &npc->mNmlTalk;
    talk->start(npc,
                dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT),
                procs);
    if (static_cast<const dAcNpcOut_c *>(npc)->mpChatPartner) {
        talk->setPartner(npc->mpChatPartner);
    }
    if (afterTalk < ACTION_COUNT) {
        setAction(npc, afterTalk);
    } else {
        setAction(npc, ACTION_AFTER_TALK);
    }
    mAfterTalkAction = ACTION_COUNT;
}

// .text 5550
u32 action_c::getMaskedIdleAction(dAcNpcOut_c *npc, BOOL allowJoin) {
    u32 action = ACTION_COUNT;
    if (npc->mMode == dAcNpcOut_c::MODE_MASKED) {
        action = 23;
    }
    return action;
}

// .text 5568
u32 action_c::getHideSeekAction(dAcNpcOut_c *npc, BOOL allowJoin) {
    int state;
    dAnimalBlock_c *block = &dSaveData_c::getRaw()->mAnimals.mTown;
    dAnimal_c *animal = npc->mpAnimal;
    if (!animal || !animal->mID.isValid() ||
        dScene::getCurrentScene() == dScene::SCENE_DM_TITLE) {
        return ACTION_COUNT;
    }
    dQuestPlayerAnimal_c *quest;
    int slot = block->getHiderSlot(&animal->mID);
    if (slot == 3) {
        return ACTION_COUNT;
    }
    u32 action = ACTION_COUNT;
    quest = &block->mHideAndSeek;
    state = quest->mBase.mState;
    const dTime_c *now = dTime_c::getCurrent();
    switch (state) {
    case 0:
        if (slot == 0) {
            action = ACTION_HIDE_SEEK_INVITE;
        } else if (allowJoin) {
            action = ACTION_HIDE_SEEK_STANDBY;
        }
        break;
    case 1:
        if (!quest->countUnfound()) {
            if (slot == 0) {
                action = ACTION_HIDE_SEEK_WIN;
            } else {
                action = ACTION_HIDE_SEEK_FINISH;
            }
        } else if (!quest->isTimeUp(*now, 0)) {
            if (quest->isFlag(slot)) {
                action = ACTION_HIDE_SEEK_PLAY;
            } else {
                action = ACTION_HIDE_SEEK_WAIT;
            }
        }
        break;
    case 2:
        action = slot == 0 ? ACTION_HIDE_SEEK_LOSE : ACTION_HIDE_SEEK_FINISH;
        break;
    }
    return action;
}

// .text 56D0
void action_c::saveDefaultAction(dAcNpcOut_c *npc, BOOL allowJoin) {
    Action_e action = ACTION_COUNT;

    if (action == ACTION_COUNT && npc->mBirthdayHost) {
        action = ACTION_VISIT_WAIT;
    }

    if (action == ACTION_COUNT &&
        (!npc->mpAnimal || npc->mpAnimal->mPlace != dAnimal_c::PLACE_OUTSIDE)) {
        action = ACTION_IN_HOUSE;
    }

    if (action == ACTION_COUNT) {
        action = (Action_e)getMaskedIdleAction(npc, allowJoin);
    }

    if (action == ACTION_COUNT) {
        action = (Action_e)getHideSeekAction(npc, allowJoin);
    }

    if (action == ACTION_COUNT) {
        action = ACTION_WANDER;
    }

    mSavedAction = action;
}

// .text 5790
u32 action_c::getMaskedWaitAction(dAcNpcOut_c *npc) {
    u32 action = ACTION_COUNT;
    if (npc->mMode == dAcNpcOut_c::MODE_MASKED) {
        action = ACTION_REMOTE_HALLOWEEN_WAIT;
    }
    return action;
}

// .text 57A8
u32 action_c::getHideSeekReturnAction(dAcNpcOut_c *npc) {
    const dTime_c *now;
    dQuestPlayerAnimal_c *quest;
    dAnimal_c *animal;
    int slot;
    dAnimalBlock_c *block;
    u32 action;
    int state;
    block = &dSaveData_c::getRaw()->mAnimals.mTown;
    animal = npc->mpAnimal;
    if (!animal || !animal->mID.isValid()) {
        return ACTION_COUNT;
    }
    slot = block->getHiderSlot(&animal->mID);
    if (slot == 3) {
        return ACTION_COUNT;
    }
    action = ACTION_COUNT;
    quest = &block->mHideAndSeek;
    state = quest->mBase.mState;
    now = dTime_c::getCurrent();
    switch (state) {
    case 1:
        if (quest->countUnfound() && !quest->isTimeUp(*now, 0) &&
            !quest->isFlag(slot)) {
            action = ACTION_REMOTE_HIDE_WAIT;
        }
        break;
    default:
        break;
    }
    return action;
}

// .text 58A0
void action_c::saveHouseAction(dAcNpcOut_c *npc) {
    int action = ACTION_COUNT;
    if (npc->isDaubInHouse()) {
        action = ACTION_REMOTE_HOUSE_WAIT;
    }
    if (action == ACTION_COUNT) {
        action = getMaskedWaitAction(npc);
    }
    if (action == ACTION_COUNT) {
        action = getHideSeekReturnAction(npc);
    }
    if (action == ACTION_COUNT) {
        action = ACTION_REMOTE_IDLE;
    }
    mSavedAction = action;
}

const u32 action_c::cCooldownFrames = 3600;

// .text 5934
void action_c::startCooldown() {
    mCooldown = cCooldownFrames;
}

// .text 5940
void action_c::restoreAction(dAcNpcOut_c *npc) {
    setAction(npc, mSavedAction);
}

// .text 5948
dAcNpcNml_c *action_c::findChatPartner(dAcNpcOut_c *npc) {
    dAnimal_c *animal = npc->mpAnimal;
    if (!animal) {
        return NULL;
    }
    u32 count = dNpcList::getNmlNum();
    for (u32 i = 0; i < count; ++i) {
        dAcNpcNml_c *other = static_cast<dAcNpcNml_c *>(dNpcList::getNml(i));
        if (other && other->mpAnimal && other->mpAnimal->mID != animal->mID) {
            const mVec3_c *npcPos = &npc->mPos;
            const mVec3_c *otherPos = other->getPosP();
            mVec3_c delta = *otherPos - *npcPos;
            if (EGG::Math<f32>::sqrt(delta.x * delta.x + delta.z * delta.z) <
                48.0f) {
                mAng angle = dActor_c::targetAngleY(npcPos, otherPos);
                angle -= npc->mAngle.y;
                if (dAcNpc_c::lookAt_c::isWithin(angle, l_lookYawMax)) {
                    return other;
                }
            }
        }
    }
    return NULL;
}

// .text 5AF0
void action_c::setMood(dAcNpcOut_c *npc, int mood, int hours, BOOL pending) {
    if (npc) {
        dNpcMood_c *npcMood = &npc->mMood;
        npcMood->startTimer(mood, hours);
        if (pending) {
            npcMood->mTimerPending = 1;
        }
    }
}

// .text 5B4C
void action_c::enterWander(dAcNpcOut_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE);
    mTimer1 = (u32)cM::rndF(600.0f) + 120;
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->offDaubInHouse();
    npc->mCheckGround = 1;
    mSavedAction = ACTION_WANDER;
    request(
        ACTION_FACE_PLAYER, ACTION_AFTER_TALK,
        dAcNpcNml_c::talk_c::getProcSet(dAcNpcNml_c::talk_c::TALK_PROC_ENTRY));
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_NORMAL);
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    mLookTimer = (u32)(120.0f + cM::rndF(600.0f));
    npc->mBusy = 0;
    npc->offDaubToolChange();
    npc->mMood.mDisabled = 0;
    npc->mMood.mShowEffects = 1;
    mBlockedFrames = 0;
    npc->mApproachActive = 0;
    npc->offDaubWatchFireworks();
    mWasShowingCatch = 0;
}

// .text 5CC0
BOOL action_c::avoidObstacle(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    BOOL result = FALSE;
    if (!action->isArrived(npc, 1)) {
        mVec3_c pos;
        int side = npc->getSidePos(&pos);
        switch (side) {
        case 1:
            action->requestWait(PRIORITY_IDLE);
            result = TRUE;
            break;
        case 2:
            action->setGoalPos(pos);
            break;
        }
    } else if (action->isDetour()) {
        action->resetGoal();
    }
    return result;
}

// .text 5D80
BOOL action_c::shouldChangeTool(dAcNpcOut_c *npc) {
    if (!npc->mpAnimal) {
        return FALSE;
    }
    const dItem::Item *item;
    dAcNpcOut_c::toolMng_c *tool = &npc->mTool;
    item = &npc->mpAnimal->mHeldItem;
    bool changed = !tool->mItem.isSame(*item);
    if (changed &&
        (item->mId == dItem::ITEM_ID_NONE || tool->isNextLoaded(npc))) {
        return TRUE;
    }
    return FALSE;
}

// .text 5E18
BOOL action_c::shouldChangeEventMode(dAcNpcOut_c *npc) {
    dAnimal_c *animal = npc->mpAnimal;
    if (!animal) {
        return FALSE;
    }
    if ((int)dSaveData_c::getRaw()->mAnimals.mTown.mEventId != 21) {
        return FALSE;
    }
    BOOL flag = animal->mEvent.isFlag(0);
    int mode = npc->mMode;
    if ((!flag && mode == 1) || (flag && mode == 0)) {
        return TRUE;
    }
    return FALSE;
}

// .text 5EB4
BOOL action_c::tryStartNpcChat(dAcNpcOut_c *npc) {
    bool attempt = false;
    if (cM::rndInt(5) == 0 && npc->canStartNpcChat()) {
        attempt = true;
    }
    if (attempt) {
        dAcNpcNml_c *other = findChatPartner(npc);
        if (other && other->startNpcChat(npc)) {
            npc->mpChatPartner = other;
            setAction(npc, ACTION_NPC_CHAT);
            return TRUE;
        }
    }
    return FALSE;
}

// .text 5F74
BOOL action_c::stopIfPushed(dAcNpcOut_c *npc) {
    float speed = npc->mSpeedF;
    if (0.0f != speed) {
        bool pushed = false;
        if (mBlockedFrames > 120) {
            pushed = true;
        }
        if (npc->mPushTimer > 120) {
            pushed = true;
        }
        if (pushed) {
            npc->mAction.requestWait(PRIORITY_IDLE);
            npc->mPushTimer = 0;
            mBlockedFrames = 0;
            return TRUE;
        }
    }
    return FALSE;
}

// .text 6014
bool action_c::tryReturnLostItem(dAcNpcOut_c *npc) {
    if (npc->checkLostItemPickup()) {
        setAction(npc, ACTION_PITFALL);
        return true;
    }
    return false;
}

// .text 6074
BOOL action_c::isPlayerShowingCatch() {
    if (dAcNpc_c::isMultiPlay()) {
        return FALSE;
    }
    if (!dPlayerMgr::isActionItemState()) {
        return FALSE;
    }
    const dItem::Item item(dPlayerMgr::getActiveActionItem());
    if (item.mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    const dItem::BITM *info = dItem::infoBank_c::get()->getBITM(item);
    if (!info) {
        return FALSE;
    }
    int kind = info->getKind();
    if (kind != 8 && kind != 7) {
        return FALSE;
    } else {
        return TRUE;
    }
    return FALSE;
}

// .text 6124
BOOL action_c::canRequestCatch(dAcNpcOut_c *npc) {
    if (dAcNpc_c::isMultiPlay()) {
        return FALSE;
    }
    if (mWasShowingCatch) {
        return FALSE;
    }
    if (npc->mLostItemComplete) {
        return FALSE;
    }
    if (!npc->mpAnimal || !npc->mpAnimal->isLostItemRequestDone(NULL, 1)) {
        return FALSE;
    }
    int mode = npc->mpEntry ? npc->mpEntry->mMoodTimer.getMode() : 5;
    if (mode != 0 && mode != 1) {
        return FALSE;
    }
    if (!isPlayerShowingCatch()) {
        return FALSE;
    }
    dAcNpc_c::lookAt_c *look = &npc->mLookAt;
    if (look->mType != dAcNpc_c::lookAt_c::LOOK_PLAYER) {
        return FALSE;
    }
    return look->isLooking() != 0;
}

// .text 622C
BOOL action_c::tryIdleAction(dAcNpcOut_c *npc) {
    if (shouldChangeTool(npc)) {
        setAction(npc, ACTION_TOOL_CHANGE);
        return TRUE;
    }
    if (npc->needsMaskChange() && shouldChangeEventMode(npc)) {
        mSavedAction = ACTION_HALLOWEEN_CHASE;
        setAction(npc, ACTION_MASK_CHANGE);
        return TRUE;
    }
    if (tryReturnLostItem(npc)) {
        return TRUE;
    }
    if (npc->isIdleTimerExpired()) {
        setAction(npc, ACTION_PUSHED);
        return TRUE;
    }
    if (npc->mHitByNet) {
        if (npc->isItchy()) {
            setAction(npc, ACTION_ITCHY);
        } else {
            setAction(npc, ACTION_NET_HIT);
        }
        return TRUE;
    }
    if (npc->wantsToApproach()) {
        setAction(npc, ACTION_APPROACH);
        return TRUE;
    }
    if (tryStartNpcChat(npc)) {
        return TRUE;
    }
    if (canRequestCatch(npc)) {
        setAction(npc, ACTION_REQUEST_CATCH);
        return TRUE;
    }
    float speed = npc->mSpeedF;
    if (0.0f != speed) {
        if (avoidObstacle(npc)) {
            return TRUE;
        }
        if (stopIfPushed(npc)) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text 6420
BOOL action_c::findHomeWalkPos(mVec3_c *pos, dAcNpcOut_c *npc) {
    mVec3_c current = npc->mPos;
    int hx = 0, hz = 0;
    if (!npc->getHouseUnit(&hx, &hz)) {
        return FALSE;
    }
    float px = npc->mPos.x;
    float pz = npc->mPos.z;
    int x = (int)px >> 5, z = (int)pz >> 5;
    if (!dNet::isSessionActive() && npc->wantsToGoHome() &&
        !npc->isPlayerNearHouse()) {
        if (x == hx && z >= hz + 2 && z <= hz + 3) {
            dFdBase_c::getUnitCenterPos(pos, hx, hz + 1);
            return TRUE;
        }
        if (x >= hx - 3 && x <= hx + 3 && z >= hz + 1 && z <= hz + 6) {
            dFdBase_c::getUnitCenterPos(pos, hx, (cM::rndInt(2) + 1) + hz);
            return TRUE;
        }
    }
    mVec3_c center = mVec3_c::Zero;
    dFdBase_c::getUnitCenterPos(&center, hx, hz + 4);
    if (dAcNpc_c::searchPosInCircle(pos, &center, &npc->mPos, 96.0f, l_8074FE14,
                                    l_8074FE10)) {
        return TRUE;
    }
    return FALSE;
}

// .text 6604
BOOL action_c::findRouteWalkPos(mVec3_c *pos, dAcNpcOut_c *npc) {
    route_c *route = &npc->mRoute;
    if (!route->isValid()) {
        return FALSE;
    }
    float px = npc->mPos.x;
    float pz = npc->mPos.z;
    int x = (int)px >> 5, z = (int)pz >> 5;
    u32 count = route->countNodes();
    if (count == 1) {
        route_c::node_s *node = route->getNode(count - 1);
        if (node) {
            mVec3_c center = mVec3_c::Zero;
            dFdBase_c::getUnitCenterPos(&center, node->mX, node->mZ);
            if (dAcNpc_c::searchPosInCircle(pos, &center, &npc->mPos, 32.0f,
                                            l_8074FE14, l_8074FE10)) {
                return TRUE;
            }
        }
    } else {
        int fromX = x, fromZ = z;
        int x0, z0, x1, z1;
        route_c::node_s *node;
        bool found;
        int index;
        if (route->hasSegment() && route->isSegmentEnd(x, z)) {
            index = route->findRouteIndex(x, z);
            fromX = -1;
            fromZ = -1;
            u32 otherIndex;
            if (x != route->mSegmentX1 || z != route->mSegmentZ1) {
                otherIndex =
                    route->findRouteIndex(route->mSegmentX1, route->mSegmentZ1);
                if (otherIndex < 100) {
                    fromX = route->mSegmentX1;
                    fromZ = route->mSegmentZ1;
                }
            } else {
                otherIndex =
                    route->findRouteIndex(route->mSegmentX0, route->mSegmentZ0);
                if (otherIndex < 100) {
                    fromX = route->mSegmentX0;
                    fromZ = route->mSegmentZ0;
                }
            }
            if (otherIndex < 100 &&
                ((u32)index >= 100 || (int)otherIndex < index)) {
                index = otherIndex;
            } else {
                fromX = x;
                fromZ = z;
            }
        } else {
            index = route->findRouteIndex(x, z);
        }
        x0 = -1;
        z0 = -1;
        x1 = -1;
        z1 = -1;
        if ((u32)index < 100) {
            node = route->getNode(index);
            if (node) {
                if (!index) {
                    mVec3_c center = mVec3_c::Zero;
                    dFdBase_c::getUnitCenterPos(&center, node->mX, node->mZ);
                    if (dAcNpc_c::searchPosInCircle(pos, &center, &npc->mPos,
                                                    32.0f, l_8074FE14,
                                                    l_8074FE10)) {
                        return TRUE;
                    }
                } else {
                    if (index < count - 1) {
                        route->popBack(NULL, count - 1 - index);
                    }
                    node = route->getNode(index - 1);
                    if (node && node->isValid()) {
                        x1 = node->mX;
                        z1 = node->mZ;
                        x0 = fromX;
                        z0 = fromZ;
                        if (fromX == x1 && fromZ == z1) {
                            found = false;
                            if (index > 1 &&
                                (node = route->getNode(index - 2))) {
                                x0 = x1;
                                z0 = z1;
                                x1 = node->mX;
                                z1 = node->mZ;
                                found = true;
                            }
                            if (!found) {
                                return FALSE;
                            }
                        }
                    }
                }
            }
        } else {
            route_c::node_s *a = route->getNode(count - 1),
                            *b = route->getNode(count - 2);
            if (a && b) {
                x0 = a->mX;
                z0 = a->mZ;
                x1 = b->mX;
                z1 = b->mZ;
            }
        }
        if (x0 != -1 && z0 != -1 && x1 != -1 && z1 != -1) {
            int dir = route_c::getDirTo(x1, z1, x0, z0);
            if (dir) {
                int matches = 0, chosenX = -1, chosenZ = -1, segmentX = -1,
                    segmentZ = -1;
                int cx = x0, cz = z0;
                static const int horizontal[] = {route_c::DIR_NORTH,
                                                 route_c::DIR_SOUTH},
                                 vertical[] = {route_c::DIR_WEST,
                                               route_c::DIR_EAST};
                while (cx != x1 || cz != z1) {
                    if (!route_c::advanceUnit(&cx, &cz, dir)) {
                        break;
                    }
                    if (cx != x || cz != z) {
                        mVec3_c candidate = mVec3_c::Zero;
                        dFdBase_c::getUnitCenterPos(&candidate, cx, cz);
                        dItem::Item empty;
                        empty.mId = dItem::ITEM_ID_NONE;
                        bool canPut = dNpc::canPutItemAt(&candidate, &empty,
                                                         NULL, 0x44, 1, 1) != 0;
                        if (canPut) {
                            mVec3_c diff = candidate - npc->mPos;
                            if (EGG::Mathf::sqrt(diff.x * diff.x +
                                                 diff.z * diff.z) <=
                                l_8074FE10) {
                                ++matches;
                                float chance = 100.0f / (float)(u32)matches;
                                if (cM::rndF(100.0f) < chance) {
                                    chosenX = cx;
                                    chosenZ = cz;
                                    segmentX = cx;
                                    segmentZ = cz;
                                }
                            }
                        }
                        const int *sides = horizontal;
                        u32 sideCount = 2;
                        if (dir & route_c::DIR_NORTH ||
                            dir & route_c::DIR_SOUTH) {
                            sides = vertical;
                            sideCount = 2;
                        }
                        for (u32 i = 0; i < sideCount; ++i) {
                            int sx = cx, sz = cz;
                            if (route_c::advanceUnit(&sx, &sz, sides[i]) &&
                                (sx != x || sz != z)) {
                                dFdBase_c::getUnitCenterPos(&candidate, sx, sz);
                                dItem::Item empty;
                                empty.mId = dItem::ITEM_ID_NONE;
                                bool canPut =
                                    dNpc::canPutItemAt(&candidate, &empty, NULL,
                                                       0x44, 1, 1) != 0;
                                if (canPut) {
                                    mVec3_c diff = candidate - npc->mPos;
                                    if (EGG::Mathf::sqrt(diff.x * diff.x +
                                                         diff.z * diff.z) <=
                                        l_8074FE10) {
                                        ++matches;
                                        float chance =
                                            100.0f / (float)(u32)matches;
                                        if (cM::rndF(100.0f) < chance) {
                                            chosenX = sx;
                                            chosenZ = sz;
                                            segmentX = cx;
                                            segmentZ = cz;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                if (chosenX != -1 && chosenZ != -1 && segmentX != -1 &&
                    segmentZ != -1) {
                    dFdBase_c::getUnitCenterPos(pos, chosenX, chosenZ);
                    route->setSegment(chosenX, chosenZ, segmentX, segmentZ);
                    return TRUE;
                }
            }
        }
    }
    route->clearSegment();
    return FALSE;
}

// .text 6CDC
BOOL action_c::findBlockWalkPos(mVec3_c *pos, int x, int z, dAcNpcOut_c *npc) {
    mVec3_c center = mVec3_c::Zero;
    dFdBase_c::getBlockCenterPos(&center, x >> 4, z >> 4);
    if (dAcNpc_c::searchPosInCircle(pos, &center, &npc->mPos, 256.0f,
                                    l_8074FE14, l_8074FE10)) {
        return TRUE;
    }
    return FALSE;
}

// .text 6D7C
BOOL action_c::findHouseWalkPos(mVec3_c *pos, dAcNpcOut_c *npc) {
    mVec3_c current = npc->mPos;
    int x = 0, z = 0;
    if (!npc->getHouseUnit(&x, &z)) {
        return FALSE;
    }
    mVec3_c center = mVec3_c::Zero;
    dFdBase_c::getUnitCenterPos(&center, x, z + 4);
    if (dAcNpc_c::searchPosInCircle(pos, &center, &npc->mPos, 96.0f, l_8074FE14,
                                    l_8074FE10)) {
        return TRUE;
    }
    return FALSE;
}

// .text 6E64
BOOL action_c::findWalkPos(mVec3_c *pos, dAcNpcOut_c *npc) {
    dAnimalSpot_c *spot = npc->mpAnimal ? &npc->mpAnimal->mSpot : NULL;
    BOOL result = FALSE;
    if (spot && spot->isValid()) {
        int type = spot->mType;
        if ((int)spot->mGroup == 1) {
            result = findRouteWalkPos(pos, npc);
        } else {
            switch (type) {
            case 1:
                result = findHomeWalkPos(pos, npc);
                break;
            case 3:
                result = findHouseWalkPos(pos, npc);
                break;
            default:
                result = findBlockWalkPos(pos, spot->mX, spot->mZ, npc);
                break;
            }
        }
    }
    if (!result) {
        result =
            dAcNpc_c::searchPosNear(pos, &npc->mPos, l_8074FE14, l_8074FE10);
    }
    return result;
}

// .text 6F94
BOOL action_c::tryGoHome(dAcNpcOut_c *npc) {
    if (npc->shouldGoHome()) {
        setAction(npc, ACTION_ENTER_HOUSE);
        return true;
    }
    return false;
}

// .text 6FF4
BOOL action_c::shouldChangeLook(dAcNpcOut_c *npc) {
    if (!mLookTimer) {
        return TRUE;
    }
    switch (npc->mLookAt.mType) {
    case 2: {
        void *insect = npc->mInsectLink.mpInsect;
        dFishFldShadow_c *fish = npc->mFishLink.mFish;
        if (insect) {
            if (!npc->isInsectVisible(insect, &l_lookFov, 128.0f)) {
                return TRUE;
            }
        } else {
            if (fish) {
                if (!npc->isFishVisible(fish, &l_lookFov, 128.0f)) {
                    return TRUE;
                }
            } else {
                return TRUE;
            }
        }
        break;
    }
    }
    return FALSE;
}

// .text 70B0
void action_c::executeWander(dAcNpcOut_c *npc) {
    dPlayActor_c *insect;
    dFishFldShadow_c *fish;
    dAcNpc_c::lookAt_c *look;
    BOOL outside;
    BOOL showing = isPlayerShowingCatch();
    if (npc->mLostItemComplete && !dAcNpc_c::isMultiPlay() && !showing) {
        npc->mLostItemComplete = 0;
    }
    npc->updateIdleTimer();
    outside = npc->mViewArea.isOutsideAll(4, npc);
    if (npc->getSpeedF() != 0.0f) {
        if ((npc->mAcch.mPrevHitFlags & 0x88) ||
            (npc->mAcch.mHitFlags & 0x88)) {
            if (mBlockedFrames < 0xFFFFFFFF) {
                ++mBlockedFrames;
            }
        } else {
            mBlockedFrames = 0;
        }
    } else {
        mBlockedFrames = 0;
    }
    if (npc->shouldChangeSpot() && npc->chooseSpot()) {
        npc->searchSpotRoute();
    }
    if (npc->finishSpotRoute() &&
        npc->mAction.mActionId != dAcNpc_c::ACTION_WAIT) {
        npc->mAction.requestWait(PRIORITY_IDLE);
        mTimer1 = 120;
        return;
    }
    if (outside) {
        setAction(npc, ACTION_OFFSCREEN);
    } else if (!tryIdleAction(npc)) {
        npc->updateHeldItem();
        if (npc->mAction.mCanChange &&
            (npc->mAction.mActionId != dAcNpc_c::ACTION_WAIT || !mTimer1)) {
            mVec3_c pos = npc->mPos;
            if (tryGoHome(npc)) {
                return;
            }
            npc->updateSpotRoute(1);
            if (npc->mAction.mActionId != dAcNpc_c::ACTION_WAIT) {
                npc->mRoute.trimToPos(&npc->mPos);
            }
            if (cM::rndInt(10) < 4 && npc->canWatchFireworks()) {
                setAction(npc, ACTION_WATCH_FIREWORKS);
                return;
            }
            if (cM::rndInt(5) < 3 && findWalkPos(&pos, npc)) {
                mAng angle(0);
                if (npc->checkTargetAngle(&angle, &pos, &l_frontAngle) &&
                    horizontalDistance(pos, npc->mPos) > l_8074FE14) {
                    npc->mAction.requestWalk(
                        PRIORITY_IDLE, pos, l_walkTurnSpeed, 0, l_moveParamWalk,
                        l_defaultAnmRate, cNpcMorphFrames);
                } else {
                    npc->mAction.requestTurnWalk(
                        PRIORITY_IDLE, pos, angle, l_turnSpeed, 0, l_moveParamWalk,
                        l_defaultAnmRate, cNpcMorphFrames);
                }
            } else {
                mTimer1 = cM::rndInt(600) + 120;
                if (npc->mAction.mActionId != dAcNpc_c::ACTION_WAIT) {
                    npc->mAction.requestWait(PRIORITY_IDLE);
                }
            }
        }
        look = &npc->mLookAt;
        int type = look->mType;
        if (mLookTimer) {
            --mLookTimer;
        }
        if (shouldChangeLook(npc)) {
            float observeChance = cObserveChance;
            int step;
            bool changed = false;
            if ((!dAcNpc_c::isMultiPlay() &&
                 cM::rndF(100.0f) < observeChance) != FALSE) {
                int timer = cM::rndInt(cObserveTimeRange) + cObserveTimeBase;
                u32 choice = cM::rndInt(4);
                if (!(choice & 1) && npc->findInsect()) {
                    if (npc->mInsectLink.mpInsect) {
                        look->setPos(0,
                                     static_cast<dPlayActor_c *>(
                                         npc->mInsectLink.mpInsect)
                                         ->mPos,
                                     true, l_lookPitchStep, l_lookYawStep,
                                     128.0f);
                        mLookTimer = timer;
                        changed = true;
                        dFishLink_c *fishLink = &npc->mFishLink;
                        if (fishLink->mFish) {
                            fishLink->mFish->removeLink(fishLink);
                        }
                    }
                } else if (npc->findFish() && npc->mFishLink.mFish) {
                    look->setPos(0, npc->mFishLink.mFish->mPos, true,
                                 l_lookPitchStep, l_lookYawStep, 128.0f);
                    mLookTimer = timer;
                    changed = true;
                    insectLink_s *insectLink = &npc->mInsectLink;
                    if (insectLink->mpInsect) {
                        dInsectField::removeNpcLink(insectLink->mpInsect,
                                                    insectLink);
                    }
                }
            }
            float lookChance = cLookAroundChance;
            if (!changed && type != dAcNpc_c::lookAt_c::LOOK_AROUND &&
                cM::rndF(100.0f) < lookChance) {
                step = l_lookYawStep;
                mAng pitch, yaw, yawStep;
                yawStep = cM::rndInt(step) + (step >> 1);
                yaw = l_lookYawMax >> 1;
                pitch = cM::rndInt(l_lookPitchMax >> 1);
                look->setAround(0, pitch, yaw, l_lookPitchStep, yawStep);
                mLookTimer =
                    cM::rndInt(cLookAroundTimeRange) + cLookAroundTimeBase;
                changed = true;
                insectLink_s *insectLink = &npc->mInsectLink;
                if (insectLink->mpInsect) {
                    dInsectField::removeNpcLink(insectLink->mpInsect,
                                                insectLink);
                }
                dFishLink_c *fishLink = &npc->mFishLink;
                if (fishLink->mFish) {
                    fishLink->mFish->removeLink(fishLink);
                }
            }
            if (!changed) {
                if (type != dAcNpc_c::lookAt_c::LOOK_PLAYER) {
                    look->setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                                    cNpcLookRange);
                }
                mLookTimer =
                    cM::rndInt(cLookFrontTimeRange) + cLookFrontTimeBase;
                insectLink_s *insectLink = &npc->mInsectLink;
                if (insectLink->mpInsect) {
                    dInsectField::removeNpcLink(insectLink->mpInsect,
                                                insectLink);
                }
                dFishLink_c *fishLink = &npc->mFishLink;
                if (fishLink->mFish) {
                    fishLink->mFish->removeLink(fishLink);
                }
            }
        } else {
            if (type == 2) {
                insect = static_cast<dPlayActor_c *>(npc->mInsectLink.mpInsect);
                fish = npc->mFishLink.mFish;
                if (insect) {
                    look->mTargetPos = insect->mPos;
                } else if (fish) {
                    look->mTargetPos = fish->mPos;
                }
            }
        }
    }
    mWasShowingCatch = showing;
}

// .text 76E0
void action_c::enterOffscreen(dAcNpcOut_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE);
    npc->mIsActive = 0;
    npc->offDaubToolChange();
    npc->mMood.mShowEffects = 0;
    mTimer0 = 120;
    npc->mApproachActive = 0;
    npc->offDaubWatchFireworks();
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
}

// .text 77A8
BOOL action_c::canStepUnit(int x, int z, int fromX, int fromZ, u32 dir) {
    if ((route_c::getRouteDirs(fromX, fromZ, NULL) >> dir) & 1) {
        return TRUE;
    }
    mVec3_c from = mVec3_c::Zero;
    dFdBase_c::getUnitCenterPos(&from, fromX, fromZ);
    float y = dBGCF::getGroundY(&from, 0);
    mVec3_c to = mVec3_c::Zero;
    dFdBase_c::getUnitCenterPos(&to, x, z);
    float ty = dBGCF::getGroundY(&to, 0);
    if (std::fabs(y - ty) < 0.1f) {
        return TRUE;
    }
    return FALSE;
}

// .text 78B8
BOOL action_c::findStepUnit(int *x, int *z, int destX, int destZ, int fromX,
                            int fromZ, int lastX, int lastZ) {
    // bool canStep;
    // bool canPut;
    // bool newUnit;
    int dir = dAcNpc_c::getFreeDirToward(destX, destZ, fromX, fromZ);
    int nx = -1, nz = -1;
    if (!route_c::offsetUnit(&nx, &nz, fromX, fromZ, dir)) {
        return FALSE;
    }
    {
        bool canPut = false;
        bool canStep = false;
        bool newUnit = nx != lastX || nz != lastZ ? false : true;
        if (!newUnit && canStepUnit(nx, nz, fromX, fromZ, dir)) {
            canStep = true;
        }
        if (canStep) {
            dItem::Item empty;
            empty.mId = dItem::ITEM_ID_NONE;
            if (dNpc::canPutItemOnUnit(nx, nz, &empty, NULL, 0x44, 1, 1)) {
                canPut = true;
            }
        }
        if (canPut) {
            *x = nx;
            *z = nz;
            return TRUE;
        }
    }
    int bits = dAcNpc_c::getFreeDirBitsToward(destX, destZ, fromX, fromZ);
    bool canPut;
    bool canStep;
    for (int i = 0; i < 4; ++i) {
        if (i != dir && ((bits >> i) & 1) &&
            route_c::offsetUnit(&nx, &nz, fromX, fromZ, i)) {
            canPut = false;
            canStep = false;
            bool newUnit = nx != lastX || nz != lastZ ? false : true;
            if (!newUnit && canStepUnit(nx, nz, fromX, fromZ, i)) {
                canStep = true;
            }
            if (canStep) {
                dItem::Item empty;
                empty.mId = dItem::ITEM_ID_NONE;
                if (dNpc::canPutItemOnUnit(nx, nz, &empty, NULL, 0x44, 1, 1)) {
                    canPut = true;
                }
            }
            if (canPut) {
                *x = nx;
                *z = nz;
                return TRUE;
            }
        }
    }
    return FALSE;
}

// .text 7B10
BOOL action_c::stepAlongRoute(dAcNpcOut_c *npc) {
    route_c *route = &npc->mRoute;
    if (!route->isValid()) {
        return FALSE;
    }
    float px = npc->mPos.x;
    float pz = npc->mPos.z;
    int x = (int)px >> 5, z = (int)pz >> 5;
    u32 count = route->countNodes();
    route_c::node_s *node = route->getNode(count - 1);
    int dx = -1, dz = -1;
    if (node) {
        if (count == 1 && (node->mX != x || node->mZ != z)) {
            dx = node->mX;
            dz = node->mZ;
        } else {
            node = route->getNode(count - 2);
            if (node && node->mX == x && node->mZ == z && count > 2) {
                node = route->getNode(count - 3);
                if (!node || (node->mX == x && node->mZ == z)) {
                    node = NULL;
                }
            }
            if (node) {
                dx = node->mX;
                dz = node->mZ;
            }
        }
    }
    if (dx != -1 && dz != -1) {
        int nx = -1, nz = -1;
        if (findStepUnit(&nx, &nz, dx, dz, x, z, route->mNextX,
                         route->mNextZ)) {
            mVec3_c pos = npc->mPos;
            dFdBase_c::getUnitCenterPos(&pos, nx, nz);
            pos.y = dBGCF::getGroundY(&pos, 0);
            npc->mLastPos = npc->mPos = pos;
            route->setNextUnit(x, z);
            return TRUE;
        }
    }
    return FALSE;
}

// .text 7D0C
BOOL action_c::stepTowardBlock(int x, int z, dAcNpcOut_c *npc) {
    route_c *route = npc->getRoute();
    float px = npc->mPos.x, pz = npc->mPos.z;
    int fromX = worldToUnit(px), fromZ = worldToUnit(pz);
    int chosenX = -1, chosenZ = -1;
    for (int i = 0; i < 9; ++i) {
        int offsetX = cM::rndInt(12) + 2;
        int offsetZ = cM::rndInt(12) + 2;
        int tx = (x & ~(UT_X_NUM - 1)) + offsetX,
            tz = (z & ~(UT_Z_NUM - 1)) + offsetZ;
        if ((tx != fromX || tz != fromZ) &&
            (tx != route->mNextX || tz != route->mNextZ)) {
            int nx = -1, nz = -1;
            if (findStepUnit(&nx, &nz, tx, tz, fromX, fromZ, route->mNextX,
                             route->mNextZ)) {
                chosenX = nx;
                chosenZ = nz;
                break;
            }
        }
    }
    if (chosenX != -1 && chosenZ != -1) {
        mVec3_c pos = npc->mPos;
        dFdBase_c::getUnitCenterPos(&pos, chosenX, chosenZ);
        pos.y = dBGCF::getGroundY(&pos, 0);
        npc->mLastPos = npc->mPos = pos;
        route->setNextUnit(fromX, fromZ);
        return TRUE;
    }
    if (stepNearby(npc)) {
        return TRUE;
    }
    return FALSE;
}

// .text 7EB4
BOOL action_c::stepNearby(dAcNpcOut_c *npc) {
    bool canStep, canStand;
    route_c *route = npc->getRoute();
    int nx = -1, nz = -1;
    // Keep the comparison reads separate from the canStepUnit arguments.
    const int &candidateX = nx, &candidateZ = nz;
    int chosenX = -1, chosenZ = -1;
    int count = 0;
    float px = npc->mPos.x;
    float pz = npc->mPos.z;
    int x = worldToUnit(px), z = worldToUnit(pz);
    for (int i = 0; i < 4; ++i) {
        canStand = false;
        canStep = false;
        if (route_c::offsetUnit(&nx, &nz, x, z, i) &&
            (candidateX != route->mNextX || candidateZ != route->mNextZ) &&
            canStepUnit(nx, nz, x, z, i)) {
            canStep = true;
        }
        if (canStep) {
            dItem::Item empty;
            empty.mId = dItem::ITEM_ID_NONE;
            if (dNpc::canPutItemOnUnit(nx, nz, &empty, NULL, 0x44, 1, 1)) {
                canStand = true;
            }
        }
        if (canStand) {
            ++count;
            float chance = 100.0f / (float)(u32)count;
            if (cM::rndF(100.0f) < chance) {
                chosenX = nx;
                chosenZ = nz;
            }
        }
    }
    if ((chosenX == -1 || chosenZ == -1) && route->hasNextUnit()) {
        int dx = x - route->mNextX, dz = z - route->mNextZ;
        if (labs(dx) <= 1 && labs(dz) <= 1 && labs(dx + dz) == 1) {
            dItem::Item empty;
            empty.mId = dItem::ITEM_ID_NONE;
            bool put = dNpc::canPutItemOnUnit(route->mNextX, route->mNextZ,
                                              &empty, NULL, 0x44, 1, 1);
            if (put) {
                chosenX = route->mNextX;
                chosenZ = route->mNextZ;
            }
        }
    }
    if (chosenX != -1 && chosenZ != -1) {
        mVec3_c pos = npc->mPos;
        dFdBase_c::getUnitCenterPos(&pos, chosenX, chosenZ);
        pos.y = dBGCF::getGroundY(&pos, 0);
        npc->mLastPos = npc->mPos = pos;
        route->setNextUnit(x, z);
        return TRUE;
    }
    return FALSE;
}

// .text 818C
BOOL action_c::stepOutsideView(dAcNpcOut_c *npc) {
    if (mTimer0) {
        return FALSE;
    }
    dAnimalSpot_c *spot = npc->mpAnimal ? &npc->mpAnimal->mSpot : NULL;
    BOOL result = FALSE;
    if (spot && spot->isValid()) {
        if ((int)spot->mGroup == 1) {
            result = stepAlongRoute(npc);
        } else {
            result = stepTowardBlock(spot->mX, spot->mZ, npc);
        }
    }
    if (!result) {
        result = stepNearby(npc);
    }
    return result;
}

// .text 8268
void action_c::executeOffscreen(dAcNpcOut_c *npc) {
    dAnimal_c *animal = npc->mpAnimal;
    npc->updateTalkWeight();
    if (!animal) {
        return;
    }
    if (npc->wantsToGoHome() && !npc->hasHouseVisitor()) {
        mVec3_c house = mVec3_c::Zero;
        if (npc->getHousePos(&house) &&
            npc->mViewArea.isOutsideAll(4, &house)) {
            if (npc->isHomeAppointmentTime()) {
                dSaveTown_c *town = dSaveData_c::getTown();
                animal->mPlace = dAnimal_c::PLACE_HOME;
                town->mAnimals.mTown.mAppointment.mBase.mState = 1;
            } else if (animal->isSleepTime(NULL)) {
                animal->mPlace = dAnimal_c::PLACE_ASLEEP;
            } else if (!dNet::isSessionActive() &&
                       (animal->isQuestStarted() ||
                        animal->countNewItems() == 4)) {
                animal->mPlace = dAnimal_c::PLACE_HOME;
            }
            int place = animal->mPlace;
            if (place) {
                dNpc::sendAnimalPlace(npc->getNpcIdx(), place);
                animal->applyNewItems(npc->getNpcIdx(), 1);
                animal->clearQuestStarted();
                dNpc::sendAnimalQuestClear(npc->getNpcIdx());
                if (place != dAnimal_c::PLACE_ASLEEP &&
                    !dNet::isSessionActive()) {
                    animal->setPlaceChangeTime(10, NULL);
                }
                setAction(npc, ACTION_IN_HOUSE);
                return;
            }
        }
    }

    switch (animal->mPlace) {
    case dAnimal_c::PLACE_OUTSIDE:
        if (!npc->mViewArea.isOutsideAll(4, npc)) {
            npc->mRoute.clearSegment();
            restoreAction(npc);
        } else if (!mSavedAction) {
            if (npc->shouldChangeSpot() && npc->chooseSpot()) {
                npc->searchSpotRoute();
            }
            npc->finishSpotRoute();
            if (!mTimer0) {
                npc->updateSpotRoute(0);
                npc->mRoute.trimToNearbyPos(&npc->mPos);
                stepOutsideView(npc);
                mTimer0 = 120;
            }
        }
        break;
    default:
        setAction(npc, ACTION_IN_HOUSE);
        break;
    }
}

// .text 84F0
void action_c::enterRemoteHouseWait(dAcNpcOut_c *npc) {
    npc->mAcceptMoveReq = 0;
    npc->mAction.requestWait(PRIORITY_IDLE);
    npc->mIsActive = 0;
    npc->mMood.mShowEffects = 0;
    npc->mBusy = 1;
    npc->mApproachActive = 0;
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
}

// .text 85A0
void action_c::executeRemoteHouseWait(dAcNpcOut_c *npc) {
    mVec3_c pos = npc->mPos;
    mAng angle = npc->mAngle.y;
    if (npc->getDaubPos(&pos, &angle)) {
        npc->mLastPos = npc->mPos = pos;
        npc->mAngle.y = angle;
        npc->mAngle3D.y = angle;
    }
    if (npc->isLocalOwner()) {
        if (npc->checkLocalOwner()) {
            npc->fn_800134A4();
            saveDefaultAction(npc, FALSE);
            restoreAction(npc);
        }
    } else {
        if (isRemoteToolReady(npc)) {
            npc->syncTool();
        } else if (shouldChangeEventMode(npc)) {
            npc->changeMask();
        } else if (npc->isClothChangeReady()) {
            npc->changeClothModel();
        } else if (!npc->isDaubInHouse()) {
            saveHouseAction(npc);
            restoreAction(npc);
        }
    }
}

// .text 870C
void action_c::enterAfterTalk(dAcNpcOut_c *npc) {
    u16 angleValue = 0;
    dActor_c *actor = npc->mNmlTalk.mpActActor;
    if (actor) {
        angleValue = npc->getAngleYTo(actor).mAngle;
    }
    mAng angle(angleValue);
    mAng step;
    step.mAngle = (u16)l_turnSpeed.mAngle;
    npc->mTalk.requestTurnTalk(angle, step, 0);
    npc->mBusy = 0;
    npc->offDaubToolChange();
    npc->offDaubWatchFireworks();
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
}

// .text 87E0
void action_c::stepAfterTalk00(dAcNpcOut_c *npc) {
    if (!dNet::isSceneOwner()) {
        npc->syncTool();
    }
    if (!npc->mTalk.isBusy()) {
        npc->addTalkWeight(-3);
        npc->setDemoState2();
        npc->mMood.mTimerPending = 1;
        dDemo::finishTalk(npc);
        ++mStep;
    }
}

const action_c::procFunc sAfterTalkSteps[] = {&action_c::stepAfterTalk00};

// .text 8868
void action_c::executeAfterTalk(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sAfterTalkSteps)) {
        (this->*sAfterTalkSteps[mStep])(npc);
    }
}

// .text 88A8
void action_c::enterFacePlayer(dAcNpcOut_c *npc) {
    mAng angle = npc->getAngleYToPlayer(npc->_1B70);
    npc->mAction.requestTurn(PRIORITY_REACTION, angle, l_turnSpeed, 0);
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
}

// .text 8958
void action_c::stepFacePlayer00(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_TURN && action->mCanChange) {
        action->requestWait(PRIORITY_REACTION);
        ++mStep;
    }
}

const action_c::procFunc sFacePlayerSteps[] = {&action_c::stepFacePlayer00};

// .text 89BC
void action_c::executeFacePlayer(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sFacePlayerSteps)) {
        (this->*sFacePlayerSteps[mStep])(npc);
    }
}

// .text 89FC
void action_c::enterHideSeekInvite(dAcNpcOut_c *npc) {
    mAng angle = npc->getAngleYToPlayer(4);
    npc->mAngle.y = angle;
    npc->mAngle3D.y = angle;
    npc->mAction.requestWait(PRIORITY_IDLE);
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 0;
    npc->mMood.startTimer(0, 0);
    npc->mMood.mTimerPending = 1;
    npc->mFlea.clear();
    npc->mApproachActive = 0;
}

// .text 8AD4
void action_c::stepHideSeekInvite00(dAcNpcOut_c *npc) {
    if (dDemo::requestSpeak(npc, NULL)) {
        request(ACTION_FACE_PLAYER, ACTION_AFTER_TALK,
                dAcNpcNml_c::talk_c::getProcSet(
                    dAcNpcNml_c::talk_c::TALK_PROC_HIDE_EXPLAIN));
    }
}

const action_c::procFunc sHideSeekInviteSteps[] = {
    &action_c::stepHideSeekInvite00};

// .text 8B2C
void action_c::executeHideSeekInvite(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sHideSeekInviteSteps)) {
        (this->*sHideSeekInviteSteps[mStep])(npc);
    }
}

// .text 8B6C
void action_c::enterHideSeekStandby(dAcNpcOut_c *npc) {
    mAng angle = npc->getAngleYToPlayer(4);
    npc->mAngle.y = angle;
    npc->mAngle3D.y = angle;
    npc->mAction.requestWait(PRIORITY_IDLE);
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 0;
    npc->mMood.startTimer(0, 0);
    npc->mMood.mTimerPending = 1;
    npc->mFlea.clear();
    npc->mApproachActive = 0;
}

// .text 8C44
void action_c::executeHideSeekStandby(dAcNpcOut_c *npc) {
}

// .text 8C48
int action_c::calcHideWait(dAnimal_c *animal) {
    static const u8 base[] = {2, 2, 2, 2, 2, 2};
    static const u8 variation[] = {5, 5, 5, 5, 5, 5};
    int time = 180;
    if (animal) {
        dAnmPersonalID_c *id = &animal->mID;
        if (id->isValid()) {
            u8 looks = id->getLooks(1);
            if (looks < LOOKS_TYPE_NUM) {
                int baseTime = base[looks] * 60;
                int variationTime = variation[looks] * 60;
                time += baseTime + cM::rndInt(variationTime);
            }
        }
    }
    return time;
}

// .text 8CE4
void action_c::enterHideSeekWait(dAcNpcOut_c *npc) {
    if (npc->mAngle.y.mAngle != 0) {
        mAng angle = 0;
        npc->mAction.requestTurn(PRIORITY_IDLE, angle, l_turnSpeed, 0);
        mStep = 0;
    } else {
        npc->mAngle3D.y = 0;
        npc->mAction.requestWait(PRIORITY_IDLE);
        mTimer0 = calcHideWait(npc->mpAnimal);
        mStep = 1;
    }
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->offDaubInHouse();
    npc->mCheckGround = 1;
    mSavedAction = ACTION_HIDE_SEEK_PLAY;
    request(ACTION_HIDE_SEEK_FACE_PLAYER, ACTION_HIDE_SEEK_AFTER_TALK,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_HIDE_HIDER));
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_FIXED);
    npc->mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 0;
    npc->mMood.startTimer(0, 0);
    npc->mMood.mTimerPending = 1;
    npc->mFlea.clear();
    npc->mApproachActive = 0;
}

// .text 8E6C
void action_c::stepHideSeekWait00(dAcNpcOut_c *npc) {
    if (npc->mAction.mActionId == dAcNpc_c::ACTION_TURN &&
        npc->mAction.mCanChange) {
        mTimer0 = calcHideWait(npc->mpAnimal);
        mStep = 1;
    }
}

// .text 8EC0
void action_c::stepHideSeekWait01(dAcNpcOut_c *npc) {
    dQuestPlayerAnimal_c *quest =
        &dSaveData_c::getRaw()->mAnimals.mTown.mHideAndSeek;
    const dTime_c *now = dTime_c::getCurrent();
    if (!quest->mBase.isActive() || quest->isTimeUp(*now, 0)) {
        mSavedAction = ACTION_WANDER;
        restoreAction(npc);
    } else if (npc->mViewArea.isOutsideAll(4, npc)) {
        mSavedAction = ACTION_HIDE_SEEK_WAIT;
        setAction(npc, ACTION_OFFSCREEN);
    } else {
        if (!mTimer0) {
            setAction(npc, ACTION_HIDE_SEEK_PEEK);
        }
    }
}

const action_c::procFunc sHideSeekWaitSteps[] = {&action_c::stepHideSeekWait00,
                                                 &action_c::stepHideSeekWait01};

// .text 8FA8
void action_c::executeHideSeekWait(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sHideSeekWaitSteps)) {
        (this->*sHideSeekWaitSteps[mStep])(npc);
    }
}

// .text 8FE8
void action_c::enterHideSeekPeek(dAcNpcOut_c *npc) {
    int anm = npc->selectFreeSide();
    npc->mAction.requestAnm(PRIORITY_IDLE, anm, 0.0f, cNpcMorphFrames,
                            m3d::PLAYMODE_INHERIT, 0);
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->offDaubInHouse();
    npc->mCheckGround = 1;
    mSavedAction = ACTION_HIDE_SEEK_PLAY;
    request(ACTION_HIDE_SEEK_FACE_PLAYER, ACTION_HIDE_SEEK_AFTER_TALK,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_HIDE_HIDER));
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_FIXED);
    npc->mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 0;
    npc->mMood.startTimer(0, 0);
    npc->mMood.mTimerPending = 1;
    npc->mFlea.clear();
    npc->mApproachActive = 0;
}

// .text 9128
void action_c::executeHideSeekPeek(dAcNpcOut_c *npc) {
    if (npc->mViewArea.isOutsideAll(4, npc)) {
        mSavedAction = ACTION_HIDE_SEEK_WAIT;
        setAction(npc, ACTION_OFFSCREEN);
    } else {
        if (npc->mAction.mCanChange) {
            setAction(npc, ACTION_HIDE_SEEK_WAIT);
        }
    }
}

// .text 91AC
void action_c::enterHideSeekPlay(dAcNpcOut_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE);
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->offDaubInHouse();
    npc->mCheckGround = 1;
    mSavedAction = ACTION_HIDE_SEEK_PLAY;
    if (dNet::isHost()) {
        request(ACTION_HIDE_SEEK_FACE_PLAYER, ACTION_HIDE_SEEK_AFTER_TALK,
                dAcNpcNml_c::talk_c::getProcSet(
                    dAcNpcNml_c::talk_c::TALK_PROC_HIDE_HIDER));
    } else {
        request(ACTION_FACE_PLAYER, ACTION_AFTER_TALK,
                dAcNpcNml_c::talk_c::getProcSet(
                    dAcNpcNml_c::talk_c::TALK_PROC_ENTRY));
    }
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_NORMAL);
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 0;
    npc->mMood.startTimer(0, 0);
    npc->mMood.mTimerPending = 1;
    npc->mFlea.clear();
    npc->mApproachActive = 0;
}

// .text 9308
void action_c::executeHideSeekPlay(dAcNpcOut_c *npc) {
    dQuestPlayerAnimal_c *quest =
        &dSaveData_c::getRaw()->mAnimals.mTown.mHideAndSeek;
    const dTime_c *now = dTime_c::getCurrent();
    if (!quest->mBase.isActive() || quest->isTimeUp(*now, 0)) {
        mSavedAction = ACTION_WANDER;
        restoreAction(npc);
    } else if (npc->mViewArea.isOutsideAll(4, npc)) {
        setAction(npc, ACTION_OFFSCREEN);
    }
}

// .text 93C8
void action_c::enterHideSeekWin(dAcNpcOut_c *npc) {
    mAng angle = npc->getAngleYToPlayer(4);
    npc->mAngle.y = angle;
    npc->mAngle3D.y = angle;
    npc->mAction.requestWait(PRIORITY_IDLE);
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->offDaubInHouse();
    npc->mCheckGround = 1;
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 0;
    npc->mMood.startTimer(0, 0);
    npc->mMood.mTimerPending = 1;
    npc->mFlea.clear();
    npc->mApproachActive = 0;
}

// .text 94DC
void action_c::executeHideSeekWin(dAcNpcOut_c *npc) {
    if (dDemo::requestSpeak(npc, NULL)) {
        request(ACTION_HIDE_SEEK_FACE_PLAYER, ACTION_HIDE_SEEK_AFTER_TALK,
                dAcNpcNml_c::talk_c::getProcSet(
                    dAcNpcNml_c::talk_c::TALK_PROC_HIDE_REWARD));
    }
}

// .text 9534
void action_c::enterHideSeekLose(dAcNpcOut_c *npc) {
    mAng angle = npc->getAngleYToPlayer(4);
    npc->mAngle.y = angle;
    npc->mAngle3D.y = angle;
    npc->mAction.requestWait(PRIORITY_IDLE);
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->offDaubInHouse();
    npc->mCheckGround = 1;
    npc->mBusy = 0;
    npc->mMood.startTimer(0, 0);
    npc->mMood.mTimerPending = 1;
    npc->mFlea.clear();
    npc->mApproachActive = 0;
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
}

// .text 964C
void action_c::executeHideSeekLose(dAcNpcOut_c *npc) {
    if (dDemo::requestSpeak(npc, NULL)) {
        request(ACTION_HIDE_SEEK_FACE_PLAYER, ACTION_HIDE_SEEK_AFTER_TALK,
                dAcNpcNml_c::talk_c::getProcSet(
                    dAcNpcNml_c::talk_c::TALK_PROC_HIDE_LOSE));
    }
}

// .text 96A4
void action_c::enterHideSeekFinish(dAcNpcOut_c *npc) {
    mAng angle = npc->getAngleYToPlayer(4);
    npc->mAngle.y = angle;
    npc->mAngle3D.y = angle;
    npc->mAction.requestWait(PRIORITY_IDLE);
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->offDaubInHouse();
    npc->mCheckGround = 1;
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 0;
    npc->mMood.startTimer(0, 0);
    npc->mMood.mTimerPending = 1;
    npc->mFlea.clear();
    npc->mApproachActive = 0;
}

// .text 97B8
void action_c::executeHideSeekFinish(dAcNpcOut_c *npc) {
    dQuestPlayerAnimal_c *quest =
        &dSaveData_c::getRaw()->mAnimals.mTown.mHideAndSeek;
    if (!quest->mBase.isActive() || quest->mBase.mState == 4) {
        mSavedAction = ACTION_WANDER;
        restoreAction(npc);
        startCooldown();
    }
}

// .text 9834
void action_c::enterRemoteHideWait(dAcNpcOut_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE);
    mTimer0 = calcHideWait(npc->mpAnimal);
    npc->mAcceptMoveReq = 1;
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    request(ACTION_HIDE_SEEK_FACE_PLAYER, ACTION_HIDE_SEEK_AFTER_TALK,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_HIDE_HIDER));
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_FIXED);
    npc->mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 0;
    npc->mMood.startTimer(0, 0);
    npc->mMood.mTimerPending = 1;
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->mFlea.clear();
    npc->mApproachActive = 0;
}

// .text 997C
void action_c::executeRemoteHideWait(dAcNpcOut_c *npc) {
    int player = 4;
    if (npc->isLocalOwner()) {
        if (npc->isPermitSelf()) {
            npc->fn_80013474();
            startTalk(npc);
        } else {
            if (npc->checkLocalOwner()) {
                npc->fn_800134A4();
                saveDefaultAction(npc, FALSE);
                restoreAction(npc);
            }
        }
    } else {
        if ((int)getHideSeekReturnAction(npc) == ACTION_COUNT) {
            saveHouseAction(npc);
            restoreAction(npc);
        } else if (!npc->isInUse(&player) && player != dNet::getMyNo() &&
                   !mTimer0 && npc->mAngle.y.mAngle == 0) {
            setAction(npc, ACTION_REMOTE_HIDE_PEEK);
        } else if (npc->isDaubInHouse()) {
            setAction(npc, ACTION_REMOTE_HOUSE_WAIT);
        }
    }
}

// .text 9AC8
void action_c::enterRemoteHidePeek(dAcNpcOut_c *npc) {
    npc->mAcceptMoveReq = 0;
    int anm = npc->selectFreeSide();
    npc->mAction.requestAnm(PRIORITY_IDLE, anm, 0.0f, cNpcMorphFrames,
                            m3d::PLAYMODE_INHERIT, 0);
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    request(ACTION_HIDE_SEEK_FACE_PLAYER, ACTION_HIDE_SEEK_AFTER_TALK,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_HIDE_HIDER));
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_FIXED);
    npc->mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 0;
    npc->mMood.startTimer(0, 0);
    npc->mMood.mTimerPending = 1;
    npc->mFlea.clear();
    npc->mApproachActive = 0;
}

// .text 9BF8
void action_c::executeRemoteHidePeek(dAcNpcOut_c *npc) {
    if (npc->isLocalOwner()) {
        if (npc->isPermitSelf()) {
            npc->fn_80013474();
            startTalk(npc);
        } else {
            if (npc->checkLocalOwner()) {
                npc->fn_800134A4();
                saveDefaultAction(npc, FALSE);
                restoreAction(npc);
            }
        }
    } else {
        if (npc->mAction.mCanChange) {
            setAction(npc, ACTION_REMOTE_HIDE_WAIT);
        } else {
            if (npc->isDaubInHouse()) {
                setAction(npc, ACTION_REMOTE_HOUSE_WAIT);
            }
        }
    }
}

// .text 9CDC
void action_c::enterHideSeekFacePlayer(dAcNpcOut_c *npc) {
    if (npc->mAction.mActionId == dAcNpc_c::ACTION_ANM &&
        !npc->mAction.mCanChange) {
        mStep = 0;
    } else {
        mAng angle = npc->getAngleYToPlayer(4);
        npc->mAction.requestTurn(PRIORITY_REACTION, angle, l_turnSpeed, 0);
        mStep = 1;
    }
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 0;
    npc->mMood.startTimer(0, 0);
    npc->mMood.mTimerPending = 1;
    npc->mFlea.clear();
    npc->mApproachActive = 0;
}

// .text 9DF0
void action_c::stepHideSeekFacePlayer00(dAcNpcOut_c *npc) {
    if (npc->mAction.mCanChange) {
        mAng angle = npc->getAngleYToPlayer(4);
        npc->mAction.requestTurn(PRIORITY_REACTION, angle, l_turnSpeed, 0);
        mStep = 1;
    }
}

// .text 9E70
void action_c::stepHideSeekFacePlayer01(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_TURN && action->mCanChange) {
        action->requestWait(PRIORITY_REACTION);
        ++mStep;
    }
}

const action_c::procFunc sHideSeekFacePlayerSteps[] = {
    &action_c::stepHideSeekFacePlayer00, &action_c::stepHideSeekFacePlayer01};

// .text 9ED4
void action_c::executeHideSeekFacePlayer(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sHideSeekFacePlayerSteps)) {
        (this->*sHideSeekFacePlayerSteps[mStep])(npc);
    }
}

// .text 9F14
void action_c::enterHideSeekAfterTalk(dAcNpcOut_c *npc) {
    if (npc->mAction.mActionId == dAcNpc_c::ACTION_ANM &&
        !npc->mAction.mCanChange) {
        mStep = 0;
    } else {
        u16 angleValue = 0;
        dActor_c *actor = npc->mNmlTalk.mpActActor;
        if (actor) {
            angleValue = (u16)npc->getAngleYTo(actor).mAngle;
        }
        mAng angle(angleValue);
        u16 stepValue = l_turnSpeed.mAngle;
        mAng step(stepValue);
        npc->mTalk.requestTurnTalk(angle, step, 0);
        mStep = 1;
    }
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mMood.startTimer(0, 0);
    npc->mMood.mTimerPending = 1;
    npc->mFlea.clear();
    npc->mApproachActive = 0;
}

// .text A02C
void action_c::stepHideSeekAfterTalk00(dAcNpcOut_c *npc) {
    if (npc->mAction.mCanChange) {
        u16 angleValue = 0;
        dActor_c *actor = npc->mNmlTalk.mpActActor;
        if (actor) {
            angleValue = (u16)npc->getAngleYTo(actor).mAngle;
        }
        mAng angle(angleValue);
        u16 stepValue = l_turnSpeed.mAngle;
        mAng step(stepValue);
        npc->mTalk.requestTurnTalk(angle, step, 0);
        mStep = 1;
    }
}

// .text A0B4
void action_c::stepHideSeekAfterTalk01(dAcNpcOut_c *npc) {
    if (!npc->mTalk.isBusy()) {
        dQuestPlayerAnimal_c *quest =
            &dSaveData_c::getTown()->mAnimals.mTown.mHideAndSeek;
        dQuestBase_c *base = &quest->mBase;
        if (base->isActive() && base->mState == 3) {
            if (quest->mItem.mId != dItem::ITEM_ID_NONE) {
                quest->mBase.mState = 4;
                dNpc::sendHideAndSeekState(4);
            } else {
                quest->clearInfo();
                dNpc::sendHideAndSeekClear();
            }
        }
        mSavedAction = ACTION_WANDER;
        startCooldown();
        dDemo::finishTalk(npc);
        ++mStep;
    }
}

const action_c::procFunc sHideSeekAfterTalkSteps[] = {
    &action_c::stepHideSeekAfterTalk00, &action_c::stepHideSeekAfterTalk01};

// .text A184
void action_c::executeHideSeekAfterTalk(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sHideSeekAfterTalkSteps)) {
        (this->*sHideSeekAfterTalkSteps[mStep])(npc);
    }
}

// .text A1C4
void action_c::enterRemoteClothWait(dAcNpcOut_c *npc) {
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->mBusy = 0;
    npc->mFlea.clear();
    npc->mApproachActive = 0;
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
}

// .text A284
void action_c::executeRemoteClothWait(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_CHANGE_CLOTH2) {
        if (action->mCanChange) {
            npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                                   cNpcLookRange);
            action->requestWait(PRIORITY_IDLE);
        }
    } else if (npc->isClothChangeReady()) {
        if (npc->isDaubClothChange()) {
            action->requestChangeCloth2(PRIORITY_IDLE, npc->mpAnimal->mCloth,
                                        (int)&npc->mpAnimal->mClothDesign);
            npc->mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
        } else if (npc->isDaubClothChanged()) {
            npc->changeClothModel();
        }
    } else if (npc->isLocalOwnerHost() && dNet::isSceneOwner()) {
        npc->fn_800134A4();
        saveDefaultAction(npc, FALSE);
        restoreAction(npc);
    }
}

// .text A3D0
void action_c::initToolChange(dAcNpcOut_c *npc, u8 putTool,
                              u8 closeUmbrellaStep, u8 takeTool,
                              u8 openUmbrellaStep) {
    dAcNpc_c::toolBase_c *tool = &npc->mTool;
    dItem::Item *item = &npc->mTool.mItem;
    int anm = ANM_TOOL_CHANGE;
    if (item->mId != dItem::ITEM_ID_NONE) {
        int type = dHmnToolBank_c::getItemToolType(item);
        f32 morph = 18.0f;
        int mode = m3d::FORWARD_ONCE;
        switch (type) {
        case HMN_TOOL_UMBRELLA:
            mStep = closeUmbrellaStep;
            anm = ANM_UMBRELLA;
            mode = m3d::REVERSE_ONCE;
            morph = cNpcMorphFrames;
            break;
        default:
            mStep = putTool;
            break;
        }
        npc->mAction.requestAnm(PRIORITY_IDLE, anm, 0.0f, morph, mode, 0);
    } else if (tool->isNextReady()) {
        int type = dHmnToolBank_c::getItemToolType(&tool->mNextItem);
        int mode = m3d::REVERSE_ONCE;
        switch (type) {
        case HMN_TOOL_UMBRELLA:
            mStep = openUmbrellaStep;
            anm = ANM_UMBRELLA;
            mode = m3d::FORWARD_ONCE;
            break;
        default:
            mStep = takeTool;
            break;
        }
        npc->mAction.requestAnm(PRIORITY_IDLE, anm, 0.0f, cNpcMorphFrames, mode,
                                0);
        tool->mLock = true;
    }
    npc->mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 1;
    npc->mApproachActive = 0;
}

// .text A544
void action_c::enterToolChange(dAcNpcOut_c *npc) {
    initToolChange(npc, TOOL_PUT_AWAY, TOOL_CLOSE_UMBRELLA, TOOL_TAKE_OUT,
                   TOOL_OPEN_UMBRELLA);
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_NORMAL);
    npc->onDaubToolChange();
    npc->offDaubInHouse();
    npc->offDaubWatchFireworks();
}

// .text A5A4
BOOL action_c::takeOutTool(dAcNpcOut_c *npc) {
    dAcNpc_c::toolBase_c *tool;
    dAcNpc_c::model_c *model = npc->getModel();
    dAcNpc_c::anm_c *anm = &model->mAnm;
    if (anm->mAnmId == ANM_TOOL_CHANGE && anm->checkFrame(18.0f)) {
        tool = &npc->mTool;
        tool->change(0.0f, npc);
        tool->mLock = false;
        npc->mAction.requestWait(PRIORITY_IDLE, 18.0f);
        int anm = tool->getAnmId();
        if (anm != HMN_ANM_NUM) {
            model->setRarmAnm(anm, m3d::PLAYMODE_INHERIT, 0.0f, 1.0f, 18.0f);
        }
        return TRUE;
    }
    return FALSE;
}

// .text A68C
void action_c::stepToolChange00(dAcNpcOut_c *npc) {
    if (takeOutTool(npc)) {
        mStep = TOOL_SCALE_OUT;
    }
}

// .text A6C8
bool action_c::scaleOutTool(dAcNpcOut_c *npc) {
    dAcNpc_c::model_c *model = npc->getModel();
    dAcNpc_c::anm_c *anm = &model->mAnm;
    f32 frame = anm->getFrame();
    f32 scale;
    if (frame >= 18.0f) {
        scale = 1.0f;
    } else {
        scale = frame / 18.0f;
    }
    npc->mTool.mScale = scale;
    if (anm->checkFrame(18.0f)) {
        return true;
    }
    return false;
}

// .text A764
void action_c::stepToolChange01(dAcNpcOut_c *npc) {
    if (scaleOutTool(npc)) {
        npc->offDaubToolChange();
        npc->onDaubToolChanged();
        saveDefaultAction(npc, FALSE);
        restoreAction(npc);
    }
}

// .text A7D0
BOOL action_c::openUmbrella(dAcNpcOut_c *npc) {
    dAcNpc_c::model_c *model = npc->getModel();
    dAcNpc_c::anm_c *anm = &model->mAnm;
    if (anm->mAnmId == ANM_UMBRELLA) {
        dAcNpc_c::toolBase_c *tool = &npc->mTool;
        if (dHmnToolBank_c::getItemToolType(&tool->mItem) !=
            HMN_TOOL_UMBRELLA) {
            tool->change(0.0f, npc);
            tool->mLock = false;
            tool->mBank.openUmbrella();
        }
        if (anm->checkFrame(22.0f)) {
            tool->mScale = 1.0f;
            return TRUE;
        }
    }
    return FALSE;
}

// .text A898
void action_c::stepToolChange02(dAcNpcOut_c *npc) {
    if (openUmbrella(npc)) {
        mStep = TOOL_FINISH_OPEN_UMBRELLA;
    }
}

// .text A8D4
BOOL action_c::finishOpeningUmbrella(dAcNpcOut_c *npc) {
    if (npc->mAction.mCanChange) {
        int anm = dHmnAnm_c::getToolAnmId(HMN_TOOL_UMBRELLA);
        if (anm != HMN_ANM_NUM) {
            npc->getModel()->setRarmAnm(anm, m3d::PLAYMODE_INHERIT, 0.0f, 1.0f,
                                        1.0f);
        }
        return TRUE;
    }
    return FALSE;
}

// .text A964
void action_c::stepToolChange03(dAcNpcOut_c *npc) {
    if (finishOpeningUmbrella(npc)) {
        npc->offDaubToolChange();
        npc->onDaubToolChanged();
        restoreAction(npc);
    }
}

// .text A9C0
BOOL action_c::putAwayTool(dAcNpcOut_c *npc) {
    dAcNpc_c::model_c *model = npc->getModel();
    dAcNpc_c::anm_c *anm = &model->mAnm;
    if (anm->mAnmId == ANM_TOOL_CHANGE) {
        dAcNpc_c::toolBase_c *tool = &npc->mTool;
        if (model->mRarmAnm.mAnmId != HMN_ANM_NUM) {
            model->stopRarmAnm(cNpcMorphFrames);
        }
        f32 frame = anm->getFrame();
        f32 scale;
        if (frame >= 18.0f) {
            scale = 0.0f;
        } else {
            scale = (18.0f - frame) / 18.0f;
        }
        tool->mScale = scale;
        if (anm->checkFrame(18.0f)) {
            tool->putAway();
            return TRUE;
        }
    }
    return FALSE;
}

// .text AA90
void action_c::stepToolChange04(dAcNpcOut_c *npc) {
    if (putAwayTool(npc)) {
        mStep = TOOL_TAKE_NEXT;
    }
}

// .text AACC
BOOL action_c::takeNextTool(dAcNpcOut_c *npc, u8 toolStep, u8 umbrellaStep) {
    dAcNpc_c::toolBase_c *tool;
    const dItem::Item *next;
    dAcNpc_c::model_c *model = npc->getModel();
    dAcNpc_c::anm_c *anm = &model->mAnm;
    tool = &npc->mTool;
    next = &tool->mNextItem;
    if (anm->mAnmId == ANM_TOOL_CHANGE && anm->checkFrame(42.0f) &&
        next->mId != dItem::ITEM_ID_NONE &&
        dHmnToolBank_c::getItemToolType(next) != HMN_TOOL_UMBRELLA &&
        tool->isNextReady()) {
        tool->change(0.0f, npc);
        npc->mAction.requestWait(PRIORITY_IDLE, 18.0f);
        int anm = tool->getAnmId();
        if (anm != HMN_ANM_NUM) {
            model->setRarmAnm(anm, m3d::PLAYMODE_INHERIT, 0.0f, 1.0f, 18.0f);
        }
        mStep = toolStep;
    }
    if (npc->mAction.mCanChange) {
        if (next->mId != dItem::ITEM_ID_NONE &&
            dHmnToolBank_c::getItemToolType(next) == HMN_TOOL_UMBRELLA &&
            tool->isNextReady()) {
            npc->mAction.requestAnm(PRIORITY_IDLE, ANM_UMBRELLA, 0.0f,
                                    cNpcMorphFrames, m3d::FORWARD_ONCE, 0);
            tool->mLock = true;
            mStep = umbrellaStep;
        } else {
            return TRUE;
        }
    }
    return FALSE;
}

// .text AC48
void action_c::stepToolChange05(dAcNpcOut_c *npc) {
    if (takeNextTool(npc, TOOL_SCALE_OUT, TOOL_OPEN_UMBRELLA)) {
        npc->offDaubToolChange();
        npc->onDaubToolChanged();
        saveDefaultAction(npc, FALSE);
        restoreAction(npc);
    }
}

// .text ACBC
BOOL action_c::closeUmbrella(dAcNpcOut_c *npc) {
    dAcNpc_c::model_c *model = npc->getModel();
    dAcNpc_c::anm_c *anm = &model->mAnm;
    if (model->mAnm.mAnmId == ANM_UMBRELLA) {
        if (model->mRarmAnm.mAnmId != HMN_ANM_NUM) {
            model->stopRarmAnm(cNpcMorphFrames);
            npc->mTool.mBank.closeUmbrella();
        }
        if (anm->checkFrame(22.0f)) {
            npc->mTool.putAway();
            return TRUE;
        }
    }
    return FALSE;
}

// .text AD5C
void action_c::stepToolChange06(dAcNpcOut_c *npc) {
    if (closeUmbrella(npc)) {
        mStep = TOOL_FINISH_CHANGE;
    }
}

// .text AD98
BOOL action_c::finishToolChange(dAcNpcOut_c *npc, u8 putTool,
                                u8 closeUmbrellaStep, u8 takeTool,
                                u8 openUmbrellaStep) {
    if (npc->mAction.mCanChange) {
        dAcNpc_c::toolBase_c *tool = &npc->mTool;
        if (tool->mNextItem.mId != dItem::ITEM_ID_NONE && tool->isNextReady()) {
            initToolChange(npc, putTool, closeUmbrellaStep, takeTool,
                           openUmbrellaStep);
        } else {
            return TRUE;
        }
    }
    return FALSE;
}

// .text AE30
void action_c::stepToolChange07(dAcNpcOut_c *npc) {
    if (finishToolChange(npc, TOOL_PUT_AWAY, TOOL_CLOSE_UMBRELLA, TOOL_TAKE_OUT,
                         TOOL_OPEN_UMBRELLA)) {
        npc->offDaubToolChange();
        npc->onDaubToolChanged();
        saveDefaultAction(npc, FALSE);
        restoreAction(npc);
    }
}

const action_c::procFunc sToolChangeSteps[] = {
    &action_c::stepToolChange00, &action_c::stepToolChange01,
    &action_c::stepToolChange02, &action_c::stepToolChange03,
    &action_c::stepToolChange04, &action_c::stepToolChange05,
    &action_c::stepToolChange06, &action_c::stepToolChange07};

// .text AEAC
void action_c::executeToolChange(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sToolChangeSteps)) {
        (this->*sToolChangeSteps[mStep])(npc);
    }
}

// .text AEEC
void action_c::enterRemoteToolChange(dAcNpcOut_c *npc) {
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->mAcceptMoveReq = 0;
    initToolChange(npc, TOOL_PUT_AWAY, TOOL_CLOSE_UMBRELLA, TOOL_TAKE_OUT,
                   TOOL_OPEN_UMBRELLA);
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_NORMAL);
    npc->mFlea.clear();
}

// .text AF74
void action_c::stepRemoteToolChange00(dAcNpcOut_c *npc) {
    BOOL done = takeOutTool(npc);
    bool local = false;
    if (npc->isLocalOwner() && npc->checkLocalOwner()) {
        npc->fn_800134A4();
        local = true;
    }
    if (done) {
        if (local) {
            mStep = TOOL_LOCAL_SCALE_OUT;
        } else {
            mStep = TOOL_SCALE_OUT;
        }
    } else if (local) {
        mStep = TOOL_LOCAL_TAKE_OUT;
    }
}

// .text B028
void action_c::stepRemoteToolChange01(dAcNpcOut_c *npc) {
    BOOL done = scaleOutTool(npc);
    bool local = false;
    if (npc->isLocalOwner() && npc->checkLocalOwner()) {
        npc->fn_800134A4();
        local = true;
    }
    if (done) {
        if (local) {
            npc->offDaubToolChange();
            npc->onDaubToolChanged();
            saveDefaultAction(npc, FALSE);
            restoreAction(npc);
        } else {
            setAction(npc, ACTION_REMOTE_IDLE);
        }
    } else if (local) {
        mStep = TOOL_LOCAL_SCALE_OUT;
    }
}

// .text B108
void action_c::stepRemoteToolChange02(dAcNpcOut_c *npc) {
    BOOL done = openUmbrella(npc);
    bool local = false;
    if (npc->isLocalOwner() && npc->checkLocalOwner()) {
        npc->fn_800134A4();
        local = true;
    }
    if (done) {
        if (local) {
            mStep = TOOL_LOCAL_FINISH_OPEN_UMBRELLA;
        } else {
            mStep = TOOL_FINISH_OPEN_UMBRELLA;
        }
    } else if (local) {
        mStep = TOOL_LOCAL_OPEN_UMBRELLA;
    }
}

// .text B1BC
void action_c::stepRemoteToolChange03(dAcNpcOut_c *npc) {
    BOOL done = finishOpeningUmbrella(npc);
    bool local = false;
    if (npc->isLocalOwner() && npc->checkLocalOwner()) {
        npc->fn_800134A4();
        local = true;
    }
    if (done) {
        if (local) {
            npc->offDaubToolChange();
            npc->onDaubToolChanged();
            saveDefaultAction(npc, FALSE);
            restoreAction(npc);
        } else {
            setAction(npc, ACTION_REMOTE_IDLE);
        }
    } else if (local) {
        mStep = TOOL_LOCAL_FINISH_OPEN_UMBRELLA;
    }
}

// .text B29C
void action_c::stepRemoteToolChange04(dAcNpcOut_c *npc) {
    BOOL done = putAwayTool(npc);
    bool local = false;
    if (npc->isLocalOwner() && npc->checkLocalOwner()) {
        npc->fn_800134A4();
        local = true;
    }
    if (done) {
        if (local) {
            mStep = TOOL_LOCAL_TAKE_NEXT;
        } else {
            mStep = TOOL_TAKE_NEXT;
        }
    } else if (local) {
        mStep = TOOL_LOCAL_PUT_AWAY;
    }
}

// .text B350
void action_c::stepRemoteToolChange05(dAcNpcOut_c *npc) {
    BOOL done = takeNextTool(npc, TOOL_SCALE_OUT, TOOL_OPEN_UMBRELLA);
    bool local = false;
    if (npc->isLocalOwner() && npc->checkLocalOwner()) {
        npc->fn_800134A4();
        local = true;
    }
    if (done) {
        if (local) {
            npc->offDaubToolChange();
            npc->onDaubToolChanged();
            saveDefaultAction(npc, FALSE);
            restoreAction(npc);
        } else {
            setAction(npc, ACTION_REMOTE_IDLE);
        }
    } else {
        if (local) {
            switch ((int)mStep) {
            case TOOL_SCALE_OUT:
                mStep = TOOL_LOCAL_SCALE_OUT;
                break;
            case TOOL_OPEN_UMBRELLA:
                mStep = TOOL_LOCAL_OPEN_UMBRELLA;
                break;
            default:
                mStep = TOOL_LOCAL_TAKE_NEXT;
                break;
            }
        }
    }
}

// .text B46C
void action_c::stepRemoteToolChange06(dAcNpcOut_c *npc) {
    BOOL done = closeUmbrella(npc);
    bool local = false;
    if (npc->isLocalOwner() && npc->checkLocalOwner()) {
        npc->fn_800134A4();
        local = true;
    }
    if (done) {
        if (local) {
            mStep = TOOL_LOCAL_FINISH_CHANGE;
        } else {
            mStep = TOOL_FINISH_CHANGE;
        }
    } else if (local) {
        mStep = TOOL_LOCAL_CLOSE_UMBRELLA;
    }
}

// .text B520
void action_c::stepRemoteToolChange07(dAcNpcOut_c *npc) {
    BOOL done = finishToolChange(npc, TOOL_PUT_AWAY, TOOL_CLOSE_UMBRELLA,
                                 TOOL_TAKE_OUT, TOOL_OPEN_UMBRELLA);
    bool local = false;
    if (npc->isLocalOwner() && npc->checkLocalOwner()) {
        npc->fn_800134A4();
        local = true;
    }
    if (done) {
        if (local) {
            npc->offDaubToolChange();
            npc->onDaubToolChanged();
            saveDefaultAction(npc, FALSE);
            restoreAction(npc);
        } else {
            setAction(npc, ACTION_REMOTE_IDLE);
        }
    } else {
        if (local) {
            switch (mStep) {
            case TOOL_PUT_AWAY:
                mStep = TOOL_LOCAL_PUT_AWAY;
                break;
            case TOOL_CLOSE_UMBRELLA:
                mStep = TOOL_LOCAL_CLOSE_UMBRELLA;
                break;
            case TOOL_TAKE_OUT:
                mStep = TOOL_LOCAL_TAKE_OUT;
                break;
            case TOOL_OPEN_UMBRELLA:
                mStep = TOOL_LOCAL_OPEN_UMBRELLA;
                break;
            default:
                mStep = TOOL_LOCAL_CLOSE_UMBRELLA;
                break;
            }
        }
    }
}

// .text B664
void action_c::stepRemoteToolChange08(dAcNpcOut_c *npc) {
    if (takeOutTool(npc)) {
        mStep = TOOL_LOCAL_SCALE_OUT;
    }
}

// .text B6A0
void action_c::stepRemoteToolChange09(dAcNpcOut_c *npc) {
    if (scaleOutTool(npc)) {
        npc->offDaubToolChange();
        npc->onDaubToolChanged();
        saveDefaultAction(npc, FALSE);
        restoreAction(npc);
    }
}

// .text B70C
void action_c::stepRemoteToolChange10(dAcNpcOut_c *npc) {
    if (openUmbrella(npc)) {
        mStep = TOOL_LOCAL_FINISH_OPEN_UMBRELLA;
    }
}

// .text B748
void action_c::stepRemoteToolChange11(dAcNpcOut_c *npc) {
    if (finishOpeningUmbrella(npc)) {
        npc->offDaubToolChange();
        npc->onDaubToolChanged();
        saveDefaultAction(npc, FALSE);
        restoreAction(npc);
    }
}

// .text B7B4
void action_c::stepRemoteToolChange12(dAcNpcOut_c *npc) {
    if (putAwayTool(npc)) {
        mStep = TOOL_LOCAL_TAKE_NEXT;
    }
}

// .text B7F0
void action_c::stepRemoteToolChange13(dAcNpcOut_c *npc) {
    if (takeNextTool(npc, TOOL_LOCAL_SCALE_OUT, TOOL_LOCAL_OPEN_UMBRELLA)) {
        npc->offDaubToolChange();
        npc->onDaubToolChanged();
        saveDefaultAction(npc, FALSE);
        restoreAction(npc);
    }
}

// .text B864
void action_c::stepRemoteToolChange14(dAcNpcOut_c *npc) {
    if (closeUmbrella(npc)) {
        mStep = TOOL_LOCAL_FINISH_CHANGE;
    }
}

// .text B8A0
void action_c::stepRemoteToolChange15(dAcNpcOut_c *npc) {
    if (finishToolChange(npc, TOOL_LOCAL_PUT_AWAY, TOOL_LOCAL_CLOSE_UMBRELLA,
                         TOOL_LOCAL_TAKE_OUT, TOOL_LOCAL_OPEN_UMBRELLA)) {
        npc->offDaubToolChange();
        npc->onDaubToolChanged();
        saveDefaultAction(npc, FALSE);
        restoreAction(npc);
    }
}

const action_c::procFunc sRemoteToolChangeSteps[] = {
    &action_c::stepRemoteToolChange00, &action_c::stepRemoteToolChange01,
    &action_c::stepRemoteToolChange02, &action_c::stepRemoteToolChange03,
    &action_c::stepRemoteToolChange04, &action_c::stepRemoteToolChange05,
    &action_c::stepRemoteToolChange06, &action_c::stepRemoteToolChange07,
    &action_c::stepRemoteToolChange08, &action_c::stepRemoteToolChange09,
    &action_c::stepRemoteToolChange10, &action_c::stepRemoteToolChange11,
    &action_c::stepRemoteToolChange12, &action_c::stepRemoteToolChange13,
    &action_c::stepRemoteToolChange14, &action_c::stepRemoteToolChange15};

// .text B91C
void action_c::executeRemoteToolChange(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sRemoteToolChangeSteps)) {
        (this->*sRemoteToolChangeSteps[mStep])(npc);
    }
}

// .text B95C
void action_c::initMaskChange(dAcNpcOut_c *npc) {
    npc->mAction.requestAnm(PRIORITY_IDLE, ANM_MASK_CHANGE, 0.0f, 6.0f,
                            m3d::PLAYMODE_INHERIT, 0);
    npc->mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 1;
    npc->mMood.startTimer(0, 0);
    npc->mMood.mTimerPending = 1;
    npc->mFlea.clear();
    npc->mApproachActive = 0;
    if (npc->mMode == dAcNpcOut_c::MODE_NORMAL) {
        npc->mAudioObj.startSound(SE_WearPumpkin);
    } else {
        npc->mAudioObj.startSound(SE_wear);
    }
}

// .text BA64
void action_c::enterMaskChange(dAcNpcOut_c *npc) {
    initMaskChange(npc);
    npc->onDaubMaskChange();
    npc->offDaubInHouse();
    npc->offDaubWatchFireworks();
}

// .text BAA8
BOOL action_c::changeMaskAtFrame(dAcNpcOut_c *npc) {
    dAcNpc_c::anm_c *anm = &npc->getModel()->mAnm;
    if (anm->mAnmId == ANM_MASK_CHANGE && anm->checkFrame(24.0f)) {
        npc->changeMask();
        return TRUE;
    }
    return FALSE;
}

// .text BB1C
void action_c::stepMaskChange00(dAcNpcOut_c *npc) {
    if (changeMaskAtFrame(npc)) {
        mStep = 1;
    }
}

// .text BB58
BOOL action_c::isMaskChangeDone(dAcNpcOut_c *npc) {
    dAcNpc_c::anm_c *anm = &npc->getModel()->mAnm;
    if (anm->mAnmId == ANM_MASK_CHANGE && anm->isStop()) {
        return TRUE;
    }
    return FALSE;
}

// .text BBB0
void action_c::stepMaskChange01(dAcNpcOut_c *npc) {
    if (isMaskChangeDone(npc)) {
        npc->offDaubMaskChange();
        saveDefaultAction(npc, FALSE);
        restoreAction(npc);
    }
}

const action_c::procFunc sMaskChangeSteps[] = {&action_c::stepMaskChange00,
                                               &action_c::stepMaskChange01};

// .text BC14
void action_c::executeMaskChange(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sMaskChangeSteps)) {
        (this->*sMaskChangeSteps[mStep])(npc);
    }
}

// .text BC54
void action_c::enterRemoteMaskChange(dAcNpcOut_c *npc) {
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->mAcceptMoveReq = 0;
    initMaskChange(npc);
}

// .text BC8C
void action_c::stepRemoteMaskChange00(dAcNpcOut_c *npc) {
    BOOL done = changeMaskAtFrame(npc);
    bool local = false;
    if (npc->isLocalOwner() && npc->checkLocalOwner()) {
        npc->fn_800134A4();
        local = true;
    }
    if (done) {
        if (local) {
            mStep = 3;
        } else {
            mStep = 1;
        }
    } else if (local) {
        mStep = 2;
    }
}

// .text BD40
void action_c::stepRemoteMaskChange01(dAcNpcOut_c *npc) {
    BOOL done = isMaskChangeDone(npc);
    bool local = false;
    if (npc->isLocalOwner() && npc->checkLocalOwner()) {
        npc->fn_800134A4();
        local = true;
    }
    if (done) {
        if (local) {
            npc->offDaubMaskChange();
            saveDefaultAction(npc, FALSE);
            restoreAction(npc);
        } else {
            saveHouseAction(npc);
            restoreAction(npc);
        }
    } else {
        if (local) {
            mStep = 3;
        }
    }
}

// .text BE20
void action_c::stepRemoteMaskChange02(dAcNpcOut_c *npc) {
    if (changeMaskAtFrame(npc)) {
        mStep = 3;
    }
}

// .text BE5C
void action_c::stepRemoteMaskChange03(dAcNpcOut_c *npc) {
    if (isMaskChangeDone(npc)) {
        npc->offDaubMaskChange();
        saveDefaultAction(npc, FALSE);
        restoreAction(npc);
    }
}

const action_c::procFunc sRemoteMaskChangeSteps[] = {
    &action_c::stepRemoteMaskChange00, &action_c::stepRemoteMaskChange01,
    &action_c::stepRemoteMaskChange02, &action_c::stepRemoteMaskChange03};

// .text BEC0
void action_c::executeRemoteMaskChange(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sRemoteMaskChangeSteps)) {
        (this->*sRemoteMaskChangeSteps[mStep])(npc);
    }
}

// .text BF00
void action_c::initIdleChat(dAcNpcOut_c *npc) {
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->mAction.requestWait(PRIORITY_IDLE);
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_NORMAL);
    npc->mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 0;
    npc->mMood.startTimer(0, 0);
    npc->mMood.mTimerPending = 1;
    npc->mFlea.clear();
    npc->mApproachActive = 0;
    mBlockedFrames = 0;
}

// .text BFFC
void action_c::enterHalloweenChase(dAcNpcOut_c *npc) {
    initIdleChat(npc);
    npc->offDaubInHouse();
    npc->mCheckGround = 1;
    npc->offDaubToolChange();
    npc->offDaubWatchFireworks();
    request(ACTION_FACE_PLAYER, ACTION_AFTER_TALK,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_HALLOWEEN_COSTUME));
    mChasePlayer = 4;
    float range = 96.0f;
    int player = dNet::getMyNo();
    if (npc->isPlayerNear(player, 1, NULL, range)) {
        mChatFlags[2] = 1;
    } else {
        mChatFlags[2] = 0;
    }
}

// .text C0CC
void action_c::stepHalloweenChase00(dAcNpcOut_c *npc) {
    dSaveTown_c *town = dSaveData_c::getTown();
    mVec3_c wanderPos;
    dAcNpc_c::action_c *action = &npc->mAction;
    if (town->mAnimals.mTown.updateHalloweenFlag(npc->getNpcIdx())) {
        mSavedAction = ACTION_WANDER;
        setAction(npc, ACTION_MASK_CHANGE);
        return;
    }
    if (npc->getSpeedF() != float(0.0f)) {
        if ((npc->mAcch.mPrevHitFlags & 0x88) ||
            (npc->mAcch.mHitFlags & 0x88)) {
            if (mBlockedFrames < 0xFFFFFFFF) {
                ++mBlockedFrames;
            }
        } else {
            mBlockedFrames = 0;
        }
    } else {
        mBlockedFrames = 0;
    }
    if (mChatFlags[2]) {
        float range = cHalloweenTrackDistance;
        int player = dNet::getMyNo();
        if (!npc->isPlayerNear(player, 1, NULL, range)) {
            mChatFlags[2] = 0;
        }
    }
    if (!mChatFlags[2]) {
        float range = cHalloweenAskDistance;
        int player = dNet::getMyNo();
        if (npc->isPlayerNear(player, 1, NULL, range)) {
            if (!dDemo::isType(dDemo::TYPE_ANY) && !npc->isInUse(NULL) &&
                dDemo::requestSpeak(npc, NULL)) {
                action->requestWait(PRIORITY_IDLE);
                npc->mBusy = 1;
                mChasePlayer = 4;
                mStep = 1;
                return;
            }
            if (dDemo::isType(dDemo::TYPE_SPEAK) ||
                dDemo::isType(dDemo::TYPE_TALK)) {
                mChatFlags[2] = 1;
            }
        }
    }
    if (mChasePlayer < 4) {
        if (!mTimer0) {
            mChasePlayer = 4;
        } else {
            if (!npc->isPlayerNear(mChasePlayer, 1, NULL,
                                   cHalloweenTrackDistance)) {
                mChasePlayer = 4;
            }
        }
        if ((int)mChasePlayer == dPlayerMgr::PLAYER_INDEX_CURRENT &&
            action->mActionId != dAcNpc_c::ACTION_WAIT) {
            action->requestWait(PRIORITY_IDLE);
        }
    }
    if ((int)mChasePlayer == dPlayerMgr::PLAYER_INDEX_CURRENT) {
        float range = cHalloweenAcquireDistance;
        u32 excluded = mChatFlags[2] ? dNet::getMyNo() : 4;
        u32 player = npc->searchPlayer(4, 1, excluded, 1, NULL, range);
        if (player < 4) {
            mChasePlayer = player;
            float duration = cHalloweenWaitFrames;
            mTimer0 = (u32)duration;
        }
    }
    if (mChasePlayer < 4) {
        if (npc->isPlayerNear(mChasePlayer, 1, NULL, cHalloweenTakeDistance)) {
            if (action->mActionId != dAcNpc_c::ACTION_WAIT) {
                action->requestWait(PRIORITY_IDLE);
            }
        } else {
            bool move = true;
            if (action->mActionId == dAcNpc_c::ACTION_WAIT) {
                float askDistance = cHalloweenAskDistance;
                if (npc->isPlayerNear(mChasePlayer, 1, NULL, askDistance)) {
                    move = false;
                }
            }
            if (move) {
                const mVec3_c *pos = dPlayerMgr::getPosition(mChasePlayer);
                if (pos) {
                    if (action->mActionId == dAcNpc_c::ACTION_RUN ||
                        action->mActionId == dAcNpc_c::ACTION_TURN_RUN) {
                        action->setTargetPos(*pos);
                    } else {
                        mVec3_c diff = npc->mPos - *pos;
                        float distance =
                            EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
                        mAng angle(0);
                        BOOL isFront =
                            npc->checkTargetAngle(&angle, pos, &l_frontAngle);
                        if (isFront && distance > 48.0f) {
                            action->requestRun(
                                PRIORITY_IDLE, *pos, l_runTurnSpeed, 0,
                                l_moveParamRun, l_8074FDEC, cNpcMorphFrames);
                        } else {
                            action->requestTurnRun(
                                PRIORITY_IDLE, *pos, angle, l_turnSpeed, 0,
                                l_moveParamRun, l_8074FDEC, cNpcMorphFrames);
                        }
                    }
                }
            } else if (action->mActionId != dAcNpc_c::ACTION_WAIT) {
                action->requestWait(PRIORITY_IDLE);
            }
        }
    } else if ((npc->getSpeedF() == float(0.0f) ||
                (!avoidObstacle(npc) && !stopIfPushed(npc))) &&
               npc->mAction.mCanChange &&
               (npc->mAction.mActionId != dAcNpc_c::ACTION_WAIT || !mTimer1)) {
        float z = npc->mPos.z, y = npc->mPos.y, x = npc->mPos.x;
        wanderPos.set(x, y, z);
        if (cM::rndInt(5) < 3 &&
            dAcNpc_c::searchPosNear(&wanderPos, &npc->mPos, l_8074FE14,
                                    l_8074FE10)) {
            mAng angle(0);
            BOOL isFront =
                npc->checkTargetAngle(&angle, &wanderPos, &l_frontAngle);
            if (isFront &&
                horizontalDistance(wanderPos, npc->mPos) > l_8074FE14) {
                npc->mAction.requestWalk(PRIORITY_IDLE, wanderPos,
                                         l_walkTurnSpeed, 0, l_moveParamWalk,
                                         l_defaultAnmRate, cNpcMorphFrames);
            } else {
                npc->mAction.requestTurnWalk(PRIORITY_IDLE, wanderPos, angle,
                                             l_turnSpeed, 0, l_moveParamWalk,
                                             l_defaultAnmRate, cNpcMorphFrames);
            }
        } else {
            mTimer1 = cM::rndInt(600) + 120;
            if (npc->mAction.mActionId != dAcNpc_c::ACTION_WAIT) {
                npc->mAction.requestWait(PRIORITY_IDLE);
            }
        }
    }
}

// .text C6E8
void action_c::stepHalloweenChase01(dAcNpcOut_c *npc) {
    float distance = cHalloweenTalkDistance;
    if (dDemo::isType(dDemo::TYPE_ANY) || npc->isInUse(NULL) ||
        !npc->isPlayerNear(dNet::getMyNo(), 1, NULL, distance)) {
        mStep = 0;
        npc->mBusy = 0;
        if (dDemo::isType(dDemo::TYPE_SPEAK) ||
            dDemo::isType(dDemo::TYPE_TALK)) {
            mChatFlags[2] = 1;
        }
    } else if (dDemo::requestSpeak(npc, NULL)) {
        npc->mBusy = 1;
        mSavedAction = ACTION_HALLOWEEN_CHASE;
    }
}

const action_c::procFunc sHalloweenChaseSteps[] = {
    &action_c::stepHalloweenChase00, &action_c::stepHalloweenChase01};

// .text C7D8
void action_c::executeHalloweenChase(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sHalloweenChaseSteps)) {
        (this->*sHalloweenChaseSteps[mStep])(npc);
    }
}

// .text C818
void action_c::enterRemoteHalloweenWait(dAcNpcOut_c *npc) {
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    initIdleChat(npc);
    npc->mAcceptMoveReq = 1;
    request(ACTION_FACE_PLAYER, ACTION_AFTER_TALK,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_HALLOWEEN_COSTUME));
    float range = 96.0f;
    int player = dNet::getMyNo();
    if (npc->isPlayerNear(player, 1, NULL, range)) {
        mChatFlags[2] = 1;
    } else {
        mChatFlags[2] = 0;
    }
}

// .text C8FC
void action_c::stepRemoteHalloweenWait00(dAcNpcOut_c *npc) {
    if (npc->isLocalOwner()) {
        if (npc->isPermitSelf()) {
            npc->fn_80013474();
            startTalk(npc);
        } else if (npc->checkLocalOwner()) {
            npc->fn_800134A4();
            saveDefaultAction(npc, FALSE);
            restoreAction(npc);
        }
    } else {
        if (shouldChangeEventMode(npc)) {
            if (npc->isDaubMaskChange()) {
                setAction(npc, ACTION_REMOTE_MASK_CHANGE);
            } else {
                npc->changeMask();
                saveHouseAction(npc);
                restoreAction(npc);
            }
        } else {
            if (mChatFlags[2]) {
                float range = cRemoteHalloweenTrackDistance;
                int player = dNet::getMyNo();
                if (!npc->isPlayerNear(player, 1, NULL, range)) {
                    mChatFlags[2] = 0;
                }
            }
            if (!mChatFlags[2]) {
                float range = 40.0f;
                int player = dNet::getMyNo();
                if (npc->isPlayerNear(player, 1, NULL, range)) {
                    if (!dDemo::isType(dDemo::TYPE_ANY) &&
                        !npc->isInUse(NULL) && dDemo::requestSpeak(npc, NULL)) {
                        npc->mBusy = 1;
                        mChasePlayer = 4;
                        mStep = 1;
                    } else if (dDemo::isType(dDemo::TYPE_SPEAK) ||
                               dDemo::isType(dDemo::TYPE_TALK)) {
                        mChatFlags[2] = 1;
                    }
                }
            }
        }
    }
}

// .text CAFC
void action_c::stepRemoteHalloweenWait01(dAcNpcOut_c *npc) {
    if (npc->isLocalOwner()) {
        if (npc->isPermitSelf()) {
            npc->fn_80013474();
            startTalk(npc);
        } else if (npc->checkLocalOwner()) {
            npc->fn_800134A4();
            saveDefaultAction(npc, FALSE);
            restoreAction(npc);
        }
    } else {
        int owner = 4;
        float distance = 40.0f;
        if ((npc->isInUse(&owner) && owner != dNet::getMyNo()) ||
            !npc->isPlayerNear(dNet::getMyNo(), 1, NULL, distance)) {
            mStep = 0;
            npc->mBusy = 0;
            if (dDemo::isType(dDemo::TYPE_SPEAK) ||
                dDemo::isType(dDemo::TYPE_TALK)) {
                mChatFlags[2] = 1;
            }
        } else if (dDemo::requestSpeak(npc, NULL)) {
            npc->mBusy = 1;
            mSavedAction = ACTION_REMOTE_HALLOWEEN_WAIT;
        }
    }
}

const action_c::procFunc sRemoteHalloweenWaitSteps[] = {
    &action_c::stepRemoteHalloweenWait00, &action_c::stepRemoteHalloweenWait01};

// .text CC64
void action_c::executeRemoteHalloweenWait(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sRemoteHalloweenWaitSteps)) {
        (this->*sRemoteHalloweenWaitSteps[mStep])(npc);
    }
}

const float action_c::cChatPartnerDistance = 60.0f;

// .text CCA4
void action_c::enterNpcChat(dAcNpcOut_c *npc) {
    dAcNpcNml_c *partner = static_cast<dAcNpcNml_c *>(npc->mpChatPartner);
    int angleValue;
    if (partner) {
        angleValue =
            (s16)dActor_c::targetAngleY(&npc->mPos, partner->getPosP());
    } else {
        angleValue = 0;
    }
    mAng angle(angleValue);
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->mCheckGround = 1;
    npc->mAction.requestTurn(PRIORITY_IDLE, angle, l_turnSpeed, 0);
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_NORMAL);
    npc->mLookAt.setActor(0, partner, true, l_lookPitchStep, l_lookYawStep,
                          cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 0;
    setMood(npc, 0, 0, 1);
    npc->mMood.mShowEffects = 1;
    npc->stopEmotionManpu();
    request(ACTION_NPC_CHAT_RESERVED, ACTION_NPC_CHAT_AFTER_TALK,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_TOWN_3P));
    mChatFlags[0] = 0;
    mChatFlags[1] = 0;
    npc->mChatOutcomePending = 0;
    npc->mApproachActive = 0;
    npc->offDaubWatchFireworks();
}

// .text CE24
void action_c::stepNpcChat00(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_TURN && action->mCanChange) {
        action->requestWait(PRIORITY_IDLE);
        mTimer0 = 300;
        mStep = 1;
    } else if (npc->mHitByNet) {
        mChatFlags[0] = 1;
        if (npc->isItchy()) {
            mChatFlags[1] = 1;
        }
        mStep = 4;
    }
}

// .text CED4
BOOL action_c::isChatPartnerNear(dAcNpcOut_c *npc) {
    if (!npc->mpChatPartner) {
        return FALSE;
    }
    mVec3_c diff = *npc->mpChatPartner->getPosP() - npc->mPos;
    float distance = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    if (distance < 0.0f || distance >= cChatPartnerDistance) {
        return FALSE;
    }
    return TRUE;
}

// .text CF60
static inline dAcNpc_c::action_c *getNpcAction(dAcNpc_c *npc) {
    return &npc->mAction;
}

void action_c::stepNpcChat01(dAcNpcOut_c *npc) {
    dAcNpcOut_c *partner = static_cast<dAcNpcOut_c *>(npc->mpChatPartner);
    if (!isChatPartnerNear(npc)) {
        mStep = 4;
        return;
    }
    if (npc->mHitByNet) {
        mChatFlags[0] = 1;
        if (npc->isItchy()) {
            mChatFlags[1] = 1;
        }
        mStep = 4;
        return;
    }
    if (!partner) {
        return;
    }
    bool otherReady;
    dAcNpc_c::action_c *npcAction = getNpcAction(npc);
    dAcNpc_c::action_c *partnerAction;
    int action = npcAction->mActionId;
    bool ready = npcAction->mCanChange &&
                 (action != dAcNpc_c::ACTION_EMOTION || !npcAction->mEmotion);
    partnerAction = getNpcAction(partner);
    int partnerActionId = partnerAction->mActionId;
    otherReady = partnerAction->mCanChange &&
                 (partnerActionId != dAcNpc_c::ACTION_EMOTION ||
                  !partnerAction->mEmotion);
    static const int emotions[] = {20, 21, 22, 25, 27, 4, 11, 29};
    if (!mTimer0) {
        if (ready) {
            if (otherReady) {
                if (dScene::getCurrentScene() == dScene::SCENE_FIELD) {
                    mStep = 2;
                    mChatFlags[0] = 0;
                    mChatFlags[1] = 0;
                    npc->mChatOutcomePending = 1;
                } else {
                    mStep = 4;
                }
            }
        } else if (action == dAcNpc_c::ACTION_EMOTION && npcAction->mEmotion) {
            dAcNpc_c::model_c *model = npc->getModel();
            dAcNpc_c::anm_c *anm = &model->mAnm;
            if ((anm->mPlayMode && npcAction->mCanChange) ||
                (!anm->mPlayMode &&
                 (anm->checkFrame(0.0f) || npcAction->mCanChange))) {
                npcAction->requestEmotion(PRIORITY_IDLE, 0, cNpcMorphFrames);
            }
        }
    } else {
        if (ready) {
            if (mTimer0 > 60 && cM::rndInt(50) == 0) {
                int emotionIndex = cM::rndInt(cChatEmotionCount);
                npcAction->requestEmotion(PRIORITY_IDLE, emotions[emotionIndex],
                                          cNpcMorphFrames);
            }
        } else if (action == dAcNpc_c::ACTION_EMOTION && npcAction->mEmotion) {
            dAcNpc_c::model_c *model = npc->getModel();
            dAcNpc_c::anm_c *anm = &model->mAnm;
            if ((anm->mPlayMode && npcAction->mCanChange) ||
                (!anm->mPlayMode &&
                 (anm->checkFrame(0.0f) || npcAction->mCanChange))) {
                npcAction->requestEmotion(PRIORITY_IDLE, 0, cNpcMorphFrames);
            }
        }
        if (otherReady && mTimer0 > 60 && cM::rndInt(50) == 0) {
            int emotionIndex = cM::rndInt(cChatEmotionCount);
            partnerAction->requestEmotion(PRIORITY_IDLE, emotions[emotionIndex],
                                          cNpcMorphFrames);
        }
    }
}

// .text D280
BOOL action_c::chooseChatLearner(dAcNpcOut_c **teacher, dAcNpcOut_c **learner,
                                 dAcNpcOut_c *npc) {
    if (!teacher || !learner) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (!player) {
        return FALSE;
    }
    dAnimal_c *animal = npc->mpAnimal;
    if (!animal) {
        return FALSE;
    }
    dAcNpcOut_c *partner = static_cast<dAcNpcOut_c *>(npc->mpChatPartner);
    if (!partner) {
        return FALSE;
    }
    dPersonalID_c *pid;
    dAnimal_c *otherAnimal = partner->mpAnimal;
    pid = &player->mPID;
    if (!otherAnimal) {
        return FALSE;
    }
    dAnimalMemory_c *a = animal->findMemory2(pid);
    s8 af;
    if (a) {
        af = a->getFriendship();
    } else {
        af = -128;
    }
    dAnimalMemory_c *b = otherAnimal->findMemory2(pid);
    s8 bf;
    if (b) {
        bf = b->getFriendship();
    } else {
        bf = -128;
    }
    if (af > bf || (af == bf && (cM::rndInt(4) & 1))) {
        *teacher = npc;
        *learner = partner;
    } else {
        *teacher = partner;
        *learner = npc;
    }
    return TRUE;
}

const u32 action_c::cChatEmotionCount = 8;

static const int sChatMoods[] = {0, 1, 2, 3};

// .text D3A4
void action_c::stepChatOutcome00(dAcNpcOut_c *npc) {
    dAcNpcOut_c *partner;
    u32 choice;
    const u8 *row;
    u32 mood;
    bool firstChanged;
    partner = static_cast<dAcNpcOut_c *>(npc->mpChatPartner);
    dAnimal_c *animal = npc->mpAnimal;
    int rank = 3;
    dAnimal_c *other = partner ? partner->mpAnimal : NULL;
    if (animal && partner) {
        rank = dNpc::getAnimalCompatibilityRank(animal, other);
    }
    static const u8 weights[3][4] = {
        {40, 40, 10, 10}, {50, 30, 10, 10}, {30, 10, 30, 30}};
    choice = 4;
    if ((u32)rank < 3) {
        row = weights[rank];
        u32 r = cM::rndInt(100);
        for (u32 i = 0; i < 4; ++i) {
            if (r < row[i]) {
                choice = i;
                break;
            }
            r -= row[i];
        }
    }
    mood = 0;
    if (choice < 4) {
        mood = sChatMoods[choice];
    }
    if (mood && mood < 5) {
        firstChanged = false;
        if (cM::rndInt(4) & 1) {
            setMood(npc, mood, 2, 1);
            firstChanged = true;
        }
        if (!firstChanged || (cM::rndInt(4) & 1)) {
            setMood(partner, mood, 2, 1);
        }
    }
}

// .text D518
void action_c::stepChatOutcome01(dAcNpcOut_c *npc) {
    setMood(npc, 0, 0, 1);
    if (npc->mpChatPartner) {
        setMood(static_cast<dAcNpcOut_c *>(npc->mpChatPartner), 0, 0, 1);
    }
}

// .text D57C
static inline bool chatAnimalDiffers(u16 key, const dAnmPersonalID_c &a,
                                     const dAnmPersonalID_c &b) {
    return !(key == b.mLand.mId && a.mLand.mRegion == b.mLand.mRegion &&
             memcmp(a.mLand.mName, b.mLand.mName, sizeof(a.mLand.mName)) == 0 &&
             a.mLand2 == b.mLand2 && a.mNpcIdx == b.mNpcIdx);
}
static inline BOOL chatLandKeyMatches(u16 key, const dLandID_c &a,
                                      const dLandID_c &b) {
    return key == b.mId && a.mRegion == b.mRegion ? TRUE : FALSE;
}
static inline BOOL chatLandMatches(u16 key, const dLandID_c &a,
                                   const dLandID_c &b) {
    return chatLandKeyMatches(key, a, b) &&
           memcmp(a.mName, b.mName, sizeof(a.mName)) == 0;
}
void action_c::stepChatOutcome02(dAcNpcOut_c *npc) {
    dAcNpcOut_c *teacher = NULL, *learner = NULL;
    if (chooseChatLearner(&teacher, &learner, npc) && teacher && learner) {
        dAnimal_c *first = teacher->mpAnimal, *second = learner->mpAnimal;
        if (first && second) {
            dAnimalBlock_c *block;
            const dAnmPersonalID_c *id;
            bool sameFirst;
            bool firstLandsMatch;
            u16 key;
            dAnimal_c *recipient = NULL;
            u32 count;
            int i;
            dAnimal_c *animal;
            dHabitWord_c firstWord;
            firstWord.clear();
            first->getHabitWord(&firstWord, 10, 0);
            dHabitWord_c secondWord;
            secondWord.clear();
            second->getHabitWord(&secondWord, 10, 0);
            if (!firstWord.isSame(&secondWord)) {
                recipient = second;
            } else {
                block = &dSaveData_c::getTown()->mAnimals.mTown;
                count = 0;
                for (i = 0; i < 10; ++i) {
                    animal = block->getAnimal(i);
                    if (animal) {
                        id = &animal->mID;
                        if (id->isValid()) {
                            key = id->mLand.mId;
                            sameFirst = false;
                            firstLandsMatch = false;
                            if (chatLandMatches(key, id->mLand,
                                                first->mID.mLand) &&
                                id->mLand2 == first->mID.mLand2) {
                                firstLandsMatch = true;
                            }
                            if (firstLandsMatch &&
                                id->mNpcIdx == first->mID.mNpcIdx) {
                                sameFirst = true;
                            }
                            if (!sameFirst &&
                                chatAnimalDiffers(key, *id, second->mID)) {
                                dHabitWord_c word;
                                word.clear();
                                animal->getHabitWord(&word, 10, 0);
                                if (!word.isSame(&firstWord)) {
                                    float chance = 100.0f / (float)(count + 1);
                                    if (cM::rndF(100.0f) <= chance) {
                                        recipient = animal;
                                    }
                                    ++count;
                                }
                            }
                        }
                    }
                }
            }
            if (recipient) {
                recipient->setHabitFromWord(&firstWord, 10);
            }
        }
        setMood(teacher, 1, 2, 1);
        setMood(learner, 1, 2, 1);
    }
}

// .text D92C
BOOL action_c::canExchangeDesign(dAcNpcOut_c *npc) {
    if (!npc->mpAnimal) {
        return FALSE;
    }
    if (npc->mpAnimal->isWearingOrgCloth()) {
        return TRUE;
    }
    dAcNpcNml_c *partner = static_cast<dAcNpcNml_c *>(npc->mpChatPartner);
    if (!partner) {
        return FALSE;
    }
    if (!partner->mpAnimal) {
        return FALSE;
    }
    if (partner->mpAnimal->isWearingOrgCloth()) {
        return TRUE;
    }
    return FALSE;
}
// .text D9B4
void action_c::stepChatOutcome03(dAcNpcOut_c *npc) {
    dAcNpcOut_c *teacher = NULL, *learner = NULL;
    if (chooseChatLearner(&teacher, &learner, npc) && teacher && learner) {
        dAnimal_c *first = teacher->mpAnimal, *second = learner->mpAnimal;
        const dAnimal_c *animal;
        dAcNpcOut_c *changed = NULL;
        if (first && second) {
            dAnimal_c *giver = NULL, *receiver = NULL;
            if (first->isWearingOrgCloth()) {
                giver = first;
                const dDesign_c *design = &first->mClothDesign;
                if (!second->isWearingOrgCloth() ||
                    design->isNotSame(&second->mClothDesign)) {
                    receiver = second;
                    changed = learner;
                }
            } else if (second->isWearingOrgCloth()) {
                giver = second;
                receiver = first;
                changed = teacher;
            }
            if (giver && receiver) {
                receiver->wearDesign(&giver->mClothDesign);
            }
        }
        BOOL requested = FALSE;
        if (changed) {
            animal = changed->mpAnimal;
            if (animal) {
                changed->mCloth.clearLoaded();
                requested = changed->mAction.requestChangeCloth2(
                    PRIORITY_IDLE, animal->mCloth, (int)&animal->mClothDesign);
            }
            setMood(teacher, 1, 2, 0);
            setMood(learner, 1, 2, 0);
        } else {
            setMood(teacher, 1, 2, 1);
            setMood(learner, 1, 2, 1);
        }
        if (requested) {
            mStep = 3;
            mChatFlags[0] = 0;
            mChatFlags[1] = 0;
        }
    }
}

// .text DB80
BOOL action_c::canExchangeCloth(dAcNpcOut_c *npc) {
    if (!npc->mpAnimal) {
        return FALSE;
    }
    if (npc->mpAnimal->isWearingCloth()) {
        return TRUE;
    }
    dAcNpcNml_c *partner = static_cast<dAcNpcNml_c *>(npc->mpChatPartner);
    if (!partner) {
        return FALSE;
    }
    if (!partner->mpAnimal) {
        return FALSE;
    }
    if (partner->mpAnimal->isWearingCloth()) {
        return TRUE;
    }
    return FALSE;
}
// .text DC08
void action_c::stepChatOutcome04(dAcNpcOut_c *npc) {
    dAcNpcOut_c *teacher = NULL, *learner = NULL;
    if (chooseChatLearner(&teacher, &learner, npc) && teacher && learner) {
        dAnimal_c *first = teacher->mpAnimal, *second = learner->mpAnimal;
        const dItem::Item *firstCloth, *secondCloth;
        dAnimal_c *animal;
        dAcNpcOut_c *changed = NULL;
        if (first && second) {
            firstCloth = &first->mCloth;
            dAnimal_c *giver = NULL, *receiver = NULL;
            BOOL firstWearing = first->isWearingCloth();
            secondCloth = &second->mCloth;
            BOOL secondWearing = second->isWearingCloth();
            if (firstWearing) {
                giver = first;
                if (!secondWearing || firstCloth->isNotSame(*secondCloth)) {
                    receiver = second;
                    changed = learner;
                }
            } else if (secondWearing) {
                giver = second;
                receiver = first;
                changed = teacher;
            }
            if (giver && receiver) {
                receiver->setCloth(&giver->mCloth);
            }
        }
        BOOL requested = FALSE;
        if (changed) {
            animal = changed->mpAnimal;
            if (animal) {
                changed->mCloth.clearLoaded();
                requested = changed->mAction.requestChangeCloth2(
                    PRIORITY_IDLE, animal->mCloth, 0);
            }
            setMood(teacher, 1, 2, 0);
            setMood(learner, 1, 2, 0);
        } else {
            setMood(teacher, 1, 2, 1);
            setMood(learner, 1, 2, 1);
        }
        if (requested) {
            mStep = 3;
            mChatFlags[0] = 0;
            mChatFlags[1] = 0;
        }
    }
}

// .text DDD4
void action_c::stepChatOutcome05(dAcNpcOut_c *npc) {
    if (npc->mpChatPartner) {
        int first = cM::rndInt(4);
        setMood(npc, sChatMoods[first], 2, 1);
        int second = cM::rndInt(4);
        setMood(npc, sChatMoods[second], 2, 1);
    }
}

// .text DE70
const dDesign_c *action_c::chooseTailorDesign(const dDesign_c *current) {
    dSaveShopTailor_c *shop = &dSaveData_c::getTown()->mShops.mShopTailor;
    BOOL valid = current ? current->mCreator.isValid() : FALSE;
    const dDesign_c *chosen = NULL;
    u32 count = 0;
    for (int i = 4; i < 8; ++i) {
        const dDesign_c *design = shop->getDesign(i);
        if (design->mCreator.isValid() &&
            (!valid || design->isNotSame(current))) {
            float chance = 100.0f / (float)(count + 1);
            if (cM::rndF(100.0f) <= chance) {
                chosen = design;
            }
            ++count;
        }
    }
    return chosen;
}

// .text DF9C
BOOL action_c::canLearnTailorDesign(dAcNpcOut_c *npc) {
    dAcNpcOut_c *teacher = NULL, *learner = NULL;
    if (chooseChatLearner(&teacher, &learner, npc) && teacher && learner &&
        teacher->mpAnimal) {
        dAnimal_c *animal = teacher->mpAnimal;
        const dDesign_c *design = NULL;
        if (animal->isWearingOrgCloth()) {
            design = &animal->mClothDesign;
        }
        if (chooseTailorDesign(design)) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text E044
void action_c::stepChatOutcome06(dAcNpcOut_c *npc) {
    dAcNpcOut_c *teacher = NULL, *learner = NULL;
    if (chooseChatLearner(&teacher, &learner, npc) && teacher && learner) {
        dAcNpcOut_c *changed = NULL;
        dAnimal_c *animal = teacher->mpAnimal;
        if (animal) {
            const dDesign_c *current = NULL;
            if (animal->isWearingOrgCloth()) {
                current = &animal->mClothDesign;
            }
            const dDesign_c *design = chooseTailorDesign(current);
            if (design) {
                animal->wearDesign(design);
                changed = teacher;
            }
        }
        BOOL requested = FALSE;
        if (changed) {
            animal = changed->mpAnimal;
            if (animal) {
                changed->mCloth.clearLoaded();
                requested = changed->mAction.requestChangeCloth2(
                    PRIORITY_IDLE, animal->mCloth, (int)&animal->mClothDesign);
            }
            setMood(teacher, 1, 2, 0);
        } else {
            setMood(teacher, 1, 2, 1);
        }
        if (requested) {
            mStep = 3;
            mChatFlags[0] = 0;
            mChatFlags[1] = 0;
        }
    }
}

const action_c::procFunc sChatOutcomeSteps[] = {
    &action_c::stepChatOutcome00, &action_c::stepChatOutcome01,
    &action_c::stepChatOutcome02, &action_c::stepChatOutcome03,
    &action_c::stepChatOutcome04, &action_c::stepChatOutcome05,
    &action_c::stepChatOutcome06};

// .text E190
void action_c::stepNpcChat02(dAcNpcOut_c *npc) {
    static BOOL (action_c::*checks[7])(dAcNpcOut_c *) = {
        NULL,
        NULL,
        NULL,
        &action_c::canExchangeDesign,
        &action_c::canExchangeCloth,
        NULL,
        &action_c::canLearnTailorDesign,
    };
    dPersonalID_c *pid;
    dAcNpcOut_c *partner = static_cast<dAcNpcOut_c *>(npc->mpChatPartner);
    dAnimal_c *other;
    npc->mBusy = 1;
    if (partner) {
        partner->mBusy = 1;
    }
    static const u8 initialWeights[] = {15, 15, 15, 15, 10, 15, 15};
    u8 weights[7];
    BOOL (action_c::* * check)(dAcNpcOut_c *);
    const procFunc *outcome;
    u8 *weightData = weights;
    cLib::memCpy(weightData, initialWeights, 7);
    check = checks;
    int total = 0;
    for (int i = 0; i < 7; ++i, ++check, ++weightData) {
        if (*check && !(this->*(*check))(npc)) {
            *weightData = 0;
        } else {
            total += *weightData;
        }
    }
    u32 choice = 0;
    if (total) {
        u32 r = cM::rndInt(total);
        for (u32 j = 0; j < 7; ++j) {
            if (r < 15) {
                choice = j;
                break;
            }
            r -= weights[j];
        }
    }
    if (choice < 7) {
        outcome = &sChatOutcomeSteps[choice];
        if (*outcome) {
            (this->*(*outcome))(npc);
        }
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player) {
        dAnimal_c *animal = npc->mpAnimal;
        pid = &player->mPID;
        if (partner) {
            other = partner->mpAnimal;
        } else {
            other = NULL;
        }
        if (animal && other) {
            dAnimalMemory_c *a = animal->findMemory(pid),
                            *b = other->findMemory(pid);
            if (a && b) {
                dAnimalMemory_c *from = NULL, *to = NULL;
                u32 hasA = a->mFlags.mHasNickname;
                u32 hasB = b->mFlags.mHasNickname;
                if (hasA && hasB) {
                    u32 aw = a->mFlags.mNicknameWait;
                    u32 bw = b->mFlags.mNicknameWait;
                    if (aw > bw) {
                        from = a;
                        to = b;
                    } else if (aw < bw) {
                        from = b;
                        to = a;
                    } else {
                        s8 af = a->getFriendship();
                        s8 bf = b->getFriendship();
                        if (af > bf) {
                            from = a;
                            to = b;
                        } else if (af < bf) {
                            from = b;
                            to = a;
                        } else if (cM::rndInt(2) == 0) {
                            from = a;
                            to = b;
                        } else {
                            from = b;
                            to = a;
                        }
                    }
                } else if (hasA) {
                    from = a;
                    to = b;
                } else if (hasB) {
                    from = b;
                    to = a;
                }
                if (from && to) {
                    to->copyNickname(from);
                }
            }
        }
    }
    if (mStep == 2) {
        mChatFlags[0] = 0;
        mChatFlags[1] = 0;
        mStep = 4;
    }
}

// .text E524
void action_c::stepNpcChat03(dAcNpcOut_c *npc) {
    dAcNpcNml_c *partner = static_cast<dAcNpcNml_c *>(npc->mpChatPartner);
    if (!partner) {
        return;
    }
    if (partner->mAction.mActionId == dAcNpc_c::ACTION_CHANGE_CLOTH2 &&
        !partner->mAction.mCanChange) {
        return;
    }
    if (npc->mAction.mActionId == dAcNpc_c::ACTION_CHANGE_CLOTH2 &&
        !npc->mAction.mCanChange) {
        return;
    }
    npc->mMood.mTimerPending = 1;
    partner->mMood.mTimerPending = 1;
    mStep = 4;
}

// .text E578
void action_c::stepNpcChat04(dAcNpcOut_c *npc) {
    if (npc->mpChatPartner &&
        static_cast<dAcNpcNml_c *>(npc->mpChatPartner)->endNpcChat()) {
        npc->stopEmotionManpu();
        npc->mpChatPartner = NULL;
        if (mChatFlags[0]) {
            if (mChatFlags[1]) {
                setAction(npc, ACTION_ITCHY);
            } else {
                setAction(npc, ACTION_NET_HIT);
            }
        } else {
            saveDefaultAction(npc, FALSE);
            restoreAction(npc);
        }
        startCooldown();
    }
}

const action_c::procFunc sNpcChatSteps[] = {
    &action_c::stepNpcChat00, &action_c::stepNpcChat01,
    &action_c::stepNpcChat02, &action_c::stepNpcChat03,
    &action_c::stepNpcChat04};

// .text E648
void action_c::executeNpcChat(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sNpcChatSteps)) {
        (this->*sNpcChatSteps[mStep])(npc);
    }
}

// .text E688
void action_c::enterNpcChatPartner(dAcNpcOut_c *npc) {
    dAcNpcNml_c *partner = static_cast<dAcNpcNml_c *>(npc->mpChatPartner);
    int angleValue;
    if (partner) {
        angleValue =
            (s16)dActor_c::targetAngleY(&npc->mPos, partner->getPosP());
    } else {
        angleValue = 0;
    }
    mAng angle(angleValue);
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->mCheckGround = 1;
    npc->mAction.requestTurn(PRIORITY_IDLE, angle, l_turnSpeed, 0);
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_NORMAL);
    npc->mLookAt.setActor(0, partner, true, l_lookPitchStep, l_lookYawStep,
                          cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 0;
    setMood(npc, 0, 0, 1);
    npc->mMood.mShowEffects = 1;
    npc->stopEmotionManpu();
    request(ACTION_NPC_CHAT_RESERVED, ACTION_NPC_CHAT_AFTER_TALK,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_TOWN_3P));
    npc->mApproachActive = 0;
    npc->offDaubWatchFireworks();
}

// .text E7FC
void action_c::stepNpcChatPartner00(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_TURN && action->mCanChange) {
        action->requestWait(PRIORITY_IDLE);
        mStep = 1;
    } else if (npc->mHitByNet) {
        dAcNpcNml_c *partner = static_cast<dAcNpcNml_c *>(npc->mpChatPartner);
        if (partner && !partner->vtC8() && partner->endNpcChat()) {
            npc->stopEmotionManpu();
            npc->mpChatPartner = NULL;
            startCooldown();
            if (npc->isItchy()) {
                setAction(npc, ACTION_ITCHY);
            } else {
                setAction(npc, ACTION_NET_HIT);
            }
        }
    }
}

// .text E91C
void action_c::stepNpcChatPartner01(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_EMOTION && action->mEmotion) {
        dAcNpc_c::model_c *model = npc->getModel();
        dAcNpc_c::anm_c *anm = &model->mAnm;
        if ((anm->mPlayMode && action->mCanChange) ||
            (!anm->mPlayMode &&
             (anm->checkFrame(0.0f) || action->mCanChange))) {
            action->requestEmotion(PRIORITY_IDLE, 0, cNpcMorphFrames);
        }
    } else if (npc->mHitByNet) {
        dAcNpcNml_c *partner = static_cast<dAcNpcNml_c *>(npc->mpChatPartner);
        if (partner && !partner->vtC8() && partner->endNpcChat()) {
            npc->stopEmotionManpu();
            npc->mpChatPartner = NULL;
            startCooldown();
            if (npc->isItchy()) {
                setAction(npc, ACTION_ITCHY);
            } else {
                setAction(npc, ACTION_NET_HIT);
            }
        }
    }
}

const action_c::procFunc sNpcChatPartnerSteps[] = {
    &action_c::stepNpcChatPartner00, &action_c::stepNpcChatPartner01};

// .text EA90
void action_c::executeNpcChatPartner(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sNpcChatPartnerSteps)) {
        (this->*sNpcChatPartnerSteps[mStep])(npc);
    }
}

// .text EAD0
void action_c::enterNpcChatReserved(dAcNpcOut_c *npc) {
}

// .text EAD4
void action_c::executeNpcChatReserved(dAcNpcOut_c *npc) {
}

// .text EAD8
void action_c::enterNpcChatAfterTalk(dAcNpcOut_c *npc) {
    npc->mTalk.requestTalk(true, 0);
    npc->mBusy = 0;
    npc->offDaubToolChange();
    npc->mApproachActive = 0;
    npc->offDaubWatchFireworks();
}

// .text EB34
void action_c::stepNpcChatAfterTalk00(dAcNpcOut_c *npc) {
    if (!npc->mTalk.isBusy()) {
        npc->mMood.mTimerPending = 1;
        dDemo::finishTalk(npc);
        npc->mpChatPartner = NULL;
        startCooldown();
        ++mStep;
    }
}

const action_c::procFunc sNpcChatAfterTalkSteps[] = {
    &action_c::stepNpcChatAfterTalk00};

// .text EBA4
void action_c::executeNpcChatAfterTalk(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sNpcChatAfterTalkSteps)) {
        (this->*sNpcChatAfterTalkSteps[mStep])(npc);
    }
}

// .text EBE4
void action_c::enterNpcChatBusy(dAcNpcOut_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE);
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->mCheckGround = 1;
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_FIXED);
    npc->mBusy = 1;
    setMood(npc, 0, 0, 1);
    npc->mMood.mShowEffects = 1;
    npc->stopEmotionManpu();
    npc->mApproachActive = 0;
    npc->offDaubWatchFireworks();
}

// .text ECAC
void action_c::executeNpcChatBusy(dAcNpcOut_c *npc) {
}

// .text ECB0
void action_c::enterEnterHouse(dAcNpcOut_c *npc) {
    npc->mAction.requestTurn(PRIORITY_IDLE, mAng(-32768), l_turnSpeed, 0);
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_FIXED);
    npc->mBusy = 1;
    npc->mMood.mShowEffects = 0;
    npc->mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->stopEmotionManpu();
    npc->mCheckGround = 0;
    npc->mApproachActive = 0;
    npc->offDaubWatchFireworks();
    dAnimal_c *animal = npc->mpAnimal;
    int place;
    if (animal) {
        place = 1;
        if (animal->isSleepTime(NULL)) {
            place = 4;
        }
        npc->setPendingPlace(place);
        animal->applyNewItems(npc->getNpcIdx(), 0);
        animal->clearQuestStarted();
        if (place != 4) {
            animal->setPlaceChangeTime(10, NULL);
        }
    }
}

// .text EE28
void action_c::stepEnterHouse00(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_TURN && action->mCanChange) {
        mStep = 5;
        if (npc->mTool.mItem.mId != dItem::ITEM_ID_NONE) {
            initToolChange(npc, 1, 2, 5, 5);
        }
        if (mStep == 5) {
            daSTR_c *house = dSTR::getAcList()->searchNearest(npc->mPos, 0);
            if (house) {
                mVec3_c pos = dAStr::getNpcDoorPos(house, &npc->mPos, 0,
                                                   dAStr::cNpcDoorOffset);
                action->requestMoveParam(PRIORITY_IDLE, ANM_WALK, pos,
                                         l_moveParamWalk, m3d::PLAYMODE_INHERIT,
                                         0.1f, 0.0f, cNpcMorphFrames);
                dAStr::requestNpcEnter(house);
                mStep = 4;
            }
        }
    }
}

// .text EF3C
void action_c::stepEnterHouse01(dAcNpcOut_c *npc) {
    if (putAwayTool(npc)) {
        mStep = 3;
    }
}

// .text EF78
void action_c::stepEnterHouse02(dAcNpcOut_c *npc) {
    if (closeUmbrella(npc)) {
        mStep = 3;
    }
}

// .text EFB4
void action_c::stepEnterHouse03(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mCanChange) {
        daSTR_c *house = dSTR::getAcList()->searchNearest(npc->mPos, 0);
        if (house) {
            mVec3_c pos = dAStr::getNpcDoorPos(house, &npc->mPos, 0,
                                               dAStr::cNpcDoorOffset);
            action->requestMoveParam(PRIORITY_IDLE, ANM_WALK, pos,
                                     l_moveParamWalk, m3d::PLAYMODE_INHERIT,
                                     0.1f, 0.0f, cNpcMorphFrames);
            dAStr::requestNpcEnter(house);
            mStep = 4;
        }
    }
}

// .text F088
void action_c::stepEnterHouse04(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_MOVE_PARAM &&
        action->mCanChange) {
        dAnimal_c *animal = npc->mpAnimal;
        if (animal) {
            u8 place = dAnimal_c::PLACE_HOME;
            if (animal->isSleepTime(NULL)) {
                place = dAnimal_c::PLACE_ASLEEP;
            }
            animal->mPlace = place;
            dNpc::sendAnimalPlace(npc->getNpcIdx(), place);
            npc->clearPendingPlace();
        }
        setAction(npc, ACTION_IN_HOUSE);
    } else {
        m3d::fanm_c *anm = &npc->getModel()->mAnm;
        float endFrame = anm->mFrameMax - 1.0f;
        if (anm->getFrame() >= endFrame) {
            npc->mIsActive = 0;
        }
    }
}

const action_c::procFunc sEnterHouseSteps[] = {
    &action_c::stepEnterHouse00, &action_c::stepEnterHouse01,
    &action_c::stepEnterHouse02, &action_c::stepEnterHouse03,
    &action_c::stepEnterHouse04};

// .text F18C
void action_c::executeEnterHouse(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sEnterHouseSteps)) {
        (this->*sEnterHouseSteps[mStep])(npc);
    }
}

// .text F1CC
void action_c::enterInHouse(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    action->requestWait(PRIORITY_IDLE);
    npc->mCheckGround = 0;
    npc->mIsActive = 0;
    npc->onDaubInHouse();
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_FIXED);
    npc->mBusy = 1;
    npc->mMood.mShowEffects = 0;
    npc->mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->stopEmotionManpu();
    dAnimal_c *animal = npc->mpAnimal;
    if (animal) {
        dSaveData_c *save = dSaveData_c::getRaw();
        mVec3_c pos = npc->mPos;
        if (save->mAnimals.mTown.getHousePosById(&pos, &animal->mID)) {
            npc->mPos = pos;
            npc->mLastPos = pos;
        }
    }
    npc->mFlea.clear();
    npc->mApproachActive = 0;
    npc->offDaubWatchFireworks();
}

// .text F314
void action_c::executeInHouse(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (npc->canLeaveHouse()) {
        mVec3_c house = mVec3_c::Zero;
        if (npc->getHousePos(&house) && !npc->isPlayerNearHouse()) {
            mVec3_c exit = house;
            exit.z += 64.0f;
            dAcNpc_c::viewArea_c *view = &npc->mViewArea;
            dFdBase_c::snapToUnitCenter(&exit, &exit);
            bool outside =
                view->isOutsideAll(4, &exit) && view->isOutsideAll(4, &house);
            bool visible = !dNet::isSessionActive() && !outside;
            if (visible || outside) {
                dAnimal_c *animal = npc->mpAnimal;
                if (animal) {
                    animal->mSpot.clear();
                    dNpc::sendAnimalSpotClear(npc->getNpcIdx());
                    animal->applyNewItems(npc->getNpcIdx(), 1);
                    animal->clearPlaceChangeTime();
                    dNpc::sendAnimalPlaceTimeClear(npc->getNpcIdx());
                }
                if (visible) {
                    setAction(npc, ACTION_LEAVE_HOUSE);
                } else {
                    if (animal) {
                        animal->mPlace = dAnimal_c::PLACE_OUTSIDE;
                        dNpc::sendAnimalPlace(npc->getNpcIdx(),
                                              dAnimal_c::PLACE_OUTSIDE);
                    }
                    exit.y = dBGCF::getGroundY(&exit, 0);
                    npc->mLastPos = npc->mPos = exit;
                    saveDefaultAction(npc, FALSE);
                    setAction(npc, ACTION_OFFSCREEN);
                    npc->mFlea.clear();
                }
            }
        }
    }
}

// .text F528
void action_c::enterLeaveHouse(dAcNpcOut_c *npc) {

    mVec3_c pos = npc->mPos;
    pos.z += 32.0f;
    daSTR_c *house = dSTR::getAcList()->searchNearest(pos, 0);
    if (house) {
        mVec3_c door =
            dAStr::getNpcDoorPos(house, &npc->mPos, 0, dAStr::cNpcDoorOffset);
        pos = door;
        pos.z += 16.0f;
    }
    npc->mPos = pos;
    npc->mLastPos = pos;
    npc->mAngle.y = 0;
    npc->mAngle3D.y = 0;
    npc->mSpeedF = 0.0f;
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_FIXED);
    npc->mBusy = 1;
    npc->mMood.mShowEffects = 0;
    npc->mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->stopEmotionManpu();
    npc->mIsActive = 0;
    if (house) {
        dAStr::requestNpcExit(house);
    }
    npc->mAction.requestAnm(PRIORITY_IDLE, ANM_LEAVE_HOUSE, 0.0f, 0.0f,
                            m3d::PLAYMODE_INHERIT, 0);
    npc->setPendingPlace(dAnimal_c::PLACE_OUTSIDE);
    npc->mFlea.clear();
    npc->mApproachActive = 0;
    npc->offDaubWatchFireworks();
}

// .text F6EC
void action_c::stepLeaveHouse00(dAcNpcOut_c *npc) {
    if (npc->mAction.mActionId == dAcNpc_c::ACTION_ANM &&
        npc->getModel()->mAnm.mAnmId == ANM_LEAVE_HOUSE) {
        if (!npc->mIsActive) {
            npc->mIsActive = 1;
            npc->mCheckGround = 1;
        }
        if (npc->mAction.mCanChange) {
            if (npc->mpAnimal) {
                npc->mpAnimal->mPlace = dAnimal_c::PLACE_OUTSIDE;
                dNpc::sendAnimalPlace(npc->getNpcIdx(),
                                      dAnimal_c::PLACE_OUTSIDE);
                npc->clearPendingPlace();
            }
            saveDefaultAction(npc, FALSE);
            restoreAction(npc);
        }
    }
}

const action_c::procFunc sLeaveHouseSteps[] = {&action_c::stepLeaveHouse00};

// .text F7B8
void action_c::executeLeaveHouse(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sLeaveHouseSteps)) {
        (this->*sLeaveHouseSteps[mStep])(npc);
    }
}

// .text F7F8
void action_c::enterRemoteIdle(dAcNpcOut_c *npc) {
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->mBusy = 0;
    npc->mAcceptMoveReq = 1;
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_NORMAL);
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    mLookTimer = (u32)(120.0f + cM::rndF(600.0f));
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    request(
        ACTION_FACE_PLAYER, ACTION_AFTER_TALK,
        dAcNpcNml_c::talk_c::getProcSet(dAcNpcNml_c::talk_c::TALK_PROC_ENTRY));
    npc->mFlea.clear();
    npc->mApproachActive = 0;
}

// .text F910
BOOL action_c::isRemoteToolReady(dAcNpcOut_c *npc) {
    dAcNpcOut_c::toolMng_c *tool;
    const dItem::Item *held;
    BOOL result = FALSE;
    if (npc->mpAnimal) {
        tool = &npc->mTool;
        held = &npc->mpAnimal->mHeldItem;
        if (held->isNotSame(tool->mItem) &&
            (held->mId == dItem::ITEM_ID_NONE || tool->isNextLoaded(npc))) {
            result = TRUE;
        }
    }
    return result;
}

// .text F9A8
void action_c::executeRemoteIdle(dAcNpcOut_c *npc) {
    if (npc->isLocalOwner()) {
        if (npc->isPermitSelf()) {
            npc->fn_80013474();
            startTalk(npc);
        } else if (npc->checkLocalOwner()) {
            npc->fn_800134A4();
            saveDefaultAction(npc, FALSE);
            restoreAction(npc);
        }
    } else {

        if (isRemoteToolReady(npc)) {
            if (npc->isDaubToolChange()) {
                setAction(npc, ACTION_REMOTE_TOOL_CHANGE);
            } else if (npc->isDaubToolChanged()) {
                npc->syncTool();
            }
        } else if (shouldChangeEventMode(npc)) {
            if (npc->isDaubMaskChange()) {
                setAction(npc, ACTION_REMOTE_MASK_CHANGE);
            } else {
                npc->changeMask();
            }
        } else if (npc->isClothChangeReady()) {
            if (npc->isDaubClothChange()) {
                setAction(npc, ACTION_REMOTE_CLOTH_CHANGE);
            } else if (npc->isDaubClothChanged()) {
                npc->changeClothModel();
            }
        } else if (npc->isDaubInHouse()) {
            setAction(npc, ACTION_REMOTE_HOUSE_WAIT);
        } else if (npc->isDaubWatchFireworks()) {
            setAction(npc, ACTION_REMOTE_WATCH_FIREWORKS);
        } else {
            dAcNpc_c::lookAt_c *look = &npc->mLookAt;
            bool around;
            int step;
            int type = look->mType;
            if (mLookTimer) {
                --mLookTimer;
            }
            if (shouldChangeLook(npc) ||
                (npc->isInUse(NULL) &&
                 type != dAcNpc_c::lookAt_c::LOOK_PLAYER)) {
                around = false;
                if (type != dAcNpc_c::lookAt_c::LOOK_AROUND &&
                    cM::rndF(100.0f) < 20.0f && !npc->isInUse(NULL)) {
                    mAng pitch, yaw, yawStep;
                    step = l_lookYawStep;
                    yawStep = cM::rndInt(step) + (step >> 1);
                    yaw = l_lookYawMax >> 1;
                    pitch = cM::rndInt(l_lookPitchMax >> 1);
                    look->setAround(0, pitch, yaw, l_lookPitchStep, yawStep);
                    mLookTimer = (u32)(120.0f + cM::rndF(120.0f));
                    around = true;
                    insectLink_s *insectLink = &npc->mInsectLink;
                    if (insectLink->mpInsect) {
                        dInsectField::removeNpcLink(insectLink->mpInsect,
                                                    insectLink);
                    }
                    dFishLink_c *fishLink = &npc->mFishLink;
                    if (fishLink->mFish) {
                        fishLink->mFish->removeLink(fishLink);
                    }
                }
                if (!around) {
                    if (type != dAcNpc_c::lookAt_c::LOOK_PLAYER) {
                        look->setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                                        cNpcLookRange);
                    }
                    mLookTimer = (u32)(240.0f + cM::rndF(240.0f));
                    insectLink_s *insectLink = &npc->mInsectLink;
                    if (insectLink->mpInsect) {
                        dInsectField::removeNpcLink(insectLink->mpInsect,
                                                    insectLink);
                    }
                    dFishLink_c *fishLink = &npc->mFishLink;
                    if (fishLink->mFish) {
                        fishLink->mFish->removeLink(fishLink);
                    }
                }
            }
        }
    }
}

// .text FD20
} // namespace dANpcOut
dAcNpcOut_c::toolMng_c::toolMng_c() {
    mItem = dItem::ITEM_ID_NONE;
    mNextItem = dItem::ITEM_ID_NONE;
    mLoadState = LOAD_PENDING;
}
// .text FD74
dAcNpcOut_c::toolMng_c::~toolMng_c() {
}
// .text FDCC
void dAcNpcOut_c::toolMng_c::create(dAcNpcOut_c *npc) {
    if (mBank.getType() == dHmnToolMng_c::SLOT_NUM) {
        mBank.create(npc->mNpcListSlot + dHmnToolMng_c::PLAYER_SLOT_NUM);
        checkItem(npc);
    }
}
// .text FE28
bool dAcNpcOut_c::toolMng_c::remove() {
    if (!mBank.syncLoad()) {
        return false;
    }
    mItem = dItem::ITEM_ID_NONE;
    mNextItem = dItem::ITEM_ID_NONE;
    mLoadState = LOAD_PENDING;
    mBank.remove();
    return true;
}
namespace dANpcOut {

// .text FE8C
flea_c::flea_c() {
    mTimer = 0;
    mState = STATE_UNINITIALIZED;
}

// .text FEE8
flea_c::~flea_c() {
}

// .text FF4C
void flea_c::clear() {
    mTimer = 0;
    mState = STATE_DISABLED;
    retire();
}

// .text FF60
void flea_c::init(dAcNpcOut_c *npc) {
    if (mState == STATE_UNINITIALIZED) {
        dAnimal_c *animal;
        bool disabled = false;
        if (dAcNpc_c::isMultiPlay() || npc->mBirthdayHost) {
            disabled = true;
        }
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
        animal = npc->mpAnimal;
        if (!disabled && (!player || player->isFlag0(13))) {
            disabled = true;
        }
        if (!disabled) {
            if (!animal || !animal->isLostItemRequestDone(NULL, 0)) {
                disabled = true;
            } else {
                dAnimalMemory_c *memory = animal->findMemory2(&player->mPID);
                if (!memory || !memory->mPlayer.isValid()) {
                    disabled = true;
                }
            }
        }
        if (!disabled && dInsectInfo::isFleaSpawn()) {
            mState = STATE_ENABLED;
        }
        if (mState != STATE_ENABLED) {
            mState = STATE_DISABLED;
        }
        mTimer = 0;
    }
}

// .text 10090
bool flea_c::isEnabled(dAcNpcOut_c *npc) {
    if (npc->mIsActive == 0) {
        return false;
    } else {
        return mState == STATE_ENABLED;
    }
}

// .text 100B4
BOOL flea_c::retire() {
    mEffect.retireAll();
    return TRUE;
}

const u32 flea_c::cEffectTimeBase = 45;
const u32 flea_c::cEffectTimeRange = 25;

// .text 100E4
void flea_c::execute(dAcNpcOut_c *npc) {
    if (!dAcNpc_c::isMultiPlay() && mState == STATE_ENABLED) {
        if (mTimer) {
            --mTimer;
        }
        if (mEffect.mHandle.IsValid() && (!isEnabled(npc) || !mTimer)) {
            retire();
        }
        if (!mTimer && isEnabled(npc)) {
            mVec3_c pos = mVec3_c::Zero;
            mAng3_c angle = mAng3_c::Zero;
            mMtx_c *matrix = &npc->mNodeMtx12;
            bool hasMatrix =
                memcmp(matrix, &mMtx_c::Identity, sizeof(mMtx_c)) != 0;
            if (hasMatrix) {
                mVec3_c world(matrix->m[0][3], matrix->m[1][3],
                              matrix->m[2][3]);
                dWorld::toFieldPosition(&pos, &world);
            }
            angle.y = npc->mAngle.y;
            if (mEffect.createEffect("af_npc_nomi", &pos, &angle, NULL)) {
                npc->mAudioObj.startSound(SE_Nomi);
                mTimer = cM::rndInt(cEffectTimeRange) + cEffectTimeBase;
            }
        }
    }
}

// .text 10284
void action_c::enterBirthdayVisit(dAcNpcOut_c *npc) {
    u32 home = dSaveData_c::getTown()->mHomes.findCurrentPlayer();
    if (home < 4) {
        int x = 0, z = 0;
        dSaveTown_c *town = dSaveData_c::getTown();
        if (town->mBuilding.mList.getPlayerHousePos(&x, &z, home)) {
            x += 2;
            z += 2;
            dFdBase_c::getUnitCenterPos(&npc->mPos, x, z);
            npc->mLastPos = npc->mPos;
        }
    }
    mAng angle = npc->getAngleYToPlayer(4);
    npc->mAngle.y = angle;
    npc->mAngle3D.y = angle;
    npc->mIsActive = 1;
    npc->mAction.requestWait(PRIORITY_IDLE);
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 0;
    npc->mMood.startTimer(0, 0);
    npc->mMood.mShowEffects = 0;
    npc->mMood.mTimerPending = 1;
    npc->stopEmotionManpu();
    fgMngProc_blockDayChange();
    request(ACTION_FACE_PLAYER, ACTION_AFTER_BIRTHDAY,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_PLAYER_BIRTHDAY));
    npc->setDemoFlag8();
    dBgm::StgField_c *field = &dBgm::l_mgr.mStgField;
    if (field) {
        field->play28();
    }
    npc->mFlea.clear();
    npc->mApproachActive = 0;
}

// .text 10440
void action_c::stepBirthdayVisit00(dAcNpcOut_c *npc) {
    dDemo::requestSpeak(npc, NULL);
}

const action_c::procFunc sBirthdayVisitSteps[] = {
    &action_c::stepBirthdayVisit00};

// .text 1044C
void action_c::executeBirthdayVisit(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sBirthdayVisitSteps)) {
        (this->*sBirthdayVisitSteps[mStep])(npc);
    }
}

// .text 1048C
void action_c::enterAfterBirthday(dAcNpcOut_c *npc) {
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mIsActive = 1;
    npc->mCheckGround = 1;
    mVec3_c pos = mVec3_c::Zero;
    const mVec3_c *player =
        dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    pos = *player;
    pos.z = npc->mPos.z;
    dPlayerMgr::moveTo(&pos, 4, dPlayerMgr::sDefaultMoveSpeed);
    npc->mBusy = 0;
    npc->offDaubToolChange();
    fgMngProc_unblockDayChange();
    npc->mApproachActive = 0;
}

// .text 1058C
void action_c::stepAfterBirthday00(dAcNpcOut_c *npc) {
    if (!dPlayerMgr::isMovingTo(4)) {
        npc->clearDemoFlag8();
        u16 angleValue = 0;
        if (npc->mNmlTalk.mpActActor) {
            angleValue = npc->getAngleYTo(npc->mNmlTalk.mpActActor).mAngle;
        }
        mAng angle(angleValue);
        mAng step;
        step.mAngle = (u16)l_turnSpeed.mAngle;
        npc->mTalk.requestTurnTalk(angle, step, 0);
        mStep = 1;
    }
}

// .text 10620
void action_c::stepAfterBirthday01(dAcNpcOut_c *npc) {
    if (!npc->mTalk.isBusy()) {
        dBgm::StgField_c *field = &dBgm::l_mgr.mStgField;
        if (field) {
            field->fn_800785C8();
        }
        npc->mMood.mTimerPending = 1;
        dDemo::finishTalk(npc);
        ++mStep;
    }
}

const action_c::procFunc sAfterBirthdaySteps[] = {
    &action_c::stepAfterBirthday00, &action_c::stepAfterBirthday01};

// .text 10694
void action_c::executeAfterBirthday(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sAfterBirthdaySteps)) {
        (this->*sAfterBirthdaySteps[mStep])(npc);
    }
}

// .text 106D4
void action_c::enterVisitWait(dAcNpcOut_c *npc) {
    npc->mAction.requestWait(PRIORITY_IDLE);
    mTimer1 = 720;
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->offDaubInHouse();
    npc->mCheckGround = 1;
    mSavedAction = ACTION_WANDER;
    request(ACTION_FACE_PLAYER, ACTION_AFTER_TALK,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_NPC_BIRTHDAY));
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_NORMAL);
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 0;
    npc->offDaubToolChange();
    npc->mMood.mShowEffects = 1;
    mSavedAction = ACTION_VISIT_WAIT;
    mBlockedFrames = 0;
    npc->mFlea.clear();
    npc->mApproachActive = 0;
}

// .text 10818
BOOL action_c::stopWander(dAcNpcOut_c *npc) {
    float zero = 0.0f;
    float speed = npc->mSpeedF;
    if (zero != speed) {
        if (avoidObstacle(npc)) {
            return TRUE;
        }
        if (stopIfPushed(npc)) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text 10894
void action_c::executeVisitWait(dAcNpcOut_c *npc) {

    BOOL outside = npc->mViewArea.isOutsideAll(4, npc);
    float speed = npc->mSpeedF;
    if (speed != 0.0f) {
        if ((npc->mAcch.mPrevHitFlags & 0x88) ||
            (npc->mAcch.mHitFlags & 0x88)) {
            if (mBlockedFrames < 0xFFFFFFFF) {
                ++mBlockedFrames;
            }
        } else {
            mBlockedFrames = 0;
        }
    } else {
        mBlockedFrames = 0;
    }
    if (outside) {
        setAction(npc, ACTION_OFFSCREEN);
    } else if (!stopWander(npc) && npc->mAction.mCanChange &&
               (npc->mAction.mActionId != dAcNpc_c::ACTION_WAIT || !mTimer1)) {
        mVec3_c pos = npc->mPos;
        if (cM::rndInt(5) < 3 &&
            dAcNpc_c::searchPosNear(&pos, &npc->mPos, l_8074FE14, l_8074FE10)) {
            mAng angle(0);
            if (npc->checkTargetAngle(&angle, &pos, &l_frontAngle)) {
                npc->mAction.requestWalk(PRIORITY_IDLE, pos, l_walkTurnSpeed, 0,
                                         l_moveParamWalk, l_defaultAnmRate,
                                         cNpcMorphFrames);
            } else {
                npc->mAction.requestTurnWalk(PRIORITY_IDLE, pos, angle,
                                             l_turnSpeed, 0, l_moveParamWalk,
                                             l_defaultAnmRate, cNpcMorphFrames);
            }
        } else {
            mTimer1 = cM::rndInt(cVisitWaitTimeRange) + cVisitWaitTimeBase;
            if (npc->mAction.mActionId != dAcNpc_c::ACTION_WAIT) {
                npc->mAction.requestWait(PRIORITY_IDLE);
            }
        }
    }
}

// .text 10AA0
void action_c::enterPitfall(dAcNpcOut_c *npc) {
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    mVec3_c pos = npc->mPos;
    dFdBase_c::snapToUnitCenter(&pos, &pos);
    npc->mAction.requestMoveParam(PRIORITY_IDLE, ANM_PITFALL_STUMBLE, pos,
                                  sPitfallMove, m3d::PLAYMODE_INHERIT, 0.1f,
                                  0.0f, cNpcMorphFrames);
    npc->mAudioObj.startSound(SE_OtoshianaGuratsuku);
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_FIXED);
    npc->mBusy = 1;
    npc->mMood.mShowEffects = 0;
    npc->mMood.mDisabled = 1;
    npc->mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->stopEmotionManpu();
    npc->mCheckGround = 0;
    npc->mPos.y = dBGCF::getGroundY(&npc->mPos, 0);
    float px = npc->mPos.x;
    float pz = npc->mPos.z;
    npc->setPendingUnit((int)px >> 5, (int)pz >> 5);
    mInPitfall = 0;
    mTimer0 = 24;
    npc->mApproachActive = 0;
    npc->offDaubWatchFireworks();
}

// .text 10C48
void action_c::stepPitfall00(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_MOVE_PARAM &&
        action->isArrived(npc, 1) && !mTimer0) {
        action->mSnapAngle = false;
        npc->mAngle3D.y = npc->mAngle.y;
        if (action->requestAnm(PRIORITY_IDLE, ANM_PITFALL_FALL, 0.0f,
                               cNpcMorphFrames, m3d::PLAYMODE_INHERIT, 0)) {
            npc->mAudioObj.startSound(SE_otoshiana_hamaru);
            float px = npc->mPos.x;
            float pz = npc->mPos.z;
            int x = (int)px >> 5;
            int z = (int)pz >> 5;
            dFgobjManager::requestPitfallEnter(dNet::getMyNo(), x, z);
            mStep = 1;
        }
    }
}

// .text 10D4C
void action_c::stepPitfall01(dAcNpcOut_c *npc) {

    dAcNpc_c::anm_c *anm = &npc->getModel()->mAnm;
    if (npc->mAction.mActionId == dAcNpc_c::ACTION_ANM &&
        anm->mAnmId == ANM_PITFALL_FALL) {
        if (npc->mAction.mCanChange) {
            if (npc->mAction.requestAnm(PRIORITY_IDLE, ANM_PITFALL_STRUGGLE,
                                        0.0f, cNpcMorphFrames,
                                        m3d::PLAYMODE_INHERIT, 0)) {
                npc->mBusy = 0;
                request(ACTION_PITFALL_TALK_RESERVED, ACTION_AFTER_PITFALL_TALK,
                        dAcNpcNml_c::talk_c::getProcSet(
                            dAcNpcNml_c::talk_c::TALK_PROC_ENTRY));
                npc->mAnmSet.setAnm0(ANM_PITFALL_STRUGGLE);
                mStep = 2;
                mInPitfall = 1;
                mTimer0 = 300;
                npc->mAudioObj.startSound(SE_OtoshianaMogaku);
            }
        } else if (anm->checkFrame(20.0f)) {
            npc->setGroundEffect("af_ply_pitfall_sand", "af_ply_pitfall_nml",
                                 NULL, NULL);
        }
    }
}

// .text 10E84
BOOL action_c::escapePitfall(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    int x, z;
    int anm = ANM_PITFALL_ESCAPE;
    int type;
    type = dHmnToolBank_c::getItemToolType(&npc->mTool.mItem);
    switch (type) {
    case HMN_TOOL_ROD:
    case HMN_TOOL_NET:
    case HMN_TOOL_UMBRELLA:
        anm = ANM_PITFALL_ESCAPE_WITH_TOOL;
        break;
    }
    BOOL result = action->requestAnm(PRIORITY_IDLE, anm, 0.0f, cNpcMorphFrames,
                                     m3d::PLAYMODE_INHERIT, 0);
    if (result) {
        npc->mBusy = 1;
        mInPitfall = 0;
        float px = npc->mPos.x;
        float pz = npc->mPos.z;
        x = (int)px >> 5;
        z = (int)pz >> 5;
        dFgobjManager::requestPitfallExit(dNet::getMyNo(), x, z);
        npc->clearPendingUnit();
        npc->mAnmSet.setAnm0(0);
        npc->mMood.mDisabled = 0;
        npc->setGroundEffect("af_ply_pitout_sand", "af_ply_pitout_nml", NULL,
                             NULL);
        npc->mAudioObj.startSound(SE_otoshiana_nukeru);
        npc->mAnmRate = 1.0f;
        return TRUE;
    }
    return FALSE;
}

// .text 10FDC
void action_c::stepPitfall02(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    dAcNpc_c::anm_c *anm = &npc->getModel()->mAnm;
    if (action->mActionId == dAcNpc_c::ACTION_ANM &&
        anm->mAnmId == ANM_PITFALL_STRUGGLE) {
        if (action->mCanChange && !mTimer0) {
            if (escapePitfall(npc)) {
                mStep = 3;
            }
        } else {
            if (anm->checkFrame(0.0f)) {
                npc->mAudioObj.startSound(SE_OtoshianaMogaku);
            }
            float rate = 1.4f;
            anm->setRate(rate);
            npc->mAnmRate = rate;
        }
    }
}

// .text 110D8
BOOL action_c::isApproachAnimationDone(dAcNpcOut_c *npc) {
    if (npc->mAction.mActionId == dAcNpc_c::ACTION_ANM &&
        (npc->getModel()->mAnm.mAnmId == ANM_PITFALL_ESCAPE ||
         npc->getModel()->mAnm.mAnmId == ANM_PITFALL_ESCAPE_WITH_TOOL) &&
        npc->mAction.mCanChange) {
        return TRUE;
    }
    return FALSE;
}

// .text 11164
void action_c::setApproachMood(dAcNpcOut_c *npc) {
    dAnimal_c *animal = npc->mpAnimal;
    int mood = 2;
    if (animal && animal->mID.isValid()) {
        u8 looks = animal->mID.getLooks(1);
        switch (looks) {
        case 3:
        case 0:
            mood = 3;
            break;
        }
    }
    setMood(npc, mood, 1, 1);
}

// .text 1120C
void action_c::stepPitfall03(dAcNpcOut_c *npc) {
    if (isApproachAnimationDone(npc)) {
        npc->halveTalkWeight();
        setApproachMood(npc);
        saveDefaultAction(npc, FALSE);
        restoreAction(npc);
    }
}

const action_c::procFunc sPitfallSteps[] = {
    &action_c::stepPitfall00, &action_c::stepPitfall01,
    &action_c::stepPitfall02, &action_c::stepPitfall03};

// .text 1127C
void action_c::executePitfall(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sPitfallSteps)) {
        (this->*sPitfallSteps[mStep])(npc);
    }
}

// .text 112BC
void action_c::enterPitfallTalkReserved(dAcNpcOut_c *npc) {
}

// .text 112C0
void action_c::executePitfallTalkReserved(dAcNpcOut_c *npc) {
    dAcNpc_c::anm_c *anm = &npc->getModel()->mAnm;
    if (anm->mAnmId == ANM_PITFALL_STRUGGLE) {
        if (anm->checkFrame(0.0f)) {
            npc->mAudioObj.startSound(SE_OtoshianaMogaku);
        }
        float rate = 1.4f;
        anm->setRate(rate);
        npc->mAnmRate = rate;
    }
}

// .text 1136C
void action_c::enterAfterPitfallTalk(dAcNpcOut_c *npc) {
    npc->mTalk.requestTalk(true, 0);
    npc->mBusy = 0;
    npc->offDaubToolChange();
    npc->mApproachActive = 0;
    npc->offDaubWatchFireworks();
}

// .text 113C8
void action_c::stepAfterPitfallTalk00(dAcNpcOut_c *npc) {
    dAcNpc_c::anm_c *anm = &npc->getModel()->mAnm;
    if (anm->mAnmId == ANM_PITFALL_STRUGGLE) {
        if (anm->checkFrame(0.0f)) {
            npc->mAudioObj.startSound(SE_OtoshianaMogaku);
        }
        float rate = 1.4f;
        anm->setRate(rate);
        npc->mAnmRate = rate;
    }
    if (!npc->mTalk.isBusy() && escapePitfall(npc)) {
        ++mStep;
    }
}

// .text 114B0
void action_c::stepAfterPitfallTalk01(dAcNpcOut_c *npc) {
    if (isApproachAnimationDone(npc)) {
        npc->halveTalkWeight();
        setApproachMood(npc);
        dDemo::finishTalk(npc);
        ++mStep;
    }
}

const action_c::procFunc sAfterPitfallTalkSteps[] = {
    &action_c::stepAfterPitfallTalk00, &action_c::stepAfterPitfallTalk01};

// .text 11518
void action_c::executeAfterPitfallTalk(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sAfterPitfallTalkSteps)) {
        (this->*sAfterPitfallTalkSteps[mStep])(npc);
    }
}

// .text 11558
void action_c::enterNetHit(dAcNpcOut_c *npc) {
    if (!npc->mApproachCooldown) {
        npc->clearNetHitCount();
    }
    npc->addNetHitCount();
    npc->mFlea.clear();
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    bool seen = false;
    if (!npc->isNetHitLimit()) {
        const mVec3_c *pos =
            dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
        if (pos &&
            dAcNpc_c::lookAt_c::canSee(npc, *pos, l_lookFov, cNpcLookRange)) {
            seen = true;
        }
    }
    npc->mAction.requestAnm(PRIORITY_IDLE,
                            seen ? ANM_NET_HIT_SEEN : ANM_NET_HIT_UNSEEN, 0.0f,
                            cNpcMorphFrames, m3d::PLAYMODE_INHERIT, 0);
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_FIXED);
    npc->mBusy = 1;
    npc->mMood.mShowEffects = 0;
    npc->mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->stopEmotionManpu();
    npc->mCheckGround = 1;
    npc->mApproachActive = 0;
    npc->offDaubWatchFireworks();
}

// .text 116D8
void action_c::stepNetHit00(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    dAcNpc_c::model_c *model = npc->getModel();
    if (action->mActionId == dAcNpc_c::ACTION_ANM &&
        !(model->mAnm.mAnmId != ANM_NET_HIT_SEEN &&
          model->mAnm.mAnmId != ANM_NET_HIT_UNSEEN)) {
        BOOL limit = npc->isNetHitLimit();
        if (limit) {
            dDemo::requestSpeak(npc, NULL);
            request(ACTION_NET_HIT_TALK_RESERVED, ACTION_AFTER_NET_HIT_TALK,
                    dAcNpcNml_c::talk_c::getProcSet(
                        dAcNpcNml_c::talk_c::TALK_PROC_ETC_HIT));
            npc->clearDemoState();
        }
        if (action->mCanChange) {
            if (limit) {
                mAng angle = npc->getAngleYToPlayer(4);
                action->requestTurn(PRIORITY_IDLE, angle, l_turnSpeed, 0);
                npc->clearNetHitCount();
                mStep = 1;
            } else {
                npc->halveTalkWeight();
                npc->setDemoState2();
                saveDefaultAction(npc, FALSE);
                restoreAction(npc);
                npc->mApproachCooldown = 3600;
            }
        }
    }
}

// .text 1182C
void action_c::stepNetHit01(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    dDemo::requestSpeak(npc, NULL);
    if (action->mActionId == dAcNpc_c::ACTION_TURN && action->mCanChange) {
        action->requestWait(PRIORITY_IDLE);
        mTimer0 = 120;
        mStep = 2;
    }
}

// .text 118AC
void action_c::stepNetHit02(dAcNpcOut_c *npc) {
    if (!mTimer0) {
        npc->setDemoState2();
        saveDefaultAction(npc, FALSE);
        restoreAction(npc);
        npc->mApproachCooldown = 3600;
    } else {
        dDemo::requestSpeak(npc, NULL);
    }
}

const action_c::procFunc sNetHitSteps[] = {
    &action_c::stepNetHit00, &action_c::stepNetHit01, &action_c::stepNetHit02};

// .text 11928
void action_c::executeNetHit(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sNetHitSteps)) {
        (this->*sNetHitSteps[mStep])(npc);
    }
}

// .text 11968
void action_c::enterNetHitTalkReserved(dAcNpcOut_c *npc) {
}

// .text 1196C
void action_c::executeNetHitTalkReserved(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    dAcNpc_c::model_c *model = npc->getModel();
    if (action->mActionId == dAcNpc_c::ACTION_ANM &&
        !(model->mAnm.mAnmId != ANM_NET_HIT_SEEN &&
          model->mAnm.mAnmId != ANM_NET_HIT_UNSEEN)) {
        if (action->mCanChange) {
            mAng angle = npc->getAngleYToPlayer(4);
            action->requestTurn(PRIORITY_REACTION, angle, l_turnSpeed, 0);
        }
    } else if (action->mActionId == dAcNpc_c::ACTION_TURN &&
               action->mCanChange) {
        action->requestWait(PRIORITY_IDLE);
    }
}

// .text 11A44
void action_c::enterAfterNetHitTalk(dAcNpcOut_c *npc) {
    if (npc->mAction.mActionId != dAcNpc_c::ACTION_ANM) {
        u16 angleValue = 0;
        if (npc->mNmlTalk.mpActActor) {
            angleValue = npc->getAngleYTo(npc->mNmlTalk.mpActActor).mAngle;
        }
        mAng angle(angleValue);
        mAng step;
        step.mAngle = (u16)l_turnSpeed.mAngle;
        npc->mTalk.requestTurnTalk(angle, step, 0);
        npc->mBusy = 0;
        npc->offDaubToolChange();
        mStep = 1;
    } else {
        mStep = 0;
    }
    npc->clearNetHitCount();
    npc->mApproachActive = 0;
    npc->offDaubWatchFireworks();
}

// .text 11B00
void action_c::stepAfterNetHitTalk00(dAcNpcOut_c *npc) {
    if (npc->mAction.mCanChange) {
        u16 angleValue = 0;
        if (npc->mNmlTalk.mpActActor) {
            angleValue = npc->getAngleYTo(npc->mNmlTalk.mpActActor).mAngle;
        }
        mAng angle(angleValue);
        mAng step;
        step.mAngle = (u16)l_turnSpeed.mAngle;
        npc->mTalk.requestTurnTalk(angle, step, 0);
        npc->mBusy = 0;
        npc->offDaubToolChange();
        ++mStep;
    }
}

// .text 11B9C
void action_c::stepAfterNetHitTalk01(dAcNpcOut_c *npc) {
    if (!npc->mTalk.isBusy()) {
        npc->halveTalkWeight();
        npc->setDemoState2();
        npc->mMood.mTimerPending = 1;
        dDemo::finishTalk(npc);
        npc->mApproachCooldown = 3600;
        ++mStep;
    }
}

const action_c::procFunc sAfterNetHitTalkSteps[] = {
    &action_c::stepAfterNetHitTalk00, &action_c::stepAfterNetHitTalk01};

// .text 11C14
void action_c::executeAfterNetHitTalk(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sAfterNetHitTalkSteps)) {
        (this->*sAfterNetHitTalkSteps[mStep])(npc);
    }
}

// .text 11C54
void action_c::enterItchy(dAcNpcOut_c *npc) {
    npc->clearNetHitCount();
    npc->mApproachCooldown = 0;
    npc->mFlea.clear();
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    mAng angle = npc->getAngleYToPlayer(4);
    npc->mAction.requestTurn(PRIORITY_IDLE, angle, l_turnSpeed, 0);
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_FIXED);
    npc->mBusy = 1;
    npc->mMood.mShowEffects = 0;
    npc->mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->stopEmotionManpu();
    npc->mCheckGround = 1;
    npc->mApproachActive = 0;
    request(ACTION_FACE_PLAYER, ACTION_AFTER_TALK,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_ETC_FLEA));
    npc->offDaubWatchFireworks();
}

// .text 11DA0
void action_c::stepItchy00(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (npc->mAction.mActionId == dAcNpc_c::ACTION_TURN) {
        npc->clearDemoState();
        dDemo::requestSpeak(npc, NULL);
        if (action->mCanChange) {
            action->requestWait(PRIORITY_IDLE);
            mTimer0 = 3600;
            ++mStep;
        }
    }
}

// .text 11E34
void action_c::stepItchy01(dAcNpcOut_c *npc) {
    if (!mTimer0) {
        npc->setDemoState2();
        saveDefaultAction(npc, FALSE);
        restoreAction(npc);
    } else {
        dDemo::requestSpeak(npc, NULL);
    }
}

const action_c::procFunc sItchySteps[] = {&action_c::stepItchy00,
                                          &action_c::stepItchy01};

// .text 11EA8
void action_c::executeItchy(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sItchySteps)) {
        (this->*sItchySteps[mStep])(npc);
    }
}

// .text 11EE8
void action_c::enterPushed(dAcNpcOut_c *npc) {
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    mAng angle = npc->getAngleYToPlayer(4);
    npc->mAction.requestTurn(PRIORITY_IDLE, angle, l_turnSpeed, 0);
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_FIXED);
    npc->mBusy = 1;
    npc->mMood.mShowEffects = 0;
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->stopEmotionManpu();
    npc->mCheckGround = 1;
    request(ACTION_FACE_PLAYER, ACTION_AFTER_PUSHED_TALK,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_ETC_PUSH));
    npc->clearIdleTimer();
    npc->mApproachActive = 0;
    npc->offDaubWatchFireworks();
}

// .text 12030
void action_c::stepPushed00(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (npc->mAction.mActionId == dAcNpc_c::ACTION_TURN) {
        npc->clearDemoState();
        dDemo::requestSpeak(npc, NULL);
        if (action->mCanChange) {
            action->requestWait(PRIORITY_IDLE);
            mTimer0 = 3600;
            ++mStep;
        }
    }
}

// .text 120C4
void action_c::stepPushed01(dAcNpcOut_c *npc) {
    if (!mTimer0) {
        npc->halveTalkWeight();
        npc->setDemoState2();
        saveDefaultAction(npc, FALSE);
        restoreAction(npc);
    } else {
        dDemo::requestSpeak(npc, NULL);
    }
}

const action_c::procFunc sPushedSteps[] = {&action_c::stepPushed00,
                                           &action_c::stepPushed01};

// .text 12140
void action_c::executePushed(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sPushedSteps)) {
        (this->*sPushedSteps[mStep])(npc);
    }
}

// .text 12180
void action_c::enterAfterPushedTalk(dAcNpcOut_c *npc) {
    u16 angleValue = 0;
    dActor_c *target = npc->mNmlTalk.mpActActor;
    if (target) {
        angleValue = npc->getAngleYTo(target).mAngle;
    }
    mAng angle(angleValue);
    mAng step;
    step.mAngle = (u16)l_turnSpeed.mAngle;
    npc->mTalk.requestTurnTalk(angle, step, 0);
    npc->mBusy = 0;
    npc->offDaubToolChange();
    npc->mApproachActive = 0;
    npc->offDaubWatchFireworks();
}

// .text 12210
void action_c::stepAfterPushedTalk00(dAcNpcOut_c *npc) {
    if (!npc->mTalk.isBusy()) {
        npc->halveTalkWeight();
        npc->setDemoState2();
        npc->mMood.mTimerPending = 1;
        dDemo::finishTalk(npc);
        ++mStep;
    }
}

const action_c::procFunc sAfterPushedTalkSteps[] = {
    &action_c::stepAfterPushedTalk00};

// .text 12280
void action_c::executeAfterPushedTalk(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sAfterPushedTalkSteps)) {
        (this->*sAfterPushedTalkSteps[mStep])(npc);
    }
}

// .text 122C0
void action_c::enterApproach(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    action->requestAnm(PRIORITY_IDLE, ANM_APPROACH_NOTICE, 0.0f,
                       cNpcMorphFrames, m3d::PLAYMODE_INHERIT, 0);
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->offDaubInHouse();
    npc->mCheckGround = 1;
    request(ACTION_FACE_PLAYER, ACTION_AFTER_APPROACH_TALK,
            dAcNpcNml_c::talk_c::getProcSet(
                dAcNpcNml_c::talk_c::TALK_PROC_APPROACH));
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_NORMAL);
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 1;
    npc->offDaubToolChange();
    npc->mMood.mDisabled = 1;
    npc->mMood.mShowEffects = 0;
    mBlockedFrames = 0;
    mStartPos = npc->mPos;
    float duration = cApproachLimitFrames;
    mTimer2 = duration;
    static const u32 emotions[] = {4, 29};
    const emotionData_s *data = getEmotionData(emotions[cM::rndInt(2)]);
    if (data) {
        mVec3_c pos = mVec3_c::Zero, left = mVec3_c::Zero,
                right = mVec3_c::Zero;
        u32 type = data->mStart.mType;
        bool stopOthers = data->mStart.mStopOthers != 0;
        npc->getManpuOfs(&pos, &left, &right, type);
        npc->mManpu.start(type, 207, stopOthers, &pos, &left, &right);
    }
    int index = npc->getNpcIdx();
    dSaveData_c::getRaw()->mAnimals.mTown.halveTalkFlags(index);
    npc->offDaubWatchFireworks();
}

// .text 12500
BOOL action_c::isNearStartPos(dAcNpcOut_c *npc) {
    float range = cApproachHomeDistance;
    mVec3_c diff = mStartPos - npc->mPos;
    return std::fabs(EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z)) <=
           range;
}

// .text 1257C
BOOL action_c::canSeePlayer(dAcNpcOut_c *npc, float distance) {
    if (npc->calcPlayerDistXZ(4, NULL) <= distance &&
        dAcNpc_c::lookAt_c::searchPlayer(dNet::getMyNo(), npc, l_lookFov,
                                         0.0f) != 4) {
        return TRUE;
    }
    return FALSE;
}

// .text 12604
void action_c::stepApproach00(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    dAcNpc_c::model_c *model = npc->getModel();
    if (action->mActionId == dAcNpc_c::ACTION_ANM &&
        model->mAnm.mAnmId == ANM_APPROACH_NOTICE && action->mCanChange) {
        npc->mApproachActive = 1;
        npc->mBusy = 0;
        npc->stopManpu();
        if (canSeePlayer(npc, cApproachNoticeDistance)) {
            action->requestAnm(PRIORITY_IDLE, getApproachAnm(npc), 0.0f,
                               cNpcMorphFrames, m3d::PLAYMODE_INHERIT, 0);
            mStep = 2;
            mTimer0 = cApproachWaitFrames;
            return;
        }
        const mVec3_c *pos =
            dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
        if (pos) {
            mVec3_c diff = npc->mPos - *pos;
            float distance =
                EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
            mAng angle(0);
            u32 range = cApproachTurnDistance;
            BOOL front = npc->checkTargetAngle(&angle, pos, &l_frontAngle);
            if (front && distance > range) {
                action->requestRun(PRIORITY_IDLE, *pos, l_runTurnSpeed, 0,
                                   l_moveParamRun, l_8074FDEC, cNpcMorphFrames);
            } else {
                action->requestTurnRun(PRIORITY_IDLE, *pos, angle, l_turnSpeed,
                                       0, l_moveParamRun, l_8074FDEC,
                                       cNpcMorphFrames);
            }
            float duration = cApproachWaitFrames;
            mTimer0 = duration;
            mStep = 1;

        } else {
            npc->clearTalkWeight();
            npc->mApproachActive = 0;
            saveDefaultAction(npc, FALSE);
            restoreAction(npc);
        }
    }
}

// .text 12838
int action_c::getApproachAnm(dAcNpcOut_c *npc) {
    int anm = ANM_APPROACH_MALE;
    if (npc->mpAnimal && npc->mpAnimal->mID.getGender(1) == GENDER_FEMALE) {
        anm = ANM_APPROACH_FEMALE;
    }
    return anm;
}

// .text 1288C
int action_c::getApproachLoopAnm(dAcNpcOut_c *npc) {
    int anm = ANM_APPROACH_MALE_LOOP;
    if (npc->mpAnimal && npc->mpAnimal->mID.getGender(1) == GENDER_FEMALE) {
        anm = ANM_APPROACH_FEMALE_LOOP;
    }
    return anm;
}

// .text 128E0
void action_c::stepApproach01(dAcNpcOut_c *npc) {
    float speed = npc->mSpeedF;
    if (speed != 0.0f) {
        if ((npc->mAcch.mPrevHitFlags & 0x88) ||
            (npc->mAcch.mHitFlags & 0x88)) {
            if (mBlockedFrames < 0xFFFFFFFF) {
                ++mBlockedFrames;
            }
        } else {
            mBlockedFrames = 0;
        }
    } else {
        mBlockedFrames = 0;
    }
    npc->updateIdleTimer();
    float range = cApproachPursueDistance;
    dAcNpc_c::action_c *action = &npc->mAction;
    const mVec3_c *pos =
        dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (canSeePlayer(npc, range)) {
        action->requestAnm(PRIORITY_IDLE, getApproachAnm(npc), 0.0f,
                           cNpcMorphFrames, m3d::PLAYMODE_INHERIT, 0);
        float duration = cApproachWaitFrames;
        mTimer0 = duration;
        mStep = 2;
    } else if (!tryReturnLostItem(npc)) {
        if (npc->isIdleTimerExpired()) {
            npc->mApproachActive = 0;
            setAction(npc, ACTION_PUSHED);
        } else if (npc->mHitByNet) {
            npc->mApproachActive = 0;
            if (npc->isItchy()) {
                setAction(npc, ACTION_ITCHY);
            } else {
                setAction(npc, ACTION_NET_HIT);
            }
        } else if (!mTimer0 || !mTimer2) {
            npc->halveTalkWeight();
            npc->mApproachActive = 0;
            saveDefaultAction(npc, FALSE);
            restoreAction(npc);
        } else if (stopIfPushed(npc)) {
            npc->halveTalkWeight();
            npc->mApproachActive = 0;
            saveDefaultAction(npc, FALSE);
            restoreAction(npc);
        } else if (isNearStartPos(npc) &&
                   (action->mActionId == dAcNpc_c::ACTION_RUN ||
                    action->mActionId == dAcNpc_c::ACTION_TURN_RUN) &&
                   pos) {
            action->setTargetPos(*pos);
        } else {
            npc->mApproachActive = 0;
            npc->halveTalkWeight();
            saveDefaultAction(npc, FALSE);
            restoreAction(npc);
        }
    }
}

// .text 12B90
void action_c::stepApproach02(dAcNpcOut_c *npc) {
    npc->updateIdleTimer();
    dAcNpc_c::action_c *action = &npc->mAction;
    if (!canSeePlayer(npc, cApproachTalkDistance)) {
        const mVec3_c *pos =
            dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
        if (pos) {
            mVec3_c diff = npc->mPos - *pos;
            float distance =
                EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
            mAng angle(0);
            u32 range = cApproachTurnDistance;
            BOOL isFront = npc->checkTargetAngle(&angle, pos, &l_frontAngle);
            if (isFront && distance > range) {
                action->requestRun(PRIORITY_IDLE, *pos, l_runTurnSpeed, 0,
                                   l_moveParamRun, l_8074FDEC, cNpcMorphFrames);
            } else {
                action->requestTurnRun(PRIORITY_IDLE, *pos, angle, l_turnSpeed,
                                       0, l_moveParamRun, l_8074FDEC,
                                       cNpcMorphFrames);
            }
            mStep = 1;

        } else {
            npc->mApproachActive = 0;
            npc->clearTalkWeight();
            saveDefaultAction(npc, FALSE);
            restoreAction(npc);
        }
    } else {
        if (tryReturnLostItem(npc)) {
            npc->mApproachActive = 0;
        } else if (npc->isIdleTimerExpired()) {
            npc->mApproachActive = 0;
            setAction(npc, ACTION_PUSHED);
        } else if (npc->mHitByNet) {
            npc->mApproachActive = 0;
            if (npc->isItchy()) {
                setAction(npc, ACTION_ITCHY);
            } else {
                setAction(npc, ACTION_NET_HIT);
            }
        } else if (!mTimer0) {
            npc->mApproachActive = 0;
            npc->halveTalkWeight();
            saveDefaultAction(npc, FALSE);
            restoreAction(npc);
        } else if (action->mActionId == dAcNpc_c::ACTION_ANM) {
            int anm = npc->getModel()->mAnm.mAnmId;
            if ((anm == ANM_APPROACH_MALE || anm == ANM_APPROACH_FEMALE) &&
                action->mCanChange) {
                action->requestAnm(PRIORITY_IDLE, getApproachLoopAnm(npc), 0.0f,
                                   cNpcMorphFrames, m3d::PLAYMODE_INHERIT, 0);
            }
        }
    }
}

const action_c::procFunc sApproachSteps[] = {&action_c::stepApproach00,
                                             &action_c::stepApproach01,
                                             &action_c::stepApproach02};

// .text 12E9C
void action_c::executeApproach(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sApproachSteps)) {
        (this->*sApproachSteps[mStep])(npc);
    }
}

// .text 12EDC
void action_c::enterAfterApproachTalk(dAcNpcOut_c *npc) {
    u16 angleValue = 0;
    dActor_c *target = npc->mNmlTalk.mpActActor;
    if (target) {
        angleValue = npc->getAngleYTo(target).mAngle;
    }
    mAng angle(angleValue);
    mAng step;
    step.mAngle = (u16)l_turnSpeed.mAngle;
    npc->mTalk.requestTurnTalk(angle, step, 0);
    npc->mBusy = 0;
    npc->offDaubToolChange();
    npc->offDaubWatchFireworks();
    npc->mMood.mDisabled = 0;
}

// .text 12F6C
void action_c::stepAfterApproachTalk00(dAcNpcOut_c *npc) {
    if (!npc->mTalk.isBusy()) {
        npc->clearTalkWeight();
        npc->setDemoState2();
        npc->mMood.mTimerPending = 1;
        dDemo::finishTalk(npc);
        ++mStep;
    }
}

const action_c::procFunc sAfterApproachTalkSteps[] = {
    &action_c::stepAfterApproachTalk00};

// .text 12FDC
void action_c::executeAfterApproachTalk(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sAfterApproachTalkSteps)) {
        (this->*sAfterApproachTalkSteps[mStep])(npc);
    }
}

// .text 1301C
void action_c::enterGreeting(dAcNpcOut_c *npc) {
    dActor_c *target =
        dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
    mAng angle = 0;
    if (target) {
        angle = npc->getAngleYTo(target);
    }
    npc->mAction.requestTurn(PRIORITY_REACTION, angle, l_turnSpeed, 0);
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_FIXED);
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 1;
    npc->mMood.mShowEffects = 0;
    npc->stopEmotionManpu();
    npc->mCheckGround = 1;
    npc->mApproachActive = 0;
    npc->offDaubToolChange();
    dPlayerMgr::fn_80101350();
    npc->offDaubWatchFireworks();
}

// .text 13160
void action_c::stepGreeting00(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_TURN && action->mCanChange) {
        int anm = npc->getGreetingAnm();
        int sound = dAcNpcOut_c::getGreetingSound(anm);
        action->requestAnm(PRIORITY_REACTION, anm, 0.0f, cNpcMorphFrames,
                           m3d::PLAYMODE_INHERIT, 0);
        npc->mAudioObj.startSound(sound);
        ++mStep;
    }
}

// .text 13210
void action_c::stepGreeting01(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_ANM && action->mCanChange) {
        action->requestWait(PRIORITY_REACTION);
        ++mStep;
    }
}

// .text 13274
void action_c::stepGreeting02(dAcNpcOut_c *npc) {
    if (dPlayerMgr::isWaitingForDemo(4)) {
        if (npc->mpEntry) {
            npc->mpEntry->mFlags.mFlag11 = 1;
        }
        dDemo::finishTalk(npc);
        ++mStep;
    }
}

const action_c::procFunc sGreetingSteps[] = {&action_c::stepGreeting00,
                                             &action_c::stepGreeting01,
                                             &action_c::stepGreeting02};

// .text 132E4
void action_c::executeGreeting(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sGreetingSteps)) {
        (this->*sGreetingSteps[mStep])(npc);
    }
}

// .text 13324
void action_c::enterWatchFireworks(dAcNpcOut_c *npc) {
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    mAng angle = dAcNpcOut_c::randomTurnAngle(&npc->mAngle.y);
    npc->mAction.requestTurn(PRIORITY_IDLE, angle, l_turnSpeed, 0);
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    mStep = 0;
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_NORMAL);
    npc->mBusy = 0;
    request(
        ACTION_FACE_PLAYER, ACTION_AFTER_TALK,
        dAcNpcNml_c::talk_c::getProcSet(dAcNpcNml_c::talk_c::TALK_PROC_ENTRY));
    npc->mMood.mShowEffects = 0;
    npc->stopEmotionManpu();
    npc->mApproachActive = 0;
    npc->mCheckGround = 1;
}

// .text 13460
void action_c::stepWatchFireworks00(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_TURN && action->mCanChange) {
        action->requestWait(PRIORITY_IDLE);
        mTimer0 = cM::rndInt(60) + 30;
        mAng yaw = (s16)abs(cM::rndInt(l_lookYawMax.mAngle >> 2));
        s16 pitchStep = l_lookPitchStep.mAngle;
        mAng pitch = (s16)abs((pitchStep >> 2) + cM::rndInt(pitchStep >> 2));
        s16 yawStep = l_lookYawStep.mAngle;
        mAng speed = (s16)abs((yawStep >> 1) + cM::rndInt(yawStep >> 1));
        npc->mLookAt.setAround(0, l_lookPitchMax, yaw, pitch, speed);
        npc->onDaubWatchFireworks();
        mStep = 1;
    }
}

// .text 13564
void action_c::stepWatchFireworks01(dAcNpcOut_c *npc) {

    bool ready = false;
    if (!mTimer0) {
        u16 pitch = npc->mLookAt.mPitch.mCur.mAngle;
        if ((s16)pitch >= l_lookPitchMax.mAngle) {
            ready = true;
        }
    }
    if (ready) {
        npc->mAction.requestEmotion(PRIORITY_IDLE, 26, cNpcMorphFrames);
        mTimer0 = cM::rndInt(120) + 180;
        mStep = 2;
    }
}

// .text 135F0
void action_c::stepWatchFireworks02(dAcNpcOut_c *npc) {
    if (npc->mAction.mActionId == dAcNpc_c::ACTION_EMOTION && !mTimer0) {
        npc->stopEmotionManpu();
        npc->offDaubWatchFireworks();
        saveDefaultAction(npc, FALSE);
        restoreAction(npc);
    }
}

const action_c::procFunc sWatchFireworksSteps[] = {
    &action_c::stepWatchFireworks00, &action_c::stepWatchFireworks01,
    &action_c::stepWatchFireworks02};

// .text 13668
void action_c::executeWatchFireworks(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sWatchFireworksSteps)) {
        (this->*sWatchFireworksSteps[mStep])(npc);
    }
}

// .text 136A8
void action_c::enterRemoteWatchFireworks(dAcNpcOut_c *npc) {
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->mBusy = 0;
    npc->mAcceptMoveReq = 1;
    mAng yaw = (s16)abs(cM::rndInt(l_lookYawMax.mAngle >> 2));
    s16 pitchStep = l_lookPitchStep.mAngle;
    mAng pitch = (s16)abs((pitchStep >> 2) + cM::rndInt(pitchStep >> 2));
    s16 yawStep = l_lookYawStep.mAngle;
    mAng speed = (s16)abs((yawStep >> 1) + cM::rndInt(yawStep >> 1));
    npc->mLookAt.setAround(0, l_lookPitchMax, yaw, pitch, speed);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_NORMAL);
    request(
        ACTION_FACE_PLAYER, ACTION_AFTER_TALK,
        dAcNpcNml_c::talk_c::getProcSet(dAcNpcNml_c::talk_c::TALK_PROC_ENTRY));
    npc->mFlea.clear();
    npc->mApproachActive = 0;
}

// .text 137F8
void action_c::executeRemoteWatchFireworks(dAcNpcOut_c *npc) {
    if (npc->isLocalOwner()) {
        if (npc->isPermitSelf()) {
            npc->fn_80013474();
            startTalk(npc);
        } else if (npc->checkLocalOwner()) {
            npc->fn_800134A4();
            saveDefaultAction(npc, FALSE);
            restoreAction(npc);
        }
    } else {

        if (isRemoteToolReady(npc)) {
            if (npc->isDaubToolChange()) {
                setAction(npc, ACTION_REMOTE_TOOL_CHANGE);
            } else if (npc->isDaubToolChanged()) {
                npc->syncTool();
            }
        } else if (shouldChangeEventMode(npc)) {
            if (npc->isDaubMaskChange()) {
                setAction(npc, ACTION_REMOTE_MASK_CHANGE);
            } else {
                npc->changeMask();
            }
        } else if (npc->isClothChangeReady()) {
            if (npc->isDaubClothChange()) {
                setAction(npc, ACTION_REMOTE_CLOTH_CHANGE);
            } else if (npc->isDaubClothChanged()) {
                npc->changeClothModel();
            }
        } else if (npc->isDaubInHouse()) {
            setAction(npc, ACTION_REMOTE_HOUSE_WAIT);
        } else if (!npc->isDaubWatchFireworks()) {
            saveHouseAction(npc);
            restoreAction(npc);
        } else {
            dAcNpc_c::action_c *action = &npc->mAction;
            if (action->mActionId != dAcNpc_c::ACTION_MOVE_PARAM) {
                u16 pitch = npc->mLookAt.mPitch.mCur.mAngle;
                if ((s16)pitch >= l_lookPitchMax.mAngle) {
                    mVec3_c pos = npc->mPos;
                    mAng angle = npc->mAngle.y;
                    npc->getDaubPos(&pos, &angle);
                    npc->mAcceptMoveReq = 0;
                    action->requestMoveParam(PRIORITY_IDLE, ANM_WATCH_FIREWORKS,
                                             pos, l_moveParamWalk,
                                             m3d::PLAYMODE_INHERIT, 0.1f, 0.0f,
                                             cNpcMorphFrames);
                }
            } else {
                mVec3_c pos = npc->mPos;
                mAng angle = npc->mAngle.y;
                npc->getDaubPos(&pos, &angle);
                action->setTargetPos(pos);
                action->mTurnDelta = dAcNpc_c::move_c::chaseAngle(
                    &npc->mAngle.y, &angle, &l_turnSpeed, 0);
                dAcNpc_c::anm_c *anm = &npc->getModel()->mAnm;
                if (anm->mAnmId == ANM_WATCH_FIREWORKS &&
                    anm->checkFrame(0.0f)) {
                    npc->mAudioObj.startSound(SE_manpu_26);
                }
            }
        }
    }
}

// .text 13B1C
void action_c::enterRequestCatch(dAcNpcOut_c *npc) {
    dActor_c *target =
        dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
    mAng angle = 0;
    if (target) {
        angle = npc->getAngleYTo(target);
    }
    npc->mAction.requestTurn(PRIORITY_REACTION, angle, l_turnSpeed, 0);
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_FIXED);
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 1;
    npc->mMood.mShowEffects = 0;
    npc->stopEmotionManpu();
    npc->mCheckGround = 1;
    npc->mApproachActive = 0;
    npc->offDaubToolChange();
    dPlayerMgr::fn_80101350();
    npc->offDaubWatchFireworks();
    npc->mLostItemComplete = 1;
}

// .text 13C64
void action_c::stepRequestCatch00(dAcNpcOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_TURN && action->mCanChange) {
        action->requestEmotion(PRIORITY_IDLE, 26, cNpcMorphFrames);
        mTimer0 = cM::rndInt(120) + 180;
        ++mStep;
    }
}

// .text 13CDC
void action_c::stepRequestCatch01(dAcNpcOut_c *npc) {
    if (!dPlayerMgr::isActionItemState() ||
        (npc->mAction.mActionId == dAcNpc_c::ACTION_EMOTION && !mTimer0)) {
        npc->stopEmotionManpu();
        saveDefaultAction(npc, FALSE);
        restoreAction(npc);
    }
}

const action_c::procFunc sRequestCatchSteps[] = {&action_c::stepRequestCatch00,
                                                 &action_c::stepRequestCatch01};

// .text 13D58
void action_c::executeRequestCatch(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sRequestCatchSteps)) {
        (this->*sRequestCatchSteps[mStep])(npc);
    }
}

// .text 13D98
void action_c::enterRemoteClothChange(dAcNpcOut_c *npc) {
    if (!npc->mIsActive) {
        npc->mIsActive = 1;
        npc->mLastPos = npc->mPos;
    }
    npc->mAcceptMoveReq = 0;
    npc->mAction.requestChangeCloth2(PRIORITY_IDLE, npc->mpAnimal->mCloth,
                                     (int)&npc->mpAnimal->mClothDesign);
    npc->mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
    insectLink_s *insectLink = &npc->mInsectLink;
    if (insectLink->mpInsect) {
        dInsectField::removeNpcLink(insectLink->mpInsect, insectLink);
    }
    dFishLink_c *fishLink = &npc->mFishLink;
    if (fishLink->mFish) {
        fishLink->mFish->removeLink(fishLink);
    }
    npc->mBusy = 1;
    npc->mMood.startTimer(0, 0);
    npc->mMood.mTimerPending = 1;
    npc->mFlea.clear();
    npc->mApproachActive = 0;
}

// .text 13E84
void action_c::stepRemoteClothChange00(dAcNpcOut_c *npc) {
    if (npc->mAction.mActionId == dAcNpc_c::ACTION_CHANGE_CLOTH2 &&
        npc->mAction.mCanChange) {
        saveHouseAction(npc);
        restoreAction(npc);
    }
}

const action_c::procFunc sRemoteClothChangeSteps[] = {
    &action_c::stepRemoteClothChange00};

// .text 13EE0
void action_c::executeRemoteClothChange(dAcNpcOut_c *npc) {
    if (mStep < ARRAY_SIZE(sRemoteClothChangeSteps)) {
        (this->*sRemoteClothChangeSteps[mStep])(npc);
    }
}

typedef char CheckOutActionSize[sizeof(action_c) == 0x48 ? 1 : -1];
} // namespace dANpcOut

typedef char CheckNpcOutSize[sizeof(dAcNpcOut_c) == 0x2C50 ? 1 : -1];
typedef char CheckNpcOutMode[offsetof(dAcNpcOut_c, mMode) == 0x219C ? 1 : -1];
typedef char CheckNpcOutRoute[offsetof(dAcNpcOut_c, mRoute) == 0x2AD8 ? 1 : -1];

namespace dANpcOut {
extern const float cPitfallMoveSpeed = 1.5f;
extern const float cPitfallMoveAccel = 0.2f;

// Retained in the original rodata; no code references have been found.
#ifdef MUST_MATCH
#pragma force_active on
extern const int sUnusedCounts[] = {2, 2, 2};
extern const float sUnusedDistances[] = {48.0f, 48.0f, 48.0f, 48.0f, 48.0f};
extern const int sUnusedCount = 6;
#pragma force_active reset
#endif
} // namespace dANpcOut
