// The villager's flea market talk: selling its boxed furniture, buying at the player's stall, and
// walking over to the stall. .text 800476E0..8004902C. See include/game/game/d_npc_talk_fmarket.hpp.
#include <game/game/d_npc_talk_fmarket.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_money.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_data.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

// 8046C7B8.. message labels
static const char l_Ev_FmarketNPC3[] = "Ev_FmarketNPC3";
static const char l_Ev_FmarketNPC4[] = "Ev_FmarketNPC4";
static const char l_Q09_TradeYes[] = "Q09_TradeYes";
static const char l_Q09_TradeNo[] = "Q09_TradeNo";
static const char l_Ev_FmarketPC[] = "Ev_FmarketPC";
static const char l_Q08_Call[] = "Q08_Call";
static const char l_Q08_Wait[] = "Q08_Wait";
static const char l_Q08_Back[] = "Q08_Back";

// 800476E0
int dAcNpcNml_c::talk_c::msgFmarket(msgInfo_s *info) {
    static const char *l_labels[8] = {
        "Ev_FmarketNPC", "Ev_FmarketNPC", "Ev_FmarketNPC", "Ev_FmarketNPC",
        "Ev_FmarketNPC", "Ev_FmarketNPC", "Ev_FmarketNPC1", "Ev_FmarketNPC2",
    };
    dAnimal_c *animal = getAnimal();
    dNpcEntry_c *entry = getEntry();
    int boxed = animal != NULL ? animal->countBoxedFtr() : 0;
    u32 kind;
    const dLandID_c *townLand = &dSaveData_c::getRaw()->mLandID;
    const dLandID_c *land = NULL;
    if (boxed == 0) {
        if (mpMemory == NULL) {
            kind = 1;
        } else {
            const dLandID_c *memLand = &mpMemory->mLand;
            if (!(*memLand == *townLand)) {
                land = memLand;
                kind = 2;
            } else {
                kind = 0;
            }
        }
    } else if (entry == NULL || !entry->_28C.mEventTalked) {
        if (mpMemory == NULL) {
            kind = 4;
        } else {
            const dLandID_c *memLand = &mpMemory->mLand;
            if (!(*memLand == *townLand)) {
                land = memLand;
                kind = 5;
            } else {
                kind = 3;
            }
        }
    } else {
        kind = 7;
        if (!entry->_28C.mFmarketSale) {
            kind = 6;
        }
    }
    u16 code = cM::rndInt(3) + 1;
    stepFunc step = NULL;
    switch (kind) {
    case 0:
        code = cM::rndInt(3) + 4;
        break;
    case 1:
        code = 0xC;
        break;
    case 2:
        code = 0xB;
        break;
    case 4:
        code = 0xE;
        break;
    case 5:
        code = 0xD;
        break;
    case 6:
        step = &talk_c::stepFmarketAsk;
        break;
    case 7:
        code = 0;
        break;
    }
    if (kind < 8) {
        const char *label = l_labels[kind];
        setProcSet(&l_talkProcSets[TALK_PROC_FMARKET]);
        if (step) {
            setStepProc(step);
        }
        if (land != NULL && land->isValid()) {
            setLandName(land, 1);
        }
        setLooksMsg(info, label, code);
        setTopic(dNpc::msgMemory_c::KIND_ANY, 0, 0);
        return TRUE;
    }
    return FALSE;
}

// 800479C4
void dAcNpcNml_c::talk_c::endFmarket(int arg) {
    recordTalk(NULL);
    dNpc::msgMemorySecond_c *mem = getRememberedMsg();
    if (mem != NULL) {
        mem->clear();
    }
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->mMsg.set(mMessageLabel, mMessageCode, mTopicKind, mTopicGroup, mTopicIdx);
        entry->_28C.mEventTalked = 1;
    }
    if (mpMemory != NULL) {
        mpMemory->mTalkCount.inc(0x44);
    }
}

// 80047A6C
BOOL dAcNpcNml_c::talk_c::stepFmarketAsk(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoiceProc(0, &talk_c::selFmarketAsk);
        setChoiceNum(2);
        setChoiceCancel(1);
        return TRUE;
    }
    return FALSE;
}

// 80047B00
void dAcNpcNml_c::talk_c::selFmarketAsk() {
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->_28C.mFmarketSale = 1;
        fn_8019C34C();
    }
}

// 80047B38
int dAcNpcNml_c::talk_c::msgSale(msgInfo_s *info) {
    BOOL canBuy = FALSE;
    u32 idx = static_cast<dAcNpcNml_c *>(mpNpc)->getRoomFtrIdx(mFtrPosX, mFtrPosZ);
    dAnimal_c *animal = getAnimal();
    mPrice = 10;
    if (animal != NULL && animal->isFtrBoxed(idx)) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player != NULL && player->findEmptyPocket(0) != -1) {
            int price = getSellPrice(&mItem0);
            mPrice = price;
            BOOL enough = player->getPocketMoney() >= price;
            if (enough) {
                canBuy = TRUE;
            }
        }
    }
    stepFunc step = NULL;
    const char *label = l_Ev_FmarketNPC4;
    if (canBuy) {
        step = &talk_c::stepSaleChoice;
        label = l_Ev_FmarketNPC3;
    }
    setLooksMsg(info, label, 0);
    if (step) {
        setStepProc(step);
    }
    if (mItem0.mId != dItem::ITEM_ID_NONE) {
        setItemName(&mItem0, 0);
    }
    if (mPrice > 0) {
        setNumber(mPrice, 3, 12, dScript::NUM_FORMAT_REGION);
    }
    return TRUE;
}

// 80047CD4
BOOL dAcNpcNml_c::talk_c::stepSaleChoice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoice(0, 0x42, 3, &talk_c::selSaleYes);
        setChoice(1, 0x45, 3, &talk_c::selSaleNo);
        setChoiceNum(2);
        setChoiceCancel(1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 80047DAC
void dAcNpcNml_c::talk_c::selSaleYes() {
    setMsgProc(&talk_c::msgSaleYes);
    startMsg();
}

// 80047E00
int dAcNpcNml_c::talk_c::msgSaleYes(msgInfo_s *info) {
    setLooksMsg(info, l_Q09_TradeYes, 0);
    setHookProc(&talk_c::endSaleYes);
    setStepProc(&talk_c::stepSaleYes);
    return FALSE;
}

// 80047E8C
void dAcNpcNml_c::talk_c::endSaleYes(int arg) {
    dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
    mFtrHandle = fn_800A8F98(fn_800A9058(), mFtrPosX, mFtrPosZ, 0);
    if (npc != NULL && npc->removeRoomFtr(mFtrHandle, mFtrPosX, mFtrPosZ)) {
        dDemo_c *ctrl = getController();
        if (ctrl != NULL) {
            ctrl->mLock = 1;
        }
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player != NULL) {
            if (mPrice > 0) {
                player->payMoney(mPrice, FALSE);
            }
            if (mItem0.mId != dItem::ITEM_ID_NONE) {
                player->pickUp(&mItem0, FALSE);
            }
        }
        dNpcEntry_c *entry = getEntry();
        if (entry != NULL) {
            entry->_28C.mFmarketSale = 0;
        }
        setActProc(&talk_c::actSaleWait);
    }
}

// 80047FA0
BOOL dAcNpcNml_c::talk_c::stepSaleYes(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        fn_8019C35C();
        return TRUE;
    }
    return FALSE;
}

// 80047FE4
void dAcNpcNml_c::talk_c::actSaleWait() {
    if (!fn_800A9354(fn_800A93BC(), mFtrHandle)) {
        dDemo_c *ctrl = getController();
        if (ctrl != NULL) {
            ctrl->mLock = 0;
        }
        setActProc(NULL);
    }
}

// 8004805C
void dAcNpcNml_c::talk_c::selSaleNo() {
    setMsgProc(&talk_c::msgSaleNo);
    startMsg();
}

// 800480B0
int dAcNpcNml_c::talk_c::msgSaleNo(msgInfo_s *info) {
    setLooksMsg(info, l_Q09_TradeNo, 0);
    return FALSE;
}

// 800480E0
int dAcNpcNml_c::talk_c::msgStallCall(msgInfo_s *info) {
    setLooksMsg(info, l_Q08_Call, 0);
    setHookProc(&talk_c::endStallTalk);
    setStepProc(&talk_c::stepStallCall);
    return TRUE;
}

// 8004816C
void dAcNpcNml_c::talk_c::endStallTalk(int arg) {
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
    }
}

// 800481FC
BOOL dAcNpcNml_c::talk_c::stepStallCall(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgStallComing);
        startMsg();
        setRequest1(0);
        mResultProc = &talk_c::resStallCall;
        return TRUE;
    }
    return FALSE;
}

// 8004829C
void dAcNpcNml_c::talk_c::resStallCall() {
    setActProc(&talk_c::actStallWalk);
}

// 800482DC
void dAcNpcNml_c::talk_c::actStallWalk() {
    dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
    if (npc != NULL) {
        npc->mIsActive = 1;
        if (npc->mAction.requestWalk(1, l_walkTargetPos, l_walkTurnSpeed, 0, l_moveParamWalk, l_8074FDE8,
                                     cNpcMorphFrames)) {
            npc->mPos.y = dBGCF::getGroundY(&npc->mPos, FALSE);
            setActProc(&talk_c::actStallArrive);
        }
    }
}

// 8004838C
void dAcNpcNml_c::talk_c::actStallArrive() {
    dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
    if (npc != NULL) {
        npc->mPos.y = dBGCF::getGroundY(&npc->mPos, FALSE);
        if (npc->mAction.mCanChange) {
            npc->mAction.requestWait(1);
            lbl_8074E9B0->fn_8018B5B8(npc->demoHook58());
            reqMsgClose();
            clearActProc();
            if (mpMemory != NULL) {
                mpMemory->onEventFlag(0);
            }
        }
    }
}

// 80048438
int dAcNpcNml_c::talk_c::msgStallComing(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_FmarketPC, 1);
    return TRUE;
}

// 80048468
int dAcNpcNml_c::talk_c::msgStallWait(msgInfo_s *info) {
    setLooksMsg(info, l_Q08_Wait, 0);
    setHookProc(&talk_c::endStallTalk);
    setStepProc(&talk_c::stepStallWait);
    return TRUE;
}

// 800484F4
BOOL dAcNpcNml_c::talk_c::stepStallWait(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgStallLook);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 8004856C
int dAcNpcNml_c::talk_c::msgStallLook(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_FmarketPC, 2);
    setHookProc(&talk_c::endStallLook);
    return TRUE;
}

// 800485D0
void dAcNpcNml_c::talk_c::endStallLook(int arg) {
    if (mpMemory != NULL) {
        mpMemory->onEventFlag(1);
    }
}

// 800485E8
int dAcNpcNml_c::talk_c::msgStallBack(msgInfo_s *info) {
    setLooksMsg(info, l_Q08_Back, 0);
    setHookProc(&talk_c::endStallTalk);
    setStepProc(&talk_c::stepStallBack);
    return TRUE;
}

// 80048674
BOOL dAcNpcNml_c::talk_c::stepStallBack(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgStallLook);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 800486EC
int dAcNpcNml_c::talk_c::msgStallNoItem(msgInfo_s *info) {
    setProcSet(&l_talkProcSets[TALK_PROC_STALL_NO_ITEM]);
    setLooksMsg(info, l_Ev_FmarketPC, 3);
    return TRUE;
}

// 8004874C
void dAcNpcNml_c::talk_c::endStallBuy(int arg) {
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
    }
}

// 800487DC
int dAcNpcNml_c::talk_c::msgStallItem(msgInfo_s *info) {
    BOOL found = FALSE;
    if (mItem0.mId != dItem::ITEM_ID_NONE) {
        mFtrHandle = fn_800A8F98(fn_800A9058(), mFtrPosX, mFtrPosZ, 0);
        if (mFtrHandle != -1) {
            found = TRUE;
        }
    }
    if (found) {
        if (mItem0.mId != dItem::ITEM_ID_NONE) {
            setItemName(&mItem0, 0);
        }
        setProcSet(&l_talkProcSets[TALK_PROC_STALL_ITEM]);
        setLooksMsg(info, l_Ev_FmarketPC, 4);
        return TRUE;
    }
    return msgStallNoItem(info);
}

// 800488B0
BOOL dAcNpcNml_c::talk_c::stepStallItem(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoice(0, 0x6D, 1, &talk_c::selStallPrice);
        setChoice(1, 0x6E, 1, &talk_c::selStallNo);
        setChoiceNum(2);
        setChoiceCancel(1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 80048988
void dAcNpcNml_c::talk_c::selStallPrice() {
    reqMenu21(1);
    mResultProc = &talk_c::resStallPrice;
}

// 800489D4
void dAcNpcNml_c::talk_c::resStallPrice() {
    BOOL done = FALSE;
    if (!isMenuInvalid()) {
        mPrice = fn_80029B20();
        setNumber(mPrice, 0, 12, dScript::NUM_FORMAT_REGION);
        if (mPrice > 0 && mItem0.mId != dItem::ITEM_ID_NONE) {
            if ((int)getBuyPrice(&mItem0) >= mPrice) {
                setMsgProc(&talk_c::msgStallPriceOK);
            } else {
                setMsgProc(&talk_c::msgStallTooHigh);
            }
            done = TRUE;
        }
    }
    if (!done) {
        setMsgProc(&talk_c::msgStallRefused);
    }
    startMsg();
    reqMsgClose();
}

// 80048B0C
int dAcNpcNml_c::talk_c::msgStallPriceOK(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_FmarketPC, 6);
    setStepProc(&talk_c::stepStallBuy);
    return TRUE;
}

// 80048B70
BOOL dAcNpcNml_c::talk_c::stepStallBuy(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        BOOL ok = FALSE;
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player != NULL && mPrice > 0 && player->getMoneyRoom(0) >= mPrice) {
            ok = TRUE;
        }
        if (ok) {
            dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
            mFtrHandle = fn_800A8F98(fn_800A9058(), mFtrPosX, mFtrPosZ, 0);
            if (npc != NULL && npc->removeRoomFtr(mFtrHandle, mFtrPosX, mFtrPosZ)) {
                dDemo_c *ctrl = getController();
                if (ctrl != NULL) {
                    ctrl->mLock = 1;
                }
                player->addMoney(mPrice);
                dAnimal_c *animal = getAnimal();
                if (animal != NULL && mItem0.mId != dItem::ITEM_ID_NONE && animal->addNewItem(&mItem0)) {
                    fn_800F0E9C(getNpcIdx(), &mItem0);
                }
                dNpcEntry_c *entry = getEntry();
                if (entry != NULL) {
                    entry->mStallBuyCount++;
                }
                setActProc(&talk_c::actStallBuyWait);
                return TRUE;
            }
            ok = FALSE;
        }
        if (!ok) {
            setMsgProc(&talk_c::msgStallNoRoom);
            startMsg();
            return TRUE;
        }
    }
    return FALSE;
}

// 80048D3C
void dAcNpcNml_c::talk_c::actStallBuyWait() {
    if (!fn_800A9354(fn_800A93BC(), mFtrHandle)) {
        dDemo_c *ctrl = getController();
        if (ctrl != NULL) {
            ctrl->mLock = 0;
        }
        setActProc(NULL);
        setMsgProc(&talk_c::msgStallBought);
        startMsg();
    }
}

// 80048DE4
int dAcNpcNml_c::talk_c::msgStallBought(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_FmarketPC, 9);
    return TRUE;
}

// 80048E14
int dAcNpcNml_c::talk_c::msgStallNoRoom(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_FmarketPC, 8);
    return TRUE;
}

// 80048E44
void dAcNpcNml_c::talk_c::selStallNo() {
    setMsgProc(&talk_c::msgStallRefused);
    startMsg();
}

// 80048E98
int dAcNpcNml_c::talk_c::msgStallRefused(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_FmarketPC, 5);
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->mStallRefuseCount++;
    }
    return TRUE;
}

// 80048EF0
int dAcNpcNml_c::talk_c::msgStallTooHigh(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_FmarketPC, 7);
    setStepProc(&talk_c::stepStallTooHigh);
    return TRUE;
}

// 80048F54
BOOL dAcNpcNml_c::talk_c::stepStallTooHigh(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoice(0, 0x6F, 1, &talk_c::selStallPrice);
        setChoice(1, 0x70, 1, &talk_c::selStallNo);
        setChoiceNum(2);
        setChoiceCancel(1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}
