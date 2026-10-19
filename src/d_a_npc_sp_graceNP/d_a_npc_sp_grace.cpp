#include <game/game/d_a_npc_sp_grace.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene_check.hpp>
#include <game/game/d_sv_villager_move.hpp>

// Gracie in her shop. Functions are in target .text order.

namespace dANpcSpGrace {
static inline dPrivateData_c *getPlayer() {
    return dPlayerMgr::Mgr_c::getCurrentPlayer();
}
static inline dPrivateData_c *getPlayerRaw() {
    return dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
}
// The hat's BITM hide bone hides the hair (a full mask or 3..6).
static inline BOOL hidesHair(int bone) {
    return bone == dItem::HIDE_BONE_FULL_MASK || (u32)(bone - 3) <= 3;
}

// She is out on her shop days and for the story talk.
static inline void updateActive(dAcNpcSpGrace_c *npc) {
    if (dSaveData_c::getTown()->isFlag(SAVE_FLAG_GRACIE_IN_SHOP) ||
        npc->mDrama) {
        npc->mIsActive = 1;
    } else {
        npc->mIsActive = 0;
    }
}

// .text 0x5C: the item colour of a hair colour (0: none).
static int getHairColor(int hairColor) {
    switch (hairColor) {
    case 0:
        return dItem::COLOR_BLACK;
    case 1:
        return dItem::COLOR_BROWN;
    case 2:
        return dItem::COLOR_RED;
    case 3:
        return dItem::COLOR_BLUE;
    case 4:
        return dItem::COLOR_YELLOW;
    case 5:
        return dItem::COLOR_GREEN;
    case 6:
        return dItem::COLOR_PINK;
    case 7:
        return dItem::COLOR_WHITE;
    }
    return dItem::COLOR_NEUTRAL;
}

// .text 0xC4: the item colour of a shoe colour (0: none).
static int getShoeColor(int shoeColor) {
    switch (shoeColor) {
    case 0:
        return dItem::COLOR_YELLOW;
    case 1:
        return dItem::COLOR_RED;
    case 2:
        return dItem::COLOR_ORANGE;
    case 3:
        return dItem::COLOR_GREEN;
    case 4:
        return dItem::COLOR_BLUE;
    case 5:
        return dItem::COLOR_WHITE;
    case 6:
        return dItem::COLOR_BLACK;
    case 7:
        return dItem::COLOR_PURPLE;
    case 8:
        return dItem::COLOR_BROWN;
    case 9:
        return dItem::COLOR_PINK;
    }
    return dItem::COLOR_NEUTRAL;
}

// .text 0x13C: the debug page hook (empty in this build).
static BOOL entryDebugPage(dAcNpcSpGrace_c *npc) {
    return FALSE;
}

// .text 0x144
BASE_PROFILE(A_NPC_SP_GRACE, dAcNpcSpGrace_c);

// Unreferenced static of the unit (.bss 0x8, built by __sinit): a cleared
// word.
struct unk_s {
    unk_s() : _0(0) {
    }
    u32 _0;
};
static unk_s l_unk;
} // namespace dANpcSpGrace

// .text 0x2C0
void dAcNpcSpGrace_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeState(STATE_LOOK);
        break;
    case DEMO_CMD_TALK:
        mGraceTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case DEMO_CMD_SPEAK:
        mGraceTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        changeState(STATE_IDLE);
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpGrace {
static const dAcNpcSpGrace_c::State_s l_states[] = {
    {&dAcNpcSpGrace_c::initIdle, &dAcNpcSpGrace_c::executeIdle},
    {&dAcNpcSpGrace_c::initLook, &dAcNpcSpGrace_c::executeLook},
    {&dAcNpcSpGrace_c::initTalk, &dAcNpcSpGrace_c::executeTalk},
};
} // namespace dANpcSpGrace

// .text 0x38C
void dAcNpcSpGrace_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpGrace::l_states[state].mInit)();
}

// .text 0x3C4
int dAcNpcSpGrace_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return 0;
    }
    setRecept(&mGraceTalk);
    return 1;
}

// .text 0x410
int dAcNpcSpGrace_c::create() {
    if (!dANpcSpGrace::getPlayerRaw()->isFlag0(
            PRIVATE_FLAG0_GRACIE_STEP_SEEN) &&
        (dANpcSpGrace::getPlayerRaw()->getGracieStep() == STORY_STEP_DRAMA ||
         dANpcSpGrace::getPlayerRaw()->getGracieStep() == STORY_STEP_DRAMA_2)) {
        mDrama = 1;
    } else {
        mDrama = 0;
    }
    dANpcSpGrace::updateActive(this);
    if (!dAcNpcSp_c::create()) {
        return 0;
    }
    dSceneCheck::fn_80081858(dANpcSpGrace::entryDebugPage(this));
    return 1;
}

// .text 0x4DC
int dAcNpcSpGrace_c::doDelete() {
    if (!dAcNpcSp_c::doDelete()) {
        return 0;
    }
    dANpcSpGrace::getPlayer()->clearFlag1(PRIVATE_FLAG1_GRACIE_DISCOUNT);
    dANpcSpGrace::getPlayer()->clearFlag1(PRIVATE_FLAG1_GRACIE_CHECKED_HERE);
    dANpcSpGrace::entryDebugPage(this);
    dSceneCheck::fn_8008185C();
    return 1;
}

// .text 0x540
int dAcNpcSpGrace_c::vtB0() {
    dANpcSpGrace::updateActive(this);
    if (mState < STATE_COUNT) {
        (this->*dANpcSpGrace::l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x5CC
void dAcNpcSpGrace_c::initIdle() {
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
}

// .text 0x5E0
void dAcNpcSpGrace_c::executeIdle() {
}

// .text 0x5E4
void dAcNpcSpGrace_c::initLook() {
    mAng angle = getAngleYToPlayer(_1B70);
    mAction.requestTurn(PRIORITY_DEMO, angle, l_turnSpeed, 0, cNpcMorphFrames);
    mStep = STEP_START;
}

// .text 0x648
void dAcNpcSpGrace_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO, cNpcMorphFrames);
        mStep++;
    }
}

namespace dANpcSpGrace {
static const dAcNpcSpGrace_c::Proc l_lookSteps[] = {&dAcNpcSpGrace_c::stepLook};
} // namespace dANpcSpGrace

// .text 0x6A8
void dAcNpcSpGrace_c::executeLook() {
    if (mStep < ARRAY_SIZE(dANpcSpGrace::l_lookSteps)) {
        (this->*dANpcSpGrace::l_lookSteps[mStep])();
    }
}

// .text 0x6E8
void dAcNpcSpGrace_c::initTalk() {
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, FALSE);
    mStep = STEP_START;
}

// .text 0x754
void dAcNpcSpGrace_c::stepTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
        mStep++;
    }
}

namespace dANpcSpGrace {
static const dAcNpcSpGrace_c::Proc l_talkSteps[] = {&dAcNpcSpGrace_c::stepTalk};
} // namespace dANpcSpGrace

// .text 0x7A0
void dAcNpcSpGrace_c::executeTalk() {
    if (mStep < ARRAY_SIZE(dANpcSpGrace::l_talkSteps)) {
        (this->*dANpcSpGrace::l_talkSteps[mStep])();
    }
}

// .text 0x7E0
dAcNpcSpGrace_c::talk_c::talk_c(dAcNpcSpGrace_c *npc) : mpOwner(npc) {
    mDramaTalked = 0;
    init();
}

// .text 0x848
dAcNpcSpGrace_c::talk_c::~talk_c() {
}

// .text 0x8A0
void dAcNpcSpGrace_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnAnswer = NULL;
    mOnRequestEnd = NULL;
    mPenalty = 0;
    mOneColorTold = 0;
    mLookTheme = 0;
    mNoLook = 0;
    mChecking = 0;
    mBasicColorsJudged = 0;
    mShoeColorJudged = 0;
    mHiddenHairColorsJudged = 0;
    mColorsJudged = 0;
    mHairJudged = 0;
    mShoeShirtJudged = 0;
    mHairNext = 0;
    mHairNextTheme = 0;
    mBasicColorsCase = 0;
    mHiddenHairColorsCase = 0;
    mColorsCase = 0;
    mShoeColorMatch = 0;
    mHairFits = 0;
    mShoesMatchShirt = 0;
    mMainColorCount = 0;
    mColorCount = 0;
    mMainColor = dItem::COLOR_COUNT;
    mFriendMentioned = 0;
}

// .text 0x980
void dAcNpcSpGrace_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    if (mpOwner->mDrama) {
        if (dANpcSpGrace::getPlayerRaw()->getGracieStep() == STORY_STEP_DRAMA) {
            if (mDramaTalked) {
                mMsgNo = MSG_DRAMA_AGAIN;
            } else {
                mMsgNo = MSG_DRAMA_SIGH;
            }
        } else if (mDramaTalked) {
            mMsgNo = MSG_DRAMA_AGAIN_2;
        } else {
            mMsgNo = MSG_DRAMA_SIGH_2;
        }
    } else if (!dANpcSpGrace::getPlayerRaw()->isFlag0(
                   PRIVATE_FLAG0_GRACIE_MET)) {
        mMsgNo = MSG_FIRST_MEETING;
    } else if (!dANpcSpGrace::getPlayer()->isFlag1(
                   PRIVATE_FLAG1_GRACIE_TALKED)) {
        if (isSavedOutfit()) {
            mMsgNo = MSG_SAVED_OUTFIT;
        } else if (dANpcSpGrace::getPlayerRaw()->isFlag0(
                       PRIVATE_FLAG0_GRACIE_CHECKED)) {
            mMsgNo = MSG_HELLO_CHECKED;
        } else {
            mMsgNo = MSG_HELLO;
        }
    } else if (dANpcSpGrace::getPlayer()->isFlag1(
                   PRIVATE_FLAG1_GRACIE_CHECKED)) {
        if (dANpcSpGrace::getPlayer()->isFlag1(
                PRIVATE_FLAG1_GRACIE_CHECKED_HERE)) {
            if (dANpcSpGrace::getPlayer()->isFlag1(
                    PRIVATE_FLAG1_GRACIE_DISCOUNT)) {
                mMsgNo = MSG_DISCOUNT;
            } else {
                mMsgNo = MSG_STUDY;
            }
        } else {
            mMsgNo = MSG_HELLO_AGAIN;
        }
    } else if (dANpcSpGrace::getPlayer()->isBedHeadVisible()) {
        mMsgNo = MSG_BED_HEAD_AGAIN;
    } else if (isSavedOutfit()) {
        mMsgNo = MSG_SAVED_OUTFIT_2;
    } else if (dANpcSpGrace::getPlayerRaw()->isFlag0(
                   PRIVATE_FLAG0_GRACIE_CHECKED)) {
        mMsgNo = MSG_HELLO_CHECKED_2;
    } else {
        mMsgNo = MSG_BUSY;
    }
}

// .text 0xB6C
void dAcNpcSpGrace_c::talk_c::getMsgInfo(msgInfo_s *info) {
    if (mpOwner->mDrama) {
        info->mLabel = "npc_SP/NPC_drama5";
        info->mCode = mMsgNo;
    } else {
        info->mLabel = "npc_SP/NPC_grace";
        info->mCode = mMsgNo;
    }
}

// .text 0xBAC
void dAcNpcSpGrace_c::talk_c::onMessageStart(int arg) {
    if (!mpOwner->mDrama) {
        if (dANpcSpGrace::getPlayer()->mEquipment.mShirt.getColorA() <
                dItem::COLOR_COUNT &&
            dANpcSpGrace::getPlayer()->mEquipment.mShirt.getColorA() !=
                dItem::COLOR_NEUTRAL) {
            int color =
                dANpcSpGrace::getPlayer()->mEquipment.mShirt.getColorA();
            dItem::nameColor_c name;
            setColorName(&name, color);
            mpController->setWord(2, &name);
        }
        if (dANpcSpGrace::getPlayer()->mEquipment.mShirt.getFashion() <
                dItem::FASHION_COUNT &&
            dANpcSpGrace::getPlayer()->mEquipment.mShirt.getFashion() !=
                dItem::FASHION_NONE) {
            int fashion =
                dANpcSpGrace::getPlayer()->mEquipment.mShirt.getFashion();
            dItem::nameFashion_c name;
            setFashionName(&name, fashion);
            mpController->setWord(3, &name);
        }
        if (dANpcSpGrace::getPlayer()->mEquipment.mShirt.getClothStyle() <
                dItem::LOOK_COUNT &&
            dANpcSpGrace::getPlayer()->mEquipment.mShirt.getClothStyle() !=
                dItem::LOOK_NONE) {
            int look =
                dANpcSpGrace::getPlayer()->mEquipment.mShirt.getClothStyle();
            dItem::nameLook_c name;
            setLookName(&name, look);
            mpController->setWord(4, &name);
        }
        if (dSaveData_c::getTown()->getFriendPlayer()->isValid()) {
            mpController->setLandName(
                0, &dSaveData_c::getTown()->getFriendPlayer()->land);
            mpController->setPersonalName(
                1, dSaveData_c::getTown()->getFriendPlayer());
        }
    }
}

// .text 0xD98
void dAcNpcSpGrace_c::talk_c::onMessageEnd(int kind) {
    if (mpOwner->mDrama) {
        switch (mMessageCode) {
        case MSG_DRAMA_TROUBLE:
        case MSG_DRAMA_TROUBLE_2:
            dANpcSpGrace::getPlayer()->setFlag0(
                PRIVATE_FLAG0_DRAMA_GRACIE_HEARD);
            break;
        }
    } else {
        int hatColor = dANpcSpGrace::getPlayer()->mEquipment.mHat.getColorA();
        int hairColor =
            dANpcSpGrace::getHairColor(dANpcSpGrace::getPlayer()->mHairColor);
        int accColor = dANpcSpGrace::getPlayer()->mEquipment.mAcc.getColorA();
        int shirtColor =
            dANpcSpGrace::getPlayer()->mEquipment.mShirt.getColorA();
        int shoeColor =
            dANpcSpGrace::getShoeColor(dANpcSpGrace::getPlayer()->mShoeColor);
        switch (mMessageCode) {
        case MSG_CHECK:
        case MSG_CHECK_2:
        case MSG_CHECK_3:
        case MSG_CHECK_4: {
            dOutfit_c outfit;
            outfit.setFromPlayer();
            dOutfit_c mannequin = dSaveData_c::getExtra()->mOutfit;
            if (mannequin.isValid() && outfit.isSame(&mannequin, TRUE)) {
                mpController->setNextMsgCode(MSG_MANNEQUIN_OUTFIT);
            } else if (dANpcSpGrace::getPlayer()->mMiiMask) {
                mpController->setNextMsgCode(MSG_MII_MASK);
            } else {
                startCheck();
            }
            break;
        }
        case MSG_NO_HAT_ACC:
        case MSG_NO_HAT_ACC_GRACE:
            addPenalty(PENALTY_NO_HAT_ACC);
            switch (judgeBasicColors()) {
            case COLOR_RESULT_OK:
                mpController->setNextMsgCode(MSG_COLOR_OK);
                break;
            case COLOR_RESULT_CLASH:
                mpController->setNextMsgCode(MSG_COLOR_CLASH);
                break;
            case COLOR_RESULT_SINGLE:
                mpController->setNextMsgCode(MSG_ALL_ONE_COLOR);
                break;
            }
            break;
        case MSG_ALL_ONE_COLOR:
            addPenalty(PENALTY_MAJOR);
            finishCheck();
            break;
        case MSG_COLOR_CLASH:
            addPenalty(PENALTY_MAJOR);
            finishCheck();
            break;
        case MSG_COLOR_OK:
            finishCheck();
            break;
        case MSG_FASHION_THEME:
            if (dANpcSpGrace::getPlayer()->mEquipment.mHat.isValid() &&
                dANpcSpGrace::hidesHair(
                    dANpcSpGrace::getPlayer()->mEquipment.mHat.getHideBone())) {
                if (shirtColor != dItem::COLOR_NEUTRAL &&
                    (hatColor == dItem::COLOR_NEUTRAL ||
                     shirtColor == hatColor) &&
                    (accColor == dItem::COLOR_NEUTRAL ||
                     shirtColor == accColor)) {
                    switch (judgeShoeShirt()) {
                    case 0:
                        mpController->setNextMsgCode(MSG_SHOES_OK);
                        break;
                    case 1:
                        mpController->setNextMsgCode(MSG_THEME_CLASH);
                        break;
                    }
                } else {
                    switch (judgeShoeColor()) {
                    case 0:
                        mpController->setNextMsgCode(MSG_SHOES_OK);
                        break;
                    case 1:
                        mpController->setNextMsgCode(MSG_THEME_CLASH);
                        break;
                    }
                }
            } else if (shirtColor != dItem::COLOR_NEUTRAL &&
                       (!dANpcSpGrace::getPlayer()->mEquipment.mHat.isValid() ||
                        shirtColor == hatColor) &&
                       (!dANpcSpGrace::getPlayer()->mEquipment.mAcc.isValid() ||
                        shirtColor == accColor) &&
                       shirtColor == hairColor && shirtColor == shoeColor) {
                mpController->setNextMsgCode(MSG_THEME_ONE_COLOR);
            } else {
                mpController->setNextMsgCode(MSG_TO_THEME_HAIR);
            }
            break;
        case MSG_ONE_COLOR:
            addPenalty(PENALTY_MAJOR);
            finishCheck();
            break;
        case MSG_THEME_CLASH:
            addPenalty(PENALTY_MINOR);
            finishCheck();
            break;
        case MSG_THEME_ONE_COLOR:
            mOneColorTold = 1;
            addPenalty(PENALTY_MAJOR);
            mpController->setNextMsgCode(MSG_TO_THEME_HAIR);
            break;
        case MSG_THEME_HAIR:
            addPenalty(PENALTY_MINOR);
            mpController->setNextMsgCode(MSG_TO_SHOES);
            break;
        case MSG_THEME_HAIR_2:
            addPenalty(PENALTY_MINOR);
            mpController->setNextMsgCode(MSG_TO_SHOES);
            break;
        case MSG_THEME_SHOES:
            addPenalty(PENALTY_MINOR);
            finishCheck();
            break;
        case MSG_THEME_SHOES_2:
            addPenalty(PENALTY_MINOR);
            finishCheck();
            break;
        case MSG_LOOK_THEME:
            mLookTheme = 1;
            mpController->setNextMsgCode(MSG_TO_COLORS);
            break;
        case MSG_LOOK_SAFE:
            mpController->setNextMsgCode(MSG_TO_COLORS);
            break;
        case MSG_LOOK_NONE:
            mNoLook = 1;
            mpController->setNextMsgCode(MSG_TO_COLORS);
            addPenalty(PENALTY_MAJOR);
            break;
        case MSG_ONE_COLOR_2:
            addPenalty(PENALTY_MAJOR);
            finishCheck();
            break;
        case MSG_ONE_COLOR_3:
            addPenalty(PENALTY_MAJOR);
            finishCheck();
            break;
        case MSG_SHOES:
            addPenalty(PENALTY_MINOR);
            finishCheck();
            break;
        case MSG_SHOES_2:
            addPenalty(PENALTY_MINOR);
            finishCheck();
            break;
        case MSG_COLOR_CLASH_2:
            addPenalty(PENALTY_MAJOR);
            finishCheck();
            break;
        case MSG_COLOR_CLASH_3:
            addPenalty(PENALTY_MAJOR);
            finishCheck();
            break;
        case MSG_ONE_COLOR_4:
            mpController->setNextMsgCode(MSG_TO_HAIR_THEME);
            addPenalty(PENALTY_MAJOR);
            break;
        case MSG_COLOR_FINE:
            mpController->setNextMsgCode(MSG_TO_HAIR_THEME);
            addPenalty(PENALTY_MINOR);
            break;
        case MSG_COLOR_FINE_2:
            mpController->setNextMsgCode(MSG_TO_HAIR_THEME);
            addPenalty(PENALTY_MINOR);
            break;
        case MSG_COLOR_FINE_3:
            addPenalty(PENALTY_MINOR);
            finishCheck();
            break;
        case MSG_COLOR_FINE_4:
            mpController->setNextMsgCode(MSG_TO_HAIR);
            addPenalty(PENALTY_MINOR);
            break;
        case MSG_HAIR:
            mpController->setNextMsgCode(MSG_TO_RESULT);
            addPenalty(PENALTY_MINOR);
            break;
        case MSG_HAIR_2:
            mpController->setNextMsgCode(MSG_TO_RESULT);
            addPenalty(PENALTY_MINOR);
            break;
        case MSG_SHOES_3:
            addPenalty(PENALTY_MINOR);
            finishCheck();
            break;
        case MSG_SHOES_4:
            addPenalty(PENALTY_MINOR);
            finishCheck();
            break;
        case MSG_COLOR_WAR:
            addPenalty(PENALTY_MAJOR);
            finishCheck();
            break;
        case MSG_COLOR_WAR_2:
            addPenalty(PENALTY_MAJOR);
            if (!isHairShown()) {
                finishCheck();
            } else if (!judgeHair()) {
                finishCheck();
            } else {
                mpController->setNextMsgCode(MSG_HAIR_3);
            }
            break;
        case MSG_HAIR_3:
            addPenalty(PENALTY_MINOR);
            finishCheck();
            break;
        case MSG_SHOES_OK:
        case MSG_SHOES_OK_2:
            if (mHairNext) {
                mpController->setNextMsgCode(MSG_TO_HAIR);
            } else if (mHairNextTheme) {
                mpController->setNextMsgCode(MSG_TO_HAIR_THEME);
            } else {
                finishCheck();
            }
            break;
        case MSG_TO_RESULT:
            finishCheck();
            break;
        case MSG_TO_HAIR:
            if (mNoLook) {
                finishCheck();
            } else if (!isHairShown()) {
                finishCheck();
            } else if (!judgeHair()) {
                mpController->setNextMsgCode(MSG_TO_RESULT);
            } else if (mPenalty > 0) {
                mpController->setNextMsgCode(MSG_HAIR);
            } else {
                mpController->setNextMsgCode(MSG_HAIR_2);
            }
            break;
        case MSG_TO_HAIR_THEME:
            if (mLookTheme) {
                mpController->setNextMsgCode(MSG_TO_HAIR);
            } else {
                mpController->setNextMsgCode(MSG_TO_RESULT);
            }
            break;
        case MSG_TO_COLORS:
            if (dANpcSpGrace::getPlayer()->mEquipment.mHat.isValid() &&
                dANpcSpGrace::hidesHair(
                    dANpcSpGrace::getPlayer()->mEquipment.mHat.getHideBone())) {
                switch (judgeHiddenHairColors()) {
                case COLOR_RESULT_OK:
                    if (!judgeShoeColor()) {
                        if (mPenalty <= 0) {
                            mpController->setNextMsgCode(MSG_SHOES_OK);
                        } else {
                            mpController->setNextMsgCode(MSG_SHOES_OK_2);
                        }
                    } else if (mNoLook) {
                        mpController->setNextMsgCode(MSG_SHOES);
                    } else {
                        mpController->setNextMsgCode(MSG_SHOES_2);
                    }
                    break;
                case COLOR_RESULT_CLASH:
                    if (mNoLook) {
                        mpController->setNextMsgCode(MSG_COLOR_CLASH_2);
                    } else {
                        mpController->setNextMsgCode(MSG_COLOR_CLASH_3);
                    }
                    break;
                case COLOR_RESULT_SINGLE:
                    if (countColors(FALSE, FALSE, FALSE, TRUE) <= 1) {
                        if (mNoLook) {
                            mpController->setNextMsgCode(MSG_ONE_COLOR_2);
                        } else {
                            mpController->setNextMsgCode(MSG_ONE_COLOR_3);
                        }
                    } else if (mPenalty <= 0) {
                        mpController->setNextMsgCode(MSG_SHOES_OK);
                    } else {
                        mpController->setNextMsgCode(MSG_SHOES_OK_2);
                    }
                    break;
                }
            } else {
                switch (judgeColors()) {
                case COLOR_RESULT_OK:
                    if (!judgeShoeColor()) {
                        if (mPenalty <= 0) {
                            mpController->setNextMsgCode(MSG_SHOES_OK);
                        } else {
                            mpController->setNextMsgCode(MSG_SHOES_OK_2);
                        }
                        mHairNext = 1;
                    } else if (mNoLook) {
                        mpController->setNextMsgCode(MSG_COLOR_FINE_3);
                    } else {
                        mpController->setNextMsgCode(MSG_COLOR_FINE_4);
                    }
                    break;
                case COLOR_RESULT_CLASH:
                    if (mNoLook) {
                        mpController->setNextMsgCode(MSG_COLOR_WAR);
                    } else {
                        mpController->setNextMsgCode(MSG_COLOR_WAR_2);
                    }
                    break;
                case COLOR_RESULT_SINGLE:
                    if (countColors(FALSE, FALSE, FALSE, TRUE) <= 1) {
                        mpController->setNextMsgCode(MSG_ONE_COLOR_4);
                    } else {
                        if (mPenalty <= 0) {
                            mpController->setNextMsgCode(MSG_SHOES_OK);
                        } else {
                            mpController->setNextMsgCode(MSG_SHOES_OK_2);
                        }
                        mHairNextTheme = 1;
                    }
                    break;
                }
            }
            break;
        case MSG_TO_SHOES:
            if (mOneColorTold) {
                if (mPenalty <= 0) {
                    mpController->setNextMsgCode(MSG_SHOES_OK);
                } else {
                    mpController->setNextMsgCode(MSG_SHOES_OK_2);
                }
            } else if (!judgeShoeColor()) {
                if (mPenalty <= 0) {
                    mpController->setNextMsgCode(MSG_SHOES_OK);
                } else {
                    mpController->setNextMsgCode(MSG_SHOES_OK_2);
                }
            } else if (mPenalty > 0) {
                mpController->setNextMsgCode(MSG_THEME_SHOES);
            } else {
                mpController->setNextMsgCode(MSG_THEME_SHOES_2);
            }
            break;
        case MSG_TO_THEME_HAIR:
            if (!judgeHair()) {
                mpController->setNextMsgCode(MSG_TO_SHOES);
            } else if (mPenalty > 0) {
                mpController->setNextMsgCode(MSG_THEME_HAIR);
            } else {
                mpController->setNextMsgCode(MSG_THEME_HAIR_2);
            }
            break;
        }
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

namespace dANpcSpGrace {
// .text 0x1910: both items have the same fashion theme.
static BOOL isSameFashion(const dItem::Item *a, const dItem::Item *b) {
    if (*a == dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    if (*b == dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    int fashionA = a->getFashion();
    int fashionB = b->getFashion();
    if (fashionA == dItem::FASHION_NONE) {
        return FALSE;
    }
    if (fashionB == dItem::FASHION_NONE) {
        return FALSE;
    }
    return fashionA == fashionB;
}

// .text 0x19A4: both items have the same look (getClothStyle).
static BOOL isSameLook(const dItem::Item *a, const dItem::Item *b) {
    if (*a == dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    if (*b == dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    int lookA = a->getClothStyle();
    int lookB = b->getClothStyle();
    if (lookA == dItem::LOOK_NONE) {
        return FALSE;
    }
    if (lookB == dItem::LOOK_NONE) {
        return FALSE;
    }
    return lookA == lookB;
}
} // namespace dANpcSpGrace

// .text 0x1A38
int dAcNpcSpGrace_c::talk_c::countColors(BOOL hat, BOOL hair, BOOL acc,
                                         BOOL shoes) {
    int counts[dItem::COLOR_COUNT];
    for (int i = 0; i < dItem::COLOR_COUNT; i++) {
        counts[i] = 0;
    }
    if (dANpcSpGrace::getPlayer()->mEquipment.mShirt.getColorA() <
        dItem::COLOR_COUNT) {
        counts[dANpcSpGrace::getPlayer()->mEquipment.mShirt.getColorA()]++;
    }
    if (hat && dANpcSpGrace::getPlayer()->mEquipment.mHat.isValid() &&
        dANpcSpGrace::getPlayer()->mEquipment.mHat.getColorA() <
            dItem::COLOR_COUNT) {
        counts[dANpcSpGrace::getPlayer()->mEquipment.mHat.getColorA()]++;
    }
    if (hair) {
        counts[dANpcSpGrace::getHairColor(
            dANpcSpGrace::getPlayer()->mHairColor)]++;
    }
    if (acc && dANpcSpGrace::getPlayer()->mEquipment.mAcc.isValid() &&
        dANpcSpGrace::getPlayer()->mEquipment.mAcc.getColorA() <
            dItem::COLOR_COUNT) {
        counts[dANpcSpGrace::getPlayer()->mEquipment.mAcc.getColorA()]++;
    }
    if (shoes) {
        counts[dANpcSpGrace::getShoeColor(
            dANpcSpGrace::getPlayer()->mShoeColor)]++;
    }
    int num = 0;
    for (int i = 0; i < dItem::COLOR_COUNT; i++) {
        if (i != dItem::COLOR_NEUTRAL && counts[i] > 0) {
            num++;
        }
    }
    return num;
}

// .text 0x1C80
void dAcNpcSpGrace_c::talk_c::startCheck() {
    int hatColor = dItem::COLOR_NEUTRAL;
    if (dANpcSpGrace::getPlayer()->mEquipment.mHat.isValid()) {
        hatColor = dANpcSpGrace::getPlayer()->mEquipment.mHat.getColorA();
    }
    int hairColor =
        dANpcSpGrace::getHairColor(dANpcSpGrace::getPlayer()->mHairColor);
    int accColor = dItem::COLOR_NEUTRAL;
    if (dANpcSpGrace::getPlayer()->mEquipment.mAcc.isValid()) {
        accColor = dANpcSpGrace::getPlayer()->mEquipment.mAcc.getColorA();
    }
    int shirtColor = dANpcSpGrace::getPlayer()->mEquipment.mShirt.getColorA();
    int shoeColor =
        dANpcSpGrace::getShoeColor(dANpcSpGrace::getPlayer()->mShoeColor);

    dItem::Item shirt = dANpcSpGrace::getPlayer()->mEquipment.mShirt;
    dItem::Item hat = dANpcSpGrace::getPlayer()->mEquipment.mHat;
    dItem::Item acc = dANpcSpGrace::getPlayer()->mEquipment.mAcc;
    if (hat.isValid() || acc.isValid()) {
        if (dANpcSpGrace::isSameFashion(&shirt, &hat) ||
            dANpcSpGrace::isSameFashion(&shirt, &acc)) {
            mpController->setNextMsgCode(MSG_FASHION_THEME);
        } else if ((!hat.isValid() || dANpcSpGrace::isSameLook(&shirt, &hat)) &&
                   (!acc.isValid() || dANpcSpGrace::isSameLook(&shirt, &acc))) {
            mpController->setNextMsgCode(MSG_LOOK_THEME);
        } else if (dANpcSpGrace::isSameLook(&shirt, &hat) ||
                   dANpcSpGrace::isSameLook(&shirt, &acc) ||
                   dANpcSpGrace::isSameLook(&hat, &acc)) {
            mpController->setNextMsgCode(MSG_LOOK_SAFE);
        } else {
            mpController->setNextMsgCode(MSG_LOOK_NONE);
        }
    } else if (shirt.getFrom() == dItem::FROM_GRACE_SPR ||
               shirt.getFrom() == dItem::FROM_GRACE_SUM ||
               shirt.getFrom() == dItem::FROM_GRACE_AUT ||
               shirt.getFrom() == dItem::FROM_GRACE_WIN) {
        mpController->setNextMsgCode(MSG_NO_HAT_ACC_GRACE);
    } else {
        mpController->setNextMsgCode(MSG_NO_HAT_ACC);
    }
    mChecking = 1;
}

// .text 0x1E8C
void dAcNpcSpGrace_c::talk_c::finishCheck() {
    if (mPenalty <= 0) {
        if (dANpcSpGrace::getPlayerRaw()->isFlag0(
                PRIVATE_FLAG0_GRACIE_PERFECT)) {
            mpController->setNextMsgCode(MSG_RESULT_PERFECT_AGAIN);
        } else {
            mpController->setNextMsgCode(MSG_RESULT_PERFECT);
        }
        dANpcSpGrace::getPlayer()->setFlag0(PRIVATE_FLAG0_GRACIE_PERFECT);
        dANpcSpGrace::getPlayer()->setFlag1(PRIVATE_FLAG1_GRACIE_DISCOUNT);
    } else if (mPenalty <= 1) {
        mpController->setNextMsgCode(MSG_RESULT_1);
    } else if (mPenalty <= 2) {
        mpController->setNextMsgCode(MSG_RESULT_2);
    } else if (mPenalty <= 3) {
        mpController->setNextMsgCode(MSG_RESULT_3);
    } else if (mPenalty <= 4) {
        mpController->setNextMsgCode(MSG_RESULT_4);
    } else {
        mpController->setNextMsgCode(MSG_RESULT_5);
    }
    dANpcSpGrace::getPlayer()->setFlag0(PRIVATE_FLAG0_GRACIE_CHECKED);
    dANpcSpGrace::getPlayer()->setFlag1(PRIVATE_FLAG1_GRACIE_CHECKED);
    dANpcSpGrace::getPlayer()->setFlag1(PRIVATE_FLAG1_GRACIE_CHECKED_HERE);
    dPrivateData_c *player = dANpcSpGrace::getPlayer();
    if (player->mGracieChecks < CHECKS_NUM) {
        player->mGracieChecks++;
    }
    mChecking = 0;
    recordOutfit();
}

// .text 0x1FC8
void dAcNpcSpGrace_c::talk_c::onAnswer(int arg) {
    int cursor = mpController->mSelect.mCursor;
    if (!mpOwner->mDrama) {
        switch (mMessageCode) {
        case MSG_INTRO:
        case MSG_INTRO_2:
            switch (cursor) {
            case ANSWER_ADVICE:
                if (dANpcSpGrace::getPlayer()->isBedHeadVisible()) {
                    mpController->setNextMsgCode(MSG_CHECK_BED_HEAD);
                } else {
                    mpController->setNextMsgCode(MSG_CHECK);
                }
                break;
            case ANSWER_SHOP:
                if (dSaveData_c::getTown()->getFriendPlayer()->isValid() &&
                    !(*dSaveData_c::getTown()->getFriendPlayer() ==
                      dANpcSpGrace::getPlayerRaw()->mPID)) {
                    mFriendMentioned = 1;
                    mpController->setNextMsgCode(MSG_FRIEND_STYLE);
                } else {
                    mpController->setNextMsgCode(MSG_SHOP);
                }
                break;
            }
            break;
        case MSG_HELLO_CHECKED:
            switch (cursor) {
            case ANSWER_ADVICE:
                if (dANpcSpGrace::getPlayer()->isBedHeadVisible()) {
                    mpController->setNextMsgCode(MSG_CHECK_BED_HEAD_2);
                } else {
                    mpController->setNextMsgCode(MSG_CHECK_2);
                }
                break;
            case ANSWER_SHOP:
                if (dSaveData_c::getTown()->getFriendPlayer()->isValid() &&
                    !(dSaveData_c::getTown()->mFriendPlayer ==
                      dANpcSpGrace::getPlayerRaw()->mPID)) {
                    mFriendMentioned = 1;
                    mpController->setNextMsgCode(MSG_FRIEND_STYLE_2);
                } else {
                    mpController->setNextMsgCode(MSG_SHOP_2);
                }
                break;
            }
            break;
        case MSG_HELLO:
            switch (cursor) {
            case ANSWER_ADVICE:
                if (dANpcSpGrace::getPlayer()->isBedHeadVisible()) {
                    mpController->setNextMsgCode(MSG_CHECK_BED_HEAD_3);
                } else {
                    mpController->setNextMsgCode(MSG_CHECK_3);
                }
                break;
            case ANSWER_SHOP:
                mpController->setNextMsgCode(MSG_FAN);
                break;
            }
            break;
        case MSG_HELLO_CHECKED_2:
            switch (cursor) {
            case ANSWER_ADVICE:
                break;
            case ANSWER_SHOP:
                if (dSaveData_c::getTown()->getFriendPlayer()->isValid() &&
                    !(*dSaveData_c::getTown()->getFriendPlayer() ==
                      dANpcSpGrace::getPlayerRaw()->mPID)) {
                    mFriendMentioned = 1;
                    mpController->setNextMsgCode(MSG_FRIEND_STYLE_3);
                } else {
                    mpController->setNextMsgCode(MSG_SHOP_3);
                }
                break;
            }
            break;
        case MSG_BUSY:
            switch (cursor) {
            case ANSWER_ADVICE:
                break;
            case ANSWER_SHOP:
                if (dSaveData_c::getTown()->getFriendPlayer()->isValid() &&
                    !(*dSaveData_c::getTown()->getFriendPlayer() ==
                      dANpcSpGrace::getPlayerRaw()->mPID)) {
                    mFriendMentioned = 1;
                    mpController->setNextMsgCode(MSG_FRIEND_STYLE_4);
                } else {
                    mpController->setNextMsgCode(MSG_SHOP_4);
                }
                break;
            }
            break;
        }
    }
    if (mOnAnswer) {
        (this->*mOnAnswer)();
        mOnAnswer = NULL;
    }
}

// .text 0x2438
void dAcNpcSpGrace_c::talk_c::preExecute() {
}

// .text 0x243C
void dAcNpcSpGrace_c::talk_c::onTalkEnd() {
    if (mpOwner->mDrama) {
        mDramaTalked = 1;
    } else {
        dANpcSpGrace::getPlayer()->setFlag0(PRIVATE_FLAG0_GRACIE_MET);
        dANpcSpGrace::getPlayer()->setFlag1(PRIVATE_FLAG1_GRACIE_TALKED);
        if (mChecking) {
            mChecking = 0;
        }
    }
    if (mFriendMentioned) {
        dSaveData_c::getTown()->getFriendPlayer()->clear();
    }
}

// .text 0x24C8
void dAcNpcSpGrace_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x252C
int dAcNpcSpGrace_c::talk_c::judgeBasicColors() {
    mBasicColorsJudged = 1;
    int shirtColor = dANpcSpGrace::getPlayer()->mEquipment.mShirt.getColorA();
    int hairColor =
        dANpcSpGrace::getHairColor(dANpcSpGrace::getPlayer()->mHairColor);
    int shoeColor =
        dANpcSpGrace::getShoeColor(dANpcSpGrace::getPlayer()->mShoeColor);
    if (dANpcSpGrace::getPlayer()->mMiiMask) {
        mBasicColorsCase = 1;
        return COLOR_RESULT_OK;
    }
    if (shirtColor != dItem::COLOR_NEUTRAL && shirtColor == hairColor &&
        shirtColor == shoeColor) {
        mBasicColorsCase = 2;
        return COLOR_RESULT_SINGLE;
    }
    if (countColors(FALSE, TRUE, FALSE, TRUE) <= 2) {
        mBasicColorsCase = 3;
        return COLOR_RESULT_OK;
    }
    if (shirtColor == dItem::COLOR_NEUTRAL) {
        mBasicColorsCase = 4;
        return COLOR_RESULT_OK;
    }
    if (hairColor == dItem::COLOR_BLACK || hairColor == dItem::COLOR_BROWN ||
        hairColor == dItem::COLOR_YELLOW) {
        mBasicColorsCase = 5;
        return COLOR_RESULT_OK;
    }
    mBasicColorsCase = 6;
    return COLOR_RESULT_CLASH;
}

// .text 0x2660
int dAcNpcSpGrace_c::talk_c::judgeShoeColor() {
    mShoeColorJudged = 1;
    int hatColor = dANpcSpGrace::getPlayer()->mEquipment.mHat.getColorA();
    int accColor = dANpcSpGrace::getPlayer()->mEquipment.mAcc.getColorA();
    int shirtColor = dANpcSpGrace::getPlayer()->mEquipment.mShirt.getColorA();
    int shoeColor =
        dANpcSpGrace::getShoeColor(dANpcSpGrace::getPlayer()->mShoeColor);
    if ((hatColor != dItem::COLOR_NEUTRAL && shoeColor == hatColor) ||
        (accColor != dItem::COLOR_NEUTRAL && shoeColor == accColor) ||
        (shirtColor != dItem::COLOR_NEUTRAL && shoeColor == shirtColor)) {
        mShoeColorMatch = 1;
        return 0;
    }
    mShoeColorMatch = 0;
    return 1;
}

// .text 0x2734
int dAcNpcSpGrace_c::talk_c::judgeHiddenHairColors() {
    mHiddenHairColorsJudged = 1;
    int shirtColor = dANpcSpGrace::getPlayer()->mEquipment.mShirt.getColorA();
    int hatColor = dANpcSpGrace::getPlayer()->mEquipment.mHat.getColorA();
    int accColor = dItem::COLOR_NEUTRAL;
    if (dANpcSpGrace::getPlayer()->mEquipment.mAcc.isValid()) {
        accColor = dANpcSpGrace::getPlayer()->mEquipment.mAcc.getColorA();
    }
    if (shirtColor != dItem::COLOR_NEUTRAL && shirtColor == hatColor &&
        (!dANpcSpGrace::getPlayer()->mEquipment.mAcc.isValid() ||
         shirtColor == accColor)) {
        mHiddenHairColorsCase = 1;
        return COLOR_RESULT_SINGLE;
    }
    if (!dANpcSpGrace::getPlayer()->mEquipment.mAcc.isValid()) {
        mHiddenHairColorsCase = 2;
        return COLOR_RESULT_OK;
    }
    if ((shirtColor != dItem::COLOR_NEUTRAL && shirtColor == hatColor) ||
        (shirtColor != dItem::COLOR_NEUTRAL && shirtColor == accColor) ||
        (hatColor != dItem::COLOR_NEUTRAL && hatColor == accColor)) {
        mHiddenHairColorsCase = 3;
        return COLOR_RESULT_OK;
    }
    if (shirtColor == dItem::COLOR_NEUTRAL ||
        hatColor == dItem::COLOR_NEUTRAL || accColor == dItem::COLOR_NEUTRAL) {
        mHiddenHairColorsCase = 4;
        return COLOR_RESULT_OK;
    }
    mHiddenHairColorsCase = 5;
    return COLOR_RESULT_CLASH;
}

// .text 0x288C
int dAcNpcSpGrace_c::talk_c::judgeColors() {
    mColorsJudged = 1;
    int hatColor = dANpcSpGrace::getPlayer()->mEquipment.mHat.getColorA();
    int hairColor =
        dANpcSpGrace::getHairColor(dANpcSpGrace::getPlayer()->mHairColor);
    int accColor = dANpcSpGrace::getPlayer()->mEquipment.mAcc.getColorA();
    int shirtColor = dANpcSpGrace::getPlayer()->mEquipment.mShirt.getColorA();
    mMainColorCount = 0;
    mColorCount = 0;
    mMainColor = dItem::COLOR_COUNT;
    int counts[dItem::COLOR_COUNT];
    for (int i = 0; i < dItem::COLOR_COUNT; i++) {
        counts[i] = 0;
    }
    if (dANpcSpGrace::getPlayer()->mEquipment.mShirt.getColorA() <
        dItem::COLOR_COUNT) {
        counts[dANpcSpGrace::getPlayer()->mEquipment.mShirt.getColorA()]++;
        mColorCount++;
    }
    if (dANpcSpGrace::getPlayer()->mEquipment.mHat.isValid() &&
        dANpcSpGrace::getPlayer()->mEquipment.mHat.getColorA() <
            dItem::COLOR_COUNT) {
        counts[dANpcSpGrace::getPlayer()->mEquipment.mHat.getColorA()]++;
        mColorCount++;
    }
    if (!dANpcSpGrace::getPlayer()->mEquipment.mHat.isValid() ||
        !dANpcSpGrace::hidesHair(
            dANpcSpGrace::getPlayer()->mEquipment.mHat.getHideBone())) {
        counts[dANpcSpGrace::getHairColor(
            dANpcSpGrace::getPlayer()->mHairColor)]++;
        mColorCount++;
    }
    if (dANpcSpGrace::getPlayer()->mEquipment.mAcc.isValid() &&
        dANpcSpGrace::getPlayer()->mEquipment.mAcc.getColorA() <
            dItem::COLOR_COUNT) {
        counts[dANpcSpGrace::getPlayer()->mEquipment.mAcc.getColorA()]++;
        mColorCount++;
    }
    for (int i = 0; i < dItem::COLOR_COUNT; i++) {
        if (i != dItem::COLOR_NEUTRAL && counts[i] > mMainColorCount) {
            mMainColorCount = counts[i];
            mMainColor = i;
        }
    }
    if (shirtColor != dItem::COLOR_NEUTRAL && shirtColor == hairColor &&
        (!dANpcSpGrace::getPlayer()->mEquipment.mHat.isValid() ||
         shirtColor == hatColor) &&
        (!dANpcSpGrace::getPlayer()->mEquipment.mAcc.isValid() ||
         shirtColor == accColor)) {
        mColorsCase = 1;
        return COLOR_RESULT_SINGLE;
    }
    if (dANpcSpGrace::getPlayer()->mEquipment.mHat.isValid() &&
        hairColor == hatColor) {
        mColorsCase = 2;
        return COLOR_RESULT_CLASH;
    }
    if (mMainColorCount * 2 >= mColorCount) {
        mColorsCase = 3;
        return COLOR_RESULT_OK;
    }
    if (hairColor == dItem::COLOR_NEUTRAL ||
        (dANpcSpGrace::getPlayer()->mEquipment.mHat.isValid() &&
         hatColor == dItem::COLOR_NEUTRAL) ||
        (dANpcSpGrace::getPlayer()->mEquipment.mAcc.isValid() &&
         accColor == dItem::COLOR_NEUTRAL &&
         shirtColor == dItem::COLOR_NEUTRAL)) {
        mColorsCase = 4;
        return COLOR_RESULT_OK;
    }
    if (hairColor == dItem::COLOR_BLACK || hairColor == dItem::COLOR_BROWN ||
        hairColor == dItem::COLOR_YELLOW) {
        mColorsCase = 5;
        return COLOR_RESULT_OK;
    }
    if (countColors(TRUE, TRUE, TRUE, FALSE) <= 2) {
        mColorsCase = 6;
        return COLOR_RESULT_OK;
    }
    mColorsCase = 7;
    return COLOR_RESULT_CLASH;
}

// .text 0x2CB4
int dAcNpcSpGrace_c::talk_c::judgeHair() {
    mHairJudged = 1;
    if ((dANpcSpGrace::getPlayer()->mEquipment.mHat.isValid() &&
         dANpcSpGrace::getPlayer()->mEquipment.mHat.getClothStyle() !=
             dItem::LOOK_NONE &&
         dPlayerMgr::currentHairHasFeature(
             dANpcSpGrace::getPlayer()->mEquipment.mHat.getClothStyle())) ||
        (dANpcSpGrace::getPlayer()->mEquipment.mAcc.isValid() &&
         dANpcSpGrace::getPlayer()->mEquipment.mAcc.getClothStyle() !=
             dItem::LOOK_NONE &&
         dPlayerMgr::currentHairHasFeature(
             dANpcSpGrace::getPlayer()->mEquipment.mAcc.getClothStyle())) ||
        (dANpcSpGrace::getPlayer()->mEquipment.mShirt.isValid() &&
         dANpcSpGrace::getPlayer()->mEquipment.mShirt.getClothStyle() !=
             dItem::LOOK_NONE &&
         dPlayerMgr::currentHairHasFeature(
             dANpcSpGrace::getPlayer()->mEquipment.mShirt.getClothStyle()))) {
        mHairFits = 1;
        return 0;
    }
    mHairFits = 0;
    return 1;
}

// .text 0x2DB4
int dAcNpcSpGrace_c::talk_c::judgeShoeShirt() {
    mShoeShirtJudged = 1;
    int hatColor = dANpcSpGrace::getPlayer()->mEquipment.mHat.getColorA();
    int accColor = dANpcSpGrace::getPlayer()->mEquipment.mAcc.getColorA();
    int shirtColor = dANpcSpGrace::getPlayer()->mEquipment.mShirt.getColorA();
    int shoeColor =
        dANpcSpGrace::getShoeColor(dANpcSpGrace::getPlayer()->mShoeColor);
    switch (shirtColor) {
    case dItem::COLOR_WHITE:
        if (dANpcSpGrace::getPlayer()->mShoeColor != 5) {
            mShoesMatchShirt = 1;
            return 0;
        }
        mShoesMatchShirt = 0;
        return 1;
    case dItem::COLOR_ORANGE:
    case dItem::COLOR_GREEN:
    case dItem::COLOR_BEIGE:
        if (dANpcSpGrace::getPlayer()->mShoeColor == 8 ||
            dANpcSpGrace::getPlayer()->mShoeColor == 0) {
            mShoesMatchShirt = 1;
            return 0;
        }
        mShoesMatchShirt = 0;
        return 1;
    case dItem::COLOR_YELLOW:
        if (dANpcSpGrace::getPlayer()->mShoeColor == 2 ||
            dANpcSpGrace::getPlayer()->mShoeColor == 3) {
            mShoesMatchShirt = 1;
            return 0;
        }
        mShoesMatchShirt = 0;
        return 1;
    case dItem::COLOR_RED:
        if (dANpcSpGrace::getPlayer()->mShoeColor == 9 ||
            dANpcSpGrace::getPlayer()->mShoeColor == 6) {
            mShoesMatchShirt = 1;
            return 0;
        }
        mShoesMatchShirt = 0;
        return 1;
    case dItem::COLOR_BLUE:
    case dItem::COLOR_BLACK:
        if (dANpcSpGrace::getPlayer()->mShoeColor == 6 ||
            dANpcSpGrace::getPlayer()->mShoeColor == 5) {
            mShoesMatchShirt = 1;
            return 0;
        }
        mShoesMatchShirt = 0;
        return 1;
    case dItem::COLOR_PURPLE:
        if (dANpcSpGrace::getPlayer()->mShoeColor == 4 ||
            dANpcSpGrace::getPlayer()->mShoeColor == 9) {
            mShoesMatchShirt = 1;
            return 0;
        }
        mShoesMatchShirt = 0;
        return 1;
    case dItem::COLOR_BROWN:
        if (dANpcSpGrace::getPlayer()->mShoeColor == 2 ||
            dANpcSpGrace::getPlayer()->mShoeColor == 3) {
            mShoesMatchShirt = 1;
            return 0;
        }
        mShoesMatchShirt = 0;
        return 1;
    case dItem::COLOR_PINK:
        if (dANpcSpGrace::getPlayer()->mShoeColor == 1 ||
            dANpcSpGrace::getPlayer()->mShoeColor == 5) {
            mShoesMatchShirt = 1;
            return 0;
        }
        mShoesMatchShirt = 0;
        return 1;
    case dItem::COLOR_GRAY:
        if (dANpcSpGrace::getPlayer()->mShoeColor == 6 ||
            dANpcSpGrace::getPlayer()->mShoeColor == 7) {
            mShoesMatchShirt = 1;
            return 0;
        }
        mShoesMatchShirt = 0;
        return 1;
    case dItem::COLOR_AQUA:
        if (dANpcSpGrace::getPlayer()->mShoeColor == 4 ||
            dANpcSpGrace::getPlayer()->mShoeColor == 7) {
            mShoesMatchShirt = 1;
            return 0;
        }
        mShoesMatchShirt = 0;
        return 1;
    case dItem::COLOR_NEUTRAL:
    case dItem::COLOR_COLORFUL:
        mShoesMatchShirt = 1;
        return 0;
    }
    mShoesMatchShirt = 1;
    return 0;
}

// .text 0x311C
BOOL dAcNpcSpGrace_c::talk_c::isHairShown() {
    dItem::Item hat = dANpcSpGrace::getPlayer()->mEquipment.mHat;
    if (hat == dItem::ITEM_ID_NONE) {
        return TRUE;
    }
    switch (hat.getHideBone()) {
    case dItem::HIDE_BONE_CAP:
    case dItem::HIDE_BONE_FULL_MASK:
    case 3:
    case 4:
    case 5:
    case 6:
        return FALSE;
    }
    return TRUE;
}

// .text 0x3188
BOOL dAcNpcSpGrace_c::talk_c::isSavedOutfit() {
    dOutfit_c outfit;
    outfit.setFromPlayer();
    for (int i = 0; i < PLAYER_OUTFIT_COUNT; i++) {
        dOutfit_c saved = *dANpcSpGrace::getPlayer()->getOutfit(i);
        if (saved.isValid() && outfit.isSame(&saved, FALSE)) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text 0x3274
void dAcNpcSpGrace_c::talk_c::recordOutfit() {
    dSaveTown_c *town = dSaveData_c::getTown();
    town->mVillagerMove.setPlayerBit(dPlayerMgr::getCurrentPlayerSlot(),
                                     mPenalty == 0);
    if (mPenalty <= 0) {
        int slot = -1;
        for (int i = 0; i < PLAYER_OUTFIT_COUNT; i++) {
            if (!dANpcSpGrace::getPlayer()->getOutfit(i)->isValid()) {
                slot = i;
                break;
            }
        }
        if (slot < 0) {
            for (slot = 1; slot < PLAYER_OUTFIT_COUNT; slot++) {
                dOutfit_c *next = dANpcSpGrace::getPlayer()->getOutfit(slot);
                dANpcSpGrace::getPlayer()->getOutfit(slot - 1)->copy(next);
            }
            slot = PLAYER_OUTFIT_COUNT - 1;
        }
        dANpcSpGrace::getPlayer()->getOutfit(slot)->setFromPlayer();
        if ((!dANpcSpGrace::getPlayer()->mEquipment.mShirt.isValid() ||
             dANpcSpGrace::getPlayer()->mEquipment.mShirt.getKind() !=
                 dItem::KIND_ORG_CLOTH) &&
            (!dANpcSpGrace::getPlayer()->mEquipment.mHat.isValid() ||
             dANpcSpGrace::getPlayer()->mEquipment.mHat.getKind() !=
                 dItem::KIND_ORG_CAP)) {
            town = dSaveData_c::getTown();
            town->mVillagerMove.setOutfit(
                dANpcSpGrace::getPlayer()->getOutfit(slot));
        }
        if (!dSaveData_c::getTown()->getFriendPlayer()->isValid()) {
            *dSaveData_c::getTown()->getFriendPlayer() =
                dANpcSpGrace::getPlayer()->mPID;
        }
    }
}

// .text 0x34A0
void dAcNpcSpGrace_c::talk_c::addPenalty(int points) {
    mPenalty += points;
}

// .text 0x34B0
int dAcNpcSpGrace_c::getEarType() {
    return NPC_EAR_TYPE_GRACIE;
}
