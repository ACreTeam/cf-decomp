#include <game/game/d_a_npc_town_out.hpp>
#include <game/framework/f_profile.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_hmn_tool_mng.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_weather.hpp>

namespace dANpcTownOut {
// .text 5C
BASE_PROFILE(A_NPC_TOWN_OUT, dAcNpcTownOut_c);

// dAcNpc_c::action_c request priorities.
enum Priority_e { PRIORITY_IDLE, PRIORITY_DEMO };
// dAcNpc_c::objc_c weights.
enum { COLLISION_WEIGHT_NORMAL = 0x80 };
} // namespace dANpcTownOut

using namespace dANpcTownOut;

// .text 1E0
bool dAcNpcTownOut_c::canInteract(dDemoActor_c *actor) {
    if (!dAcNpcNml_c::canInteract(actor)) {
        return false;
    }
    if (actor == NULL) {
        return false;
    }
    return mTalk.isBusy() == false;
}

// .text 248
void dAcNpcTownOut_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        mTownAction.setAction(this, dANpcTownOut::action_c::ACTION_FACE_PLAYER);
        break;
    case DEMO_CMD_TALK: {
        const talk_c::procSet_s *set =
            talk_c::getProcSet(talk_c::TALK_PROC_TOWN);
        mNmlTalk.start(
            this,
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT),
            set);
        mTownAction.setAction(this, dANpcTownOut::action_c::ACTION_TALK);
        break;
    }
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        mTownAction.setAction(this, dANpcTownOut::action_c::ACTION_WANDER);
        break;
    }
    dAcNpc_c::onDemoCmd(cmd, member);
}

// .text 320
int dAcNpcTownOut_c::preCreate() {
    if (!dAcNpcTown_c::preCreate()) {
        return 0;
    }
    mTool.create(this);
    return 1;
}

// .text 36C
int dAcNpcTownOut_c::create() {
    if (!dAcNpcNml_c::create()) {
        return 0;
    }
    if (!mTool.loadNext(this)) {
        return 0;
    }
    takeInitialTool();
    mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep, cNpcLookRange);
    mTownAction.setAction(this, dANpcTownOut::action_c::ACTION_WANDER);
    return 1;
}

// .text 408
int dAcNpcTownOut_c::doDelete() {
    if (!mTool.remove()) {
        return 0;
    }
    if (!dAcNpcNml_c::doDelete()) {
        return 0;
    }
    return 1;
}

// .text 45C
int dAcNpcTownOut_c::vtB0() {
    mTownAction.execute(this);
    return 1;
}

// .text 488
const dItem::Item *dAcNpcTownOut_c::getHoldItem() {
    dAnimal_c *animal = mpAnimal;
    if (animal == NULL) {
        return NULL;
    }
    return &animal->mHeldItem;
}

// .text 4A4
BOOL dAcNpcTownOut_c::takeInitialTool() {
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

// .text 560
BOOL dAcNpcTownOut_c::updateUmbrella() {
    dAnimal_c *animal = mpAnimal;
    if (animal == NULL) {
        return FALSE;
    }
    dItem::Item *held = &animal->mHeldItem;
    bool changed = !held->isSame(mTool.mItem);
    if (changed) {
        return FALSE;
    }
    if (cM::rndInt(60) != 0) {
        return FALSE;
    }
    BOOL rain = FALSE;
    dWeather_c *weather = dWeather_c::sInstance;
    if (weather != NULL && dWeather_c::isRainKind(weather->mWeather)) {
        rain = TRUE;
    }
    if (held->mId != dItem::ITEM_ID_NONE) {
        if (!rain) {
            animal->setHeldItem(&dItem::Item());
            return TRUE;
        }
    } else if (rain) {
        dItem::Item umbrella = animal->getUmbrella();
        animal->setHeldItem(&umbrella);
        return TRUE;
    }
    return FALSE;
}

// .text 678
dAcNpcTownOut_c::toolMng_c::toolMng_c() {
    mItem = dItem::ITEM_ID_NONE;
    mNextItem = dItem::ITEM_ID_NONE;
    mLoadState = LOAD_PENDING;
}

// .text 6CC
dAcNpcTownOut_c::toolMng_c::~toolMng_c() {
}

// .text 724
void dAcNpcTownOut_c::toolMng_c::create(dAcNpcTownOut_c *npc) {
    if (mBank.getType() == dHmnToolMng_c::SLOT_NUM) {
        mBank.create(npc->mNpcListSlot + dHmnToolMng_c::PLAYER_SLOT_NUM);
        checkItem(npc);
    }
}

// .text 780
bool dAcNpcTownOut_c::toolMng_c::remove() {
    if (!mBank.syncLoad()) {
        return false;
    }
    mItem = dItem::ITEM_ID_NONE;
    mNextItem = dItem::ITEM_ID_NONE;
    mLoadState = LOAD_PENDING;
    mBank.remove();
    return true;
}

namespace dANpcTownOut {
// .text 7E4
action_c::action_c() {
    mLookTimer = 0;
    mBlockedFrames = 0;
    mAction = ACTION_NONE;
    mpProcs = NULL;
    mStep = 0;
    mWaitTimer = 0;
}

// .text 808
action_c::~action_c() {
}

static const action_c::procPair_s l_actions[] = {
    {&action_c::enterWander, &action_c::executeWander},
    {&action_c::enterTalk, &action_c::executeTalk},
    {&action_c::enterFacePlayer, &action_c::executeFacePlayer},
    {&action_c::enterToolChange, &action_c::executeToolChange},
};
const u32 action_c::cWaitTimeBase = 120;
const u32 action_c::cWaitTimeRange = 600;

// .text 848
void action_c::setAction(dAcNpcTownOut_c *npc, u32 action) {
    if (action < ACTION_NUM) {
        mAction = action;
        mpProcs = &l_actions[action];
        mStep = 0;
        if (mpProcs != NULL) {
            (this->*mpProcs->enter)(npc);
        }
    }
}

// .text 898
void action_c::execute(dAcNpcTownOut_c *npc) {
    if (mWaitTimer != 0) {
        mWaitTimer--;
    }
    if (mpProcs != NULL) {
        (this->*mpProcs->execute)(npc);
    }
}

// .text 8E0
void action_c::enterWander(dAcNpcTownOut_c *npc) {
    mWaitTimer = cM::rndInt(cWaitTimeRange) + cWaitTimeBase;
    npc->mAction.requestWait(PRIORITY_IDLE);
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_NORMAL);
    mLookTimer = cWaitTimeBase + cM::rndF(cWaitTimeRange);
    npc->mBusy = 0;
    mBlockedFrames = 0;
}

// .text 970
BOOL action_c::checkDetour(dAcNpcTownOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    BOOL stopped = FALSE;
    if (!action->isArrived(npc, TRUE)) {
        mVec3_c pos;
        switch (npc->getSidePos(&pos)) {
        case dAcNpc_c::SIDE_POS_BLOCKED:
            action->requestWait(PRIORITY_IDLE);
            stopped = TRUE;
            break;
        case dAcNpc_c::SIDE_POS_DETOUR:
            action->setGoalPos(pos);
            break;
        }
    } else if (action->isDetour()) {
        action->resetGoal();
    }
    return stopped;
}

// .text A30
BOOL action_c::checkBlocked(dAcNpcTownOut_c *npc) {
    if (npc->getSpeedF() != 0.0f) {
        BOOL blocked = FALSE;
        if (mBlockedFrames > 120) {
            blocked = TRUE;
        }
        if (npc->mPushTimer > 120) {
            blocked = TRUE;
        }
        if (blocked) {
            npc->mAction.requestWait(PRIORITY_IDLE);
            npc->mPushTimer = 0;
            mBlockedFrames = 0;
            return TRUE;
        }
    }
    return FALSE;
}

// .text AD0
BOOL action_c::shouldChangeTool(dAcNpcTownOut_c *npc) {
    if (!npc->mpAnimal) {
        return FALSE;
    }
    const dItem::Item *item;
    dAcNpcTownOut_c::toolMng_c *tool = &npc->mTool;
    item = &npc->mpAnimal->mHeldItem;
    bool changed = !tool->mItem.isSame(*item);
    if (changed &&
        (item->mId == dItem::ITEM_ID_NONE || tool->isNextLoaded(npc))) {
        return TRUE;
    }
    return FALSE;
}

// .text B68
BOOL action_c::tryIdleAction(dAcNpcTownOut_c *npc) {
    if (shouldChangeTool(npc)) {
        setAction(npc, ACTION_TOOL_CHANGE);
        return TRUE;
    }
    if (npc->getSpeedF() != 0.0f) {
        if (checkDetour(npc)) {
            return TRUE;
        }
        if (checkBlocked(npc)) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text C10
void action_c::executeWander(dAcNpcTownOut_c *npc) {
    const mVec3_c *cur;
    int yawStep;
    BOOL found;
    if (npc->getSpeedF() != 0.0f) {
        if (npc->mAcch.mHitFlags &
            (dBGCF::HIT_WALL_FRONT | dBGCF::HIT_STEP_EXT_FRONT)) {
            if (mBlockedFrames < 0xFFFFFFFF) {
                mBlockedFrames++;
            }
        } else {
            mBlockedFrames = 0;
        }
    } else {
        mBlockedFrames = 0;
    }
    if (tryIdleAction(npc)) {
        return;
    }
    npc->updateUmbrella();
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mCanChange &&
        (action->mActionId != dAcNpc_c::ACTION_WAIT || !mWaitTimer)) {
        mVec3_c pos = npc->mPos;
        cur = &npc->mPos;
        found = FALSE;
        if (cM::rndInt(5) < 3) {
            if (dAcNpc_c::searchPosInRect(&pos, cur, cCityMinX, cCityMinZ,
                                          cCityMaxX, cCityMaxZ, l_8074FE14,
                                          l_8074FE10)) {
                found = TRUE;
            } else if (cur->x <= cCityMinX || cur->x >= cCityMaxX ||
                       cur->z <= cCityMinZ || cur->z >= cCityMaxZ) {
                if (dAcNpc_c::searchPosNear(&pos, cur, l_8074FE14,
                                            l_8074FE10)) {
                    found = TRUE;
                }
            }
        }
        if (found) {
            mAng angle(0);
            if (npc->checkTargetAngle(&angle, &pos, &l_frontAngle)) {
                action->requestWalk(PRIORITY_IDLE, pos, l_defaultAnmRate,
                                    l_walkTurnSpeed, 0, l_moveParamWalk,
                                    cNpcMorphFrames);
            } else {
                action->requestTurnWalk(PRIORITY_IDLE, pos, angle, l_turnSpeed,
                                        0, l_moveParamWalk, l_defaultAnmRate,
                                        cNpcMorphFrames);
            }
        } else {
            mWaitTimer = cM::rndInt(cWaitTimeRange) + cWaitTimeBase;
            if (action->mActionId != dAcNpc_c::ACTION_WAIT) {
                action->requestWait(PRIORITY_IDLE);
            }
        }
    }
    if (mLookTimer != 0) {
        mLookTimer--;
    }
    if (mLookTimer == 0) {
        dAcNpc_c::lookAt_c *look = &npc->mLookAt;
        if (look->mType != dAcNpc_c::lookAt_c::LOOK_AROUND) {
            yawStep = l_lookYawStep;
            npc->mLookAt.setAround(0, mAng(cM::rndInt(l_lookPitchMax >> 1)),
                                   mAng(l_lookYawMax >> 1), l_lookPitchStep,
                                   mAng((yawStep >> 1) + cM::rndInt(yawStep)));
        } else {
            look->setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                            cNpcLookRange);
        }
        mLookTimer = cWaitTimeBase + cM::rndF(cWaitTimeRange);
    }
}

// .text F6C
void action_c::enterTalk(dAcNpcTownOut_c *npc) {
    u16 angleValue = 0;
    dActor_c *actor = npc->mNmlTalk.mpActActor;
    if (actor) {
        angleValue = npc->getAngleYTo(actor).mAngle;
    }
    mAng angle(angleValue);
    mAng step;
    step.mAngle = (u16)l_turnSpeed.mAngle;
    npc->mTalk.requestTurnTalk(angle, step, 0);
    npc->mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
}

// .text 1000
void action_c::stepTalk00(dAcNpcTownOut_c *npc) {
    if (!npc->mTalk.isBusy()) {
        dDemo::finishTalk(npc);
        ++mStep;
    }
}

static const action_c::procFunc l_talkSteps[] = {&action_c::stepTalk00};

// .text 1058
void action_c::executeTalk(dAcNpcTownOut_c *npc) {
    if (mStep < ARRAY_SIZE(l_talkSteps)) {
        (this->*l_talkSteps[mStep])(npc);
    }
}

// .text 1098
void action_c::enterFacePlayer(dAcNpcTownOut_c *npc) {
    mAng angle = npc->getAngleYToPlayer(dPlayerMgr::PLAYER_INDEX_CURRENT);
    npc->mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0);
}

// .text 10F8
void action_c::stepFacePlayer00(dAcNpcTownOut_c *npc) {
    dAcNpc_c::action_c *action = &npc->mAction;
    if (action->mActionId == dAcNpc_c::ACTION_TURN && action->mCanChange) {
        action->requestWait(PRIORITY_DEMO);
        ++mStep;
    }
}

static const action_c::procFunc l_facePlayerSteps[] = {
    &action_c::stepFacePlayer00};

// .text 115C
void action_c::executeFacePlayer(dAcNpcTownOut_c *npc) {
    if (mStep < ARRAY_SIZE(l_facePlayerSteps)) {
        (this->*l_facePlayerSteps[mStep])(npc);
    }
}

// .text 119C
void action_c::initToolChange(dAcNpcTownOut_c *npc, u8 putTool,
                              u8 closeUmbrellaStep, u8 takeTool,
                              u8 openUmbrellaStep) {
    dAcNpc_c::toolBase_c *tool = &npc->mTool;
    dItem::Item *item = &npc->mTool.mItem;
    int anm = HMN_ANM_TOOL_PUTAWAY1;
    if (item->mId != dItem::ITEM_ID_NONE) {
        int type = dHmnToolBank_c::getItemToolType(item);
        f32 morph = 18.0f;
        int mode = m3d::FORWARD_ONCE;
        switch (type) {
        case HMN_TOOL_UMBRELLA:
            mStep = closeUmbrellaStep;
            anm = HMN_ANM_UMB_OPEN1;
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
            anm = HMN_ANM_UMB_OPEN1;
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
    npc->mBusy = 1;
}

// .text 12E0
void action_c::enterToolChange(dAcNpcTownOut_c *npc) {
    initToolChange(npc, TOOL_PUT_AWAY, TOOL_CLOSE_UMBRELLA, TOOL_TAKE_OUT,
                   TOOL_OPEN_UMBRELLA);
    npc->mObjc.setDefWeight(COLLISION_WEIGHT_NORMAL);
}

// .text 1328
BOOL action_c::takeOutTool(dAcNpcTownOut_c *npc) {
    dAcNpc_c::toolBase_c *tool;
    dAcNpc_c::model_c *model = npc->getModel();
    dAcNpc_c::anm_c *anm = &model->mAnm;
    if (anm->mAnmId == HMN_ANM_TOOL_PUTAWAY1 && anm->checkFrame(18.0f)) {
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

// .text 1410
void action_c::stepToolChange00(dAcNpcTownOut_c *npc) {
    if (takeOutTool(npc)) {
        mStep = TOOL_SCALE_OUT;
    }
}

// .text 144C
BOOL action_c::scaleOutTool(dAcNpcTownOut_c *npc) {
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
        return TRUE;
    }
    return FALSE;
}

// .text 14E8
void action_c::stepToolChange01(dAcNpcTownOut_c *npc) {
    if (scaleOutTool(npc)) {
        setAction(npc, ACTION_WANDER);
    }
}

// .text 1538
BOOL action_c::openUmbrella(dAcNpcTownOut_c *npc) {
    dAcNpc_c::model_c *model = npc->getModel();
    dAcNpc_c::anm_c *anm = &model->mAnm;
    if (anm->mAnmId == HMN_ANM_UMB_OPEN1) {
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

// .text 1600
void action_c::stepToolChange02(dAcNpcTownOut_c *npc) {
    if (openUmbrella(npc)) {
        mStep = TOOL_FINISH_OPEN_UMBRELLA;
    }
}

// .text 163C
BOOL action_c::finishOpeningUmbrella(dAcNpcTownOut_c *npc) {
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

// .text 16CC
void action_c::stepToolChange03(dAcNpcTownOut_c *npc) {
    if (finishOpeningUmbrella(npc)) {
        setAction(npc, ACTION_WANDER);
    }
}

// .text 171C
BOOL action_c::putAwayTool(dAcNpcTownOut_c *npc) {
    dAcNpc_c::model_c *model = npc->getModel();
    dAcNpc_c::anm_c *anm = &model->mAnm;
    if (anm->mAnmId == HMN_ANM_TOOL_PUTAWAY1) {
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

// .text 17EC
void action_c::stepToolChange04(dAcNpcTownOut_c *npc) {
    if (putAwayTool(npc)) {
        mStep = TOOL_TAKE_NEXT;
    }
}

// .text 1828
BOOL action_c::takeNextTool(dAcNpcTownOut_c *npc, u8 toolStep,
                            u8 umbrellaStep) {
    dAcNpc_c::toolBase_c *tool;
    const dItem::Item *next;
    dAcNpc_c::model_c *model = npc->getModel();
    dAcNpc_c::anm_c *anm = &model->mAnm;
    tool = &npc->mTool;
    next = &tool->mNextItem;
    if (anm->mAnmId == HMN_ANM_TOOL_PUTAWAY1 && anm->checkFrame(42.0f) &&
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
            npc->mAction.requestAnm(PRIORITY_IDLE, HMN_ANM_UMB_OPEN1, 0.0f,
                                    cNpcMorphFrames, m3d::FORWARD_ONCE, 0);
            tool->mLock = true;
            mStep = umbrellaStep;
        } else {
            return TRUE;
        }
    }
    return FALSE;
}

// .text 19A4
void action_c::stepToolChange05(dAcNpcTownOut_c *npc) {
    if (takeNextTool(npc, TOOL_SCALE_OUT, TOOL_OPEN_UMBRELLA)) {
        setAction(npc, ACTION_WANDER);
    }
}

// .text 19FC
BOOL action_c::closeUmbrella(dAcNpcTownOut_c *npc) {
    dAcNpc_c::model_c *model = npc->getModel();
    dAcNpc_c::anm_c *anm = &model->mAnm;
    if (model->mAnm.mAnmId == HMN_ANM_UMB_OPEN1) {
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

// .text 1A9C
void action_c::stepToolChange06(dAcNpcTownOut_c *npc) {
    if (closeUmbrella(npc)) {
        mStep = TOOL_FINISH_CHANGE;
    }
}

// .text 1AD8
BOOL action_c::finishToolChange(dAcNpcTownOut_c *npc, u8 putTool,
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

// .text 1B70
void action_c::stepToolChange07(dAcNpcTownOut_c *npc) {
    if (finishToolChange(npc, TOOL_PUT_AWAY, TOOL_CLOSE_UMBRELLA, TOOL_TAKE_OUT,
                         TOOL_OPEN_UMBRELLA)) {
        setAction(npc, ACTION_WANDER);
    }
}

static const action_c::procFunc l_toolChangeSteps[] = {
    &action_c::stepToolChange00, &action_c::stepToolChange01,
    &action_c::stepToolChange02, &action_c::stepToolChange03,
    &action_c::stepToolChange04, &action_c::stepToolChange05,
    &action_c::stepToolChange06, &action_c::stepToolChange07};

// .text 1BD0
void action_c::executeToolChange(dAcNpcTownOut_c *npc) {
    if (mStep < ARRAY_SIZE(l_toolChangeSteps)) {
        (this->*l_toolChangeSteps[mStep])(npc);
    }
}
} // namespace dANpcTownOut
