#include <game/game/d_a_npc_cafe.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_a_rmobj_cafe.hpp>


namespace dANpcCafe {
// .text 0x5C
CUSTOM_BASE_PROFILE(A_NPC_CAFE, dAcNpcCafe_c, 0x54, 0x90);
} // namespace dANpcCafe

// .text 0x1D8
bool dAcNpcCafe_c::canInteract(dDemoActor_c *actor) {
    return dAcNpcNml_c::canInteract(actor) != FALSE;
}
// .text 0x204
void dAcNpcCafe_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case 4:
        mState.change(this, dANpcCafe::State_c::STATE_LOOK);
        break;
    case 1: {
        const talk_c::procSet_s *set = talk_c::getProcSet(0);
        mNmlTalk.start(this, dPlayerMgr::getNpcTargetActor(4), set);
        mState.change(this, dANpcCafe::State_c::STATE_TALK);
        break;
    }
    case 6:
    case 9:
        mState.change(this, dANpcCafe::State_c::STATE_IDLE);
        break;
    }
    dAcNpc_c::onDemoCmd(cmd, member);
}
// .text 0x2DC
int dAcNpcCafe_c::preCreate() {
    clearNetSync();
    if (!dAcNpcNml_c::preCreate()) {
        return 0;
    }
    anmSet_c *anm = &mAnmSet;
    anm->setAnm0(0x31);
    anm->setAnm1(0x31);
    anm->setAnm2(0x31);
    return 1;
}
// .text 0x34C
int dAcNpcCafe_c::create() {
    if (!dAcNpcNml_c::create()) {
        return 0;
    }
    mState.change(this, dANpcCafe::State_c::STATE_IDLE);
    mObjc.setDefWeight(0xFF);
    dARmobjCafe::g_npcCreated = 1;
    return 1;
}
// .text 0x3B4
int dAcNpcCafe_c::doDelete() {
    return dAcNpcNml_c::doDelete() != 0;
}
// .text 0x3E0
int dAcNpcCafe_c::vtB0() {
    mState.execute(this);
    return 1;
}

namespace dANpcCafe {
// .text 0x40C
State_c::State_c() {
    mState = STATE_NONE;
    mpState = NULL;
    mStep = STEP_START;
}
// .text 0x424
State_c::~State_c() {
}
static const State_c::State_s l_states[] = {
    {&State_c::initIdle, &State_c::executeIdle},
    {&State_c::initTalk, &State_c::executeTalk},
    {&State_c::initLook, &State_c::executeLook}};
// .text 0x464
void State_c::change(dAcNpcCafe_c *npc, u32 state) {
    if (state < STATE_COUNT) {
        mState = state;
        mpState = &l_states[state];
        mStep = STEP_START;
        if (mpState != NULL) {
            (this->*mpState->mInit)(npc);
        }
    }
}
// .text 0x4B4
void State_c::execute(dAcNpcCafe_c *npc) {
    if (mpState != NULL) {
        (this->*mpState->mExecute)(npc);
    }
}
// .text 0x4E8
void State_c::initIdle(dAcNpcCafe_c *npc) {
    npc->mAction.requestWait(0);
    npc->mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
    npc->mMood.mShowEffects = 0;
    npc->mBusy = 0;
}
// .text 0x54C
void State_c::executeIdle(dAcNpcCafe_c *) {
}
// .text 0x550
void State_c::initTalk(dAcNpcCafe_c *npc) {
    npc->mLookAt.setPlayerNo(0, dNet::getMyNo(), false, l_lookPitchStep,
                             l_lookYawStep, cNpcLookRange);
    npc->mTalk.requestTalk(true, 0);
}
// .text 0x5B8
void State_c::stepTalk(dAcNpcCafe_c *npc) {
    if (!npc->mTalk.isBusy()) {
        dDemo::finishTalk(npc);
        ++mStep;
    }
}
static const State_c::Proc l_talkSteps[] = {&State_c::stepTalk};
// .text 0x610
void State_c::executeTalk(dAcNpcCafe_c *npc) {
    if (mStep < STEP_DONE) {
        (this->*l_talkSteps[mStep])(npc);
    }
}
// .text 0x650
void State_c::initLook(dAcNpcCafe_c *npc) {
    npc->mLookAt.setPlayer(0, false, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
}
// .text 0x678
void State_c::executeLook(dAcNpcCafe_c *) {
}
} // namespace dANpcCafe
