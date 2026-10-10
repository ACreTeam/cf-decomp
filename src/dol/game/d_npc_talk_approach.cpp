// The remarks a villager makes when it walks up to the player ("Ap*" messages).
// .text 80036324..80038134. See include/game/game/d_npc_talk_approach.hpp.
#include <game/game/d_npc_talk_approach.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_hr.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_data.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

// Labels of getApproachLabel by group (group 2 has none).
static const char *l_apDLabels[4] = {"ApD_Moving", "ApD_Fortune", "ApD_Fortune", "ApD_Fortune"};
static const char *l_apBCLabels[7] = {
    "ApB_Habit", "ApB_Hello", "ApB_Nickname", "ApC_Present", "ApC_Sell", "ApC_Trade", "ApC_Want",
};
static const char *l_apALabels[2] = {"ApA_Letter", "ApA_Always"};

// 80036324
const char *dAcNpcNml_c::talk_c::getApproachLabel(int group, u32 idx, int unused) {
    const char *label = NULL;
    switch (group) {
    case 0:
        if (idx < 4) {
            label = l_apDLabels[idx];
        }
        break;
    case 1:
        if (idx < 7) {
            label = l_apBCLabels[idx];
        }
        break;
    case 3:
        if (idx < 2) {
            label = l_apALabels[idx];
        }
        break;
    }
    return label;
}

// 800363A4
int dAcNpcNml_c::talk_c::reqApMoveOut() {
    dAnimalBlock_c *block = &dSaveData_c::getTown()->mAnimals.mTown;
    int idx = block->mMoveOutIdx;
    if ((u32)idx < 10) {
        dAnimal_c *animal = block->getAnimal(idx);
        block->mMoveOutIdx = -1;
        block->mMoveDays = 0;
        block->mMoveInIdx = idx;
        if (animal != NULL && animal->mID.isValid()) {
            animal->clearMemoryFlag23();
        }
    }
    return FALSE;
}

// 8003643C
int dAcNpcNml_c::talk_c::msgApMoving(msgInfo_s *info) {
    dAnimal_c *animal = getAnimal();
    BOOL ok = FALSE;
    if (animal != NULL && mpMemory != NULL && !mpMemory->mFlags.mMoveOutTold) {
        ok = TRUE;
    }
    if (ok && !dSaveData_c::getRaw()->mAnimals.mTown.isMoveOutAnimal(&animal->mID)) {
        ok = FALSE;
    }
    if (ok) {
        const char *label = getApproachLabel(0, 0, 0);
        if (label != NULL) {
            setProcSet(&l_talkProcSets[TALK_PROC_APPROACH]);
            if (mpMemory != NULL) {
                mpMemory->mFlags.mMoveOutTold = 1;
            }
            setLooksMsg(info, label, 0);
            mReqProc[0] = &talk_c::reqApMoveOut;
            return TRUE;
        }
    }
    return FALSE;
}

// 80036564
int dAcNpcNml_c::talk_c::msgApFortuneLove(msgInfo_s *info) {
    dAnimal_c *animal = getAnimal();
    BOOL ok = FALSE;
    if (animal != NULL && mpMemory != NULL && !mpMemory->mFlags.mFortuneTold) {
        ok = TRUE;
    }
    if (ok) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player == NULL || player->mFortune != FORTUNE_LOVE || animal->mID.getGender(1) == player->mPID.player.getGender()) {
            ok = FALSE;
        }
    }
    if (ok) {
        const char *label = getApproachLabel(0, 1, 0);
        if (label != NULL) {
            setProcSet(&l_talkProcSets[TALK_PROC_APPROACH]);
            setStepProc(&talk_c::stepApFortune);
            setLooksMsg(info, label, cM::rndInt(3) + 1);
            return TRUE;
        }
    }
    return FALSE;
}

// 80036698
int dAcNpcNml_c::talk_c::msgApFortuneFriend(msgInfo_s *info) {
    dAnimal_c *animal = getAnimal();
    BOOL ok = FALSE;
    if (animal != NULL && mpMemory != NULL && !mpMemory->mFlags.mFortuneTold) {
        ok = TRUE;
    }
    if (ok) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player == NULL || player->mFortune != FORTUNE_FRIENDSHIP || animal->mID.getGender(1) != player->mPID.player.getGender()) {
            ok = FALSE;
        }
    }
    if (ok) {
        const char *label = getApproachLabel(0, 2, 0);
        if (label != NULL) {
            setProcSet(&l_talkProcSets[TALK_PROC_APPROACH]);
            setStepProc(&talk_c::stepApFortune);
            setLooksMsg(info, label, cM::rndInt(3) + 4);
            return TRUE;
        }
    }
    return FALSE;
}

// 800367CC
int dAcNpcNml_c::talk_c::msgApFortuneItem(msgInfo_s *info) {
    BOOL ok = FALSE;
    if (mpMemory != NULL && !mpMemory->mFlags.mFortuneTold) {
        ok = TRUE;
    }
    if (ok) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player == NULL || player->mFortune != FORTUNE_ITEM) {
            ok = FALSE;
        }
    }
    if (ok) {
        const char *label = getApproachLabel(0, 3, 0);
        if (label != NULL) {
            setProcSet(&l_talkProcSets[TALK_PROC_APPROACH]);
            setHookProc(&talk_c::endApFortuneItem);
            setLooksMsg(info, label, cM::rndInt(3) + 7);
            return TRUE;
        }
    }
    return FALSE;
}

// 800368DC
int dAcNpcNml_c::talk_c::msgApD(msgInfo_s *info) {
    static const msgFunc l_checks[4] = {
        &talk_c::msgApMoving,
        &talk_c::msgApFortuneLove,
        &talk_c::msgApFortuneFriend,
        &talk_c::msgApFortuneItem,
    };
    for (int i = 0; i < 4; i++) {
        if (l_checks[i] && (this->*l_checks[i])(info)) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80036974
int dAcNpcNml_c::talk_c::msgApHabit(msgInfo_s *info) {
    dAnimal_c *animal = getAnimal();
    BOOL ok = FALSE;
    if (animal != NULL && animal->mHabitCooldown == 0) {
        ok = TRUE;
    }
    if (ok) {
        const char *label = getApproachLabel(1, 0, 0);
        if (label != NULL) {
            setProcSet(&l_talkProcSets[TALK_PROC_APPROACH]);
            setLooksMsg(info, label, 0);
            return TRUE;
        }
    }
    return FALSE;
}

// 80036A28
int dAcNpcNml_c::talk_c::msgApHello(msgInfo_s *info) {
    BOOL ok = FALSE;
    if (mpMemory != NULL && mpMemory->mFlags.mGreetingWait == 0) {
        ok = TRUE;
    }
    if (ok) {
        const char *label = getApproachLabel(1, 1, 0);
        if (label != NULL) {
            setProcSet(&l_talkProcSets[TALK_PROC_APPROACH]);
            setLooksMsg(info, label, 0);
            return TRUE;
        }
    }
    return FALSE;
}

// 80036ADC
int dAcNpcNml_c::talk_c::msgApNickname(msgInfo_s *info) {
    BOOL ok = FALSE;
    if (!isNicknameWaiting() && mpMemory->getFriendship() > 0) {
        ok = TRUE;
    }
    if (ok) {
        const char *label = getApproachLabel(1, 2, 0);
        if (label != NULL) {
            if (getController() != NULL) {
                dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
                dAnimal_c *animal = getAnimal();
                if (player != NULL && animal != NULL) {
                    dHmnName::Word_c name;
                    player->mPID.setWord(&name);
                    fn_801A309C(getController(), &name, animal->mID.getLooks(1));
                }
            }
            setProcSet(&l_talkProcSets[TALK_PROC_APPROACH]);
            setStepProc(&talk_c::stepApNickname);
            setLooksMsg(info, label, 0);
            return TRUE;
        }
    }
    return FALSE;
}

// 80036C28
int dAcNpcNml_c::talk_c::msgApPresent(msgInfo_s *info) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    BOOL ok = FALSE;
    if (player != NULL && player->findEmptyPocket(0) != -1) {
        ok = TRUE;
    }
    if (ok) {
        const char *label = getApproachLabel(1, 3, 0);
        if (label != NULL) {
            setProcSet(&l_talkProcSets[TALK_PROC_APPROACH]);
            setHookProc(&talk_c::endApPresent);
            setLooksMsg(info, label, 0);
            return TRUE;
        }
    }
    return FALSE;
}

// 80036D08
int dAcNpcNml_c::talk_c::msgApSell(msgInfo_s *info) {
    BOOL ok;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    ok = FALSE;
    if (player != NULL && player->findEmptyPocket(0) != -1 && player->getPocketMoney() >= 2000) {
        ok = TRUE;
    }
    if (ok) {
        const char *label = getApproachLabel(1, 4, 0);
        if (label != NULL) {
            setProcSet(&l_talkProcSets[TALK_PROC_APPROACH]);
            setHookProc(&talk_c::endApSell);
            setLooksMsg(info, label, 0);
            return TRUE;
        }
    }
    return FALSE;
}

// 80036E04
dItem::Item dAcNpcNml_c::talk_c::pickWantedPocketItem(const dAcNpcNml_c::talk_c *talk, dPrivateData_c *player) {
    // Preferred item kind by the villager's wish (dQuestWish_c::mKind).
    static const int l_wishKind[8] = {
        dItem::KIND_INSECT, dItem::KIND_FISH, dItem::KIND_FOSSIL, dItem::KIND_CLOTH,
        dItem::KIND_FTR,    dItem::KIND_COUNT, dItem::KIND_COUNT, dItem::KIND_COUNT,
    };
    dItem::Item result;
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player == NULL) {
        return result;
    }
    dAnimal_c *animal = talk->getAnimal();
    if (animal == NULL) {
        return result;
    }
    dQuestWish_c *wish = animal->getWish();
    int want = dItem::KIND_COUNT;
    if (wish->isValid()) {
        u32 wishKind = wish->mKind;
        if (wishKind < 8) {
            want = l_wishKind[wishKind];
        }
    }
    BOOL found = FALSE;
    u32 count = 0;
    dItem::Item *pockets = player->mPockets;
    for (int i = 0; i < 15; i++) {
        dItem::Item item = pockets[i];
        if (item.isValid() && !player->getPocketFlag(i) && !animal->isRequestedItem(&item)) {
            const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
            BOOL ok = FALSE;
            if (bitm != NULL && !bitm->m_noPurchase) {
                int kind = bitm->getKind();
                if (!found) {
                    if (kind == want) {
                        ok = TRUE;
                        count = 0;
                        found = TRUE;
                    } else {
                        switch (kind) {
                        case dItem::KIND_WALL:
                        case dItem::KIND_CARPET:
                        case dItem::KIND_FTR:
                        case dItem::KIND_CLOTH:
                        case dItem::KIND_INSECT:
                        case dItem::KIND_FISH:
                        case dItem::KIND_FOSSIL:
                            ok = TRUE;
                            break;
                        }
                    }
                } else if (kind != dItem::KIND_COUNT && kind == want) {
                    ok = TRUE;
                }
                if (ok) {
                    f32 rate = 100.0f / (count + 1);
                    if (cM::rndF(100.0f) <= rate) {
                        result = item;
                    }
                    count++;
                }
            }
        }
    }
    return result;
}

// 80037038
int dAcNpcNml_c::talk_c::msgApTrade(msgInfo_s *info) {
    dItem::Item item = pickWantedPocketItem(this, NULL);
    if (item.isValid()) {
        const char *label = getApproachLabel(1, 5, 0);
        if (label != NULL) {
            setProcSet(&l_talkProcSets[TALK_PROC_APPROACH]);
            setHookProc(&talk_c::endApTrade);
            setLooksMsg(info, label, 0);
            return TRUE;
        }
    }
    return FALSE;
}

// 80037108
int dAcNpcNml_c::talk_c::msgApWant(msgInfo_s *info) {
    dItem::Item item = pickWantedPocketItem(this, NULL);
    if (item.isValid()) {
        const char *label = getApproachLabel(1, 6, 0);
        if (label != NULL) {
            setProcSet(&l_talkProcSets[TALK_PROC_APPROACH]);
            setHookProc(&talk_c::endApWant);
            setLooksMsg(info, label, 0);
            return TRUE;
        }
    }
    return FALSE;
}

// 800371D8
int dAcNpcNml_c::talk_c::msgApBC(msgInfo_s *info) {
    static const msgFunc l_checks[7] = {
        &talk_c::msgApHabit, &talk_c::msgApHello, &talk_c::msgApNickname, &talk_c::msgApPresent,
        &talk_c::msgApSell, &talk_c::msgApTrade, &talk_c::msgApWant,
    };
    u32 mask = 0;
    for (int i = 0; i < 7; i++) {
        mask |= 1 << i;
    }
    int num = 7;
    for (int tries = 3; tries != 0 && num != 0 && mask != 0; tries--) {
        int pick = cM::rndInt(num);
        for (int i = 0; i < 7; i++) {
            if ((mask >> i) & 1) {
                if (pick == 0) {
                    if (l_checks[i] && (this->*l_checks[i])(info)) {
                        return TRUE;
                    }
                    num &= ~(1 << i);
                    num--;
                    break;
                }
                pick--;
            }
        }
    }
    return FALSE;
}

// 800372CC
int dAcNpcNml_c::talk_c::msgApPendingQuest(msgInfo_s *info) {
    return msgPendingQuest(info, 25.0f);
}

// 800372D4
int dAcNpcNml_c::talk_c::msgApLetter(msgInfo_s *info) {
    BOOL ok;
    dPrivateData_c *const current = dPlayerMgr_c::getCurrentPlayer();
    dPrivateData_c *player = current;
    dAnimal_c *animal = getAnimal();
    ok = FALSE;
    if (animal != NULL && animal->hasAnyLetter() && player != NULL && player->mPID.isValid() &&
        !animal->hasLetterFrom(&player->mPID)) {
        ok = TRUE;
    }
    if (ok) {
        const char *label = getApproachLabel(3, 0, 0);
        if (label != NULL) {
            setProcSet(&l_talkProcSets[TALK_PROC_APPROACH]);
            setLooksMsg(info, label, 0);
            return TRUE;
        }
    }
    return FALSE;
}

// 800373BC
int dAcNpcNml_c::talk_c::msgApAlways(msgInfo_s *info) {
    const char *label = getApproachLabel(3, 1, 0);
    if (label != NULL) {
        setProcSet(&l_talkProcSets[TALK_PROC_APPROACH]);
        setLooksMsg(info, label, 0);
        return TRUE;
    }
    return FALSE;
}

// 80037448
int dAcNpcNml_c::talk_c::msgApA(msgInfo_s *info) {
    static const msgFunc l_checks[2] = {
        &talk_c::msgApLetter,
        &talk_c::msgApAlways,
    };
    const msgFunc *check = &l_checks[cM::rndInt(2)];
    if (*check && (this->*(*check))(info)) {
        return TRUE;
    }
    return msgApAlways(info);
}

// 800374DC
int dAcNpcNml_c::talk_c::msgApproach(msgInfo_s *info) {
    static const msgFunc l_checks[4] = {
        &talk_c::msgApD,
        &talk_c::msgApBC,
        &talk_c::msgApPendingQuest,
        &talk_c::msgApA,
    };
    if (isSpeakerItchy()) {
        return msgApAlways(info);
    }
    for (int i = 0; i < 4; i++) {
        if (l_checks[i] && (this->*l_checks[i])(info)) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8003759C
void dAcNpcNml_c::talk_c::endApproach(int arg) {
    recordTalk(NULL);
    dNpc::msgMemorySecond_c *mem = getRememberedMsg();
    if (mem != NULL) {
        mem->clear();
    }
    if (mpMemory != NULL) {
        mpMemory->mTalkCount.inc(0x44);
    }
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->mMsg.set(mMessageLabel, mMessageCode, mTopicKind, mTopicGroup, mTopicIdx);
    }
}

// 8003762C
BOOL dAcNpcNml_c::talk_c::stepApFortune(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        if (mpMemory != NULL) {
            mpMemory->mFlags.mFortuneTold = 1;
        }
        return TRUE;
    }
    return FALSE;
}

// 8003766C
void dAcNpcNml_c::talk_c::endApFortuneItem(int arg) {
    endApproach(arg);
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    int slot = player != NULL ? player->findEmptyPocket(0) : -1;
    mItem0 = dItem::Item();
    if (!fn_800F4510(&mItem0, 1, lbl_8059FF80, NULL, 0)) {
        mItem0 = dItem::Item();
    }
    if (mItem0.isValid()) {
        setItemName(&mItem0, 0);
        mGiveFlag = 1;
    }
    if ((u32)slot < 15) {
        setMsgProc(&talk_c::msgApFortuneGift);
        startMsg();
    } else {
        setMsgProc(&talk_c::msgApFortuneFull);
        startMsg();
    }
    if (mpMemory != NULL) {
        mpMemory->mFlags.mFortuneTold = 1;
    }
}

// 800377A8
int dAcNpcNml_c::talk_c::msgApFortuneGift(msgInfo_s *info) {
    const char *label = getApproachLabel(0, 3, 0);
    if (label != NULL) {
        setLooksMsg(info, label, mMessageCode + 3);
        return TRUE;
    }
    return FALSE;
}

// 8003781C
int dAcNpcNml_c::talk_c::msgApFortuneFull(msgInfo_s *info) {
    const char *label = getApproachLabel(0, 3, 0);
    if (label != NULL) {
        setLooksMsg(info, label, 13);
        return TRUE;
    }
    return FALSE;
}

// 80037888
BOOL dAcNpcNml_c::talk_c::stepApNickname(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoiceProc(0, &talk_c::selApNicknameNew);
        setChoiceNum(2);
        setChoiceCancel(1);
        return TRUE;
    }
    return FALSE;
}

// 8003791C
void dAcNpcNml_c::talk_c::selApNicknameNew() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimal_c *animal = getAnimal();
    if (player != NULL && animal != NULL && mpMemory != NULL) {
        mpMemory->getNickname(&mName);
        dHmnName::Word_c nickname;
        u8 gender = player->mPID.player.getGender();
        fn_8011F19C(&nickname, &mName, gender, animal->mID.getLooks(1));
        mHadNickname = mpMemory->mFlags.mHasNickname;
        mpMemory->setNickname(&nickname);
        setStepProc(&talk_c::stepApNicknameChoice);
    }
}

// 80037A08
BOOL dAcNpcNml_c::talk_c::stepApNicknameChoice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoiceProc(0, &talk_c::selApNicknameKeep);
        setChoiceProc(1, &talk_c::selApNicknameUndo);
        setChoiceNum(2);
        setChoiceCancel(1);
        return TRUE;
    }
    return FALSE;
}

// 80037AC8
void dAcNpcNml_c::talk_c::selApNicknameKeep() {
    if (mpMemory != NULL) {
        mpMemory->mFlags.mNicknameWait = 14;
    }
}

// 80037AE8
void dAcNpcNml_c::talk_c::selApNicknameUndo() {
    if (mpMemory != NULL && mpMemory->getFriendship() <= 63) {
        mpMemory->setNickname(&mName);
        if (!mHadNickname) {
            mpMemory->mFlags.mHasNickname = 0;
        }
    }
}

// 80037B58
void dAcNpcNml_c::talk_c::endApPresent(int arg) {
    endApproach(arg);
    mItem0 = dItem::Item();
    if (!fn_800F4510(&mItem0, 1, lbl_8059FF80, NULL, 0)) {
        mItem0 = dItem::Item();
    }
    if (mItem0.isValid()) {
        setItemName(&mItem0, 0);
    }
}

// 80037BE0
void dAcNpcNml_c::talk_c::endApSell(int arg) {
    endApproach(arg);
    mItem0 = dItem::Item();
    dAnimal_c *animal = getAnimal();
    dItem::Item *pick = animal != NULL ? animal->pickNewItem(NULL, 0, FALSE) : NULL;
    mIsAnimalItem = 0;
    if (pick != NULL) {
        mItem0 = *pick;
    }
    if (!mItem0.isValid()) {
        if (!fn_800F4510(&mItem0, 1, lbl_8059FF80, NULL, 0)) {
            mItem0 = dItem::Item();
        }
    } else {
        mIsAnimalItem = 1;
    }
    mPrice = 10;
    if (mItem0.isValid()) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player != NULL) {
            f32 price = mItem0.getPrice();
            price *= 0.8f + cM::rndF(0.8f);
            u32 rating = 0;
            u32 home = dSaveData_c::getTown()->mHomes.findOwner(player);
            if (home < 4) {
                const u8 value = *dHR::getRate(home);
                rating = value;
            }
            rating *= 3;
            if (price > rating) {
                mPrice = (int)price - rating;
            }
            int money = player->getPocketMoney();
            if (money < mPrice) {
                mPrice = money * (0.5f + cM::rndF(0.3f));
                if (mPrice < 10) {
                    mPrice = 10;
                }
            }
        }
        setItemName(&mItem0, 0);
        setNumber(mPrice, 0, 12, dScript::NUM_FORMAT_REGION);
    }
}

// 80037E10
void dAcNpcNml_c::talk_c::endApTrade(int arg) {
    endApproach(arg);
    mItem0 = dItem::Item();
    dAnimal_c *animal = getAnimal();
    dItem::Item *pick = animal != NULL ? animal->pickNewItem(NULL, 0, FALSE) : NULL;
    mIsAnimalItem = 0;
    if (pick != NULL) {
        mItem0 = *pick;
    }
    if (!mItem0.isValid()) {
        if (!fn_800F4510(&mItem0, 1, lbl_8059FF80, NULL, 0)) {
            mItem0 = dItem::Item();
        }
    } else {
        mIsAnimalItem = 1;
    }
    mItem2 = pickWantedPocketItem(this, NULL);
    if (mItem0.isValid() && mItem2.isValid()) {
        setItemName(&mItem2, 0);
        setItemName(&mItem0, 1);
    }
}

// 80037F1C
void dAcNpcNml_c::talk_c::endApWant(int arg) {
    endApproach(arg);
    mItem0 = pickWantedPocketItem(this, NULL);
    mPrice = 10;
    if (mItem0.isValid()) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player != NULL) {
            f32 price = 0.25f * mItem0.getPrice();
            price *= 0.8f + cM::rndF(0.8f);
            u32 rating = 0;
            u32 home = dSaveData_c::getTown()->mHomes.findOwner(player);
            if (home < 4) {
                const u8 value = *dHR::getRate(home);
                rating = value;
            }
            mPrice = (int)price + rating * 3;
            if (mPrice < 10) {
                mPrice = 10;
            }
            int room = player->getMoneyRoom(1);
            if (mPrice > room) {
                mPrice = room;
            }
        }
        setItemName(&mItem0, 0);
        setNumber(mPrice, 0, 12, dScript::NUM_FORMAT_REGION);
    }
}

// 80038078
BOOL dAcNpcNml_c::talk_c::isNicknameWaiting() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        const dPersonalID_c *pid = &player->mPID;
        dAnimalBlock_c *block = &dSaveData_c::getRaw()->mAnimals.mTown;
        for (int i = 0; i < 10; i++) {
            dAnimal_c *animal = block->getAnimalConst(i);
            if (animal != NULL && animal->mID.isValid()) {
                dAnimalMemory_c *memory = animal->findMemory2(pid);
                if (memory != NULL && memory->mFlags.mNicknameWait) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}
