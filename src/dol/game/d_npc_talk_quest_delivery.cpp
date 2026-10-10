// Villager talk when the player brings an errand delivery (errand kinds 7 / 8: a package or clothes),
// plus the shared quest end-of-talk hook endQuestCommon. .text 80052EB0..800546F4. See
// include/game/game/d_npc_talk_quest_delivery.hpp.
#include <game/game/d_npc_talk_quest_delivery.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_animal_id.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_demo.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

// 80052EB0
int dAcNpcNml_c::talk_c::msgDelivery(msgInfo_s *info) {
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    dAnmPersonalID_c *self = &animal->mID;
    const dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL || player->isFlag0(0xD)) {
        return FALSE;
    }
    if (isEventOngoing()) {
        return FALSE;
    }
    const dQuestErrand_c *errand;
    if (player != NULL) {
        errand = player->mErrand.get(0);
    } else {
        errand = NULL;
    }
    if (errand == NULL) {
        return FALSE;
    }
    if (!errand->mBase.isActive() || errand->mBase.mState != QUEST_ERRAND_DELIVERING) {
        return FALSE;
    }
    if (!(*errand->getAnimal(1) == *self)) {
        return FALSE;
    }
    const dItem::Item *item = &errand->mBase.mItem;
    if (!item->isValid() || (!countPocketsItem(item, 0, NULL, NULL) && !countPocketsItem(item, 2, NULL, NULL))) {
        return FALSE;
    }

    int questKind = errand->mBase.mKind;
    stepFunc step = NULL;
    int kind = 0;
    switch (questKind) {
    case QUEST_KIND_ERRAND_REQUEST:
        step = &talk_c::stepPackageChoice;
        kind = 0;
        break;
    case QUEST_KIND_ERRAND_REQUEST_FINAL:
        step = &talk_c::stepClothesChoice;
        kind = 1;
        break;
    }
    if (step) {
        const char *label = getMsgLabel(TALK_QUEST_DELIVERY, kind, 0);
        if (label != NULL) {
            dAnmPersonalID_c *sender = errand->getAnimal(0);
            setAnmPersonalName(sender, 2);
            u16 unit = fn_800F3F38(sender->getGender(1), self->getLooks(1));
            if (unit == 0) {
                clearWord(3);
            } else {
                getController()->fn_801A5874(3, unit, "sys_STRING/STR_Unit");
            }
            setItemName(&errand->mBase.mItem, 4);
            setLooksMsg(info, label, 0);
            setProcSet(&l_talkEntrySets[TALK_QUEST_DELIVERY]);
            setStepProc(step);
            setTopic(dNpc::msgMemory_c::KIND_QUEST, kind, 0);
            return TRUE;
        }
    }
    return FALSE;
}

// 80053224
void dAcNpcNml_c::talk_c::endQuestCommon(int arg) {
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

// 800532B4
void dAcNpcNml_c::talk_c::startDeliverySelect(hookFunc next) {
    const dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    const dQuestErrand_c *errand = player != NULL ? player->mErrand.get(0) : NULL;
    if (errand != NULL) {
        dItem::Item item = errand->mBase.mItem;
        if (item.isValid()) {
            u16 mask = 0;
            if (!countPocketsItem(&item, 2, &mask, NULL)) {
                countPocketsItem(&item, 0, &mask, NULL);
            }
            mask = ~mask;
            reqSelectItem(mask, 0x22, TRUE);
            mResultProc = next;
        }
    }
}

// 80053390
int dAcNpcNml_c::talk_c::msgDeliveryCancel(msgInfo_s *info) {
    setLooksMsg(info, l_Q_Cancel, 0);
    return TRUE;
}

// 800533C0
void dAcNpcNml_c::talk_c::selDeliveryResume() {
    const procSet_s *set = getEventProcSet();
    if (set == NULL) {
        set = &l_talkEntrySets[TALK_FREE];
    }
    setProcSet(set);
    startMsg();
}

// 80053414
BOOL dAcNpcNml_c::talk_c::stepPackageChoice(int kind) {
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
        setChoice(n, 0xD, 3, &talk_c::selPackageGive);
        setChoice(n + 1, 4, 3, &talk_c::selDeliveryResume);
        setChoiceNum(n + 2);
        setChoiceCancel(n + 1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 800535E8
void dAcNpcNml_c::talk_c::selPackageGive() {
    startDeliverySelect(&talk_c::resPackageSelect);
}

// 80053628
void dAcNpcNml_c::talk_c::resPackageSelect() {
    BOOL done = FALSE;
    if (!isMenuInvalid()) {
        u32 slot = getMenuSelSlot();
        if (slot < 15) {
            dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
            if (player != NULL) {
                dItem::Item item = player->mPockets[slot];
                int flag = player->getPocketFlag(slot);
                player->clearPocket(slot);
                requestItemActEx(7, &item, flag, 0, 0, 2);
                if (flag == 2) {
                    mNextResultProc = &talk_c::resPackageWrapped;
                } else {
                    mNextResultProc = &talk_c::resPackageOpened;
                }
                done = TRUE;
            }
        }
    }
    if (!done) {
        setMsgProc(&talk_c::msgDeliveryCancel);
        startMsg();
        reqMsgClose();
    }
}

// 80053774
void dAcNpcNml_c::talk_c::resPackageWrapped() {
    setMsgProc(&talk_c::msgPackageGet);
    startMsg();
    reqMsgClose();
}

// 800537D0
int dAcNpcNml_c::talk_c::msgPackageGet(msgInfo_s *info) {
    static const char l_Q06_Get0[] = "Q06_Get0";
    setLooksMsg(info, l_Q06_Get0, 0);
    setStepProc(&talk_c::stepPackageGet);
    addFriendship(5);
    return TRUE;
}

// 80053840
BOOL dAcNpcNml_c::talk_c::stepPackageGet(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgPackageGet2);
        startMsg();
        requestHandActF();
        return TRUE;
    }
    return FALSE;
}

// 800538C0
int dAcNpcNml_c::talk_c::msgPackageGet2(msgInfo_s *info) {
    static const char l_Q06_Get1[] = "Q06_Get1";
    setLooksMsg(info, l_Q06_Get1, 0);
    setStepProc(&talk_c::stepPackageGet2);
    return TRUE;
}

// 80053924
BOOL dAcNpcNml_c::talk_c::stepPackageGet2(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        dQuestErrand_c *errand = player != NULL ? player->mErrand.get(0) : NULL;
        if (errand != NULL) {
            if (errand->mBase.isPastDeadline(NULL)) {
                errand->mBase.mState = QUEST_ERRAND_LATE;
            } else {
                errand->mBase.mState = QUEST_ERRAND_DONE;
            }
        }
        setMsgProc(&talk_c::msgPackageFin);
        startMsg();
        requestHandActD();
        return TRUE;
    }
    return FALSE;
}

// 80053A04
int dAcNpcNml_c::talk_c::msgPackageFin(msgInfo_s *info) {
    static const char l_Q06_Fin[] = "Q06_Fin";
    setLooksMsg(info, l_Q06_Fin, 0);
    return TRUE;
}

// 80053A30
void dAcNpcNml_c::talk_c::resPackageOpened() {
    setMsgProc(&talk_c::msgPackageOpened);
    startMsg();
    reqMsgClose();
}

// 80053A8C
int dAcNpcNml_c::talk_c::msgPackageOpened(msgInfo_s *info) {
    static const char l_Q06_Open[] = "Q06_Open";
    setLooksMsg(info, l_Q06_Open, 0);
    setStepProc(&talk_c::stepPackageOpened);
    addFriendship(3);
    return TRUE;
}

// 80053AFC
BOOL dAcNpcNml_c::talk_c::stepPackageOpened(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        dQuestErrand_c *errand = player != NULL ? player->mErrand.get(0) : NULL;
        if (errand != NULL) {
            if (errand->mBase.isPastDeadline(NULL)) {
                errand->mBase.mState = QUEST_ERRAND_LATE;
            } else {
                errand->mBase.mState = QUEST_ERRAND_DONE;
            }
        }
        setMsgProc(&talk_c::msgPackageFin);
        startMsg();
        requestHandActD();
        return TRUE;
    }
    return FALSE;
}

// 80053BDC
BOOL dAcNpcNml_c::talk_c::stepClothesChoice(int kind) {
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
        setChoice(n, 0xD, 3, &talk_c::selClothesGive);
        setChoice(n + 1, 4, 3, &talk_c::selDeliveryResume);
        setChoiceNum(n + 2);
        setChoiceCancel(n + 1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 80053DB0
void dAcNpcNml_c::talk_c::selClothesGive() {
    startDeliverySelect(&talk_c::resClothesSelect);
}

// 80053DF0
void dAcNpcNml_c::talk_c::resClothesSelect() {
    BOOL done = FALSE;
    if (!isMenuInvalid()) {
        u32 slot = getMenuSelSlot();
        if (slot < 15) {
            dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
            if (player != NULL) {
                dItem::Item item = player->mPockets[slot];
                int flag = player->getPocketFlag(slot);
                player->clearPocket(slot);
                mItem1 = item;
                requestItemActEx(7, &item, flag, 0, 0, 2);
                if (flag == 2) {
                    mNextResultProc = &talk_c::resClothesWrapped;
                } else {
                    mNextResultProc = &talk_c::resClothesOpened;
                }
                done = TRUE;
            }
        }
    }
    if (!done) {
        setMsgProc(&talk_c::msgDeliveryCancel);
        startMsg();
        reqMsgClose();
    }
}

// 80053F44
void dAcNpcNml_c::talk_c::resClothesWrapped() {
    setMsgProc(&talk_c::msgClothesGet);
    startMsg();
    reqMsgClose();
}

// 80053FA0
int dAcNpcNml_c::talk_c::msgClothesGet(msgInfo_s *info) {
    static const char l_Q07_Get0[] = "Q07_Get0";
    setLooksMsg(info, l_Q07_Get0, 0);
    setStepProc(&talk_c::stepClothesGet);
    return TRUE;
}

// 80054004
int dAcNpcNml_c::talk_c::updateClothesMatch() {
    dAnimal_c *animal = getAnimal();
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dQuestErrand_c *errand = player != NULL ? player->mErrand.get(0) : NULL;
    int match = errand != NULL && animal != NULL ? animal->getStyleMatch(&errand->mBase.mItem) : 2;
    if (errand != NULL) {
        if (errand->mBase.isPastDeadline(NULL)) {
            errand->mBase.mState = QUEST_ERRAND_FINAL_LATE;
        } else {
            errand->mBase.mState = match + QUEST_ERRAND_DONE;
        }
    }
    return match;
}

// 800540C8
BOOL dAcNpcNml_c::talk_c::stepClothesGet(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        switch (updateClothesMatch()) {
        case 0:
            setMsgProc(&talk_c::msgClothesGet1);
            break;
        case 1:
            setMsgProc(&talk_c::msgClothesGet3);
            break;
        default:
            setMsgProc(&talk_c::msgClothesGet2);
            break;
        }
        startMsg();
        requestHandActF();
        return TRUE;
    }
    return FALSE;
}

// 800541D0
int dAcNpcNml_c::talk_c::msgClothesGet1(msgInfo_s *info) {
    static const char l_Q07_Get1[] = "Q07_Get1";
    setLooksMsg(info, l_Q07_Get1, 0);
    setStepProc(&talk_c::stepClothesWear);
    addFriendship(5);
    return TRUE;
}

// 80054240
BOOL dAcNpcNml_c::talk_c::stepClothesWear(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgClothesFin);
        startMsg();
        dAnimal_c *animal = getAnimal();
        if (animal != NULL && mItem1.isValid()) {
            animal->setCloth(&mItem1);
            dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
            fn_800F1A68(getNpcIdx(), &animal->mCloth);
            npc->onDaubClothChange();
            npc->offDaubClothChanged();
        }
        requestHandActC();
        mResultProc = &talk_c::resClothesWear;
        return TRUE;
    }
    return FALSE;
}

// 80054338
void dAcNpcNml_c::talk_c::resClothesWear() {
    dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
    npc->offDaubClothChange();
    npc->onDaubClothChanged();
}

// 80054370
int dAcNpcNml_c::talk_c::msgClothesFin(msgInfo_s *info) {
    static const char l_Q07_Fin[] = "Q07_Fin";
    setLooksMsg(info, l_Q07_Fin, 0);
    return TRUE;
}

// 8005439C
int dAcNpcNml_c::talk_c::msgClothesGet2(msgInfo_s *info) {
    static const char l_Q07_Get2[] = "Q07_Get2";
    setLooksMsg(info, l_Q07_Get2, 0);
    setStepProc(&talk_c::stepClothesWear);
    addFriendship(5);
    return TRUE;
}

// 8005440C
int dAcNpcNml_c::talk_c::msgClothesGet3(msgInfo_s *info) {
    static const char l_Q07_Get3[] = "Q07_Get3";
    setLooksMsg(info, l_Q07_Get3, 0);
    setStepProc(&talk_c::stepClothesKeep);
    addFriendship(5);
    return TRUE;
}

// 8005447C
BOOL dAcNpcNml_c::talk_c::stepClothesKeep(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        requestHandActD();
        return TRUE;
    }
    return FALSE;
}

// 800544C0
void dAcNpcNml_c::talk_c::resClothesOpened() {
    switch (updateClothesMatch()) {
    case 0:
        setMsgProc(&talk_c::msgClothesOpen1);
        break;
    case 1:
        setMsgProc(&talk_c::msgClothesOpen3);
        break;
    default:
        setMsgProc(&talk_c::msgClothesOpen2);
        break;
    }
    startMsg();
    reqMsgClose();
}

// 800545A4
int dAcNpcNml_c::talk_c::msgClothesOpen1(msgInfo_s *info) {
    static const char l_Q07_Open1[] = "Q07_Open1";
    setLooksMsg(info, l_Q07_Open1, 0);
    setStepProc(&talk_c::stepClothesWear);
    addFriendship(3);
    return TRUE;
}

// 80054614
int dAcNpcNml_c::talk_c::msgClothesOpen2(msgInfo_s *info) {
    static const char l_Q07_Open2[] = "Q07_Open2";
    setLooksMsg(info, l_Q07_Open2, 0);
    setStepProc(&talk_c::stepClothesWear);
    addFriendship(3);
    return TRUE;
}

// 80054684
int dAcNpcNml_c::talk_c::msgClothesOpen3(msgInfo_s *info) {
    static const char l_Q07_Open3[] = "Q07_Open3";
    setLooksMsg(info, l_Q07_Open3, 0);
    setStepProc(&talk_c::stepClothesKeep);
    addFriendship(3);
    return TRUE;
}
