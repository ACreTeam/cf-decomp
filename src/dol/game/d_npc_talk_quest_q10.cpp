// Villager talk for hide and seek (QUEST_KIND_HIDE_AND_SEEK): the offer, the game start, the hiders' talks,
// the reward and the losing end. .text 800573D0..80058990. See include/game/game/d_npc_talk_quest_q10.hpp.
#include <game/game/d_npc_talk_quest_q10.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_weather.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_field_assessment.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

// The weather manager's mode is the given one (as d_a_npc_nml's msgGreeting).
static inline BOOL isWeatherIn(int mode, int want) {
    return mode == want;
}

static inline BOOL isRainMode(int mode) {
    BOOL rain = FALSE;
    if (isWeatherIn(mode, 4) || isWeatherIn(mode, 3)) {
        rain = TRUE;
    }
    return rain;
}

static inline BOOL isSnowMode(int mode) {
    BOOL snow = FALSE;
    if (isWeatherIn(mode, 6) || isWeatherIn(mode, 5)) {
        snow = TRUE;
    }
    return snow;
}

// 800573D0
int dAcNpcNml_c::talk_c::msgHideOffer(msgInfo_s *info) {
    if (!isCurrentSceneAttr(SCENE_ATTR_TOWN)) {
        return FALSE;
    }
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL || player->isFlag0(0xD)) {
        return FALSE;
    }
    if (fn_800DCEDC()) {
        return FALSE;
    }
    dUnk8074EBE8_c *weather = lbl_8074EBE8;
    if (weather != NULL) {
        int mode = weather->_5884;
        if (isRainMode(mode) || isSnowMode(mode)) {
            return FALSE;
        }
    }
    dTime_c *now = dTime_c::getCurrent();
    if (now->hour < 6 || now->hour >= 22) {
        return FALSE;
    }
    dAnimalBlock_c *block = dSaveData_c::getAnimalBlock();
    if (!block->canStartHideAndSeek(player)) {
        return FALSE;
    }
    if (!block->canAskHideAndSeekById(&animal->mID)) {
        return FALSE;
    }
    if (!block->countHiders(&animal->mID)) {
        return FALSE;
    }
    mStartDay = now->mday;
    return startQuestOffer(info, 10, &talk_c::endQuestCommon, &talk_c::stepHideOffer, TRUE);
}

// 800575AC
BOOL dAcNpcNml_c::talk_c::stepHideOffer(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgHideReq);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

static const char l_Q10_Req[] = "Q10_Req"; // 80750220

// 80057624
int dAcNpcNml_c::talk_c::msgHideReq(msgInfo_s *info) {
    setLooksMsg(info, l_Q10_Req, 0);
    setStepProc(&talk_c::stepHideReq);
    return TRUE;
}

// 80057684
BOOL dAcNpcNml_c::talk_c::stepHideReq(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoiceProc(0, &talk_c::selHideYes);
        setChoiceProc(1, &talk_c::selHideNo);
        setChoiceNum(2);
        setChoiceCancel(1);
        dNpcEntry_c *entry = getEntry();
        dQuestBase_c *quest = entry != NULL ? &entry->mQuest : NULL;
        if (quest != NULL) {
            quest->clear();
        }
        return TRUE;
    }
    return FALSE;
}

// 8005776C
void dAcNpcNml_c::talk_c::selHideYes() {
    setMsgProc(&talk_c::msgHideOK);
    startMsg();
}

static const char l_Q10_OK[] = "Q10_OK"; // 80750228

// 800577C0
int dAcNpcNml_c::talk_c::msgHideOK(msgInfo_s *info) {
    BOOL today = TRUE;
    dTime_c *now = dTime_c::getCurrent();
    if (mStartDay != now->mday) {
        today = FALSE;
    }
    if (today) {
        setLooksMsg(info, l_Q10_OK, 0);
        setHookProc(&talk_c::endHideOK);
        setStepProc(&talk_c::stepHideOK);
    } else {
        setLooksMsg(info, l_Q10_OK, 4);
    }
    return TRUE;
}

// 800578A0
void dAcNpcNml_c::talk_c::endHideOK(int arg) {
    dAnimal_c *animal = getAnimal();
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (animal != NULL && player != NULL) {
        dSaveTown_c *town = dSaveData_c::getTown();
        town->mAnimals.mTown.startHideAndSeek(&player->mPID.player, &animal->mID, dTime_c::getCurrent());
    }
}

// 80057918
BOOL dAcNpcNml_c::talk_c::stepHideOK(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setRequest1(0);
        mResultProc = &talk_c::resHideSetup;
        return TRUE;
    }
    return FALSE;
}

// 80057988
void dAcNpcNml_c::talk_c::resHideSetup() {
    fn_800FC598();
    s16 angle;
    int state;
    mVec3_c pos;
    fn_800FE688(&state, &pos, &angle);
    fn_801A8134(&pos, -1, 6);
    getSceneChange()->setReturnExit(0, &pos, state, 0);
    getSceneChange()->requestExit(0, 2, 2);
    fn_801A6748(0);
    fgMngProc_blockDayChange();
    dBgm::l_mgr.mStgField.set3A();
}

// 80057A10
void dAcNpcNml_c::talk_c::selHideNo() {
    setMsgProc(&talk_c::msgHideNo);
    startMsg();
}

static const char l_Q10_No[] = "Q10_No"; // 80750230

// 80057A64
int dAcNpcNml_c::talk_c::msgHideNo(msgInfo_s *info) {
    setLooksMsg(info, l_Q10_No, 0);
    return TRUE;
}

// 80749AB0: label of the lost game (getMsgLabel(TALK_QUEST, QUEST_TALK_HIDE_AND_SEEK, 0)).
const char *l_q10Labels[2] = {"Q10_Badend", NULL};

// 80057A90
int dAcNpcNml_c::talk_c::msgHideQuest(msgInfo_s *info) {
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    dAnimalBlock_c *block = dSaveData_c::getAnimalBlock();
    dQuestBase_c *quest;
    dQuestPlayerAnimal_c *game;
    dAnmPersonalID_c *id = &animal->mID;
    game = &block->mHideAndSeek;
    quest = &game->mBase;
    if (!quest->isActive() || quest->getKind() != QUEST_KIND_HIDE_AND_SEEK || quest->mState == 4) {
        return FALSE;
    }
    if (block->getHiderSlot(id) == 3) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL || !player->mPID.isValid() || !player->mPID.isFromTown()) {
        return FALSE;
    }
    if (!player->mPID.player.isSame(&game->mPlayer)) {
        return FALSE;
    }
    int ret = FALSE;
    if (!game->isTimeUp(*dTime_c::getCurrent(), FALSE)) {
        ret = msgHideHider(info);
    }
    if (!ret) {
        const char *label = getMsgLabel(TALK_QUEST, QUEST_TALK_HIDE_AND_SEEK, 0);
        if (label != NULL) {
            setProcSet(&l_talkEntrySets[TALK_QUEST]);
            setLooksMsg(info, label, 0);
            setStepProc(&talk_c::stepHideBadend);
            setTopic(dNpc::msgMemory_c::KIND_QUEST, QUEST_TALK_HIDE_AND_SEEK, 0);
            ret = TRUE;
            addHiderFriendship(-5);
        }
    }
    return ret;
}

// 80057C64
BOOL dAcNpcNml_c::talk_c::stepHideBadend(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dSaveData_c::getTown()->mAnimals.mTown.mHideAndSeek.clearInfo();
        fn_800F0650();
        return TRUE;
    }
    return FALSE;
}

static const char l_Q10_Explain[] = "Q10_Explain"; // 8046CA18

// 80057CB8
int dAcNpcNml_c::talk_c::msgHideExplain(msgInfo_s *info) {
    dQuestPlayerAnimal_c *game;
    int code = 1;
    game = &dSaveData_c::getRaw()->mAnimals.mTown.mHideAndSeek;
    u32 left = game->countUnfound();
    if (left == 3) {
        code = 2;
    }
    setLooksMsg(info, l_Q10_Explain, code);
    formatNumber(game->mMinutes, 0, 12, dScript::NUM_FORMAT_REGION);
    formatNumber(left, 1, 12, dScript::NUM_FORMAT_REGION);
    setHookProc(&talk_c::endQuestCommon);
    setStepProc(&talk_c::stepHideExplain);
    return TRUE;
}

// 80057DB0
BOOL dAcNpcNml_c::talk_c::stepHideExplain(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setRequest1(0);
        mResultProc = &talk_c::resHideStart;
        return TRUE;
    }
    return FALSE;
}

// 80057E20
void dAcNpcNml_c::talk_c::resHideStart() {
    fn_800FC604();
    getSceneChange()->setReturnExitHere(0.0f);
    getSceneChange()->requestExit(0, 2, 2);
    fn_801A6748(1);
    fgMngProc_unblockDayChange();
    dQuestPlayerAnimal_c *game = &dSaveData_c::getTown()->mAnimals.mTown.mHideAndSeek;
    game->mBase.mState = 1;
    game->mBase.setTimeLimit(*dTime_c::getCurrent());
    fn_801AD168();
    dBgm::l_mgr.mStgField.set3A();
}

static const char l_Q10_Find[] = "Q10_Find";   // 8046CA24
static const char l_Q10_Over[] = "Q10_Over";   // 8046CA30
static const char l_Q10_Wait[] = "Q10_Wait";   // 8046CA3C
static const char l_Q10_Visit[] = "Q10_Visit"; // 8046CA48

// 80057EA8
int dAcNpcNml_c::talk_c::msgHideHider(msgInfo_s *info) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    BOOL fromTown = player != NULL ? player->mPID.isFromTown() : FALSE;
    const char *label;
    stepFunc step = NULL;
    dAnimal_c *animal = getAnimal();
    u16 code = 0;
    if (fromTown) {
        dAnimalBlock_c *block = dSaveData_c::getAnimalBlock();
        dQuestPlayerAnimal_c *game = &block->mHideAndSeek;
        u32 slot = animal != NULL ? block->getHiderSlot(&animal->mID) : 3;
        BOOL found = slot < 3 ? game->isFlag(slot) : TRUE;
        BOOL timeUp = game->isTimeUp(*dTime_c::getCurrent(), TRUE);
        if (found) {
            label = l_Q10_Wait;
            if (timeUp) {
                step = &talk_c::stepHideOver;
                code = 4;
            }
        } else if (timeUp) {
            label = l_Q10_Over;
            step = &talk_c::stepHideOver;
        } else {
            label = l_Q10_Find;
            step = &talk_c::stepHideFind;
        }
    } else {
        label = l_Q10_Visit;
        dPrivateData_c *visitor = dPlayerMgr_c::getNetPlayer(0);
        if (visitor != NULL) {
            dPersonalID_c *pid = &visitor->mPID;
            if (pid->isValid()) {
                setPersonalName(pid, 0);
                if (animal != NULL) {
                    u16 unit = fn_800F3F38(pid->player.getGender(), animal->mID.getLooks(1));
                    if (unit == 0) {
                        clearWord(1);
                    } else {
                        getController()->fn_801A5874(1, unit, "sys_STRING/STR_Unit");
                    }
                }
            }
        }
    }
    setLooksMsg(info, label, code);
    setHookProc(&talk_c::endQuestCommon);
    setStepProc(step);
    return TRUE;
}

// 80058114
BOOL dAcNpcNml_c::talk_c::stepHideFind(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dAnimalBlock_c *block = &dSaveData_c::getTown()->mAnimals.mTown;
        dAnimal_c *animal = getAnimal();
        int slot = animal != NULL ? block->getHiderSlot(&animal->mID) : 3;
        dQuestPlayerAnimal_c *game = &block->mHideAndSeek;
        if (slot < 3) {
            game->setFlag(slot);
            fn_800F0708(slot);
        }
        if (game->countUnfound() == 0) {
            if (fn_800DCEDC()) {
                setProcSet(&l_talkProcSets[TALK_PROC_HIDE_REWARD]);
                startMsg();
            } else {
                setRequest1(0);
                mResultProc = &talk_c::resHideEnd;
            }
        } else {
            setMsgProc(&talk_c::msgHideContinue);
            startMsg();
        }
        addFriendship(5);
        return TRUE;
    }
    return FALSE;
}

// 80058270
void dAcNpcNml_c::talk_c::resHideEnd() {
    fn_800FC598();
    s16 angle;
    int state;
    mVec3_c pos;
    fn_800FE688(&state, &pos, &angle);
    fn_801A8134(&pos, -1, 6);
    getSceneChange()->setReturnExit(0, &pos, state, 0);
    getSceneChange()->requestExit(0, 2, 2);
    fn_801A6748(2);
    fgMngProc_blockDayChange();
    dBgm::l_mgr.mStgField.set3A();
}

static const char l_Q10_Continue[] = "Q10_Continue"; // 8046CA54

// 800582F8
int dAcNpcNml_c::talk_c::msgHideContinue(msgInfo_s *info) {
    u32 left = dSaveData_c::getRaw()->mAnimals.mTown.mHideAndSeek.countUnfound();
    int code = 1;
    if (left == 2) {
        code = 2;
    }
    setLooksMsg(info, l_Q10_Continue, code);
    formatNumber(left, 1, 12, dScript::NUM_FORMAT_REGION);
    return TRUE;
}

// 80058388
BOOL dAcNpcNml_c::talk_c::stepHideOver(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dQuestPlayerAnimal_c *game = &dSaveData_c::getTown()->mAnimals.mTown.mHideAndSeek;
        game->mBase.mState = 2;
        fn_800F06A8(2);
        if (fn_800DCEDC()) {
            setProcSet(&l_talkProcSets[TALK_PROC_HIDE_LOSE]);
            startMsg();
        } else {
            setRequest1(0);
            mResultProc = &talk_c::resHideEnd;
        }
        addHiderFriendship(-2);
        return TRUE;
    }
    return FALSE;
}

static const char l_Q10_Item[] = "Q10_Item";         // 8046CA64
static const char l_Q10_ItemFull[] = "Q10_ItemFull"; // 8046CA70

// 8005844C
int dAcNpcNml_c::talk_c::msgHideReward(msgInfo_s *info) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    stepFunc step = &talk_c::stepHideReward;
    const char *label = l_Q10_Item;
    BOOL full = player != NULL ? player->findEmptyPocket(0) == -1 : TRUE;
    if (full) {
        step = &talk_c::stepHideRewardFull;
        label = l_Q10_ItemFull;
    }
    setLooksMsg(info, label, 0);
    setHookProc(&talk_c::endHideReward);
    setStepProc(step);
    return TRUE;
}

// 80058574
void dAcNpcNml_c::talk_c::endHideReward(int arg) {
    dAnimalBlock_c *block;
    dAnimal_c *animal = getAnimal();
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    mItem0 = dItem::ITEM_ID_NONE;
    if (animal != NULL && player != NULL) {
        block = &dSaveData_c::getTown()->mAnimals.mTown;
        dItem::Item present;
        dQuestPlayerAnimal_c *game = &block->mHideAndSeek;
        int kind = block->pickHideAndSeekPresent(&present, animal);
        if (present != dItem::ITEM_ID_NONE) {
            mItem0 = present;
            if (kind == 1 && animal->removeNewItem(&present)) {
                fn_800F0FE4(getNpcIdx(), &mItem0);
            }
            int pocket = player->findEmptyPocket(0);
            if (pocket == -1) {
                game->mItem = present;
                game->mAnimal.copy(&animal->mID);
                if (block->sendHideAndSeekLetter(FALSE)) {
                    game->mItem = dItem::ITEM_ID_NONE;
                }
            } else {
                player->setPocket(&present, pocket, FALSE);
            }
            setItemName(&present, 2);
        }
        game->mBase.mState = 3;
        fn_800F06A8(3);
    }
    fgMngProc_unblockDayChange();
}

// 800586BC
BOOL dAcNpcNml_c::talk_c::stepHideReward(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        if (mItem0 != dItem::ITEM_ID_NONE) {
            requestItemAct(&mItem0, 0, 0);
        }
        setMsgProc(&talk_c::msgHideEnd);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

static const char l_Q10_End[] = "Q10_End"; // 80750240

// 80058754
int dAcNpcNml_c::talk_c::msgHideEnd(msgInfo_s *info) {
    setLooksMsg(info, l_Q10_End, 0);
    return TRUE;
}

// 80058780
BOOL dAcNpcNml_c::talk_c::stepHideRewardFull(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgHideEnd);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

static const char l_Q10_Lose[] = "Q10_Lose"; // 8046CA80

// 800587F8
int dAcNpcNml_c::talk_c::msgHideLose(msgInfo_s *info) {
    setLooksMsg(info, l_Q10_Lose, 0);
    setHookProc(&talk_c::endHideLose);
    return TRUE;
}

// 8005885C
void dAcNpcNml_c::talk_c::endHideLose(int arg) {
    dQuestPlayerAnimal_c *game = &dSaveData_c::getTown()->mAnimals.mTown.mHideAndSeek;
    if (fn_800DCEDC()) {
        game->clearInfo();
        fn_800F0650();
    } else {
        game->mBase.mState = 3;
        fn_800F06A8(3);
    }
    fgMngProc_unblockDayChange();
}

// 800588BC
void dAcNpcNml_c::talk_c::addHiderFriendship(s8 delta) {
    dQuestPlayerAnimal_c *game = &dSaveData_c::getRaw()->mAnimals.mTown.mHideAndSeek;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        for (int i = 0; i < 3; i++) {
            int idx = game->getHider(i);
            if (idx != -1 && !game->isFlag(i)) {
                dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
                if (animal != NULL) {
                    dAnimalMemory_c *memory = animal->findMemory(&player->mPID);
                    if (memory != NULL && memory->mPlayer.isValid()) {
                        memory->addFriendship(delta);
                    }
                }
            }
        }
    }
}
