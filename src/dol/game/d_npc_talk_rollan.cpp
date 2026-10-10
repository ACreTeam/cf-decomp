// The villager's "Ev_Rollan" talk: a present offered as one answer of a two-choice menu.
// .text 80060980..800613BC. See include/game/game/d_npc_talk_rollan.hpp.
#include <game/game/d_npc_talk_rollan.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_hr.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_sv_player_flag.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

// 8046CD20
static const char l_Ev_Rollan[] = "Ev_Rollan";

// 80060980
const talk_c::hookFunc dAcNpcNml_c::talk_c::getRollanChoice(u16 *code, u8 *flag) {
    static const hookFunc l_answers[4] = {
        &talk_c::selRollanFloor,
        &talk_c::selRollanFloorFull,
        &talk_c::selRollanWall,
        &talk_c::selRollanWallFull,
    };
    static const u16 l_codes[4] = {7, 7, 10, 10};

    dAnimalBlock_c *block;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL || player->isFlag0(0xD)) {
        return NULL;
    }
    if (!player->mPID.isFromTown()) {
        return NULL;
    }
    if (!dEvent::isVisitorHere(9)) {
        return NULL;
    }
    hookFunc none = NULL;
    hookFunc proc = none;
    u16 dummyCode = 1;
    u8 dummyFlag = 1;
    if (code == NULL) {
        code = &dummyCode;
    }
    if (flag == NULL) {
        flag = &dummyFlag;
    }
    dAnimal_c *animal = getAnimal();
    if (animal == NULL || !animal->mID.isValid()) {
        return none;
    }
    block = &dSaveData_c::getRaw()->mAnimals.mTown;
    if (animal->isMoving() || block->isSickAnimal(&animal->mID) || block->isLostItemAnimal(&animal->mID)) {
        return none;
    }
    int state = 0;
    if (animal != NULL) {
        u32 idx = block->getAnimalIdx(&animal->mID);
        if (idx < 10) {
            state = fn_8015112C(dSaveData_c::getRaw()->_072CC0, idx);
        }
    }
    BOOL full = TRUE;
    if (player->findEmptyPocket(0) != -1) {
        full = FALSE;
    }
    int answer = 4;
    switch (state) {
    case 1:
        if (full) {
            answer = 1;
        } else {
            answer = 0;
        }
        break;
    case 2:
        answer = 2;
        if (full) {
            answer = 3;
        }
        break;
    }
    if ((u32)answer < 4) {
        proc = l_answers[answer];
        *code = l_codes[answer];
        *flag = 3;
    }
    return proc;
}

// 80060C44
int dAcNpcNml_c::talk_c::msgRollan(msgInfo_s *info) {
    hookFunc proc = getRollanChoice(NULL, NULL);
    if (!proc) {
        return FALSE;
    }
    const char *label = getMsgLabel(TALK_ROLLAN, 0, 0);
    if (label != NULL) {
        setProcSet(&l_talkEntrySets[TALK_ROLLAN]);
        setLooksMsg(info, label, 0);
        setTopic(dNpc::msgMemory_c::KIND_ANY, 0, 0);
        return TRUE;
    }
    return FALSE;
}

// 80060D28
void dAcNpcNml_c::talk_c::endRollan(int arg) {
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

// 80060DB8
BOOL dAcNpcNml_c::talk_c::stepRollan(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        u16 code = 1;
        u8 flag = 1;
        hookFunc proc = getRollanChoice(&code, &flag);
        if (proc) {
            clearChoice();
            setChoice(0, code, flag, proc);
            setChoice(1, 4, 3, &talk_c::selResumeTalk);
            setChoiceNum(2);
            setChoiceCancel(1);
            showChoice();
            return TRUE;
        }
    }
    return FALSE;
}

// 80060ED8
BOOL dAcNpcNml_c::talk_c::rollRollanGift() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL || player->findEmptyPocket(0) == -1 || player->isFlag0(0xD)) {
        return FALSE;
    }
    u32 rating2 = 0;
    u32 rating1 = 0;
    int home = dSaveData_c::getTown()->mHomes.findOwner(player);
    if ((u32)home < 4) {
        const u8 *rating = fn_800AC28C(home);
        rating2 = rating[2];
        rating1 = rating[1];
    }
    u32 chance = (rating2 + rating1) / 10 + 20;
    if (player->mFortune == FORTUNE_ITEM) {
        chance += 5;
    }
    if (chance > 100) {
        chance = 100;
    }
    if ((u32)cM::rndInt(100) < chance) {
        return TRUE;
    }
    return FALSE;
}

// 80060FCC
void dAcNpcNml_c::talk_c::clearRollanFlag() {
    dAnimal_c *animal = getAnimal();
    if (animal != NULL) {
        u32 idx = dSaveData_c::getRaw()->mAnimals.mTown.getAnimalIdx(&animal->mID);
        if (idx < 10) {
            fn_801510EC(dSaveData_c::getTown()->_072CC0, idx);
        }
    }
}

// 80061034
void dAcNpcNml_c::talk_c::selRollanFloor() {
    if (rollRollanGift()) {
        setMsgProc(&talk_c::msgRollanFloorGift);
    } else {
        setMsgProc(&talk_c::msgRollanFloorNone);
    }
    startMsg();
}

// 800610C4
int dAcNpcNml_c::talk_c::msgRollanFloorGift(msgInfo_s *info) {
    u16 code = cM::rndInt(3) + 4;
    mItem0.setFromIndex(dItem::ITEM_IDX_OLD_FLOORING);
    setLooksMsg(info, l_Ev_Rollan, code);
    clearRollanFlag();
    return TRUE;
}

// 80061140
int dAcNpcNml_c::talk_c::msgRollanFloorNone(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Rollan, cM::rndInt(3) + 1);
    clearRollanFlag();
    return TRUE;
}

// 800611A4
void dAcNpcNml_c::talk_c::selRollanFloorFull() {
    setMsgProc(&talk_c::msgRollanFloorNone);
    startMsg();
}

// 800611F8
void dAcNpcNml_c::talk_c::selRollanWall() {
    if (rollRollanGift()) {
        setMsgProc(&talk_c::msgRollanWallGift);
    } else {
        setMsgProc(&talk_c::msgRollanWallNone);
    }
    startMsg();
}

// 80061288
int dAcNpcNml_c::talk_c::msgRollanWallGift(msgInfo_s *info) {
    u16 code = cM::rndInt(3) + 0xE;
    mItem0.setFromIndex(dItem::ITEM_IDX_OLD_WALLPAPER);
    setLooksMsg(info, l_Ev_Rollan, code);
    clearRollanFlag();
    return TRUE;
}

// 80061304
int dAcNpcNml_c::talk_c::msgRollanWallNone(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Rollan, cM::rndInt(3) + 0xB);
    clearRollanFlag();
    return TRUE;
}

// 80061368
void dAcNpcNml_c::talk_c::selRollanWallFull() {
    setMsgProc(&talk_c::msgRollanWallNone);
    startMsg();
}
