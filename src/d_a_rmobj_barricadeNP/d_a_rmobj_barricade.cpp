#include <game/game/d_a_rmobj_barricade.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_str_debug.hpp>
#include <game/sLib/s_lib.hpp>
#include <lib/egg/math/eggMath.h>
#include <cstring>

// The entrance barricade. Functions are in target .text order.

namespace dARmobjBarricade {
// .text 0x5C
BASE_PROFILE(A_RMOBJ_BARRICADE, daRmobjBarricade_c);
} // namespace dARmobjBarricade

bool daRmobjBarricade_c::sPlaced;

// .text 0x368
bool daRmobjBarricade_c::isPlaced() {
    return sPlaced;
}

// .text 0x374
int daRmobjBarricade_c::create() {
    const char *name = dScene::getCurrentScene() == dScene::SCENE_RM_THEATER_ENT
                           ? "obj_tw_theater_ent_pole"
                           : "obj_tw_HRA_ent_pole";
    Param_s param;
    memcpy(&param, (const void *)mParam, sizeof(Param_s));
    u8 scene = param.mScene;
    u8 prev = dScene::getPrevScene();
    if (prev == scene) {
        return FAILED;
    }
    dTime_c time = *dTime_c::getCurrent();
    if (!dSTR::isForceClosed() && time.hour >= param.mOpenHour &&
        time.hour < param.mCloseHour) {
        return FAILED;
    }
    setDemoID((u16)param.mDemoID);
    if (mModel.create(name, 0)) {
        setBox(param.mSizeX, param.mSizeZ);
        f32 sizeZ = param.mSizeZ;
        f32 sizeX = param.mSizeX;
        dBGCF::getMvbgList()->add(sizeX, sizeZ, 32.0f, &mBg, mPos);
        mModel.mMdl.setPriorityDraw(2, 9);
        changeState(STATE_WAIT);
        sPlaced = true;
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x55C
int daRmobjBarricade_c::execute() {
    mVec3_c min(mPos);
    mVec3_c max(mPos);
    min.x -= 11.2f;
    max.x += 11.2f;
    setLookPos(&min, &max);
    executeState();
    mBox.regist();
    return SUCCEEDED;
}

// .text 0x5E8
int daRmobjBarricade_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

// .text 0x610
int daRmobjBarricade_c::doDelete() {
    if (mModel.remove()) {
        if (mBg.mActive) {
            dBGCF::getMvbgList()->remove(&mBg);
            sPlaced = false;
        }
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x678
bool daRmobjBarricade_c::canInteract(dDemoActor_c *actor) {
    if (dScene::getCurrentScene() == dScene::SCENE_RM_THEATER_ENT) {
        return false;
    }
    if (actor != NULL) {
        mVec3_c diff = mPos - actor->getPos();
        if (EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z) < 41.6f) {
            if (sLib::distanceAngle(mAngle.y.mAngle + 0x8000,
                                    actor->getAngle().y.mAngle) < 0x2000) {
                return true;
            }
        }
    }
    return false;
}

// .text 0x730
void daRmobjBarricade_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_TALK:
        changeState(STATE_TALK_START);
        break;
    case DEMO_CMD_END:
        changeState(STATE_WAIT);
        break;
    }
}

// .text 0x758
BOOL daRmobjBarricade_c::changeState(int state) {
    static const InitProc l_init[] = {
        &daRmobjBarricade_c::initWait,
        &daRmobjBarricade_c::initTalkStart,
        &daRmobjBarricade_c::initTalk,
    };
    if (state < STATE_NUM && (this->*l_init[state])()) {
        mState = state;
        return TRUE;
    }
    return FALSE;
}

// .text 0x7C4
void daRmobjBarricade_c::executeState() {
    static const ExecuteProc l_execute[] = {
        &daRmobjBarricade_c::executeWait,
        &daRmobjBarricade_c::executeTalkStart,
        &daRmobjBarricade_c::executeTalk,
    };
    if (mState < STATE_NUM) {
        (this->*l_execute[mState])();
    }
}

// .text 0x804
BOOL daRmobjBarricade_c::initWait() {
    return TRUE;
}

// .text 0x80C
void daRmobjBarricade_c::executeWait() {
}

// .text 0x810
BOOL daRmobjBarricade_c::initTalkStart() {
    attachDemoActor(this);
    setMessageLabel("sys_MESS/SYS_Door");
    Param_s param;
    memcpy(&param, (const void *)mParam, sizeof(Param_s));
    setMessageCode(param.mMsgCode);
    getController()->requestStart(0);
    return TRUE;
}

// .text 0x890
void daRmobjBarricade_c::executeTalkStart() {
    dMsg::Comp_c *comp = getController();
    if (comp != NULL) {
        if (!comp->isState(&dMsg::Comp_c::executeHidden)) {
            changeState(STATE_TALK);
        }
    }
}

// .text 0x904
BOOL daRmobjBarricade_c::initTalk() {
    return TRUE;
}

// .text 0x90C
void daRmobjBarricade_c::executeTalk() {
    dMsg::Comp_c *comp = getController();
    if (comp != NULL) {
        if (comp->isState(&dMsg::Comp_c::executeHidden)) {
            detachDemoActor(this);
            dDemo::finishTalk(this);
        }
    }
}

// .text 0x994
void daRmobjBarricade_c::onMessageStart(int arg) {
}

// .text 0x998
void daRmobjBarricade_c::onMessageEnd(int kind) {
}

// .text 0x99C
void daRmobjBarricade_c::onAnswer(int arg) {
}

#ifdef MUST_MATCH
// Matching-only emission-order workaround; no evidence of an original helper.
// The qualified call queues anmChr::getType before anmVis::getType.
// MWCC omits the unused body. See notes/TUs/d_a_rmobj_audit.txt.
static void orderRmobjWeakInlines(m3d::anmChr_c *anm) {
    anm->m3d::anmChr_c::getType();
}
#endif
