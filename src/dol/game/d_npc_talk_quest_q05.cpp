// Villager talk for the furniture request (QUEST_KIND_REQUEST_FTR): the request, the delivery
// check, the reward and the winner's later visits. .text 8004E31C..8004FADC. See
// include/game/game/d_npc_talk_quest_q05.hpp.
#include <game/game/d_npc_talk_quest_q05.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_demo.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

// 8004E31C
int dAcNpcNml_c::talk_c::msgFtrOffer(msgInfo_s *info) {
    return startRequestOffer(info, QUEST_TALK_FTR, QUEST_KIND_REQUEST_FTR, &talk_c::stepFtrAccept);
}

// The accept step's data (its message proc and the unit-word file name). The accept step is the last
// function in .text, but its .data comes right after the offer function's: defined here, by name.
static talk_c::msgFunc l_ftrReqMsg = &talk_c::msgFtrReq;
static char l_strUnit[] = "sys_STRING/STR_Unit";

// Player name word idx and its unit word idx + 1 (by the player's gender and the npc's looks).
static inline void setPlayerUnit(talk_c *talk, dAnimal_c *animal, const dPlayerID_c *pid, int idx) {
    talk->setPlayerName(pid, idx);
    u8 gender = pid->mGender;
    u16 unit = fn_800F3F38(gender, animal->mID.getLooks(1));
    if (unit == 0) {
        talk->clearWord(idx + 1);
    } else {
        talk->getController()->fn_801A5874(idx + 1, unit, l_strUnit);
    }
}

// 8004E364
int dAcNpcNml_c::talk_c::msgFtrReq(msgInfo_s *info) {
    static const char l_Q05_Req[] = "Q05_Req";
    setLooksMsg(info, l_Q05_Req, 0);
    setStepProc(&talk_c::stepRequestChoice);
    return TRUE;
}

// 804A3610: labels of the quest-talk states (getMsgLabel(TALK_QUEST, QUEST_TALK_FTR, state)).
const char *l_q05Labels[13] = {
    l_Ai_Quest, l_Ai_Quest, l_Ai_Quest, "Q05_Lose2", "Q05_Lose1", l_Ai_Quest, l_Ai_Quest,
    l_Ai_Quest, l_Ai_Quest, "Q05_Req",   "Q05_Req",   "Q05_Return", "Q05_Comp",
};

// 8004E3C4
int dAcNpcNml_c::talk_c::msgFtrQuest(msgInfo_s *info) {
    u8 state;
    if (isEventOngoing()) {
        return FALSE;
    }
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    dQuestVillager_c *quest = &animal->mQuest.mQuest;
    if (!quest->mBase.isActive() || quest->mBase.getKind() != QUEST_KIND_REQUEST_FTR) {
        return FALSE;
    }
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
    if (state >= QUEST_REQUEST_WON && state < QUEST_REQUEST_NUM && requester->isValid()) {
        if (requester->isSame(pid)) {
            if (state == QUEST_REQUEST_RETURN || state == QUEST_REQUEST_COMPLETE ||
                (state == QUEST_REQUEST_DISPLAYED && !isCurrentSceneAttr(SCENE_ATTR_VILLAGER_HOUSE))) {
                return FALSE;
            }
        } else if (quest->findPlayer(pid) == -1) {
            return FALSE;
        }
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

    int idx = 0;
    if (state >= QUEST_REQUEST_WON && state < QUEST_REQUEST_NUM && requester->isValid()) {
        if (requester->isSame(pid)) {
            if (state == QUEST_REQUEST_WON) {
                idx = 11;
            } else {
                idx = 12;
            }
        } else {
            idx = 4;
        }
    } else if (quest->findPlayer(pid) == -1) {
        idx = 9;
    } else if (quest->mBase.isPastDeadline(NULL)) {
        idx = 3;
    } else if (countPocketsKind(NULL, 3, player, TRUE)) {
        idx = 5;
    }

    if (idx >= 9 && idx < 11) {
        BOOL ok = TRUE;
        if (dAcNpc_c::isMultiPlay()) {
            ok = FALSE;
        }
        if (!dQuestBase_c::checkEventSchedule(QUEST_KIND_REQUEST_FTR, NULL)) {
            ok = FALSE;
        }
        if (ok && !dQuestBase_c::checkTodayEvents(QUEST_KIND_REQUEST_FTR)) {
            ok = FALSE;
        }
        if (!ok) {
            return FALSE;
        }
        return startQuestOffer(info, QUEST_TALK_FTR, &talk_c::endQuestCommon, &talk_c::stepFtrAccept, TRUE);
    }

    stepFunc step = &talk_c::stepFtrTalkChoice;
    mCountTalk = TRUE;
    switch (idx) {
    case 3:
    case 4:
        step = &talk_c::stepFtrOver;
        mCountTalk = FALSE;
        break;
    case 5:
        step = &talk_c::stepFtrGiveChoice;
        break;
    case 11:
        step = &talk_c::stepFtrReturn;
        break;
    case 12:
        step = &talk_c::stepFtrComp;
        break;
    }

    const char *label = getMsgLabel(TALK_QUEST, QUEST_TALK_FTR, idx);
    if (label != NULL) {
        setProcSet(&l_talkEntrySets[TALK_QUEST]);
        setLooksMsg(info, label, 0);
        setStepProc(step);
        setTopic(dNpc::msgMemory_c::KIND_QUEST, QUEST_TALK_FTR, 0);
        dPlayerID_c *other = quest->pickOtherPlayer(pid);
        if (other != NULL) {
            setPlayerUnit(this, animal, other, 0);
        }
        if (requester->isValid()) {
            setPlayerUnit(this, animal, requester, 2);
        }
        setQ5Word(quest->mMatchMode, quest->mMatchParam, 5);
        return TRUE;
    }
    return FALSE;
}

// 8004E990
BOOL dAcNpcNml_c::talk_c::stepFtrGiveChoice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        int n = 0;
        u16 code = 1;
        u8 range = 0;
        hookFunc proc = getRollanChoice(&code, &range);
        if (proc != NULL) {
            setChoice(0, code, range, proc);
            n = 1;
        }
        hookFunc proc2 = getHarvestChoice(&code, &range, TRUE);
        if (proc2 != NULL) {
            setChoice(n, code, range, proc2);
            n++;
        }
        setChoice(n, 0x41, 1, &talk_c::selFtrGive);
        setChoice(n + 1, 4, 3, &talk_c::selResumeTalk);
        setChoiceNum(n + 2);
        setChoiceCancel(n + 1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 8004EB64
void dAcNpcNml_c::talk_c::selFtrGive() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        u16 mask = 0;
        countPocketsKind(&mask, 3, player, TRUE);
        mask = ~mask;
        reqSelectItem(mask, 0x22, TRUE);
        mResultProc = &talk_c::resFtrSelect;
    }
}

// 8004EBF0
void dAcNpcNml_c::talk_c::resFtrSelect() {
    BOOL done = FALSE;
    if (!isMenuInvalid()) {
        u32 slot = getMenuSelSlot();
        if (slot < 15) {
            dAnimal_c *animal = getAnimal();
            dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
            if (player != NULL && animal != NULL) {
                mItem0 = player->mPockets[slot];
                u32 result = animal->checkFtrRequest(&mItem0, animal->mQuest.mQuest.mMatchMode,
                                                     animal->mQuest.mQuest.mMatchParam);
                if (mItem0.isValid()) {
                    setItemName(&mItem0, 6);
                }
                requestItemActEx(7, &mItem0, 0, 0, 0, 2);
                if (result == 0) {
                    player->clearPocket(slot);
                    mNextResultProc = &talk_c::resFtrMatch;
                    mItem3 = mItem0;
                } else {
                    mNextResultProc = &talk_c::resFtrMismatch;
                }
                done = TRUE;
            }
        }
    }
    if (!done) {
        setMsgProc(&talk_c::msgCancel);
        startMsg();
        reqMsgClose();
    }
}

// 8004ED80
void dAcNpcNml_c::talk_c::resFtrMatch() {
    setMsgProc(&talk_c::msgFtrWin);
    startMsg();
    reqMsgClose();
}

// 8004EDDC
int dAcNpcNml_c::talk_c::msgFtrWin(msgInfo_s *info) {
    static const char l_Q05_Win[] = "Q05_Win";
    setLooksMsg(info, l_Q05_Win, 0);
    setHookProc(&talk_c::endFtrReward);
    setStepProc(&talk_c::stepFtrWin);
    return TRUE;
}

// 8004EE64
void dAcNpcNml_c::talk_c::endFtrReward(int arg) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimal_c *animal = getAnimal();
    dQuestVillager_c *quest = animal != NULL ? &animal->mQuest.mQuest : NULL;
    int price = 0;
    int mode = 7;
    if (quest != NULL) {
        mode = quest->mMatchMode;
    }
    dItem::Item item;
    u32 result;
    if (animal != NULL) {
        result = animal->pickFtrReward(&item, &price, player, mode, &mItem0);
    } else {
        result = 3;
    }
    addFriendship(5);
    mItem0 = item;
    if (mItem0.isValid()) {
        switch (result) {
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

// 8004F000
BOOL dAcNpcNml_c::talk_c::stepFtrWin(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dAnimal_c *animal = getAnimal();
        dQuestVillager_c *quest = animal != NULL ? &animal->mQuest.mQuest : NULL;
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (quest != NULL && player != NULL) {
            quest->mBase.mState = QUEST_REQUEST_WON;
            fn_800F12D8(getNpcIdx(), QUEST_KIND_REQUEST_FTR, QUEST_REQUEST_WON);
            quest->setRequester(&player->mPID.player);
            quest->removePlayer(&player->mPID.player);
            animal->mQuest.mWish.addValue(5);
            if (!fn_800DCEDC() && isCurrentSceneAttr(SCENE_ATTR_TOWN)) {
                animal->setQuestStarted();
            }
        }
        if (animal != NULL && mItem3.isValid()) {
            if (animal->addNewItem(&mItem3)) {
                fn_800F0E9C(getNpcIdx(), &mItem3);
            }
            if (mpMemory != NULL && mMemoryIdx < 16 && mpMemory->setPresent(&mItem3)) {
                fn_800F1514(getNpcIdx(), mMemoryIdx, &mItem3);
            }
        }
        setMsgProc(&talk_c::msgFtrWin2);
        startMsg();
        requestHandActD();
        return TRUE;
    }
    return FALSE;
}

// 8004F1A8
int dAcNpcNml_c::talk_c::msgFtrWin2(msgInfo_s *info) {
    static const char l_Q05_Win[] = "Q05_Win";
    setLooksMsg(info, l_Q05_Win, 4);
    setStepProc(&talk_c::stepFtrWin2);
    return TRUE;
}

// 8004F208
BOOL dAcNpcNml_c::talk_c::stepFtrWin2(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgFtrWin3);
        startMsg();
        if (mItem0.isValid()) {
            requestItemAct(&mItem0, 0, 0);
        }
        mpNpc->mAudioObj.startSound(0x171F);
        return TRUE;
    }
    return FALSE;
}

// 8004F2B8
int dAcNpcNml_c::talk_c::msgFtrWin3(msgInfo_s *info) {
    static const char l_Q05_Win[] = "Q05_Win";
    setLooksMsg(info, l_Q05_Win, 8);
    return TRUE;
}

// 8004F2E4
void dAcNpcNml_c::talk_c::resFtrMismatch() {
    setMsgProc(&talk_c::msgFtrNG);
    startMsg();
    reqMsgClose();
}

// 8004F340
int dAcNpcNml_c::talk_c::msgFtrNG(msgInfo_s *info) {
    static const char l_Q05_NG[] = "Q05_NG";
    u16 code = 1;
    dAnimal_c *animal = getAnimal();
    if (animal != NULL) {
        switch (animal->checkFtrRequest(&mItem0, animal->mQuest.mQuest.mMatchMode,
                                        animal->mQuest.mQuest.mMatchParam)) {
        case 1:
            code = 2;
            break;
        case 2:
            code = 3;
            break;
        }
    }
    setLooksMsg(info, l_Q05_NG, code);
    setStepProc(&talk_c::stepFtrNG);
    return TRUE;
}

// 8004F400
BOOL dAcNpcNml_c::talk_c::stepFtrNG(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        requestHandActE();
        return TRUE;
    }
    return FALSE;
}

// 8004F444
BOOL dAcNpcNml_c::talk_c::stepFtrOver(int kind) {
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

// 8004F4D8
BOOL dAcNpcNml_c::talk_c::stepFtrTalkChoice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        int n = 0;
        u16 code = 1;
        u8 range = 0;
        hookFunc proc = getRollanChoice(&code, &range);
        if (proc != NULL) {
            setChoice(0, code, range, proc);
            n = 1;
        }
        hookFunc proc2 = getHarvestChoice(&code, &range, TRUE);
        if (proc2 != NULL) {
            setChoice(n, code, range, proc2);
            n++;
        }
        setChoice(n, 1, 3, &talk_c::selFtrCon);
        setChoice(n + 1, 4, 3, &talk_c::selResumeTalk);
        setChoiceNum(n + 2);
        setChoiceCancel(n + 1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 8004F6AC
void dAcNpcNml_c::talk_c::selFtrCon() {
    setMsgProc(&talk_c::msgFtrCon);
    startMsg();
}

// 8004F700
int dAcNpcNml_c::talk_c::msgFtrCon(msgInfo_s *info) {
    static const char l_Q05_Con[] = "Q05_Con";
    setLooksMsg(info, l_Q05_Con, 0);
    setStepProc(&talk_c::stepFtrCon);
    return TRUE;
}

// 8004F760
BOOL dAcNpcNml_c::talk_c::stepFtrCon(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgFtrCon2);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 8004F7D8
int dAcNpcNml_c::talk_c::msgFtrCon2(msgInfo_s *info) {
    static const char l_Q05_Con[] = "Q05_Con";
    u16 code = 7;
    dAnimal_c *animal = getAnimal();
    if (animal != NULL) {
        u32 num = animal->mQuest.mQuest.countPlayers(FALSE);
        if (num != 0) {
            code = 7 + (num < 3 ? num - 1 : 2);
        }
    }
    setLooksMsg(info, l_Q05_Con, code);
    return TRUE;
}

// 8004F868
BOOL dAcNpcNml_c::talk_c::stepFtrReturn(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dAnimal_c *animal = getAnimal();
        if (animal != NULL) {
            animal->mQuest.mQuest.mBase.mState = QUEST_REQUEST_RETURN;
            fn_800F12D8(getNpcIdx(), QUEST_KIND_REQUEST_FTR, QUEST_REQUEST_RETURN);
        }
        return TRUE;
    }
    return FALSE;
}

// 8004F8DC
BOOL dAcNpcNml_c::talk_c::stepFtrComp(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dAnimal_c *animal = getAnimal();
        if (animal != NULL) {
            animal->mQuest.mQuest.mBase.mState = QUEST_REQUEST_COMPLETE;
        }
        return TRUE;
    }
    return FALSE;
}

// 8004F930
BOOL dAcNpcNml_c::talk_c::stepFtrAccept(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dQuestVillager_c *quest;
        dAnimal_c *animal = getAnimal();
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (animal != NULL && player != NULL) {
            quest = &animal->mQuest.mQuest;
            if (!quest->mBase.isActive()) {
                u8 param = 0;
                u32 mode = animal->pickRequestItem(NULL, &param, QUEST_KIND_REQUEST_FTR);
                dItem::Item item;
                quest->start(QUEST_KIND_REQUEST_FTR, &player->mPID.player, &item, NULL, QUEST_DEADLINE_LIMIT, QUEST_REQUEST_OPEN);
                quest->mMatchMode = mode;
                quest->mMatchParam = param;
            } else {
                quest->addPlayer(&player->mPID.player, TRUE);
                dPlayerID_c *other = quest->pickOtherPlayer(&player->mPID.player);
                if (other != NULL) {
                    setPlayerUnit(this, animal, other, 0);
                }
            }
            setQ5Word(quest->mMatchMode, quest->mMatchParam, 5);
            setMsgProc(l_ftrReqMsg);
            startMsg();
            return TRUE;
        }
    }
    return FALSE;
}
