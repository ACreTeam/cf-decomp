// The villager's reactions when the player starts a conversation (moving out / in, first meeting,
// birthday, not seen for a while, moods, holidays, ...). .text 8005E584..80060980.
// See include/game/game/d_npc_talk_reaction.hpp.
#include <game/game/d_npc_talk_reaction.hpp>
#include <lib/egg/math/eggMath.h>
#include <game/game/d_animal.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_play_util.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

// 804A4A48: labels of the reactions (getMsgLabel(TALK_REACTION, idx)).
const char *l_reactionLabels[REACTION_NUM] = {
    "Re_Moveout", "Re_Cafe",     "Re_Fishing",  "Re_Fall",    "Re_Run",       "Ev_First",
    "Re_FirstA1", "Re_FirstA2",  "Re_FirstB1",  "Re_FirstB2", "Re_FirstC1",   "Re_FirstC2",
    "Re_FirstV",  "Re_Movein",   "Re_Birthday", "Re_30days",  "Re_7days",     "Re_Tire",
    "Re_Anger",   "Re_Sad",      "Re_BeeFace",  "Re_Poison",  "Re_Xmas",      "Re_Newyear",
    "Re_Harvest", "Re_Halloween", "Re_Fireworks", "download",
};

// 8005E584
void dAcNpcNml_c::talk_c::resetActiveMood() {
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL && (u32)entry->mTimer.getMode() > 1) {
        resetMood();
    }
}

// 8005E5DC
int dAcNpcNml_c::talk_c::msgReMoveout(msgInfo_s *info) {
    dAnimal_c *animal = getAnimal();
    int code = 0;
    BOOL movingOut = animal != NULL ? animal->isMovingOut() : FALSE;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (movingOut && (player == NULL || player->isFlag0(0xD))) {
        movingOut = FALSE;
    }
    if (movingOut) {
        if (player->mPID.isFromTown()) {
            if (mpMemory != NULL) {
                if (mpMemory->mFlags.mMoveoutTalked) {
                    code = 4;
                } else if (mpMemory->mFlags.mMoveOutTold) {
                    code = 1;
                } else {
                    code = 2;
                }
            } else {
                code = 3;
            }
        } else {
            code = 5;
        }
    }
    if (code != 0) {
        if (info == NULL) {
            return TRUE;
        }
        const char *label = getMsgLabel(TALK_REACTION, REACTION_MOVEOUT, 0);
        if (label != NULL) {
            setLooksMsg(info, label, code);
            setStepProc(&talk_c::stepMoveout);
            mCountTalk = 1;
            resetActiveMood();
            return TRUE;
        }
    }
    return FALSE;
}

// 8005E740
int dAcNpcNml_c::talk_c::msgReCafe(msgInfo_s *info) {
    if (getCurrentScene() == SCENE_RM_MM_CAFE) {
        if (info == NULL) {
            return TRUE;
        }
        const char *label = getMsgLabel(TALK_REACTION, REACTION_CAFE, 0);
        if (label != NULL) {
            setLooksMsg(info, label, 0);
            return TRUE;
        }
    }
    return FALSE;
}

// 8005E7CC
int dAcNpcNml_c::talk_c::msgReFishing(msgInfo_s *info) {
    return FALSE;
}

// 8005E7D4
int dAcNpcNml_c::talk_c::msgReFall(msgInfo_s *info) {
    BOOL fall = mpNpc != NULL ? static_cast<dAcNpcNml_c *>(mpNpc)->isInPitfall() : FALSE;
    if (fall) {
        if (info == NULL) {
            return TRUE;
        }
        const char *label = getMsgLabel(TALK_REACTION, REACTION_FALL, 0);
        if (label != NULL) {
            setLooksMsg(info, label, 0);
            mNoRecordTalk = 1;
            return TRUE;
        }
    }
    return FALSE;
}

// 8005E888
int dAcNpcNml_c::talk_c::msgReRun(msgInfo_s *info) {
    BOOL busy = FALSE;
    if (!fn_800DCEDC()) {
        busy = fgMngProc_isBusy() != 0;
    }
    if (busy) {
        if (info == NULL) {
            return TRUE;
        }
        const char *label = getMsgLabel(TALK_REACTION, REACTION_RUN, 0);
        if (label != NULL) {
            setLooksMsg(info, label, 0);
            mNoRecordTalk = 1;
            resetActiveMood();
            return TRUE;
        }
    }
    return FALSE;
}

// 8005E944
int dAcNpcNml_c::talk_c::msgEvFirst(msgInfo_s *info) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimal_c *animal = getAnimal();
    if (player != NULL && animal != NULL) {
        BOOL first = player->isFlag0(0xD);
        BOOL known = mpMemory != NULL;
        if (first && !known) {
            if (info == NULL) {
                return TRUE;
            }
            const char *label = getMsgLabel(TALK_REACTION, REACTION_EV_FIRST, 0);
            if (label != NULL) {
                u16 code = 0;
                if (animal->isMovingIn()) {
                    code = 7;
                } else if (animal->isMovingOut()) {
                    code = 8;
                }
                setLooksMsg(info, label, code);
                mCountTalk = 1;
                resetActiveMood();
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 8005EA54
BOOL dAcNpcNml_c::talk_c::msgFirstMeeting(msgInfo_s *info, u32 known, u32 fromTown, u32 npcFromTown,
                                      u32 movingIn, int labelIdx, BOOL checkSick, u8 checkLostItem) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimal_c *animal = getAnimal();
    if (player != NULL && animal != NULL) {
        dLandID_c *townLand = dSaveData_c::getTownLand();
        dAnimalBlock_c *block = dSaveData_c::getAnimalBlock();
        dAnimalMemory_c *mem = mpMemory;
        BOOL checkPlayer = TRUE;
        BOOL isKnown = FALSE;
        if (mem != NULL && mem->mLand == *townLand) {
            isKnown = TRUE;
        }
        switch (labelIdx) {
        case REACTION_FIRST_B1:
        case REACTION_FIRST_B2:
            isKnown = mem != NULL;
            break;
        case REACTION_FIRST_C1:
        case REACTION_FIRST_C2:
            isKnown = FALSE;
            if (mem != NULL && mem->mLand != *townLand) {
                isKnown = TRUE;
            }
            checkPlayer = FALSE;
            break;
        case REACTION_FIRST_V:
            isKnown = mem != NULL;
            break;
        }
        bool playerFromTown = player->mPID.land == *townLand;
        bool animalFromTown = animal->mID.mLand2 == *townLand;
        if (known == isKnown && ((checkPlayer && fromTown == playerFromTown) || !checkPlayer) &&
            npcFromTown == animalFromTown && movingIn == animal->isMovingIn()) {
            if (info == NULL) {
                return TRUE;
            }
            const char *label = getMsgLabel(TALK_REACTION, labelIdx, 0);
            if (label != NULL) {
                u16 code = 0;
                if (checkSick && block->isSickAnimal(&animal->mID)) {
                    code = 0x15;
                } else if (checkLostItem && !animal->isLostItemRequestDone(NULL, TRUE)) {
                    code = 0x16;
                }
                setLooksMsg(info, label, code);
                if (player->isFlag0(2) && player->_83F5 == 0) {
                    setStepProc(&talk_c::stepBeeFaceSeen);
                }
                mCountTalk = 1;
                resetActiveMood();
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 8005EDA0
void dAcNpcNml_c::talk_c::setPrevLandWord(int idx) {
    dAnimal_c *animal = getAnimal();
    if (animal != NULL) {
        dLandID_c *land = &animal->mPrevLand;
        if (land->isValid()) {
            setLandName(land, idx);
            mMsgLand.copy(land);
        }
    }
}

// 8005EE18
int dAcNpcNml_c::talk_c::msgReFirstA1(msgInfo_s *info) {
    return msgFirstMeeting(info, FALSE, TRUE, TRUE, TRUE, REACTION_FIRST_A1, FALSE, FALSE);
}

// 8005EE58
int dAcNpcNml_c::talk_c::msgReFirstA2(msgInfo_s *info) {
    if (msgFirstMeeting(info, FALSE, TRUE, TRUE, FALSE, REACTION_FIRST_A2, TRUE, TRUE)) {
        if (info != NULL) {
            setStepProc(&talk_c::stepReaction);
        }
        return TRUE;
    }
    return FALSE;
}

// 8005EEF4
int dAcNpcNml_c::talk_c::msgReFirstB1(msgInfo_s *info) {
    BOOL ret = msgFirstMeeting(info, FALSE, TRUE, FALSE, TRUE, REACTION_FIRST_B1, FALSE, FALSE);
    if (ret && info != NULL) {
        setPrevLandWord(0);
    }
    return ret;
}

// 8005EF78
int dAcNpcNml_c::talk_c::msgReFirstB2(msgInfo_s *info) {
    BOOL ret = msgFirstMeeting(info, FALSE, TRUE, FALSE, FALSE, REACTION_FIRST_B2, TRUE, TRUE);
    if (ret && info != NULL) {
        setPrevLandWord(0);
        setStepProc(&talk_c::stepReaction);
    }
    return ret;
}

// 8005F024
int dAcNpcNml_c::talk_c::msgReFirstC1(msgInfo_s *info) {
    BOOL ret = msgFirstMeeting(info, TRUE, TRUE, FALSE, TRUE, REACTION_FIRST_C1, FALSE, FALSE);
    if (ret && info != NULL) {
        setPrevLandWord(0);
    }
    return ret;
}

// 8005F0A8
int dAcNpcNml_c::talk_c::msgReFirstC2(msgInfo_s *info) {
    BOOL ret = msgFirstMeeting(info, TRUE, TRUE, FALSE, FALSE, REACTION_FIRST_C2, TRUE, TRUE);
    if (ret && info != NULL) {
        setPrevLandWord(0);
        setStepProc(&talk_c::stepReaction);
    }
    return ret;
}

// 8005F154
void dAcNpcNml_c::talk_c::setPlayerLandWord() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dPersonalID_c *pid = player != NULL ? &player->mPID : NULL;
    if (pid != NULL && pid->isValid()) {
        setLandName(&pid->land, 0);
    }
}

// 8005F1C4
int dAcNpcNml_c::talk_c::msgReFirstV(msgInfo_s *info) {
    if (msgFirstMeeting(info, FALSE, FALSE, FALSE, FALSE, REACTION_FIRST_V, FALSE, TRUE)) {
        if (info != NULL) {
            setPlayerLandWord();
        }
        return TRUE;
    }
    if (msgFirstMeeting(info, FALSE, FALSE, TRUE, FALSE, REACTION_FIRST_V, FALSE, TRUE)) {
        if (info != NULL) {
            setPlayerLandWord();
        }
        return TRUE;
    }
    if (msgFirstMeeting(info, FALSE, FALSE, FALSE, TRUE, REACTION_FIRST_V, FALSE, TRUE)) {
        if (info != NULL) {
            setPlayerLandWord();
        }
        return TRUE;
    }
    if (msgFirstMeeting(info, FALSE, FALSE, TRUE, TRUE, REACTION_FIRST_V, FALSE, TRUE)) {
        if (info != NULL) {
            setPlayerLandWord();
        }
        return TRUE;
    }
    return FALSE;
}

// 8005F320
int dAcNpcNml_c::talk_c::msgReMovein(msgInfo_s *info) {
    dAnimal_c *animal = getAnimal();
    BOOL movingIn = animal != NULL ? animal->isMovingIn() : FALSE;
    if (movingIn) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player == NULL || player->isFlag0(0xD)) {
            movingIn = FALSE;
        }
    }
    if (movingIn) {
        if (info == NULL) {
            return TRUE;
        }
        const char *label = getMsgLabel(TALK_REACTION, REACTION_MOVEIN, 0);
        if (label != NULL) {
            setLooksMsg(info, label, 0);
            resetActiveMood();
            return TRUE;
        }
    }
    return FALSE;
}

// 8005F3F8
BOOL dAcNpcNml_c::talk_c::isReactionBlocked() const {
    dAnimal_c *animal = getAnimal();
    if (animal == NULL || !animal->mID.isValid()) {
        return TRUE;
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayerRaw();
    if (player == NULL || player->isFlag0(0xD)) {
        return TRUE;
    }
    if (animal->isLostItemRequestDone(player, TRUE) == FALSE) {
        return TRUE;
    }
    return dSaveData_c::getRaw()->mAnimals.mTown.isSickAnimal(&animal->mID) != 0;
}

// 8005F4B4
int dAcNpcNml_c::talk_c::msgReBirthday(msgInfo_s *info) {
    BOOL ok = FALSE;
    dBirthday_c *birthday;
    if (mpMemory != NULL && mpMemory->mFlags.mBirthdayDone) {
        int playerNo = fn_801017B8();
        bool valid = false;
        if (playerNo >= 0 && playerNo < 4) {
            valid = true;
        }
        if (valid) {
            dQuestEvent_e event = (dQuestEvent_e)(EVENT_PLAYER_BIRTHDAY_0 + playerNo);
            if (dEvent::isOngoing(event)) {
                ok = TRUE;
            }
        }
    }
    if (ok && isReactionBlocked()) {
        ok = FALSE;
    }
    if (ok) {
        if (info == NULL) {
            return TRUE;
        }
        const char *label = getMsgLabel(TALK_REACTION, REACTION_BIRTHDAY, 0);
        if (label != NULL) {
            int code = 0;
            dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
            if (player != NULL) {
                birthday = &player->mBirthday;
                dTime_c *now = dTime_c::getCurrent();
                if (birthday->isSame(1, 29)) {
                    code = (dTime_c::isLeapYear(now->year) != 0) + 11;
                }
            }
            setLooksMsg(info, label, code);
            setStepProc(&talk_c::stepBirthday);
            resetActiveMood();
            return TRUE;
        }
    }
    return FALSE;
}

// 8005F63C
BOOL dAcNpcNml_c::talk_c::msgNotSeen(msgInfo_s *info, int minDays, int divisor, int labelIdx) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL || !player->mPID.isFromTown()) {
        return FALSE;
    }
    if (isReactionBlocked()) {
        return FALSE;
    }
    if (mpMemory != NULL) {
        int days = mpMemory->mLastTalkTime.diffDaysFromNow(TRUE, FALSE);
        if (days >= minDays) {
            if (info == NULL) {
                return TRUE;
            }
            const char *label = getMsgLabel(TALK_REACTION, labelIdx, 0);
            if (label != NULL) {
                int num = days / divisor;
                u16 code = 0;
                if (labelIdx == REACTION_30DAYS && num >= 12) {
                    code = 1;
                }
                setLooksMsg(info, label, code);
                if (player->isFlag0(2) && player->_83F5 == 0) {
                    setStepProc(&talk_c::stepBeeFaceSeen);
                }
                mCountTalk = 1;
                if (code != 1) {
                    formatNumber(num, 0, 2, dScript::NUM_FORMAT_REGION);
                }
                resetActiveMood();
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 8005F7C4
int dAcNpcNml_c::talk_c::msgRe30days(msgInfo_s *info) {
    switch (getLanguage()) {
    case LANGUAGE_JP:
        return msgNotSeen(info, 30, 30, REACTION_30DAYS);
    default:
        return msgNotSeen(info, 60, 30, REACTION_30DAYS);
    }
}

// 8005F83C
int dAcNpcNml_c::talk_c::msgRe7days(msgInfo_s *info) {
    switch (getLanguage()) {
    case LANGUAGE_JP:
        return msgNotSeen(info, 7, 7, REACTION_7DAYS);
    default:
        return msgNotSeen(info, 14, 7, REACTION_7DAYS);
    }
}

// 8005F8B4
int dAcNpcNml_c::talk_c::msgReTire(msgInfo_s *info) {
    dNpcEntry_c *entry = getEntry();
    BOOL ok = FALSE;
    if (entry != NULL && entry->mTimer.getMode() == 4) {
        ok = TRUE;
    }
    if (ok && isReactionBlocked()) {
        ok = FALSE;
    }
    if (ok) {
        if (info == NULL) {
            return TRUE;
        }
        const char *label = getMsgLabel(TALK_REACTION, REACTION_TIRE, 0);
        if (label != NULL) {
            setLooksMsg(info, label, 0);
            return TRUE;
        }
    }
    return FALSE;
}

// 8005F980
int dAcNpcNml_c::talk_c::msgReAnger(msgInfo_s *info) {
    dNpcEntry_c *entry = getEntry();
    BOOL ok = FALSE;
    if (entry != NULL && entry->mTimer.getMode() == 2) {
        ok = TRUE;
    }
    if (ok && isReactionBlocked()) {
        ok = FALSE;
    }
    if (ok) {
        if (info == NULL) {
            return TRUE;
        }
        const char *label = getMsgLabel(TALK_REACTION, REACTION_ANGER, 0);
        if (label != NULL) {
            setLooksMsg(info, label, 0);
            return TRUE;
        }
    }
    return FALSE;
}

// 8005FA4C
int dAcNpcNml_c::talk_c::msgReSad(msgInfo_s *info) {
    dNpcEntry_c *entry = getEntry();
    BOOL ok = FALSE;
    if (entry != NULL && entry->mTimer.getMode() == 3) {
        ok = TRUE;
    }
    if (ok && isReactionBlocked()) {
        ok = FALSE;
    }
    if (ok) {
        if (info == NULL) {
            return TRUE;
        }
        const char *label = getMsgLabel(TALK_REACTION, REACTION_SAD, 0);
        if (label != NULL) {
            setLooksMsg(info, label, 0);
            return TRUE;
        }
    }
    return FALSE;
}

// 8005FB18
int dAcNpcNml_c::talk_c::msgReBeeFace(msgInfo_s *info) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayerRaw();
    BOOL ok = player != NULL ? player->isFlag0(2) : FALSE;
    if (ok && isReactionBlocked()) {
        ok = FALSE;
    }
    if (ok && player->_83F5 != 0) {
        ok = FALSE;
    }
    if (ok && (mpMemory == NULL || mpMemory->mFlags.mBeeFaceTalked)) {
        ok = FALSE;
    }
    if (ok) {
        if (info == NULL) {
            return TRUE;
        }
        const char *label = getMsgLabel(TALK_REACTION, REACTION_BEE_FACE, 0);
        if (label != NULL) {
            setLooksMsg(info, label, 0);
            mCountTalk = 1;
            setStepProc(&talk_c::stepBeeFaceSeen);
            return TRUE;
        }
    }
    return FALSE;
}

// 8005FC5C
int dAcNpcNml_c::talk_c::findNearItemCb(dPlayActor_c *actor, void *arg) {
    static dItem::Item l_scorpion(dItem::ITEM_IDX_SCORPION);
    static dItem::Item l_tarantula(dItem::ITEM_IDX_TARANTULA);
    dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(arg);
    if (npc == NULL) {
        return FALSE;
    }
    if (actor == NULL) {
        return FALSE;
    }
    dItem::Item item = getActorItem(actor);
    if (item.isNotSame(l_scorpion) && item.isNotSame(l_tarantula)) {
        return FALSE;
    }
    mVec3_c *pos = &actor->mPos;
    if (std::fabs(pos->y - npc->mPos.y) > 48.0f) {
        return FALSE;
    }
    f32 dist = EGG::Math<f32>::sqrt(PSVECSquareDistance(*pos, npc->mPos));
    if (dist > 96.0f) {
        return FALSE;
    }
    if (!npc->mNearItem.isValid() || dist < npc->mNearDist) {
        npc->mNearItem = item;
        npc->mNearDist = dist;
    }
    return TRUE;
}

// 8005FDD8
dItem::Item dAcNpcNml_c::talk_c::findNearItem() {
    dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
    if (npc == NULL) {
        return dItem::Item();
    }
    npc->mNearItem = dItem::ITEM_ID_NONE;
    npc->mNearDist = 96.0f;
    BOOL found = lbl_8074E838 != NULL ? lbl_8074E838->forEachActiveActor(findNearItemCb, npc) : FALSE;
    if (found) {
        return npc->mNearItem;
    }
    return dItem::Item();
}

// 8005FE7C
int dAcNpcNml_c::talk_c::msgRePoison(msgInfo_s *info) {
    dNpcEntry_c *entry = getEntry();
    BOOL ok = entry != NULL ? !entry->_28C.mPoisonDone : FALSE;
    dItem::Item item;
    if (ok && !isCurrentSceneAttr(SCENE_ATTR_TOWN)) {
        ok = FALSE;
    }
    if (ok && isReactionBlocked()) {
        ok = FALSE;
    }
    if (ok) {
        item = findNearItem();
        if (!item.isValid()) {
            ok = FALSE;
        }
    }
    if (ok) {
        if (info == NULL) {
            return TRUE;
        }
        const char *label = getMsgLabel(TALK_REACTION, REACTION_POISON, 0);
        if (label != NULL) {
            setLooksMsg(info, label, 0);
            setStepProc(&talk_c::stepPoison);
            mCountTalk = 1;
            if (item.isValid()) {
                setItemName(&item, 0);
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 8005FFE4
int dAcNpcNml_c::talk_c::msgReXmas(msgInfo_s *info) {
    BOOL eve;
    BOOL gift;
    dTime_c *now = dTime_c::getCurrent();
    dAnimal_c *animal = getAnimal();
    BOOL valid = FALSE;
    if (animal != NULL && animal->mID.isValid()) {
        valid = TRUE;
    }
    u8 looks = valid ? animal->mID.getLooks(1) : 6;
    int code = 0;
    BOOL ok = FALSE;
    if (dEvent::isOngoing(EVENT_TOY_DAY) || (now->month == 11 && now->mday == 25)) {
        ok = TRUE;
    }
    gift = FALSE;
    bool christmas = false;
    if (now->month == 11 && now->mday == 25) {
        christmas = true;
    }
    if (christmas && fn_800F4250(looks, now)) {
        gift = TRUE;
    }
    eve = FALSE;
    if (ok && !gift) {
        eve = TRUE;
    }
    if (ok && isReactionBlocked()) {
        ok = FALSE;
    }
    if (ok && (animal == NULL || animal->isMoving())) {
        ok = FALSE;
    }
    if (ok && (mpMemory == NULL || mpMemory->mFlags.mEventTalked)) {
        ok = FALSE;
    }
    if (ok) {
        code = eve ? 1 : 2;
    }
    if ((u16)code != 0) {
        if (info == NULL) {
            return TRUE;
        }
        const char *label = getMsgLabel(TALK_REACTION, REACTION_XMAS, 0);
        if (label != NULL) {
            setLooksMsg(info, label, code);
            setStepProc(&talk_c::stepEventTalked);
            mCountTalk = 1;
            return TRUE;
        }
    }
    return FALSE;
}

// 800601F8
int dAcNpcNml_c::talk_c::msgReNewyear(msgInfo_s *info) {
    BOOL ok = dEvent::isOngoing(EVENT_NEW_YEARS_DAY) != 0;
    if (ok && isReactionBlocked()) {
        ok = FALSE;
    }
    if (ok && (mpMemory == NULL || mpMemory->mFlags.mEventTalked)) {
        ok = FALSE;
    }
    if (ok) {
        if (info == NULL) {
            return TRUE;
        }
        const char *label = getMsgLabel(TALK_REACTION, REACTION_NEW_YEAR, 0);
        if (label != NULL) {
            setLooksMsg(info, label, 0);
            setStepProc(&talk_c::stepEventTalked);
            mCountTalk = 1;
            return TRUE;
        }
    }
    return FALSE;
}

// 80060304
int dAcNpcNml_c::talk_c::msgReHarvest(msgInfo_s *info) {
    BOOL ok = dEvent::isOver(EVENT_HARVEST_FESTIVAL) != 0;
    dSaveTown_c *town = dSaveData_c::getTown();
    dAnimal_c *animal = getAnimal();
    if ((dQuestEvent_e)town->mAnimals.mTown.mEventId != EVENT_HARVEST_FESTIVAL || animal == NULL || !animal->mEvent.isInEvent()) {
        ok = FALSE;
    }
    if (ok && isReactionBlocked()) {
        ok = FALSE;
    }
    if (mpMemory == NULL || mpMemory->mFlags.mEventTalked) {
        ok = FALSE;
    }
    if (ok) {
        if (info == NULL) {
            return TRUE;
        }
        const char *label = getMsgLabel(TALK_REACTION, REACTION_HARVEST, 0);
        if (label != NULL) {
            setLooksMsg(info, label, 0);
            setStepProc(&talk_c::stepEventTalked);
            mCountTalk = 1;
            return TRUE;
        }
    }
    return FALSE;
}

// 80060450
int dAcNpcNml_c::talk_c::msgReHalloween(msgInfo_s *info) {
    BOOL ok = dEvent::isOver(EVENT_HALLOWEEN) != 0;
    if (ok && isReactionBlocked()) {
        ok = FALSE;
    }
    if (ok) {
        dSaveTown_c *town = dSaveData_c::getTown();
        dAnimal_c *animal = getAnimal();
        if ((dQuestEvent_e)town->mAnimals.mTown.mEventId != EVENT_HALLOWEEN || animal == NULL || !animal->mEvent.isInEvent()) {
            ok = FALSE;
        }
    }
    if (ok && (mpMemory == NULL || mpMemory->mFlags.mEventTalked)) {
        ok = FALSE;
    }
    if (ok) {
        if (info == NULL) {
            return TRUE;
        }
        const char *label = getMsgLabel(TALK_REACTION, REACTION_HALLOWEEN, 0);
        if (label != NULL) {
            setLooksMsg(info, label, 0);
            setStepProc(&talk_c::stepEventTalked);
            mCountTalk = 1;
            return TRUE;
        }
    }
    return FALSE;
}

// 800605A8
int dAcNpcNml_c::talk_c::msgReFireworks(msgInfo_s *info) {
    dTime_c *now = dTime_c::getCurrent();
    BOOL ok = FALSE;
    if (dEvent::isOver(EVENT_FIREWORKS) && now->hour < 2) {
        ok = TRUE;
    }
    if (ok && isReactionBlocked()) {
        ok = FALSE;
    }
    if (ok) {
        dSaveTown_c *town = dSaveData_c::getTown();
        dAnimal_c *animal = getAnimal();
        if ((dQuestEvent_e)town->mAnimals.mTown.mEventId != EVENT_FIREWORKS || animal == NULL || !animal->mEvent.isInEvent()) {
            ok = FALSE;
        }
    }
    if (ok && (mpMemory == NULL || mpMemory->mFlags.mEventTalked)) {
        ok = FALSE;
    }
    if (ok) {
        if (info == NULL) {
            return TRUE;
        }
        const char *label = getMsgLabel(TALK_REACTION, REACTION_FIREWORKS, 0);
        if (label != NULL) {
            setLooksMsg(info, label, 0);
            setStepProc(&talk_c::stepEventTalked);
            mCountTalk = 1;
            return TRUE;
        }
    }
    return FALSE;
}

// 8006071C
int dAcNpcNml_c::talk_c::msgReDownload(msgInfo_s *info) {
    const u32 *msg = NULL;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayerRaw();
    dAnimal_c *animal = getAnimal();
    BOOL valid = FALSE;
    if (animal != NULL && animal->mID.isValid()) {
        valid = TRUE;
    }
    u8 looks = valid ? animal->mID.getLooks(1) : 6;
    if ((!fn_800DCEDC() || fn_800DCF30() <= 1) && player != NULL && player->mPID.isFromTown() && looks < 6 &&
        (u32)getLanguage() < LANGUAGE_NUM) {
        msg = fn_80117318(&dSaveData_c::getRawExtra()->_0001CE, fn_801017B8(), looks, 10);
    }
    if (msg != NULL && isReactionBlocked()) {
        msg = NULL;
    }
    if (msg != NULL) {
        if (info == NULL) {
            return TRUE;
        }
        mLangFlag = looks;
        mpBmgData = msg + 1;
        setLooksMsg(info, NULL, 0);
        mCountTalk = 1;
        return TRUE;
    }
    return FALSE;
}

// 80060878
BOOL dAcNpcNml_c::talk_c::stepMoveout(int kind) {
    if (mpMemory != NULL) {
        mpMemory->mFlags.mMoveoutTalked = 1;
    }
    return TRUE;
}

// 80060898
BOOL dAcNpcNml_c::talk_c::stepBeeFaceSeen(int kind) {
    if (mpMemory != NULL) {
        mpMemory->mFlags.mBeeFaceTalked = 1;
    }
    return TRUE;
}

// 800608B8
BOOL dAcNpcNml_c::talk_c::stepPoison(int kind) {
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->_28C.mPoisonDone = 1;
    }
    return TRUE;
}

// 800608F0
BOOL dAcNpcNml_c::talk_c::stepEventTalked(int kind) {
    if (mpMemory != NULL) {
        mpMemory->mFlags.mEventTalked = 1;
        if (mpNpc != NULL) {
            fn_800F05C8(static_cast<dAcNpcNml_c *>(mpNpc)->getNpcIdx(), mMemoryIdx);
        }
    }
    return TRUE;
}

// 8006094C
BOOL dAcNpcNml_c::talk_c::stepBirthday(int kind) {
    if (mpMemory != NULL) {
        mpMemory->mFlags.mBirthdayDone = 0;
    }
    return TRUE;
}

// 8006096C
dItem::Item dAcNpcNml_c::talk_c::getActorItem(dPlayActor_c *actor) {
    return dItem::Item(dItem::ITEM_IDX_COMMON_BUTTERFLY, actor->m2F0, FALSE);
}
