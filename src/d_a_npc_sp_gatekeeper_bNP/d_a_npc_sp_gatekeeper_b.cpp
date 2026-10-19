#include <game/game/d_a_npc_sp_gatekeeper_b.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_def.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_police_box.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <cstring>

// Booker, the second gatekeeper. Functions are in target .text order.

namespace dANpcSpGatekeeperB {
// The label of Booker's messages (.data 0x18; the string is .data 0x0).
const char *l_msgLabel = "NPC_sp/NPC_gatekeeperB";

// .text 0x5C
BASE_PROFILE(A_NPC_SP_GATEKEEPER_B, dAcNpcSpGkB_c);

// The current player's house (the const accessor).
static inline const dHome_c *getHouse(const dHomeList_c *homes, u32 home) {
    return homes->getHome(home);
}
} // namespace dANpcSpGatekeeperB

// .text 0x1C78 (weak): defined inline in this file, its weak copy comes first
// in the weak tail (before d_a_npc.hpp's inline virtuals).
inline int dAcNpcSpGkB_c::getEarType() {
    return NPC_EAR_TYPE_BOOKER;
}

// .text 0x1D4
void dAcNpcSpGkB_c::onDemoCmd(int cmd, u8 member) {
    const int originalCmd = cmd;
    _1B70 = member;
    int permit = requestPermit(cmd, member, TRUE);
    switch (cmd) {
    case DEMO_CMD_START:
        if (permit == PERMIT_REMOTE_START || permit == PERMIT_LOCAL_START) {
            changeMode(&dAcNpcSpGkB_c::executeDemoWait);
        }
        break;
    case DEMO_CMD_TALK:
        if (permit == PERMIT_REMOTE_TALK) {
            changeMode(&dAcNpcSpGkB_c::executeRemoteTalk);
        } else if (permit == PERMIT_LOCAL_TALK) {
            mTalkGk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeMode(&dAcNpcSpGkB_c::executeTalk);
        }
        break;
    case DEMO_CMD_END:
        if (permit == PERMIT_END_SINGLE) {
            changeMode(&dAcNpcSpGkB_c::executeTurnBack);
        } else if (permit == PERMIT_END_MULTI) {
            changeMode(&dAcNpcSpGkB_c::executeRemote);
        }
        break;
    case CMD_RELEASE_PERMIT:
        if (permit == PERMIT_RELEASED) {
            changeMode(&dAcNpcSpGkB_c::executeTurnBack);
        }
        break;
    }
    dAcNpcSp_c::onDemoCmd(originalCmd, member);
}

// .text 0x980
int dAcNpcSpGkB_c::preCreate() {
    if (dScene::getCurrentScene() == dScene::SCENE_DM_CHKP_MISS ||
        dScene::getCurrentScene() == dScene::SCENE_DM_CHKP_CNNCT ||
        dScene::getCurrentScene() == dScene::SCENE_DM_CHKP_DCNNCT ||
        !isMultiPlay()) {
        clearNetSync();
    }
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mTalkGk);
    mAnmSet.setAnm0(HMN_ANM_PLC_WAIT1);
    mAnmSet.setAnm1(HMN_ANM_PLC_WALK1);
    return SUCCEEDED;
}

// .text 0xA2C
int dAcNpcSpGkB_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    static const dState::mode_c<dAcNpcSpGkB_c>::Entry modes[] = {
        {"待機", &dAcNpcSpGkB_c::initWait, &dAcNpcSpGkB_c::executeWait},
        {"デモ待機", &dAcNpcSpGkB_c::initDemoWait,
         &dAcNpcSpGkB_c::executeDemoWait},
        {"会話", &dAcNpcSpGkB_c::initTalk, &dAcNpcSpGkB_c::executeTalk},
        {"初期角度に復帰", &dAcNpcSpGkB_c::initTurnBack,
         &dAcNpcSpGkB_c::executeTurnBack},
        {"通信：リモート", &dAcNpcSpGkB_c::initRemote,
         &dAcNpcSpGkB_c::executeRemote},
        {"通信：リモート会話", &dAcNpcSpGkB_c::initRemoteTalk,
         &dAcNpcSpGkB_c::executeRemoteTalk},
    };
    dState::mode_c<dAcNpcSpGkB_c>::initialize("dAcNpcSpGkB_c", modes,
                                              ARRAY_SIZE(modes));
    if (isLocalOwner()) {
        changeMode(&dAcNpcSpGkB_c::executeWait);
    } else {
        changeMode(&dAcNpcSpGkB_c::executeRemote);
    }
    return SUCCEEDED;
}

// .text 0xD4C
int dAcNpcSpGkB_c::doDelete() {
    if (!dAcNpcSp_c::doDelete()) {
        return NOT_READY;
    }
    return SUCCEEDED;
}

// .text 0xD78
int dAcNpcSpGkB_c::vtB0() {
    dState::base_c<dAcNpcSpGkB_c>::execute();
    return SUCCEEDED;
}

// .text 0xDCC
void dAcNpcSpGkB_c::initWait() {
    mAction.requestWait(PRIORITY_IDLE);
}

// .text 0xDE0
void dAcNpcSpGkB_c::executeWait() {
}

// .text 0xDE4
void dAcNpcSpGkB_c::initDemoWait() {
}

// .text 0xDE8
void dAcNpcSpGkB_c::executeDemoWait() {
}

// .text 0xDEC
void dAcNpcSpGkB_c::initTalk() {
    if (!mTalk.isBusy()) {
        mAng angle = getTalkAngle();
        mAng speed = getTalkTurnSpeed();
        mTalk.requestTurnTalk(angle, speed, FALSE);
    }
}

// .text 0xE64
void dAcNpcSpGkB_c::executeTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
    }
}

// .text 0xEA4
void dAcNpcSpGkB_c::initTurnBack() {
    mAng angle(0);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0);
}

// .text 0xEEC
void dAcNpcSpGkB_c::executeTurnBack() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        changeMode(&dAcNpcSpGkB_c::executeWait);
    }
}

// .text 0x1060
void dAcNpcSpGkB_c::initRemote() {
}

// .text 0x1064
void dAcNpcSpGkB_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mTalkGk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeMode(&dAcNpcSpGkB_c::executeTalk);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeMode(&dAcNpcSpGkB_c::executeTurnBack);
        }
    }
}

// .text 0x1338
void dAcNpcSpGkB_c::initRemoteTalk() {
}

// .text 0x133C
void dAcNpcSpGkB_c::executeRemoteTalk() {
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
        changeMode(&dAcNpcSpGkB_c::executeTurnBack);
    }
}

// .text 0x14B8
dAcNpcSpGkB_c::talk_c::talk_c(dAcNpcSpGkB_c *npc) {
    mpOwner = npc;
    init();
}

// .text 0x1528
void dAcNpcSpGkB_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgCode = 0;
    mRequestEndMethod = NULL;
}

// .text 0x1578
void dAcNpcSpGkB_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    if (isPlayerFromThisTown()) {
        dHomeList_c *homes = &dSaveData_c::getRaw()->mHomes;
        if (dANpcSpGatekeeperB::getHouse(homes, homes->findCurrentPlayer()) ==
            NULL) {
            mMsgCode = MSG_WELCOME;
        } else {
            mMsgCode = MSG_MENU;
        }
    } else if (!dNpcInfo::gatekeeperB_c::get()->mGreeted) {
        dNpcInfo::gatekeeperB_c *info = dNpcInfo::gatekeeperB_c::get();
        info->mGreeted = 1;
        mMsgCode = MSG_FIRST_MEETING;
    } else {
        mMsgCode = MSG_HELLO;
    }
}

// .text 0x1638
void dAcNpcSpGkB_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = dANpcSpGatekeeperB::l_msgLabel;
    info->mCode = mMsgCode;
}

// .text 0x1650
void dAcNpcSpGkB_c::talk_c::preExecute() {
}

// .text 0x1654
void dAcNpcSpGkB_c::talk_c::onTalkEnd() {
}

// .text 0x1658
void dAcNpcSpGkB_c::talk_c::onMessageEnd(int kind) {
    struct Entry {
        const char *label;
        int code;
        Method method;
    };
    static Entry messages[] = {
        {dANpcSpGatekeeperB::l_msgLabel, MSG_LOST_AND_FOUND,
         &talk_c::openLostAndFoundMenu},
        {dANpcSpGatekeeperB::l_msgLabel, MSG_CHANGE_FLAG,
         &talk_c::openFlagMenu},
    };
    u16 code = getMessageCode();
    const char *label;
    Entry *entry = messages;
    label = getMessageLabel();
    for (int i = 0; i < (int)ARRAY_SIZE(messages); i++, entry++) {
        if (std::strcmp(label, entry->label) == 0 && code == entry->code) {
            callMessageMethod(entry->method);
        }
    }
}

// .text 0x1714
void dAcNpcSpGkB_c::talk_c::openLostAndFoundMenu() {
    reqMenu1B(0);
    mRequestEndMethod = &talk_c::onLostAndFoundMenuEnd;
}

// .text 0x1760
void dAcNpcSpGkB_c::talk_c::openFlagMenu() {
    reqMenu16(0);
    mRequestEndMethod = &talk_c::onFlagMenuEnd;
}

// .text 0x17AC
void dAcNpcSpGkB_c::talk_c::onAnswer(int answer) {
    struct Entry {
        const char *label;
        int code;
        int answer;
        Method method;
    };
    static Entry messages[] = {
        {dANpcSpGatekeeperB::l_msgLabel, MSG_MENU, ANSWER_WHATS_NEW,
         &talk_c::answerWhatsNew},
        {dANpcSpGatekeeperB::l_msgLabel, MSG_MENU, ANSWER_SOMETHING_LOST,
         &talk_c::answerSomethingLost},
    };
    u16 code = getMessageCode();
    const char *label;
    int selected = mpController->mSelect.mCursor;
    Entry *entry = messages;
    label = getMessageLabel();
    for (int i = 0; i < (int)ARRAY_SIZE(messages); i++, entry++) {
        if (std::strcmp(label, entry->label) == 0 && code == entry->code &&
            selected == entry->answer) {
            callMessageMethod(entry->method);
        }
    }
}

// .text 0x187C
void dAcNpcSpGkB_c::talk_c::answerWhatsNew() {
    if (isMultiPlay()) {
        dPrivateData_c *visitor =
            dPlayerMgr::Mgr_c::getNetPlayerRaw(dSvMgr_c::getLastVisitor());
        setPersonalName(&visitor->mPID, 1);
        mpController->setNextMsgCode(MSG_LAST_VISITOR);
        return;
    }
    u16 npc;
    dNpcMng_c::getTodayEventNpc(&npc);
    switch (npc) {
    case SP_NPC_KABUPEDDLER:
        mpController->setNextMsgCode(MSG_VISITOR_JOAN);
        break;
    case SP_NPC_CARPETPEDDLER:
        mpController->setNextMsgCode(MSG_VISITOR_SAHARAH);
        break;
    case SP_NPC_MISSING_CHILD:
        mpController->setNextMsgCode(MSG_VISITOR_LOST_KITTEN);
        break;
    case SP_NPC_MISSING_MOTHER:
        mpController->setNextMsgCode(MSG_VISITOR_MOTHER_CAT);
        break;
    case SP_NPC_ARTIST:
        mpController->setNextMsgCode(MSG_VISITOR_WENDELL);
        break;
    case SP_NPC_TOTAKEKE:
        mpController->setNextMsgCode(MSG_VISITOR_KK_SLIDER);
        break;
    case SP_NPC_MASK_CAT:
        mpController->setNextMsgCode(MSG_VISITOR_MYSTERIOUS_CAT);
        break;
    case SP_NPC_FISHING_MASTER:
        mpController->setNextMsgCode(MSG_VISITOR_CHIP);
        break;
    case SP_NPC_BUG_MASTER:
        mpController->setNextMsgCode(MSG_VISITOR_NAT);
        break;
    case SP_NPC_EASTER_BUNNY:
        mpController->setNextMsgCode(MSG_VISITOR_ZIPPER);
        break;
    case SP_NPC_DANCER:
        mpController->setNextMsgCode(MSG_VISITOR_PAVE);
        break;
    default:
        mpController->setNextMsgCode(MSG_NO_VISITOR);
        break;
    }
}

// .text 0x1A58
void dAcNpcSpGkB_c::talk_c::answerSomethingLost() {
    int count = dSaveData_c::getTown()->mPoliceBox.count();
    if (count) {
        formatNumber(count, 0, 12, dScript::NUM_FORMAT_REGION);
        mpController->setNextMsgCode(MSG_LOST_AND_FOUND);
    } else {
        mpController->setNextMsgCode(MSG_LOST_AND_FOUND_EMPTY);
    }
}

// .text 0x1ACC
void dAcNpcSpGkB_c::talk_c::onRequestEnd(int kind) {
    if (mRequestEndMethod) {
        Method method = mRequestEndMethod;
        callMessageMethod(method);
        if (method == mRequestEndMethod) {
            mRequestEndMethod = NULL;
        }
    }
}

// .text 0x1B64
void dAcNpcSpGkB_c::talk_c::onLostAndFoundMenuEnd() {
    int taken = fn_8019B250();
    if (taken != LOST_ITEMS_NONE) {
        if (taken == LOST_ITEMS_ONE) {
            dItem::Item item(fn_8019B260());
            setItemName(item, 0);
            mpController->setNextMsgCode(MSG_LOST_ITEM_TAKEN);
        } else {
            mpController->setNextMsgCode(MSG_LOST_ITEMS_TAKEN);
        }
    } else {
        mpController->setNextMsgCode(MSG_LOST_ITEM_CANCELLED);
    }
}

// .text 0x1BE4
void dAcNpcSpGkB_c::talk_c::onFlagMenuEnd() {
    if (lbl_8074EA7D == 1) {
        if ((int)fn_8019AFE4() != -1) {
            dPlayerMgr::refreshEquippedUmbrellaDesign();
            if (dPlayerMgr::isDesignEquipped((u8)fn_8019AFE4())) {
                mEquip.setFromPlayer();
                requestEquip(&mEquip, 5, dPlayerMgr::EQUIP_CHANGE_NORMAL);
            }
        }
        mpController->setNextMsgCode(MSG_FLAG_CHANGED);
    } else {
        mpController->setNextMsgCode(MSG_FLAG_CANCELLED);
    }
}
