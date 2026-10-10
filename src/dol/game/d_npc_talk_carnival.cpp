// The villager talk at the Festivale (EVENT_FESTIVALE, "Ev_Carnival", "Ev_Carnival1".."Ev_Carnival5").
// .text 8003F354..80043C34. See include/game/game/d_npc_talk_carnival.hpp.
#include <game/game/d_npc_talk_carnival.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <cstring>

// The prizes and stakes are the four candies (ITEM_IDX_BLUE_CANDY..ITEM_IDX_GREEN_CANDY, BITM kind
// dItem::KIND_CANDY).

static const char l_Ev_Carnival[] = "Ev_Carnival";   // 8046C630
static const char l_Ev_Carnival1[] = "Ev_Carnival1"; // 8046C63C
static const char l_Ev_Carnival2[] = "Ev_Carnival2"; // 8046C64C
static const char l_Ev_Carnival3[] = "Ev_Carnival3"; // 8046C65C
static const char l_Ev_Carnival4[] = "Ev_Carnival4"; // 8046C66C
static const char l_Ev_Carnival5[] = "Ev_Carnival5"; // 8046C67C

// 804A2308: label per start state (carnivalState_e; getMsgLabel kind TALK_CARNIVAL)
const char *l_carnivalLabels[CARNIVAL_STATE_NUM] = {
    l_Ev_Carnival,  l_Ev_Carnival,  l_Ev_Carnival,  l_Ev_Carnival,  l_Ev_Carnival1,
    l_Ev_Carnival1, l_Ev_Carnival1, l_Ev_Carnival1, l_Ev_Carnival2, l_Ev_Carnival2,
    l_Ev_Carnival2, l_Ev_Carnival2, l_Ev_Carnival3, l_Ev_Carnival3, l_Ev_Carnival3,
    l_Ev_Carnival3, l_Ev_Carnival4, l_Ev_Carnival4, l_Ev_Carnival4, l_Ev_Carnival4,
    l_Ev_Carnival5, l_Ev_Carnival5, l_Ev_Carnival5, l_Ev_Carnival5, l_Ev_Carnival,
};

// 8003F354
int dAcNpcNml_c::talk_c::msgCarnival(msgInfo_s *info) {
    // first state of each game
    static const int l_gameStates[5] = {CARNIVAL_GAME1, CARNIVAL_GAME2, CARNIVAL_GAME3, CARNIVAL_GAME4,
                                        CARNIVAL_GAME5};

    BOOL active = FALSE;
    if (isCurrentSceneAttr(SCENE_ATTR_TOWN) && dEvent::isOngoing(EVENT_FESTIVALE)) {
        active = TRUE;
    }
    dAnimal_c *animal = getAnimal();
    if (active) {
        if ((dQuestEvent_e)dSaveData_c::getTown()->mAnimals.mTown.mEventId != EVENT_FESTIVALE || animal == NULL ||
            !animal->mEvent.isInEvent()) {
            active = FALSE;
        }
    }

    u32 state = CARNIVAL_STATE_NUM;
    if (active) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        dNpcEntry_c *entry = getEntry();
        if (mpMemory == NULL || !mpMemory->isEventFlag(0)) {
            // first talk at the event (endCarnival sets the flag)
            mEquip.setFromPlayer();
            if (player == NULL || !isInCostume(&mEquip)) {
                state = CARNIVAL_NO_COSTUME;
            } else if (player == NULL || player->findEmptyPocket(0) == -1) {
                state = CARNIVAL_GIFT_FULL;
            } else {
                state = CARNIVAL_GIFT;
            }
        } else if (entry == NULL || entry->_28C.mEventTalked) { // set by setStakeMsg
            state = CARNIVAL_PLAYED;
        } else if (player == NULL || player->findEmptyPocket(0) == -1) {
            state = CARNIVAL_FULL;
        } else {
            state = l_gameStates[cM::rndInt(5)];
        }
    }

    stepFunc step = NULL;
    u16 code = 0;
    switch (state) {
    case CARNIVAL_NO_COSTUME:
        code = 1;
        break;
    case CARNIVAL_GIFT:
        step = &dAcNpcNml_c::talk_c::stepCarnivalGift;
        code = 4;
        break;
    case CARNIVAL_GIFT_FULL:
        code = 4;
        break;
    case CARNIVAL_FULL:
        code = 14;
        break;
    case CARNIVAL_GAME1:
    case CARNIVAL_GAME1 + 1:
    case CARNIVAL_GAME1 + 2:
    case CARNIVAL_GAME1 + 3:
        step = &dAcNpcNml_c::talk_c::stepCarnival1Start;
        code = 1;
        break;
    case CARNIVAL_GAME2:
    case CARNIVAL_GAME2 + 1:
    case CARNIVAL_GAME2 + 2:
    case CARNIVAL_GAME2 + 3:
        step = &dAcNpcNml_c::talk_c::stepCarnival2Start;
        code = 1;
        break;
    case CARNIVAL_GAME3:
    case CARNIVAL_GAME3 + 1:
    case CARNIVAL_GAME3 + 2:
    case CARNIVAL_GAME3 + 3:
        step = &dAcNpcNml_c::talk_c::stepCarnival3Start;
        code = 1;
        break;
    case CARNIVAL_GAME4:
    case CARNIVAL_GAME4 + 1:
    case CARNIVAL_GAME4 + 2:
    case CARNIVAL_GAME4 + 3:
        step = &dAcNpcNml_c::talk_c::stepCarnival4Start;
        code = 1;
        break;
    case CARNIVAL_GAME5:
    case CARNIVAL_GAME5 + 1:
    case CARNIVAL_GAME5 + 2:
    case CARNIVAL_GAME5 + 3:
        step = &dAcNpcNml_c::talk_c::stepCarnival5Start;
        code = 1;
        break;
    case CARNIVAL_PLAYED:
        code = 15;
        break;
    }

    if (state < CARNIVAL_STATE_NUM) {
        const char *label = getMsgLabel(TALK_CARNIVAL, state, 0);
        if (label != NULL) {
            setLooksMsg(info, label, code);
            setProcSet(&l_talkEntrySets[TALK_CARNIVAL]);
            if (step != NULL) {
                setStepProc(step);
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 8003F6A0
void dAcNpcNml_c::talk_c::endCarnival(int arg) {
    recordTalk(NULL);
    dNpc::msgMemorySecond_c *mem = getRememberedMsg();
    if (mem != NULL) {
        mem->clear();
    }
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->mMsg.set(mMessageLabel, mMessageCode, mTopicKind, mTopicGroup, mTopicIdx);
    }
    if (mpMemory != NULL) {
        mpMemory->mTalkCount.inc(0x44);
        mpMemory->onEventFlag(0);
        dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
        if (npc != NULL) {
            fn_800F04E8(npc->getNpcIdx(), mMemoryIdx, 0);
        }
    }
}

// 8003F758
BOOL dAcNpcNml_c::talk_c::stepCarnivalGift(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        mItem0.setFromIndex(dItem::ITEM_IDX_BLUE_CANDY, cM::rndInt(4), FALSE);
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player != NULL && player->findEmptyPocket(0) != -1) {
            player->pickUp(&mItem0, FALSE);
        }
        requestItemAct(&mItem0, 0, 0);
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnivalGift);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 8003F83C
int dAcNpcNml_c::talk_c::msgCarnivalGift(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Carnival, 13);
    return TRUE;
}

// 8003F86C
BOOL dAcNpcNml_c::talk_c::startCarnivalMsg(msgFunc next) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(next);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 8003F8E0
int dAcNpcNml_c::talk_c::countStakeItems(u16 *slotMask, dPrivateData_c *player) {
    int count = 0;
    u16 dummy = 0;
    if (slotMask == NULL) {
        slotMask = &dummy;
    }
    *slotMask = 0;
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player != NULL) {
        dItem::Item *pockets = player->mPockets;
        for (int i = 0; i < 15; i++) {
            if (pockets[i].isValid() && !player->getPocketFlag(i) && pockets[i].getPrice() > 0) {
                const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(pockets[i]);
                if (bitm != NULL && !bitm->m_noPurchase) {
                    *slotMask |= 1 << i;
                    count++;
                }
            }
        }
    }
    return count;
}

// 8046C6A0: the candy colours
static const int l_candyIdx[4] = {dItem::ITEM_IDX_BLUE_CANDY, dItem::ITEM_IDX_RED_CANDY, dItem::ITEM_IDX_YELLOW_CANDY,
                                  dItem::ITEM_IDX_GREEN_CANDY};
// 8003F9D4
BOOL dAcNpcNml_c::talk_c::setStakeMsg(msgInfo_s *info, const char *label, stepFunc next) {
    // code 3 candy, 4 500 bells, 5 another item, 6 nothing to stake
    u16 code = 6;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    stepFunc step = NULL;
    mItem0 = dItem::ITEM_ID_NONE;
    mPrice = 0;
    if (player != NULL) {
        u16 mask = 0;
        int num = countPocketsKind(&mask, dItem::KIND_CANDY, player, FALSE);
        if (num != 0 && mask != 0) {
            // the player has candies: the stake is one of the colour the player has most of (ties broken at
            // random), never of the town's colour of the day (town byte 0x735AE)
            u32 today = dSaveData_c::getRaw()->_0735AE;
            if (today < 4) {
                dItem::Item todayCandy(l_candyIdx[today]);
                u8 counts[4];
                memset(counts, 0, sizeof(counts));
                for (int i = 0; i < 15; i++) {
                    dItem::Item *pocket = &player->mPockets[i];
                    if ((mask >> i) & 1 && pocket != NULL && pocket->isNotSame(todayCandy)) {
                        u32 color = 4;
                        for (int j = 0; j < 4; j++) {
                            if (pocket->isSame(dItem::Item(l_candyIdx[j]))) {
                                color = j;
                                break;
                            }
                        }
                        if (color < 4) {
                            counts[color]++;
                        }
                    }
                }
                u32 best = 4;
                u32 max = 0;
                u32 ties = 0;
                for (int j = 0; j < 4; j++) {
                    u8 count = counts[j];
                    if (count != 0) {
                        if (count > max) {
                            best = j;
                            max = count;
                            ties = 1;
                        } else if (count == max) {
                            ties++;
                            f32 limit = 100.0f / ties;
                            if (cM::rndF(100.0f) < limit) {
                                best = j;
                            }
                        }
                    }
                }
                if (best < 4) {
                    mItem0.setFromIndex(l_candyIdx[best]);
                }
            }
            if (!mItem0.isValid()) {
                mItem0 = player->mPockets[pickRandomBit(mask, num, 15)];
            }
            if (mItem0.isValid()) {
                setItemName(&mItem0, 1);
            }
            code = 3;
            step = next;
        } else if (player->getPocketMoney() >= 500) {
            mPrice = 500;
            if (mPrice > 0) {
                setNumber(mPrice, 0, 12, dScript::NUM_FORMAT_REGION);
            }
            code = 4;
            step = next;
        } else {
            mask = 0;
            num = countStakeItems(&mask, player);
            if (num != 0 && mask != 0) {
                mItem0 = player->mPockets[pickRandomBit(mask, num, 15)];
                if (mItem0.isValid()) {
                    setItemName(&mItem0, 0);
                }
                code = 5;
                step = next;
            }
        }
    }
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->_28C.mEventTalked = TRUE;
    }
    setLooksMsg(info, label, code);
    if (step != NULL) {
        setStepProc(step);
    }
    return TRUE;
}

// 8003FDD8
BOOL dAcNpcNml_c::talk_c::setStakeChoice(hookFunc yes) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoiceProc(0, yes);
        setChoiceNum(2);
        setChoiceCancel(1);
        return TRUE;
    }
    return FALSE;
}

// ---- Game 1 ----

// 8003FE74
BOOL dAcNpcNml_c::talk_c::stepCarnival1Start(int kind) {
    return startCarnivalMsg(&dAcNpcNml_c::talk_c::msgCarnival1Stake);
}

// 8003FEB4
int dAcNpcNml_c::talk_c::msgCarnival1Stake(msgInfo_s *info) {
    return setStakeMsg(info, l_Ev_Carnival1, &dAcNpcNml_c::talk_c::stepCarnival1Stake);
}

// 8003FEFC
BOOL dAcNpcNml_c::talk_c::stepCarnival1Stake(int kind) {
    return setStakeChoice(&dAcNpcNml_c::talk_c::selCarnival1Accept);
}

// 8003FF3C
void dAcNpcNml_c::talk_c::selCarnival1Accept() {
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival1Accept);
}

// 8003FF7C
BOOL dAcNpcNml_c::talk_c::stepCarnival1Accept(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival1Play);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 8003FFF4
int dAcNpcNml_c::talk_c::msgCarnival1Play(msgInfo_s *info) {
    mGameCount[0] = 0; // won a round
    setLooksMsg(info, l_Ev_Carnival1, 11);
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival1Choice);
    return TRUE;
}

// 80040060
BOOL dAcNpcNml_c::talk_c::stepCarnival1Choice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoiceProc(0, &dAcNpcNml_c::talk_c::selCarnival1Answer);
        setChoiceProc(1, &dAcNpcNml_c::talk_c::selCarnival1Answer);
        setChoiceNum(2);
        setChoiceCancel(-1);
        return TRUE;
    }
    return FALSE;
}

// 80040120
void dAcNpcNml_c::talk_c::selCarnival1Answer() {
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival1Answer);
}

// 80040160
BOOL dAcNpcNml_c::talk_c::stepCarnival1Answer(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival1Result);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 800401D8
int dAcNpcNml_c::talk_c::msgCarnival1Result(msgInfo_s *info) {
    // 70%, 85% for the second round; two wins in a row win the prize
    f32 chance = mGameCount[0] != 0 ? 85.0f : 70.0f;
    BOOL win = cM::rndF(100.0f) < chance;
    endFunc hook = &dAcNpcNml_c::talk_c::endCarnivalWin;
    stepFunc step = &dAcNpcNml_c::talk_c::stepCarnival1Choice;
    int code = 14;
    if (win) {
        if (mGameCount[0] != 0) {
            code = 15;
        } else {
            mGameCount[0] = 1;
        }
    } else {
        code = mGameCount[0] == 0 ? 16 : 17;
        hook = &dAcNpcNml_c::talk_c::endCarnivalLose;
    }
    switch ((u16)code) {
    case 15:
        mItem0.setFromIndex(dItem::ITEM_IDX_BLUE_CANDY, cM::rndInt(4), FALSE);
        if (mItem0.isValid()) {
            setItemName(&mItem0, 2);
        }
        step = &dAcNpcNml_c::talk_c::stepCarnival1Prize;
        break;
    case 16:
    case 17:
        step = &dAcNpcNml_c::talk_c::stepCarnival1Lost;
        break;
    }
    setLooksMsg(info, l_Ev_Carnival1, code);
    setHookProc(hook);
    setStepProc(step);
    return TRUE;
}

// 800403E4
void dAcNpcNml_c::talk_c::endCarnivalWin(int arg) {
    if (mpNpc != NULL) {
        mpNpc->mAudioObj.startSound(0x17A1); // the player won
    }
}

// 80040408
void dAcNpcNml_c::talk_c::endCarnivalLose(int arg) {
    if (mpNpc != NULL) {
        mpNpc->mAudioObj.startSound(0x17A2); // the player lost
    }
}

// 8004042C
BOOL dAcNpcNml_c::talk_c::stepCarnival1Prize(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0 && mItem0.isValid()) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player != NULL) {
            player->pickUp(&mItem0, FALSE);
        }
        requestItemAct(&mItem0, 0, 0);
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival1Prize);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 800404E0
int dAcNpcNml_c::talk_c::msgCarnival1Prize(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Carnival1, 22);
    return TRUE;
}

// 80040510
BOOL dAcNpcNml_c::talk_c::stepCarnival1Lost(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival1Pay);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80040588
int dAcNpcNml_c::talk_c::msgCarnival1Pay(msgInfo_s *info) {
    u16 code = 19;
    if (mPrice > 0) {
        code = 20;
        mItem0.setFromIndex(dItem::ITEM_IDX_100_BELLS);
    } else if (mItem0.isValid()) {
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(mItem0);
        if (bitm != NULL && bitm->getKind() != dItem::KIND_CANDY) {
            code = 21;
        }
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        if (mPrice > 0) {
            player->payMoney(500, FALSE);
        } else if (mItem0.isValid()) {
            dItem::Item *pockets = player->mPockets;
            for (int i = 0; i < 15; i++) {
                if (!player->getPocketFlag(i) && pockets[i].isSame(mItem0)) {
                    player->clearPocket(i);
                    break;
                }
            }
        }
    }
    setLooksMsg(info, l_Ev_Carnival1, code);
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival1Pay);
    return TRUE;
}

// 80040708
BOOL dAcNpcNml_c::talk_c::stepCarnival1Pay(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        if (mItem0.isValid()) {
            requestItemActEx(6, &mItem0, 0, 0, 0, 2);
        }
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival1Paid);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 800407AC
int dAcNpcNml_c::talk_c::msgCarnival1Paid(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Carnival1, 23);
    return TRUE;
}

// ---- Game 2 (rock-paper-scissors) ----

// 800407DC
BOOL dAcNpcNml_c::talk_c::stepCarnival2Start(int kind) {
    return startCarnivalMsg(&dAcNpcNml_c::talk_c::msgCarnival2Stake);
}

// 8004081C
int dAcNpcNml_c::talk_c::msgCarnival2Stake(msgInfo_s *info) {
    return setStakeMsg(info, l_Ev_Carnival2, &dAcNpcNml_c::talk_c::stepCarnival2Stake);
}

// 80040864
BOOL dAcNpcNml_c::talk_c::stepCarnival2Stake(int kind) {
    return setStakeChoice(&dAcNpcNml_c::talk_c::selCarnival2Accept);
}

// 800408A4
void dAcNpcNml_c::talk_c::selCarnival2Accept() {
    mGameCount[0] = 0; // losses
    mGameCount[1] = 0; // wins
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival2Choice);
}

// 800408F0
BOOL dAcNpcNml_c::talk_c::stepCarnival2Next(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival2Next);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80040968
int dAcNpcNml_c::talk_c::msgCarnival2Next(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Carnival2, 11);
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival2Choice);
    return TRUE;
}

// 800409CC
BOOL dAcNpcNml_c::talk_c::stepCarnival2Choice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoiceProc(0, &dAcNpcNml_c::talk_c::selCarnival2Hand);
        setChoiceProc(1, &dAcNpcNml_c::talk_c::selCarnival2Hand);
        setChoiceProc(2, &dAcNpcNml_c::talk_c::selCarnival2Hand);
        setChoiceNum(3);
        setChoiceCancel(-1);
        return TRUE;
    }
    return FALSE;
}

// 80040AC8
void dAcNpcNml_c::talk_c::selCarnival2Hand() {
    setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival2Hand);
    startMsg();
}

// 80040B1C
int dAcNpcNml_c::talk_c::msgCarnival2Hand(msgInfo_s *info) {
    u16 code;
    stepFunc step;
    // the npc's hand: 0 the player loses, 1 the player wins, 2 draw (code by the player's hand mAnswer)
    switch (cM::rndInt(3)) {
    case 0:
        switch (mAnswer) {
        case 0:
            code = 14;
            break;
        case 1:
            code = 12;
            break;
        default:
            code = 13;
            break;
        }
        mGameCount[0]++;
        step = &dAcNpcNml_c::talk_c::stepCarnival2Lose;
        break;
    case 1:
        switch (mAnswer) {
        case 0:
            code = 13;
            break;
        case 1:
            code = 14;
            break;
        default:
            code = 12;
            break;
        }
        mGameCount[1]++;
        step = &dAcNpcNml_c::talk_c::stepCarnival2Win;
        break;
    default:
        switch (mAnswer) {
        case 0:
            code = 12;
            break;
        case 1:
            code = 13;
            break;
        default:
            code = 14;
            break;
        }
        step = &dAcNpcNml_c::talk_c::stepCarnival2Draw;
        break;
    }
    setLooksMsg(info, l_Ev_Carnival2, code);
    setStepProc(step);
    return TRUE;
}

// 80040CC0
BOOL dAcNpcNml_c::talk_c::stepCarnival2Lose(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival2Lose);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80040D38
int dAcNpcNml_c::talk_c::msgCarnival2Lose(msgInfo_s *info) {
    u16 code = 20;
    switch (mAnswer) {
    case 1:
        code = 21;
        break;
    case 2:
        code = 22;
        break;
    }
    setLooksMsg(info, l_Ev_Carnival2, code);
    setHookProc(&dAcNpcNml_c::talk_c::endCarnivalLose);
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival2Score);
    return TRUE;
}

// 80040DF0
BOOL dAcNpcNml_c::talk_c::stepCarnival2Win(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival2Win);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80040E68
int dAcNpcNml_c::talk_c::msgCarnival2Win(msgInfo_s *info) {
    u16 code = 17;
    switch (mAnswer) {
    case 1:
        code = 18;
        break;
    case 2:
        code = 19;
        break;
    }
    setLooksMsg(info, l_Ev_Carnival2, code);
    setHookProc(&dAcNpcNml_c::talk_c::endCarnivalWin);
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival2Score);
    return TRUE;
}

// 80040F20
BOOL dAcNpcNml_c::talk_c::stepCarnival2Draw(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival2Draw);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80040F98
int dAcNpcNml_c::talk_c::msgCarnival2Draw(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Carnival2, 15);
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival2Choice);
    return TRUE;
}

// 80040FFC
BOOL dAcNpcNml_c::talk_c::stepCarnival2Score(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival2Score);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80041074
int dAcNpcNml_c::talk_c::msgCarnival2Score(msgInfo_s *info) {
    // two wins: prize; two losses: payment; else the next round
    stepFunc step = &dAcNpcNml_c::talk_c::stepCarnival2Next;
    u16 code = 23;
    switch (mGameCount[0]) {
    case 0:
        if (mGameCount[1] == 2) {
            step = &dAcNpcNml_c::talk_c::stepCarnival2Won;
            code = 26;
        }
        break;
    case 1:
        switch (mGameCount[1]) {
        case 0:
            code = 25;
            break;
        case 1:
            code = 24;
            break;
        default:
            step = &dAcNpcNml_c::talk_c::stepCarnival2Won;
            code = 26;
            break;
        }
        break;
    default:
        step = &dAcNpcNml_c::talk_c::stepCarnival2Lost;
        code = 27;
        break;
    }
    setLooksMsg(info, l_Ev_Carnival2, code);
    setStepProc(step);
    return TRUE;
}

// 800411B8
BOOL dAcNpcNml_c::talk_c::stepCarnival2Won(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival2Won);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80041230
int dAcNpcNml_c::talk_c::msgCarnival2Won(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Carnival2, 28);
    mItem0.setFromIndex(dItem::ITEM_IDX_BLUE_CANDY, cM::rndInt(4), FALSE);
    if (mItem0.isValid()) {
        setItemName(&mItem0, 2);
    }
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival2Prize);
    return TRUE;
}

// 800412CC
BOOL dAcNpcNml_c::talk_c::stepCarnival2Prize(int kind) {
    if (mItem0.isValid()) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player != NULL) {
            player->pickUp(&mItem0, FALSE);
        }
        requestItemAct(&mItem0, 0, 0);
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival2Prize);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80041368
int dAcNpcNml_c::talk_c::msgCarnival2Prize(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Carnival2, 32);
    return TRUE;
}

// 80041398
BOOL dAcNpcNml_c::talk_c::stepCarnival2Lost(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival2Pay);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80041410
int dAcNpcNml_c::talk_c::msgCarnival2Pay(msgInfo_s *info) {
    u16 code = 29;
    if (mPrice > 0) {
        code = 30;
        mItem0.setFromIndex(dItem::ITEM_IDX_100_BELLS);
    } else if (mItem0.isValid()) {
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(mItem0);
        if (bitm != NULL && bitm->getKind() != dItem::KIND_CANDY) {
            code = 31;
        }
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        if (mPrice > 0) {
            player->payMoney(500, FALSE);
        } else if (mItem0.isValid()) {
            dItem::Item *pockets = player->mPockets;
            for (int i = 0; i < 15; i++) {
                if (!player->getPocketFlag(i) && pockets[i].isSame(mItem0)) {
                    player->clearPocket(i);
                    break;
                }
            }
        }
    }
    setLooksMsg(info, l_Ev_Carnival2, code);
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival2Pay);
    return TRUE;
}

// 80041590
BOOL dAcNpcNml_c::talk_c::stepCarnival2Pay(int kind) {
    if (mItem0.isValid()) {
        requestItemActEx(6, &mItem0, 0, 0, 0, 2);
    }
    setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival2Paid);
    startMsg();
    return TRUE;
}

// 80041614
int dAcNpcNml_c::talk_c::msgCarnival2Paid(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Carnival2, 33);
    return TRUE;
}

// ---- Game 3 ----

// 80041644
BOOL dAcNpcNml_c::talk_c::stepCarnival3Start(int kind) {
    return startCarnivalMsg(&dAcNpcNml_c::talk_c::msgCarnival3Stake);
}

// 80041684
int dAcNpcNml_c::talk_c::msgCarnival3Stake(msgInfo_s *info) {
    return setStakeMsg(info, l_Ev_Carnival3, &dAcNpcNml_c::talk_c::stepCarnival3Stake);
}

// 800416CC
BOOL dAcNpcNml_c::talk_c::stepCarnival3Stake(int kind) {
    return setStakeChoice(&dAcNpcNml_c::talk_c::selCarnival3Accept);
}

// 8004170C
void dAcNpcNml_c::talk_c::selCarnival3Accept() {
    mGameCount[0] = 0; // losses
    mGameCount[1] = 0; // wins
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival3Choice);
}

// 80041758
BOOL dAcNpcNml_c::talk_c::stepCarnival3Choice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoiceProc(0, &dAcNpcNml_c::talk_c::selCarnival3Answer);
        setChoiceProc(1, &dAcNpcNml_c::talk_c::selCarnival3Answer);
        setChoiceNum(2);
        setChoiceCancel(-1);
        return TRUE;
    }
    return FALSE;
}

// 80041818
void dAcNpcNml_c::talk_c::selCarnival3Answer() {
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival3Answer);
}

// 80041858
BOOL dAcNpcNml_c::talk_c::stepCarnival3Answer(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival3Round1);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 800418D0
int dAcNpcNml_c::talk_c::msgCarnival3Round1(msgInfo_s *info) {
    BOOL lose = cM::rndF(100.0f) < 20.0f; // the player loses this round
    endFunc hook = &dAcNpcNml_c::talk_c::endCarnivalLose;
    int code = 17;
    if (lose) {
        if (mGameCount[3] != 0) { // lost a round before: second loss variant
            code = 18;
        }
        mGameCount[3] = 1;
        mGameCount[0]++; // losses
    } else {
        code = 15 + (mGameCount[2] != 0); // second win variant
        hook = &dAcNpcNml_c::talk_c::endCarnivalWin;
        mGameCount[2] = 1;
        mGameCount[1]++; // wins
    }
    setLooksMsg(info, l_Ev_Carnival3, code);
    setHookProc(hook);
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival3Choice2);
    return TRUE;
}

// 80041A24
BOOL dAcNpcNml_c::talk_c::stepCarnival3Choice2(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoiceProc(0, &dAcNpcNml_c::talk_c::selCarnival3Answer2);
        setChoiceProc(1, &dAcNpcNml_c::talk_c::selCarnival3Answer2);
        setChoiceNum(2);
        setChoiceCancel(-1);
        return TRUE;
    }
    return FALSE;
}

// 80041AE4
void dAcNpcNml_c::talk_c::selCarnival3Answer2() {
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival3Answer2);
}

// 80041B24
BOOL dAcNpcNml_c::talk_c::stepCarnival3Answer2(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival3Round2);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80041B9C
int dAcNpcNml_c::talk_c::msgCarnival3Round2(msgInfo_s *info) {
    BOOL lose = cM::rndF(100.0f) < 20.0f; // the player loses this round
    endFunc hook = &dAcNpcNml_c::talk_c::endCarnivalLose;
    int code = 25;
    if (lose) {
        if (mGameShown[1] != 0) { // lost a round before: second loss variant
            code = 26;
        }
        mGameShown[1] = 1;
        mGameCount[0]++; // losses
    } else {
        code = 23 + (mGameShown[0] != 0); // second win variant
        hook = &dAcNpcNml_c::talk_c::endCarnivalWin;
        mGameShown[0] = 1;
        mGameCount[1]++; // wins
    }
    setLooksMsg(info, l_Ev_Carnival3, code);
    setHookProc(hook);
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival3Score);
    return TRUE;
}

// 80041CF0
BOOL dAcNpcNml_c::talk_c::stepCarnival3Score(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival3Score);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80041D68
int dAcNpcNml_c::talk_c::msgCarnival3Score(msgInfo_s *info) {
    stepFunc step = &dAcNpcNml_c::talk_c::stepCarnival3Draw;
    u16 code = 27;
    if (mGameCount[0] > mGameCount[1]) {
        step = &dAcNpcNml_c::talk_c::stepCarnival3Lost;
        code = 30;
    } else if (mGameCount[0] < mGameCount[1]) {
        step = &dAcNpcNml_c::talk_c::stepCarnival3Won;
        code = 29;
    }
    setLooksMsg(info, l_Ev_Carnival3, code);
    setStepProc(step);
    return TRUE;
}

// 80041E48
BOOL dAcNpcNml_c::talk_c::stepCarnival3Draw(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival3Draw);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80041EC0
int dAcNpcNml_c::talk_c::msgCarnival3Draw(msgInfo_s *info) {
    // one round each: replay both rounds
    mGameCount[0] = 1;
    mGameCount[1] = 1;
    setLooksMsg(info, l_Ev_Carnival3, 12);
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival3Choice);
    return TRUE;
}

// 80041F30
BOOL dAcNpcNml_c::talk_c::stepCarnival3Won(int kind) {
    mItem0.setFromIndex(dItem::ITEM_IDX_BLUE_CANDY, cM::rndInt(4), FALSE);
    if (mItem0.isValid()) {
        setItemName(&mItem0, 2);
    }
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival3Prize);
    return TRUE;
}

// 80041FBC
BOOL dAcNpcNml_c::talk_c::stepCarnival3Prize(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player != NULL) {
            player->pickUp(&mItem0, FALSE);
        }
        requestItemAct(&mItem0, 0, 0);
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival3Prize);
        startMsg();
        return TRUE;
    }
    return TRUE;
}

// 80042064
int dAcNpcNml_c::talk_c::msgCarnival3Prize(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Carnival3, 35);
    return TRUE;
}

// 80042094
BOOL dAcNpcNml_c::talk_c::stepCarnival3Lost(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival3Pay);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 8004210C
int dAcNpcNml_c::talk_c::msgCarnival3Pay(msgInfo_s *info) {
    u16 code = 32;
    if (mPrice > 0) {
        code = 33;
        mItem0.setFromIndex(dItem::ITEM_IDX_100_BELLS);
    } else if (mItem0.isValid()) {
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(mItem0);
        if (bitm != NULL && bitm->getKind() != dItem::KIND_CANDY) {
            code = 34;
        }
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        if (mPrice > 0) {
            player->payMoney(500, FALSE);
        } else if (mItem0.isValid()) {
            dItem::Item *pockets = player->mPockets;
            for (int i = 0; i < 15; i++) {
                if (!player->getPocketFlag(i) && pockets[i].isSame(mItem0)) {
                    player->clearPocket(i);
                    break;
                }
            }
        }
    }
    setLooksMsg(info, l_Ev_Carnival3, code);
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival3Pay);
    return TRUE;
}

// 8004228C
BOOL dAcNpcNml_c::talk_c::stepCarnival3Pay(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        if (mItem0.isValid()) {
            requestItemActEx(6, &mItem0, 0, 0, 0, 2);
        }
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival3Paid);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80042330
int dAcNpcNml_c::talk_c::msgCarnival3Paid(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Carnival3, 36);
    return TRUE;
}

// ---- Game 4 ----

// 80042360
BOOL dAcNpcNml_c::talk_c::stepCarnival4Start(int kind) {
    return startCarnivalMsg(&dAcNpcNml_c::talk_c::msgCarnival4Stake);
}

// 800423A0
int dAcNpcNml_c::talk_c::msgCarnival4Stake(msgInfo_s *info) {
    return setStakeMsg(info, l_Ev_Carnival4, &dAcNpcNml_c::talk_c::stepCarnival4Stake);
}

// 800423E8
BOOL dAcNpcNml_c::talk_c::stepCarnival4Stake(int kind) {
    return setStakeChoice(&dAcNpcNml_c::talk_c::selCarnival4Accept);
}

// 80042428
void dAcNpcNml_c::talk_c::selCarnival4Accept() {
    mGameCount[0] = 0; // round
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival4Choice);
}

// 80042470
BOOL dAcNpcNml_c::talk_c::stepCarnival4Choice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoiceProc(0, &dAcNpcNml_c::talk_c::selCarnival4Answer);
        setChoiceProc(1, &dAcNpcNml_c::talk_c::selCarnival4Answer);
        setChoiceNum(2);
        setChoiceCancel(-1);
        return TRUE;
    }
    return FALSE;
}

// 80042530
void dAcNpcNml_c::talk_c::selCarnival4Answer() {
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival4Answer);
}

// 80042570
BOOL dAcNpcNml_c::talk_c::stepCarnival4Answer(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival4Result);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 800425E8
int dAcNpcNml_c::talk_c::msgCarnival4Result(msgInfo_s *info) {
    // win chance of each round
    static const f32 l_roundChance[4] = {80.0f, 85.0f, 90.0f, 0.0f};

    f32 chance = l_roundChance[mGameCount[0] < 3 ? mGameCount[0] : 0];
    BOOL win = cM::rndF(100.0f) < chance;
    endFunc hook = &dAcNpcNml_c::talk_c::endCarnivalWin;
    stepFunc step = &dAcNpcNml_c::talk_c::stepCarnival4Next;
    u16 code = 14;
    if (win) {
        if (mGameCount[0] == 1) {
            code = 15;
        } else if (mGameCount[0] == 2) {
            step = &dAcNpcNml_c::talk_c::stepCarnival4Color;
            code = 16;
        }
        mGameCount[0]++;
    } else {
        switch (mGameCount[0]) {
        case 0:
            code = 17;
            break;
        case 1:
            code = 18;
            break;
        default:
            code = 19;
            break;
        }
        hook = &dAcNpcNml_c::talk_c::endCarnivalLose;
        step = &dAcNpcNml_c::talk_c::stepCarnival4Lost;
    }
    setLooksMsg(info, l_Ev_Carnival4, code);
    setHookProc(hook);
    setStepProc(step);
    return TRUE;
}

// 800427C8
BOOL dAcNpcNml_c::talk_c::stepCarnival4Next(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival4Next);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80042840
int dAcNpcNml_c::talk_c::msgCarnival4Next(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Carnival4, 11);
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival4Choice);
    return TRUE;
}

// 800428A4
BOOL dAcNpcNml_c::talk_c::stepCarnival4Color(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoiceProc(0, &dAcNpcNml_c::talk_c::selCarnival4Color);
        setChoiceProc(1, &dAcNpcNml_c::talk_c::selCarnival4Color);
        setChoiceProc(2, &dAcNpcNml_c::talk_c::selCarnival4Color);
        setChoiceProc(3, &dAcNpcNml_c::talk_c::selCarnival4Color);
        setChoiceNum(4);
        setChoiceCancel(-1);
        return TRUE;
    }
    return FALSE;
}

// 800429CC
void dAcNpcNml_c::talk_c::selCarnival4Color() {
    // the candies in answer order
    static dItem::Item l_candies[4] = {dItem::Item(dItem::ITEM_IDX_RED_CANDY), dItem::Item(dItem::ITEM_IDX_BLUE_CANDY),
                                       dItem::Item(dItem::ITEM_IDX_YELLOW_CANDY), dItem::Item(dItem::ITEM_IDX_GREEN_CANDY)};

    mItem0 = l_candies[mAnswer < 4 ? mAnswer : cM::rndInt(4)];
    if (mItem0.isValid()) {
        setItemName(&mItem0, 2);
    }
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival4Prize);
}

// 80042B08
BOOL dAcNpcNml_c::talk_c::stepCarnival4Prize(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        if (mItem0.isValid()) {
            dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
            if (player != NULL) {
                player->pickUp(&mItem0, FALSE);
            }
            requestItemAct(&mItem0, 0, 0);
        }
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival4Prize);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80042BBC
int dAcNpcNml_c::talk_c::msgCarnival4Prize(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Carnival4, 25);
    return TRUE;
}

// 80042BEC
BOOL dAcNpcNml_c::talk_c::stepCarnival4Lost(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival4Pay);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80042C64
int dAcNpcNml_c::talk_c::msgCarnival4Pay(msgInfo_s *info) {
    u16 code = 22;
    if (mPrice > 0) {
        code = 23;
        mItem0.setFromIndex(dItem::ITEM_IDX_100_BELLS);
    } else if (mItem0.isValid()) {
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(mItem0);
        if (bitm != NULL && bitm->getKind() != dItem::KIND_CANDY) {
            code = 24;
        }
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        if (mPrice > 0) {
            player->payMoney(500, FALSE);
        } else if (mItem0.isValid()) {
            dItem::Item *pockets = player->mPockets;
            for (int i = 0; i < 15; i++) {
                if (!player->getPocketFlag(i) && pockets[i].isSame(mItem0)) {
                    player->clearPocket(i);
                    break;
                }
            }
        }
    }
    setLooksMsg(info, l_Ev_Carnival4, code);
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival4Pay);
    return TRUE;
}

// 80042DE4
BOOL dAcNpcNml_c::talk_c::stepCarnival4Pay(int kind) {
    if (mItem0.isValid()) {
        requestItemActEx(6, &mItem0, 0, 0, 0, 2);
    }
    setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival4Paid);
    startMsg();
    return TRUE;
}

// 80042E68
int dAcNpcNml_c::talk_c::msgCarnival4Paid(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Carnival4, 26);
    return TRUE;
}

// ---- Game 5 ----

// 80042E98
BOOL dAcNpcNml_c::talk_c::stepCarnival5Start(int kind) {
    return startCarnivalMsg(&dAcNpcNml_c::talk_c::msgCarnival5Stake);
}

// 80042ED8
int dAcNpcNml_c::talk_c::msgCarnival5Stake(msgInfo_s *info) {
    return setStakeMsg(info, l_Ev_Carnival5, &dAcNpcNml_c::talk_c::stepCarnival5Stake);
}

// 80042F20
BOOL dAcNpcNml_c::talk_c::stepCarnival5Stake(int kind) {
    return setStakeChoice(&dAcNpcNml_c::talk_c::selCarnival5Accept);
}

// 80042F60
void dAcNpcNml_c::talk_c::selCarnival5Accept() {
    mGameCount[0] = 0;
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival5Choice);
}

// 80042FA8
BOOL dAcNpcNml_c::talk_c::stepCarnival5Choice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoiceProc(0, &dAcNpcNml_c::talk_c::selCarnival5Answer);
        setChoiceProc(1, &dAcNpcNml_c::talk_c::selCarnival5Answer);
        setChoiceProc(2, &dAcNpcNml_c::talk_c::selCarnival5Answer);
        setChoiceNum(3);
        setChoiceCancel(-1);
        return TRUE;
    }
    return FALSE;
}

// 800430A4
void dAcNpcNml_c::talk_c::selCarnival5Answer() {
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival5Answer);
}

// 800430E4
BOOL dAcNpcNml_c::talk_c::stepCarnival5Answer(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival5Result);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 8004315C
int dAcNpcNml_c::talk_c::msgCarnival5Result(msgInfo_s *info) {
    f32 chance = 85.0f;
    BOOL win = cM::rndF(100.0f) < chance;
    endFunc hook = &dAcNpcNml_c::talk_c::endCarnivalWin;
    stepFunc step = &dAcNpcNml_c::talk_c::stepCarnival5Choice2;
    u16 code = 15;
    if (!win) {
        u32 pick = cM::rndInt(2);
        if (pick == mAnswer) {
            pick++;
        }
        code = pick + 16;
        hook = &dAcNpcNml_c::talk_c::endCarnivalLose;
        step = &dAcNpcNml_c::talk_c::stepCarnival5Lost;
    }
    setLooksMsg(info, l_Ev_Carnival5, code);
    setHookProc(hook);
    setStepProc(step);
    return TRUE;
}

// 800432C0
BOOL dAcNpcNml_c::talk_c::stepCarnival5Lost(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival5Pay);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80043338
int dAcNpcNml_c::talk_c::msgCarnival5Pay(msgInfo_s *info) {
    u16 code = 33;
    if (mPrice > 0) {
        code = 34;
        mItem0.setFromIndex(dItem::ITEM_IDX_100_BELLS);
    } else if (mItem0.isValid()) {
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(mItem0);
        if (bitm != NULL && bitm->getKind() != dItem::KIND_CANDY) {
            code = 35;
        }
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        if (mPrice > 0) {
            player->payMoney(500, FALSE);
        } else if (mItem0.isValid()) {
            dItem::Item *pockets = player->mPockets;
            for (int i = 0; i < 15; i++) {
                if (!player->getPocketFlag(i) && pockets[i].isSame(mItem0)) {
                    player->clearPocket(i);
                    break;
                }
            }
        }
    }
    setLooksMsg(info, l_Ev_Carnival5, code);
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival5Pay);
    return TRUE;
}

// 800434B8
BOOL dAcNpcNml_c::talk_c::stepCarnival5Pay(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        if (mItem0.isValid()) {
            requestItemActEx(6, &mItem0, 0, 0, 0, 2);
        }
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival5Paid);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 8004355C
int dAcNpcNml_c::talk_c::msgCarnival5Paid(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Carnival5, 37);
    return TRUE;
}

// 8004358C
BOOL dAcNpcNml_c::talk_c::stepCarnival5Choice2(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoiceProc(0, &dAcNpcNml_c::talk_c::selCarnival5Answer2);
        setChoiceProc(1, &dAcNpcNml_c::talk_c::selCarnival5Answer2);
        setChoiceProc(2, &dAcNpcNml_c::talk_c::selCarnival5Answer2);
        setChoiceProc(3, &dAcNpcNml_c::talk_c::selCarnival5Answer2);
        setChoiceProc(4, &dAcNpcNml_c::talk_c::selCarnival5Answer2);
        setChoiceNum(5);
        setChoiceCancel(-1);
        return TRUE;
    }
    return FALSE;
}

// 800436E0
void dAcNpcNml_c::talk_c::selCarnival5Answer2() {
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival5Answer2);
}

// 80043720
BOOL dAcNpcNml_c::talk_c::stepCarnival5Answer2(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival5Result2);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80043798
int dAcNpcNml_c::talk_c::msgCarnival5Result2(msgInfo_s *info) {
    BOOL win = cM::rndF(100.0f) < 70.0f;
    endFunc hook = &dAcNpcNml_c::talk_c::endCarnivalWin;
    stepFunc step = &dAcNpcNml_c::talk_c::stepCarnival5Color;
    u16 code = 25;
    if (!win) {
        u32 pick = cM::rndInt(4);
        if (pick == mAnswer) {
            pick++;
        }
        code = pick + 26;
        hook = &dAcNpcNml_c::talk_c::endCarnivalLose;
        step = &dAcNpcNml_c::talk_c::stepCarnival5Lost;
    }
    setLooksMsg(info, l_Ev_Carnival5, code);
    setHookProc(hook);
    setStepProc(step);
    return TRUE;
}

// 800438EC
BOOL dAcNpcNml_c::talk_c::stepCarnival5Color(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoiceProc(0, &dAcNpcNml_c::talk_c::selCarnival5Color);
        setChoiceProc(1, &dAcNpcNml_c::talk_c::selCarnival5Color);
        setChoiceProc(2, &dAcNpcNml_c::talk_c::selCarnival5Color);
        setChoiceProc(3, &dAcNpcNml_c::talk_c::selCarnival5Color);
        setChoiceNum(4);
        setChoiceCancel(-1);
        return TRUE;
    }
    return FALSE;
}

// 80043A14
void dAcNpcNml_c::talk_c::selCarnival5Color() {
    // the candies in answer order
    static dItem::Item l_candies[4] = {dItem::Item(dItem::ITEM_IDX_RED_CANDY), dItem::Item(dItem::ITEM_IDX_BLUE_CANDY),
                                       dItem::Item(dItem::ITEM_IDX_YELLOW_CANDY), dItem::Item(dItem::ITEM_IDX_GREEN_CANDY)};

    mItem0 = l_candies[mAnswer < 4 ? mAnswer : cM::rndInt(4)];
    if (mItem0.isValid()) {
        setItemName(&mItem0, 2);
    }
    setStepProc(&dAcNpcNml_c::talk_c::stepCarnival5Prize);
}

// 80043B50
BOOL dAcNpcNml_c::talk_c::stepCarnival5Prize(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        if (mItem0.isValid()) {
            dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
            if (player != NULL) {
                player->pickUp(&mItem0, FALSE);
            }
            requestItemAct(&mItem0, 0, 0);
        }
        setMsgProc(&dAcNpcNml_c::talk_c::msgCarnival5Prize);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80043C04
int dAcNpcNml_c::talk_c::msgCarnival5Prize(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Carnival5, 36);
    return TRUE;
}
