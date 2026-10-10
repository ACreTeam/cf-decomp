// Villager talk of appointment quest 9 (QUEST_KIND_APPOINTMENT_0): the villager invites the player to
// their house. .text 8005A3C4..8005C82C. See include/game/game/d_npc_talk_quest_q09.hpp.
#include <game/game/d_npc_talk_quest_q09.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_hr.hpp>
#include <game/game/d_money.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

static const char l_Q09_Req[] = "Q09_Req";

// 8005A3C4
int dAcNpcNml_c::talk_c::msgInviteOffer(msgInfo_s *info) {
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
    if (!save->mAnimals.mTown.canStartAppointment(player, dTime_c::getCurrent(), QUEST_KIND_APPOINTMENT_0)) {
        return FALSE;
    }
    return startQuestOffer(info, 9, &talk_c::endQuestCommon, &talk_c::stepInviteOffer, TRUE);
}

// 8005A4E8
BOOL dAcNpcNml_c::talk_c::stepInviteOffer(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgInviteReq);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 8005A560
int dAcNpcNml_c::talk_c::msgInviteReq(msgInfo_s *info) {
    setLooksMsg(info, l_Q09_Req, 0);
    setStepProc(&talk_c::stepInviteChoice);
    return TRUE;
}

// 8005A5C0
BOOL dAcNpcNml_c::talk_c::stepInviteChoice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoice(0, 0x2E, 5, &talk_c::selInviteYes);
        setChoice(1, 0x1F, 10, &talk_c::selInviteNo);
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

// 8005A6C0
void dAcNpcNml_c::talk_c::selInviteYes() {
    setMsgProc(&talk_c::msgInviteReserve);
    startMsg();
}

static const char l_Q09_Reserve[] = "Q09_Reserve";

// 8005A714
int dAcNpcNml_c::talk_c::msgInviteReserve(msgInfo_s *info) {
    setLooksMsg(info, l_Q09_Reserve, 0);
    setStepProc(&talk_c::stepInviteTimeMenu);
    return TRUE;
}

// 8005A778
BOOL dAcNpcNml_c::talk_c::stepInviteTimeMenu(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        reqMenu27(1);
        mResultProc = &talk_c::resInviteTime;
        return TRUE;
    }
    return FALSE;
}

// 8005A7E8
void dAcNpcNml_c::talk_c::resInviteTime() {
    msgFunc msg = &talk_c::msgInviteTooLate;
    if (!isMenuInvalid()) {
        dAnimal_c *animal = getAnimal();
        dTime_c *now = dTime_c::getCurrent();
        dTime_c entered = getMenuResult();
        msg = &talk_c::msgInviteReserved;
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
            msg = &talk_c::msgInviteTooLate;
        } else {
            limit.set(now->year, now->month, now->mday, now->hour, now->min + 30, 0);
            limit.normalize();
            if (!dTime_c::isSameOrAfter(meet, limit)) {
                msg = &talk_c::msgInviteTooSoon;
            } else if (animal->isSleepTime(&meet)) {
                msg = &talk_c::msgInviteSleeping;
            } else if (meet.hour < 6) {
                msg = &talk_c::msgInviteTooLate;
            }
        }
    }
    setMsgProc(msg);
    startMsg();
    reqMsgClose();
}

static const char l_Q09_Reserved[] = "Q09_Reserved";

// 8005AC64
int dAcNpcNml_c::talk_c::msgInviteReserved(msgInfo_s *info) {
    setLooksMsg(info, l_Q09_Reserved, 0);
    setHookProc(&talk_c::endInviteReserved);
    return TRUE;
}

// 8005ACC8
void dAcNpcNml_c::talk_c::endInviteReserved(int arg) {
    dAnimalBlock_c *block = &dSaveData_c::getTown()->mAnimals.mTown;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimal_c *animal = getAnimal();
    if (animal != NULL && player != NULL) {
        dTime_c *now = dTime_c::getCurrent();
        dTime_c entered = getMenuResult();
        block->startAppointment(QUEST_KIND_APPOINTMENT_0, &player->mPID.player, &animal->mID, entered.hour,
                                entered.min, now);
        dTime_c meet = block->mAppointment.getMeetTime();
        setDayName((u8)meet.mday, 2);
        setTime(meet.hour, 3, 1);
        setMinute(meet.min, 2);
        addFriendship(2);
    }
}

static const char l_Q09_Error1[] = "Q09_Error1";

// 8005AE48
int dAcNpcNml_c::talk_c::msgInviteTooSoon(msgInfo_s *info) {
    setLooksMsg(info, l_Q09_Error1, 0);
    setStepProc(&talk_c::stepInviteTimeMenu);
    return TRUE;
}

static const char l_Q09_Error2[] = "Q09_Error2";

// 8005AEAC
int dAcNpcNml_c::talk_c::msgInviteTooLate(msgInfo_s *info) {
    setLooksMsg(info, l_Q09_Error2, 0);
    setStepProc(&talk_c::stepInviteTimeMenu);
    return TRUE;
}

static const char l_Q09_Error3[] = "Q09_Error3";

// 8005AF10
int dAcNpcNml_c::talk_c::msgInviteSleeping(msgInfo_s *info) {
    setLooksMsg(info, l_Q09_Error3, 0);
    setStepProc(&talk_c::stepInviteTimeMenu);
    return TRUE;
}

// 8005AF74
void dAcNpcNml_c::talk_c::selInviteNo() {
    setMsgProc(&talk_c::msgInviteNo);
    startMsg();
}

static const char l_Q09_No[] = "Q09_No";

// 8005AFC8
int dAcNpcNml_c::talk_c::msgInviteNo(msgInfo_s *info) {
    setLooksMsg(info, l_Q09_No, 0);
    return TRUE;
}

// Labels of the quest talk (getMsgLabel(TALK_QUEST, QUEST_TALK_INVITATION, sub)): before the meeting,
// then broken / late / missed appointment.
const char *l_q09Labels[4] = {l_Ai_Quest, "Q09_Leave", "Q09_Leave", "Q09_Leave"};

// 8005AFF4
int dAcNpcNml_c::talk_c::msgInviteQuest(msgInfo_s *info) {
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
    if (!quest->isActive() || quest->getKind() != QUEST_KIND_APPOINTMENT_0) {
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
        sub = 1;
        code = 1;
        addFriendship(-20);
    } else if (appt->isPastMeetTime(*dTime_c::getCurrent(), 0)) {
        if (!appt->isPastMeetTime(*dTime_c::getCurrent(), 30)) {
            sub = 3;
            code = 3;
        } else {
            sub = 2;
            code = 2;
        }
    }
    dTime_c meet = appt->getMeetTime();
    setDayName((u8)meet.mday, 2);
    setTime(meet.hour, 3, 1);
    setMinute(meet.min, 2);
    stepFunc step = &talk_c::stepInviteTalkMenu;
    switch (sub) {
    case 1:
    case 2:
    case 3:
        step = &talk_c::stepInviteClear;
        break;
    }
    const char *label = getMsgLabel(TALK_QUEST, QUEST_TALK_INVITATION, sub);
    if (label != NULL) {
        setProcSet(&l_talkEntrySets[TALK_QUEST]);
        setLooksMsg(info, label, code);
        setStepProc(step);
        setTopic(dNpc::msgMemory_c::KIND_QUEST, QUEST_TALK_INVITATION, 0);
        return TRUE;
    }
    return FALSE;
}

// 8005B2E4
BOOL dAcNpcNml_c::talk_c::stepInviteTalkMenu(int kind) {
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
        setChoice(num, 1, 3, &talk_c::selInviteCon);
        setChoice(num + 1, 4, 3, &talk_c::selInviteResume);
        setChoiceNum(num + 2);
        setChoiceCancel(num + 1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 8005B4B8
void dAcNpcNml_c::talk_c::selInviteCon() {
    setMsgProc(&talk_c::msgInviteCon);
    startMsg();
}

static const char l_Q09_Con[] = "Q09_Con";

// 8005B50C
int dAcNpcNml_c::talk_c::msgInviteCon(msgInfo_s *info) {
    setLooksMsg(info, l_Q09_Con, 0);
    return TRUE;
}

// 8005B538
void dAcNpcNml_c::talk_c::selInviteResume() {
    const procSet_s *set = getEventProcSet();
    if (set == NULL) {
        set = &l_talkEntrySets[TALK_FREE];
    }
    setProcSet(set);
    startMsg();
}

// 8005B58C
BOOL dAcNpcNml_c::talk_c::stepInviteClear(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dSaveData_c::getTown()->mAnimals.mTown.mAppointment.clear();
        return TRUE;
    }
    return FALSE;
}

static const char l_Q09_Welcome[] = "Q09_Welcome";

// 8005B5DC
int dAcNpcNml_c::talk_c::msgInviteWelcome(msgInfo_s *info) {
    setLooksMsg(info, l_Q09_Welcome, 0);
    setHookProc(&talk_c::endQuestCommon);
    setStepProc(&talk_c::stepInviteWelcome);
    addFriendship(10);
    return TRUE;
}

// 8005B674
BOOL dAcNpcNml_c::talk_c::stepInviteWelcome(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dSaveData_c::getTown()->mAnimals.mTown.mAppointment.setFlag(0);
        return TRUE;
    }
    return FALSE;
}

// 8005B6C8
int dAcNpcNml_c::talk_c::msgInviteRoomtalk(msgInfo_s *info) {
    setLooksMsg(info, "Q_Roomtalk", 0);
    return TRUE;
}

// 8005B6F8
u16 dAcNpcNml_c::talk_c::pickHouseFtrMsg(dItem::Item *item, u8 looks) {
    u16 msg = 0;
    if (!isCurrentSceneAttr(SCENE_ATTR_VILLAGER_HOUSE)) {
        return 0;
    }
    dFdBase_c *field = fn_80190C44(0);
    if (field == NULL) {
        return 0;
    }
    const dFdBlock_c *block = static_cast<const dFdBase_c *>(field)->getBlock(0, 0);
    if (block == NULL) {
        return 0;
    }
    dItem::Item *unit = block->getItem(0, 0, 0);
    if (unit == NULL) {
        return 0;
    }
    u32 count = 0;
    for (int i = 0; i < 0x100; i++, unit++) {
        if (unit->isValid()) {
            u16 code = getNpcMsgFlagged(*unit, looks, 0);
            if (code != 0) {
                f32 chance = 100.0f / (count + 1);
                if (cM::rndF(100.0f) <= chance) {
                    msg = code;
                    *item = *unit;
                }
                count++;
            }
        }
    }
    return msg;
}

static const char l_Q09_Furniture[] = "Q09_Furniture";

// 8005B858
int dAcNpcNml_c::talk_c::msgInviteFurniture(msgInfo_s *info) {
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    dItem::Item item;
    u16 code = pickHouseFtrMsg(&item, animal->mID.getLooks(1));
    if (code == 0) {
        return FALSE;
    }
    setLooksMsg(info, l_Q09_Furniture, code);
    if (item.isValid()) {
        setItemName(&item, 0);
    }
    dItem::Item music = animal->getMusic();
    if (item.isValid()) {
        setItemName(&music, 1);
    }
    return TRUE;
}

static const char l_Q09_Trade1[] = "Q09_Trade1";
static const char l_Q09_First[] = "Q09_First";
static const char l_Q09_Trade2[] = "Q09_Trade2";

// 8005B940
int dAcNpcNml_c::talk_c::msgInviteTradeOffer(msgInfo_s *info) {
    if (dSaveData_c::getRaw()->mAnimals.mTown.mAppointment.isFlag(3)) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL) {
        return FALSE;
    }
    if (player->findEmptyPocket(0) == -1) {
        return FALSE;
    }
    if (mpNpc == NULL || static_cast<dAcNpcNml_c *>(mpNpc)->getTradeFtrNum() == 0) {
        return FALSE;
    }
    setLooksMsg(info, l_Q09_Trade1, 0);
    setStepProc(&talk_c::stepInviteTradeOffer);
    return TRUE;
}

// 8005BA28
int dAcNpcNml_c::talk_c::msgInviteFirst(msgInfo_s *info) {
    static const msgFunc l_roomTalk[3] = {
        &talk_c::msgInviteRoomtalk,
        &talk_c::msgInviteFurniture,
        &talk_c::msgInviteTradeOffer,
    };
    dQuestPlayerItem_c *appt = &dSaveData_c::getRaw()->mAnimals.mTown.mAppointment;
    if (!appt->isFlag(1)) {
        setLooksMsg(info, l_Q09_First, 0);
        setHookProc(&talk_c::endInviteFirst);
        return TRUE;
    }
    if (appt->isFlag(2) && !appt->isFlag(3)) {
        setLooksMsg(info, l_Q09_Trade2, 0);
        setHookProc(&talk_c::endQuestCommon);
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

// 8005BBD0
void dAcNpcNml_c::talk_c::endInviteFirst(int arg) {
    endQuestCommon(arg);
    dSaveData_c::getTown()->mAnimals.mTown.mAppointment.setFlag(1);
}

// 8005BC04
BOOL dAcNpcNml_c::talk_c::stepInviteTradeOffer(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoiceProc(0, &talk_c::selInviteTradeOffer);
        setChoiceNum(2);
        setChoiceCancel(1);
        return TRUE;
    }
    return FALSE;
}

// 8005BC98
void dAcNpcNml_c::talk_c::selInviteTradeOffer() {
    dSaveData_c::getTown()->mAnimals.mTown.mAppointment.setFlag(2);
    fn_8019C34C();
}

// 8005BCCC
u32 dAcNpcNml_c::talk_c::getSellPrice(const dItem::Item *item) {
    u32 price = 10;
    if (item->isValid()) {
        price = item->getPrice();
    }
    int friendship = mpMemory != NULL ? mpMemory->getFriendship() : 0;
    u32 value = price * (255 - friendship) / 512;
    if (value < 10) {
        value = 10;
    }
    return value;
}

// 8005BD4C
u32 dAcNpcNml_c::talk_c::getBuyPrice(const dItem::Item *item) {
    if (!item->isValid()) {
        return 0;
    }
    if (mpMemory == NULL) {
        return 0;
    }
    u32 price = item->getPrice();
    if (price == 0) {
        return 0;
    }
    u32 value = price * (mpMemory->getFriendship() + 255) / 512 + 5;
    value -= value % 10;
    if (value < 10) {
        value = 10;
    }
    return value;
}

static const char l_Q09_Trade3[] = "Q09_Trade3";
static const char l_Q09_Trade4[] = "Q09_Trade4";

// 8005BE04
int dAcNpcNml_c::talk_c::msgInviteTrade(msgInfo_s *info) {
    dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    BOOL canBuy = FALSE;
    int idx = npc->getRoomFtrIdx(mFtrPosX, mFtrPosZ);
    mPrice = 10;
    if (npc->isTradeFtr(idx) && player != NULL && player->findEmptyPocket(0) != -1) {
        u32 price = getSellPrice(&mItem0);
        mPrice = price;
        BOOL enough = player->getPocketMoney() >= (int)price;
        if (enough) {
            canBuy = TRUE;
        }
    }
    stepFunc step = NULL;
    const char *label = l_Q09_Trade4;
    if (canBuy) {
        step = &talk_c::stepInviteTradeChoice;
        label = l_Q09_Trade3;
    }
    setLooksMsg(info, label, 0);
    setHookProc(&talk_c::endQuestCommon);
    if (step) {
        setStepProc(step);
    }
    if (mItem0.isValid()) {
        setItemName(&mItem0, 0);
    }
    if (mPrice > 0) {
        setNumber(mPrice, 3, 12, dScript::NUM_FORMAT_REGION);
    }
    return TRUE;
}

// 8005BFC0
BOOL dAcNpcNml_c::talk_c::stepInviteTradeChoice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoice(0, 0x42, 3, &talk_c::selInviteTradeYes);
        setChoice(1, 0x45, 3, &talk_c::selInviteTradeNo);
        setChoiceNum(2);
        setChoiceCancel(1);
        showChoice();
        return TRUE;
    }
    return FALSE;
}

// 8005C098
void dAcNpcNml_c::talk_c::selInviteTradeYes() {
    setMsgProc(&talk_c::msgInviteTradeYes);
    startMsg();
}

static const char l_Q09_TradeYes[] = "Q09_TradeYes";

// 8005C0EC
int dAcNpcNml_c::talk_c::msgInviteTradeYes(msgInfo_s *info) {
    setLooksMsg(info, l_Q09_TradeYes, 0);
    setHookProc(&talk_c::endInviteTradeYes);
    setStepProc(&talk_c::stepInviteTradeYes);
    return FALSE;
}

// 8005C178
void dAcNpcNml_c::talk_c::endInviteTradeYes(int arg) {
    dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
    int handle = fn_800A8F98(fn_800A9058(), mFtrPosX, mFtrPosZ, 0);
    mFtrHandle = handle;
    if (npc != NULL && npc->removeRoomFtr(handle, mFtrPosX, mFtrPosZ)) {
        dDemo_c *ctrl = getController();
        if (ctrl != NULL) {
            ctrl->mLock = 1;
        }
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player != NULL) {
            if (mPrice > 0) {
                player->payMoney(mPrice, FALSE);
            }
            if (mItem0.isValid()) {
                player->pickUp(&mItem0, FALSE);
            }
        }
        dSaveData_c::getTown()->mAnimals.mTown.mAppointment.setFlag(3);
        setActProc(&talk_c::actInviteTradeWait);
    }
}

// 8005C284
BOOL dAcNpcNml_c::talk_c::stepInviteTradeYes(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        fn_8019C35C();
        return TRUE;
    }
    return FALSE;
}

// 8005C2C8
void dAcNpcNml_c::talk_c::actInviteTradeWait() {
    if (!fn_800A9354(fn_800A93BC(), mFtrHandle)) {
        dDemo_c *ctrl = getController();
        if (ctrl != NULL) {
            ctrl->mLock = 0;
        }
        setActProc(NULL);
    }
}

// 8005C340
void dAcNpcNml_c::talk_c::selInviteTradeNo() {
    setMsgProc(&talk_c::msgInviteTradeNo);
    startMsg();
}

static const char l_Q09_TradeNo[] = "Q09_TradeNo";

// 8005C394
int dAcNpcNml_c::talk_c::msgInviteTradeNo(msgInfo_s *info) {
    setLooksMsg(info, l_Q09_TradeNo, 0);
    return FALSE;
}

static const char l_Q09_Wait[] = "Q09_Wait";

// 8005C3C4
int dAcNpcNml_c::talk_c::msgInviteWait(msgInfo_s *info) {
    setLooksMsg(info, l_Q09_Wait, 0);
    setHookProc(&talk_c::endQuestCommon);
    return TRUE;
}

static const char l_Q09_Analog[] = "Q09_Analog";

// 8005C428
int dAcNpcNml_c::talk_c::msgInviteAnalog(msgInfo_s *info) {
    setLooksMsg(info, l_Q09_Analog, 0);
    setHookProc(&talk_c::endQuestCommon);
    setStepProc(&talk_c::stepInvitePresentChoice);
    return TRUE;
}

// 8005C4B4
BOOL dAcNpcNml_c::talk_c::stepInvitePresentChoice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearNpcChoice();
        setNpcChoice(0x4A, 1, 0x4B, 1, &talk_c::selInvitePresent);
        showNpcChoice();
        return TRUE;
    }
    return FALSE;
}

// 8005C544
void dAcNpcNml_c::talk_c::selInvitePresent() {
    // pickAppointmentPresent2's flag by house rank (1..5) and answer
    static const u8 l_presentFlag[5][8] = {
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 1},
        {0, 0, 0, 0, 1, 1, 0, 0},
        {1, 1, 0, 0, 0, 0, 0, 0},
    };
    dQuestPlayerItem_c *appt;
    int res;
    u8 answer;
    s8 idx;
    u32 rank;
    dAnimal_c *animal;
    dPrivateData_c *player;
    player = dPlayerMgr_c::getCurrentPlayer();
    animal = getAnimal();
    appt = &dSaveData_c::getTown()->mAnimals.mTown.mAppointment;
    idx = appt->mAnimalIdx;
    rank = 0;
    dHR::getAnimalRank(idx, &rank);
    if (rank == 0) {
        rank = 1;
    } else if (rank > 5) {
        rank = 5;
    }
    answer = mAnswer;
    if (answer >= 8) {
        answer = 8;
    }
    dItem::Item item;
    const u8 *flags = l_presentFlag[rank - 1];
    res = pickAppointmentPresent2(&item, player, animal, flags[answer]);
    if (item.isValid()) {
        appt->mItem = item;
        if (res == 1 && animal->removeNewItem(&item)) {
            fn_800F0FE4(getNpcIdx(), &item);
        }
    }
    setMsgProc(&talk_c::msgInvitePresent);
    startMsg();
}

// 8005C698
int dAcNpcNml_c::talk_c::msgInvitePresent(msgInfo_s *info) {
    static const char l_label[] = "Q09_Analog";
    // message code by house rank (1..5) and answer
    static const u16 l_code[5][8] = {
        {10, 10, 10, 11, 11, 11, 12, 12},
        {10, 10, 10, 11, 11, 11, 12, 12},
        {10, 10, 10, 11, 11, 11, 12, 12},
        {7, 7, 7, 7, 8, 8, 9, 9},
        {4, 4, 5, 5, 6, 6, 6, 6},
    };
    getAnimal();
    s8 idx = dSaveData_c::getTown()->mAnimals.mTown.mAppointment.mAnimalIdx;
    u32 rank = 0;
    dHR::getAnimalRank(idx, &rank);
    if (rank == 0) {
        rank = 1;
    } else if (rank > 5) {
        rank = 5;
    }
    u8 answer = mAnswer;
    if (answer >= 8) {
        answer = 8;
    }
    const u16 *codes = l_code[rank - 1];
    setLooksMsg(info, l_label, codes[answer]);
    setStepProc(&talk_c::stepInviteBye);
    return TRUE;
}

// 8005C788
BOOL dAcNpcNml_c::talk_c::stepInviteBye(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgInviteBye);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

static const char l_Q09_Bye[] = "Q09_Bye";

// 8005C800
int dAcNpcNml_c::talk_c::msgInviteBye(msgInfo_s *info) {
    setLooksMsg(info, l_Q09_Bye, 0);
    return FALSE;
}
