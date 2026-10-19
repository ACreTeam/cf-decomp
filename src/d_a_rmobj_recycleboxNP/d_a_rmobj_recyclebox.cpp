#include <game/game/d_a_rmobj_recyclebox.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/snd/snd_id.hpp>

// The town hall's recycle box. Functions are in target .text order.

namespace dARmobjRecyclebox {
// .text 0x5C
BASE_PROFILE(A_RMOBJ_RECYCLEBOX, daRmobjRB_c);
} // namespace dARmobjRecyclebox

// .text 0x368
int daRmobjRB_c::create() {
    setDemoID(0);
    if (mMdl.create("obj_r_box", 0)) {
        mCloseSePlayed = TRUE;
        addBg();
        changeTalkState(TALK_NONE);
        changeLidState(getLidRecord());
        mBox.set(mPos, 32.0f, 64.0f, 32.0f, 0, 0x80400000, this, 0);
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x424
int daRmobjRB_c::execute() {
    mSnd.setPos(&mPos);
    mBox.regist();
    executeLidState();
    executeTalkState();
    mVec3_c min(mPos);
    mVec3_c max(mPos);
    min.z -= 11.2f;
    max.z += 11.2f;
    setLookPos(&min, &max);
    return SUCCEEDED;
}

// .text 0x4C8
int daRmobjRB_c::draw() {
    mMdl.entry();
    return SUCCEEDED;
}

// .text 0x4F0
int daRmobjRB_c::doDelete() {
    if (mMdl.remove()) {
        removeBg();
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x53C
bool daRmobjRB_c::canInteract(dDemoActor_c *actor) {
    if (actor != NULL) {
        if (mPos.xzDistTo(actor->getPos()) < 41.6f) {
            if (sLib::distanceAngle(mAngle.y.mAngle + 0x8000,
                                    actor->getAngle().y.mAngle) < 0x2000) {
                return true;
            }
        }
    }
    return false;
}

// .text 0x5DC
void daRmobjRB_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_TALK:
        changeTalkState(TALK_MESSAGE);
        break;
    case DEMO_CMD_END:
        changeTalkState(TALK_NONE);
        break;
    }
}

namespace dARmobjRecyclebox {
static const daRmobjRB_c::InitFunc l_lidStateInit[] = {
    &daRmobjRB_c::initLidClosed,
    &daRmobjRB_c::initLidOpening,
    &daRmobjRB_c::initLidOpen,
    &daRmobjRB_c::initLidClosing,
};
static const daRmobjRB_c::ExecFunc l_lidStateExec[] = {
    &daRmobjRB_c::executeLidClosed,
    &daRmobjRB_c::executeLidOpening,
    &daRmobjRB_c::executeLidOpen,
    &daRmobjRB_c::executeLidClosing,
};
} // namespace dARmobjRecyclebox

// .text 0x604
BOOL daRmobjRB_c::changeLidState(u8 state) {
    if (state < LID_NUM &&
        (this->*dARmobjRecyclebox::l_lidStateInit[state])()) {
        mLidState = state;
        setLidRecord(state);
        return TRUE;
    }
    return FALSE;
}

// .text 0x67C
void daRmobjRB_c::executeLidState() {
    if (mLidState < LID_NUM) {
        (this->*dARmobjRecyclebox::l_lidStateExec[mLidState])();
    }
    if (mTalkState == TALK_NONE) {
        u8 state = getLidRecord();
        if (mLidState != state) {
            changeLidState(state);
        }
    }
}

// .text 0x6F4
BOOL daRmobjRB_c::initLidClosed() {
    if (mMdl.setAnmChr(ANM_LID, m3d::FORWARD_ONCE, 0.0f)) {
        if (!mCloseSePlayed) {
            mSnd.startSound(SE_RecycleBox_Close);
        }
        return TRUE;
    }
    return FALSE;
}

// .text 0x760
void daRmobjRB_c::executeLidClosed() {
}

// .text 0x764
BOOL daRmobjRB_c::initLidOpening() {
    if (mMdl.setAnmChr(ANM_LID, m3d::FORWARD_ONCE, 0.0f)) {
        mSnd.startSound(SE_RecycleBox_Open);
        return TRUE;
    }
    return FALSE;
}

// .text 0x7C4
void daRmobjRB_c::executeLidOpening() {
    if (mMdl.isStopChr()) {
        if (mTalkState != TALK_NONE) {
            changeLidState(LID_OPEN);
        }
    } else {
        mMdl.playMdl();
    }
}

// .text 0x820
BOOL daRmobjRB_c::initLidOpen() {
    return mMdl.setAnmChr(ANM_LID, m3d::REVERSE_ONCE, 0.0f);
}

// .text 0x838
void daRmobjRB_c::executeLidOpen() {
}

// .text 0x83C
BOOL daRmobjRB_c::initLidClosing() {
    if (mMdl.setAnmChr(ANM_LID, m3d::REVERSE_ONCE, 0.0f)) {
        mCloseSePlayed = FALSE;
        return TRUE;
    }
    return FALSE;
}

// .text 0x898
void daRmobjRB_c::executeLidClosing() {
    if (mMdl.mAnmChr.checkFrame(3.0f)) {
        mSnd.startSound(SE_RecycleBox_Close);
        mCloseSePlayed = TRUE;
    }
    if (mMdl.isStopChr()) {
        if (mTalkState != TALK_NONE) {
            changeLidState(LID_CLOSED);
        }
    } else {
        mMdl.playMdl();
    }
}

namespace dARmobjRecyclebox {
static const daRmobjRB_c::InitFunc l_talkStateInit[] = {
    &daRmobjRB_c::initTalkNone,       &daRmobjRB_c::initTalkMessage,
    &daRmobjRB_c::initTalkMessageEnd, &daRmobjRB_c::initTalkOpen,
    &daRmobjRB_c::initTalkMenu,       &daRmobjRB_c::initTalkClose,
};
static const daRmobjRB_c::ExecFunc l_talkStateExec[] = {
    &daRmobjRB_c::executeTalkNone,       &daRmobjRB_c::executeTalkMessage,
    &daRmobjRB_c::executeTalkMessageEnd, &daRmobjRB_c::executeTalkOpen,
    &daRmobjRB_c::executeTalkMenu,       &daRmobjRB_c::executeTalkClose,
};
} // namespace dARmobjRecyclebox

// .text 0x920
BOOL daRmobjRB_c::changeTalkState(int state) {
    if (state < TALK_NUM &&
        (this->*dARmobjRecyclebox::l_talkStateInit[state])()) {
        mTalkState = state;
        return TRUE;
    }
    return FALSE;
}

// .text 0x98C
void daRmobjRB_c::executeTalkState() {
    if (mTalkState < TALK_NUM) {
        (this->*dARmobjRecyclebox::l_talkStateExec[mTalkState])();
    }
}

// .text 0x9CC
BOOL daRmobjRB_c::initTalkNone() {
    return TRUE;
}

// .text 0x9D4
void daRmobjRB_c::executeTalkNone() {
}

// .text 0x9D8
BOOL daRmobjRB_c::initTalkMessage() {
    attachDemoActor(this);
    setMessageLabel("npc_SP/NPC_obj");
    setMessageCode(MSG_RECYCLING_BIN);
    getController()->requestStart(0);
    return TRUE;
}

// .text 0xA44
void daRmobjRB_c::executeTalkMessage() {
    dMsg::Comp_c *comp = getController();
    if (comp != NULL) {
        if (!comp->isState(&dMsg::Comp_c::executeHidden)) {
            changeTalkState(TALK_MESSAGE_END);
        }
    }
}

// .text 0xAB8
BOOL daRmobjRB_c::initTalkMessageEnd() {
    return TRUE;
}

// .text 0xAC0
void daRmobjRB_c::executeTalkMessageEnd() {
    dMsg::Comp_c *comp = getController();
    if (comp != NULL) {
        if (comp->isState(&dMsg::Comp_c::executeHidden)) {
            detachDemoActor(this);
            changeTalkState(TALK_OPEN);
        }
    }
}

// .text 0xB4C
BOOL daRmobjRB_c::initTalkOpen() {
    return changeLidState(LID_OPENING);
}

// .text 0xB54
void daRmobjRB_c::executeTalkOpen() {
    if (mLidState == LID_OPEN) {
        changeTalkState(TALK_MENU);
    }
}

// .text 0xB6C
BOOL daRmobjRB_c::initTalkMenu() {
    return dMenuMng_c::openMenu(dMenuMng_c::MENU_RECYCLE_BOX) != FALSE;
}

// .text 0xB9C
void daRmobjRB_c::executeTalkMenu() {
    if (dMenuMng_c::isMenuEnd()) {
        changeTalkState(TALK_CLOSE);
    }
}

// .text 0xBDC
BOOL daRmobjRB_c::initTalkClose() {
    return changeLidState(LID_CLOSING);
}

// .text 0xBE4
void daRmobjRB_c::executeTalkClose() {
    if (mLidState == LID_CLOSED) {
        dDemo::finishTalk(this);
    }
}

// .text 0xBF8
void daRmobjRB_c::onMessageStart(int arg) {
}

// .text 0xBFC
void daRmobjRB_c::onMessageEnd(int kind) {
    getController()->mBgmKeep = TRUE;
}

// .text 0xC0C
void daRmobjRB_c::onAnswer(int arg) {
}

// .text 0xC10
void daRmobjRB_c::addBg() {
    dBGCF::getMvbgList()->add(32.0f, 64.0f, 32.0f, &mBg, mPos, mAng(0), NULL);
}

// .text 0xC6C
void daRmobjRB_c::removeBg() {
    dBGCF::getMvbgList()->remove(&mBg);
}

// .text 0xCA0
void daRmobjRB_c::setLidRecord(u8 state) {
    if (mTalkState != TALK_NONE) {
        dNet::setRecord(NET_REC_RECYCLEBOX_LID, &state, 0);
    }
}

// .text 0xCDC
u8 daRmobjRB_c::getLidRecord() {
    if (mTalkState == TALK_NONE) {
        u8 *record = (u8 *)dNet::getRecord(NET_REC_RECYCLEBOX_LID);
        if (record != NULL) {
            return *record;
        }
    }
    return LID_CLOSED;
}

#ifdef MUST_MATCH
// Matching-only emission-order workaround; no evidence of an original helper.
// The qualified call queues anmChr::getType before anmVis::getType.
// MWCC omits the unused body. See notes/TUs/d_a_rmobj_audit.txt.
static void orderRmobjWeakInlines(m3d::anmChr_c *anm) {
    anm->m3d::anmChr_c::getType();
}
#endif
