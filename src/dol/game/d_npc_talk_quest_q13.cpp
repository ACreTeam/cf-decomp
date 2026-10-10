// Villager talk for the style quest (QUEST_KIND_STYLE): the villager asks the player to look at another
// player's style, later asks what it was and rewards a right answer. .text 80058990..8005A3C4.
// See include/game/game/d_npc_talk_quest_q13.hpp.
#include <game/game/d_npc_talk_quest_q13.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_string.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

// 80058990
int dAcNpcNml_c::talk_c::msgStyleOffer(msgInfo_s *info) {
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL || player->isFlag0(0xD)) {
        return FALSE;
    }
    dSaveData_c *save = dSaveData_c::getRaw();
    if (!save->mAnimals.mTown.canStartStyle(animal, save->mPlayers, player)) {
        return FALSE;
    }
    return startQuestOffer(info, 7, &talk_c::endQuestCommon, &talk_c::stepStyleOffer, TRUE);
}

// 80058A98
BOOL dAcNpcNml_c::talk_c::stepStyleOffer(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgStyleReq);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

static const char l_Q13_Req[] = "Q13_Req"; // 80750248

// 80058B10
int dAcNpcNml_c::talk_c::msgStyleReq(msgInfo_s *info) {
    setLooksMsg(info, l_Q13_Req, 0);
    setHookProc(&talk_c::endStyleReq);
    setStepProc(&talk_c::stepStyleReq);
    return TRUE;
}

// 80058B98
void dAcNpcNml_c::talk_c::endStyleReq(int arg) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimal_c *animal = getAnimal();
    if (player != NULL && animal != NULL) {
        dSaveData_c *save = dSaveData_c::getRaw();
        mStylePlayer = save->mAnimals.mTown.pickStyleOtherPlayer(animal, save->mPlayers, player);
        if (mStylePlayer != NULL) {
            setPlayerName(mStylePlayer, 0);
            dAnimal_c *self = getAnimal();
            u16 unit = self != NULL ? fn_800F3F38(mStylePlayer->getGender(), self->mID.getLooks(1)) : 0;
            if (unit == 0) {
                clearWord(1);
            } else {
                getController()->fn_801A5874(1, unit, "sys_STRING/STR_Unit");
            }
        }
    }
}

// 80058C98
BOOL dAcNpcNml_c::talk_c::stepStyleReq(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoice(0, 0x15, 10, &talk_c::selStyleYes);
        setChoice(1, 0x1F, 10, &talk_c::selNo);
        setChoiceNum(2);
        setChoiceCancel(1);
        showChoice();
        dNpcEntry_c *entry = getEntry();
        dQuestBase_c *quest = entry != NULL ? &entry->mQuest : NULL;
        if (quest != NULL) {
            quest->clear();
        }
        return TRUE;
    }
    return FALSE;
}

// 80058D98
void dAcNpcNml_c::talk_c::selStyleYes() {
    setMsgProc(&talk_c::msgStyleYes);
    startMsg();
}

static const char l_Q13_Yes[] = "Q13_Yes"; // 80750250

// 80058DEC
int dAcNpcNml_c::talk_c::msgStyleYes(msgInfo_s *info) {
    setLooksMsg(info, l_Q13_Yes, 0);
    setHookProc(&talk_c::endStyleYes);
    return TRUE;
}

// 80058E4C
void dAcNpcNml_c::talk_c::endStyleYes(int arg) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimal_c *animal = getAnimal();
    dAnimalBlock_c *block = &dSaveData_c::getTown()->mAnimals.mTown;
    if (player != NULL && animal != NULL && mStylePlayer != NULL) {
        if (block->startStyle(&animal->mID, &player->mPID.player, mStylePlayer, dTime_c::getCurrent())) {
            dString::Word_c topic(block->mStyle.mTopicText);
            getController()->setWord(4, &topic);
        }
    }
}

// 804A431C: labels of the quest-talk states (getMsgLabel(TALK_QUEST, QUEST_TALK_STYLE, state)).
const char *l_q13Labels[7] = {l_Ai_Quest, "Q13_Ask", "Q13_Ask", "Q13_Ask2", "Q13_Item", "Q13_ItemFull", "Q13_Over"};

// 80058F18
int dAcNpcNml_c::talk_c::msgStyleQuest(msgInfo_s *info) {
    if (isEventOngoing()) {
        return FALSE;
    }
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    dAnimalBlock_c *block = dSaveData_c::getAnimalBlock();
    dAnmPersonalID_c *self = &animal->mID;
    dQuestPlayerPair_c *style = &block->mStyle;
    dQuestBase_c *quest = &style->mBase;
    if (!quest->isActive() || quest->getKind() != QUEST_KIND_STYLE) {
        return FALSE;
    }
    if (!block->isStyleAnimal(self)) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL || !player->mPID.isFromTown()) {
        return FALSE;
    }
    dPlayerID_c *me = &player->mPID.player;
    dPlayerID_c *player0 = style->getPlayer(0);
    dPlayerID_c *player1 = style->getPlayer(1);
    if (!me->isSame(player0) && !me->isSame(player1)) {
        return FALSE;
    }
    int which = !me->isSame(player0);
    u8 state = quest->getState();
    if ((state == 3 && which == 0) || (state >= 2 && which == 1)) {
        return FALSE;
    }
    if (which == 1 && quest->isPastDeadline(NULL)) {
        return FALSE;
    }
    int idx = 0;
    if (which == 0) {
        if (state == 2) {
            int pocket = player->findEmptyPocket(0);
            idx = 5;
            if (pocket != -1) {
                idx = 4;
            }
            addFriendship(5);
        } else if (quest->isPastDeadline(NULL)) {
            idx = 6;
        }
    } else {
        idx = 3;
        if (state == 0) {
            idx = 1;
        }
    }
    if (player1->isValid()) {
        setPlayerName(player1, 0);
        u8 gender = player1->getGender();
        u16 unit = fn_800F3F38(gender, animal->mID.getLooks(1));
        if (unit == 0) {
            clearWord(1);
        } else {
            getController()->fn_801A5874(1, unit, "sys_STRING/STR_Unit");
        }
    }
    if (player0->isValid()) {
        setPlayerName(player0, 2);
        u8 gender = player0->getGender();
        u16 unit = fn_800F3F38(gender, animal->mID.getLooks(1));
        if (unit == 0) {
            clearWord(3);
        } else {
            getController()->fn_801A5874(3, unit, "sys_STRING/STR_Unit");
        }
        mMsgPlayer.copy(player0);
    }
    dString::Word_c topic(style->mTopicText);
    getController()->setWord(4, &topic);
    if (idx >= 1 && idx < 4) {
        dNpcEntry_c *entry = getEntry();
        BOOL noOffer = entry != NULL ? entry->_28C.mTalked != 0 : TRUE;
        if (!noOffer) {
            return startQuestOffer(info, 9, &talk_c::endQuestCommon, &talk_c::stepStyleAsk, FALSE);
        }
        setProcSet(&l_talkEntrySets[TALK_QUEST]);
        setHookProc(&talk_c::endQuestCommon);
        setTopic(dNpc::msgMemory_c::KIND_QUEST, QUEST_TALK_STYLE, 0);
        mCountTalk = 1;
        return msgStyleAsk(info);
    }
    stepFunc step = &talk_c::stepStyleTalk;
    endFunc hook = NULL;
    switch (idx) {
    case 4:
        hook = &talk_c::endStyleItem;
        step = &talk_c::stepStyleItem;
        break;
    case 5:
        hook = &talk_c::endStyleItemFull;
        step = NULL;
        break;
    case 6:
        step = &talk_c::stepStyleOver;
        break;
    }
    const char *label = getMsgLabel(TALK_QUEST, QUEST_TALK_STYLE, idx);
    if (label != NULL) {
        setProcSet(&l_talkEntrySets[TALK_QUEST]);
        setLooksMsg(info, label, 0);
        if (hook != NULL) {
            setHookProc(hook);
        }
        setStepProc(step);
        setTopic(dNpc::msgMemory_c::KIND_QUEST, QUEST_TALK_STYLE, 0);
        return TRUE;
    }
    return FALSE;
}

// 80059504
BOOL dAcNpcNml_c::talk_c::stepStyleAsk(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgStyleAsk);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

static const char l_Q13_Ask[] = "Q13_Ask"; // 80750258

// 8005957C
int dAcNpcNml_c::talk_c::msgStyleAsk(msgInfo_s *info) {
    static const char l_Q13_Ask2[] = "Q13_Ask2"; // 8046CA90
    const char *label = l_Q13_Ask;
    if (dSaveData_c::getRaw()->mAnimals.mTown.mStyle.mBase.mState == 1) {
        label = l_Q13_Ask2;
    }
    setLooksMsg(info, label, 0);
    setStepProc(&talk_c::stepStyleAskChoice);
    return TRUE;
}

// 80059618
BOOL dAcNpcNml_c::talk_c::stepStyleAskChoice(int kind) {
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
        setChoice(n, 0x36, 3, &talk_c::selStyleAnswer);
        setChoice(n + 1, 0x39, 3, &talk_c::selStyleForget);
        setChoiceNum(n + 2);
        setChoiceCancel(n + 1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 800597EC
void dAcNpcNml_c::talk_c::selStyleAnswer() {
    reqMenu22(TRUE);
    mResultProc = &talk_c::resStyleAnswer;
}

// 80059838
void dAcNpcNml_c::talk_c::resStyleAnswer() {
    const wchar_t *topic;
    BOOL right = FALSE;
    if (!isMenuInvalid()) {
        const wchar_t *answer = (const wchar_t *)getMenuWork();
        topic = dSaveData_c::getRaw()->mAnimals.mTown.mStyle.mTopicText;
        if (answer != NULL && topic != NULL) {
            dString::Word_c answerWord(answer);
            dString::Word_c topicWord(topic);
            if (answerWord.isSame(&topicWord)) {
                setMsgProc(&talk_c::msgStyleAnswerOK);
                right = TRUE;
            }
        }
    }
    if (!right) {
        setMsgProc(&talk_c::msgStyleAnswerNG);
    }
    startMsg();
    reqMsgClose();
}

// 8005995C
int dAcNpcNml_c::talk_c::msgStyleAnswerOK(msgInfo_s *info) {
    static const char l_Q13_AnswerOK[] = "Q13_AnswerOK"; // 8046CA9C
    setLooksMsg(info, l_Q13_AnswerOK, 0);
    setStepProc(&talk_c::stepStyleAnswerOK);
    return TRUE;
}

// 800599C0
BOOL dAcNpcNml_c::talk_c::stepStyleAnswerOK(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgStyleReward);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80059A38
int dAcNpcNml_c::talk_c::msgStyleReward(msgInfo_s *info) {
    static const char l_Q13_AnswerOK[] = "Q13_AnswerOK"; // 8046CAAC
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    endFunc hook = &talk_c::endStyleRewardFull;
    stepFunc step = &talk_c::stepStyleRewardFull;
    u16 code = 10;
    if (player != NULL && player->findEmptyPocket(0) != -1) {
        hook = &talk_c::endStyleReward;
        step = &talk_c::stepStyleReward;
        code = 9;
    }
    setLooksMsg(info, l_Q13_AnswerOK, code);
    setHookProc(hook);
    setStepProc(step);
    addFriendship(3);
    return TRUE;
}

// 80059B88
void dAcNpcNml_c::talk_c::endStyleReward(int arg) {
    dAnimalBlock_c *block;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    block = &dSaveData_c::getTown()->mAnimals.mTown;
    mItem0 = block->pickStylePresent(player);
    if (mItem0.isValid()) {
        player->pickUp(&mItem0, 0);
        setItemName(&mItem0, 5);
    }
    block->mStyle.mBase.mState = 2;
}

// 80059C20
BOOL dAcNpcNml_c::talk_c::stepStyleReward(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgStyleThank);
        startMsg();
        if (mItem0.isValid()) {
            requestItemAct(&mItem0, 0, 0);
        }
        return TRUE;
    }
    return FALSE;
}

// 80059CB8
int dAcNpcNml_c::talk_c::msgStyleThank(msgInfo_s *info) {
    static const char l_Q13_Thank[] = "Q13_Thank"; // 8046CABC
    setLooksMsg(info, l_Q13_Thank, 0);
    return TRUE;
}

// 80059CE8
void dAcNpcNml_c::talk_c::endStyleRewardFull(int arg) {
    dAnimalBlock_c *block = &dSaveData_c::getTown()->mAnimals.mTown;
    dQuestPlayerPair_c *style = &block->mStyle;
    if (!block->sendStyleLetterTo(1, FALSE)) {
        style->_5E |= 2;
    }
    style->mBase.mState = 2;
}

// 80059D44
BOOL dAcNpcNml_c::talk_c::stepStyleRewardFull(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgStyleThank);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80059DBC
int dAcNpcNml_c::talk_c::msgStyleAnswerNG(msgInfo_s *info) {
    static const char l_Q13_AnswerNG[] = "Q13_AnswerNG"; // 8046CAC8
    setLooksMsg(info, l_Q13_AnswerNG, 0);
    setStepProc(&talk_c::stepStyleAnswerNG);
    return TRUE;
}

// 80059E20
BOOL dAcNpcNml_c::talk_c::stepStyleAnswerNG(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dQuestBase_c *quest = &dSaveData_c::getTown()->mAnimals.mTown.mStyle.mBase;
        quest->mState = 1;
        return TRUE;
    }
    return FALSE;
}

// 80059E70
void dAcNpcNml_c::talk_c::selStyleForget() {
    setMsgProc(&talk_c::msgStyleForget);
    startMsg();
}

// 80059EC4
int dAcNpcNml_c::talk_c::msgStyleForget(msgInfo_s *info) {
    static const char l_Q13_Forget[] = "Q13_Forget"; // 8046CAD8
    setLooksMsg(info, l_Q13_Forget, 0);
    return TRUE;
}

// 80059EF4
BOOL dAcNpcNml_c::talk_c::stepStyleTalk(int kind) {
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
        setChoice(n, 1, 3, &talk_c::selStyleCon);
        setChoice(n + 1, 4, 3, &talk_c::selStyleResume);
        setChoiceNum(n + 2);
        setChoiceCancel(n + 1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 8005A0C8
void dAcNpcNml_c::talk_c::selStyleCon() {
    setMsgProc(&talk_c::msgStyleCon);
    startMsg();
}

static const char l_Q13_Con[] = "Q13_Con"; // 80750260

// 8005A11C
int dAcNpcNml_c::talk_c::msgStyleCon(msgInfo_s *info) {
    setLooksMsg(info, l_Q13_Con, 0);
    return TRUE;
}

// 8005A148
void dAcNpcNml_c::talk_c::selStyleResume() {
    const procSet_s *set = getEventProcSet();
    if (set == NULL) {
        set = &l_talkEntrySets[TALK_FREE];
    }
    setProcSet(set);
    startMsg();
}

// 8005A19C
BOOL dAcNpcNml_c::talk_c::stepStyleOver(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dSaveData_c::getTown()->mAnimals.mTown.mStyle.clearInfo();
        return TRUE;
    }
    return FALSE;
}

// 8005A1EC
void dAcNpcNml_c::talk_c::endStyleItem(int arg) {
    endQuestTalk(arg);
    dAnimalBlock_c *block;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    block = &dSaveData_c::getTown()->mAnimals.mTown;
    mItem0 = block->pickStylePresent(player);
    if (mItem0.isValid()) {
        player->pickUp(&mItem0, 0);
        setItemName(&mItem0, 5);
    }
    dQuestPlayerPair_c *style = &block->mStyle;
    if ((style->_5E >> 1) & 1) {
        style->mBase.mState = 3;
    } else {
        style->clearInfo();
    }
}

// 8005A2A0
BOOL dAcNpcNml_c::talk_c::stepStyleItem(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgStyleEnd);
        startMsg();
        if (mItem0.isValid()) {
            requestItemAct(&mItem0, 0, 0);
        }
        return TRUE;
    }
    return FALSE;
}

static const char l_Q13_End[] = "Q13_End"; // 80750268

// 8005A338
int dAcNpcNml_c::talk_c::msgStyleEnd(msgInfo_s *info) {
    setLooksMsg(info, l_Q13_End, 0);
    return TRUE;
}

// 8005A364
void dAcNpcNml_c::talk_c::endStyleItemFull(int arg) {
    endQuestTalk(arg);
    dAnimalBlock_c *block = &dSaveData_c::getTown()->mAnimals.mTown;
    dQuestPlayerPair_c *style = &block->mStyle;
    if (!block->sendStyleLetterTo(0, FALSE)) {
        style->_5E |= 1;
    }
    style->mBase.mState = 3;
}
