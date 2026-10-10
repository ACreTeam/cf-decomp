// The villager talk on Halloween: candy, tricks and the costume remarks.
// .text 80043EBC..80045154. See include/game/game/d_npc_talk_halloween.hpp.
#include <game/game/d_npc_talk_halloween.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

const char l_Ev_Halloween[13] = "Ev_Halloween"; // 8046C6D0

// 80043EBC
int dAcNpcNml_c::talk_c::msgCandyAsk(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Halloween, 0x15);
    setHookProc(&talk_c::endCandyAsk);
    setStepProc(&talk_c::stepCandyChoice);
    return TRUE;
}

// 80043F48
void dAcNpcNml_c::talk_c::endCandyAsk(int arg) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (mpMemory != NULL && player != NULL) {
        recordTalk(NULL);
    }
    dNpc::msgMemorySecond_c *mem = getRememberedMsg();
    if (mem != NULL) {
        mem->clear();
    }
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->mMsg.set(mMessageLabel, mMessageCode, mTopicKind, mTopicGroup, mTopicIdx);
    }
}

// 80043FDC
BOOL dAcNpcNml_c::talk_c::stepCandyChoice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoiceProc(0, &talk_c::selCandyGive);
        setChoiceProc(1, &talk_c::selCandyNone);
        setChoiceNum(2);
        setChoiceCancel(1);
        return TRUE;
    }
    return FALSE;
}

// 8004409C
void dAcNpcNml_c::talk_c::selCandyGive() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    u16 mask = 0;
    if (player != NULL) {
        countPocketsUsed(&mask, NULL);
    }
    mask = ~mask;
    reqSelectItem(mask, 0x22, TRUE);
    mResultProc = &talk_c::resCandyGive;
}

// 80044120
void dAcNpcNml_c::talk_c::resCandyGive() {
    BOOL done = FALSE;
    if (!isMenuInvalid()) {
        u32 slot = getMenuSelSlot();
        if (slot < 15) {
            dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
            if (player != NULL) {
                BOOL candy = FALSE;
                dItem::Item item = player->mPockets[slot];
                if (item.isValid()) {
                    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
                    if (bitm != NULL && bitm->getKind() == dItem::KIND_CANDY) {
                        candy = TRUE;
                    }
                }
                requestItemActEx(7, &item, 0, 0, 0, 2);
                player->clearPocket(slot);
                if (candy) {
                    mNextResultProc = &talk_c::resCandyGiven;
                } else {
                    mNextResultProc = &talk_c::resCandyWrong;
                }
                done = TRUE;
            }
        }
    }
    if (!done) {
        setMsgProc(&talk_c::msgCandyNone);
        startMsg();
        reqMsgClose();
    }
}

// 800442A8
void dAcNpcNml_c::talk_c::resCandyGiven() {
    setMsgProc(&talk_c::msgCandyThanks);
    startMsg();
    reqMsgClose();
}

// 80044304
int dAcNpcNml_c::talk_c::msgCandyThanks(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Halloween, 0x17);
    setStepProc(&talk_c::stepCandyThanks);
    return TRUE;
}

// 80044368
BOOL dAcNpcNml_c::talk_c::stepCandyThanks(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        requestHandAct10(2);
        setMsgProc(&talk_c::msgCandyCostume);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 800443EC
int dAcNpcNml_c::talk_c::msgCandyCostume(msgInfo_s *info) {
    int code = 0x1A;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (mpMemory != NULL && player != NULL) {
        mEquip.setFromPlayer();
        code = isInCostume(&mEquip) ? 0x19 : 0x18;
    }
    setLooksMsg(info, l_Ev_Halloween, code);
    return TRUE;
}

// 80044480
void dAcNpcNml_c::talk_c::resCandyWrong() {
    setMsgProc(&talk_c::msgCandyWrong);
    startMsg();
    reqMsgClose();
}

// 800444DC
int dAcNpcNml_c::talk_c::msgCandyWrong(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Halloween, 0x1B);
    setStepProc(&talk_c::stepCandyWrong);
    return TRUE;
}

// 80044540
BOOL dAcNpcNml_c::talk_c::stepCandyWrong(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        requestHandActD();
        mResultProc = &talk_c::resCandyTrick;
        setMsgProc(&talk_c::msgCandyTrick);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 800445DC
BOOL dAcNpcNml_c::talk_c::filterTrickItem(const dItem::Item *item, int arg) {
    if (!item->isValid() || arg != 0) {
        return FALSE;
    }
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL) {
        return FALSE;
    }
    return bitm->m_noPurchase ^ 1;
}

// 80044640
int dAcNpcNml_c::talk_c::countTrickPockets(u16 *slotMask, dPrivateData_c *player) {
    int count = 0;
    if (slotMask != NULL) {
        *slotMask = 0;
    }
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player != NULL) {
        u32 num = player->countPocketsFlag(filterTrickItem, slotMask);
        if (num <= 15) {
            count = num;
        }
    }
    return count;
}

// 800446C4
BOOL dAcNpcNml_c::talk_c::trickPockets() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL) {
        return FALSE;
    }
    u16 mask = 0;
    int num = countTrickPockets(&mask, player);
    if (num == 0) {
        return FALSE;
    }
    dItem::Item jackInTheBox(dItem::ITEM_IDX_JACK_IN_THE_BOX);
    int pick = cM::rndInt(num);
    for (int i = 0; i < 15; i++) {
        if ((mask >> i) & 1) {
            if (pick == 0) {
                player->setPocket(&jackInTheBox, i, FALSE);
                break;
            }
            pick--;
        }
    }
    return TRUE;
}

// 8004478C
BOOL dAcNpcNml_c::talk_c::isMatchingOutfit(const dItem::Item *hat, const dItem::Item *shirt, const dItem::Item *acc) {
    if (fn_800F8948(hat, shirt) || fn_800F8948(acc, shirt)) {
        return TRUE;
    }
    return FALSE;
}

// 800447EC
BOOL dAcNpcNml_c::talk_c::isInCostume(const dEquip_c *equip) {
    return isMatchingOutfit(&equip->mHat, &equip->mShirt, &equip->mAcc);
}

// 80044800
void dAcNpcNml_c::talk_c::trickClothes() {
    mEquip.setFromPlayer();
    dItem::Item *hat = &mEquip.mHat;
    dItem::Item *shirt = &mEquip.mShirt;
    dItem::Item pumpkinHead(dItem::ITEM_IDX_PUMPKIN_HEAD);
    dItem::Item moldyShirt(dItem::ITEM_IDX_MOLDY_SHIRT);
    dItem::Item patchedShirt(dItem::ITEM_IDX_PATCHED_SHIRT);
    const dItem::BITM *hatBitm;
    if (hat->isValid()) {
        hatBitm = dItem::infoBank_c::get()->getBITM(*hat);
    } else {
        hatBitm = NULL;
    }
    const dItem::BITM *accBitm;
    if (mEquip.mAcc.isValid()) {
        accBitm = dItem::infoBank_c::get()->getBITM(mEquip.mAcc);
    } else {
        accBitm = NULL;
    }
    const dItem::BITM *shirtBitm;
    // @BUG - devs checked the hat instead of the shirt: with no hat on, the shirt trick never happens
    // (a player without a hat and with an unbuyable accessory gets the pocket trick instead)
#ifndef BUGFIXES
    if (hat->isValid()) {
#else
    if (shirt->isValid()) {
#endif
        shirtBitm = dItem::infoBank_c::get()->getBITM(*shirt);
    } else {
        shirtBitm = NULL;
    }
    if (isInCostume(&mEquip)) {
        trickPockets();
    } else if (hat->isNotSame(pumpkinHead) && (!hat->isValid() || (hatBitm != NULL && !hatBitm->m_noPurchase)) &&
               (!mEquip.mAcc.isValid() || (accBitm != NULL && !accBitm->m_noPurchase))) {
        mEquip.mHat = pumpkinHead;
        mEquip.mAcc = dItem::Item();
    } else if (shirt->isNotSame(patchedShirt) && shirt->isNotSame(moldyShirt) &&
               (!shirt->isValid() || (shirtBitm != NULL && !shirtBitm->m_noPurchase))) {
        if (cM::rndInt(2) == 0) {
            mEquip.mShirt = moldyShirt;
        } else {
            mEquip.mShirt = patchedShirt;
        }
    } else {
        trickPockets();
    }
    requestEquip(&mEquip, 6, 0);
}

// 80044A00
void dAcNpcNml_c::talk_c::resCandyTrick() {
    trickClothes();
}

// 80044A04
int dAcNpcNml_c::talk_c::msgCandyTrick(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Halloween, 0x1C);
    return TRUE;
}

// 80044A34
int dAcNpcNml_c::talk_c::msgCandyNone(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Halloween, 0x1D);
    setStepProc(&talk_c::stepCandyNone);
    return TRUE;
}

// 80044A98
BOOL dAcNpcNml_c::talk_c::stepCandyNone(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgCandyNoneTrick);
        startMsg();
        trickClothes();
        return TRUE;
    }
    return FALSE;
}

// 80044B18
int dAcNpcNml_c::talk_c::msgCandyNoneTrick(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Halloween, 0x1E);
    return TRUE;
}

// 80044B48
void dAcNpcNml_c::talk_c::selCandyNone() {
    setMsgProc(&talk_c::msgCandyNone);
    startMsg();
}

// 80044B9C
int dAcNpcNml_c::talk_c::msgHalloweenCostume(msgInfo_s *info) {
    static const char l_label[] = "Ev_Halloween"; // 8046C6E0
    u16 code = 0x1F;
    dEquip_c equip;
    equip.setFromPlayer();
    if (isInCostume(&equip)) {
        code = 0x20;
    }
    setLooksMsg(info, l_label, code);
    setHookProc(&talk_c::endHalloweenCostume);
    return TRUE;
}

// 80044C50
void dAcNpcNml_c::talk_c::endHalloweenCostume(int arg) {
    recordTalk(NULL);
    dNpc::msgMemorySecond_c *mem = getRememberedMsg();
    if (mem != NULL) {
        mem->clear();
    }
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->mMsg.set(mMessageLabel, mMessageCode, mTopicKind, mTopicGroup, mTopicIdx);
    }
}

// 80044CC8
int dAcNpcNml_c::talk_c::msgHalloween(msgInfo_s *info) {
    static dQuestEvent_e l_event = EVENT_HALLOWEEN; // 80749A48
    BOOL on = dEvent::isOngoing(l_event) != FALSE;
    if (on) {
        dSaveTown_c *town = dSaveData_c::getTown();
        dAnimal_c *animal = getAnimal();
        if ((dQuestEvent_e)town->mAnimals.mTown.mEventId != EVENT_HALLOWEEN || animal == NULL || !animal->mEvent.isInEvent() ||
            !isCurrentSceneAttr(SCENE_ATTR_VILLAGER_HOUSE)) {
            on = FALSE;
        }
    }
    if (on) {
        int idx = 0;
        if (mpNpc != NULL && static_cast<dAcNpcNml_c *>(mpNpc)->isSessionFlag(0)) {
            idx = 3;
        } else {
            dEquip_c equip;
            equip.setFromPlayer();
            if (isInCostume(&equip)) {
                idx = 2;
            } else if (mpMemory != NULL && mpMemory->isEventFlag(0)) {
                idx = 1;
            }
        }
        const char *label = getMsgLabel(TALK_HALLOWEEN, idx, 0);
        if (label != NULL) {
            stepFunc step = NULL;
            u16 code = 1;
            switch (idx) {
            case 0:
                step = &talk_c::stepHalloweenFirst;
                code = 1;
                break;
            case 1:
                code = 4;
                break;
            case 2:
                step = &talk_c::stepHalloweenPresent;
                code = 2;
                break;
            case 3:
                code = 5;
                break;
            }
            setLooksMsg(info, label, code);
            setProcSet(&l_talkEntrySets[TALK_HALLOWEEN]);
            if (step) {
                setStepProc(step);
            }
            dNpc::msgMemorySecond_c *mem = getRememberedMsg();
            if (mem != NULL) {
                mem->clear();
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 80044F2C
void dAcNpcNml_c::talk_c::endHalloween(int arg) {
    recordTalk(NULL);
    if (mpMemory != NULL) {
        mpMemory->mTalkCount.inc(0x44);
    }
    dNpc::msgMemorySecond_c *mem = getRememberedMsg();
    if (mem != NULL) {
        mem->clear();
    }
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->mMsg.set(mMessageLabel, mMessageCode, mTopicKind, mTopicGroup, mTopicIdx);
    }
}

// 80044FBC
BOOL dAcNpcNml_c::talk_c::stepHalloweenFirst(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        if (mpMemory != NULL) {
            mpMemory->onEventFlag(0);
            if (mpNpc != NULL) {
                fn_800F04E8(static_cast<dAcNpcNml_c *>(mpNpc)->getNpcIdx(), mMemoryIdx, 0);
            }
        }
        return TRUE;
    }
    return FALSE;
}

// 80045038
BOOL dAcNpcNml_c::talk_c::stepHalloweenPresent(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        u16 code = mMessageCode;
        int slot = -1;
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        dItem::Item present;
        if (code == 3 && player != NULL) {
            slot = player->findEmptyPocket(0);
        }
        if ((u32)slot < 15) {
            int range[2];
            range[0] = dItem::KIND_CANDY;
            range[1] = 0x2C;
            fn_800C60B4(&present, 1, range, 1, lbl_8059FF80, NULL, 0, 0);
        }
        if (present.isValid()) {
            requestItemAct(&present, 0, 0);
            player->setPocket(&present, slot, FALSE);
            if (mpNpc != NULL) {
                static_cast<dAcNpcNml_c *>(mpNpc)->onSessionFlag(0);
            }
        }
        return TRUE;
    }
    return FALSE;
}
