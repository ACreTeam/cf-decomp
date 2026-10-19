// A sign of the collision data (REL d_a_sign_tagNP). See
// include/game/game/d_a_sign_tag.hpp.
#include <game/game/d_a_sign_tag.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_bg_util.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_scene.hpp>
#include <game/sLib/s_lib.hpp>
#include <lib/egg/math/eggMath.h>

namespace dASignTag {
// .text:0x5C
BASE_PROFILE(A_SIGN_TAG, daSignTag_c);
} // namespace dASignTag

int daSignTag_c::sNum;

// .text:0x27C
int daSignTag_c::create() {
    mMsgCode = dBgUtil::getSignMsg();
    mSize = *dBgUtil::getSignSize();
    mSolid = dBgUtil::getSignFlag();
    mRadius =
        0.5f * EGG::Math<f32>::sqrt(mSize.x * mSize.x + mSize.z * mSize.z);
    mTalkRange = 12.0f + mRadius;
    setInteractionRadius(mTalkRange);
    setInteractionHeight(64.0f);
    mNo = sNum++;
    setDemoID((dScene::getCurrentScene() << 8 & 0xFF00) | mNo);
    setMode(MODE_WAIT);
    if (mSolid) {
        dBGCF::getMvbgList()->add(mSize.x, mSize.z, mSize.y, &mMvbg, mPos,
                                  mAngle.y.mAngle);
    }
    mDpdBox.set(mPos, mSize.x, mSize.z, mSize.y, mAngle.y, dObjc::HIT_RMOBJ,
                this, 0);
    return SUCCEEDED;
}

// .text:0x3BC
int daSignTag_c::execute() {
    dAcPlayer_c *player =
        dPlayerMgr::getPlayerActor(4); // this console's player
    if (player != NULL) {
        mPos.y = player->getPos().y;
    }
    executeMode();
    mDpdBox.regist();
    return SUCCEEDED;
}

// .text:0x410
int daSignTag_c::draw() {
    return SUCCEEDED;
}

// .text:0x418
int daSignTag_c::doDelete() {
    if (mSolid) {
        dBGCF::getMvbgList()->remove(&mMvbg);
    }
    return SUCCEEDED;
}

// .text:0x45C
bool daSignTag_c::canInteract(dDemoActor_c *actor) {
    f32 range = 44.8f + mRadius;
    if (actor != NULL) {
        mVec3_c dist = getPos() - actor->getPos();
        if (EGG::Math<f32>::sqrt(dist.x * dist.x + dist.z * dist.z) < range) {
            if (sLib::distanceAngle(mAngle.y.mAngle + DEG_TO_ANGLE(180),
                                    actor->getAngle().y.mAngle) <
                DEG_TO_ANGLE(45)) {
                return true;
            }
        }
    }
    return false;
}

// .text:0x514
void daSignTag_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_TALK:
        setMode(MODE_TALK);
        break;
    case DEMO_CMD_END:
        setMode(MODE_WAIT);
        break;
    }
}

// .text:0x53C
const daSignTag_c::InitFunc daSignTag_c::sInitFuncs[MODE_NUM] = {
    &daSignTag_c::initWait,
    &daSignTag_c::initTalk,
    &daSignTag_c::initTalkEnd,
};
BOOL daSignTag_c::setMode(int mode) {
    if (mode < MODE_NUM && (this->*sInitFuncs[mode])()) {
        mMode = mode;
        return TRUE;
    }
    return FALSE;
}

// .text:0x5A8
const daSignTag_c::ExecFunc daSignTag_c::sExecuteFuncs[MODE_NUM] = {
    &daSignTag_c::executeWait,
    &daSignTag_c::executeTalk,
    &daSignTag_c::executeTalkEnd,
};
void daSignTag_c::executeMode() {
    if (mMode < MODE_NUM) {
        (this->*sExecuteFuncs[mMode])();
    }
}

// .text:0x5E8
BOOL daSignTag_c::initWait() {
    return TRUE;
}

// .text:0x5F0
void daSignTag_c::executeWait() {
}

// .text:0x5F4
BOOL daSignTag_c::initTalk() {
    attachDemoActor(this);
    setMessageLabel("sys_MESS/SYS_Board");
    setMessageCode(mMsgCode);
    getController()->requestStart(0);
    return TRUE;
}

// .text:0x660
void daSignTag_c::executeTalk() {
    dMsg::Comp_c *comp = getController();
    if (comp != NULL) {
        if (!comp->isState(&dMsg::Comp_c::executeHidden)) {
            setMode(MODE_TALK_END);
        }
    }
}

// .text:0x6D4
BOOL daSignTag_c::initTalkEnd() {
    return TRUE;
}

// .text:0x6DC
void daSignTag_c::executeTalkEnd() {
    dMsg::Comp_c *comp = getController();
    if (comp != NULL) {
        if (comp->isState(&dMsg::Comp_c::executeHidden)) {
            detachDemoActor(this);
            dDemo::finishTalk(this);
        }
    }
}
