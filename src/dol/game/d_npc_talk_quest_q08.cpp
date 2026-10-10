// Villager talk of appointment quest 8 (QUEST_KIND_APPOINTMENT_1): the villager visits the player's
// house. .text 8005C82C..8005E584. See include/game/game/d_npc_talk_quest_q08.hpp.
#include <game/game/d_npc_talk_quest_q08.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_hr.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_string.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

static const char l_Q08_Req[] = "Q08_Req";

// 8005C82C
int dAcNpcNml_c::talk_c::msgVisitOffer(msgInfo_s *info) {
    if (!isCurrentSceneAttr(SCENE_ATTR_TOWN)) {
        return FALSE;
    }
    if (getAnimal() == NULL) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL || player->isFlag0(0xD)) {
        return FALSE;
    }
    dSaveData_c *save = dSaveData_c::getRaw();
    if (!save->mAnimals.mTown.canStartAppointment(player, dTime_c::getCurrent(), QUEST_KIND_APPOINTMENT_1)) {
        return FALSE;
    }
    return startQuestOffer(info, 8, &talk_c::endQuestCommon, &talk_c::stepVisitOffer, TRUE);
}

// 8005C950
BOOL dAcNpcNml_c::talk_c::stepVisitOffer(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgVisitReq);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 8005C9C8
int dAcNpcNml_c::talk_c::msgVisitReq(msgInfo_s *info) {
    setLooksMsg(info, l_Q08_Req, 0);
    setStepProc(&talk_c::stepVisitChoice);
    return TRUE;
}

// 8005CA28
BOOL dAcNpcNml_c::talk_c::stepVisitChoice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoice(0, 0x29, 5, &talk_c::selVisitYes);
        setChoice(1, 0x1F, 10, &talk_c::selVisitNo);
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

// 8005CB28
void dAcNpcNml_c::talk_c::selVisitYes() {
    setMsgProc(&talk_c::msgVisitReserve);
    startMsg();
}

static const char l_Q08_Reserve[] = "Q08_Reserve";

// 8005CB7C
int dAcNpcNml_c::talk_c::msgVisitReserve(msgInfo_s *info) {
    setLooksMsg(info, l_Q08_Reserve, 0);
    setStepProc(&talk_c::stepVisitTimeMenu);
    return TRUE;
}

// 8005CBE0
BOOL dAcNpcNml_c::talk_c::stepVisitTimeMenu(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        reqMenu27(1);
        mResultProc = &talk_c::resVisitTime;
        return TRUE;
    }
    return FALSE;
}

// 8005CC50
void dAcNpcNml_c::talk_c::resVisitTime() {
    msgFunc msg = &talk_c::msgVisitTooLate;
    if (!isMenuInvalid()) {
        dAnimal_c *animal = getAnimal();
        dTime_c *now = dTime_c::getCurrent();
        dTime_c entered = getMenuResult();
        msg = &talk_c::msgVisitReserved;
        dTime_c meet;
        meet.set(now->year, now->month, now->mday, entered.hour, entered.min, 0);
        meet.normalize();
        if (dTime_c::isSameOrAfter(*now, meet)) {
            meet.add(1, 0, 0, 0);
        }
        dTime_c limit;
        limit.set(now->year, now->month, now->mday, now->hour + 12, now->min, 0);
        limit.normalize();
        BOOL tooLate = animal == NULL || !dTime_c::isSameOrAfter(limit, meet);
        if (tooLate) {
            msg = &talk_c::msgVisitTooLate;
        } else {
            limit.set(now->year, now->month, now->mday, now->hour, now->min + 30, 0);
            limit.normalize();
            if (!dTime_c::isSameOrAfter(meet, limit)) {
                msg = &talk_c::msgVisitTooSoon;
            } else if (animal->isSleepTime(&meet)) {
                msg = &talk_c::msgVisitSleeping;
            } else if (meet.hour < 6) {
                msg = &talk_c::msgVisitTooLate;
            }
        }
    }
    setMsgProc(msg);
    startMsg();
    reqMsgClose();
}

static const char l_Q08_Reserved[] = "Q08_Reserved";

// 8005D0CC
int dAcNpcNml_c::talk_c::msgVisitReserved(msgInfo_s *info) {
    setLooksMsg(info, l_Q08_Reserved, 0);
    setHookProc(&talk_c::endVisitReserved);
    return TRUE;
}

// 8005D130
void dAcNpcNml_c::talk_c::endVisitReserved(int arg) {
    dAnimalBlock_c *block = &dSaveData_c::getTown()->mAnimals.mTown;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimal_c *animal = getAnimal();
    if (animal != NULL && player != NULL) {
        dTime_c *now = dTime_c::getCurrent();
        dTime_c entered = getMenuResult();
        block->startAppointment(QUEST_KIND_APPOINTMENT_1, &player->mPID.player, &animal->mID, entered.hour,
                                entered.min, now);
        dTime_c meet = block->mAppointment.getMeetTime();
        setDayName((u8)meet.mday, 2);
        setTime(meet.hour, 3, 1);
        setMinute(meet.min, 2);
        addFriendship(2);
    }
}

static const char l_Q08_Error1[] = "Q08_Error1";

// 8005D2B0
int dAcNpcNml_c::talk_c::msgVisitTooSoon(msgInfo_s *info) {
    setLooksMsg(info, l_Q08_Error1, 0);
    setStepProc(&talk_c::stepVisitTimeMenu);
    return TRUE;
}

static const char l_Q08_Error2[] = "Q08_Error2";

// 8005D314
int dAcNpcNml_c::talk_c::msgVisitTooLate(msgInfo_s *info) {
    setLooksMsg(info, l_Q08_Error2, 0);
    setStepProc(&talk_c::stepVisitTimeMenu);
    return TRUE;
}

static const char l_Q08_Error3[] = "Q08_Error3";

// 8005D378
int dAcNpcNml_c::talk_c::msgVisitSleeping(msgInfo_s *info) {
    setLooksMsg(info, l_Q08_Error3, 0);
    setStepProc(&talk_c::stepVisitTimeMenu);
    return TRUE;
}

// 8005D3DC
void dAcNpcNml_c::talk_c::selVisitNo() {
    setMsgProc(&talk_c::msgVisitNo);
    startMsg();
}

static const char l_Q08_No[] = "Q08_No";

// 8005D430
int dAcNpcNml_c::talk_c::msgVisitNo(msgInfo_s *info) {
    setLooksMsg(info, l_Q08_No, 0);
    return TRUE;
}

// Labels of the quest talk (getMsgLabel(TALK_QUEST, QUEST_TALK_HOUSE_VISIT, sub)): before the meeting,
// then broken / late / missed appointment.
const char *l_q08Labels[3] = {l_Ai_Quest, "Q08_Leave", "Q08_Leave"};

// 8005D45C
int dAcNpcNml_c::talk_c::msgVisitQuest(msgInfo_s *info) {
    if (isEventOngoing()) {
        return FALSE;
    }
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    dAnimalBlock_c *block = &dSaveData_c::getRaw()->mAnimals.mTown;
    const dAnmPersonalID_c *id = &animal->mID;
    dQuestPlayerItem_c *appt = &block->mAppointment;
    dQuestBase_c *quest = &appt->mBase;
    if (!quest->isActive() || quest->getKind() != QUEST_KIND_APPOINTMENT_1) {
        return FALSE;
    }
    if (!block->isAppointmentAnimal(id)) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL || !player->mPID.isFromTown()) {
        return FALSE;
    }
    if (!player->mPID.player.isSame(&appt->mPlayer)) {
        return FALSE;
    }
    if (quest->mState == 2) {
        return FALSE;
    }
    int sub = 0;
    u16 code = 0;
    if (quest->mState == 1) {
        sub = 2;
        code = 2;
    } else if (appt->isPastMeetTime(*dTime_c::getCurrent(), 0)) {
        sub = 1;
        code = 1;
        addFriendship(-20);
    }
    dTime_c meet = appt->getMeetTime();
    setDayName((u8)meet.mday, 2);
    setTime(meet.hour, 3, 1);
    setMinute(meet.min, 2);
    stepFunc step = &talk_c::stepVisitTalkMenu;
    switch (sub) {
    case 1:
    case 2:
        step = &talk_c::stepVisitClear;
        break;
    }
    const char *label = getMsgLabel(TALK_QUEST, QUEST_TALK_HOUSE_VISIT, sub);
    if (label != NULL) {
        setProcSet(&l_talkEntrySets[TALK_QUEST]);
        setLooksMsg(info, label, code);
        setStepProc(step);
        setTopic(dNpc::msgMemory_c::KIND_QUEST, QUEST_TALK_HOUSE_VISIT, 0);
        return TRUE;
    }
    return FALSE;
}

// 8005D724
BOOL dAcNpcNml_c::talk_c::stepVisitTalkMenu(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        int num = 0;
        u16 code = 1;
        u8 range = 0;
        hookFunc rollan = getRollanChoice(&code, &range);
        if (rollan) {
            setChoice(0, code, range, rollan);
            num = 1;
        }
        hookFunc harvest = getHarvestChoice(&code, &range, TRUE);
        if (harvest) {
            setChoice(num, code, range, harvest);
            num++;
        }
        setChoice(num, 1, 3, &talk_c::selVisitCon);
        setChoice(num + 1, 4, 3, &talk_c::selVisitResume);
        setChoiceNum(num + 2);
        setChoiceCancel(num + 1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 8005D8F8
void dAcNpcNml_c::talk_c::selVisitCon() {
    setMsgProc(&talk_c::msgVisitCon);
    startMsg();
}

static const char l_Q08_Con[] = "Q08_Con";

// 8005D94C
int dAcNpcNml_c::talk_c::msgVisitCon(msgInfo_s *info) {
    setLooksMsg(info, l_Q08_Con, 0);
    return TRUE;
}

// 8005D978
void dAcNpcNml_c::talk_c::selVisitResume() {
    const procSet_s *set = getEventProcSet();
    if (set == NULL) {
        set = &l_talkEntrySets[TALK_FREE];
    }
    setProcSet(set);
    startMsg();
}

// 8005D9CC
BOOL dAcNpcNml_c::talk_c::stepVisitClear(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dSaveData_c::getTown()->mAnimals.mTown.mAppointment.clear();
        return TRUE;
    }
    return FALSE;
}

static const char l_Q08_Call[] = "Q08_Call";

// 8005DA1C
int dAcNpcNml_c::talk_c::msgVisitCall(msgInfo_s *info) {
    setLooksMsg(info, l_Q08_Call, 0);
    setHookProc(&talk_c::endQuestCommon);
    setStepProc(&talk_c::stepVisitCall);
    return TRUE;
}

// 8005DAA8
BOOL dAcNpcNml_c::talk_c::stepVisitCall(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgVisitDoor);
        startMsg();
        setRequest1(0);
        mResultProc = &talk_c::resVisitCall;
        return TRUE;
    }
    return FALSE;
}

// 8005DB48
void dAcNpcNml_c::talk_c::resVisitCall() {
    setActProc(&talk_c::actVisitWalk);
}

// 8005DB88
void dAcNpcNml_c::talk_c::actVisitWalk() {
    dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
    if (npc != NULL) {
        npc->mIsActive = 1;
        if (npc->mAction.requestWalk(1, l_walkTargetPos, l_walkTurnSpeed, 0, l_moveParamWalk, l_defaultAnmRate,
                                     cNpcMorphFrames)) {
            npc->mPos.y = dBGCF::getGroundY(&npc->mPos, FALSE);
            setActProc(&talk_c::actVisitArrive);
        }
    }
}

// 8005DC38
void dAcNpcNml_c::talk_c::actVisitArrive() {
    dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
    if (npc != NULL) {
        npc->mPos.y = dBGCF::getGroundY(&npc->mPos, FALSE);
        if (npc->mAction.mCanChange) {
            npc->mAction.requestWait(1);
            reqMsgClose();
            lbl_8074E9B0->fn_8018B5B8(npc->demoHook58());
            clearActProc();
            dSaveData_c::getTown()->mAnimals.mTown.mAppointment.setFlag(0);
        }
    }
}

static const char l_Q08_Door[] = "Q08_Door";

// 8005DCE4
int dAcNpcNml_c::talk_c::msgVisitDoor(msgInfo_s *info) {
    setLooksMsg(info, l_Q08_Door, 0);
    addFriendship(10);
    return TRUE;
}

// 8005DD2C
int dAcNpcNml_c::talk_c::msgVisitRoomtalk(msgInfo_s *info) {
    setLooksMsg(info, "Q_Roomtalk", 0);
    return TRUE;
}

// 8005DD5C
u32 dAcNpcNml_c::talk_c::pickRoomFtrMsg(u16 *msg, dItem::Item *item, u32 count, int layer, u8 looks) {
    dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
    dHome_c *home = homes->getHome(homes->findCurrentPlayer());
    if (home != NULL) {
        dHomeRoom_c *room = home->getRoom(0);
        if (room != NULL) {
            dItem::Item *unit = (dItem::Item *)room->getLayer(layer);
            if (unit != NULL) {
                for (int i = 0; i < 0x100; i++, unit++) {
                    if (unit->isValid()) {
                        u16 code = getNpcMsgFlagged(*unit, looks, TRUE);
                        if (code != 0) {
                            f32 chance = 100.0f / (count + 1);
                            if (cM::rndF(100.0f) <= chance) {
                                *msg = code;
                                *item = *unit;
                            }
                            count++;
                        }
                    }
                }
            }
        }
    }
    return count;
}

static const char l_Q08_Furniture[] = "Q08_Furniture";

// 8005DE9C
int dAcNpcNml_c::talk_c::msgVisitFurniture(msgInfo_s *info) {
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    u16 msg = 0;
    u8 looks = animal->mID.getLooks(1);
    dItem::Item item;
    u32 count = pickRoomFtrMsg(&msg, &item, 0, 0, looks);
    if (count == 0 || msg == 0 || !item.isValid()) {
        return FALSE;
    }
    pickRoomFtrMsg(&msg, &item, count, 1, looks);
    if (item.isValid()) {
        setItemName(&item, 0);
    }
    setLooksMsg(info, l_Q08_Furniture, msg);
    return TRUE;
}

static const char l_Q08_Layout[] = "Q08_Layout";
static const char l_Q08_First[] = "Q08_First";

// 8005DFA0
int dAcNpcNml_c::talk_c::msgVisitLayout(msgInfo_s *info) {
    // the rating as stars (word 0)
    static const wchar_t *l_stars[5] = {
        L"\x2606",
        L"\x2606\x2606",
        L"\x2606\x2606\x2606",
        L"\x2606\x2606\x2606\x2606",
        L"\x2606\x2606\x2606\x2606\x2606",
    };
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL) {
        return FALSE;
    }
    int home = dSaveData_c::getRaw()->mHomes.findCurrentPlayer();
    u32 rank = 0;
    int flags = dHR::getPlayerRank(home, &rank);
    if (flags == 0) {
        return FALSE;
    }
    if (rank == 0) {
        rank = 1;
    } else if (rank > 5) {
        rank = 5;
    }
    u16 code;
    if (flags & HR_LAYOUT_MESSY) {
        code = cM::rndInt(2) + 1;
    } else if (flags & HR_LAYOUT_FACING_WALL) {
        code = cM::rndInt(2) + 3;
    } else if (!(flags & HR_LAYOUT_COMFY)) {
        code = cM::rndInt(2) + 5;
    } else if (flags & HR_LAYOUT_SERIES) {
        code = 7;
    } else if (flags & HR_LAYOUT_SERIES_PARTS) {
        code = cM::rndInt(2) + 8;
    } else {
        code = cM::rndInt(2) + 10;
    }
    setLooksMsg(info, l_Q08_Layout, code);
    dString::Word_c word(l_stars[rank - 1]);
    getController()->setWord(0, &word);
    dQuestPlayerItem_c *appt = &dSaveData_c::getTown()->mAnimals.mTown.mAppointment;
    if (!appt->mItem.isValid()) {
        const dItem::Item present = pickAppointmentPresent(player, rank, (flags >> 2) & 1);
        if (present.isValid()) {
            appt->mItem = present;
        }
    }
    return TRUE;
}

// 8005E178
int dAcNpcNml_c::talk_c::msgVisitFirst(msgInfo_s *info) {
    static const msgFunc l_roomTalk[3] = {
        &talk_c::msgVisitRoomtalk,
        &talk_c::msgVisitFurniture,
        &talk_c::msgVisitLayout,
    };
    if (!dSaveData_c::getRaw()->mAnimals.mTown.mAppointment.isFlag(1)) {
        setLooksMsg(info, l_Q08_First, 0);
        setHookProc(&talk_c::endVisitFirst);
        return TRUE;
    }
    const msgFunc *proc = &l_roomTalk[cM::rndInt(3)];
    int ret = FALSE;
    if (*proc) {
        ret = (this->**proc)(info);
    }
    if (!ret) {
        ret = (this->*l_roomTalk[0])(info);
    }
    if (ret) {
        setHookProc(&talk_c::endQuestCommon);
    }
    return ret;
}

// 8005E2BC
void dAcNpcNml_c::talk_c::endVisitFirst(int arg) {
    endQuestCommon(arg);
    dSaveData_c::getTown()->mAnimals.mTown.mAppointment.setFlag(1);
}

static const char l_Q08_Wait[] = "Q08_Wait";

// 8005E2F0
int dAcNpcNml_c::talk_c::msgVisitWait(msgInfo_s *info) {
    setLooksMsg(info, l_Q08_Wait, 0);
    setHookProc(&talk_c::endQuestCommon);
    setStepProc(&talk_c::stepVisitWait);
    return TRUE;
}

// 8005E37C
BOOL dAcNpcNml_c::talk_c::stepVisitWait(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgVisitBye);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

static const char l_Q08_Bye[] = "Q08_Bye";

// 8005E3F4
int dAcNpcNml_c::talk_c::msgVisitBye(msgInfo_s *info) {
    setLooksMsg(info, l_Q08_Bye, 0);
    setHookProc(&talk_c::endVisitBye);
    return TRUE;
}

// 8005E454
void dAcNpcNml_c::talk_c::endVisitBye(int arg) {
    dQuestPlayerItem_c *appt = &dSaveData_c::getTown()->mAnimals.mTown.mAppointment;
    appt->mBase.mState = 2;
}

static const char l_Q08_Back[] = "Q08_Back";

// 8005E480
int dAcNpcNml_c::talk_c::msgVisitBack(msgInfo_s *info) {
    setLooksMsg(info, l_Q08_Back, 0);
    setHookProc(&talk_c::endQuestCommon);
    setStepProc(&talk_c::stepVisitBack);
    return TRUE;
}

// 8005E50C
BOOL dAcNpcNml_c::talk_c::stepVisitBack(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgVisitBye);
        startMsg();
        return TRUE;
    }
    return FALSE;
}
