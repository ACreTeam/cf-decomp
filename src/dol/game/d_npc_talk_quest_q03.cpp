// The villager's fossil request (quest kind QUEST_KIND_REQUEST_FOSSIL, quest talk QUEST_TALK_FOSSIL).
// .text 8004CBCC..8004E31C. See include/game/game/d_npc_talk_quest_q03.hpp.
#include <game/game/d_npc_talk_quest_q03.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_scene.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

// 8004CBCC
int dAcNpcNml_c::talk_c::msgFossilOffer(msgInfo_s *info) {
    return startRequestOffer(info, QUEST_TALK_FOSSIL, QUEST_KIND_REQUEST_FOSSIL, &talk_c::stepFossilAccept);
}

// The accept step's data (its message proc and the unit-word file name). The accept step is the last
// function in .text, but its .data comes right after the offer function's: defined here, by name.
static talk_c::msgFunc l_fossilReqMsg = &talk_c::msgFossilReq;
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

// 8004CC14
int dAcNpcNml_c::talk_c::msgFossilReq(msgInfo_s *info) {
    static const char l_Q03_Req[] = "Q03_Req";
    int code = 1;
    dAnimal_c *animal = getAnimal();
    if (animal != NULL && animal->mQuest.mQuest.mMatchMode == QUEST_MATCH_ITEM) {
        code = 4;
    }
    code += (int)cM::rndF(3.0f);
    setLooksMsg(info, l_Q03_Req, code);
    setStepProc(&talk_c::stepRequestChoice);
    return TRUE;
}

// Labels of the quest talk (getMsgLabel(TALK_QUEST, QUEST_TALK_FOSSIL, state)).
const char *l_q03Labels[10] = {
    l_Ai_Quest, l_Ai_Quest, l_Ai_Quest, "Q03_Lose2", "Q03_Lose1", l_Ai_Quest, "Q03_Req", "Q03_Req", "Q03_Return",
    "Q03_Comp",
};

// 8004CCC8
int dAcNpcNml_c::talk_c::msgFossilQuest(msgInfo_s *info) {
    if (isEventOngoing()) {
        return FALSE;
    }
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    dQuestVillager_c *quest = &animal->mQuest.mQuest;
    if (!quest->mBase.isActive() || quest->mBase.getKind() != QUEST_KIND_REQUEST_FOSSIL) {
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

    int sub = 0;
    if (state >= QUEST_REQUEST_WON && state < QUEST_REQUEST_NUM && requester->isValid()) {
        if (requester->isSame(pid)) {
            if (state == QUEST_REQUEST_WON) {
                sub = 8;
            } else {
                sub = 9;
            }
        } else {
            sub = 4;
        }
    } else if (quest->findPlayer(pid) == -1) {
        sub = 6;
    } else if (quest->mBase.isPastDeadline(NULL)) {
        sub = 3;
    } else if (countPocketsFossil(NULL, quest, player)) {
        sub = 5;
    }

    if (sub >= 6 && sub < 8) {
        BOOL offer = TRUE;
        if (dAcNpc_c::isMultiPlay()) {
            offer = FALSE;
        }
        if (!dQuestBase_c::checkEventSchedule(QUEST_KIND_REQUEST_FOSSIL, NULL)) {
            offer = FALSE;
        }
        if (offer && !dQuestBase_c::checkTodayEvents(QUEST_KIND_REQUEST_FOSSIL)) {
            offer = FALSE;
        }
        if (!offer) {
            return FALSE;
        }
        return startQuestOffer(info, QUEST_TALK_FOSSIL, &talk_c::endQuestCommon, &talk_c::stepFossilAccept, TRUE);
    }

    stepFunc step = &talk_c::stepFossilChoice;
    mCountTalk = 1;
    switch (sub) {
    case 3:
    case 4:
        step = &talk_c::stepFossilLose;
        mCountTalk = 0;
        break;
    case 5:
        step = &talk_c::stepFossilGiveChoice;
        break;
    case 8:
        step = &talk_c::stepFossilReturn;
        mCountTalk = 0;
        break;
    case 9:
        step = &talk_c::stepFossilComp;
        mCountTalk = 0;
        break;
    }
    const char *label = getMsgLabel(TALK_QUEST, QUEST_TALK_FOSSIL, sub);
    if (label != NULL) {
        setProcSet(&l_talkEntrySets[TALK_QUEST]);
        setLooksMsg(info, label, 0);
        setStepProc(step);
        setTopic(dNpc::msgMemory_c::KIND_QUEST, QUEST_TALK_FOSSIL, 0);
        dPlayerID_c *other = quest->pickOtherPlayer(pid);
        if (other != NULL) {
            setPlayerUnit(this, other, 0, animal);
        }
        if (requester->isValid()) {
            setPlayerUnit(this, requester, 2, animal);
        }
        if ((dQuestMatchMode_e)quest->mMatchMode == QUEST_MATCH_ANY) {
            getController()->fn_801A5874(5, 0x63, l_strUnit);
        } else if (quest->mBase.mItem.isValid()) {
            setItemName(&quest->mBase.mItem, 5);
        }
        return TRUE;
    }
    return FALSE;
}

// 8004D2D0
BOOL dAcNpcNml_c::talk_c::stepFossilGiveChoice(int kind) {
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
        setChoice(num, 0x3F, 1, &talk_c::selFossilGive);
        setChoice(num + 1, 4, 3, &talk_c::selResumeTalk);
        setChoiceNum(num + 2);
        setChoiceCancel(num + 1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 8004D4A4
void dAcNpcNml_c::talk_c::selFossilGive() {
    dAnimal_c *animal = getAnimal();
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (animal != NULL && player != NULL) {
        u16 mask = 0;
        countPocketsFossil(&mask, &animal->mQuest.mQuest, player);
        mask = ~mask;
        reqSelectItem(mask, 0x22, TRUE);
        mResultProc = &talk_c::resFossilGive;
    }
}

// 8004D548
void dAcNpcNml_c::talk_c::resFossilGive() {
    BOOL taken = FALSE;
    if (!isMenuInvalid()) {
        u32 slot = getMenuSelSlot();
        if (slot < 15) {
            dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
            if (player != NULL) {
                dItem::Item item = player->mPockets[slot];
                if (item.isValid()) {
                    setItemName(&item, 6);
                }
                mItem3 = item;
                player->clearPocket(slot);
                requestItemActEx(7, &item, 0, 0, 0, 2);
                mNextResultProc = &talk_c::resFossilGiven;
                taken = TRUE;
            }
        }
    }
    if (!taken) {
        setMsgProc(&talk_c::msgCancel);
        startMsg();
        reqMsgClose();
    }
}

// 8004D688
void dAcNpcNml_c::talk_c::resFossilGiven() {
    setMsgProc(&talk_c::msgFossilWin);
    startMsg();
    reqMsgClose();
}

// 8004D6E4
int dAcNpcNml_c::talk_c::msgFossilWin(msgInfo_s *info) {
    static const char l_Q03_Win[] = "Q03_Win";
    setLooksMsg(info, l_Q03_Win, 0);
    setHookProc(&talk_c::endFossilWin);
    setStepProc(&talk_c::stepFossilWin);
    return TRUE;
}

// 8004D76C
void dAcNpcNml_c::talk_c::endFossilWin(int arg) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimal_c *animal = getAnimal();
    dQuestVillager_c *quest = animal != NULL ? &animal->mQuest.mQuest : NULL;
    int price = 0;
    mItem0 = dItem::Item();
    dItem::Item wanted;
    int mode = 7;
    if (quest != NULL) {
        wanted = quest->mBase.mItem;
        mode = quest->mMatchMode;
    }
    int res;
    if (animal != NULL) {
        res = animal->pickFossilReward(&mItem0, &price, player, mode, &wanted);
    } else {
        res = 3;
    }
    addFriendship(10);
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

// 8004D910
BOOL dAcNpcNml_c::talk_c::stepFossilWin(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dAnimal_c *animal = getAnimal();
        dQuestVillager_c *quest = animal != NULL ? &animal->mQuest.mQuest : NULL;
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (quest != NULL && player != NULL) {
            quest->mBase.mState = QUEST_REQUEST_WON;
            fn_800F12D8(getNpcIdx(), QUEST_KIND_REQUEST_FOSSIL, QUEST_REQUEST_WON);
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
        setMsgProc(&talk_c::msgFossilReward);
        startMsg();
        requestHandActD();
        return TRUE;
    }
    return FALSE;
}

// 8004DAB8
int dAcNpcNml_c::talk_c::msgFossilReward(msgInfo_s *info) {
    static const char l_Q03_Win[] = "Q03_Win";
    setLooksMsg(info, l_Q03_Win, 4);
    setStepProc(&talk_c::stepFossilReward);
    return TRUE;
}

// 8004DB18
BOOL dAcNpcNml_c::talk_c::stepFossilReward(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgFossilRewardEnd);
        startMsg();
        if (mItem0.isValid()) {
            requestItemAct(&mItem0, 0, 0);
        }
        mpNpc->mAudioObj.startSound(0x171F);
        return TRUE;
    }
    return FALSE;
}

// 8004DBC8
int dAcNpcNml_c::talk_c::msgFossilRewardEnd(msgInfo_s *info) {
    static const char l_Q03_Win[] = "Q03_Win";
    setLooksMsg(info, l_Q03_Win, 8);
    return TRUE;
}

// 8004DBF4
BOOL dAcNpcNml_c::talk_c::stepFossilLose(int kind) {
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

// 8004DC88
BOOL dAcNpcNml_c::talk_c::stepFossilChoice(int kind) {
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
        setChoice(num, 1, 3, &talk_c::selFossilCon);
        setChoice(num + 1, 4, 3, &talk_c::selResumeTalk);
        setChoiceNum(num + 2);
        setChoiceCancel(num + 1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 8004DE5C
void dAcNpcNml_c::talk_c::selFossilCon() {
    setMsgProc(&talk_c::msgFossilCon);
    startMsg();
}

// 8004DEB0
int dAcNpcNml_c::talk_c::msgFossilCon(msgInfo_s *info) {
    static const char l_Q03_Con[] = "Q03_Con";
    int code = 1;
    dAnimal_c *animal = getAnimal();
    if (animal != NULL && animal->mQuest.mQuest.mMatchMode == QUEST_MATCH_ITEM &&
        animal->mQuest.mQuest.mBase.mItem.isValid()) {
        code = 4;
    }
    code += (int)cM::rndF(3.0f);
    setLooksMsg(info, l_Q03_Con, code);
    setStepProc(&talk_c::stepFossilCon);
    return TRUE;
}

// 8004DF70
BOOL dAcNpcNml_c::talk_c::stepFossilCon(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgFossilConPlayers);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 8004DFE8
int dAcNpcNml_c::talk_c::msgFossilConPlayers(msgInfo_s *info) {
    static const char l_Q03_Con[] = "Q03_Con";
    dAnimal_c *animal = getAnimal();
    dQuestVillager_c *quest = animal != NULL ? &animal->mQuest.mQuest : NULL;
    u16 code = 7;
    if (quest != NULL) {
        u32 num = quest->countPlayers(FALSE);
        if (num != 0) {
            code = (num < 3 ? num - 1 : 2) + 7;
        }
    }
    setLooksMsg(info, l_Q03_Con, code);
    return TRUE;
}

// 8004E088
BOOL dAcNpcNml_c::talk_c::stepFossilReturn(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dAnimal_c *animal = getAnimal();
        if (animal != NULL) {
            animal->mQuest.mQuest.mBase.mState = QUEST_REQUEST_RETURN;
            fn_800F12D8(getNpcIdx(), QUEST_KIND_REQUEST_FOSSIL, QUEST_REQUEST_RETURN);
        }
        return TRUE;
    }
    return FALSE;
}

// 8004E0FC
BOOL dAcNpcNml_c::talk_c::stepFossilComp(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dAnimal_c *animal = getAnimal();
        if (animal != NULL) {
            animal->mQuest.mQuest.mBase.mState = QUEST_REQUEST_COMPLETE;
        }
        return TRUE;
    }
    return FALSE;
}

// 8004E150
BOOL dAcNpcNml_c::talk_c::stepFossilAccept(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dQuestVillager_c *quest;
        dAnimal_c *animal = getAnimal();
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (animal != NULL && player != NULL) {
            quest = &animal->mQuest.mQuest;
            if (!quest->mBase.isActive()) {
                dItem::Item item;
                u32 mode = animal->pickRequestItem(&item, NULL, QUEST_KIND_REQUEST_FOSSIL);
                quest->start(QUEST_KIND_REQUEST_FOSSIL, &player->mPID.player, &item, NULL, QUEST_DEADLINE_LIMIT, QUEST_REQUEST_OPEN);
                quest->mMatchMode = mode;
            } else {
                quest->addPlayer(&player->mPID.player, TRUE);
                dPlayerID_c *other = quest->pickOtherPlayer(&player->mPID.player);
                if (other != NULL) {
                    setPlayerUnit(this, other, 0, animal);
                }
            }
            if ((dQuestMatchMode_e)quest->mMatchMode == QUEST_MATCH_ANY) {
                getController()->fn_801A5874(5, 0x63, l_strUnit);
            } else {
                dItem::Item *item = &quest->mBase.mItem;
                if (item->isValid()) {
                    setItemName(item, 5);
                }
            }
            setMsgProc(l_fossilReqMsg);
            startMsg();
            return TRUE;
        }
    }
    return FALSE;
}
