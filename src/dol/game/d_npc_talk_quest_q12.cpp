// The villager's lost-key request (quest kind QUEST_KIND_REQUEST_6, quest talk QUEST_TALK_LOST_KEY).
// .text 80050F4C..80051C68. See include/game/game/d_npc_talk_quest_q12.hpp.
#include <game/game/d_npc_talk_quest_q12.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

// Names a player as word idx, with the unit word (idx + 1) for the villager's personality.
static inline void setPlayerUnit(talk_c *talk, const dPlayerID_c *player, int idx, dAnimal_c *animal) {
    talk->setPlayerName(player, idx);
    u8 gender = player->mGender;
    u16 unit = fn_800F3F38(gender, animal->mID.getLooks(1));
    if (unit == 0) {
        talk->clearWord(idx + 1);
    } else {
        talk->getController()->fn_801A5874(idx + 1, unit, "sys_STRING/STR_Unit");
    }
}

// Labels of the quest talk (getMsgLabel(TALK_QUEST, QUEST_TALK_LOST_KEY, state)).
const char *l_q12Labels[7] = {
    "Q12_Con1", "Q12_Report2", "Q12_Report1", "Q12_Con2", "Q12_Con2", "Q12_Req", "Q12_Visit",
};

// 80050F4C
int dAcNpcNml_c::talk_c::msgLostKeyQuest(msgInfo_s *info) {
    u8 state;
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    dQuestVillager_c *quest = &animal->mQuest.mQuest;
    if (!quest->mBase.isActive() || quest->mBase.getKind() != QUEST_KIND_REQUEST_6) {
        return FALSE;
    }
    BOOL fromTown = TRUE;
    dPlayerID_c *pid;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL || !player->mPID.isFromTown()) {
        fromTown = FALSE;
    }
    state = quest->mBase.mState;
    pid = &player->mPID.player;
    dPlayerID_c *requester = &quest->mRequester;
    if (state == 2) {
        if (requester->isValid() && requester->isSame(pid)) {
            return FALSE;
        }
        if (quest->findPlayer(pid) == -1) {
            return FALSE;
        }
        if (!fromTown) {
            return FALSE;
        }
    }

    int sub = 0;
    if (!fromTown) {
        sub = 6;
    } else if (state == 2) {
        if (requester->isValid()) {
            sub = 2;
        } else {
            sub = 1;
        }
    } else if (quest->findPlayer(pid) == -1) {
        sub = 5;
    } else if (countPocketsKind(NULL, 0x35, player, TRUE)) {
        sub = 3;
    }

    stepFunc step = NULL;
    switch (sub) {
    case 1:
    case 2:
        step = &talk_c::stepLostKeyReport;
        break;
    case 3:
    case 4:
        step = &talk_c::stepLostKeyChoice;
        break;
    case 5:
        step = &talk_c::stepLostKeyVisit;
        break;
    }
    const char *label = getMsgLabel(TALK_QUEST, QUEST_TALK_LOST_KEY, sub);
    if (label != NULL) {
        setProcSet(&l_talkEntrySets[TALK_QUEST]);
        setLooksMsg(info, label, 0);
        setStepProc(step);
        setTopic(dNpc::msgMemory_c::KIND_QUEST, QUEST_TALK_LOST_KEY, 0);
        if (requester->isValid()) {
            setPlayerUnit(this, requester, 0, animal);
        }
        return TRUE;
    }
    return FALSE;
}

// 8005125C
BOOL dAcNpcNml_c::talk_c::stepLostKeyChoice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoiceProc(0, &talk_c::selLostKeyGive);
        setChoiceNum(2);
        setChoiceCancel(1);
        return TRUE;
    }
    return FALSE;
}

// 800512F0
void dAcNpcNml_c::talk_c::selLostKeyGive() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        u16 mask = 0;
        countPocketsKind(&mask, 0x35, player, TRUE);
        mask = ~mask;
        reqSelectItem(mask, 0x22, 0x78, TRUE);
        mResultProc = &talk_c::resLostKeyGive;
    }
}

// 80051380
void dAcNpcNml_c::talk_c::resLostKeyGive() {
    BOOL taken = FALSE;
    if (!isMenuInvalid()) {
        u32 slot = getMenuSelSlot();
        if (slot < 15) {
            dAnimal_c *animal = getAnimal();
            dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
            if (player != NULL && animal != NULL) {
                mItem0 = player->mPockets[slot];
                if (animal->mQuest.mQuest.mBase.mItem.isSame(mItem0)) {
                    player->clearPocket(slot);
                    mNextResultProc = &talk_c::resLostKeyOK;
                } else {
                    mNextResultProc = &talk_c::resLostKeyNG;
                }
                requestItemActEx(7, &mItem0, 0, 0, 0, 2);
                taken = TRUE;
            }
        }
    }
    if (!taken) {
        setMsgProc(&talk_c::msgLostKeyCancel);
        startMsg();
        reqMsgClose();
    }
}

// 800514E0
void dAcNpcNml_c::talk_c::resLostKeyOK() {
    setMsgProc(&talk_c::msgLostKeyOK);
    startMsg();
    reqMsgClose();
}

// 8005153C
int dAcNpcNml_c::talk_c::msgLostKeyOK(msgInfo_s *info) {
    static const char l_Q12_KeyOK[] = "Q12_KeyOK";
    setLooksMsg(info, l_Q12_KeyOK, 0);
    setStepProc(&talk_c::stepLostKeyOK);
    return TRUE;
}

// 800515A0
BOOL dAcNpcNml_c::talk_c::stepLostKeyOK(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgLostKeyItem);
        startMsg();
        requestHandActD();
        addFriendship(5);
        return TRUE;
    }
    return FALSE;
}

// 8005162C
int dAcNpcNml_c::talk_c::msgLostKeyItem(msgInfo_s *info) {
    static const char l_Q12_Item[] = "Q12_Item";
    setLooksMsg(info, l_Q12_Item, 0);
    setHookProc(&talk_c::endLostKeyItem);
    setStepProc(&talk_c::stepLostKeyItem);
    return TRUE;
}

// 800516B8
void dAcNpcNml_c::talk_c::endLostKeyItem(int arg) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimal_c *animal = getAnimal();
    int price = 0;
    dItem::Item item;
    int res;
    if (animal != NULL) {
        res = animal->pickLostItemReward(&item, &price, player);
    } else {
        res = 3;
    }
    mItem0 = item;
    if (mItem0.isValid()) {
        switch (res) {
        case 0:
            if (player != NULL) {
                player->pickUp(&mItem0, FALSE);
            }
            setItemName(&mItem0, 2);
            break;
        case 1:
            if (animal != NULL && animal->removeNewItem(&mItem0)) {
                fn_800F0FE4(getNpcIdx(), &mItem0);
            }
            if (player != NULL) {
                player->pickUp(&mItem0, FALSE);
            }
            setItemName(&mItem0, 2);
            break;
        default:
            if (price > 0 && player != NULL) {
                player->addMoney(price);
                setBells(price, 2);
            }
            break;
        }
    }
}

// 80051810
BOOL dAcNpcNml_c::talk_c::stepLostKeyItem(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dAnimal_c *animal = getAnimal();
        dQuestVillager_c *quest = animal != NULL ? &animal->mQuest.mQuest : NULL;
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (quest != NULL && player != NULL) {
            quest->mBase.mState = 2;
            fn_800F12D8(getNpcIdx(), QUEST_KIND_REQUEST_6, 2);
            quest->setRequester(&player->mPID.player);
            quest->removePlayer(&player->mPID.player);
            if (!fn_800DCEDC() && isCurrentSceneAttr(SCENE_ATTR_TOWN) && animal != NULL) {
                animal->setQuestStarted();
            }
            u8 minute = calcHeldItemChangeMinute((u8)dTime_c::getCurrent()->min, FALSE);
            animal->mHeldItemChangeMinute = minute;
            fn_800F0054(getNpcIdx(), minute);
            if (quest->countPlayers(FALSE) == 0) {
                dSaveTown_c *town = dSaveData_c::getTown();
                town->mAnimals.mTown.mLostItem.setTime(*dTime_c::getCurrent());
                quest->clear();
            }
        }
        setMsgProc(&talk_c::msgLostKeyItemEnd);
        startMsg();
        if (mItem0.isValid()) {
            requestItemAct(&mItem0, 0, 0);
        }
        mpNpc->mAudioObj.startSound(0x171F);
        return TRUE;
    }
    return FALSE;
}

// 800519D4
int dAcNpcNml_c::talk_c::msgLostKeyItemEnd(msgInfo_s *info) {
    static const char l_Q12_Item[] = "Q12_Item";
    setLooksMsg(info, l_Q12_Item, 4);
    return TRUE;
}

// 80051A04
void dAcNpcNml_c::talk_c::resLostKeyNG() {
    setMsgProc(&talk_c::msgLostKeyNG);
    startMsg();
    reqMsgClose();
}

// 80051A60
int dAcNpcNml_c::talk_c::msgLostKeyNG(msgInfo_s *info) {
    static const char l_Q12_KeyNG[] = "Q12_KeyNG";
    setLooksMsg(info, l_Q12_KeyNG, 0);
    setStepProc(&talk_c::stepLostKeyNG);
    return TRUE;
}

// 80051AC4
BOOL dAcNpcNml_c::talk_c::stepLostKeyNG(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        requestHandActE();
        return TRUE;
    }
    return FALSE;
}

// 80051B08
int dAcNpcNml_c::talk_c::msgLostKeyCancel(msgInfo_s *info) {
    static const char l_Q12_Cancel[] = "Q12_Cancel";
    setLooksMsg(info, l_Q12_Cancel, 0);
    return TRUE;
}

// 80051B38
BOOL dAcNpcNml_c::talk_c::stepLostKeyReport(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dAnimal_c *animal = getAnimal();
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (animal != NULL) {
            dQuestVillager_c *quest = &animal->mQuest.mQuest;
            if (player != NULL) {
                quest->removePlayer(&player->mPID.player);
            }
            if (quest->countPlayers(FALSE) == 0) {
                dSaveTown_c *town = dSaveData_c::getTown();
                town->mAnimals.mTown.mLostItem.setTime(*dTime_c::getCurrent());
                quest->clear();
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 80051BF0
BOOL dAcNpcNml_c::talk_c::stepLostKeyVisit(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dAnimal_c *animal = getAnimal();
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (animal != NULL) {
            dQuestVillager_c *quest = &animal->mQuest.mQuest;
            if (player != NULL) {
                quest->addPlayer(&player->mPID.player, TRUE);
            }
        }
        return TRUE;
    }
    return FALSE;
}
