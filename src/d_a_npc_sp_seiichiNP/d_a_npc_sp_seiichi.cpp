#include <game/game/d_a_npc_sp_seiichi.hpp>
#include <game/framework/f_profile.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_dsn.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_fish_info.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_pattern_copy.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_string.hpp>

// Wendell. Functions are in target .text order.

// npc_SP/NPC_seiichi message codes, named from the English text.
enum {
    MSG_HUNGRY = 0x01,        // "S-so huuuuuuuungry. M-must eat something!"
    MSG_DECLINED = 0x02,      // "Awww... No good, huh..."
    MSG_CANT_EAT = 0x03,      // "Wh-wh-what is this? ... I can't eat this!"
    MSG_STILL_HUNGRY = 0x04,  // "Aww, still so hungry..."
    MSG_CANT_EAT_TOOL = 0x05, // "...if I DID eat it, I think you'd miss it."
                              // (silver and gold tools)
    MSG_THANKS = 0x06,        // "OOOOOOOOOOOOOOOH! Th-thank you! Good food!"
    MSG_DESIGN_OFFER = 0x07,  // "Oh, so GOOOOOOOOOOD! ... must give you one of
                              // my designs"
    MSG_DESIGN_DECLINED = 0x08, // "Oh, I see... Well, I guess that's OK, too."
    MSG_DESIGN_WHERE = 0x09,    // "S-so, where do you want it?"
    MSG_DESIGN_GIVEN = 0x0A,    // "So glad I found someone good to take
                                // <design>!"
    MSG_HUNGRY_AGAIN = 0x0B,    // "N-not enough! N-need more to eat!"
    MSG_FORBIDDEN = 0x0C,       // "P-pretty sure Mother told me never to eat
                                // these."
    MSG_FED = 0x15,             // 0x15..0x17: he already gave a design today
    MSG_FED_2 = 0x16,           // "Th-thank you so much for the food earlier!"
    MSG_FED_3 = 0x17,           // "P-people around here are so n-nice!"
    MSG_DIST_SEEN = 0x1A,       // the player already has the distributed design
    MSG_DIST_OFFER = 0x23, // "Say... You there! ... I want to give you one of
                           // my new designs."
    MSG_DIST_OFFER_AGAIN = 0x24, // the same, offered before
};

// The answers of MSG_HUNGRY / MSG_HUNGRY_AGAIN.
enum { ANSWER_GIVE = 0 /* "Here you go." */, ANSWER_NO /* "Tough it out!" */ };

// Wendell's designs are the items ITEM_IDX_GRASS + n (KIND_DSN_DATA_SEIICHI).
#define WENDELL_DESIGN(item) (dItem::item - dItem::ITEM_IDX_GRASS)
#define WENDELL_DESIGN_NUM 0x34
#define WENDELL_DESIGN_NONE WENDELL_DESIGN_NUM

namespace dANpcSpSeiichi {
// .text 0x5C
BASE_PROFILE(A_NPC_SP_SEIICHI, dAcNpcSpSeiichi_c);
} // namespace dANpcSpSeiichi

// .text 0x1D4
void dAcNpcSpSeiichi_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeState(STATE_LOOK);
        break;
    case DEMO_CMD_TALK:
        mWendellTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case DEMO_CMD_SPEAK:
        fgMngProc_unblockDayChange();
        mWendellTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_FETCH);
        break;
    case DEMO_CMD_END:
    case CMD_RELEASE_PERMIT:
        changeState(STATE_WALK);
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpSeiichi {
static const dAcNpcSpSeiichi_c::State_s l_states[] = {
    {&dAcNpcSpSeiichi_c::initArrive, &dAcNpcSpSeiichi_c::executeArrive},
    {&dAcNpcSpSeiichi_c::initWalk, &dAcNpcSpSeiichi_c::executeWalk},
    {&dAcNpcSpSeiichi_c::initLook, &dAcNpcSpSeiichi_c::executeLook},
    {&dAcNpcSpSeiichi_c::initTalk, &dAcNpcSpSeiichi_c::executeTalk},
    {&dAcNpcSpSeiichi_c::initFetch, &dAcNpcSpSeiichi_c::executeFetch},
};
} // namespace dANpcSpSeiichi

// .text 0x2A4
void dAcNpcSpSeiichi_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpSeiichi::l_states[state].mInit)();
}

// .text 0x2DC
int dAcNpcSpSeiichi_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mWendellTalk);
    return SUCCEEDED;
}

namespace dANpcSpSeiichi {
// The villager whose birthday party the current player is invited to (-1:
// none).
static inline int getPlayerBirthdayHostIdx() {
    dSaveData_c *save = dSaveData_c::getRaw();
    return save->mAnimals.mTown.getBirthdayHostIdx(
        dPlayerMgr::getCurrentPlayerSlot(), 1, 1, 1);
}

// A distributed design arrived since the player last got one.
static inline BOOL isNewDistPattern(dPrivateData_c *player) {
    u32 dist = dSaveData_c::getExtra()->mDistPattern.mVersion;
    return dist != player->mDistPatternVersion;
}
} // namespace dANpcSpSeiichi

// .text 0x328
int dAcNpcSpSeiichi_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player == NULL) {
        return FAILED;
    }
    if (player->isFlag1(PRIVATE_FLAG1_WENDELL_DESIGN) ||
        player->isFlag1(PRIVATE_FLAG1_WENDELL_DIST_DESIGN)) {
        return FAILED;
    }
    u32 offered = player->isFlag1(PRIVATE_FLAG1_WENDELL_DIST_OFFERED);
    if (offered == TRUE || (dScene::getPrevScene() == dScene::SCENE_DM_PL_SEL &&
                            !dNpcMng_c::_8074EACF &&
                            dANpcSpSeiichi::getPlayerBirthdayHostIdx() == -1 &&
                            dANpcSpSeiichi::isNewDistPattern(player))) {
        mDistDesign = 1;
    } else {
        mDistDesign = 0;
        if (!dEvent::isVisitorHere(VISITOR_WENDELL)) {
            return FAILED;
        }
    }
    mObjc.setDefWeight(objc_c::WEIGHT_DEFAULT);
    mDesignItem = 0;
    _1E1C = 0;
    mArriving = 0;
    if (mDistDesign == 1 && dScene::getPrevScene() == dScene::SCENE_DM_PL_SEL &&
        !dNpcMng_c::_8074EACF) {
        player->clearFlag1(PRIVATE_FLAG1_WENDELL_DIST_OFFERED);
        fgMngProc_blockDayChange();
        changeState(STATE_ARRIVE);
    } else {
        changeState(STATE_WALK);
    }
    return SUCCEEDED;
}

// .text 0x4D8
int dAcNpcSpSeiichi_c::doDelete() {
    if (!mDesignLoader.release()) {
        return NOT_READY;
    }
    if (!dAcNpcSp_c::doDelete()) {
        return NOT_READY;
    }
    return SUCCEEDED;
}

// .text 0x52C
int dAcNpcSpSeiichi_c::vtB0() {
    if (mDesignItem != 0 &&
        mDesignLoader.loadIndex(mDesignItem, EGG::Heap::getCurrentHeap())) {
        dPatternCopy::getClipboard()->clear();
        nw4r::g3d::ResFile file(mDesignLoader.getData());
        void *tex = file.GetResTex(0).GetTexData();
        dPatternCopy::getClipboard()->setTexture(tex, FALSE);
        dPatternCopy::getClipboard()->setFromItem(mDesignItem);
        mDesignLoader.release();
        mDesignItem = 0;
    }
    if (mState < STATE_COUNT) {
        (this->*dANpcSpSeiichi::l_states[mState].mExecute)();
    }
    return SUCCEEDED;
}

// .text 0x610
void dAcNpcSpSeiichi_c::initArrive() {
    setDemoFlag8();
    mArriving = 1;
    mAction.requestWait(PRIORITY_IDLE);
}

// .text 0x658
void dAcNpcSpSeiichi_c::executeArrive() {
    dDemo::requestSpeak(this, NULL);
}

// .text 0x660
void dAcNpcSpSeiichi_c::initWalk() {
    initWander();
}

// .text 0x664
void dAcNpcSpSeiichi_c::executeWalk() {
    executeWander(NULL);
}

// .text 0x678
void dAcNpcSpSeiichi_c::initLook() {
    if (mArriving) {
        mAction.requestWait(PRIORITY_DEMO);
        mStep = STEP_DONE;
    } else {
        mAction.requestTurn(PRIORITY_DEMO, getAngleYToPlayer(_1B70),
                            l_turnSpeed, 0);
        mStep = STEP_START;
    }
}

// .text 0x708
void dAcNpcSpSeiichi_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO);
        mStep++;
    }
}

namespace dANpcSpSeiichi {
static const dAcNpcSpSeiichi_c::Proc l_lookSteps[] = {
    &dAcNpcSpSeiichi_c::stepLook};
} // namespace dANpcSpSeiichi

// .text 0x768
void dAcNpcSpSeiichi_c::executeLook() {
    if (mStep < ARRAY_SIZE(dANpcSpSeiichi::l_lookSteps)) {
        (this->*dANpcSpSeiichi::l_lookSteps[mStep])();
    }
}

// .text 0x7A8
void dAcNpcSpSeiichi_c::initTalk() {
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, FALSE);
    mStep = STEP_START;
}

// .text 0x840
void dAcNpcSpSeiichi_c::stepTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
        mStep++;
    }
}

namespace dANpcSpSeiichi {
static const dAcNpcSpSeiichi_c::Proc l_talkSteps[] = {
    &dAcNpcSpSeiichi_c::stepTalk};
} // namespace dANpcSpSeiichi

// .text 0x88C
void dAcNpcSpSeiichi_c::executeTalk() {
    if (mStep < ARRAY_SIZE(dANpcSpSeiichi::l_talkSteps)) {
        (this->*dANpcSpSeiichi::l_talkSteps[mStep])();
    }
}

// .text 0x8CC
void dAcNpcSpSeiichi_c::initFetch() {
    mVec3_c pos;
    pos = *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    pos.z = mPos.z;
    dPlayerMgr::moveTo(&pos, dPlayerMgr::PLAYER_INDEX_CURRENT,
                       dPlayerMgr::sDefaultMoveSpeed);
    mStep = STEP_START;
}

// .text 0x938
void dAcNpcSpSeiichi_c::executeFetch() {
    if (!dPlayerMgr::isMovingTo(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        clearDemoFlag8();
        mArriving = 0;
        changeState(STATE_TALK);
    }
}

// .text 0x98C
dAcNpcSpSeiichi_c::talk_c::talk_c(dAcNpcSpSeiichi_c *npc) : mpOwner(npc) {
    init();
}

// .text 0x9F8
dAcNpcSpSeiichi_c::talk_c::~talk_c() {
}

// .text 0xA50
void dAcNpcSpSeiichi_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnAnswer = NULL;
    mOnRequestEnd = NULL;
    mNextOnRequestEnd = NULL;
    mWaitEquip = 0;
}

// .text 0xAEC
void dAcNpcSpSeiichi_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player->isFlag1(PRIVATE_FLAG1_WENDELL_DESIGN)) {
        switch (cM::rndInt(3)) {
        case 0:
            mMsgNo = MSG_FED;
            break;
        case 1:
            mMsgNo = MSG_FED_2;
            break;
        default:
            mMsgNo = MSG_FED_3;
            break;
        }
    } else if (mpOwner->mDistDesign) {
        u32 version =
            dPlayerMgr::Mgr_c::getCurrentPlayer()->mDistPatternVersion;
        u32 dist = dSaveData_c::getExtra()->mDistPattern.mVersion;
        if (dist != version) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(
                    PRIVATE_FLAG1_WENDELL_DIST_OFFERED)) {
                mMsgNo = MSG_DIST_OFFER_AGAIN;
            } else {
                mMsgNo = MSG_DIST_OFFER;
            }
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
                PRIVATE_FLAG1_WENDELL_DIST_OFFERED);
            dPatternCopy::getClipboard()->clear();
            dDesign_c *design = &dSaveData_c::getExtra()->mDistPattern.mDesign;
            *dPatternCopy::getClipboard() = *design;
        } else {
            mMsgNo = MSG_DIST_SEEN;
        }
    } else if (player->isFlag1(PRIVATE_FLAG1_WENDELL_FED)) {
        mMsgNo = MSG_HUNGRY_AGAIN;
    } else {
        mMsgNo = MSG_HUNGRY;
    }
}

// .text 0xE38
void dAcNpcSpSeiichi_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_seiichi";
    info->mCode = mMsgNo;
}

// .text 0xE50
void dAcNpcSpSeiichi_c::talk_c::onMessageStart(int arg) {
}

// .text 0xE54
void dAcNpcSpSeiichi_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_THANKS:
        mOnMessageEnd = &talk_c::eatFood;
        mOnRequestEnd = &talk_c::setForceNext;
        dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
            PRIVATE_FLAG1_WENDELL_FED);
        break;
    case MSG_CANT_EAT:
    case MSG_CANT_EAT_TOOL:
    case MSG_FORBIDDEN:
        mOnMessageEnd = &talk_c::returnFood;
        mOnRequestEnd = &talk_c::pickUpFood;
        break;
    case MSG_DESIGN_WHERE:
        mOnMessageEnd = &talk_c::openSlotSelect;
        mOnRequestEnd = &talk_c::onSlotSelected;
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0xFA4
void dAcNpcSpSeiichi_c::talk_c::onAnswer(int arg) {
    u8 answer = mpController->mSelect.mCursor;
    switch (mMessageCode) {
    case MSG_HUNGRY:
    case MSG_HUNGRY_AGAIN:
        if (answer == ANSWER_GIVE) {
            mOnAnswer = &talk_c::openFoodSelect;
        }
        break;
    }
    if (mOnAnswer) {
        (this->*mOnAnswer)();
        mOnAnswer = NULL;
    }
}

// .text 0x1050
void dAcNpcSpSeiichi_c::talk_c::preExecute() {
    if (mWaitEquip && !dPlayerMgr::isChangingEquipment()) {
        reqMsgClose();
        mWaitEquip = 0;
    }
}

// .text 0x10A0
void dAcNpcSpSeiichi_c::talk_c::onTalkEnd() {
}

// .text 0x10A4
void dAcNpcSpSeiichi_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = mNextOnRequestEnd;
        mNextOnRequestEnd = NULL;
    }
}

// .text 0x1120
void dAcNpcSpSeiichi_c::talk_c::openFoodSelect() {
    if (mpController->mNextMsgCode == 0) {
        mpController->setNextMsgCode(dMsg::MSG_CODE_INVALID);
    }
    reqSelectItem((u16)0, 0xE7, 0x2D, 1);
    mOnRequestEnd = &talk_c::onFoodSelected;
}

// .text 0x11A0
void dAcNpcSpSeiichi_c::talk_c::onFoodSelected() {
    if (isMenuInvalid()) {
        mpController->setNextMsgCode(MSG_DECLINED);
        reqMsgClose();
    } else {
        int slot = getMenuSelSlot();
        mFood = dPlayerMgr::Mgr_c::getCurrentPlayer()->mPockets[slot];
        takeFood();
        mNextOnRequestEnd = &talk_c::tasteFood;
    }
}

// .text 0x1234
void dAcNpcSpSeiichi_c::talk_c::takeFood() {
    requestItemActEx(7, &mFood, 0, 0, 0, 2);
}

// .text 0x1250
void dAcNpcSpSeiichi_c::talk_c::tasteFood() {
    dItem::Item none;
    dPlayerMgr::Mgr_c::getCurrentPlayer()->setPocket(&none, getMenuSelSlot(),
                                                     FALSE);
    int msg = MSG_CANT_EAT;
    int design = WENDELL_DESIGN_NONE;
    if (mFood.isFish()) {
        if (dFishInfo::isLarge(dItem::seeker_c::get()->findLike(mFood))) {
            msg = MSG_FORBIDDEN;
        } else {
            u16 fish = mFood.getId();
            msg = MSG_THANKS;
            if (fish == dItem::Item(dItem::ITEM_IDX_BITTERLING).getId() ||
                fish == dItem::Item(dItem::ITEM_IDX_POND_SMELT).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_INTERSECTION);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_PALE_CHUB).getId() ||
                       fish == dItem::Item(dItem::ITEM_IDX_YELLOW_PERCH).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_STREET_CORNER_4);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_CRUCIAN_CARP).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_GROUND);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_DACE).getId() ||
                       fish == dItem::Item(dItem::ITEM_IDX_CHERRY_SALMON).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_STREET_CORNER_1);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_BARBEL_STEED).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_VERTICAL_PATH);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_CARP).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_HORIZONTAL_PATH);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_KOI).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_FOOTSTEP);
            } else if ((fish == dItem::Item(dItem::ITEM_IDX_GOLDFISH).getId() ||
                        fish ==
                            dItem::Item(dItem::ITEM_IDX_POPEYED_GOLDFISH).getId()) ||
                       fish == dItem::Item(dItem::ITEM_IDX_DORADO).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_MONEY);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_KILLIFISH).getId() ||
                       fish == dItem::Item(dItem::ITEM_IDX_FROG).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_WATER_PUDDLE);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_CRAWFISH).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_RHINO_BEETLE);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_FRESHWATER_GOBY).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_MYSTERY_CIRCLE_A);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_LOACH).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_MYSTERY_CIRCLE_B);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_CATFISH).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_MYSTERY_CIRCLE_C);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_EEL).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_MYSTERY_CIRCLE_D);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_GIANT_SNAKEHEAD).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_MANHOLE);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_BLUEGILL).getId() ||
                       fish == dItem::Item(dItem::ITEM_IDX_CHAR).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_STREET_CORNER_2);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_BLACK_BASS).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_GRASS);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_PIKE).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_STONE_PAVING);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_SWEETFISH).getId() ||
                       fish == dItem::Item(dItem::ITEM_IDX_RAINBOW_TROUT).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_STREET_CORNER_3);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_SALMON).getId() ||
                       fish == dItem::Item(dItem::ITEM_IDX_LOBSTER).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_RED_CARPET);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_KING_SALMON).getId() ||
                       fish == dItem::Item(dItem::ITEM_IDX_RAY).getId()) {
                design =
                    WENDELL_DESIGN(ITEM_IDX_TREASURE_CHEST_DSN_DATA_SEIICHI);
            } else if ((fish == dItem::Item(dItem::ITEM_IDX_GUPPY).getId() ||
                        fish == dItem::Item(dItem::ITEM_IDX_ANGELFISH).getId()) ||
                       fish == dItem::Item(dItem::ITEM_IDX_NEON_TETRA).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_FIELD_OF_FLOWERS);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_PIRANHA).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_CRACKED_GROUND);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_AROWANA).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_PITFALL);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_GAR).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_GYROID);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_SEA_BUTTERFLY).getId() ||
                       fish == dItem::Item(dItem::ITEM_IDX_JELLYFISH).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_OUTER_SPACE);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_SEA_HORSE).getId() ||
                       fish == dItem::Item(dItem::ITEM_IDX_CLOWNFISH).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_CURVED_TRACK_4);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_SURGEONFISH).getId() ||
                       fish == dItem::Item(dItem::ITEM_IDX_BUTTERFLYFISH).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_VERTICAL_TRACK);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_ZEBRA_TURKEYFISH).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_SIGN);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_PUFFER_FISH).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_FORBIDDEN_SIGN);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_HORSE_MACKEREL).getId() ||
                       fish == dItem::Item(dItem::ITEM_IDX_SEA_BASS).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_HORIZONTAL_TRACK);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_BARRED_KNIFEJAW).getId() ||
                       fish == dItem::Item(dItem::ITEM_IDX_RED_SNAPPER).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_CURVED_TRACK_3);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_DAB).getId() ||
                       fish == dItem::Item(dItem::ITEM_IDX_OLIVE_FLOUNDER).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_CURVED_TRACK_1);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_SQUID).getId() ||
                       fish == dItem::Item(dItem::ITEM_IDX_OCTOPUS).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_CURVED_TRACK_2);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_MORAY_EEL).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_PITFALL_WARNING);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_FOOTBALL_FISH).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_STAIRS);
            } else if (fish == dItem::Item(dItem::ITEM_IDX_TUNA).getId() ||
                       fish == dItem::Item(dItem::ITEM_IDX_BLUE_MARLIN).getId()) {
                design = WENDELL_DESIGN(ITEM_IDX_IRON_FLOOR);
            }
        }
    } else {
        switch (dItem::infoBank_c::get()->getBITM(mFood)->getKind()) {
        case dItem::KIND_SILVER_FISHINGROD:
        case dItem::KIND_GOLD_FISHINGROD:
        case dItem::KIND_SILVER_SCOOP:
        case dItem::KIND_GOLD_SCOOP:
        case dItem::KIND_SILVER_AXE:
        case dItem::KIND_GOLD_AXE:
        case dItem::KIND_SILVER_WATERING:
        case dItem::KIND_GOLD_WATERING:
        case dItem::KIND_SILVER_NET:
        case dItem::KIND_GOLD_NET:
        case dItem::KIND_GOLD_PACHINKO:
        case dItem::KIND_SILVER_PACHINKO:
            msg = MSG_CANT_EAT_TOOL;
            break;
        case dItem::KIND_FRUIT:
            msg = MSG_THANKS;
            if (mFood == dItem::Item(dItem::ITEM_IDX_COCONUT)) {
                design = WENDELL_DESIGN(ITEM_IDX_COLORING_HOUSE);
            } else if (mFood == dItem::Item(dItem::ITEM_IDX_APPLE)) {
                design = WENDELL_DESIGN(ITEM_IDX_COLORING_NOOK);
            } else if (mFood == dItem::Item(dItem::ITEM_IDX_ORANGE)) {
                design = WENDELL_DESIGN(ITEM_IDX_COLORING_PELLY);
            } else if (mFood == dItem::Item(dItem::ITEM_IDX_PEAR)) {
                design = WENDELL_DESIGN(ITEM_IDX_COLORING_KK);
            } else if (mFood == dItem::Item(dItem::ITEM_IDX_PEACH)) {
                design = WENDELL_DESIGN(ITEM_IDX_COLORING_ROVER);
            } else if (mFood == dItem::Item(dItem::ITEM_IDX_CHERRY)) {
                design = WENDELL_DESIGN(ITEM_IDX_COLORING_RESETTI);
            }
            break;
        case dItem::KIND_KABU:
            msg = MSG_THANKS;
            design = WENDELL_DESIGN(ITEM_IDX_BRICK);
            break;
        case dItem::KIND_RKABU:
            msg = MSG_THANKS;
            design = WENDELL_DESIGN(ITEM_IDX_BLOCK_DSN_DATA_SEIICHI);
            break;
        case dItem::KIND_CANDY:
            msg = MSG_THANKS;
            if (mFood == dItem::Item(dItem::ITEM_IDX_BLUE_CANDY) ||
                mFood == dItem::Item(dItem::ITEM_IDX_YELLOW_CANDY)) {
                design = WENDELL_DESIGN(ITEM_IDX_COLORING_BOY);
            } else if (mFood == dItem::Item(dItem::ITEM_IDX_RED_CANDY) ||
                       mFood == dItem::Item(dItem::ITEM_IDX_GREEN_CANDY)) {
                design = WENDELL_DESIGN(ITEM_IDX_COLORING_GIRL);
            }
            break;
        case dItem::KIND_MUSHROOM:
            msg = MSG_THANKS;
            if (mFood == dItem::Item(dItem::ITEM_IDX_RARE_MUSHROOM)) {
                design = WENDELL_DESIGN(ITEM_IDX_PICTURE_FRAME);
            } else {
                design = WENDELL_DESIGN(ITEM_IDX_MEMO);
            }
            break;
        case dItem::KIND_CHOCOLATE:
            msg = MSG_THANKS;
            design = WENDELL_DESIGN(ITEM_IDX_CHOCOLATE);
            break;
        default:
            if (mFood == dItem::Item(dItem::ITEM_IDX_BIRTHDAY_CAKE)) {
                msg = MSG_THANKS;
                design = WENDELL_DESIGN(ITEM_IDX_SELF_PORTRAIT);
            }
            break;
        }
    }
    if (design < WENDELL_DESIGN_NUM) {
        mpOwner->mDesignItem = dItem::ITEM_IDX_GRASS + design;
    }
    mpController->setNextMsgCode(msg);
    reqMsgClose();
}

// .text 0x1D28
void dAcNpcSpSeiichi_c::talk_c::eatFood() {
    requestHandAct10(1);
}

// .text 0x1D30
void dAcNpcSpSeiichi_c::talk_c::setForceNext() {
    mpController->mForceNext = 1;
}

// .text 0x1D40
void dAcNpcSpSeiichi_c::talk_c::returnFood() {
    requestHandActE();
}

// .text 0x1D44
void dAcNpcSpSeiichi_c::talk_c::pickUpFood() {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(mFood, FALSE);
    mpController->setNextMsgCode(MSG_STILL_HUNGRY);
}

// .text 0x1D88
void dAcNpcSpSeiichi_c::talk_c::openSlotSelect() {
    reqMenu17(1);
}

// .text 0x1D90
void dAcNpcSpSeiichi_c::talk_c::onSlotSelected() {
    if (isMenuInvalid()) {
        mpController->setNextMsgCode(MSG_DESIGN_DECLINED);
    } else {
        u32 slot = fn_80029A84();
        dPatternCopy::copy(dPatternCopy::PATTERN_CLIPBOARD, 0,
                           dPatternCopy::PATTERN_MY_PLAYER, slot, TRUE);
        dString::Word_c word;
        word.set(dPatternCopy::getClipboard()->mName, 0);
        mpController->setWord(0, &word);
        mpController->setNextMsgCode(MSG_DESIGN_GIVEN);
        if (mpOwner->mDistDesign) {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
                PRIVATE_FLAG1_WENDELL_DIST_DESIGN);
            dPlayerMgr::Mgr_c::getCurrentPlayer()->mDistPatternVersion =
                dSaveData_c::getExtra()->mDistPattern.mVersion;
        } else {
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag1(
                PRIVATE_FLAG1_WENDELL_DESIGN);
        }
        mWaitEquip = dPlayerMgr::refreshEquippedDesign((u8)slot);
    }
    if (!mWaitEquip) {
        reqMsgClose();
    }
}
