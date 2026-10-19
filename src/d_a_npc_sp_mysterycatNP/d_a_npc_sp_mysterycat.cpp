#include <game/game/d_a_npc_sp_mysterycat.hpp>
#include <game/cLib/c_math.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_dsn.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <string.h>

// Blanca, the mysterious cat. Functions are in target .text order.

// The first STR_Unit word of the design styles (STYLE_CUTE: "cute", ...).
#define MYSTERYCAT_STYLE_WORD 0x51

namespace dANpcSpMysterycat {
static dAcNpc_c::recept_c::msgInfo_s l_msgInfo = {"npc_SP/NPC_mysterycat", 0};

// .text 0x5C
BASE_PROFILE(A_NPC_SP_MYSTERYCAT, dAcNpcSpMysterycat_c);
} // namespace dANpcSpMysterycat

// .text 0x20C
bool dAcNpcSpMysterycat_c::canInteract(dDemoActor_c *actor) {
    if (dAcNpc_c::canInteract(actor) == false) {
        return false;
    }
    return !mTalk.isBusy();
}

// .text 0x258
void dAcNpcSpMysterycat_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeMode(&dAcNpcSpMysterycat_c::executeWait);
        break;
    case DEMO_CMD_TALK:
        mMysteryTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeMode(&dAcNpcSpMysterycat_c::executeTalk);
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        changeMode(&dAcNpcSpMysterycat_c::executeWait);
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

// .text 0x650
int dAcNpcSpMysterycat_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mMysteryTalk);
    return SUCCEEDED;
}

// .text 0x69C
int dAcNpcSpMysterycat_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    static const dState::mode_c<dAcNpcSpMysterycat_c>::Entry modes[] = {
        {"待機", &dAcNpcSpMysterycat_c::initWait,
         &dAcNpcSpMysterycat_c::executeWait},
        {"デモ待機", &dAcNpcSpMysterycat_c::initDemoWait,
         &dAcNpcSpMysterycat_c::executeDemoWait},
        {"会話", &dAcNpcSpMysterycat_c::initTalk,
         &dAcNpcSpMysterycat_c::executeTalk},
        {"うろうろ歩き", &dAcNpcSpMysterycat_c::initWanderWalk,
         &dAcNpcSpMysterycat_c::executeWanderWalk},
        {"非表示", &dAcNpcSpMysterycat_c::initHidden,
         &dAcNpcSpMysterycat_c::executeHidden},
    };
    dState::mode_c<dAcNpcSpMysterycat_c>::initialize(
        "dAcNpcSpMysterycat_c", modes, ARRAY_SIZE(modes), false);
    mObjc.setDefWeight(objc_c::WEIGHT_DEFAULT);
    mFaceRes.loadDesign(&dSaveData_c::getExtra()->mDesignBoard.mDesign,
                        dHeap::localHeap_p, ORG_DESIGN_WIDTH,
                        ORG_DESIGN_HEIGHT);
    nw4r::g3d::ResFile file(mFaceRes.getData());
    nw4r::g3d::ResTex tex = file.GetResTex(0);
    nw4r::g3d::ResPltt pltt = file.GetResPltt(0);
    nw4r::g3d::ResMdl mdl = getModel()->mMdl.getResMdl();
    nw4r::g3d::ResMat mat = mdl.GetResMat("face");
    if (mat.IsValid()) {
        const char *texName = "face";
        mat.ReleaseTexByName(texName, strlen(texName));
        mat.BindTexByName(tex, texName);
        const char *plttName = "skin";
        mat.ReleasePlttByName(plttName, strlen(plttName));
        mat.BindPlttByName(pltt, plttName);
    }
    clearNetSync();
    return SUCCEEDED;
}

// .text 0x848
int dAcNpcSpMysterycat_c::doDelete() {
    if (mFaceRes.release() == false) {
        return NOT_READY;
    }
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x89C
int dAcNpcSpMysterycat_c::vtB0() {
    if (mTimer != 0) {
        mTimer--;
    }
    dState::base_c<dAcNpcSpMysterycat_c>::execute();
    return SUCCEEDED;
}

// .text 0x904: copies the board design into her face texture and palette.
void dAcNpcSpMysterycat_c::updateFaceTex() {
    dDesign_c *design = &dSaveData_c::getDesignBoard()->mDesign;
    nw4r::g3d::ResFile file(mFaceRes.getData());
    nw4r::g3d::ResTex tex = file.GetResTex(0);
    void *data = tex.GetTexData();
    int texSize = (tex.GetWidth() * tex.GetHeight()) >> 1;
    memcpy(data, design->getTexture(), texSize);
    DCStoreRangeNoSync(data, texSize);
    tex.DCStore(false);
    nw4r::g3d::ResPltt pltt = file.GetResPltt(0);
    data = pltt.GetPlttData();
    memcpy(data, design->getPalette(), ORG_DESIGN_PALETTE_SIZE);
    DCStoreRangeNoSync(data, ORG_DESIGN_PALETTE_SIZE);
    pltt.DCStore(false);
}

// .text 0xA10
void dAcNpcSpMysterycat_c::initWait() {
    mAction.requestWait(PRIORITY_IDLE);
}

// .text 0xA24
void dAcNpcSpMysterycat_c::executeWait() {
    changeMode(&dAcNpcSpMysterycat_c::executeWanderWalk);
}

// .text 0xB80
void dAcNpcSpMysterycat_c::initDemoWait() {
    mAnmSet.setAnm0(HMN_ANM_NML_WAIT1);
    mAng angle = getAngleYToPlayer(0);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0);
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
}

// .text 0xC14 (+ a dead blr at 0xC40)
void dAcNpcSpMysterycat_c::executeDemoWait() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO);
    }
}

// .text 0xC44
void dAcNpcSpMysterycat_c::initTalk() {
    if (!mTalk.isBusy()) {
        mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                          cNpcLookRange);
        mAng angle = getTalkAngle();
        mAng speed = getTalkTurnSpeed();
        mTalk.requestTurnTalk(angle, speed, FALSE);
    }
}

// .text 0xCE4
void dAcNpcSpMysterycat_c::executeTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
    }
}

// .text 0xD24
void dAcNpcSpMysterycat_c::initWanderWalk() {
    initWander();
}

// .text 0xD28
void dAcNpcSpMysterycat_c::executeWanderWalk() {
    if (mViewArea.isOutsideAll(NET_MEMBER_MAX, this)) {
        changeMode(&dAcNpcSpMysterycat_c::executeHidden);
    } else {
        executeWander(NULL);
    }
}

// .text 0xEB8
void dAcNpcSpMysterycat_c::initHidden() {
    mAction.requestWait(PRIORITY_IDLE);
}

// .text 0xECC
void dAcNpcSpMysterycat_c::executeHidden() {
    if (!mViewArea.isOutsideAll(NET_MEMBER_MAX, this)) {
        changeMode(&dAcNpcSpMysterycat_c::executeWanderWalk);
    }
}

// .text 0x1040
void dAcNpcSpMysterycat_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mRequestEndMethod = NULL;
}

// .text 0x1088
void dAcNpcSpMysterycat_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
}

// .text 0x10D0
void dAcNpcSpMysterycat_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = dANpcSpMysterycat::l_msgInfo.mLabel;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dRelayDesign_c *board = dSaveData_c::getDesignBoard();
    pickRandomStyle();
    if (!board->mDesign.getCreator()->isValid()) {
        info->mCode = MSG_NO_FACE;
    } else if (board->mFaceDrawn) {
        if (*player->getPersonalID() == *board->mDesign.getCreator()) {
            info->mCode = MSG_MY_FACE_BY_YOU;
        } else {
            info->mCode = MSG_MY_FACE_BY_OTHER;
        }
    } else if (!player->isFlag0(PRIVATE_FLAG0_BLANCA_MET)) {
        player->setFlag0(PRIVATE_FLAG0_BLANCA_MET);
        info->mCode = MSG_FIRST_ASK;
    } else {
        info->mCode = MSG_SICK_OF_FACE;
    }
}

// .text 0x120C
void dAcNpcSpMysterycat_c::talk_c::onMessageStart(int arg) {
    dRelayDesign_c *board = dSaveData_c::getDesignBoard();
    setLandName(&board->mDesign.mCreator.land, 0);
    setPersonalName(&board->mDesign.mCreator, 1);
    mStyle = board->mDesign.getStyle();
    mpController->setStringWord(WORD_STYLE, MYSTERYCAT_STYLE_WORD + mStyle,
                                "sys_STRING/STR_Unit");
    mpController->setStringWord(WORD_RANDOM_STYLE,
                                MYSTERYCAT_STYLE_WORD + mRandomStyle,
                                "sys_STRING/STR_Unit");
}

// .text 0x12A8
void dAcNpcSpMysterycat_c::talk_c::pickRandomStyle() {
    int style = dSaveData_c::getDesignBoard()->mDesign.getStyle();
    int rnd;
    do {
        rnd = cM::rndInt(STYLE_UNSET);
    } while (style == rnd);
    mRandomStyle = rnd;
}

// .text 0x12FC
void dAcNpcSpMysterycat_c::talk_c::onMessageEnd(int kind) {
    switch (kind) {
    case END_REQ_DRAW_FACE:
        reqMenu32(FALSE);
        mRequestEndMethod = &talk_c::onDesignMenuEnd;
        break;
    case END_REQ_11:
        break;
    case END_REQ_CLEAR_FACE:
        dSaveData_c::getDesignBoard()->mDesign.initBlank();
        mpOwner->updateFaceTex();
        break;
    }
}

// .text 0x1384
void dAcNpcSpMysterycat_c::talk_c::onAnswer(int arg) {
}

// .text 0x1388
void dAcNpcSpMysterycat_c::talk_c::preExecute() {
}

// .text 0x138C
void dAcNpcSpMysterycat_c::talk_c::onTalkEnd() {
}

// .text 0x1390
void dAcNpcSpMysterycat_c::talk_c::onRequestEnd(int kind) {
    if (mRequestEndMethod) {
        Method method = mRequestEndMethod;
        callMessageMethod(method);
        if (method == mRequestEndMethod) {
            mRequestEndMethod = NULL;
        }
    }
}

// .text 0x1428: the design canvas closed (END_REQ_DRAW_FACE).
void dAcNpcSpMysterycat_c::talk_c::onDesignMenuEnd() {
    u16 code = getMessageCode();
    if (isMenuInvalid()) {
        switch (code) {
        case MSG_NO_FACE_DRAW:
            mpController->setNextMsgCode(MSG_NO_FACE_CANCEL);
            break;
        case MSG_CRITICIZED_DRAW:
            mpController->setNextMsgCode(MSG_CRITICIZED_CANCEL);
            break;
        case MSG_FIRST_DRAW:
            mpController->setNextMsgCode(MSG_FIRST_CANCEL);
            break;
        case MSG_SICK_DRAW:
            mpController->setNextMsgCode(MSG_SICK_CANCEL);
            break;
        }
    } else {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        dRelayDesign_c *board = dSaveData_c::getDesignBoard();
        board->mDesign.mCreator = player->mPID;
        if (code == MSG_CRITICIZED_DRAW) {
            board->mDesign.setStyle(mStyle);
        } else {
            board->mDesign.setStyle(mRandomStyle);
        }
        board->mFaceDrawn = 1;
        board->mSendPending = 1;
        board->mDays = 1;
        mpOwner->updateFaceTex();
        mpController->setNextMsgCode(code + 1);
    }
}

// .text 0x15E8
int dAcNpcSpMysterycat_c::getEarType() {
    return NPC_EAR_TYPE_BLANCA;
}
