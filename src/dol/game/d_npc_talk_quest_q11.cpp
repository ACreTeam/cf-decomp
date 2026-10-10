// Villager talk for the sick-villager request (QUEST_KIND_REQUEST_5): the medicine, the thanks and the
// reward. .text 80051C68..80052EB0. See include/game/game/d_npc_talk_quest_q11.hpp.
#include <game/game/d_npc_talk_quest_q11.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_demo.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

// Player name word idx and its unit word idx + 1 (by the player's gender and the npc's looks).
static inline void setPlayerUnit(talk_c *talk, dAnimal_c *animal, const dPlayerID_c *pid, int idx) {
    talk->setPlayerName(pid, idx);
    u8 gender = pid->mGender;
    u16 unit = fn_800F3F38(gender, animal->mID.getLooks(1));
    if (unit == 0) {
        talk->clearWord(idx + 1);
    } else {
        talk->getController()->fn_801A5874(idx + 1, unit, "sys_STRING/STR_Unit");
    }
}

// 804A3A68: labels of the quest-talk states (getMsgLabel(TALK_QUEST, QUEST_TALK_SICK, state)).
const char *l_q11Labels[13] = {
    "Q11_GreetA", "Q11_GreetA", "Q11_GreetA", "Q11_GreetB",  "Q11_GreetB",  "Q11_GreetB", "Q11_GreetA",
    "Q11_GreetA", "Q11_GreetA", "Q11_Item",   "Q11_Full",    "Q11_Report1", "Q11_Report2",
};

// 80051C68
int dAcNpcNml_c::talk_c::msgSickQuest(msgInfo_s *info) {
    dPlayerID_c *pid;
    dPrivateData_c *player;
    u8 state;
    dQuestVillager_c *quest;
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    quest = &animal->mQuest.mQuest;
    if (!quest->mBase.isActive() || quest->mBase.getKind() != QUEST_KIND_REQUEST_5) {
        return FALSE;
    }
    player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL || !player->mPID.isFromTown()) {
        return FALSE;
    }
    state = quest->mBase.mState;
    pid = &player->mPID.player;
    dPlayerID_c *requester = &quest->mRequester;
    dQuestSick_c *sick = &dSaveData_c::getRaw()->mAnimals.mTown.mSick;
    if ((state == 1 || state == 2) && quest->findPlayer(pid) == -1) {
        return FALSE;
    }

    int param = quest->mMatchParam;
    int idx = 3;
    u16 code = 0;
    if (state == 1 || state == 2) {
        if (requester->isValid() && pid->isSame(requester)) {
            if (player->findEmptyPocket(0) != -1) {
                idx = 9;
            } else {
                idx = 10;
            }
        } else if (sick->hasPlayer(pid)) {
            idx = 11;
        } else {
            idx = 12;
            if (sick->countPlayers() == 0) {
                code = 10;
            }
        }
    } else {
        dPlayerID_c *visitor = sick->getPlayer(param);
        if (visitor != NULL && !visitor->isValid()) {
            dItem::Item medicine = dQuestSick_c::getMedicine();
            int num = countPocketsItem(&medicine, 0, NULL, NULL);
            idx = 0;
            if (num != 0) {
                idx = 6;
            }
        }
    }

    stepFunc step = &talk_c::stepSickVisit;
    endFunc hook = NULL;
    switch (idx) {
    case 0:
    case 1:
    case 2:
        code = param * 3 + (u32)cM::rndF(3.0f) + 1;
        step = &talk_c::stepSickGreet;
        break;
    case 6:
    case 7:
    case 8:
        code = param * 3 + (u32)cM::rndF(3.0f) + 1;
        step = &talk_c::stepSickChoice;
        break;
    case 9:
        hook = &talk_c::endSickReward;
        step = &talk_c::stepSickReward;
        break;
    case 10:
        hook = &talk_c::endSickRewardFull;
        step = &talk_c::stepSickRewardFull;
        break;
    case 11:
    case 12:
        step = &talk_c::stepSickReport;
        break;
    default:
        code = param * 3 + (u32)cM::rndF(3.0f) + 1;
        break;
    }

    const char *label = getMsgLabel(TALK_QUEST, QUEST_TALK_SICK, idx);
    if (label != NULL) {
        setProcSet(&l_talkEntrySets[TALK_QUEST]);
        setLooksMsg(info, label, code);
        if (hook) {
            setHookProc(hook);
        }
        setStepProc(step);
        setTopic(dNpc::msgMemory_c::KIND_QUEST, QUEST_TALK_SICK, 0);
        if (param < 3) {
            dPlayerID_c *visitor = sick->getPlayer(param);
            if (visitor != NULL && visitor->isValid()) {
                setPlayerUnit(this, animal, visitor, 0);
            }
        }
        if (requester->isValid()) {
            setPlayerUnit(this, animal, requester, 2);
        }
        return TRUE;
    }
    return FALSE;
}

// 80052190
BOOL dAcNpcNml_c::talk_c::stepSickChoice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoice(0, 0x48, 1, &talk_c::selSickMedicine);
        setChoice(1, 0x49, 1, &talk_c::selSickTalk);
        setChoiceNum(2);
        setChoiceCancel(1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 80052268
void dAcNpcNml_c::talk_c::selSickMedicine() {
    setMsgProc(&talk_c::msgSickMedicine);
    startMsg();
}

// 800522BC
int dAcNpcNml_c::talk_c::msgSickMedicine(msgInfo_s *info) {
    static const char l_Q11_Medicine[] = "Q11_Medicine";
    dAnimal_c *animal = getAnimal();
    if (animal != NULL) {
        int param = animal->mQuest.mQuest.mMatchParam;
        u16 code = 1;
        if (param < 3) {
            code = param * 3 + (u32)cM::rndF(3.0f) + 1;
        }
        setLooksMsg(info, l_Q11_Medicine, code);
        setStepProc(&talk_c::stepSickMedicine);
        return TRUE;
    }
    return FALSE;
}

// 8005237C
BOOL dAcNpcNml_c::talk_c::stepSickMedicine(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player != NULL) {
            dItem::Item medicine = dQuestSick_c::getMedicine();
            u16 mask = 0;
            countPocketsItem(&medicine, 0, &mask, player);
            mask = ~mask;
            reqSelectItem(mask, 0x22, TRUE);
            mResultProc = &talk_c::resSickMedicine;
            return TRUE;
        }
    }
    return FALSE;
}

// 80052440
void dAcNpcNml_c::talk_c::resSickMedicine() {
    BOOL done = FALSE;
    if (!isMenuInvalid()) {
        u32 slot = getMenuSelSlot();
        if (slot < 15) {
            dAnimal_c *animal = getAnimal();
            dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
            if (player != NULL && animal != NULL) {
                mItem0 = player->mPockets[slot];
                player->clearPocket(slot);
                mNextResultProc = &talk_c::resSickCure;
                requestItemActEx(9, &mItem0, 0, 0, 0, 2);
                done = TRUE;
                addFriendship(7);
            }
        }
    }
    if (!done) {
        setMsgProc(&talk_c::msgSickCancel);
        startMsg();
        reqMsgClose();
    }
}

// 80052578
void dAcNpcNml_c::talk_c::resSickCure() {
    setMsgProc(&talk_c::msgSickCure);
    startMsg();
    reqMsgClose();
}

// 800525D4
int dAcNpcNml_c::talk_c::msgSickCure(msgInfo_s *info) {
    static const char l_Q11_Cure[] = "Q11_Cure";
    dAnimal_c *animal = getAnimal();
    if (animal != NULL) {
        int param = animal->mQuest.mQuest.mMatchParam;
        u16 code = 1;
        if (param < 3) {
            code = param * 3 + (u32)cM::rndF(3.0f) + 1;
        }
        setLooksMsg(info, l_Q11_Cure, code);
        setStepProc(&talk_c::stepSickCure);
        return TRUE;
    }
    return FALSE;
}

// 80052694
BOOL dAcNpcNml_c::talk_c::stepSickCure(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        dAnimal_c *animal = getAnimal();
        if (player != NULL && animal != NULL) {
            dQuestVillager_c *quest = &animal->mQuest.mQuest;
            dPlayerID_c *pid = &player->mPID.player;
            int param = animal->mQuest.mQuest.mMatchParam;
            if (param < 3) {
                dSaveData_c::getTown()->mAnimals.mTown.mSick.setPlayer(param, pid);
            }
            quest->addPlayer(pid, TRUE);
            return TRUE;
        }
    }
    return FALSE;
}

// 8005274C
int dAcNpcNml_c::talk_c::msgSickCancel(msgInfo_s *info) {
    static const char l_Q11_Cancel[] = "Q11_Cancel";
    dAnimal_c *animal = getAnimal();
    if (animal != NULL) {
        int param = animal->mQuest.mQuest.mMatchParam;
        u16 code = 1;
        if (param < 3) {
            code = param * 3 + (u32)cM::rndF(3.0f) + 1;
        }
        setLooksMsg(info, l_Q11_Cancel, code);
        setStepProc(&talk_c::stepSickVisit);
        addFriendship(-1);
        return TRUE;
    }
    return FALSE;
}

// 80052818
BOOL dAcNpcNml_c::talk_c::stepSickVisit(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        dAnimal_c *animal = getAnimal();
        if (player != NULL && animal != NULL) {
            animal->mQuest.mQuest.addPlayer(&player->mPID.player, TRUE);
            return TRUE;
        }
    }
    return FALSE;
}

// 8005289C
void dAcNpcNml_c::talk_c::selSickTalk() {
    setMsgProc(&talk_c::msgSickTalk);
    startMsg();
}

// 800528F0
int dAcNpcNml_c::talk_c::msgSickTalk(msgInfo_s *info) {
    static const char l_Q11_Talk[] = "Q11_Talk";
    dAnimal_c *animal = getAnimal();
    if (animal != NULL) {
        int param = animal->mQuest.mQuest.mMatchParam;
        u16 code = 1;
        if (param < 3) {
            code = param * 3 + (u32)cM::rndF(3.0f) + 1;
        }
        setLooksMsg(info, l_Q11_Talk, code);
        setStepProc(&talk_c::stepSickTalk);
        return TRUE;
    }
    return FALSE;
}

// 800529B0
BOOL dAcNpcNml_c::talk_c::stepSickTalk(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        dAnimal_c *animal = getAnimal();
        if (player != NULL && animal != NULL) {
            animal->mQuest.mQuest.addPlayer(&player->mPID.player, TRUE);
            return TRUE;
        }
    }
    return FALSE;
}

// 80052A34
BOOL dAcNpcNml_c::talk_c::stepSickGreet(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgSickTalk);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80052AAC
void dAcNpcNml_c::talk_c::endSickReward(int arg) {
    endQuestTalk(arg);
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimal_c *animal = getAnimal();
    dItem::Item item;
    if (animal != NULL) {
        animal->pickSickReward(&item, player);
    }
    mItem0 = item;
    if (mItem0.isValid()) {
        if (player != NULL) {
            player->pickUp(&mItem0, FALSE);
        }
        setItemName(&mItem0, 4);
    }
}

// 80052B48
BOOL dAcNpcNml_c::talk_c::stepSickReward(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dAnimal_c *animal = getAnimal();
        dQuestVillager_c *quest = animal != NULL ? &animal->mQuest.mQuest : NULL;
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (quest != NULL && player != NULL) {
            quest->removePlayer(&player->mPID.player);
            if (quest->countPlayers(FALSE) == 0) {
                quest->clear();
            }
        }
        setMsgProc(&talk_c::msgSickEnd);
        startMsg();
        if (mItem0.isValid()) {
            requestItemAct(&mItem0, 0, 0);
        }
        mpNpc->mAudioObj.startSound(0x171F);
        return TRUE;
    }
    return FALSE;
}

// 80052C5C
int dAcNpcNml_c::talk_c::msgSickEnd(msgInfo_s *info) {
    static const char l_Q11_End[] = "Q11_End";
    setLooksMsg(info, l_Q11_End, 0);
    return TRUE;
}

// 80052C88
void dAcNpcNml_c::talk_c::endSickRewardFull(int arg) {
    endQuestTalk(arg);
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimal_c *animal = getAnimal();
    if (animal != NULL) {
        dItem::Item item;
        if (!animal->sendSickReward(&item, player, 0)) {
            animal->mQuest.mQuest.mBase.mState = 2;
        }
    }
}

// 80052D04
BOOL dAcNpcNml_c::talk_c::stepSickRewardFull(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dAnimal_c *animal = getAnimal();
        dQuestVillager_c *quest = animal != NULL ? &animal->mQuest.mQuest : NULL;
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (quest != NULL && player != NULL) {
            quest->removePlayer(&player->mPID.player);
            if (quest->mBase.mState == 1 && quest->countPlayers(FALSE) == 0) {
                quest->clear();
            }
        }
        mpNpc->mAudioObj.startSound(0x171F);
        setMsgProc(&talk_c::msgSickEnd);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80052E04
BOOL dAcNpcNml_c::talk_c::stepSickReport(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dAnimal_c *animal = getAnimal();
        dQuestVillager_c *quest = animal != NULL ? &animal->mQuest.mQuest : NULL;
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (quest != NULL && player != NULL) {
            quest->removePlayer(&player->mPID.player);
            if (quest->mBase.mState == 1 && quest->countPlayers(FALSE) == 0) {
                quest->clear();
            }
        }
        return TRUE;
    }
    return FALSE;
}
