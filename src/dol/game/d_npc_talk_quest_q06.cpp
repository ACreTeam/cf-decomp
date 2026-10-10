// Villager talk for errand quest 6 (QUEST_KIND_ERRAND_REQUEST): the request, the delivery and its reward.
// .text 80056024..800573D0. See include/game/game/d_npc_talk_quest_q06.hpp.
#include <game/game/d_npc_talk_quest_q06.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_item_sel.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

// 80056024
int dAcNpcNml_c::talk_c::msgErrandOffer(msgInfo_s *info) {
    return startErrandOffer(info, QUEST_KIND_ERRAND_REQUEST, QUEST_TALK_ERRAND, &talk_c::stepErrandOffer);
}

// 8005606C
BOOL dAcNpcNml_c::talk_c::stepErrandOffer(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgErrandReq);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

static const char l_Q06_Req[] = "Q06_Req"; // 80750208

// 800560E4
int dAcNpcNml_c::talk_c::msgErrandReq(msgInfo_s *info) {
    setLooksMsg(info, l_Q06_Req, 0);
    setStepProc(&talk_c::stepErrandReq);
    return TRUE;
}

// 80056144
BOOL dAcNpcNml_c::talk_c::stepErrandReq(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoice(0, 0x15, 10, &talk_c::selErrandYes);
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

// 80056244
void dAcNpcNml_c::talk_c::selErrandYes() {
    setMsgProc(&talk_c::msgErrandYes);
    startMsg();
}

// 80056298
int dAcNpcNml_c::talk_c::msgErrandYes(msgInfo_s *info) {
    setLooksMsg(info, l_Q_Yes, 0);
    setHookProc(&talk_c::endErrandYes);
    setStepProc(&talk_c::stepErrandYes);
    return TRUE;
}

// 80056324
void dAcNpcNml_c::talk_c::endErrandYes(int arg) {
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
        if (!fn_800F4608(&mItem0, 1, lbl_8059FF80, NULL, 0)) {
            mItem0.setFromIndex(dItem::ITEM_IDX_EXOTIC_BED);
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
                errand->start(QUEST_KIND_ERRAND_REQUEST, self, &other->mID, &mItem0, now, mErrandDeadline, QUEST_ERRAND_DELIVERING);
                dTime_c deadline = errand->mBase.getDeadline(*now);
                setTime(deadline.hour, 6, 0);
            }
        }
    }
}

// 80056534
BOOL dAcNpcNml_c::talk_c::stepErrandYes(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgYes);
        startMsg();
        requestItemAct(&mItem0, 2, 0);
        mpNpc->mAudioObj.startSound(0x171E);
        return TRUE;
    }
    return FALSE;
}

// 804A4008: labels of the quest-talk states (getMsgLabel(TALK_QUEST, QUEST_TALK_ERRAND, state)).
const char *l_q06Labels[6] = {l_Ai_Quest, l_Q_Timeover, l_Q_Timeover, l_Ai_Quest, l_Ai_Quest, l_Ai_Quest};

// 800565D8
int dAcNpcNml_c::talk_c::msgErrandQuest(msgInfo_s *info) {
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
    if (errand->mBase.getKind() != QUEST_KIND_ERRAND_REQUEST) {
        return FALSE;
    }
    if (errand->_18E != 0) {
        return FALSE;
    }
    if (!(*errand->getAnimal(0) == *self)) {
        return FALSE;
    }

    int state = 0;
    stepFunc step = &talk_c::stepErrandChoice;
    if ((u32)errand->mBase.mState == QUEST_ERRAND_DONE || (u32)errand->mBase.mState == QUEST_ERRAND_LATE) {
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
        step = &talk_c::stepErrandReportChoice;
        break;
    }

    const char *label = getMsgLabel(TALK_QUEST, QUEST_TALK_ERRAND, state);
    if (label != NULL) {
        setProcSet(&l_talkEntrySets[TALK_QUEST]);
        setLooksMsg(info, label, 0);
        setStepProc(step);
        setTopic(dNpc::msgMemory_c::KIND_QUEST, QUEST_TALK_ERRAND, 0);
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
        setItemName(&errand->mBase.mItem, 4);
        return TRUE;
    }
    return FALSE;
}

// 80056928
BOOL dAcNpcNml_c::talk_c::stepErrandReportChoice(int kind) {
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
        setChoice(n, 0x10, 3, &talk_c::selErrandReport);
        setChoice(n + 1, 4, 3, &talk_c::selResumeTalk);
        setChoiceNum(n + 2);
        setChoiceCancel(n + 1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 80056AFC
void dAcNpcNml_c::talk_c::selErrandReport() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dQuestErrand_c *errand = player != NULL ? player->mErrand.get(0) : NULL;
    if (errand != NULL && errand->mBase.mState == QUEST_ERRAND_DONE) {
        setMsgProc(&talk_c::msgErrandReport);
        addFriendship(5);
    } else {
        setMsgProc(&talk_c::msgTimeover2);
        addFriendship(-1);
    }
    startMsg();
}

static const char l_Q06_Report[] = "Q06_Report"; // 8046CA0C

// 80056BCC
int dAcNpcNml_c::talk_c::msgErrandReport(msgInfo_s *info) {
    setLooksMsg(info, l_Q06_Report, 0);
    setStepProc(&talk_c::stepErrandReport);
    return TRUE;
}

// 80056C30
BOOL dAcNpcNml_c::talk_c::stepErrandReport(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        BOOL full = player != NULL ? player->findEmptyPocket(0) == -1 : TRUE;
        msgFunc msg;
        if (full) {
            msg = &talk_c::msgErrandRewardFull;
        } else {
            msg = &talk_c::msgErrandReward;
        }
        setMsgProc(msg);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80056D14
int dAcNpcNml_c::talk_c::msgErrandReward(msgInfo_s *info) {
    setLooksMsg(info, l_Q_Item, 0);
    setHookProc(&talk_c::endErrandReward);
    setStepProc(&talk_c::stepErrandReward);
    return TRUE;
}

// 80056DA0
void dAcNpcNml_c::talk_c::endErrandReward(int arg) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimal_c *animal = getAnimal();
    int price = 0;
    mItem0 = dItem::ITEM_ID_NONE;
    u32 kind = animal != NULL ? animal->pickErrandReward(&mItem0, &price, player) : 3;
    if (mItem0 != dItem::ITEM_ID_NONE) {
        switch (kind) {
        case 0:
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

// 80056EF4
BOOL dAcNpcNml_c::talk_c::stepErrandReward(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        dQuestErrand_c *errand = player != NULL ? player->mErrand.get(0) : NULL;
        if (errand != NULL) {
            errand->clear();
        }
        setMsgProc(&talk_c::msgErrandEnd);
        startMsg();
        if (mItem0 != dItem::ITEM_ID_NONE) {
            requestItemAct(&mItem0, 0, 0);
        }
        mpNpc->mAudioObj.startSound(0x171F);
        return TRUE;
    }
    return FALSE;
}

static const char l_Q06_End[] = "Q06_End"; // 80750210

// 80056FD4
int dAcNpcNml_c::talk_c::msgErrandEnd(msgInfo_s *info) {
    setLooksMsg(info, l_Q06_End, 0);
    return TRUE;
}

// 80057000
int dAcNpcNml_c::talk_c::msgErrandRewardFull(msgInfo_s *info) {
    setLooksMsg(info, l_Q_ItemFull, 0);
    setStepProc(&talk_c::stepErrandRewardFull);
    return TRUE;
}

// 80057064
BOOL dAcNpcNml_c::talk_c::stepErrandRewardFull(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        dQuestErrand_c *errand = player != NULL ? player->mErrand.get(0) : NULL;
        if (errand != NULL) {
            errand->_18E = 1;
            dAnimal_c *animal = getAnimal();
            if (animal != NULL) {
                animal->completeErrandRequest(getNpcIdx(), player, FALSE);
            }
        }
        mpNpc->mAudioObj.startSound(0x171F);
        setMsgProc(&talk_c::msgErrandEnd);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80057170
BOOL dAcNpcNml_c::talk_c::stepErrandChoice(int kind) {
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
        setChoice(n, 1, 3, &talk_c::selErrandCon);
        setChoice(n + 1, 4, 3, &talk_c::selResumeTalk);
        setChoiceNum(n + 2);
        setChoiceCancel(n + 1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 80057344
void dAcNpcNml_c::talk_c::selErrandCon() {
    setMsgProc(&talk_c::msgErrandCon);
    startMsg();
}

static const char l_Q06_Con[] = "Q06_Con"; // 80750218

// 80057398
int dAcNpcNml_c::talk_c::msgErrandCon(msgInfo_s *info) {
    setLooksMsg(info, l_Q06_Con, 0);
    return TRUE;
}

// 800573C4
dSceneChange_c *dAcNpcNml_c::talk_c::getSceneChange() {
    return &gSceneChange;
}
