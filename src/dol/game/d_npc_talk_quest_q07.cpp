// Villager talk for the final errand (QUEST_KIND_ERRAND_REQUEST_FINAL): the request, the delivery with its
// three-way answer and the final reward. .text 800546F4..80056024. See
// include/game/game/d_npc_talk_quest_q07.hpp.
#include <game/game/d_npc_talk_quest_q07.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_item_sel.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

// 800546F4
int dAcNpcNml_c::talk_c::msgErrandFinalOffer(msgInfo_s *info) {
    return startErrandOffer(info, QUEST_KIND_ERRAND_REQUEST_FINAL, QUEST_TALK_ERRAND_FINAL, &talk_c::stepErrandFinalOffer);
}

// 8005473C
BOOL dAcNpcNml_c::talk_c::stepErrandFinalOffer(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgErrandFinalReq);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

static const char l_Q07_Req[] = "Q07_Req"; // 807501D8

// 800547B4
int dAcNpcNml_c::talk_c::msgErrandFinalReq(msgInfo_s *info) {
    setLooksMsg(info, l_Q07_Req, 0);
    setStepProc(&talk_c::stepErrandFinalReq);
    return TRUE;
}

// 80054814
BOOL dAcNpcNml_c::talk_c::stepErrandFinalReq(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoice(0, 0x15, 10, &talk_c::selErrandFinalYes);
        setChoice(1, 0x1F, 10, &talk_c::selNo);
        setChoiceNum(2);
        setChoiceCancel(1);
        showChoice();
        dNpcEntry_c *entry = getEntry();
        dQuestBase_c *quest = entry != NULL ? &entry->mQuest : NULL;
        if (quest != NULL) {
            quest->clear();
        }
        return TRUE;
    }
    return FALSE;
}

// 80054914
void dAcNpcNml_c::talk_c::selErrandFinalYes() {
    setMsgProc(&talk_c::msgErrandFinalYes);
    startMsg();
}

// 80054968
int dAcNpcNml_c::talk_c::msgErrandFinalYes(msgInfo_s *info) {
    setLooksMsg(info, l_Q_Yes, 0);
    setHookProc(&talk_c::endErrandFinalYes);
    setStepProc(&talk_c::stepErrandFinalYes);
    return TRUE;
}

// 800549F4
void dAcNpcNml_c::talk_c::endErrandFinalYes(int arg) {
    static const int l_deadlines[3] = {QUEST_DEADLINE_NEXT_HOUR, QUEST_DEADLINE_NEXT_PERIOD, QUEST_DEADLINE_MIDNIGHT};

    dAnmPersonalID_c *self;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dQuestErrand_c *errand = player != NULL ? player->mErrand.get(0) : NULL;
    if (errand != NULL) {
        dTime_c *now = dTime_c::getCurrent();
        mErrandDeadline = dQuestBase_c::pickDeadline(l_deadlines, 3, now);
        self = &getAnimal()->mID;
        const dAnmPersonalID_c *exclude = self;
        dSaveTown_c *town = dSaveData_c::getTown();
        dAnimal_c *other = town->mAnimals.mTown.pickRandomAvailableAnimal(&exclude, 1);
        dItemSelFilter_c filter(NULL, 0, 1);
        dItem::Item excludeItems[2];
        excludeItems[0] = other->mCloth;
        dItemSelRange_c range(4, 3);
        if (!fn_800C60B4(&mItem0, 1, &range, 1, &filter, excludeItems, 1, 0)) {
            mItem0.setFromIndex(dItem::ITEM_IDX_WORK_UNIFORM);
        }
        errand->clear();
        if (other != NULL && mItem0.mId != dItem::ITEM_ID_NONE) {
            setAnmPersonalName(&other->mID, 0);
            u16 unit = fn_800F3F38(other->mID.getGender(1), self->getLooks(1));
            if (unit == 0) {
                clearWord(1);
            } else {
                getController()->fn_801A5874(1, unit, "sys_STRING/STR_Unit");
            }
            if (player != NULL && player->pickUp(&mItem0, 2)) {
                errand->start(QUEST_KIND_ERRAND_REQUEST_FINAL, self, &other->mID, &mItem0, now, mErrandDeadline, QUEST_ERRAND_DELIVERING);
                dTime_c deadline = errand->mBase.getDeadline(*now);
                setTime(deadline.hour, 6, 0);
            }
        }
    }
}

// 80054C48
BOOL dAcNpcNml_c::talk_c::stepErrandFinalYes(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgYes);
        startMsg();
        requestItemAct(&mItem0, 2, 0);
        mpNpc->mAudioObj.startSound(0x171E);
        return TRUE;
    }
    return FALSE;
}

// 804A3E28: labels of the quest-talk states (getMsgLabel(TALK_QUEST, QUEST_TALK_ERRAND_FINAL, state)).
const char *l_q07Labels[6] = {l_Ai_Quest, l_Q_Timeover, l_Q_Timeover, l_Ai_Quest, l_Ai_Quest, l_Ai_Quest};

// 80054CEC
int dAcNpcNml_c::talk_c::msgErrandFinalQuest(msgInfo_s *info) {
    if (isEventOngoing()) {
        return FALSE;
    }
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    dAnmPersonalID_c *self = &animal->mID;
    const dQuestErrand_c *errand;
    const dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        errand = player->mErrand.get(0);
    } else {
        errand = NULL;
    }
    if (errand == NULL || !errand->mBase.isActive()) {
        return FALSE;
    }
    if (errand->mBase.getKind() != QUEST_KIND_ERRAND_REQUEST_FINAL) {
        return FALSE;
    }
    if (errand->_18E != 0) {
        return FALSE;
    }
    if (!(*errand->getAnimal(0) == *self)) {
        return FALSE;
    }

    int state = 0;
    stepFunc step = &talk_c::stepErrandFinalChoice;
    if (errand->mBase.mState >= QUEST_ERRAND_DONE) {
        state = 3;
    } else if (errand->mBase.isPastDeadline(NULL)) {
        state = 1;
    }
    switch (state) {
    case 1:
    case 2:
        step = &talk_c::stepErrandOver;
        break;
    case 3:
    case 4:
    case 5:
        step = &talk_c::stepErrandFinalReportChoice;
        break;
    }

    const char *label = getMsgLabel(TALK_QUEST, QUEST_TALK_ERRAND_FINAL, state);
    if (label != NULL) {
        setProcSet(&l_talkEntrySets[TALK_QUEST]);
        setLooksMsg(info, label, 0);
        setStepProc(step);
        setTopic(dNpc::msgMemory_c::KIND_QUEST, QUEST_TALK_ERRAND_FINAL, 0);
        if (state != 1 && state != 2) {
            mCountTalk = 1;
        }
        dAnmPersonalID_c *recipient = errand->getAnimal(1);
        setAnmPersonalName(recipient, 0);
        u16 unit = fn_800F3F38(recipient->getGender(1), self->getLooks(1));
        if (unit == 0) {
            clearWord(1);
        } else {
            getController()->fn_801A5874(1, unit, "sys_STRING/STR_Unit");
        }
        dAnmPersonalID_c *requester = errand->getAnimal(0);
        if (requester->isValid()) {
            setAnmPersonalName(requester, 2);
            u16 unit2 = fn_800F3F38(requester->getGender(1), self->getLooks(1));
            if (unit2 == 0) {
                clearWord(3);
            } else {
                getController()->fn_801A5874(3, unit2, "sys_STRING/STR_Unit");
            }
        }
        setItemName(&errand->mBase.mItem, 4);
        return TRUE;
    }
    return FALSE;
}

// 800550B0
BOOL dAcNpcNml_c::talk_c::stepErrandFinalReportChoice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        int n = 0;
        u16 code = 1;
        u8 range = 0;
        hookFunc rollan = getRollanChoice(&code, &range);
        if (rollan != NULL) {
            setChoice(0, code, range, rollan);
            n = 1;
        }
        hookFunc harvest = getHarvestChoice(&code, &range, TRUE);
        if (harvest != NULL) {
            setChoice(n, code, range, harvest);
            n++;
        }
        setChoice(n, 0x10, 3, &talk_c::selErrandFinalReport);
        setChoice(n + 1, 4, 3, &talk_c::selErrandFinalResume);
        setChoiceNum(n + 2);
        setChoiceCancel(n + 1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 80055284
void dAcNpcNml_c::talk_c::selErrandFinalReport() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dQuestErrand_c *errand = player != NULL ? player->mErrand.get(0) : NULL;
    if (errand != NULL && errand->mBase.mState == QUEST_ERRAND_FINAL_LATE) {
        setMsgProc(&talk_c::msgTimeover2);
        addFriendship(-1);
    } else {
        setMsgProc(&talk_c::msgErrandFinalReport);
    }
    startMsg();
}

static const char l_Q07_Report[] = "Q07_Report"; // 8046C9DC

// 80055348
int dAcNpcNml_c::talk_c::msgErrandFinalReport(msgInfo_s *info) {
    setLooksMsg(info, l_Q07_Report, 0);
    setStepProc(&talk_c::stepErrandFinalReport);
    return TRUE;
}

// 800553AC
BOOL dAcNpcNml_c::talk_c::stepErrandFinalReport(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoice(0, 0x33, 1, &talk_c::selErrandFinalGood);
        setChoice(1, 0x34, 1, &talk_c::selErrandFinalNormal);
        setChoice(2, 0x35, 1, &talk_c::selErrandFinalBad);
        setChoiceNum(3);
        setChoiceCancel(-1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 800554C8
void dAcNpcNml_c::talk_c::selErrandFinalGood() {
    setMsgProc(&talk_c::msgErrandFinalGood);
    startMsg();
}

// 8005551C
void dAcNpcNml_c::talk_c::selErrandFinalNormal() {
    setMsgProc(&talk_c::msgErrandFinalNormal);
    startMsg();
}

// 80055570
void dAcNpcNml_c::talk_c::selErrandFinalBad() {
    setMsgProc(&talk_c::msgErrandFinalBad);
    startMsg();
}

static const char l_Q07_Good[] = "Q07_Good"; // 8046C9E8

// 800555C4
int dAcNpcNml_c::talk_c::msgErrandFinalGood(msgInfo_s *info) {
    setLooksMsg(info, l_Q07_Good, 0);
    setStepProc(&talk_c::stepErrandFinalAnswer);
    addAnswerFriendship(1);
    return TRUE;
}

static const char l_Q07_Normal[] = "Q07_Normal"; // 8046C9F4

// 80055634
int dAcNpcNml_c::talk_c::msgErrandFinalNormal(msgInfo_s *info) {
    setLooksMsg(info, l_Q07_Normal, 0);
    setStepProc(&talk_c::stepErrandFinalAnswer);
    addAnswerFriendship(3);
    return TRUE;
}

static const char l_Q07_Bad[] = "Q07_Bad"; // 807501E0

// 800556A4
int dAcNpcNml_c::talk_c::msgErrandFinalBad(msgInfo_s *info) {
    setLooksMsg(info, l_Q07_Bad, 0);
    setStepProc(&talk_c::stepErrandFinalAnswer);
    addAnswerFriendship(2);
    return TRUE;
}

// 80055710
BOOL dAcNpcNml_c::talk_c::stepErrandFinalAnswer(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        BOOL full = player != NULL ? player->findEmptyPocket(0) == -1 : TRUE;
        if (full) {
            setMsgProc(&talk_c::msgErrandFinalRewardFull);
        } else {
            setMsgProc(&talk_c::msgErrandFinalReward);
        }
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 800557E8
int dAcNpcNml_c::talk_c::msgErrandFinalReward(msgInfo_s *info) {
    setLooksMsg(info, l_Q_Item, 0);
    setHookProc(&talk_c::endErrandFinalReward);
    setStepProc(&talk_c::stepErrandFinalReward);
    return TRUE;
}

// 80055874
void dAcNpcNml_c::talk_c::endErrandFinalReward(int arg) {
    static const u8 l_answerKinds[3] = {0, 2, 1};

    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dQuestErrand_c *errand = player != NULL ? player->mErrand.get(0) : NULL;
    dAnimal_c *animal = getAnimal();
    int price = 0;
    mItem0 = dItem::ITEM_ID_NONE;
    if (mAnswer >= 3) {
        mAnswer = 1;
    }
    u32 state = errand != NULL ? errand->mBase.mState : 3;
    u32 grade = state >= 1 && state < 4 ? state - 1 : 2;
    u32 kind = animal != NULL ? animal->pickErrandFinalReward(&mItem0, &price, player, grade, l_answerKinds[mAnswer]) : 4;
    if (mItem0 != dItem::ITEM_ID_NONE) {
        switch (kind) {
        case 0:
        case 3:
            if (player != NULL) {
                player->pickUp(&mItem0, FALSE);
            }
            setItemName(&mItem0, 5);
            break;
        case 1:
            if (animal != NULL && animal->removeNewItem(&mItem0)) {
                fn_800F0FE4(getNpcIdx(), &mItem0);
            }
            if (player != NULL) {
                player->pickUp(&mItem0, FALSE);
            }
            setItemName(&mItem0, 5);
            break;
        default:
            if (price > 0 && player != NULL) {
                player->addMoney(price);
                setBells(price, 5);
            }
            break;
        }
    }
}

// 80055A5C
BOOL dAcNpcNml_c::talk_c::stepErrandFinalReward(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        dQuestErrand_c *errand = player != NULL ? player->mErrand.get(0) : NULL;
        if (errand != NULL) {
            errand->clear();
        }
        setMsgProc(&talk_c::msgErrandFinalEnd);
        startMsg();
        if (mItem0 != dItem::ITEM_ID_NONE) {
            requestItemAct(&mItem0, 0, 0);
        }
        mpNpc->mAudioObj.startSound(0x171F);
        return TRUE;
    }
    return FALSE;
}

static const char l_Q07_End[] = "Q07_End"; // 807501F0

// 80055B3C
int dAcNpcNml_c::talk_c::msgErrandFinalEnd(msgInfo_s *info) {
    setLooksMsg(info, l_Q07_End, 0);
    return TRUE;
}

// 80055B68
int dAcNpcNml_c::talk_c::msgErrandFinalRewardFull(msgInfo_s *info) {
    setLooksMsg(info, l_Q_ItemFull, 0);
    setStepProc(&talk_c::stepErrandFinalRewardFull);
    return TRUE;
}

// 80055BCC
BOOL dAcNpcNml_c::talk_c::stepErrandFinalRewardFull(int kind) {
    static const u8 l_answerKinds[3] = {0, 2, 1};

    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        dQuestErrand_c *errand = player != NULL ? player->mErrand.get(0) : NULL;
        if (errand != NULL) {
            if (mAnswer >= 3) {
                mAnswer = 1;
            }
            errand->_18E = l_answerKinds[mAnswer] + 1;
            dAnimal_c *animal = getAnimal();
            if (animal != NULL) {
                animal->completeErrandRequestFinal(getNpcIdx(), player, FALSE);
            }
        }
        mpNpc->mAudioObj.startSound(0x171F);
        setMsgProc(&talk_c::msgErrandFinalEnd);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80055CF8
void dAcNpcNml_c::talk_c::selErrandFinalResume() {
    const procSet_s *set = getEventProcSet();
    if (set == NULL) {
        set = &l_talkEntrySets[TALK_FREE];
    }
    setProcSet(set);
    startMsg();
}

// 80055D4C
BOOL dAcNpcNml_c::talk_c::stepErrandFinalChoice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        int n = 0;
        u16 code = 1;
        u8 range = 0;
        hookFunc rollan = getRollanChoice(&code, &range);
        if (rollan != NULL) {
            setChoice(0, code, range, rollan);
            n = 1;
        }
        hookFunc harvest = getHarvestChoice(&code, &range, TRUE);
        if (harvest != NULL) {
            setChoice(n, code, range, harvest);
            n++;
        }
        setChoice(n, 1, 3, &talk_c::selErrandFinalCon);
        setChoice(n + 1, 4, 3, &talk_c::selErrandFinalResume);
        setChoiceNum(n + 2);
        setChoiceCancel(n + 1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 80055F20
void dAcNpcNml_c::talk_c::selErrandFinalCon() {
    setMsgProc(&talk_c::msgErrandFinalCon);
    startMsg();
}

static const char l_Q07_Con[] = "Q07_Con"; // 80750200

// 80055F74
int dAcNpcNml_c::talk_c::msgErrandFinalCon(msgInfo_s *info) {
    setLooksMsg(info, l_Q07_Con, 0);
    return TRUE;
}

// 80055FA0
void dAcNpcNml_c::talk_c::addAnswerFriendship(int answer) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dQuestErrand_c *errand = player != NULL ? player->mErrand.get(0) : NULL;
    if (errand != NULL) {
        if (answer == errand->mBase.mState) {
            addFriendship(5);
        } else {
            addFriendship(3);
        }
    }
}
