// NPC / villager management. .text 800EBB24..800F59C8 (sinit 800F5768), built with -sym on.
// First-pass scaffold; see include/game/game/d_npc.hpp and notes/d_npc.txt.
#include <game/game/d_npc.hpp>
#include <game/game/d_random_field.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_script.hpp>
#include <game/cLib/c_math.hpp>
#include <game/cLib/c_lib.hpp>
#include <lib/egg/math/eggMath.h>
#include <lib/egg/core/eggHeap.h>
#include <game/game/d_dvd.hpp>
#include <revolution/OS/OSTime.h>
#include <cstring>
#include <game/game/d_ftr.hpp>
#include <game/game/d_letter.hpp>

// Dependencies whose owners are not recovered yet.
extern "C" {

// Save sync (unsplit TU 800CCC54..800DE0E4).
void fn_800DD4C8();
void fn_800DD518(const void *data, u32 size);
void fn_800DD588(int type, int arg);

// dAnimalBlock_c / dAnimal_c (d_animal).

// Save sync / net state (unsplit TU 800CCC54..800DE0E4).
BOOL fn_800DCEDC();
u32 fn_800DCF30();
BOOL fn_800DCF2C(int player);
int fn_800DCF58(); // own player index
void fn_800DD5F8(int id, int a, int b);
void *fn_800DD64C(int id);
u32 fn_800DD680(int id);
mVec3_c fn_801506F8(const void *data);

// Field map / actors.
dActor_c *fn_800F9860(int x, int z);
dActor_c *fn_800F98CC(int x, int z);
BOOL fn_80169FA4(mVec3_c *out, const dItem::Item *item, int i);
BOOL fn_80013550();
int fn_800C60B4(dItem::Item *out, int num, const void *table, int tableNum, const void *filter, const dItem::Item *exclude, int excludeNum, int);
BOOL fn_8016AE68(dScript::Word_c *word, u16 index, const char *group);
u16 fn_800F88AC(const dItem::Item *key);
dActor_c *fn_800F9878(const dItem::Item *key);
BOOL fn_80013F98(int x, int z, int arg);
void *fn_800F9F64(int *arg);
BOOL fn_800DD960();
}

// Villager hours (first file-scope data, so they lead .rodata).
struct dNpcHours_c {
    /* 0x0 */ u8 mStartHour;
    /* 0x1 */ u8 mStartMin;
    /* 0x2 */ u8 mEndHour;
    /* 0x3 */ u8 mEndMin;
};

static const dNpcHours_c sHoursA[6] = {
    {1, 30, 8, 0}, {2, 0, 6, 30}, {4, 30, 10, 0}, {1, 0, 5, 0}, {2, 30, 7, 0}, {3, 30, 9, 0},
};

static const dNpcHours_c sHoursB[6] = {
    {10, 0, 20, 0}, {9, 0, 22, 0}, {12, 0, 23, 0}, {8, 0, 19, 0}, {10, 0, 21, 0}, {11, 0, 22, 0},
};

// ---------------------------------------------------------------------------
// dNpc::msgMemory_c

namespace dNpc {

// 800ECD90
msgMemory_c::msgMemory_c() {}

// 800ECDA0
msgMemory_c::~msgMemory_c() {}

// 800ECDE0
void msgMemory_c::clear() {
    mMsgId = 0;
    memset(mGroup, 0, sizeof(mGroup));
    mKind = KIND_NONE;
    mTopicGroup = 0;
    mTopicIdx = 0;
}

// 800ECE34
BOOL msgMemory_c::isValid() {
    BOOL valid = FALSE;
    if (mMsgId != 0 && strlen(mGroup) != 0) {
        valid = TRUE;
    }
    return valid;
}

// 800ECE80
void msgMemory_c::set(const char *group, u16 msgId, u32 kind, u8 topicGroup, u8 topicIdx) {
    clear();
    strncpy(mGroup, group, sizeof(mGroup) - 1);
    mMsgId = msgId;
    mKind = kind;
    mTopicGroup = topicGroup;
    mTopicIdx = topicIdx;
}

// 800ECEF4
msgMemorySecond_c::msgMemorySecond_c() {
    mItem0 = dItem::ITEM_ID_NONE;
    mItem1 = dItem::ITEM_ID_NONE;
}

// 800ECF40
msgMemorySecond_c::~msgMemorySecond_c() {}

// 800ECF98
void msgMemorySecond_c::clear() {
    msgMemory_c::clear();
    _A0 = -1;
    mPersonal.clear();
    mPlayer.clear();
    mLand.clear();
    _A4 = -1;
    mItem0 = dItem::ITEM_ID_NONE;
    mItem1 = dItem::ITEM_ID_NONE;
    memset(mText, 0, sizeof(mText));
    _A8 = -1;
    _AC = 50;
}

// 800ED01C
BOOL msgMemorySecond_c::setA0(int value) {
    if (isValid()) {
        _A0 = value;
        return TRUE;
    }
    return FALSE;
}

// 800ED06C
BOOL msgMemorySecond_c::setPersonal(const dPersonalID_c *pid) {
    if (pid->isValid()) {
        mPersonal.copy(pid);
        return TRUE;
    }
    return FALSE;
}

// 800ED0C8
BOOL msgMemorySecond_c::setPlayer(const dPlayerID_c *player) {
    if (player->isValid()) {
        mPlayer.copy(player);
        return TRUE;
    }
    return FALSE;
}

// 800ED124
BOOL msgMemorySecond_c::setLand(const dLandID_c *land) {
    if (land->isValid()) {
        mLand.copy(land);
        return TRUE;
    }
    return FALSE;
}

// 800ED180
BOOL msgMemorySecond_c::setA4(u32 value) {
    if (value < 4) {
        _A4 = value;
        return TRUE;
    }
    return FALSE;
}

// 800ED19C
BOOL msgMemorySecond_c::setText(const wchar_t *text) {
    if (text != NULL) {
        memset(mText, 0, sizeof(mText));
        u32 len = dScript::getStringLength(text, 16, 0);
        if (len > 16) {
            len = 16;
        }
        memcpy(mText, text, len * sizeof(wchar_t));
        return TRUE;
    }
    return FALSE;
}

// 800ED224
BOOL msgMemorySecond_c::setAC(u32 value) {
    if (value < 50) {
        _AC = value;
        return TRUE;
    }
    return FALSE;
}

} // namespace dNpc

// ---------------------------------------------------------------------------
// dNpcTimer_c

// 800ED240
dNpcTimer_c::dNpcTimer_c() {}

// 800ED244
dNpcTimer_c::~dNpcTimer_c() {}

// 800ED284
void dNpcTimer_c::clear() {
    mEnd = 0;
    mMode = 0;
    mMinutes = 0;
}

// 800ED29C
void dNpcTimer_c::reset() {
    mEnd = 0;
    mMode = 5;
    mMinutes = 0;
}

// 800ED2B8
void dNpcTimer_c::addMinutes(u32 mins) {
    u32 total = mins + mMinutes;
    if (total > 0xFFFF) {
        total = 0xFFFF;
    }
    mMinutes = total;
}

// 800ED2D8: TODO the tick arithmetic is approximate.
void dNpcTimer_c::set(int mode, u16 mins) {
    if ((u32)mode >= 5) {
        mode = 0;
    }
    if (mode == 0) {
        mMode = mode;
        mMinutes = 0;
        mEnd = 0;
        return;
    }

    if (mode == getMode()) {
        addMinutes(mins);
    } else {
        mMode = mode;
        mMinutes = mins;
    }

    dTime_c now = *dTime_c::getCurrent();
    int frames = mMinutes;
    f32 ms = 1000.0f * ((f32)(frames % 60) / 60.0f);
    int msecs = ms;
    int usecs = 1000.0f * (ms - msecs);
    s64 ticks = OSCalendarTimeToTicks(&now);
    mEnd = ticks + OS_SEC_TO_TICKS(frames / 60) + OS_MSEC_TO_TICKS(msecs) + OS_USEC_TO_TICKS(usecs);
}

// 800ED4C0
void dNpcTimer_c::copy(const dNpcTimer_c *other) {
    set(((dNpcTimer_c *)other)->getMode(), other->mMinutes);
}

// 800ED50C
void dNpcTimer_c::tick() {
    if (mMinutes != 0) {
        mMinutes--;
    }
    if (mMinutes == 0 && mMode != 0) {
        mMode = 0;
    }
}

// 800ED544
int dNpcTimer_c::getMode() {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        return 0;
    }
    return mMode;
}

// 800ED590
void dNpcTimer_c::update() {
    if (mMode != 0) {
        dTime_c now = *dTime_c::getCurrent();
        s64 ticks = OSCalendarTimeToTicks(&now);
        if (mEnd <= ticks) {
            mEnd = 0;
            mMinutes = 0;
            mMode = 0;
        } else {
            mMinutes = (mEnd - ticks) * 60 / OS_TIME_SPEED;
        }
    }
}

// ---------------------------------------------------------------------------
// dNpcUnk288_c

// 800ED690
dNpcUnk288_c::dNpcUnk288_c() {}

// 800ED694
dNpcUnk288_c::~dNpcUnk288_c() {}

// 800ED6D4
void dNpcUnk288_c::clear() {
    mValue = 0;
}

// 800ED6E0
void dNpcUnk288_c::fn_800ED6E0(dAnimal_c *animal, dPrivateData_c *player) {
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayerRaw();
    }

    clear();
    add(fn_800ED7D4(10, 1, animal, player));
    if (animal->mID.isValid() && player != NULL &&
        dSaveData_c::getTown()->mAnimals.mTown.isMoveOutAnimal(&animal->mID)) {
        dAnimalMemory_c *memory = animal->findMemory2(&player->mPID);
        if (memory != NULL && memory->mPlayer.isValid() && !((*(u32 *)memory >> 23) & 1)) {
            set(32);
        }
    }
}

// 800ED7D4
int dNpcUnk288_c::fn_800ED7D4(u32 max, u32 min, dAnimal_c *animal, dPrivateData_c *player) {
    if (min > max) {
        return 0;
    }
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayerRaw();
    }
    if (!animal->mID.isValid() || player == NULL) {
        return 0;
    }

    dPersonalID_c *pid = &player->mPID;
    if (!pid->isValid() || !pid->isFromTown()) {
        return 0;
    }

    dAnimalMemory_c *memory = animal->findMemory2(pid);
    if (memory == NULL || !memory->mPlayer.isValid()) {
        return 0;
    }

    s8 friendship = memory->getFriendship();
    int value = min + cM::rndInt(max - min + 1);
    return (256.0f + friendship) * value * (1.0f / 128.0f);
}

// 800ED92C
void dNpcUnk288_c::set(u32 value) {
    if (value >= 32) {
        value = 32;
    }
    mValue = value;
}

// 800ED940
void dNpcUnk288_c::add(int delta) {
    int value = mValue + delta;
    if (value < 0) {
        value = 0;
    } else if ((u32)value >= 32) {
        value = 32;
    }
    set(value);
}

// 800ED964
void dNpcUnk288_c::half() {
    set((u32)mValue >> 1);
}

// 800ED970
void dNpcUnk288_c::twice() {
    set(mValue << 1);
}

// ---------------------------------------------------------------------------
// dNpcEntry_c

// 800ED97C
dNpcEntry_c::dNpcEntry_c() {}

// 800ED9D4
dNpcEntry_c::~dNpcEntry_c() {}

// 800EDA6C
void dNpcEntry_c::clear() {
    memset(&_28C, 0, sizeof(_28C));
    mMsg.clear();
    mMsg2.clear();
    _120.clear();
    mQuest.clear();
    mTimer.clear();
    mTime.init();
    mStallBuyCount = 0;
    mStallRefuseCount = 0;
    _288.clear();
}

// 800EDB00
dNpcUnk120_c::dNpcUnk120_c() {}

// 800EDB04
dNpcUnk120_c::~dNpcUnk120_c() {}

// 800EDB44
void dNpcUnk120_c::clear() {
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 20; j++) {
            _000[i][j] = 0;
        }
    }
    for (int i = 0; i < 6; i++) {
        _118[i] = -1;
    }
}

// 800EDBD0
void dNpcUnk120_c::set(u32 i, u16 value, u32 j) {
    if (i < 7 && j < 20) {
        _000[i][j] = value;
    }
}

// 800EDBF4
u16 dNpcUnk120_c::get(u32 i, u32 j) {
    if (i < 7 && j < 20) {
        return _000[i][j];
    }
    return 0;
}

// 800EDC20
int dNpcUnk120_c::getIdx(u32 i) {
    if (i < 10) {
        return _118[i];
    }
    return -1;
}

// 800EDC40
void dNpcUnk120_c::setIdx(u32 i, s8 value) {
    if (i < 10) {
        _118[i] = value;
    }
}

// 800EDC54: the villager remembered in slot i, or a random new one other than `exclude`.
dAnimal_c *dNpcUnk120_c::fn_800EDC54(u32 i, const dAnmPersonalID_c **exclude, u32 numExclude) {
    if (i >= 10) {
        return NULL;
    }

    dAnimalBlock_c *block = &dSaveData_c::getTown()->mAnimals.mTown;
    dAnimal_c *animal = block->getAnimal(getIdx(i));
    if (animal == NULL || !animal->mID.isValid()) {
        u32 num = 0;
        const dAnmPersonalID_c *list[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
        const dAnmPersonalID_c **p = list;
        if (exclude != NULL) {
            u32 n = numExclude < 2 ? numExclude : 2;
            for (u32 k = 0; k < n; k++, exclude++) {
                if (*exclude != NULL && num < 8) {
                    *p++ = *exclude;
                    num++;
                }
            }
        }

        p = &list[num];
        for (u32 k = 0; k < 10; k++) {
            if (k != i) {
                dAnimal_c *other = block->getAnimal(getIdx(k));
                if (other != NULL && other->mID.isValid() && num < 8) {
                    num++;
                    *p++ = &other->mID;
                }
            }
        }

        animal = block->pickRandomAnimal(list, num, 0);
        if (animal != NULL) {
            u32 idx = block->getAnimalIdx(&animal->mID);
            if (idx < 10) {
                setIdx(i, idx);
            }
        }
    }
    return animal;
}

// ---------------------------------------------------------------------------
// dNpc::list_c / dNpc::listLand_c

namespace dNpc {

// 800EDE0C
void list_c::clear() {
    dNpcEntry_c *entry = mEntries;
    if (entry != NULL) {
        for (u32 i = 0; i < mNum; i++, entry++) {
            entry->clear();
        }
    }
}

// 800EDE74
void list_c::init() {
    clear();
}

// 800EDE84
dNpcEntry_c *list_c::get(u32 i) {
    if (i < mNum) {
        return &mEntries[i];
    }
    return NULL;
}

// 800EDEA8
void listLand_c::clear() {
    list_c::clear();
    mTime.init();
}

// 800EDEDC
void listLand_c::setTimeNow() {
    memcpy(&mTime, dTime_c::getCurrent(), sizeof(dTime_c));
}

} // namespace dNpc

// ---------------------------------------------------------------------------
// Villager compatibility

// 800EDF18: personality x personality (lbl_80474FA0).
u8 fn_800EDF18(u32 a, u32 b) {
    static const u8 sTable[6][6] = {
        {0x28, 0x60, 0x08, 0x08, 0x28, 0x60},
        {0x60, 0x08, 0x28, 0x28, 0x60, 0x08},
        {0x08, 0x28, 0x60, 0x60, 0x08, 0x28},
        {0x08, 0x28, 0x60, 0x28, 0x08, 0x60},
        {0x28, 0x60, 0x08, 0x08, 0x60, 0x28},
        {0x60, 0x08, 0x28, 0x60, 0x28, 0x08},
    };
    if (a < 6 && b < 6) {
        return sTable[a][b];
    }
    return 0;
}

// 800EDF48: star sign -> element (lbl_80474FC4).
u8 fn_800EDF48(u32 sign) {
    static const u8 sTable[12] = {1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0};
    if (sign < 12) {
        return sTable[sign];
    }
    return 4;
}

// 800EDF68: element x element (lbl_80474FD0).
u8 fn_800EDF68(u32 signA, u32 signB) {
    static const u8 sTable[4][4] = {
        {0x80, 0x30, 0x30, 0x00},
        {0x30, 0x80, 0x00, 0x30},
        {0x30, 0x00, 0x80, 0x30},
        {0x00, 0x30, 0x30, 0x80},
    };
    u32 a = fn_800EDF48(signA);
    u32 b = fn_800EDF48(signB);
    if (a < 4 && b < 4) {
        return sTable[a][b];
    }
    return 0;
}

// 800EDFD4
u8 fn_800EDFD4(int monthA, int dayA, int monthB, int dayB) {
    u32 a = dTime_c::getStarSign(monthA, dayA);
    return fn_800EDF68(a, dTime_c::getStarSign(monthB, dayB));
}

// 800EE030: finds the unordered pair (a, b) in `pairs`.
const u8 *fn_800EE030(u32 a, u32 b, const u8 *pairs, int num) {
    if (a < 0x21 && b < 0x21) {
        const u8 *end = pairs + num * 2;
        for (; pairs != end; pairs += 2) {
            if ((pairs[0] == a && pairs[1] == b) || (pairs[0] == b && pairs[1] == a)) {
                return pairs;
            }
        }
    }
    return NULL;
}

// 800EE094: species x species.
u32 fn_800EE094(u32 a, u32 b) {
    static const u8 sPairs[7][2] = {
        {0x04, 0x09}, {0x0D, 0x0E}, {0x1D, 0x15}, {0x03, 0x19}, {0x05, 0x0A}, {0x12, 0x02}, {0x13, 0x00},
    };
    static const u8 sBadPairs[4][2] = {{0x04, 0x1B}, {0x04, 0x20}, {0x09, 0x02}, {0x00, 0x0A}};

    u32 score = 0;
    if (a < 0x21 && b < 0x21) {
        if (fn_800EE030(a, b, sPairs[0], 7)) {
            score = 0x80;
        } else if (a == b) {
            score = 0x40;
        } else if (!fn_800EE030(a, b, sBadPairs[0], 4)) {
            score = 0x20;
        }
    }
    return score;
}

// 800EE138: compatibility score between two villagers.
u32 fn_800EE138(dAnimal_c *a, dAnimal_c *b) {
    u32 score = 0;
    if (a != NULL && b != NULL) {
        dAnmPersonalID_c *idA = &a->mID;
        dAnmPersonalID_c *idB = &b->mID;
        if (idA->isValid() && idB->isValid()) {
            u8 looksB = idB->getLooks(1);
            u32 looks = fn_800EDF18(idA->getLooks(1), looksB);
            u8 dayB = b->getBirthDay();
            u8 monthB = b->getBirthMonth();
            u8 dayA = a->getBirthDay();
            u32 signs = fn_800EDFD4((u8)a->getBirthMonth(), dayA, monthB, dayB);
            u8 speciesB = b->getSpecies();
            score = fn_800EE094((u8)a->getSpecies(), speciesB) + (signs + looks);
        }
    }
    return score;
}

// 800EE244: 0 = good, 1 = okay, 2 = bad.
int fn_800EE244(dAnimal_c *a, dAnimal_c *b) {
    int rank = 2;
    u32 score = fn_800EE138(a, b);
    if (score >= 200) {
        rank = 0;
    } else if (score >= 61) {
        rank = 1;
    }
    return rank;
}

// ---------------------------------------------------------------------------
// dNpcAnimal_c / dNpcAnimalBuf_c: four spare villagers (lbl_805C1000).

// 800EE290
dNpcAnimal_c::dNpcAnimal_c() {}

// 800EE2C0
dNpcAnimal_c::~dNpcAnimal_c() {}

// 800EE318
void dNpcAnimal_c::clear() {
    mAnimal.clear();
}

// 800EE31C
dNpcAnimalBuf_c::dNpcAnimalBuf_c() {
    clear();
}

// 800EE36C
dNpcAnimalBuf_c::~dNpcAnimalBuf_c() {}

// 800EE3D0
void dNpcAnimalBuf_c::clear() {
    dNpcAnimal_c *animal = mAnimals;
    for (int i = 0; i < 4; i++, animal++) {
        animal->clear();
    }
}

// 800EE41C
BOOL dNpcAnimalBuf_c::isValidIdx(u32 i) {
    return i < 4;
}

// 800EE434
dNpcAnimal_c *dNpcAnimalBuf_c::get(int i) {
    return &mAnimals[i];
}

// 800EE440
BOOL dNpcAnimalBuf_c::fn_800EE440(int arg) {
    dSaveTown_c *save = dSaveData_c::getTown();
    dNpcAnimal_c *animal = get(0);
    animal->clear();
    return save->mAnimals.takeMovedAnimal(&animal->mAnimal, arg);
}

// 800EE4AC
BOOL dNpcAnimalBuf_c::fn_800EE4AC() {
    dSaveTown_c *save = dSaveData_c::getTown();
    dNpcAnimal_c *animal = get(0);
    BOOL result = save->mAnimals.addMovedAnimal(&animal->mAnimal);
    animal->clear();
    return result;
}

// 800EE514
void dNpcAnimalBuf_c::fn_800EE514() {
    dNpcAnimal_c *first = get(0);
    dAnimalSave_c *animals = &dSaveData_c::getTown()->mAnimals;
    dNpcAnimal_c *second = get(1);
    BOOL moved = animals->takeMovedAnimal(&second->mAnimal, 1);
    animals->addMovedAnimal(&first->mAnimal);
    first->clear();
    if (moved) {
        cLib::memCpy(first, second, sizeof(dNpcAnimal_c));
        second->mAnimal.clear();
    }
}

// 800EE5C4
BOOL dNpcAnimalBuf_c::fn_800EE5C4(const dAnimal_c *src, u32 idx) {
    if (!isValidIdx(idx)) {
        return FALSE;
    }

    u32 pick = 4;
    u32 count = 0;
    for (int i = 0; i < 4; i++) {
        if (i != idx && fn_800DCF2C(i)) {
            f32 chance = 100.0f / (count + 1);
            if (cM::rndF(100.0f) <= chance) {
                pick = i;
            }
            count++;
        }
    }

    if (!isValidIdx(pick)) {
        return FALSE;
    }

    dNpcAnimal_c *picked = get(pick);
    cLib::memCpy(get(idx), picked, sizeof(dNpcAnimal_c));
    if (src != NULL) {
        cLib::memCpy(picked, src, sizeof(dNpcAnimal_c));
    } else {
        picked->clear();
    }
    return TRUE;
}

// 800EE71C
BOOL dNpcAnimalBuf_c::fn_800EE71C(const dAnimal_c *src) {
    if (src == NULL) {
        return FALSE;
    }
    cLib::memCpy(get(0), src, sizeof(dNpcAnimal_c));
    return TRUE;
}

// 800EE76C
u32 dNpcAnimalBuf_c::fn_800EE76C(dAnimal_c *dst, u32 idx) {
    if (dst == NULL) {
        return 0;
    }
    if (!isValidIdx(idx)) {
        return 0;
    }

    dNpcAnimal_c *animal = get(idx);
    if (animal->mAnimal.mID.isValid()) {
        cLib::memCpy(dst, animal, sizeof(dNpcAnimal_c));
        animal->clear();
        return sizeof(dNpcAnimal_c);
    }
    return 0;
}

// 800EE818
dNpcAnimal_c *dNpcAnimalBuf_c::fn_800EE818(u32 idx) {
    if (!isValidIdx(idx)) {
        return NULL;
    }

    dNpcAnimal_c *animal = get(idx);
    if (animal->mAnimal.mID.isValid()) {
        return animal;
    }
    return NULL;
}

// 800EE890
BOOL dNpcAnimalBuf_c::fn_800EE890(u32 idx) {
    if (!isValidIdx(idx)) {
        return FALSE;
    }
    get(idx)->clear();
    return TRUE;
}

// ---------------------------------------------------------------------------
// dNpcDaub_c: a 0x2F-byte packed record shared over the network.

// 800EE8F0
dNpcDaub_c::dNpcDaub_c() {}

// 800EE8F4
dNpcDaub_c::dNpcDaub_c(const dNpcDaub_c &other) {
    copy(&other);
}

// 800EE924
dNpcDaub_c::~dNpcDaub_c() {}

// 800EE964
void dNpcDaub_c::clear() {
    memset(this, 0, sizeof(*this));
    _0A = 4;
    _0B = 4;
    _0C = 0x44;
}

// 800EE9AC
void dNpcDaub_c::copy(const dNpcDaub_c *other) {
    cLib::memCpy(_00, other->_00, 4);
    cLib::memCpy(_04, other->_04, 4);
    cLib::memCpy(_08, other->_08, 2);
    cLib::memCpy(&_0A, &other->_0A, 1);
    cLib::memCpy(&_0B, &other->_0B, 1);
    cLib::memCpy(&_0C, &other->_0C, 1);
    cLib::memCpy(_0D, other->_0D, 2);
    cLib::memCpy(_0F, other->_0F, 2);
    cLib::memCpy(_11, other->_11, 8);
    cLib::memCpy(_19, other->_19, 0x15);
    mFlag = other->mFlag;
}

// 800EEA88: packs x/z of a vector into 8 bytes.
void dNpcDaub_c::packXZ(u8 *dst, const mVec3_c *src) {
    cLib::memCpy(dst, &src->x, 4);
    cLib::memCpy(dst + 4, &src->z, 4);
}

// 800EEAD4
void dNpcDaub_c::unpackXZ(const u8 *src, mVec3_c *dst) {
    cLib::memCpy(&dst->x, src, 4);
    cLib::memCpy(&dst->z, src + 4, 4);
}

// 800EEB28
void dNpcDaub_c::set08(const u16 *value) {
    cLib::memCpy(_08, value, 2);
}

// 800EEB34
mAng dNpcDaub_c::get08() {
    mAng value;
    cLib::memCpy(&value, _08, 2);
    return value;
}

// 800EEB6C
void dNpcDaub_c::set0D(mAng value) {
    cLib::memCpy(_0D, &value, 2);
}

// 800EEB78
mAng dNpcDaub_c::get0D() {
    mAng value;
    cLib::memCpy(&value, _0D, 2);
    return value;
}

// 800EEBB0
void dNpcDaub_c::set0F(mAng value) {
    cLib::memCpy(_0F, &value, 2);
}

// 800EEBBC
mAng dNpcDaub_c::get0F() {
    mAng value;
    cLib::memCpy(&value, _0F, 2);
    return value;
}

// 800EEBF4
void dNpcDaub_c::get11(void *dst, u32 size) {
    if (dst != NULL && size != 0) {
        cLib::memCpy(dst, _11, size > 8 ? 8 : size);
    }
}

// 800EEC2C
void dNpcDaub_c::set11(const void *src, u32 size) {
    if (src != NULL && size != 0) {
        cLib::memCpy(_11, src, size > 8 ? 8 : size);
    }
}

// 800EEC5C
void dNpcDaub_c::get19(u32 *a, u32 *b) {
    if (a != NULL) {
        *a = _19[0];
    }
    if (b != NULL) {
        *b = _19[1];
    }
}

// 800EEC80
void dNpcDaub_c::set19(u32 a, u32 b) {
    _19[0] = a;
    _19[1] = b;
}

// 800EEC8C
void dNpcDaub_c::getMove(mVec3_c *from, mVec3_c *to, u16 *angle, f32 *speed) {
    if (from != NULL) {
        cLib::memCpy(&from->x, &_19[2], 4);
        cLib::memCpy(&from->z, &_19[6], 4);
    }
    if (to != NULL) {
        cLib::memCpy(&to->x, &_19[10], 4);
        cLib::memCpy(&to->z, &_19[14], 4);
    }
    if (angle != NULL) {
        cLib::memCpy(angle, &_19[18], 2);
    }
    if (speed != NULL) {
        *speed = _19[20];
    }
}

// 800EED5C
void dNpcDaub_c::setMove(const mVec3_c *from, const mVec3_c *to, const u16 *angle, f32 speed) {
    cLib::memCpy(&_19[2], &from->x, 4);
    cLib::memCpy(&_19[6], &from->z, 4);
    cLib::memCpy(&_19[10], &to->x, 4);
    cLib::memCpy(&_19[14], &to->z, 4);
    cLib::memCpy(&_19[18], angle, 2);
    _19[20] = speed;
}

// 800EEE10
void dNpcDaub_c::getTalk(u16 *a, u16 *b, f32 *c) {
    if (a != NULL) {
        cLib::memCpy(a, &_19[2], 2);
    }
    if (b != NULL) {
        cLib::memCpy(b, &_19[4], 2);
    }
    if (c != NULL) {
        *c = _19[6];
    }
}

// 800EEEA8
void dNpcDaub_c::setTalk(const u16 *a, const u16 *b, f32 c) {
    cLib::memCpy(&_19[2], a, 2);
    cLib::memCpy(&_19[4], b, 2);
    _19[6] = c;
}

// 800EEF14
void dNpcDaub_c::getAct(u32 *a, u8 *b, f32 *c, f32 *d) {
    if (a != NULL) {
        u16 value = 0;
        cLib::memCpy(&value, &_19[2], 2);
        *a = value;
    }
    if (b != NULL) {
        *b = _19[4];
    }
    if (c != NULL) {
        *c = _19[5];
    }
    if (d != NULL) {
        *d = _19[6];
    }
}

// 800EEFDC
void dNpcDaub_c::setAct(u32 a, u8 b, f32 c, f32 d) {
    u16 value = a;
    cLib::memCpy(&_19[2], &value, 2);
    _19[4] = b;
    _19[5] = c;
    _19[6] = d;
}

// ---------------------------------------------------------------------------
// dNpc::daubMng_c

namespace dNpc {

// 800EF060
daubMng_c::daubMng_c(dNpcDaub_c *daubs, u32 num) : mDaubs(daubs), mNum(num) {}

} // namespace dNpc

namespace dNpc {

// 800EF078
daubMng_c::~daubMng_c() {
    mDaubs = NULL;
    mNum = 0;
}

// 800EF0C4
void daubMng_c::clear() {
    dNpcDaub_c *daub = mDaubs;
    for (u32 i = 0; i < mNum; i++, daub++) {
        daub->clear();
    }
}

// 800EF124
void daubMng_c::init() {
    clear();
}

// 800EF128
BOOL daubMng_c::isValidIdx(u32 idx) {
    return idx < mNum;
}

// 800EF140
BOOL daubMng_c::isValidKey(const u16 *key) {
    return isValidIdx(getIdx(key));
}

// 800EF184
BOOL daubMng_c::isValidId(int id) {
    return isValidIdx(idToIdx(id));
}

// 800EF1BC
int daubMng_c::keyToId(const u16 *key) {
    u32 idx = getIdx(key);
    if (isValidIdx(idx)) {
        return getBase() + idx;
    }
    return 0xBA;
}

// 800EF234
int daubMng_c::idToIdx(int id) {
    int idx = id - getBase();
    if (!isValidIdx(idx)) {
        return -1;
    }
    return idx;
}

// 800EF298
dNpcDaub_c *daubMng_c::get(u32 idx) {
    if (isValidIdx(idx)) {
        return &mDaubs[idx];
    }
    return NULL;
}

// 800EF2EC
dNpcDaub_c *daubMng_c::getByKey(const u16 *key) {
    return get(getIdx(key));
}

// 800EF330
dNpcDaub_c *daubMng_c::getById(int id) {
    return get(idToIdx(id));
}

// 800EF368
daubMngNormal_c::daubMngNormal_c() : daubMng_c(mList, 10) {}

// 800EF3CC
daubMngNormal_c::~daubMngNormal_c() {}

static inline BOOL isNormalKeyA(u16 key) {
    BOOL result = FALSE;
    if (((key >> 12) & 0xF) == 0xE && (key & 0x800)) {
        result = TRUE;
    }
    return result;
}

static inline BOOL isNormalKeyB(u16 key) {
    BOOL result = FALSE;
    if (((key >> 12) & 0xF) == 0xE && (key & 0x400)) {
        result = TRUE;
    }
    return result;
}

// Villager keys: type 0xE without the 0x800 and 0x400 flags.
static inline BOOL isNormalKey(u16 key) {
    BOOL type = FALSE;
    BOOL valid = FALSE;
    if (((key >> 12) & 0xF) == 0xE && !isNormalKeyA(key)) {
        type = TRUE;
    }
    if (type && !isNormalKeyB(key)) {
        valid = TRUE;
    }
    return valid;
}

// 800EF440
int daubMngNormal_c::getIdx(const u16 *key) {
    if (!isNormalKey(*key)) {
        return -1;
    }
    return *key & 0xFFF;
}

// 800EF4B8: places each normal daub at its villager's house.
void daubMngNormal_c::fn_800EF4B8() {
    dAnimalBlock_c *block = &dSaveData_c::getTown()->mAnimals.mTown;
    for (u32 i = 0; i < mNum; i++) {
        dNpcDaub_c *daub = get(i);
        if (daub != NULL) {
            mVec3_c pos = mVec3_c::Zero;
            if (block->getHouseFrontPos(&pos, i)) {
                dNpcDaub_c::packXZ(daub->_00, &pos);
                daub->_0C = 0;
                daub->mFlag = 1;
            }
        }
    }
}

// 800EF57C
daubMngSpecial_c::daubMngSpecial_c() : daubMng_c(mList, 97) {}

// 800EF5E0
daubMngSpecial_c::~daubMngSpecial_c() {}

// 800EF654
int daubMngSpecial_c::getIdx(const u16 *key) {
    u16 k = *key;
    if (((k >> 12) & 0xF) == 8) {
        return k & 0xFFF;
    }
    return -1;
}

} // namespace dNpc

// 800EF670: finds the placement whose key matches in layout `layout`.
BOOL fn_800EF670(mVec3_c *pos, s16 *angle, const dItem::Item &key, u8 layout) {
    dNpcLayout_c *data = (dNpcLayout_c *)getSceneData(layout);
    if (data != NULL) {
        int num = data->mNum;
        for (int i = 0; i < num; i++) {
            dNpcLayoutGroup_c *group = &data->mGroups[i];
            if (group->mType == 0 && group->mEntries != NULL) {
                dNpcLayoutEntry_c *entry = group->mEntries;
                for (int j = 0; j < group->mCount; j++, entry++) {
                    if (entry->mKey == key.mId) {
                        dNpcLayoutEntry_c *found = &group->mEntries[j];
                        pos->x = found->mPos.x;
                        pos->y = found->mPos.y;
                        pos->z = found->mPos.z;
                        *angle = found->mAngle;
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}

static dNpc::listLand_c sLandList;            // lbl_805BF610
static dNpcAnimalBuf_c sAnimalBuf;            // lbl_805C1000
static dNpc::daubMngNormal_c sDaubNormal;   // lbl_805CD10C
static dNpc::daubMngSpecial_c sDaubSpecial; // lbl_805CD2FC

namespace dNpc {

// 800EF768: places each special NPC's daub at its spot.
void daubMngSpecial_c::fn_800EF768() {
    static const dItem::Item sKeys[19] = {
        0x801E, 0x8006, 0x8024, 0x8001, 0x8019, 0x801A, 0x801D, 0x8026, 0x8027, 0x8002,
        0x800F, 0x8010, 0x8005, 0x8007, 0x8017, 0x8008, 0x8009, 0x800D, 0x800E,
    };

    const dItem::Item *key = sKeys;
    for (u32 i = 0; i < 19; i++, key++) {
        dNpcDaub_c *daub = getByKey(&key->mId);
        if (daub != NULL) {
            mVec3_c pos = mVec3_c::Zero;
            s16 angle = cM::rndInt(0xFFFF);
            u16 id = key->mId;
            switch (id) {
                case 0x801E:
                    if ((*((u8 *)dSaveData_c::getTown() + 0x66747) >> 6) & 1) {
                        pos = fn_801506F8((u8 *)dSaveData_c::getTown() + 0x66745);
                    } else {
                        fn_800F3830(&pos, NULL, 4, NULL, 0x400, TRUE, TRUE, 0.0f);
                    }
                    angle = 0;
                    break;
                case 0x8019:
                case 0x801A:
                    fn_800F28AC(&pos);
                    break;
                case 0x8026:
                case 0x8027:
                    fn_800EF670(&pos, &angle, id, 0x1C);
                    break;
                case 0x8002:
                    fn_800EF670(&pos, &angle, id, 0x27);
                    break;
                case 0x800F:
                case 0x8010:
                    fn_800EF670(&pos, &angle, id, 0x2B);
                    break;
                case 0x8005:
                    fn_800EF670(&pos, &angle, id, 0x1E);
                    break;
                case 0x8007:
                case 0x8017:
                    fn_800EF670(&pos, &angle, id, 0x25);
                    break;
                case 0x8008:
                case 0x8009:
                    fn_800EF670(&pos, &angle, id, 0x2C);
                    break;
                case 0x800D:
                    fn_800EF670(&pos, &angle, id, 0x1B);
                    break;
                case 0x800E:
                    fn_800EF670(&pos, &angle, id, 0x26);
                    break;
                default:
                    fn_800F2644(&pos, NULL, 4, NULL, 0.0f);
                    break;
            }
            dNpcDaub_c::packXZ(daub->_00, &pos);
            daub->set08((u16 *)&angle);
            daub->_0C = 0;
            daub->mFlag = 1;
        }
    }
}

} // namespace dNpc

// ---------------------------------------------------------------------------
// Daub manager access

static dNpc::daubMng_c *sDaubMngs[2] = {&sDaubNormal, &sDaubSpecial}; // lbl_8074AE88

// 800EFBBC
dNpc::daubMng_c *fn_800EFBBC(u32 i) {
    if (i < 2) {
        return sDaubMngs[i];
    }
    return NULL;
}

// 800EFBDC
dNpc::daubMng_c *fn_800EFBDC(const u16 *key) {
    for (int i = 0; i < 2; i++) {
        dNpc::daubMng_c *mng = fn_800EFBBC(i);
        if (mng != NULL && mng->isValidKey(key)) {
            return mng;
        }
    }
    return NULL;
}

// 800EFC54
dNpc::daubMng_c *fn_800EFC54(int id) {
    for (int i = 0; i < 2; i++) {
        dNpc::daubMng_c *mng = fn_800EFBBC(i);
        if (mng != NULL && mng->isValidId(id)) {
            return mng;
        }
    }
    return NULL;
}

// 800EFCCC
void fn_800EFCCC() {
    sDaubNormal.init();
    sDaubSpecial.init();
    fn_800DCEDC();
}

// 800EFD04
dNpcDaub_c *fn_800EFD04(const u16 *key) {
    dNpc::daubMng_c *mng = fn_800EFBDC(key);
    if (mng != NULL) {
        return mng->getByKey(key);
    }
    return NULL;
}

// 800EFD48
void fn_800EFD48() {
    dNpc::daubMngNormal_c *mng = &sDaubNormal;
    mng->fn_800EF4B8();
    if (fn_800DCEDC()) {
        for (int i = 0; i < 10; i++) {
            u16 key = (i & 0x3FF) + 0xE000;
            int id = mng->keyToId(&key);
            if (id != 0xBA) {
                fn_800DD5F8(id, 0, 0);
            }
        }
    }
}

// 800EFDD0
void fn_800EFDD0(void *dst, int id) {
    fn_800EFEDC(dst, id);
}

// 800EFDD4
void fn_800EFDD4() {
    dNpc::daubMngSpecial_c *mng = &sDaubSpecial;
    mng->fn_800EF768();
    if (fn_800DCEDC()) {
        u32 num = mng->mNum;
        for (u32 i = 0; i < num; i++) {
            u16 key = (i & 0xFFF) | 0x8000;
            int id = mng->keyToId(&key);
            if (id != 0xBA) {
                fn_800DD5F8(id, 0, 0);
            }
        }
    }
}

// 800EFE68
void fn_800EFE68(void *dst, int id) {
    fn_800EFEDC(dst, id);
}

// 800EFE6C
const dNpcDaub_c *fn_800EFE6C(const u16 *key) {
    if (!fn_800DCEDC()) {
        return NULL;
    }
    dNpc::daubMng_c *mng = fn_800EFBDC(key);
    if (mng == NULL) {
        return NULL;
    }
    int id = mng->keyToId(key);
    if (id == 0xBA) {
        return NULL;
    }
    return (const dNpcDaub_c *)fn_800DD64C(id);
}

// 800EFEDC
void fn_800EFEDC(void *dst, int id) {
    dNpc::daubMng_c *mng = fn_800EFC54(id);
    if (mng != NULL) {
        dNpcDaub_c *daub = mng->getById(id);
        if (daub != NULL) {
            daub->mFlag = 1;
            cLib::memCpy(dst, daub, fn_800DD680(id));
        }
    }
}

// 800EFF60
void fn_800EFF60(int *idx, void *dst, const u8 *src) {
    if (src[2] < 10) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    cLib::memCpy(dst, src, 2);
}

// 800EFF8C
void fn_800EFF8C(u8 *dst, int idx, const void *src) {
    dst[2] = idx;
    cLib::memCpy(dst, src, 2);
}

// 800EFFA0
void fn_800EFFA0(u32 idx, const void *data) {
    if (fn_800DCEDC() && fn_800DCF30() > 1 && idx < 10) {
        u8 buf[3];
        fn_800EFF8C(buf, idx, data);
        fn_800DD4C8();
        fn_800DD518(buf, 3);
        fn_800DD588(0x55, 4);
    }
}

// ---------------------------------------------------------------------------
// Network messages: a parse function (packet -> values), a build function (values -> packet)
// and a send function for each message type. Index values are villager slots (< 10).

// 800F0020
void fn_800F0020(int *idx, u8 *value, const u8 *src) {
    if (src[1] < 10) {
        *idx = src[1];
    } else {
        *idx = -1;
    }
    *value = src[0];
}

// 800F0048
void fn_800F0048(u8 *dst, int idx, int value) {
    dst[0] = value;
    dst[1] = idx;
}

// 800F0054
void fn_800F0054(u32 idx, u8 value) {
    if (fn_800DCEDC() && fn_800DCF30() > 1 && idx < 10) {
        u8 buf[2];
        fn_800F0048(buf, idx, value);
        fn_800DD4C8();
        fn_800DD518(buf, 2);
        fn_800DD588(0x56, 4);
    }
}

// 800F00D4
void fn_800F00D4(int *idx, u8 *value, const u8 *src) {
    if (src[1] < 10) {
        *idx = src[1];
    } else {
        *idx = -1;
    }
    *value = src[0];
}

// 800F00FC
void fn_800F00FC(u8 *dst, int idx, int value) {
    dst[1] = idx;
    dst[0] = value;
}

// 800F0108
void fn_800F0108(u32 idx, u8 value) {
    if (fn_800DCEDC() && fn_800DCF30() > 1 && idx < 10) {
        u8 buf[2];
        fn_800F00FC(buf, idx, value);
        fn_800DD4C8();
        fn_800DD518(buf, 2);
        fn_800DD588(0x57, 4);
    }
}

// 800F0188
void fn_800F0188(int *idx, int *kind, u8 *count, u16 *msg, u8 *flag, const u8 *src) {
    if (src[4] < 10) {
        *idx = src[4];
    } else {
        *idx = -1;
    }
    if (src[0] < 16) {
        *kind = src[0];
    } else {
        *kind = -1;
    }
    if (src[1] < 50) {
        *count = src[1];
    } else {
        *count = 50;
    }
    *msg = src[3];
    *flag = src[2];
}

// 800F01F0
void fn_800F01F0(u8 *dst, int idx, int kind, int count, int msg, int flag) {
    dst[4] = idx;
    dst[0] = kind;
    dst[1] = count;
    dst[3] = msg;
    dst[2] = flag;
}

// 800F0208
void fn_800F0208(u32 idx, u32 kind, u8 count, u8 msg, s8 flag) {
    if (fn_800DCEDC() && fn_800DCF30() > 1 && idx < 10 && kind < 16) {
        u8 buf[5];
        fn_800F01F0(buf, idx, kind, count, msg, flag);
        fn_800DD4C8();
        fn_800DD518(buf, 5);
        fn_800DD588(0x59, 4);
    }
}

// 800F02A8
void fn_800F02A8(u32 idx, u32 kind, u8 count, u8 msg, s8 flag) {
    if (fn_800DCEDC() && fn_800DCF30() > 1 && idx < 10 && kind < 16) {
        u8 buf[5];
        fn_800F01F0(buf, idx, kind, count, msg, flag);
        fn_800DD4C8();
        fn_800DD518(buf, 5);
        fn_800DD588(0x5A, 4);
    }
}

// 800F0348
void fn_800F0348(int *idx, int *kind, u8 *value, const u8 *src) {
    if (src[2] < 10) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    if (src[0] < 16) {
        *kind = src[0];
    } else {
        *kind = -1;
    }
    *value = src[1];
}

// 800F038C
void fn_800F038C(u8 *dst, int idx, int kind, int value) {
    dst[2] = idx;
    dst[0] = kind;
    dst[1] = value;
}

// 800F039C
void fn_800F039C(u32 idx, u32 kind, s8 value) {
    if (fn_800DCEDC() && fn_800DCF30() > 1 && idx < 10 && kind < 16) {
        u8 buf[3];
        fn_800F038C(buf, idx, kind, value);
        fn_800DD4C8();
        fn_800DD518(buf, 3);
        fn_800DD588(0x5B, 4);
    }
}

// 800F0434
void fn_800F0434(const dAnmPersonalID_c *animal, u32 kind, s8 value) {
    fn_800F039C(dSaveData_c::getRaw()->mAnimals.mTown.getAnimalIdx(animal), kind, value);
}

// 800F0494
void fn_800F0494(int *idx, int *kind, int *value, const u8 *src) {
    if (src[2] < 10) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    if (src[0] < 16) {
        *kind = src[0];
    } else {
        *kind = -1;
    }
    *value = src[1];
}

// 800F04D8
void fn_800F04D8(u8 *dst, int idx, int kind, int value) {
    dst[2] = idx;
    dst[0] = kind;
    dst[1] = value;
}

// 800F04E8
void fn_800F04E8(u32 idx, u32 kind, u8 value) {
    if (fn_800DCEDC() && fn_800DCF30() > 1 && idx < 10 && kind < 16) {
        u8 buf[3];
        fn_800F04D8(buf, idx, kind, value);
        fn_800DD4C8();
        fn_800DD518(buf, 3);
        fn_800DD588(0x5C, 4);
    }
}

// 800F0580
void fn_800F0580(int *idx, int *kind, const u8 *src) {
    if (src[1] < 10) {
        *idx = src[1];
    } else {
        *idx = -1;
    }
    if (src[0] < 16) {
        *kind = src[0];
    } else {
        *kind = -1;
    }
}

// 800F05BC
void fn_800F05BC(u8 *dst, int idx, int kind) {
    dst[1] = idx;
    dst[0] = kind;
}

// 800F05C8
void fn_800F05C8(u32 idx, u32 kind) {
    if (fn_800DCEDC() && fn_800DCF30() > 1 && idx < 10 && kind < 16) {
        u8 buf[2];
        fn_800F05BC(buf, idx, kind);
        fn_800DD4C8();
        fn_800DD518(buf, 2);
        fn_800DD588(0x5E, 4);
    }
}

// 800F0650
void fn_800F0650() {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 value = 1;
        fn_800DD4C8();
        fn_800DD518(&value, 1);
        fn_800DD588(0x64, 4);
    }
}

// 800F06A8
void fn_800F06A8(u8 value) {
    u8 buf = value;
    if (fn_800DCEDC() && fn_800DCF30() > 1 && buf < 5) {
        fn_800DD4C8();
        fn_800DD518(&buf, 1);
        fn_800DD588(0x65, 4);
    }
}

// 800F0708
void fn_800F0708(int value) {
    if (fn_800DCEDC() && fn_800DCF30() > 1 && value < 3) {
        u8 buf = value;
        fn_800DD4C8();
        fn_800DD518(&buf, 1);
        fn_800DD588(0x66, 4);
    }
}

// 800F0770
void fn_800F0770(u8 *kind, int *x, int *z, const u8 *src) {
    u8 value = 0x44;
    cLib::memCpy(&value, src, 1);
    if (value < 0x44) {
        *kind = value;
    } else {
        *kind = 0x44;
    }
    if (src[1] < 16 && src[2] < 16) {
        *x = src[1];
        *z = src[2];
    } else {
        *x = -1;
        *z = -1;
    }
}

// 800F080C
void fn_800F080C(u8 *dst, int kind, int x, int z) {
    u8 value = kind;
    cLib::memCpy(dst, &value, 1);
    dst[1] = x;
    dst[2] = z;
}

// 800F0864
void fn_800F0864(int kind, int x, int z) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[3];
        fn_800F080C(buf, kind, x, z);
        fn_800DD4C8();
        fn_800DD518(buf, 3);
        fn_800DD588(0x69, 4);
    }
}

// 800F08EC
void fn_800F08EC(int *idx, void *item, int *count, const u8 *src) {
    if (src[2] < 10) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    cLib::memCpy(item, src, 2);
    if (src[3] < 10) {
        *count = src[3];
    } else {
        *count = 10;
    }
}

// 800F0968
void fn_800F0968(u8 *dst, int idx, const void *item, int count) {
    dst[2] = idx;
    cLib::memCpy(dst, item, 2);
    dst[3] = count;
}

// 800F09B4
void fn_800F09B4(int idx, const void *item, int count) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[4];
        fn_800F0968(buf, idx, item, count);
        fn_800DD4C8();
        fn_800DD518(buf, 4);
        fn_800DD588(0x6A, 4);
    }
}

// 800F0A3C
void fn_800F0A3C(int *idx, int *mode, const u8 *src) {
    if (src[1] < 10) {
        *idx = src[1];
    } else {
        *idx = -1;
    }
    if (src[0] < 5) {
        *mode = src[0];
    } else {
        *mode = 4;
    }
}

// 800F0A78
void fn_800F0A78(u8 *dst, int idx, int mode) {
    dst[1] = idx;
    dst[0] = mode;
}

// 800F0A84
void fn_800F0A84(int idx, int mode) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[2];
        fn_800F0A78(buf, idx, mode);
        fn_800DD4C8();
        fn_800DD518(buf, 2);
        fn_800DD588(0x6B, 4);
    }
}

// 800F0AFC
void fn_800F0AFC(int *idx, int *a, int *b, u32 *c, const u8 *src) {
    if (src[3] < 10) {
        *idx = src[3];
    } else {
        *idx = -1;
    }
    *a = (s8)src[0];
    *b = (s8)src[1];
    u32 value = src[2];
    if (value <= 4) {
        *c = value;
    } else {
        *c = 4;
    }
}

// 800F0B50
void fn_800F0B50(u8 *dst, int idx, int a, int b, int c) {
    dst[3] = idx;
    dst[0] = a;
    dst[1] = b;
    dst[2] = c;
}

// 800F0B64
void fn_800F0B64(int idx, int a, int b, int c) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[4];
        fn_800F0B50(buf, idx, a, b, c);
        fn_800DD4C8();
        fn_800DD518(buf, 4);
        fn_800DD588(0x6C, 4);
    }
}

// 800F0BFC
void fn_800F0BFC(int *idx, u32 *kind, void *data, const u8 *src) {
    if (src[9] < 10) {
        *idx = src[9];
    } else {
        *idx = -1;
    }
    u32 value = src[8];
    if (value <= 2) {
        *kind = value;
    } else {
        *kind = 2;
    }
    cLib::memCpy(data, src, 8);
}

// 800F0C44
void fn_800F0C44(u8 *dst, int idx, int kind, const void *data) {
    dst[8] = kind;
    dst[9] = idx;
    cLib::memCpy(dst, data, 8);
}

// 800F0C5C
void fn_800F0C5C(int idx, int kind, const void *data) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[10];
        fn_800F0C44(buf, idx, kind, data);
        fn_800DD4C8();
        fn_800DD518(buf, 10);
        fn_800DD588(0x6D, 4);
    }
}

// 800F0CE4
void fn_800F0CE4(int *idx, const u8 *src) {
    if (src[0] < 10) {
        *idx = src[0];
    } else {
        *idx = -1;
    }
}

// 800F0D04
void fn_800F0D04(u8 *dst, int idx) {
    dst[0] = idx;
}

// 800F0D0C
void fn_800F0D0C(int idx) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf;
        fn_800F0D04(&buf, idx);
        fn_800DD4C8();
        fn_800DD518(&buf, 1);
        fn_800DD588(0x6E, 4);
    }
}

// 800F0D74
void fn_800F0D74(int *idx, int *count, void *item, const u8 *src) {
    if (src[3] < 10) {
        *idx = src[3];
    } else {
        *idx = -1;
    }
    if (src[2] < 10) {
        *count = src[2];
    } else {
        *count = 10;
    }
    cLib::memCpy(item, src, 2);
}

// 800F0DBC
void fn_800F0DBC(u8 *dst, int idx, int count, const void *item) {
    dst[2] = count;
    dst[3] = idx;
    cLib::memCpy(dst, item, 2);
}

// 800F0DD4
void fn_800F0DD4(int idx, int count, const void *item) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[4];
        fn_800F0DBC(buf, idx, count, item);
        fn_800DD4C8();
        fn_800DD518(buf, 4);
        fn_800DD588(0x6F, 4);
    }
}

// 800F0E5C
void fn_800F0E5C(int *idx, void *item, const u8 *src) {
    if (src[2] < 10) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    cLib::memCpy(item, src, 2);
}

// 800F0E88
void fn_800F0E88(u8 *dst, int idx, const void *item) {
    dst[2] = idx;
    cLib::memCpy(dst, item, 2);
}

// 800F0E9C
void fn_800F0E9C(int idx, const void *item) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[3];
        fn_800F0E88(buf, idx, item);
        fn_800DD4C8();
        fn_800DD518(buf, 3);
        fn_800DD588(0x70, 4);
    }
}

// 800F0F14
void fn_800F0F14(int *idx, const u8 *src) {
    if (src[0] < 10) {
        *idx = src[0];
    } else {
        *idx = -1;
    }
}

// 800F0F34
void fn_800F0F34(u8 *dst, int idx) {
    dst[0] = idx;
}

// 800F0F3C
void fn_800F0F3C(int idx) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf;
        fn_800F0F34(&buf, idx);
        fn_800DD4C8();
        fn_800DD518(&buf, 1);
        fn_800DD588(0x71, 4);
    }
}

// 800F0FA4
void fn_800F0FA4(int *idx, void *item, const u8 *src) {
    if (src[2] < 10) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    cLib::memCpy(item, src, 2);
}

// 800F0FD0
void fn_800F0FD0(u8 *dst, int idx, const void *item) {
    dst[2] = idx;
    cLib::memCpy(dst, item, 2);
}

// 800F0FE4
void fn_800F0FE4(int idx, const void *item) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[3];
        fn_800F0FD0(buf, idx, item);
        fn_800DD4C8();
        fn_800DD518(buf, 3);
        fn_800DD588(0x72, 4);
    }
}

// 800F105C
void fn_800F105C(int *idx, void *item, const u8 *src) {
    if (src[2] < 10) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    cLib::memCpy(item, src, 2);
}

// 800F1088
void fn_800F1088(u8 *dst, int idx, const void *item) {
    dst[2] = idx;
    cLib::memCpy(dst, item, 2);
}

// 800F109C
void fn_800F109C(int idx, const void *item) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[3];
        fn_800F1088(buf, idx, item);
        fn_800DD4C8();
        fn_800DD518(buf, 3);
        fn_800DD588(0x73, 4);
    }
}

// 800F1114
void fn_800F1114(int *idx, void *item, const u8 *src) {
    if (src[2] < 10) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    cLib::memCpy(item, src, 2);
}

// 800F1140
void fn_800F1140(u8 *dst, int idx, const void *item) {
    dst[2] = idx;
    cLib::memCpy(dst, item, 2);
}

// 800F1154
void fn_800F1154(int idx, const void *item) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[3];
        fn_800F1140(buf, idx, item);
        fn_800DD4C8();
        fn_800DD518(buf, 3);
        fn_800DD588(0x74, 4);
    }
}

// 800F11CC
void fn_800F11CC(int *idx, void *item, const u8 *src) {
    if (src[2] < 10) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    cLib::memCpy(item, src, 2);
}

// 800F11F8
void fn_800F11F8(u8 *dst, int idx, const void *item) {
    dst[2] = idx;
    cLib::memCpy(dst, item, 2);
}

// 800F120C
void fn_800F120C(int idx, const void *item) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[3];
        fn_800F11F8(buf, idx, item);
        fn_800DD4C8();
        fn_800DD518(buf, 3);
        fn_800DD588(0x75, 4);
    }
}

// 800F1284
void fn_800F1284(int *idx, int *kind, u8 *value, const u8 *src) {
    if (src[2] < 10) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    if (src[0] < 0x15) {
        *kind = src[0];
    } else {
        *kind = 0x15;
    }
    *value = src[1];
}

// 800F12C8
void fn_800F12C8(u8 *dst, int idx, int kind, int value) {
    dst[2] = idx;
    dst[0] = kind;
    dst[1] = value;
}

// 800F12D8
void fn_800F12D8(int idx, int kind, int value) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[3];
        fn_800F12C8(buf, idx, kind, value);
        fn_800DD4C8();
        fn_800DD518(buf, 3);
        fn_800DD588(0x76, 4);
    }
}

// 800F1360
void fn_800F1360(int *idx, const u8 *src) {
    if (src[0] < 10) {
        *idx = src[0];
    } else {
        *idx = -1;
    }
}

// 800F1380
void fn_800F1380(u8 *dst, int idx) {
    dst[0] = idx;
}

// 800F1388
void fn_800F1388(int idx) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf;
        fn_800F1380(&buf, idx);
        fn_800DD4C8();
        fn_800DD518(&buf, 1);
        fn_800DD588(0x77, 4);
    }
}

// 800F13F0
void fn_800F13F0(int *idx, const u8 *src) {
    if (src[0] < 10) {
        *idx = src[0];
    } else {
        *idx = -1;
    }
}

// 800F1410
void fn_800F1410(u8 *dst, int idx) {
    dst[0] = idx;
}

// 800F1418
void fn_800F1418(int idx) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf;
        fn_800F1410(&buf, idx);
        fn_800DD4C8();
        fn_800DD518(&buf, 1);
        fn_800DD588(0x78, 4);
    }
}

// 800F1480
void fn_800F1480(int *idx, int *kind, void *item, const u8 *src) {
    if (src[3] < 10) {
        *idx = src[3];
    } else {
        *idx = -1;
    }
    cLib::memCpy(item, src, 2);
    if (src[2] < 16) {
        *kind = src[2];
    } else {
        *kind = -1;
    }
}

// 800F14FC
void fn_800F14FC(u8 *dst, int idx, int kind, const void *item) {
    dst[2] = kind;
    dst[3] = idx;
    cLib::memCpy(dst, item, 2);
}

// 800F1514
void fn_800F1514(int idx, int kind, const void *item) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[4];
        fn_800F14FC(buf, idx, kind, item);
        fn_800DD4C8();
        fn_800DD518(buf, 4);
        fn_800DD588(0x79, 4);
    }
}

// 800F159C
void fn_800F159C(void *item, int *mode, const u8 *src) {
    cLib::memCpy(item, src, 2);
    if (src[2] < 4) {
        *mode = src[2];
    } else {
        *mode = 4;
    }
}

// 800F15F8
void fn_800F15F8(u8 *dst, const void *item, int mode) {
    cLib::memCpy(dst, item, 2);
    dst[2] = mode;
}

// 800F1638
void fn_800F1638(const void *item, int mode) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[3];
        fn_800F15F8(buf, item, mode);
        fn_800DD4C8();
        fn_800DD518(buf, 3);
        fn_800DD588(0x7A, 4);
    }
}

// 800F16B0
void fn_800F16B0(const void *item, int mode) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[3];
        fn_800F15F8(buf, item, mode);
        fn_800DD4C8();
        fn_800DD518(buf, 3);
        fn_800DD588(0x7B, 4);
    }
}

// 800F1728
void fn_800F1728(void *item, const void *src) {
    cLib::memCpy(item, src, 2);
}

// 800F1730
void fn_800F1730(void *dst, const void *item) {
    cLib::memCpy(dst, item, 2);
}

// 800F1738: sends to one other connected player.
void fn_800F1738(const void *item, u32 player) {
    if (fn_800DCEDC() && fn_800DCF30() > 1 && player < 4 && (int)player != fn_800DCF58() && fn_800DCF2C(player)) {
        u8 buf[2];
        fn_800F1730(buf, item);
        fn_800DD4C8();
        fn_800DD518(buf, 2);
        fn_800DD588(0x7C, player);
    }
}

// 800F17D0
void fn_800F17D0(const void *item) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[2];
        fn_800F1730(buf, item);
        fn_800DD4C8();
        fn_800DD518(buf, 2);
        fn_800DD588(0x7D, 4);
    }
}

// 800F1838
void fn_800F1838(const void *item, u32 player) {
    if (fn_800DCEDC() && fn_800DCF30() > 1 && player < 4 && (int)player != fn_800DCF58() && fn_800DCF2C(player)) {
        u8 buf[2];
        fn_800F1730(buf, item);
        fn_800DD4C8();
        fn_800DD518(buf, 2);
        fn_800DD588(0x7E, player);
    }
}

// 800F18D0
void fn_800F18D0(const void *item, int mode) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[3];
        fn_800F15F8(buf, item, mode);
        fn_800DD4C8();
        fn_800DD518(buf, 3);
        fn_800DD588(0x7F, 4);
    }
}

// 800F1948
void fn_800F1948(const void *item, int mode) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[3];
        fn_800F15F8(buf, item, mode);
        fn_800DD4C8();
        fn_800DD518(buf, 3);
        fn_800DD588(0x80, 4);
    }
}

// 800F19C0
void fn_800F19C0(const void *item) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[2];
        fn_800F1730(buf, item);
        fn_800DD4C8();
        fn_800DD518(buf, 2);
        fn_800DD588(0x81, 4);
    }
}

// 800F1A28
void fn_800F1A28(int *idx, void *item, const u8 *src) {
    if (src[2] < 10) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    cLib::memCpy(item, src, 2);
}

// 800F1A54
void fn_800F1A54(u8 *dst, int idx, const void *item) {
    dst[2] = idx;
    cLib::memCpy(dst, item, 2);
}

// 800F1A68
void fn_800F1A68(int idx, const void *item) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        u8 buf[3];
        fn_800F1A54(buf, idx, item);
        fn_800DD4C8();
        fn_800DD518(buf, 3);
        fn_800DD588(0x82, 4);
    }
}

// ---------------------------------------------------------------------------
// Spot pickers. Positions are in units (32.0f world units each).

static inline BOOL isIdInRange(u16 id, u16 lo, u16 hi) {
    BOOL result = FALSE;
    if (id >= lo && id <= hi) {
        result = TRUE;
    }
    return result;
}

static inline int getFtrFunc(const dItem::BITM *bitm) {
    int func = 1;
    if (static_cast<u32>(bitm->m_ftrFunc) < 0x41) {
        func = bitm->m_ftrFunc;
    }
    return func;
}

// 800F1AE0: the player standing on unit (x, z).
dPlayerActor_c *getPlayerOnUnit(int x, int z) {
    for (int i = 0; i < 4; i++) {
        dPlayerActor_c *player = fn_800FBC7C(i);
        if (player != NULL) {
            int px = (int)player->mPos.x >> 5;
            int pz = (int)player->mPos.z >> 5;
            if (x == px && z == pz) {
                return player;
            }
        }
    }
    return NULL;
}

// 800F1B7C: any actor standing on unit (x, z).
dActor_c *getActorOnUnit(int x, int z) {
    dActor_c *actor = getPlayerOnUnit(x, z);
    if (actor != NULL) {
        return actor;
    }
    actor = fn_800F9860(x, z);
    if (actor != NULL) {
        return actor;
    }
    return fn_800F98CC(x, z);
}

// 800F1BE4: the furniture whose footprint covers unit (x, z).
dItem::Item fn_800F1BE4(int *outX, int *outZ, int x, int z, dFdBase_c *map, u8 kind) {
    if (kind == SCENE_NUM) {
        kind = getCurrentScene();
    }
    if (!isSceneAttr(kind, SCENE_ATTR_ROOM)) {
        return dItem::Item();
    }
    if (map == NULL) {
        map = fn_80190C44(0);
    }
    if (map == NULL) {
        return dItem::Item();
    }
    if (x <= 0 || x >= 15 || z <= 0 || z >= 15) {
        return dItem::Item();
    }

    int dummyX;
    int dummyZ;
    if (outX == NULL) {
        outX = &dummyX;
    }
    if (outZ == NULL) {
        outZ = &dummyZ;
    }
    for (int zz = z - 1; zz <= z + 1; zz++) {
        for (int xx = x - 1; xx <= x + 1; xx++) {
            dItem::Item *item = map->getItem(xx, zz, 0);
            if (item != NULL && item->mId != dItem::ITEM_ID_NONE) {
                const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
                if (bitm != NULL && getFtrFunc(bitm) != 0) {
                    dNpcFtrShape_c shape;
                    fn_800A8B28(&shape, *item);
                    for (u32 i = 0; i < fn_800A8BB8(&shape); i++) {
                        const int *ofs = fn_800A8BE4(&shape, i);
                        if (x == xx + ofs[0] && z == zz + ofs[1]) {
                            *outX = xx;
                            *outZ = zz;
                            return *item;
                        }
                    }
                }
            }
        }
    }
    return dItem::Item();
}

// 800F1DF0: whether `item` can be placed on unit (x, z).
BOOL canPutItemOnUnit(int x, int z, const dItem::Item *item, dFdBase_c *map, u8 kind, BOOL allowFg94, BOOL checkA) {
    BOOL result = FALSE;
    if (map == NULL) {
        map = fn_80190C44(0);
    }
    if (kind == SCENE_NUM) {
        kind = getCurrentScene();
    }
    if (map == NULL) {
        return result;
    }
    if (!((checkA && map->canNpcPutItem(x, z)) || (!checkA && map->canPutItem(x, z)))) {
        return result;
    }

    dItem::Item *cur = map->getItem(x, z, 0);
    if (cur == NULL) {
        return result;
    }
    if (cur->mId != dItem::ITEM_ID_NONE) {
        int cat = (cur->mId >> 12) & 0xF;
        BOOL isFtr = FALSE;
        if (cat >= 9 && cat <= 12) {
            isFtr = TRUE;
        }
        if (isFtr && (isSceneAttr(kind, SCENE_ATTR_TOWN) || !cur->hasFtrFunc())) {
            goto free;
        }
        if (cur->mId >= 0xE5) {
            return result;
        }
        if (cur->getFgInfo() != NULL && cur->getFgInfo()->mTreeStage == 0) {
            goto free;
        }
        if (!allowFg94 && isIdInRange(cur->mId, 0x94, 0x94)) {
            goto free;
        }
        if (cur->isFg74() || isIdInRange(cur->mId, 0x01, 0x04) || isIdInRange(cur->mId, 0x57, 0x5A) ||
            isIdInRange(cur->mId, 0xE0, 0xE1) || cur->isAnyFlower() || isIdInRange(cur->mId, 0x95, 0x9D))
        {
            goto free;
        }
        return result;
    }

free:
    dActor_c *actor = getActorOnUnit(x, z);
    if (actor != NULL) {
        if (item->mId == dItem::ITEM_ID_NONE || item->mId != actor->mParam) {
            return result;
        }
    }
    result = TRUE;
    if (isSceneAttr(kind, SCENE_ATTR_ROOM)) {
        dItem::Item ftr = fn_800F1BE4(NULL, NULL, x, z, NULL, 0x44);
        if (ftr.mId != dItem::ITEM_ID_NONE) {
            const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(ftr);
            if (bitm != NULL && getFtrFunc(bitm) != 0) {
                result = FALSE;
            }
        }
    }
    return result;
}

// 800F20D0
BOOL canPutItemAt(const mVec3_c *pos, const dItem::Item *item, dFdBase_c *map, u8 kind, BOOL allowFg94, BOOL checkA) {
    return canPutItemOnUnit((int)pos->x >> 5, (int)pos->z >> 5, item, map, kind, allowFg94, checkA);
}

// 800F2138: a random free unit closest to `pos`, searching up to `radius` rings out.
BOOL fn_800F2138(mVec3_c *out, const mVec3_c *pos, u32 radius) {
    dFdBase_c *map = fn_80190C44(0);
    if (map == NULL) {
        return FALSE;
    }

    int cx = (int)pos->x >> 5;
    int cz = (int)pos->z >> 5;
    int bestX = -1;
    int bestZ = -1;
    u32 maxX = cx;
    u32 maxZ = cz;
    for (u32 r = 0; r <= radius; r++, maxX++, maxZ++) {
        u32 num = 0;
        for (u32 z = cz - r; z <= maxZ; z++) {
            for (u32 x = cx - r; x <= maxX; x++) {
                dItem::Item none;
                if (canPutItemOnUnit(x, z, &none, map, SCENE_NUM, TRUE, TRUE) != FALSE) {
                    f32 chance = 100.0f / (num + 1);
                    if (cM::rndF(100.0f) <= chance) {
                        bestX = x;
                        bestZ = z;
                    }
                    num++;
                }
            }
        }
        if (bestX != -1 && bestZ != -1) {
            break;
        }
    }
    if (bestX != -1 && bestZ != -1) {
        dFdBase_c::getUnitCenterPos(out, bestX, bestZ);
        return TRUE;
    }
    return FALSE;
}

// 800F22FC: whether unit (x, z) is within `dist` of `pos` (XZ only).
BOOL fn_800F22FC(const mVec3_c *pos, int x, int z, f32 dist) {
    if (pos == NULL || dist <= 0.0f) {
        return FALSE;
    }
    mVec3_c unit = mVec3_c::Zero;
    dFdBase_c::getUnitCenterPos(&unit, x, z);
    mVec3_c diff = *pos - unit;
    if (EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z) <= dist) {
        return TRUE;
    }
    return FALSE;
}

// 800F23C0: a random free unit in [x0, x1) x [z0, z1), optionally filtered by `func`
// and kept further than `dist` from `exclude`.
BOOL fn_800F23C0(int *outX, int *outZ, dFdBase_c *map, u8 kind, int x0, int x1, int z0, int z1,
                 dNpcSpotFunc func, int arg, const mVec3_c *exclude, f32 dist) {
    if (outX == NULL || outZ == NULL) {
        return FALSE;
    }
    if (map == NULL) {
        return FALSE;
    }

    u32 num = 0;
    for (int z = z0; z < z1; z++) {
        for (int x = x0; x < x1; x++) {
            BOOL ok = FALSE;
            BOOL free = FALSE;
            if (!fn_800F22FC(exclude, x, z, dist)) {
                dItem::Item none;
                if (canPutItemOnUnit(x, z, &none, map, kind, TRUE, TRUE)) {
                    free = TRUE;
                }
            }
            if (free) {
                BOOL pass = FALSE;
                if (func == NULL || func(x, z, arg)) {
                    pass = TRUE;
                }
                if (pass) {
                    ok = TRUE;
                }
            }
            if (ok) {
                f32 chance = 100.0f / (num + 1);
                if (cM::rndF(100.0f) <= chance) {
                    *outX = x;
                    *outZ = z;
                }
                num++;
            }
        }
    }
    return num != 0;
}

// 800F25A4: fn_800F23C0 over the outdoor map, keeping a one-block border.
BOOL fn_800F25A4(int *outX, int *outZ, dNpcSpotFunc func, int arg, const mVec3_c *exclude, f32 dist) {
    dFdBase_c *map = fn_80190C44(1);
    if (map == NULL) {
        return FALSE;
    }
    return fn_800F23C0(outX, outZ, map, 0, 0x10, map->mUnitW - 0x10, 0x10, map->mUnitH - 0x10, func, arg, exclude,
                       dist);
}

// 800F2644
BOOL fn_800F2644(mVec3_c *out, dNpcSpotFunc func, int arg, const mVec3_c *exclude, f32 dist) {
    if (out == NULL) {
        return FALSE;
    }
    int x = -1;
    int z = -1;
    if (fn_800F25A4(&x, &z, func, arg, exclude, dist)) {
        dFdBase_c::getUnitCenterPos(out, x, z);
        return TRUE;
    }
    return FALSE;
}

// 800F26C8: a random free unit inside a block with two free units north of it.
BOOL fn_800F26C8(int *outX, int *outZ) {
    if (outX == NULL || outZ == NULL) {
        return FALSE;
    }
    dFdBase_c *map = fn_80190C44(1);
    if (map == NULL) {
        return FALSE;
    }

    int blockW = map->mBlockW;
    int blockH = map->mBlockH;
    u32 num = 0;
    dSaveBuildingList_c *fg = dSaveBuildingList_c::get();
    for (int bz = 2, z0 = 0x20; bz < blockH - 1; bz++, z0 += 0x10) {
        for (int bx = 2, x0 = 0x20; bx < blockW - 2; bx++, x0 += 0x10) {
            for (int j = 4; j < 12; j++) {
                int z = z0 + j;
                for (int i = 4; i < 12; i++) {
                    int x = x0 + i;
                    dItem::Item none;
                    if (canPutItemOnUnit(x, z, &none, map, 0, TRUE, TRUE) != FALSE) {
                        dItem::Item a = fg->getAt(x, z - 1, 1);
                        dItem::Item b = fg->getAt(x, z - 2, 1);
                        if (a.mId == dItem::ITEM_ID_NONE && b.mId == dItem::ITEM_ID_NONE) {
                            num++;
                            f32 chance = 100.0f / num;
                            if (cM::rndF(100.0f) <= chance) {
                                *outX = x;
                                *outZ = z;
                            }
                        }
                    }
                }
            }
        }
    }
    return num != 0;
}

// 800F28AC
BOOL fn_800F28AC(mVec3_c *out) {
    if (out == NULL) {
        return FALSE;
    }
    int x = 0;
    int z = 0;
    if (fn_800F26C8(&x, &z)) {
        dFdBase_c::getUnitCenterPos(out, x, z);
        return TRUE;
    }
    return FALSE;
}

// 800F2920: fn_800F23C0 over the current scene's whole map.
BOOL fn_800F2920(int *outX, int *outZ, dNpcSpotFunc func, int arg, const mVec3_c *exclude, f32 dist) {
    dFdBase_c *map = fn_80190C44(0);
    if (map == NULL) {
        return FALSE;
    }
    u8 kind = getCurrentScene();
    return fn_800F23C0(outX, outZ, map, kind, 0, map->mUnitW, 0, map->mUnitH, func, arg, exclude, dist);
}

// 800F29C4
BOOL fn_800F29C4(mVec3_c *out, dNpcSpotFunc func, int arg, const mVec3_c *exclude, f32 dist) {
    if (out == NULL) {
        return FALSE;
    }
    int x = -1;
    int z = -1;
    if (fn_800F2920(&x, &z, func, arg, exclude, dist)) {
        dFdBase_c::getUnitCenterPos(out, x, z);
        return TRUE;
    }
    return FALSE;
}

// 800F2A48
BOOL fn_800F2A48(const dItem::Item *item) {
    static dItem::Item sItems[] = {
        0xD001, 0xD002, 0xD003, 0xD004, 0xD009, 0xD00A, 0xD00B, 0xD00C, 0xD00D,
        0xD00E, 0xD00F, 0xD010, 0xD011, 0xD012, 0xD013, 0xD015, 0xD016, 0xD017,
    };
    static u32 sNum = ARRAY_SIZE(sItems);

    for (dItem::Item *p = sItems; p != sItems + sNum; p++) {
        if (item->isSame(*p)) {
            return TRUE;
        }
    }
    return FALSE;
}

static inline BOOL isGrownTree(dItem::Item *item) {
    BOOL result = FALSE;
    dItem::FgInfo *info = item->getFgInfo();
    if (info != NULL && info->mTreeStage >= 4) {
        result = TRUE;
    }
    return result;
}

// 800F2C94: adds the free units of block (bx, bz) next to signs and walls to the pick.
u32 fn_800F2C94(int *outX, int *outZ, u32 num, u32 bx, u32 bz, dFdBase_c *map, const mVec3_c *exclude,
                BOOL allowFg94, f32 dist) {
    if (map == NULL) {
        return num;
    }
    if (bx >= map->mBlockW || bz >= map->mBlockH) {
        return num;
    }

    dSaveBuildingList_c *fg = dSaveBuildingList_c::get();
    int x0 = bx << 4;
    int z0 = bz << 4;
    int x1 = x0 + 0x10;
    int z1 = z0 + 0x10;
    for (int z = z0; z < z1; z++) {
        for (int x = x0; x < x1; x++) {
            if (fn_800F22FC(exclude, x, z, dist)) {
                continue;
            }
            dItem::Item item = fg->getAt(x, z, 1);
            if (item.mId != dItem::ITEM_ID_NONE) {
                if (fn_800F2A48(&item)) {
                    for (int i = 0; i < 2; i++) {
                        mVec3_c pos = mVec3_c::Zero;
                        if (fn_80169FA4(&pos, &item, i)) {
                            dItem::Item none;
                            if (canPutItemOnUnit((int)pos.x >> 5, (int)pos.z >> 5, &none, map, 0, allowFg94, TRUE) != FALSE) {
                                f32 chance = 100.0f / (num + 1);
                                if (cM::rndF(100.0f) <= chance) {
                                    *outX = (int)pos.x >> 5;
                                    *outZ = (int)pos.z >> 5;
                                }
                                num++;
                            }
                        }
                    }
                }
            } else {
                BOOL ok = FALSE;
                if (fg->isBuildSite(x, z)) {
                    dItem::Item none;
                    if (canPutItemOnUnit(x, z - 1, &none, map, 0, allowFg94, TRUE)) {
                        ok = TRUE;
                    }
                }
                if (ok) {
                    f32 chance = 100.0f / (num + 1);
                    if (cM::rndF(100.0f) <= chance) {
                        *outX = x;
                        *outZ = z - 1;
                    }
                    num++;
                }
            }
        }
    }
    return num;
}

// 800F2F8C: adds the free units south of grown trees in block (bx, bz) to the pick.
u32 fn_800F2F8C(int *outX, int *outZ, u32 num, u32 bx, u32 bz, dFdBase_c *map, const mVec3_c *exclude,
                BOOL allowFg94, f32 dist) {
    if (map == NULL) {
        return num;
    }
    if (bx >= map->mBlockW || bz >= map->mBlockH) {
        return num;
    }

    int x0 = bx << 4;
    int z0 = bz << 4;
    int x1 = x0 + 0x10;
    int z1 = z0 + 0x10;
    for (int z = z0; z < z1; z++) {
        for (int x = x0; x < x1; x++) {
            if (fn_800F22FC(exclude, x, z, dist)) {
                continue;
            }
            dItem::Item *item = map->getItem(x, z, 0);
            if (item == NULL || item->mId == dItem::ITEM_ID_NONE) {
                continue;
            }
            dItem::FgInfo *info = item->getFgInfo();
            BOOL ok = FALSE;
            BOOL tree = FALSE;
            if (info != NULL && info->mTreeStage >= 4) {
                tree = TRUE;
            }
            if (tree) {
                dItem::Item none;
                if (canPutItemOnUnit(x, z - 1, &none, map, 0, allowFg94, TRUE)) {
                    ok = TRUE;
                }
            }
            if (ok) {
                f32 chance = 100.0f / (num + 1);
                if (cM::rndF(100.0f) <= chance) {
                    *outX = x;
                    *outZ = z - 1;
                }
                num++;
            }
        }
    }
    return num;
}

// 800F3178: picks a unit on the outdoor 5x5 block grid, preferring blocks of type `type`
// (0x400: around that block) that pass `func`.
BOOL fn_800F3178(int *outX, int *outZ, dNpcSpotFunc func, int arg, const mVec3_c *exclude, u32 type,
                 BOOL skipTrees, BOOL allowFg94, f32 dist) {
    dFdBase_c *map = fn_80190C44(1);
    if (map == NULL) {
        return FALSE;
    }

    u8 used[7];
    memset(used, 0, sizeof(used));
    if (type == 0x400) {
        int foundX = 0;
        int foundZ = 0;
        for (int z = 0; z < 5; z++) {
            for (int x = 0; x < 5; x++) {
                if (map->hasBlockFlag(x + 1, z + 1, type)) {
                    foundX = x;
                    foundZ = z;
                }
            }
        }
        for (int z = 0; z < 5; z++) {
            for (int x = 0; x < 5; x++) {
                if (x >= foundX - 1 && x <= foundX + 1 && z >= foundZ - 1 && z <= foundZ) {
                    used[z] |= 1 << x;
                }
            }
        }
    } else if (type != 0) {
        for (int z = 0; z < 5; z++) {
            for (int x = 0; x < 5; x++) {
                if (map->hasBlockFlag(x + 1, z + 1, type)) {
                    used[z] |= 1 << x;
                }
            }
        }
    }
    if (func != NULL) {
        for (int z = 0; z < 5; z++) {
            for (int x = 0; x < 5; x++) {
                if (!func(x + 1, z + 1, arg)) {
                    used[z] |= 1 << x;
                }
            }
        }
    }

    u32 num = 0;
    for (int z = 0; z < 5; z++) {
        for (int x = 0; x < 5; x++) {
            if ((used[z] >> x) & 1) {
                continue;
            }
            u32 next = fn_800F2C94(outX, outZ, num, x + 1, z + 1, map, exclude, allowFg94, dist);
            BOOL busy = FALSE;
            if (next != num || map->hasBlockFlag(x + 1, z + 1, 0x100200)) {
                busy = TRUE;
            } else {
                const dFdBlock_c *block = map->getBlock(x + 1, z + 1);
                if (block != NULL && dRF::isPondType(block->mType)) {
                    busy = TRUE;
                }
            }
            num = next;
            if (!skipTrees && busy) {
                num = fn_800F2F8C(outX, outZ, num, x + 1, z + 1, map, exclude, allowFg94, dist);
                used[z] |= 1 << x;
            }
        }
    }

    if (num == 0) {
        for (int x = 0; x < 5; x++) {
            if (!((used[4] >> x) & 1)) {
                num = fn_800F2F8C(outX, outZ, num, x + 1, 5, map, exclude, allowFg94, dist);
                used[4] |= 1 << x;
            }
        }

        int fx = 0;
        int fz = 0;
        dItem::Item item(0xD014);
        if (dSaveBuildingList_c::get()->getPos(&fx, &fz, &item, 1) != FALSE) {
            u32 bx = (fx >> 4) - 1;
            u32 bz = (fz >> 4) - 1;
            if (bx < 5 && bz < 5) {
                for (int x = 0; x < 5; x++) {
                    if (x != bx && !((used[0] >> x) & 1)) {
                        num = fn_800F2F8C(outX, outZ, num, x + 1, 1, map, exclude, allowFg94, dist);
                        used[0] |= 1 << x;
                    }
                }
                for (int z = 1; z < 4; z++) {
                    if (!((used[z] >> bx) & 1)) {
                        num = fn_800F2F8C(outX, outZ, num, bx + 1, z + 1, map, exclude, allowFg94, dist);
                        used[z] |= 1 << bx;
                    }
                }
            }
        }

        for (int z = 1; z < 4; z++) {
            if (!(used[z] & 1)) {
                num = fn_800F2F8C(outX, outZ, num, 1, z + 1, map, exclude, allowFg94, dist);
                used[z] |= 1;
            }
            if (!((used[z] >> 4) & 1)) {
                num = fn_800F2F8C(outX, outZ, num, 5, z + 1, map, exclude, allowFg94, dist);
                used[z] |= 0x10;
            }
        }
    }

    if (num == 0) {
        for (int z = 0; z < 5; z++) {
            for (int x = 0; x < 5; x++) {
                if (!((used[z] >> x) & 1)) {
                    num = fn_800F2F8C(outX, outZ, num, x + 1, z + 1, map, exclude, allowFg94, dist);
                    used[z] |= 1 << x;
                }
            }
        }
    }

    if (num != 0) {
        return TRUE;
    }
    return FALSE;
}

static inline BOOL isFg94(const dItem::Item *item) {
    return isIdInRange(item->mId, 0x94, 0x94);
}

// 800F3830: fn_800F3178 as a position; clears any 0x94 object on the spot and steps
// aside from a tree to the south.
BOOL fn_800F3830(mVec3_c *out, dNpcSpotFunc func, int arg, const mVec3_c *exclude, u32 type, BOOL skipTrees,
                 BOOL allowFg94, f32 dist) {
    if (out == NULL) {
        return FALSE;
    }
    int x = -1;
    int z = -1;
    if (!fn_800F3178(&x, &z, func, arg, exclude, type, skipTrees, allowFg94, dist)) {
        return FALSE;
    }

    dFdBase_c::getUnitCenterPos(out, x, z);
    dFdBase_c *map = fn_80190C44(1);
    dItem::Item *item = map->getItem(x, z, 0);
    if (!fn_80013550() && item != NULL && isFg94(item)) {
        dItem::Item none;
        map->setItem(&none, x, z, 0);
    }

    dItem::Item *south = fn_80190C44(1)->getItem(x, z + 1, 0);
    if (south != NULL && south->mId != dItem::ITEM_ID_NONE && isGrownTree(south)) {
        mVec3_c left(out->x - 32.0f, out->y, out->z);
        mVec3_c leftHalf(out->x - 14.4f, out->y, out->z);
        mVec3_c rightHalf(out->x + 14.4f, out->y, out->z);
        mVec3_c right(out->x + 32.0f, out->y, out->z);
        dItem::Item none;
        if (canPutItemAt(&left, &none, NULL, SCENE_NUM, TRUE, TRUE)) {
            *out = leftHalf;
            BOOL useRight = FALSE;
            dItem::Item none2;
            if (canPutItemAt(&right, &none2, NULL, SCENE_NUM, TRUE, TRUE) && cM::rndInt(2) == 0) {
                useRight = TRUE;
            }
            if (useRight) {
                *out = rightHalf;
            }
        } else {
            dItem::Item none2;
            if (canPutItemAt(&right, &none2, NULL, SCENE_NUM, TRUE, TRUE)) {
                *out = rightHalf;
            } else if (!fn_80013550()) {
                dItem::Item *l = map->getItem(x - 1, z, 0);
                dItem::Item *r = map->getItem(x + 1, z, 0);
                if (l != NULL && isFg94(l)) {
                    dItem::Item none3;
                    map->setItem(&none3, x - 1, z, 0);
                    *out = leftHalf;
                } else if (r != NULL && isFg94(r)) {
                    dItem::Item none3;
                    map->setItem(&none3, x + 1, z, 0);
                    *out = rightHalf;
                }
            }
        }
    }
    return TRUE;
}

// 800F3C0C: whether unit (x, z) faces something to its south (tree, wall, or blocked unit).
BOOL fn_800F3C0C(int x, int z) {
    dFdBase_c *map = fn_80190C44(1);
    if (map == NULL) {
        return FALSE;
    }
    dItem::Item *item = map->getItem(x, z + 1, 0);
    if (item != NULL && item->mId != dItem::ITEM_ID_NONE) {
        dItem::FgInfo *info = item->getFgInfo();
        if (info != NULL && info->mTreeStage >= 4) {
            return TRUE;
        }
    }
    if (dSaveBuildingList_c::get()->isBuildSite(x, z + 1)) {
        dItem::Item fg = dSaveBuildingList_c::get()->getAt(x, z + 1, 1);
        if (fg.mId == dItem::ITEM_ID_NONE) {
            return TRUE;
        }
    }
    if (map->_24 != NULL && (map->_24->getAttr(x, z + 1) & dFdUnitAttr_c::STR_COL)) {
        return TRUE;
    }
    return FALSE;
}

// 800F3D20
BOOL fn_800F3D20(const mVec3_c *pos) {
    return fn_800F3C0C((int)pos->x >> 5, (int)pos->z >> 5);
}

// ---------------------------------------------------------------------------
// Names and words

// 800F3D68: sets `word` to the name of the NPC with key `key`.
BOOL getNpcName(dScript::Word_c *word, const dItem::Item *key, int language) {
    int type = (key->mId >> 12) & 0xF;
    BOOL result = FALSE;
    word->clear();
    switch (type) {
        case 0xE: {
            dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.getAnimalByKey(key);
            if (animal != NULL) {
                animal->mID.setWord(word, language);
                result = TRUE;
            }
            break;
        }
        case 8:
            return fn_8016AE68(word, fn_800F88AC(key), "sys_STRING/STR_SPNpc_name");
    }
    return result;
}


// 800F3E24
void fn_800F3E24() {
    dNpc::list_c *list = &sLandList;
    list->init();
}

// 800F3E38
dTime_c *fn_800F3E38() {
    return &sLandList.mTime;
}

// 800F3E48
void fn_800F3E48() {
    sLandList.setTimeNow();
}

// 800F3E54
dNpcEntry_c *fn_800F3E54(u32 i) {
    return sLandList.get(i);
}

// 800F3E64: the entry of the villager with key `key`.
dNpcEntry_c *fn_800F3E64(const dItem::Item *key) {
    if (!dNpc::isNormalKey(key->mId)) {
        return NULL;
    }
    return sLandList.get(key->mId & 0xFFF);
}

// 800F3EE8: the entry of a town villager.
dNpcEntry_c *fn_800F3EE8(const dAnmPersonalID_c *animal) {
    dItem::Item key = dSaveData_c::getTown()->mAnimals.getAnimalKey(animal);
    return fn_800F3E64(&key);
}

// 800F3F30
u32 fn_800F3F30(dAnimal_c *a, dAnimal_c *b) {
    return fn_800EE138(a, b);
}

// 800F3F34
int fn_800F3F34(dAnimal_c *a, dAnimal_c *b) {
    return fn_800EE244(a, b);
}

// 800F3F38: the STR_Unit message for a player of gender `gender` talking to a villager
// of personality `looks`.
int fn_800F3F38(u8 gender, u8 looks) {
    int msg = 0;
    switch (looks) {
        case 5:
            if (gender == 1) {
                msg = 9;
            }
            break;
        case 0:
        case 4:
            msg = gender == 0 ? 8 : 9;
            break;
    }
    return msg;
}

// 800F3F84
int fn_800F3F84(const dPersonalID_c *pid, u8 looks, dAnimal_c *animal) {
    if (animal != NULL && animal->usesNickname(pid)) {
        switch (looks) {
            case 4:
            case 5:
                return 0;
        }
    }
    return fn_800F3F38(pid->player.mGender, looks);
}

// 800F3FFC
int fn_800F3FFC(u8 idx) {
    static const int sTable[6] = {0, 0, 2, 1, 1, 1};

    int result = 0;
    if (idx < 6) {
        result = sTable[idx];
    }
    return result;
}

// 800F4020: whether `now` is inside hours A of `idx`.
BOOL fn_800F4020(u8 idx, const dTime_c *now) {
    if (idx >= 6) {
        return FALSE;
    }
    if (now == NULL) {
        now = dTime_c::getCurrent();
    }

    const dNpcHours_c *hours = &sHoursA[idx];
    dTime_c start;
    start.set(now->year, now->month, now->mday, hours->mStartHour, hours->mStartMin, 0);
    start.normalize();
    dTime_c end;
    end.set(now->year, now->month, now->mday, hours->mEndHour, hours->mEndMin, 0);
    end.normalize();

    BOOL result = FALSE;
    if (!dTime_c::isSameOrAfter(*now, end)) {
        if (dTime_c::isSameOrAfter(*now, start)) {
            result = TRUE;
        }
    }
    return result;
}

// 800F4250: whether `now` is past the end of hours A of `idx`.
BOOL fn_800F4250(u8 idx, const dTime_c *now) {
    if (idx >= 6) {
        return FALSE;
    }
    if (now == NULL) {
        now = dTime_c::getCurrent();
    }

    const dNpcHours_c *hours = &sHoursA[idx];
    if (now->hour > hours->mEndHour) {
        return TRUE;
    }
    if (now->hour == hours->mEndHour && now->min >= hours->mEndMin) {
        return TRUE;
    }
    return FALSE;
}

// 800F42E0: whether `now` is inside hours B of `idx`.
BOOL fn_800F42E0(u8 idx, const dTime_c *now) {
    if (idx >= 6) {
        return FALSE;
    }
    if (now == NULL) {
        now = dTime_c::getCurrent();
    }

    const dNpcHours_c *hours = &sHoursB[idx];
    dTime_c start;
    start.set(now->year, now->month, now->mday, hours->mStartHour, hours->mStartMin, 0);
    start.normalize();
    dTime_c end;
    end.set(now->year, now->month, now->mday, hours->mEndHour, hours->mEndMin, 0);
    end.normalize();

    BOOL result = FALSE;
    if (!dTime_c::isSameOrAfter(*now, end)) {
        if (dTime_c::isSameOrAfter(*now, start)) {
            result = TRUE;
        }
    }
    return result;
}

struct dNpcPair_c {
    dNpcPair_c(int a, int b) : mA(a), mB(b) {}

    /* 0x0 */ int mA;
    /* 0x4 */ int mB;
};

// 800F4510
int fn_800F4510(dItem::Item *out, int num, const void *filter, const dItem::Item *exclude, int excludeNum) {
    static dNpcPair_c sPairs[4] = {
        dNpcPair_c(4, 0), dNpcPair_c(3, 0), dNpcPair_c(2, 0), dNpcPair_c(1, 0),
    };
    return fn_800C60B4(out, num, sPairs, 4, filter, exclude, excludeNum, 0);
}

// 800F45A4
int fn_800F45A4(dItem::Item *out, int num, const void *filter, const dItem::Item *exclude, int excludeNum) {
    static dNpcPair_c sPairs[3] = {
        dNpcPair_c(3, 0), dNpcPair_c(2, 0), dNpcPair_c(1, 0),
    };
    return fn_800C60B4(out, num, sPairs, 3, filter, exclude, excludeNum, 0);
}

// 800F4608
int fn_800F4608(dItem::Item *out, int num, const void *filter, const dItem::Item *exclude, int excludeNum) {
    static dNpcPair_c sPairs[3] = {
        dNpcPair_c(3, 3), dNpcPair_c(2, 3), dNpcPair_c(1, 3),
    };
    return fn_800C60B4(out, num, sPairs, 3, filter, exclude, excludeNum, 0);
}

// 800F4668
int fn_800F4668(dItem::Item *out, int num, const void *filter, const dItem::Item *exclude, int excludeNum) {
    static dNpcPair_c sPairs[3] = {
        dNpcPair_c(3, 5), dNpcPair_c(2, 5), dNpcPair_c(1, 5),
    };
    return fn_800C60B4(out, num, sPairs, 3, filter, exclude, excludeNum, 0);
}

// 800F46CC
void fn_800F46CC(const dAnmPersonalID_c *animal, int slot) {
    if (animal->isValid()) {
        fn_800CBC70(slot, animal);
    }
}

// 800F4718
void fn_800F4718(const dPersonalID_c *pid, int slot) {
    if (pid->isValid()) {
        fn_800CBBB0(slot, pid);
    }
}

// 800F4764
void fn_800F4764(const dAnmPersonalID_c *animal, int slot) {
    fn_800CBC10(slot, &animal->mLand);
}

// 800F4774
void fn_800F4774(const dItem::Item *item, int slot) {
    if (item->mId != dItem::ITEM_ID_NONE) {
        fn_800CBDA0(slot, item);
    }
}

// 800F4794
void fn_800F4794(u8 gender, u8 looks, int slot) {
    u16 msg = fn_800F3F38(gender, looks);
    if (msg != 0) {
        fn_800CBD30(slot, msg, "sys_STRING/STR_Unit");
    }
}

// ---------------------------------------------------------------------------
// Spare villager buffer access


// 800F47D8
void fn_800F47D8() {
    sAnimalBuf.clear();
}

// 800F47E4
BOOL fn_800F47E4(int arg) {
    return sAnimalBuf.fn_800EE440(arg);
}

// 800F47F4
BOOL fn_800F47F4() {
    return sAnimalBuf.fn_800EE4AC();
}

// 800F4800
void fn_800F4800() {
    if (fn_800DCEDC() && fn_800DCF30() == 2) {
        sAnimalBuf.fn_800EE514();
    }
}

// 800F4840
BOOL fn_800F4840(const dAnimal_c *src, u32 idx) {
    return sAnimalBuf.fn_800EE5C4(src, idx);
}

// 800F4858
dNpcAnimal_c *fn_800F4858() {
    return sAnimalBuf.fn_800EE818(0);
}

// 800F4868
BOOL fn_800F4868() {
    return sAnimalBuf.fn_800EE890(0);
}

// 800F4878
u32 fn_800F4878(dAnimal_c *dst, u32 idx) {
    return sAnimalBuf.fn_800EE76C(dst, idx);
}

// 800F4890
BOOL fn_800F4890(const dAnimal_c *src) {
    return sAnimalBuf.fn_800EE71C(src);
}

// ---------------------------------------------------------------------------
// Villager moves

// 800F48A0
BOOL fn_800F48A0(dAnimal_c *animal) {
    if (dSaveData_c::getTown()->mAnimals.takeMovedAnimal(animal, 0) && animal->mID.isValid()) {
        return TRUE;
    }
    return FALSE;
}

// 800F4904: removes the moved-out villager `id` from the town's second list.
BOOL fn_800F4904(const dAnmPersonalID_c *id) {
    if (!id->isValid()) {
        return FALSE;
    }
    dMovedAnimalList_c *list = &dSaveData_c::getTown()->mAnimals.mMoved;
    dAnimal_c *animal = list->getAnimal(list->getAnimalIdx(id));
    if (animal == NULL) {
        return FALSE;
    }
    animal->clear();
    return TRUE;
}

// 800F4984: adds `animal` to the pool of moved-out villagers (dMovedAnimalList_c).
BOOL fn_800F4984(dAnimal_c *animal) {
    if (!animal->mID.isValid()) {
        return FALSE;
    }
    if (!animal->isVersionValid()) {
        return FALSE;
    }
    if (!animal->isChecksumValid()) {
        return FALSE;
    }
    BOOL result = dSaveData_c::getTown()->mAnimals.addMovedAnimal(animal);
    return result;
}

// 800F4A08
dAnimal_c *fn_800F4A08() {
    return dSaveData_c::getTown()->mAnimals.pickMovedAnimal();
}

// 800F4A34
BOOL fn_800F4A34(dAnimal_c *animal) {
    if (!animal->mID.isValid()) {
        return FALSE;
    }
    if (!animal->isVersionValid()) {
        return FALSE;
    }
    if (!animal->isChecksumValid()) {
        return FALSE;
    }
    BOOL result = dSaveData_c::getTown()->mAnimals.addMovedAnimal(animal);
    return result;
}

// 800F4AB8: adds one random valid villager of `animals` to the moved-out pool.
BOOL fn_800F4AB8(dAnimal_c **animals) {
    if (animals == NULL) {
        return FALSE;
    }

    dAnimal_c *tmp = &sAnimalBuf.get(0)->mAnimal;
    dAnimalSave_c *save = &dSaveData_c::getTown()->mAnimals;
    u32 mask = 0;
    int num = 0;
    for (u32 i = 0; i < 6; i++) {
        tmp->clear();
        if (animals[i] != NULL) {
            cLib::memCpy(tmp, animals[i], sizeof(dAnimal_c));
            if (tmp->mID.isValid() && tmp->isChecksumValid() && tmp->isVersionValid()) {
                num++;
                mask |= 1 << i;
            }
        }
    }

    BOOL result = FALSE;
    if (num != 0 && mask != 0) {
        u32 pick = pickRandomBit(mask, num, 6);
        if (pick < 6 && animals[pick] != NULL) {
            tmp->clear();
            cLib::memCpy(tmp, animals[pick], sizeof(dAnimal_c));
            result = save->addMovedAnimal(tmp);
        }
    }
    tmp->clear();
    return result;
}

// 800F4C08: whether the villager `animal` is not standing on a blocked unit.
BOOL fn_800F4C08(dAnimal_c *animal, int arg) {
    if (!isCurrentSceneAttr(SCENE_ATTR_TOWN)) {
        return FALSE;
    }
    const dAnmPersonalID_c *id = &animal->mID;
    if (!id->isValid()) {
        return FALSE;
    }
    dItem::Item key = dSaveData_c::getRaw()->mAnimals.mTown.getAnimalKey(id);
    dActor_c *npc = fn_800F9878(&key);
    if (npc == NULL) {
        return FALSE;
    }
    if (*((u8 *)npc + 0x1C38) == 0) {
        return FALSE;
    }
    const mVec3_c *pos = npc->getPosP();
    if (pos == NULL) {
        return FALSE;
    }
    if (fn_80013F98((int)pos->x >> 5, (int)pos->z >> 5, arg)) {
        return FALSE;
    }
    return TRUE;
}

// 800F4D08
BOOL fn_800F4D08() {
    dAnimalBlock_c *block = &dSaveData_c::getTown()->mAnimals.mTown;
    dLandID_c *land = &dSaveData_c::getTown()->mLandID;
    if (block->getAnimalNum() < 6) {
        int arg = 0;
        void *data = fn_800F9F64(&arg);
        block->initNewTown(data, arg, land);
    }
    return TRUE;
}

struct dNpcOffset_c {
    /* 0x0 */ f32 mX;
    /* 0x4 */ f32 mZ;
};

// 800F4D84: hand item offset of villager species `idx` (axis 0: x, 1: z; dAcNpc_c::getHandItemOfsX/Z).
f32 getAnimalHandItemOfs(u8 idx, u32 axis) {
    static const dNpcOffset_c sOffsets[33] = {
        {-4.0f, 0.0f}, {-8.0f, 0.0f}, {-10.0f, 0.0f}, {-8.0f, 0.0f}, {-7.0f, 0.0f}, {0.0f, 3.0f}, {-5.0f, 2.0f},
        {-8.0f, 0.0f}, {-7.0f, 2.0f}, {-7.0f, 0.0f}, {-4.0f, 6.0f}, {-6.0f, 0.0f}, {-6.0f, 0.0f}, {-8.0f, 0.0f},
        {-8.0f, 0.0f}, {-6.0f, 0.0f}, {-4.0f, 4.0f}, {-6.0f, 0.0f}, {-8.0f, 0.0f}, {-6.0f, 0.0f}, {-8.0f, 0.0f},
        {-4.0f, 6.0f}, {-6.0f, 0.0f}, {-4.0f, 4.0f}, {-6.0f, 0.0f}, {-6.0f, 4.0f}, {-6.0f, 0.0f}, {-8.0f, 0.0f},
        {-5.0f, 0.0f}, {-6.0f, 0.0f}, {-8.0f, 0.0f}, {-4.0f, 4.0f}, {-6.0f, 0.0f},
    };

    if (idx < 33 && axis < 2) {
        return (&sOffsets[idx].mX)[axis];
    }
    return 0.0f;
}

// 800F4DB8: hand item offset of special NPC `key` (axis 0: x, 1: z).
f32 getSpHandItemOfs(const dItem::Item *key, u32 axis) {
    static const dNpcOffset_c sOffsets[97] = {
        {0.0f, 0.0f},  {-8.0f, 0.0f}, {-6.0f, 0.0f}, {0.0f, 0.0f},  {0.0f, 0.0f},  {-8.0f, 0.0f},  {-6.0f, 0.0f},
        {0.0f, 0.0f},  {0.0f, 0.0f},  {0.0f, 0.0f},  {0.0f, 0.0f},  {0.0f, 0.0f},  {0.0f, 0.0f},   {0.0f, 0.0f},
        {0.0f, 0.0f},  {0.0f, 0.0f},  {0.0f, 0.0f},  {0.0f, 0.0f},  {0.0f, 0.0f},  {0.0f, 0.0f},   {0.0f, 0.0f},
        {-8.0f, 0.0f}, {-8.0f, -4.0f}, {0.0f, 0.0f}, {0.0f, 0.0f},  {0.0f, 0.0f},  {0.0f, 0.0f},   {0.0f, 0.0f},
        {-8.0f, 0.0f}, {-10.0f, 0.0f}, {-6.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f},  {0.0f, 0.0f},   {0.0f, 0.0f},
        {0.0f, 0.0f},  {-8.0f, 0.0f}, {0.0f, 0.0f},  {0.0f, 0.0f},  {0.0f, 0.0f},  {-6.0f, 0.0f},  {0.0f, 0.0f},
        {0.0f, 0.0f},  {0.0f, 0.0f},  {-6.0f, 0.0f}, {-6.0f, 0.0f}, {-6.0f, 0.0f}, {-6.0f, 0.0f},  {-6.0f, 0.0f},
        {-6.0f, 0.0f}, {-6.0f, 0.0f}, {-6.0f, 0.0f}, {-6.0f, 0.0f}, {-6.0f, 0.0f}, {-6.0f, 0.0f},  {-6.0f, 0.0f},
        {-6.0f, 0.0f}, {-6.0f, 0.0f}, {-6.0f, 0.0f}, {-6.0f, 0.0f}, {-6.0f, 0.0f}, {-6.0f, 0.0f},  {-6.0f, 0.0f},
        {-6.0f, 0.0f}, {-6.0f, 0.0f}, {-6.0f, 0.0f}, {-6.0f, 0.0f}, {-6.0f, 0.0f}, {-6.0f, 0.0f},  {-6.0f, 0.0f},
        {-6.0f, 0.0f}, {-6.0f, 0.0f}, {-6.0f, 0.0f}, {-6.0f, 0.0f}, {-6.0f, 0.0f}, {0.0f, 0.0f},   {-8.0f, 0.0f},
        {0.0f, 0.0f},  {0.0f, 0.0f},  {0.0f, 0.0f},  {0.0f, 0.0f},  {0.0f, 0.0f},  {-8.0f, 0.0f},  {-8.0f, 0.0f},
        {-8.0f, 0.0f}, {-8.0f, 0.0f}, {-8.0f, 0.0f}, {-8.0f, 0.0f}, {-8.0f, 0.0f}, {-8.0f, 0.0f},  {-8.0f, 0.0f},
        {-8.0f, 0.0f}, {-7.0f, 0.0f}, {0.0f, 0.0f},  {0.0f, 0.0f},  {0.0f, 0.0f},  {-6.0f, 0.0f},
    };

    u16 k = key->mId;
    if (((k >> 12) & 0xF) == 8) {
        u32 idx = k & 0xFFF;
        if (idx < 97 && axis < 2) {
            return (&sOffsets[idx].mX)[axis];
        }
    }
    return 0.0f;
}

// ---------------------------------------------------------------------------
// Permits

// 800F4E00
dNpcPermit_c::dNpcPermit_c() {
    mState = 4;
    mFlags = 0;
    reset2();
}

// 800F4E40
dNpcPermit_c::~dNpcPermit_c() {}

// 800F4E80
void dNpcPermit_c::clear() {
    mState = 4;
    mFlags = 0;
    reset2();
}

// 800F4E94
void dNpcPermit_c::setState(u32 state) {
    if (!fn_800DCEDC() || fn_800DCF30() <= 1) {
        return;
    }
    if (state >= 4) {
        state = 4;
    }
    mState = state;
}

// 800F4EF4
BOOL dNpcPermit_c::hasState() {
    if (!fn_800DCEDC() || fn_800DCF30() <= 1) {
        return TRUE;
    }
    return (u32)mState < 4;
}

// 800F4F54
BOOL dNpcPermit_c::isFlag(u32 i) {
    if (i >= 4) {
        return FALSE;
    }
    return (mFlags >> i) & 1;
}

// 800F4F74
void dNpcPermit_c::setFlag(u32 i) {
    if (i < 4) {
        mFlags |= 1 << i;
    }
}

// 800F4F94
void dNpcPermit_c::set2(u32 value) {
    if (!fn_800DCEDC() || fn_800DCF30() <= 1) {
        return;
    }
    if (value >= 4) {
        value = 4;
    }
    _2 = value;
}

// 800F4FF4
void dNpcPermit_c::reset2() {
    _2 = 4;
}

// 800F5000
BOOL dNpcPermit_c::fn_800F5000() {
    if (!fn_800DCEDC() || fn_800DCF30() <= 1) {
        return TRUE;
    }
    int state = mState;
    if (state == 4) {
        if (fn_800DD960()) {
            return TRUE;
        }
    } else if (mFlags == 0 && fn_800DCF58() == state) {
        return TRUE;
    }
    return FALSE;
}

// 800F5094
BOOL dNpcPermit_c::fn_800F5094() {
    if (!fn_800DCEDC() || fn_800DCF30() <= 1) {
        return TRUE;
    }
    if (fn_800F5000()) {
        return TRUE;
    }
    for (int i = 0; i < 4; i++) {
        if (fn_800DCF2C(i) && !isFlag(i)) {
            return FALSE;
        }
    }
    return TRUE;
}

namespace dNpc {

// 800F513C
void permitMng_c::init() {
    dNpcPermit_c *permit = get(0);
    if (permit != NULL) {
        for (u32 i = 0; i < mNum; i++, permit++) {
            permit->clear();
        }
    }
}

// 800F51AC
dNpcPermit_c *permitMng_c::get(u32 i) {
    if (mEntries == NULL) {
        return NULL;
    }
    if (i >= mNum) {
        return NULL;
    }
    return &mEntries[i];
}

// 800F51E4
dNpcPermit_c *permitMng_c::getByKey(const dItem::Item *key) {
    return get(getIdx(key));
}

// 800F5228
int normalPermitMng_c::getIdx(const dItem::Item *key) {
    if (key->mId == dItem::ITEM_ID_NONE || !isNormalKey(key->mId)) {
        return -1;
    }
    return key->mId & 0xFFF;
}

// 800F52AC
int specialPermitMng_c::getIdx(const dItem::Item *key) {
    u16 k = key->mId;
    if (k == dItem::ITEM_ID_NONE || ((k >> 12) & 0xF) != 8) {
        return -1;
    }
    return k & 0xFFF;
}

} // namespace dNpc

static dNpc::normalPermitMng_c *sNormalPermit;   // lbl_8074E568
static dNpc::specialPermitMng_c *sSpecialPermit; // lbl_8074E56C

// 800F52D4
dNpc::permitMng_c *fn_800F52D4(const dItem::Item *key) {
    if (key->mId == dItem::ITEM_ID_NONE) {
        return NULL;
    }
    u16 k = key->mId;
    if (dNpc::isNormalKey(k)) {
        return sNormalPermit;
    }
    if (((k >> 12) & 0xF) == 8) {
        return sSpecialPermit;
    }
    return NULL;
}

// 800F5370
u32 fn_800F5370() {
    return sizeof(dNpc::normalPermitMng_c) + sizeof(dNpc::specialPermitMng_c);
}

// 800F5378
void fn_800F5378(EGG::Heap *heap) {
    if (heap != NULL) {
        if (sNormalPermit == NULL) {
            sNormalPermit = (dNpc::normalPermitMng_c *)heap->alloc(sizeof(dNpc::normalPermitMng_c), 4);
            if (sNormalPermit != NULL) {
                new (sNormalPermit) dNpc::normalPermitMng_c();
            }
        }
        if (sSpecialPermit == NULL) {
            sSpecialPermit = (dNpc::specialPermitMng_c *)heap->alloc(sizeof(dNpc::specialPermitMng_c), 4);
            if (sSpecialPermit != NULL) {
                new (sSpecialPermit) dNpc::specialPermitMng_c();
            }
        }
    }
}

// 800F548C
void fn_800F548C() {
    if (sNormalPermit != NULL) {
        sNormalPermit->init();
    }
    if (sSpecialPermit != NULL) {
        sSpecialPermit->init();
    }
}

// 800F54E0
dNpcPermit_c *fn_800F54E0(const dItem::Item *key) {
    dNpc::permitMng_c *mng = fn_800F52D4(key);
    if (mng != NULL) {
        return mng->getByKey(key);
    }
    return NULL;
}

static u8 sPermitPlayer = 0x44; // lbl_8074AE94

// 800F5524
void fn_800F5524(u8 player) {
    sPermitPlayer = player;
}

// 800F552C
BOOL fn_800F552C() {
    return (u32)sPermitPlayer < 0x44;
}

// 800F5548
void fn_800F5548() {
    fn_800F5524(0x44);
}

// 800F5550
BOOL fn_800F5550() {
    if (fn_800DD960()) {
        if (!fn_800DCEDC() || fn_800DCF30() <= 1) {
            return TRUE;
        }
        if (fn_800F552C()) {
            return TRUE;
        }
    }
    return FALSE;
}

// 800F55B0
void fn_800F55B0() {
    fn_800F548C();
}

// 800F55B4
void fn_800F55B4(int, int, int, u8 player) {
    fn_800F5524(player);
}

// 800F55BC
int fn_800F55BC() {
    fn_800F5548();
    return -1;
}

// ---------------------------------------------------------------------------

class dNpcLoader_c {
public:
    dNpcLoader_c() {
        _14 = 0;
        _18 = 0xFFFF;
    }

    /* 0x00 */ dDvd::loader_c mLoader;
    /* 0x14 */ u32 _14;
    /* 0x18 */ u16 _18;
}; // size 0x1C

class dNpcLoaders_c {
public:
    dNpcLoaders_c() {
        for (int i = 0; i < 6; i++) {
            mLoaders[i]._14 = 0;
            mLoaders[i]._18 = 0xFFFF;
        }
    }

    /* 0x00 */ dNpcLoader_c mLoaders[6];
}; // size 0xA8

static dNpcLoaders_c sLoaders; // lbl_805CE758
