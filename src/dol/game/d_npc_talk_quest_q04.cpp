// Villager talk for the clothing request (QUEST_KIND_REQUEST_CLOTH): the request, the delivery
// check and its reward. .text 80049FDC..8004B75C. See include/game/game/d_npc_talk_quest_q04.hpp.
#include <game/game/d_npc_talk_quest_q04.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_demo.hpp>

typedef dAcNpcNml_c::talk_c talk_c;


// 80049FDC
int dAcNpcNml_c::talk_c::msgClothOffer(msgInfo_s *info) {
    return startRequestOffer(info, QUEST_TALK_CLOTH, QUEST_KIND_REQUEST_CLOTH, &talk_c::stepClothAccept);
}

// The accept step's data (its message proc and the unit-word file name). The accept step is the last
// function in .text, but its .data comes right after the offer function's: defined here, by name.
static talk_c::msgFunc l_clothReqMsg = &talk_c::msgClothReq;
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

// 8004A024
int dAcNpcNml_c::talk_c::msgClothReq(msgInfo_s *info) {
    static const char l_Q04_Req[] = "Q04_Req";
    int code = 1;
    dAnimal_c *animal = getAnimal();
    if (animal != NULL && animal->mQuest.mQuest.mMatchMode == QUEST_MATCH_ITEM) {
        code = 4;
    }
    code += (int)cM::rndF(3.0f);
    setLooksMsg(info, l_Q04_Req, code);
    setStepProc(&talk_c::stepRequestChoice);
    return TRUE;
}

// 804A2FC0: labels of the quest-talk states (getMsgLabel(TALK_QUEST, QUEST_TALK_CLOTH, state)).
const char *l_q04Labels[16] = {
    l_Ai_Quest, l_Ai_Quest, l_Ai_Quest, l_Ai_Quest, l_Ai_Quest, l_Ai_Quest, "Q04_Lose2", "Q04_Lose1",
    l_Ai_Quest, l_Ai_Quest, l_Ai_Quest, l_Ai_Quest, l_Ai_Quest, l_Ai_Quest, "Q04_Req",   "Q04_Req",
};

// 8004A0D8
int dAcNpcNml_c::talk_c::msgClothQuest(msgInfo_s *info) {
    if (isEventOngoing()) {
        return FALSE;
    }
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    dQuestVillager_c *quest = &animal->mQuest.mQuest;
    if (!quest->mBase.isActive() || quest->mBase.getKind() != QUEST_KIND_REQUEST_CLOTH) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL || !player->mPID.isFromTown()) {
        return FALSE;
    }
    if (player->isFlag0(0xD)) {
        return FALSE;
    }
    u8 state = quest->mBase.mState;
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

    int idx = 0;
    if (state == QUEST_REQUEST_WON && requester->isValid()) {
        idx = 7;
    } else if (quest->findPlayer(pid) == -1) {
        idx = 14;
    } else if (quest->mBase.isPastDeadline(NULL)) {
        idx = 6;
    } else if (countPocketsKind(NULL, 4, player, TRUE)) {
        idx = 8;
    }

    if (idx >= 14 && idx < 16) {
        BOOL ok = TRUE;
        if (dAcNpc_c::isMultiPlay()) {
            ok = FALSE;
        }
        if (!dQuestBase_c::checkEventSchedule(QUEST_KIND_REQUEST_CLOTH, NULL)) {
            ok = FALSE;
        }
        if (ok && !dQuestBase_c::checkTodayEvents(QUEST_KIND_REQUEST_CLOTH)) {
            ok = FALSE;
        }
        if (!ok) {
            return FALSE;
        }
        return startQuestOffer(info, QUEST_TALK_CLOTH, &talk_c::endQuestCommon, &talk_c::stepClothAccept, TRUE);
    }

    stepFunc step = &talk_c::stepClothTalkChoice;
    mCountTalk = TRUE;
    switch (idx) {
    case 6:
    case 7:
        step = &talk_c::stepClothOver;
        mCountTalk = FALSE;
        break;
    case 8:
        step = &talk_c::stepClothGiveChoice;
        break;
    }

    const char *label = getMsgLabel(TALK_QUEST, QUEST_TALK_CLOTH, idx);
    if (label != NULL) {
        setProcSet(&l_talkEntrySets[TALK_QUEST]);
        setLooksMsg(info, label, 0);
        setStepProc(step);
        setTopic(dNpc::msgMemory_c::KIND_QUEST, QUEST_TALK_CLOTH, 0);
        dPlayerID_c *other = quest->pickOtherPlayer(pid);
        if (other != NULL) {
            setPlayerUnit(this, animal, other, 0);
        }
        if (requester->isValid()) {
            setPlayerUnit(this, animal, requester, 2);
        }
        if (quest->mBase.mItem.isValid()) {
            setItemName(&quest->mBase.mItem, 5);
        }
        setQ4Word(quest->mMatchMode, 6);
        return TRUE;
    }
    return FALSE;
}

// 8004A5F8
BOOL dAcNpcNml_c::talk_c::stepClothGiveChoice(int kind) {
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
        setChoice(n, 0x40, 1, &talk_c::selClothGive);
        setChoice(n + 1, 4, 3, &talk_c::selResumeTalk);
        setChoiceNum(n + 2);
        setChoiceCancel(n + 1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 8004A7CC
void dAcNpcNml_c::talk_c::selClothGive() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        u16 mask = 0;
        countPocketsKind(&mask, 4, player, TRUE);
        mask = ~mask;
        reqSelectItem(mask, 0x22, TRUE);
        mResultProc = &talk_c::resClothSelect;
    }
}

// 8004A858
void dAcNpcNml_c::talk_c::resClothSelect() {
    BOOL done = FALSE;
    if (!isMenuInvalid()) {
        u32 slot = getMenuSelSlot();
        if (slot < 15) {
            dAnimal_c *animal = getAnimal();
            dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
            if (player != NULL && animal != NULL) {
                mItem0 = player->mPockets[slot];
                BOOL match = animal->isClothRequestMatch(&mItem0, animal->mQuest.mQuest.mMatchMode,
                                                         &animal->mQuest.mQuest.mBase.mItem);
                if (mItem0.isValid()) {
                    setItemName(&mItem0, 7);
                }
                mItem1 = mItem0;
                requestItemActEx(7, &mItem0, 0, 0, 0, 2);
                if (match) {
                    player->clearPocket(slot);
                    mNextResultProc = &talk_c::resClothMatch;
                } else {
                    mNextResultProc = &talk_c::resClothMismatch;
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

// 8004A9E8
void dAcNpcNml_c::talk_c::resClothMatch() {
    setMsgProc(&talk_c::msgClothWin);
    startMsg();
    reqMsgClose();
}

// 8004AA44
int dAcNpcNml_c::talk_c::msgClothWin(msgInfo_s *info) {
    static const char l_Q04_Win[] = "Q04_Win";
    setLooksMsg(info, l_Q04_Win, 0);
    setHookProc(&talk_c::endClothReward);
    setStepProc(&talk_c::stepClothWin);
    return TRUE;
}

// 8004AACC
void dAcNpcNml_c::talk_c::endClothReward(int arg) {
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
        result = animal->pickClothReward(&item, &price, player, mode, &mItem0);
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

// 8004AC68
BOOL dAcNpcNml_c::talk_c::stepClothWin(int kind) {
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
        setMsgProc(&talk_c::msgClothWin2);
        startMsg();
        if (mItem1.isValid()) {
            animal->setCloth(&mItem1);
            fn_800F1A68(getNpcIdx(), &animal->mCloth);
            dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
            npc->onDaubClothChange();
            npc->offDaubClothChanged();
        }
        requestHandActC();
        mResultProc = &talk_c::resClothWear;
        return TRUE;
    }
    return FALSE;
}

// 8004ADBC
void dAcNpcNml_c::talk_c::resClothWear() {
    dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
    npc->offDaubClothChange();
    npc->onDaubClothChanged();
}

// 8004ADF4
int dAcNpcNml_c::talk_c::msgClothWin2(msgInfo_s *info) {
    static const char l_Q04_Win[] = "Q04_Win";
    setLooksMsg(info, l_Q04_Win, 4);
    setStepProc(&talk_c::stepClothWin2);
    return TRUE;
}

// 8004AE54
BOOL dAcNpcNml_c::talk_c::stepClothWin2(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgClothWin3);
        startMsg();
        if (mItem0.isValid()) {
            requestItemAct(&mItem0, 0, 0);
        }
        mpNpc->mAudioObj.startSound(0x171F);
        return TRUE;
    }
    return FALSE;
}

// 8004AF04
int dAcNpcNml_c::talk_c::msgClothWin3(msgInfo_s *info) {
    static const char l_Q04_Win[] = "Q04_Win";
    setLooksMsg(info, l_Q04_Win, 8);
    return TRUE;
}

// 8004AF30
void dAcNpcNml_c::talk_c::resClothMismatch() {
    setMsgProc(&talk_c::msgClothNG);
    startMsg();
    reqMsgClose();
}

// 8004AF8C
int dAcNpcNml_c::talk_c::msgClothNG(msgInfo_s *info) {
    static const char l_Q04_NG[] = "Q04_NG";
    u16 code = 1;
    dAnimal_c *animal = getAnimal();
    if (animal != NULL) {
        switch (animal->mQuest.mQuest.mMatchMode) {
        case QUEST_MATCH_ANY:
            if (mItem0.isSame(animal->mCloth)) {
                code = 1;
            } else {
                code = 2;
            }
            break;
        case QUEST_MATCH_LIKED_STYLE:
            if (mItem0.isSame(animal->mCloth)) {
                code = 1;
            } else {
                code = 4;
            }
            break;
        default:
            code = 3;
            break;
        }
    }
    if (animal != NULL) {
        s8 look = animal->mTemplate.getLikedStyle();
        dItem::nameLook_c name;
        setLookName(&name, look);
        getController()->setWord(6, &name);
    }
    setLooksMsg(info, l_Q04_NG, code);
    setStepProc(&talk_c::stepClothNG);
    return TRUE;
}

// 8004B0D8
BOOL dAcNpcNml_c::talk_c::stepClothNG(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        requestHandActE();
        return TRUE;
    }
    return FALSE;
}

// 8004B11C
BOOL dAcNpcNml_c::talk_c::stepClothOver(int kind) {
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

// 8004B1B0
BOOL dAcNpcNml_c::talk_c::stepClothTalkChoice(int kind) {
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
        setChoice(n, 1, 3, &talk_c::selClothCon);
        setChoice(n + 1, 4, 3, &talk_c::selResumeTalk);
        setChoiceNum(n + 2);
        setChoiceCancel(n + 1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 8004B384
void dAcNpcNml_c::talk_c::selClothCon() {
    setMsgProc(&talk_c::msgClothCon);
    startMsg();
}

// 8004B3D8
int dAcNpcNml_c::talk_c::msgClothCon(msgInfo_s *info) {
    static const char l_Q04_ConA[] = "Q04_ConA";
    static const char l_Q04_ConB[] = "Q04_ConB";
    const char *label = l_Q04_ConA;
    dAnimal_c *animal = getAnimal();
    if (animal != NULL && animal->mQuest.mQuest.mMatchMode == QUEST_MATCH_ITEM) {
        label = l_Q04_ConB;
    }
    setLooksMsg(info, label, 0);
    setStepProc(&talk_c::stepClothCon);
    return TRUE;
}

// 8004B47C
BOOL dAcNpcNml_c::talk_c::stepClothCon(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgClothCon2);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 8004B4F4
int dAcNpcNml_c::talk_c::msgClothCon2(msgInfo_s *info) {
    static const char l_Q04_ConA[] = "Q04_ConA";
    static const char l_Q04_ConB[] = "Q04_ConB";
    const char *label = l_Q04_ConA;
    u16 code = 7;
    dAnimal_c *animal = getAnimal();
    if (animal != NULL) {
        dQuestVillager_c *quest = &animal->mQuest.mQuest;
        if (quest->mMatchMode == QUEST_MATCH_ITEM) {
            label = l_Q04_ConB;
        }
        u32 num = quest->countPlayers(FALSE);
        if (num != 0) {
            code = 7 + (num < 3 ? num - 1 : 2);
        }
    }
    setLooksMsg(info, label, code);
    return TRUE;
}

// 8004B5A8
BOOL dAcNpcNml_c::talk_c::stepClothAccept(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dQuestVillager_c *quest;
        dAnimal_c *animal = getAnimal();
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (animal != NULL && player != NULL) {
            quest = &animal->mQuest.mQuest;
            if (!quest->mBase.isActive()) {
                dItem::Item item;
                u32 mode = animal->pickRequestItem(&item, NULL, QUEST_KIND_REQUEST_CLOTH);
                quest->start(QUEST_KIND_REQUEST_CLOTH, &player->mPID.player, &item, NULL, QUEST_DEADLINE_LIMIT, QUEST_REQUEST_OPEN);
                quest->mMatchMode = mode;
            } else {
                quest->addPlayer(&player->mPID.player, TRUE);
                dPlayerID_c *other = quest->pickOtherPlayer(&player->mPID.player);
                if (other != NULL) {
                    setPlayerUnit(this, animal, other, 0);
                }
            }
            const dItem::Item *want = &quest->mBase.mItem;
            if (want->isValid()) {
                setItemName(want, 5);
            }
            setQ4Word(quest->mMatchMode, 6);
            setMsgProc(l_clothReqMsg);
            startMsg();
            return TRUE;
        }
    }
    return FALSE;
}
