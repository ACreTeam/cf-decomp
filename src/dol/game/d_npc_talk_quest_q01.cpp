// The villager's insect request (quest kind QUEST_KIND_REQUEST_INSECT, quest talk QUEST_TALK_INSECT).
// .text 8004FADC..80050F4C. See include/game/game/d_npc_talk_quest_q01.hpp.
#include <game/game/d_npc_talk_quest_q01.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_quest.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

// 8004FADC
int dAcNpcNml_c::talk_c::msgInsectOffer(msgInfo_s *info) {
    return startRequestOffer(info, QUEST_TALK_INSECT, QUEST_KIND_REQUEST_INSECT, &talk_c::stepInsectAccept);
}

// The accept step's data (its message proc and the unit-word file name). The accept step is the last
// function in .text, but its .data comes right after the offer function's: defined here, by name.
static talk_c::msgFunc l_insectReqMsg = &talk_c::msgInsectReq;
static char l_strUnit[] = "sys_STRING/STR_Unit";

// Names a player as word idx, with the unit word (idx + 1) for the villager's personality.
static inline void setPlayerUnit(talk_c *talk, const dPlayerID_c *player, int idx, dAnimal_c *animal) {
    talk->setPlayerName(player, idx);
    u8 gender = player->mGender;
    u16 unit = fn_800F3F38(gender, animal->mID.getLooks(1));
    if (unit == 0) {
        talk->clearWord(idx + 1);
    } else {
        talk->getController()->fn_801A5874(idx + 1, unit, l_strUnit);
    }
}

// 8004FB24
int dAcNpcNml_c::talk_c::msgInsectReq(msgInfo_s *info) {
    static const char l_Q01_Req[] = "Q01_Req";
    setLooksMsg(info, l_Q01_Req, 0);
    setStepProc(&talk_c::stepRequestChoice);
    return TRUE;
}

// Labels of the quest talk (getMsgLabel(TALK_QUEST, QUEST_TALK_INSECT, state)).
const char *l_q01Labels[8] = {
    l_Ai_Quest, l_Ai_Quest, l_Ai_Quest, "Q01_Lose2", "Q01_Lose1", l_Ai_Quest, "Q01_Req", "Q01_Req",
};

// 8004FB84
int dAcNpcNml_c::talk_c::msgInsectQuest(msgInfo_s *info) {
    if (isEventOngoing()) {
        return FALSE;
    }
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    dQuestVillager_c *quest = &animal->mQuest.mQuest;
    if (!quest->mBase.isActive() || quest->mBase.getKind() != QUEST_KIND_REQUEST_INSECT) {
        return FALSE;
    }
    u8 state;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL || !player->mPID.isFromTown()) {
        return FALSE;
    }
    if (player->isFlag0(0xD)) {
        return FALSE;
    }
    state = quest->mBase.mState;
    dPlayerID_c *pid = &player->mPID.player;
    dPlayerID_c *requester = &quest->mRequester;
    if (state == QUEST_REQUEST_WON && requester->isValid() && (requester->isSame(pid) || quest->findPlayer(pid) == -1)) {
        return FALSE;
    }
    if (quest->mBase.isPastDeadline(NULL) && quest->findPlayer(pid) == -1) {
        return FALSE;
    }
    if (quest->findPlayer(pid) != -1 && !quest->getPlayerFlag(pid)) {
        return FALSE;
    }
    f32 rnd = cM::rndF(100.0f);
    if (state == QUEST_REQUEST_OPEN && quest->findPlayer(pid) == -1 && rnd < 70.0f) {
        return FALSE;
    }

    int sub = 0;
    if (state == QUEST_REQUEST_WON && requester->isValid()) {
        sub = 4;
    } else if (quest->findPlayer(pid) == -1) {
        sub = 6;
    } else if (quest->mBase.isPastDeadline(NULL)) {
        sub = 3;
    } else if (countPocketsItem(&quest->mBase.mItem, 0, NULL, NULL)) {
        sub = 5;
    }

    if (sub >= 6 && sub < 8) {
        BOOL offer = TRUE;
        if (dAcNpc_c::isMultiPlay()) {
            offer = FALSE;
        }
        if (!dQuestBase_c::checkEventSchedule(QUEST_KIND_REQUEST_INSECT, NULL)) {
            offer = FALSE;
        }
        if (offer && !dQuestBase_c::checkTodayEvents(QUEST_KIND_REQUEST_INSECT)) {
            offer = FALSE;
        }
        if (!offer) {
            return FALSE;
        }
        return startQuestOffer(info, QUEST_TALK_INSECT, &talk_c::endQuestCommon, &talk_c::stepInsectAccept, TRUE);
    }

    stepFunc step = &talk_c::stepInsectChoice;
    mCountTalk = 1;
    switch (sub) {
    case 3:
    case 4:
        step = &talk_c::stepInsectLose;
        mCountTalk = 0;
        break;
    case 5:
        step = &talk_c::stepInsectGiveChoice;
        break;
    }
    const char *label = getMsgLabel(TALK_QUEST, QUEST_TALK_INSECT, sub);
    if (label != NULL) {
        setProcSet(&l_talkEntrySets[TALK_QUEST]);
        setLooksMsg(info, label, 0);
        setStepProc(step);
        setTopic(dNpc::msgMemory_c::KIND_QUEST, QUEST_TALK_INSECT, 0);
        dPlayerID_c *other = quest->pickOtherPlayer(pid);
        if (other != NULL) {
            setPlayerUnit(this, other, 0, animal);
        }
        if (requester->isValid()) {
            setPlayerUnit(this, requester, 2, animal);
        }
        if (quest->mBase.mItem.isValid()) {
            setItemName(&quest->mBase.mItem, 5);
        }
        return TRUE;
    }
    return FALSE;
}

// 80050094
BOOL dAcNpcNml_c::talk_c::stepInsectGiveChoice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        int num = 0;
        u16 code = 1;
        u8 range = num;
        hookFunc proc = getRollanChoice(&code, &range);
        if (proc) {
            setChoice(0, code, range, proc);
            num = 1;
        }
        hookFunc proc2 = getHarvestChoice(&code, &range, TRUE);
        if (proc2) {
            setChoice(num, code, range, proc2);
            num++;
        }
        setChoice(num, 0x3D, 1, &talk_c::selInsectGive);
        setChoice(num + 1, 4, 3, &talk_c::selResumeTalk);
        setChoiceNum(num + 2);
        setChoiceCancel(num + 1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 80050268
void dAcNpcNml_c::talk_c::selInsectGive() {
    dAnimal_c *animal = getAnimal();
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (animal != NULL && player != NULL) {
        dItem::Item *item = &animal->mQuest.mQuest.mBase.mItem;
        if (item->isValid()) {
            u16 mask = 0;
            countPocketsItem(item, 0, &mask, NULL);
            mask = ~mask;
            reqSelectItem(mask, 0x22, TRUE);
            mResultProc = &talk_c::resInsectGive;
        }
    }
}

// 80050318
void dAcNpcNml_c::talk_c::resInsectGive() {
    BOOL taken = FALSE;
    if (!isMenuInvalid()) {
        u32 slot = getMenuSelSlot();
        if (slot < 15) {
            dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
            if (player != NULL) {
                dItem::Item item = player->mPockets[slot];
                player->getPocketFlag(slot);
                player->clearPocket(slot);
                requestItemActEx(7, &item, 0, 0, 0, 2);
                mNextResultProc = &talk_c::resInsectGiven;
                taken = TRUE;
                mItem3 = item;
            }
        }
    }
    if (!taken) {
        setMsgProc(&talk_c::msgCancel);
        startMsg();
        reqMsgClose();
    }
}

// 80050448
void dAcNpcNml_c::talk_c::resInsectGiven() {
    setMsgProc(&talk_c::msgInsectWin);
    startMsg();
    reqMsgClose();
}

// 800504A4
int dAcNpcNml_c::talk_c::msgInsectWin(msgInfo_s *info) {
    static const char l_Q01_Win[] = "Q01_Win";
    setLooksMsg(info, l_Q01_Win, 0);
    setHookProc(&talk_c::endInsectWin);
    setStepProc(&talk_c::stepInsectWin);
    return TRUE;
}

// 8005052C
void dAcNpcNml_c::talk_c::endInsectWin(int arg) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimal_c *animal = getAnimal();
    dQuestVillager_c *quest = animal != NULL ? &animal->mQuest.mQuest : NULL;
    int price = 0;
    mItem0 = dItem::Item();
    dItem::Item wanted;
    int rank = 3;
    if (quest != NULL) {
        wanted = quest->mBase.mItem;
        rank = quest->mMatchParam;
    }
    int res;
    if (animal != NULL) {
        res = animal->pickInsectFishReward(&mItem0, &price, player, rank, &wanted);
    } else {
        res = 3;
    }
    addFriendship(5);
    if (mItem0.isValid()) {
        switch (res) {
        case 0:
            if (player != NULL) {
                player->pickUp(&mItem0, FALSE);
            }
            setItemName(&mItem0, 4);
            break;
        case 1:
            if (animal != NULL && animal->removeNewItem(&mItem0)) {
                fn_800F0FE4(getNpcIdx(), &mItem0);
            }
            if (player != NULL) {
                player->pickUp(&mItem0, FALSE);
            }
            setItemName(&mItem0, 4);
            break;
        default:
            if (price > 0 && player != NULL) {
                player->addMoney(price);
                setBells(price, 4);
            }
            break;
        }
    }
}

// 800506D0
BOOL dAcNpcNml_c::talk_c::stepInsectWin(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dAnimal_c *animal = getAnimal();
        dQuestVillager_c *quest = animal != NULL ? &animal->mQuest.mQuest : NULL;
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (quest != NULL && player != NULL) {
            quest->mBase.mState = QUEST_REQUEST_WON;
            quest->setRequester(&player->mPID.player);
            quest->removePlayer(&player->mPID.player);
            animal->mQuest.mWish.addValue(5);
        }
        if (quest != NULL && quest->countPlayers(FALSE) == 0) {
            quest->clear();
        }
        if (animal != NULL && mItem3.isValid() && animal->addNewItem(&mItem3)) {
            fn_800F0E9C(getNpcIdx(), &mItem3);
        }
        setMsgProc(&talk_c::msgInsectReward);
        startMsg();
        requestHandActD();
        return TRUE;
    }
    return FALSE;
}

// 80050828
int dAcNpcNml_c::talk_c::msgInsectReward(msgInfo_s *info) {
    static const char l_Q01_Win[] = "Q01_Win";
    setLooksMsg(info, l_Q01_Win, 4);
    setStepProc(&talk_c::stepInsectReward);
    return TRUE;
}

// 80050888
BOOL dAcNpcNml_c::talk_c::stepInsectReward(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgInsectRewardEnd);
        startMsg();
        if (mItem0.isValid()) {
            requestItemAct(&mItem0, 0, 0);
        }
        mpNpc->mAudioObj.startSound(0x171F);
        return TRUE;
    }
    return FALSE;
}

// 80050938
int dAcNpcNml_c::talk_c::msgInsectRewardEnd(msgInfo_s *info) {
    static const char l_Q01_Win[] = "Q01_Win";
    setLooksMsg(info, l_Q01_Win, 8);
    return TRUE;
}

// 80050964
BOOL dAcNpcNml_c::talk_c::stepInsectLose(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dAnimal_c *animal = getAnimal();
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (animal != NULL) {
            dQuestVillager_c *quest = &animal->mQuest.mQuest;
            if (player != NULL) {
                quest->removePlayer(&player->mPID.player);
            }
            if (quest->countPlayers(FALSE) == 0) {
                quest->clear();
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 800509F8
BOOL dAcNpcNml_c::talk_c::stepInsectChoice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        int num = 0;
        u16 code = 1;
        u8 range = num;
        hookFunc proc = getRollanChoice(&code, &range);
        if (proc) {
            setChoice(0, code, range, proc);
            num = 1;
        }
        hookFunc proc2 = getHarvestChoice(&code, &range, TRUE);
        if (proc2) {
            setChoice(num, code, range, proc2);
            num++;
        }
        setChoice(num, 1, 3, &talk_c::selInsectCon);
        setChoice(num + 1, 4, 3, &talk_c::selResumeTalk);
        setChoiceNum(num + 2);
        setChoiceCancel(num + 1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 80050BCC
void dAcNpcNml_c::talk_c::selInsectCon() {
    setMsgProc(&talk_c::msgInsectCon);
    startMsg();
}

// 80050C20
int dAcNpcNml_c::talk_c::msgInsectCon(msgInfo_s *info) {
    static const char l_Q01_Con[] = "Q01_Con";
    setLooksMsg(info, l_Q01_Con, 0);
    setStepProc(&talk_c::stepInsectCon);
    return TRUE;
}

// 80050C80
BOOL dAcNpcNml_c::talk_c::stepInsectCon(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgInsectConPlayers);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80050CF8
int dAcNpcNml_c::talk_c::msgInsectConPlayers(msgInfo_s *info) {
    static const char l_Q01_Con[] = "Q01_Con";
    dAnimal_c *animal = getAnimal();
    dQuestVillager_c *quest = animal != NULL ? &animal->mQuest.mQuest : NULL;
    u16 code = 7;
    if (quest != NULL) {
        u32 num = quest->countPlayers(FALSE);
        if (num != 0) {
            code = (num < 3 ? num - 1 : 2) + 7;
        }
    }
    setLooksMsg(info, l_Q01_Con, code);
    return TRUE;
}

// 80050D98
BOOL dAcNpcNml_c::talk_c::stepInsectAccept(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dQuestVillager_c *quest;
        dAnimal_c *animal = getAnimal();
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (animal != NULL && player != NULL) {
            quest = &animal->mQuest.mQuest;
            if (!quest->mBase.isActive()) {
                dItem::Item item;
                u8 param = 0;
                u32 mode = animal->pickRequestItem(&item, &param, QUEST_KIND_REQUEST_INSECT);
                quest->start(QUEST_KIND_REQUEST_INSECT, &player->mPID.player, &item, NULL, QUEST_DEADLINE_LIMIT, QUEST_REQUEST_OPEN);
                quest->mMatchMode = mode;
                quest->mMatchParam = param;
            } else {
                quest->addPlayer(&player->mPID.player, TRUE);
                dPlayerID_c *other = quest->pickOtherPlayer(&player->mPID.player);
                if (other != NULL) {
                    setPlayerUnit(this, other, 0, animal);
                }
            }
            dItem::Item *item = &quest->mBase.mItem;
            if (item->isValid()) {
                setItemName(item, 5);
            }
            setMsgProc(l_insectReqMsg);
            startMsg();
            return TRUE;
        }
    }
    return FALSE;
}
