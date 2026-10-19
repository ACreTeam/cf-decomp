// The theater's guest villager. Functions are in target .text order.
#include <game/game/d_a_npc_town_theater.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_player_mgr.hpp>

namespace dANpcTownTheater {
// .text 0x5C
BASE_PROFILE(A_NPC_TOWN_THEATER, dAcNpcTownTheater_c);
} // namespace dANpcTownTheater

// .text 0x1D8
bool dAcNpcTownTheater_c::canInteract(dDemoActor_c *actor) {
    if (!dAcNpcNml_c::canInteract(actor)) {
        return false;
    }
    if (actor == NULL) {
        return false;
    }
    return !mTalk.isBusy();
}

// .text 0x240
void dAcNpcTownTheater_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        mState.change(this, dANpcTownTheater::State_c::STATE_LOOK);
        break;
    case DEMO_CMD_TALK: {
        const talk_c::procSet_s *set =
            talk_c::getProcSet(talk_c::TALK_PROC_TOWN_THEATER);
        mNmlTalk.start(
            this,
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT),
            set);
        mState.change(this, dANpcTownTheater::State_c::STATE_TALK);
        break;
    }
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        mState.change(this, dANpcTownTheater::State_c::STATE_IDLE);
        break;
    }
    dAcNpc_c::onDemoCmd(cmd, member);
}

// .text 0x318
int dAcNpcTownTheater_c::preCreate() {
    if (!dAcNpcTown_c::preCreate()) {
        return 0;
    }
    anmSet_c *anm = &mAnmSet;
    anm->setAnm0(HMN_ANM_NPC_SITDOWNT_WAIT1);
    anm->setAnm1(HMN_ANM_NPC_SITDOWNT_WAIT1);
    anm->setAnm2(HMN_ANM_NPC_SITDOWNT_WAIT1);
    return 1;
}

// .text 0x380
int dAcNpcTownTheater_c::create() {
    if (!dAcNpcNml_c::create()) {
        return 0;
    }
    mState.change(this, dANpcTownTheater::State_c::STATE_IDLE);
    mCheckGround = 0;
    mObjc.setDefWeight(0xFF);
    return 1;
}

// .text 0x3E4
int dAcNpcTownTheater_c::doDelete() {
    return dAcNpcNml_c::doDelete() != 0;
}

// .text 0x410
int dAcNpcTownTheater_c::vtB0() {
    mState.execute(this);
    return 1;
}

namespace dANpcTownTheater {
// .text 0x43C
State_c::State_c() {
    mState = STATE_NONE;
    mpState = NULL;
    mStep = STEP_START;
}

// .text 0x454
State_c::~State_c() {
}

static const State_c::State_s l_states[] = {
    {&State_c::initIdle, &State_c::executeIdle},
    {&State_c::initTalk, &State_c::executeTalk},
    {&State_c::initLook, &State_c::executeLook},
};

// .text 0x494
void State_c::change(dAcNpcTownTheater_c *npc, u32 state) {
    if (state < STATE_COUNT) {
        mState = state;
        mpState = &l_states[state];
        mStep = STEP_START;
        if (mpState != NULL) {
            (this->*mpState->mInit)(npc);
        }
    }
}

// .text 0x4E4
void State_c::execute(dAcNpcTownTheater_c *npc) {
    if (mpState != NULL) {
        (this->*mpState->mExecute)(npc);
    }
}

// .text 0x518
void State_c::initIdle(dAcNpcTownTheater_c *npc) {
    npc->mLookAt.setFront(0, l_lookPitchStep, l_lookYawStep);
    npc->mAction.requestWait(0);
}

// .text 0x570
void State_c::executeIdle(dAcNpcTownTheater_c *) {
}

// .text 0x574
void State_c::initTalk(dAcNpcTownTheater_c *npc) {
    npc->mLookAt.setPlayerNo(0, dNet::getMyNo(), false, l_lookPitchStep,
                             l_lookYawStep, cNpcLookRange);
    npc->mTalk.requestTalk(true, 0);
}

// .text 0x5DC
void State_c::stepTalk(dAcNpcTownTheater_c *npc) {
    if (!npc->mTalk.isBusy()) {
        dDemo::finishTalk(npc);
        ++mStep;
    }
}

static const State_c::Proc l_talkSteps[] = {&State_c::stepTalk};

// .text 0x634
void State_c::executeTalk(dAcNpcTownTheater_c *npc) {
    if (mStep < STEP_DONE) {
        (this->*l_talkSteps[mStep])(npc);
    }
}

// .text 0x674
void State_c::initLook(dAcNpcTownTheater_c *npc) {
    npc->mLookAt.setPlayer(0, false, l_lookPitchStep, l_lookYawStep,
                           cNpcLookRange);
}

// .text 0x69C
void State_c::executeLook(dAcNpcTownTheater_c *) {
}
} // namespace dANpcTownTheater
