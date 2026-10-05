// Bulletin board notices: posting BBS messages for dates, events and villagers, and the
// town board note count. .text 800EBB24..800ECD90 (no static initializer).
// Notes: notes/d_npc_notice.txt.
#include <game/game/d_npc_notice.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_letter.hpp>
#include <game/game/d_script.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_quest.hpp>
#include <game/cLib/c_math.hpp>
#include <game/cLib/c_lib.hpp>
#include <cstring>

// Dependencies whose owners are not recovered yet.
extern "C" {
// dLetter word helpers (TU around 800CB638).
void fn_800CB720(dScript::Word_c *word, u16 msgId, const char *group);
void fn_800CBAF0(int slot, int value);
void fn_800CBB50(int slot, u8 value);
void fn_800CBBB0(int slot, const dPersonalID_c *pid);
void fn_800CBC70(int slot, const dAnmPersonalID_c *animal);
void fn_800CBD30(int slot, u16 msgId, const char *group);

// dQuestTime_c -> dTime_c (unsplit TU 8014BD88..80153818).
dTime_c fn_8014C2C4(const dQuestTime_c *time);

// Event schedule (unsplit TU 80088AD4..8008BCCC).
BOOL fn_80089890(const int &event, const dTime_c *day);

// Save sync (unsplit TU 800CCC54..800DE0E4).
void fn_800DD4C8();
void fn_800DD518(const void *data, u32 size);
void fn_800DD588(int type, int arg);

// dAnimalBlock_c / dAnimal_c (d_animal).

// Misc.
bool fn_801C9C08();
}

// ---------------------------------------------------------------------------
// Bulletin board posting

static u32 sNoteCount;   // lbl_8074E558

// 800EBB24
void fn_800EBB24(u16 msgId, const char *group, const dTime_c *date, const void *sender) {
    static dLetter::Word_c sWord;

    sWord.clear();
    fn_800CB720(&sWord, msgId, group);
    dNotice_c note;

    dTime_c time = *dTime_c::getCurrent();
    if (date != NULL) {
        time = *date;
        if (time.year < 2000) {
            time.year += 36;
            time.normalize();
        }
    }
    note.init(&time);
    if (sender != NULL) {
        note.setSender((const wchar_t *)sender);
    }
    dScript::Word_c *word = &sWord;
    note.setText(word->getBuffer());
    note.setFlag20();
    dSaveData_c::getTown()->mNoticeBoard.add(&note);
    sNoteCount++;
}

// 800EBCF4
void fn_800EBCF4(const dNotice_c *note) {
    dSaveData_c::getTown()->mNoticeBoard.add(note);
    sNoteCount++;
}

// 800EBD3C
void fn_800EBD3C(u8 idx) {
    dSaveData_c::getTown()->mNoticeBoard.remove(idx);
}

// 800EBD78
dNotice_c *fn_800EBD78(int idx) {
    return dSaveData_c::getRaw()->mNoticeBoard.getNotice(idx);
}

// 800EBDB4
int fn_800EBDB4() {
    return dSaveData_c::getRaw()->mNoticeBoard.getNum();
}

// 800EBDE0
BOOL fn_800EBDE0() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL) {
        return TRUE;
    }

    dSaveData_c *save = dSaveData_c::getRaw();
    int playerIdx = dPrivateData_c::find(save->mPlayers, &player->mPID);
    if (playerIdx == -1) {
        return FALSE;
    }

    int num = fn_800EBDB4();
    for (int i = 0; i < num; i++) {
        if (!save->mNoticeBoard.getNotice(i)->isRead(playerIdx)) {
            return TRUE;
        }
    }
    return FALSE;
}

// 800EBEA4
void fn_800EBEA4(int idx) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dSaveData_c *save = dSaveData_c::getTown();
    int playerIdx = dPrivateData_c::find(save->mPlayers, &player->mPID);
    if (playerIdx != -1) {
        save->mNoticeBoard.setRead(idx, playerIdx);
    }
}

// 800EBF14
BOOL fn_800EBF14(int idx) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dSaveData_c *save = dSaveData_c::getRaw();
    int playerIdx = dPrivateData_c::find(save->mPlayers, &player->mPID);
    if (playerIdx == -1) {
        return TRUE;
    }
    return save->mNoticeBoard.getNotice(idx)->isRead(playerIdx);
}

// Fixed-date notices (lbl_80474DC8): month, day, message.
struct dNpcDateNotice_c {
    u8 mMonth;
    u8 mDay;
    u16 mMsgId;
};

static const dNpcDateNotice_c sDateNotices[12] = {
    {1, 1, 1},   {1, 15, 2},  {3, 15, 3},  {5, 6, 4},   {7, 1, 5},    {11, 12, 6},
    {12, 9, 7},  {2, 7, 14},  {12, 20, 21}, {12, 24, 22}, {12, 27, 23}, {12, 31, 24},
};

// 800EBF94
void fn_800EBF94(const dTime_c *day) {
    const dNpcDateNotice_c *notice = sDateNotices;
    for (int i = 0; i < 12; i++, notice++) {
        if (day->month + 1 == notice->mMonth && day->mday == notice->mDay) {
            fn_800EBB24(notice->mMsgId, "BBS_event", day, NULL);
        }
    }
    if (day->month + 1 == 2 && day->mday == 29) {
        fn_800EBB24(0x21, "BBS_event", day, NULL);
    }
}

// 800EC060
void fn_800EC060(const dTime_c *day) {
    for (int i = 0; i < 4; i++) {
        dPrivateData_c *player = dPlayerMgr_c::getPlayer(i);
        if (player->isBirthday(*day)) {
            fn_800CBBB0(1, &player->mPID);
            fn_800EBB24(cM::rndInt(4) + 0x1C, "BBS_event", day, NULL);
        }
    }

    for (int i = 0; i < 10; i++) {
        dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(i);
        if (animal != NULL && animal->mID.isValid() && day->month == (u8)animal->getBirthMonth() &&
            day->mday == (u8)animal->getBirthDay()) {
            fn_800CBC70(0, &animal->mID);
            fn_800EBB24(cM::rndInt(3) + 0x19, "BBS_event", day, NULL);
        }
    }
}

// Event notices (lbl_80474DF8): event id, which of sDays to check, message.
struct dNpcEventNotice_c {
    int mEvent;
    int mDay;
    u16 mMsgId;
};

static const dNpcEventNotice_c sEventNotices[31] = {
    {0x0E, 3, 0x08}, {0x0E, 1, 0x09}, {0x0F, 3, 0x0A}, {0x0F, 1, 0x0B},
    {0x11, 4, 0x0C}, {0x11, 1, 0x0D}, {0x18, 5, 0x0F}, {0x19, 4, 0x10},
    {0x19, 1, 0x35}, {0x10, 1, 0x12}, {0x1A, 1, 0x22}, {0x1B, 1, 0x23},
    {0x1C, 1, 0x24}, {0x2E, 0, 0x26}, {0x1D, 1, 0x27}, {0x1F, 1, 0x28},
    {0x20, 1, 0x29}, {0x21, 5, 0x2A}, {0x22, 5, 0x2B}, {0x23, 1, 0x2C},
    {0x27, 1, 0x2C}, {0x24, 0, 0x2D}, {0x25, 1, 0x2E}, {0x26, 0, 0x2F},
    {0x28, 0, 0x30}, {0x2B, 1, 0x31}, {0x29, 1, 0x32}, {0x2D, 0, 0x25},
    {0x2A, 1, 0x34}, {0x16, 2, 0x14}, {0x10, 4, 0x11},
};

static const dTime_c *sDays[10]; // lbl_805BF5D8

// 800EC19C
void fn_800EC19C(const dTime_c *day) {
    dTime_c d1 = *day;
    d1.add(1, 0, 0, 0);
    dTime_c d3 = *day;
    d3.add(3, 0, 0, 0);
    dTime_c d5 = *day;
    d5.add(5, 0, 0, 0);
    dTime_c d6 = *day;
    d6.add(6, 0, 0, 0);
    dTime_c d7 = *day;
    d7.add(7, 0, 0, 0);

    sDays[0] = day;
    sDays[1] = &d1;
    sDays[2] = &d3;
    sDays[3] = &d5;
    sDays[4] = &d6;
    sDays[5] = &d7;

    const dNpcEventNotice_c *notice = sEventNotices;
    for (int i = 0; i < 31; i++, notice++) {
        if (fn_80089890(notice->mEvent, sDays[notice->mDay])) {
            u16 msgId = notice->mMsgId;
            if (notice->mEvent == 0x2D && ((dSaveData_c::getTown()->_0735C2 >> 4) & 0xF) == 9) {
                msgId = 0x33;
            }
            if (notice->mEvent == 0x10 && notice->mDay == 4 && d6.mday > 7) {
                msgId = 0;
            }
            if (msgId != 0) {
                fn_800CBAF0(2, sDays[notice->mDay]->month);
                fn_800CBB50(3, sDays[notice->mDay]->mday);
                fn_800EBB24(msgId, "BBS_event", day, NULL);
            }
        }
    }
}

// 800EC4B0
void fn_800EC4B0(const dTime_c *day) {
    if (day->month == 9 && day->wday == 1 && day->mday >= 18 && day->mday <= 24) {
        fn_800EBB24(0x13, "BBS_event", day, NULL);
    }
}

// 800EC4F8: a notice three days before the shop changes (save+0x683D0).
void fn_800EC4F8(const dTime_c *day) {
    dQuestTime_c *time = (dQuestTime_c *)((u8 *)dSaveData_c::getTown() + 0x683D0);
    if (!fn_8014C384(time)) {
        dTime_c date = fn_8014C2C4(time);
        date.add(-3, 0, 0, 0);
        if (dTime_c::isSameDay(date, *day)) {
            fn_800CBD30(0, *(u32 *)((u8 *)dSaveData_c::getTown() + 0x630C0) + 0x2E, "sys_STRING/STR_Unit");
            u16 msgId = cM::rndInt(2) + 1;
            dTime_c when = fn_8014C2C4(time);
            fn_800CBAF0(2, when.month);
            fn_800CBB50(3, when.mday);
            fn_800EBB24(msgId, "BBS_tanukichi", day, NULL);
        }
    }
}

// 800EC72C
void fn_800EC72C(const dTime_c *day) {
    BOOL offline = !fn_801C9C08();
    if (offline) {
        fn_800EBB24(0x20, "BBS_event", day, NULL);
    }
}

// 800EC77C
void fn_800EC77C(const dTime_c *day) {
    fn_800EBF94(day);
    fn_800EC060(day);
    fn_800EC19C(day);
    fn_800EC4B0(day);
    fn_800EC4F8(day);
}

// 800EC7C8: posts the notices for every day since the board was last updated (up to 30).
void fn_800EC7C8(const dTime_c *now) {
    sNoteCount = 0;
    dSaveData_c *save = dSaveData_c::getTown();
    dTime_c today = *now;
    dTime_c last = save->mNoticeBoard.getTime();
    if (!dTime_c::isSameDay(last, today)) {
        if (dTime_c::isSameOrBeforeDay(last, today)) {
            last.add(1, 0, 0, 0);
            dTime_c limit = today;
            limit.add(-30, 0, 0, 0);
            if (dTime_c::isSameOrBeforeDay(last, limit)) {
                last = limit;
            }
            while (dTime_c::isSameOrBeforeDay(last, today)) {
                fn_800EC77C(&last);
                last.add(1, 0, 0, 0);
            }
            fn_800EC72C(now);
        } else {
            fn_800EC77C(now);
            fn_800EC72C(now);
        }
    }
    save->mNoticeBoard.setTime(now);
}

// 800ECC78
void fn_800ECC78() {
    if ((int)sNoteCount > 1) {
        dSaveData_c *town = dSaveData_c::getTown();
        town->mNoticeBoard.sort(sNoteCount);
    }
}

// 800ECCB4
void fn_800ECCB4(int unused, const void *data) {
    dNotice_c note;
    cLib::memCpy(&note, data, sizeof(dNotice_c));
    fn_800EBCF4(&note);
}

// 800ECD08
void fn_800ECD08(const void *data) {
    fn_800DD4C8();
    fn_800DD518(data, 0x19A);
    fn_800DD588(0x2F, 4);
}

// 800ECD4C
void fn_800ECD4C(int unused, const u8 *data) {
    fn_800EBD3C(*data);
}

// 800ECD54
void fn_800ECD54(u8 idx) {
    u8 data = idx;
    fn_800DD4C8();
    fn_800DD518(&data, 1);
    fn_800DD588(0x30, 4);
}
