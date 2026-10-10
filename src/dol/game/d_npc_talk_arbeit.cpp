// The villager talk for the newcomer's first jobs ("Ev_Arbeit") and the moving-in / moving-out remarks.
// .text 80038134..80039B38. See include/game/game/d_npc_talk_arbeit.hpp.
#include <game/game/d_npc_talk_arbeit.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_save_data.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

// Message label groups of the errand talk (getMsgLabel(TALK_ARBEIT, idx, ..)).
const char *l_arbeitLabels[6] = {l_Ev_Arbeit, l_Ev_Arbeit, l_Ai_Quest, l_Ai_Quest, l_Ai_Quest, l_Ev_Arbeit};

// 80038134
int dAcNpcNml_c::talk_c::msgArbeitMoveIn(msgInfo_s *info) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimal_c *animal = getAnimal();
    BOOL ok = FALSE;
    if (player != NULL && animal != NULL && player->isFlag0(0xD)) {
        ok = animal->isMovingIn();
    }
    if (ok) {
        const char *label = getMsgLabel(TALK_ARBEIT, 0, 0);
        if (label != NULL) {
            setLooksMsg(info, label, 0x10);
            setProcSet(&l_talkEntrySets[TALK_ARBEIT]);
            dNpc::msgMemorySecond_c *mem = getRememberedMsg();
            if (mem != NULL) {
                mem->clear();
            }
            setTopic(dNpc::msgMemory_c::KIND_ARBEIT, 0, 0);
            return TRUE;
        }
    }
    return FALSE;
}

// 80038238
int dAcNpcNml_c::talk_c::msgArbeitMoveOut(msgInfo_s *info) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimal_c *animal = getAnimal();
    BOOL ok = FALSE;
    if (player != NULL && animal != NULL && player->isFlag0(0xD)) {
        ok = animal->isMovingOut();
    }
    if (ok) {
        const char *label = getMsgLabel(TALK_ARBEIT, 1, 0);
        if (label != NULL) {
            setLooksMsg(info, label, 0x11);
            setProcSet(&l_talkEntrySets[TALK_ARBEIT]);
            dNpc::msgMemorySecond_c *mem = getRememberedMsg();
            if (mem != NULL) {
                mem->clear();
            }
            setTopic(dNpc::msgMemory_c::KIND_ARBEIT, 1, 0);
            mCountTalk = 1;
            return TRUE;
        }
    }
    return FALSE;
}

// 80038344
BOOL dAcNpcNml_c::talk_c::msgArbeitErrand(msgInfo_s *info, u8 errandKind, u8 errandState, u8 labelIdx) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimal_c *animal = getAnimal();
    dQuestErrandList_c *errand;
    BOOL ok = FALSE;
    if (player != NULL && animal != NULL) {
        errand = &player->mErrand;
        if (errand->isActive()) {
            const dAnmPersonalID_c *sender = errand->getSender();
            if (sender != NULL && *sender == animal->mID) {
                int kind = errand->getKind();
                if (kind == errandKind && errand->isState(errandState)) {
                    dItem::Item item = *errand->getItem();
                    if (countPocketsItem(&item, 3, NULL, NULL)) {
                        ok = TRUE;
                    }
                }
            }
        }
    }
    if (ok) {
        const char *label = getMsgLabel(TALK_ARBEIT, labelIdx, 0);
        if (label != NULL) {
            setLooksMsg(info, label, 0);
            setProcSet(&l_talkEntrySets[TALK_ARBEIT]);
            setStepProc(&talk_c::stepArbeitErrandChoice);
            dNpc::msgMemorySecond_c *mem = getRememberedMsg();
            if (mem != NULL) {
                mem->clear();
            }
            setTopic(dNpc::msgMemory_c::KIND_ARBEIT, labelIdx, 0);
            mCountTalk = 1;
            return TRUE;
        }
    }
    return FALSE;
}

// 80038570
int dAcNpcNml_c::talk_c::msgArbeitDeliverFtr(msgInfo_s *info) {
    return msgArbeitErrand(info, QUEST_KIND_FIRSTJOB_DELIVER_FTR, 0, 2);
}

// 80038580
int dAcNpcNml_c::talk_c::msgArbeitDeliverCarpet(msgInfo_s *info) {
    return msgArbeitErrand(info, QUEST_KIND_FIRSTJOB_DELIVER_CARPET, 0, 3);
}

// 80038590
int dAcNpcNml_c::talk_c::msgArbeitDeliverCan(msgInfo_s *info) {
    return msgArbeitErrand(info, QUEST_KIND_FIRSTJOB_DELIVER_WATERING_CAN, 0, 4);
}

// 800385A0
int dAcNpcNml_c::talk_c::msgArbeitNewcomer(msgInfo_s *info) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL ? player->isFlag0(0xD) : FALSE) {
        const char *label = getMsgLabel(TALK_ARBEIT, 5, 0);
        if (label != NULL) {
            setLooksMsg(info, label, 0);
            setProcSet(&l_talkEntrySets[TALK_ARBEIT]);
            setStepProc(&talk_c::stepArbeitNewcomerChoice);
            dNpc::msgMemorySecond_c *mem = getRememberedMsg();
            if (mem != NULL) {
                mem->clear();
            }
            setTopic(dNpc::msgMemory_c::KIND_ARBEIT, 5, 0);
            return TRUE;
        }
    }
    return FALSE;
}

// 800386A0
int dAcNpcNml_c::talk_c::msgArbeit(msgInfo_s *info) {
    static const msgFunc l_checks[6] = {
        &talk_c::msgArbeitMoveIn, &talk_c::msgArbeitMoveOut, &talk_c::msgArbeitDeliverFtr,
        &talk_c::msgArbeitDeliverCarpet, &talk_c::msgArbeitDeliverCan, &talk_c::msgArbeitNewcomer,
    };
    for (int i = 0; i < 6; i++) {
        if (l_checks[i] && (this->*l_checks[i])(info)) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80038738
void dAcNpcNml_c::talk_c::endArbeit(int arg) {
    recordTalk(NULL);
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->mMsg.set(mMessageLabel, mMessageCode, mTopicKind, mTopicGroup, mTopicIdx);
    }
    if (mCountTalk) {
        if (mpMemory != NULL) {
            mpMemory->mTalkCount.inc(0x44);
        }
        mCountTalk = 0;
    }
}

// 800387BC
BOOL dAcNpcNml_c::talk_c::stepArbeitErrandChoice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoice(0, 0xD, 3, &talk_c::selArbeitGive);
        setChoice(1, 0x4, 3, &talk_c::selArbeitOther);
        setChoiceNum(2);
        setChoiceCancel(1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 80038894
void dAcNpcNml_c::talk_c::selArbeitGive() {
    dItem::Item item = *dPlayerMgr_c::getCurrentPlayer()->mErrand.getItem();
    u16 mask = 0;
    countPocketsItem(&item, 3, &mask, NULL);
    mask = ~mask;
    reqSelectItem(mask, 0x22, TRUE);
    mResultProc = &talk_c::resArbeitGive;
}

// 80038928
void dAcNpcNml_c::talk_c::resArbeitGive() {
    BOOL done = FALSE;
    if (!isMenuInvalid()) {
        u32 slot = getMenuSelSlot();
        if (slot < 15) {
            dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
            if (player != NULL) {
                dItem::Item item = player->mPockets[slot];
                player->clearPocket(slot);
                switch (player->mErrand.getKind()) {
                case QUEST_KIND_FIRSTJOB_DELIVER_FTR:
                    requestItemActEx(7, &item, 0, 0, 0, 2);
                    break;
                case QUEST_KIND_FIRSTJOB_SEND_LETTER:
                    break;
                case QUEST_KIND_FIRSTJOB_DELIVER_CARPET:
                case QUEST_KIND_FIRSTJOB_DELIVER_WATERING_CAN:
                    requestItemActEx(6, &item, 0, 0, 0, 2);
                    break;
                }
                mNextResultProc = &talk_c::resArbeitGiven;
                done = TRUE;
            }
        }
    }
    if (!done) {
        setMsgProc(&talk_c::msgArbeitCancel);
        startMsg();
        reqMsgClose();
    }
}

// 80038A94
void dAcNpcNml_c::talk_c::resArbeitGiven() {
    setMsgProc(&talk_c::msgArbeitGiven);
    startMsg();
    reqMsgClose();
}

// 80038AF0
int dAcNpcNml_c::talk_c::msgArbeitGiven(msgInfo_s *info) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dQuestErrandList_c *errand = &player->mErrand;
    int kind = errand->getKind();
    stepFunc step = NULL;
    u16 code = 0x15;
    switch (kind) {
    case QUEST_KIND_FIRSTJOB_DELIVER_FTR:
        step = &talk_c::stepArbeitFtrDone;
        code = 0x14;
        break;
    case QUEST_KIND_FIRSTJOB_DELIVER_CARPET: {
        step = &talk_c::stepArbeitCarpetReward;
        code = 0x1E;
        int range[2];
        range[0] = 2;
        range[1] = 3;
        if (!fn_800C60B4(&mItem0, 1, range, 1, lbl_8059FF80, NULL, 0, 0)) {
            mItem0.setFromIndex(dItem::ITEM_IDX_EXOTIC_RUG);
        }
        setItemName(&mItem0, 1);
        dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayer();
        if (current != NULL) {
            current->pickUp(&mItem0, FALSE);
        }
        break;
    }
    case QUEST_KIND_FIRSTJOB_DELIVER_WATERING_CAN: {
        BOOL isSender;
        dAnimal_c *animal = getAnimal();
        isSender = FALSE;
        if (animal != NULL && errand->isSender(&animal->mID)) {
            isSender = TRUE;
        }
        BOOL hasLetter = FALSE;
        if (animal != NULL && animal->hasLetterFrom(&player->mPID)) {
            hasLetter = TRUE;
        }
        if (!isSender) {
            step = &talk_c::stepArbeitErrandNext;
            code = 0x23;
        } else if (hasLetter) {
            step = &talk_c::stepArbeitLetterMenu;
            code = 0x20;
        } else {
            step = &talk_c::stepArbeitErrandNext;
            code = 0x22;
        }
        break;
    }
    }
    setLooksMsg(info, l_Ev_Arbeit, code);
    setStepProc(step);
    return TRUE;
}

// 80038D4C
BOOL dAcNpcNml_c::talk_c::stepArbeitFtrDone(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgArbeitFtrReward);
        startMsg();
        requestHandActD();
        return TRUE;
    }
    return FALSE;
}

// 80038DCC
int dAcNpcNml_c::talk_c::msgArbeitFtrReward(msgInfo_s *info) {
    int range[2];
    range[0] = 3;
    range[1] = 3;
    if (!fn_800C60B4(&mItem0, 1, range, 1, lbl_8059FF80, NULL, 0, 0)) {
        mItem0.setFromIndex(dItem::ITEM_IDX_EXOTIC_BED);
    }
    setItemName(&mItem0, 0);
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        player->pickUp(&mItem0, FALSE);
    }
    setLooksMsg(info, l_Ev_Arbeit, 0x15);
    setStepProc(&talk_c::stepArbeitFtrReward);
    return TRUE;
}

// 80038EB4
BOOL dAcNpcNml_c::talk_c::stepArbeitFtrReward(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgArbeitBirthdayAsk);
        startMsg();
        requestItemAct(&mItem0, 0, 0);
        return TRUE;
    }
    return FALSE;
}

// 80038F40
int dAcNpcNml_c::talk_c::msgArbeitBirthdayAsk(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Arbeit, 0x16);
    setStepProc(&talk_c::stepArbeitBirthdayMenu);
    return TRUE;
}

// 80038FA4
BOOL dAcNpcNml_c::talk_c::stepArbeitBirthdayMenu(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        reqMenu1C(FALSE);
        mResultProc = &talk_c::resArbeitBirthday;
        return TRUE;
    }
    return FALSE;
}

// 80039014
void dAcNpcNml_c::talk_c::resArbeitBirthday() {
    setMsgProc(&talk_c::msgArbeitBirthdayCheck);
    startMsg();
}

// 80039068
int dAcNpcNml_c::talk_c::msgArbeitBirthdayCheck(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Arbeit, 0x18);
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    const dBirthday_c *birthday = player != NULL ? &player->mBirthday : NULL;
    int month = birthday != NULL ? birthday->mMonth : 0;
    int day = birthday != NULL ? birthday->mDay : 1;
    setMonthName(month, 2);
    setDayName(day, 3);
    setStepProc(&talk_c::stepArbeitBirthdayChoice);
    return TRUE;
}

// 80039134
BOOL dAcNpcNml_c::talk_c::stepArbeitBirthdayChoice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoice(0, 0x6B, 1, &talk_c::selArbeitBirthdayYes);
        setChoice(1, 0x6C, 1, &talk_c::selArbeitBirthdayNo);
        setChoiceNum(2);
        setChoiceCancel(1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 8003920C
void dAcNpcNml_c::talk_c::selArbeitBirthdayYes() {
    setMsgProc(&talk_c::msgArbeitBirthdayYes);
    startMsg();
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        player->mErrand.setState(2);
    }
}

// 80039278
int dAcNpcNml_c::talk_c::msgArbeitBirthdayYes(msgInfo_s *info) {
    u16 code = 0x1C;
    dTime_c *now = dTime_c::getCurrent();
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    const dBirthday_c *birthday = player != NULL ? &player->mBirthday : NULL;
    if (birthday != NULL) {
        if (now->month == birthday->mMonth && now->mday == birthday->mDay) {
            code = 0x1A;
        } else {
            dAnimal_c *animal = getAnimal();
            if (animal != NULL) {
                u8 month = birthday->mMonth;
                if (month == (u8)animal->getBirthMonth()) {
                    u8 day = birthday->mDay;
                    if (day == (u8)animal->getBirthDay()) {
                        code = 0x1B;
                    }
                }
            }
        }
    }
    setLooksMsg(info, l_Ev_Arbeit, code);
    return TRUE;
}

// 80039364
void dAcNpcNml_c::talk_c::selArbeitBirthdayNo() {
    setMsgProc(&talk_c::msgArbeitBirthdayNo);
    startMsg();
}

// 800393B8
int dAcNpcNml_c::talk_c::msgArbeitBirthdayNo(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Arbeit, 0x19);
    setStepProc(&talk_c::stepArbeitBirthdayMenu);
    return TRUE;
}

// 8003941C
BOOL dAcNpcNml_c::talk_c::stepArbeitCarpetReward(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgArbeitCarpetThanks);
        startMsg();
        requestItemAct(&mItem0, 0, 0);
        return TRUE;
    }
    return FALSE;
}

// 800394A8
int dAcNpcNml_c::talk_c::msgArbeitCarpetThanks(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Arbeit, 0x1F);
    setStepProc(&talk_c::stepArbeitErrandDone);
    return TRUE;
}

// 8003950C
BOOL dAcNpcNml_c::talk_c::stepArbeitErrandDone(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player != NULL) {
            player->mErrand.setState(2);
        }
        return TRUE;
    }
    return FALSE;
}

// 80039564
BOOL dAcNpcNml_c::talk_c::stepArbeitErrandNext(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player != NULL) {
            player->mErrand.setState(1);
        }
        return TRUE;
    }
    return FALSE;
}

// 800395BC
BOOL dAcNpcNml_c::talk_c::stepArbeitLetterMenu(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgArbeitLetter);
        startMsg();
        dAnimal_c *animal = getAnimal();
        if (animal != NULL) {
            reqMailMenu(&animal->mLetter, FALSE);
        }
        return TRUE;
    }
    return FALSE;
}

// 80039658
int dAcNpcNml_c::talk_c::msgArbeitLetter(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Arbeit, 0x21);
    setStepProc(&talk_c::stepArbeitErrandNext);
    return TRUE;
}

// 800396BC
int dAcNpcNml_c::talk_c::msgArbeitCancel(msgInfo_s *info) {
    setLooksMsg(info, l_Q_Cancel, 0);
    return TRUE;
}

// 800396EC
void dAcNpcNml_c::talk_c::selArbeitOther() {
    setMsgProc(&talk_c::msgArbeitProgress);
    startMsg();
}

// 80039740
int dAcNpcNml_c::talk_c::msgArbeitProgress(msgInfo_s *info) {
    u16 code = getArbeitProgressCode();
    setStepProc(&talk_c::stepArbeitProgressNext);
    setLooksMsg(info, l_Ev_Arbeit, code);
    setTopic(dNpc::msgMemory_c::KIND_ARBEIT, 5, 0);
    return TRUE;
}

// 800397DC
BOOL dAcNpcNml_c::talk_c::stepArbeitNewcomerChoice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoice(0, 0x65, 3, &talk_c::selArbeitProgress);
        setChoice(1, 0x68, 3, &talk_c::selArbeitNo);
        setChoiceNum(2);
        setChoiceCancel(1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 800398B4
u16 dAcNpcNml_c::talk_c::getArbeitProgressCode() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    u16 code = 6;
    if (player != NULL) {
        code = player->mNewcomerTip + 6;
    }
    return code;
}

// 800398F4
void dAcNpcNml_c::talk_c::selArbeitProgress() {
    setMsgProc(&talk_c::msgArbeitProgressCheck);
    startMsg();
}

// 80039948
int dAcNpcNml_c::talk_c::msgArbeitProgressCheck(msgInfo_s *info) {
    u16 code = 5;
    u32 home = dSaveData_c::getRaw()->mHomes.findCurrentPlayer();
    if (home < 4) {
        code = getArbeitProgressCode();
        if (code == 0xB) {
            u32 num = dPrivateData_c::count(dSaveData_c::getTown()->mPlayers);
            if (num == 4) {
                dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
                if (player != NULL) {
                    player->incNewcomerTip();
                }
                code = getArbeitProgressCode();
            }
        }
    }
    if (code != 5) {
        setStepProc(&talk_c::stepArbeitProgressNext);
    }
    setLooksMsg(info, l_Ev_Arbeit, code);
    setTopic(dNpc::msgMemory_c::KIND_ARBEIT, 5, 0);
    return TRUE;
}

// 80039A44
BOOL dAcNpcNml_c::talk_c::stepArbeitProgressNext(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player != NULL) {
            player->incNewcomerTip();
        }
        return TRUE;
    }
    return FALSE;
}

// 80039A94
void dAcNpcNml_c::talk_c::selArbeitNo() {
    setMsgProc(&talk_c::msgArbeitNo);
    startMsg();
}

// 80039AE8
int dAcNpcNml_c::talk_c::msgArbeitNo(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Arbeit, 4);
    setTopic(dNpc::msgMemory_c::KIND_ARBEIT, 5, 0);
    return TRUE;
}
