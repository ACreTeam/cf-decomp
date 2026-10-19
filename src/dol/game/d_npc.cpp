// NPC / villager management. .text 800ECD90..800F59C8 (sinit 800F5768).
// See include/game/game/d_npc.hpp and notes/TUs/d_npc.txt; built with -sym on.
#include <game/game/d_npc.hpp>
#include <game/game/d_a_npc.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_str.hpp>
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
#include <game/game/d_net.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_npc_setup.hpp>
#include <game/game/d_npc_list.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_a_player.hpp>

namespace dNpc {

// Villager hours (first file-scope data, so they lead .rodata).
struct hours_s {
    /* 0x0 */ u8 mStartHour;
    /* 0x1 */ u8 mStartMin;
    /* 0x2 */ u8 mEndHour;
    /* 0x3 */ u8 mEndMin;
};

static const hours_s sSleepHours[LOOKS_TYPE_NUM] = {
    {1, 30, 8, 0}, {2, 0, 6, 30}, {4, 30, 10, 0}, {1, 0, 5, 0}, {2, 30, 7, 0}, {3, 30, 9, 0},
};

static const hours_s sDaytimeHours[LOOKS_TYPE_NUM] = {
    {10, 0, 20, 0}, {9, 0, 22, 0}, {12, 0, 23, 0}, {8, 0, 19, 0}, {10, 0, 21, 0}, {11, 0, 22, 0},
};

// ---------------------------------------------------------------------------
// msgMemory_c

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
BOOL msgMemory_c::isValid() const {
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
    mJinxTown = -1;
    mPersonal.clear();
    mPlayer.clear();
    mLand.clear();
    mRumorTown = -1;
    mItem0 = dItem::ITEM_ID_NONE;
    mItem1 = dItem::ITEM_ID_NONE;
    memset(mText, 0, sizeof(mText));
    mMessageYear = -1;
    mImpression = 50;
}

// 800ED01C
BOOL msgMemorySecond_c::setJinxTown(int value) {
    if (isValid()) {
        mJinxTown = value;
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
BOOL msgMemorySecond_c::setRumorTown(u32 value) {
    if (value < 4) {
        mRumorTown = value;
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
BOOL msgMemorySecond_c::setImpression(u32 value) {
    if (value < 50) {
        mImpression = value;
        return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------
// timer_c

// 800ED240
timer_c::timer_c() {}

// 800ED244
timer_c::~timer_c() {}

// 800ED284
void timer_c::clear() {
    mEndTick = 0;
    mMode = 0;
    mFrames = 0;
}

// 800ED29C
void timer_c::reset() {
    mEndTick = 0;
    mMode = 5;
    mFrames = 0;
}

// 800ED2B8
void timer_c::addFrames(u32 frameCount) {
    u32 total = frameCount + mFrames;
    if (total > 0xFFFF) {
        total = 0xFFFF;
    }
    mFrames = total;
}

// 800ED2D8
void timer_c::set(int mode, u16 frameCount) {
    if ((u32)mode >= 5) {
        mode = 0;
    }
    if (mode == 0) {
        mMode = mode;
        mFrames = 0;
        mEndTick = 0;
        return;
    }

    if (mode == getMode()) {
        addFrames(frameCount);
    } else {
        mMode = mode;
        mFrames = frameCount;
    }

    dTime_c now = *dTime_c::getCurrent();
    int frames = mFrames;
    f32 seconds = (f32)(frames % 60) / 60.0f;
    int msecs = 1000.0f * seconds;
    int usecs = 1000.0f * (1000.0f * seconds - msecs);
    s64 ticks = OSCalendarTimeToTicks(&now);
    ticks += OS_SEC_TO_TICKS(frames / 60);
    ticks += OS_MSEC_TO_TICKS(msecs);
    ticks += OS_USEC_TO_TICKS(usecs);
    mEndTick = ticks;
}

// 800ED4C0
void timer_c::copy(const timer_c *other) {
    set(((timer_c *)other)->getMode(), other->mFrames);
}

// 800ED50C
void timer_c::tick() {
    if (mFrames != 0) {
        mFrames--;
    }
    if (mFrames == 0 && mMode != 0) {
        mMode = 0;
    }
}

// 800ED544
int timer_c::getMode() {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        return 0;
    }
    return mMode;
}

// 800ED590
void timer_c::update() {
    if (mMode != 0) {
        dTime_c now = *dTime_c::getCurrent();
        s64 ticks = OSCalendarTimeToTicks(&now);
        if (mEndTick <= ticks) {
            mEndTick = 0;
            mFrames = 0;
            mMode = 0;
        } else {
            mFrames = (mEndTick - ticks) * 60 / OS_TIME_SPEED;
        }
    }
}

// ---------------------------------------------------------------------------
// talkWeight_c

// 800ED690
talkWeight_c::talkWeight_c() {}

// 800ED694
talkWeight_c::~talkWeight_c() {}

// 800ED6D4
void talkWeight_c::clear() {
    mWeight = 0;
}

// 800ED6E0
void talkWeight_c::initFromFriendship(dAnimal_c *animal, dPrivateData_c *player) {
    if (player == NULL) {
        player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    }

    clear();
    add(calcFriendshipValue(10, 1, animal, player));
    if (!animal->mID.isValid() || player == NULL) {
        return;
    }
    if (dSaveData_c::getTown()->mAnimals.mTown.isMoveOutAnimal(&animal->mID)) {
        dAnimalMemory_c *memory = animal->findMemory2(&player->mPID);
        if (memory == NULL || !memory->mPlayer.isValid()) {
            return;
        }
        if (!((*(u32 *)memory >> 23) & 1)) {
            set(32);
        }
    }
}

// 800ED7D4
int talkWeight_c::calcFriendshipValue(u32 max, u32 min, dAnimal_c *animal, dPrivateData_c *player) {
    if (min > max) {
        return 0;
    }
    if (player == NULL) {
        player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
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
    u32 value = min + cM::rndInt(max - min + 1);
    f32 result = value;
    result = (256.0f + friendship) * result;
    return result / 128.0f;
}

// 800ED92C
void talkWeight_c::set(u32 value) {
    if (value >= 32) {
        value = 32;
    }
    mWeight = value;
}

// 800ED940
void talkWeight_c::add(int delta) {
    int value = mWeight + delta;
    if (value < 0) {
        value = 0;
    } else if ((u32)value >= 32) {
        value = 32;
    }
    set(value);
}

// 800ED964
void talkWeight_c::half() {
    set((u32)mWeight >> 1);
}

// 800ED970
void talkWeight_c::twice() {
    set(mWeight << 1);
}

// ---------------------------------------------------------------------------
// entry_c

// 800ED97C
entry_c::entry_c() {}

// 800ED9D4
entry_c::~entry_c() {}

// 800EDA6C
void entry_c::clear() {
    memset(&mFlags, 0, sizeof(mFlags));
    mTopicMemory.clear();
    mRememberedMessage.clear();
    mDialogueCache.clear();
    mQuest.clear();
    mMoodTimer.clear();
    mTime.init();
    mStallBuyCount = 0;
    mStallRefuseCount = 0;
    mTalkWeight.clear();
}

// 800EDB00
dialogueCache_c::dialogueCache_c() {}

// 800EDB04
dialogueCache_c::~dialogueCache_c() {}

// 800EDB44
void dialogueCache_c::clear() {
    for (int i = 0; i < MSG_RANDOM_WORD_KIND_NUM; i++) {
        for (int j = 0; j < MSG_WORD_NUM; j++) {
            mWordIds[i][j] = 0;
        }
    }
    clearAnimalIndices();
}

// 800EDBB0
void dialogueCache_c::clearAnimalIndices() {
    for (int i = 0; i < 6; i++) {
        mAnimalIndices[i] = -1;
    }
}

// 800EDBD0
void dialogueCache_c::setWord(u32 i, u16 value, u32 j) {
    if (i < MSG_RANDOM_WORD_KIND_NUM && j < MSG_WORD_NUM) {
        mWordIds[i][j] = value;
    }
}

// 800EDBF4
u16 dialogueCache_c::getWord(u32 i, u32 j) {
    if (i < MSG_RANDOM_WORD_KIND_NUM && j < MSG_WORD_NUM) {
        return mWordIds[i][j];
    }
    return 0;
}

// 800EDC20
int dialogueCache_c::getAnimalIdx(u32 i) {
    if (i < ANIMAL_NUM) {
        return mAnimalIndices[i];
    }
    return -1;
}

// 800EDC40
void dialogueCache_c::setAnimalIdx(u32 i, int value) {
    if (i < ANIMAL_NUM) {
        mAnimalIndices[i] = value;
    }
}

// 800EDC54: the villager remembered in slot i, or a random new one other than `exclude`.
dAnimal_c *dialogueCache_c::pickAnimal(u32 i, const dAnmPersonalID_c **exclude, u32 numExclude) {
    if (i >= ANIMAL_NUM) {
        return NULL;
    }

    dAnimalBlock_c *block = &dSaveData_c::getTown()->mAnimals.mTown;
    const dAnmPersonalID_c **p;
    const dAnmPersonalID_c **base;
    dAnimal_c *animal = block->getAnimal(getAnimalIdx(i));
    if (animal == NULL || !animal->mID.isValid()) {
        u32 num = 0;
        const dAnmPersonalID_c *list[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
        base = list;

        if (exclude != NULL) {
            u32 n = numExclude < 2 ? numExclude : 2;
            const dAnmPersonalID_c **p = base;
            for (u32 k = 0; k < n; k++, exclude++) {
                if (*exclude != NULL && num < 8) {
                    *p++ = *exclude;
                    num++;
                }
            }
        }

        p = &base[num];
        for (int k = 0; k < ANIMAL_NUM; k++) {
            if (k != i) {
                dAnimal_c *other = block->getAnimal(getAnimalIdx(k));
                if (other != NULL && other->mID.isValid() && num < 8) {
                    num++;
                    *p++ = &other->mID;
                }
            }
        }

        animal = block->pickRandomAnimal(base, num, 0);
        if (animal != NULL) {
            u32 idx = block->getAnimalIdx(&animal->mID);
            if (idx < ANIMAL_NUM) {
                setAnimalIdx(i, idx);
            }
        }
    }
    return animal;
}

// ---------------------------------------------------------------------------
// list_c / listLand_c

// 800EDE0C
void list_c::clear() {
    entry_c *entry = mEntries;
    if (entry != NULL) {
        for (u32 i = 0; i < mEntryCount; i++, entry++) {
            entry->clear();
        }
    }
}

// 800EDE74
void list_c::init() {
    clear();
}

// 800EDE84
entry_c *list_c::get(u32 i) {
    if (i < mEntryCount) {
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

// ---------------------------------------------------------------------------
// Villager compatibility

// 800EDF18: personality x personality (lbl_80474FA0).
u32 getPersonalityCompatibility(u8 a, u8 b) {
    static const u8 sPersonalityCompatibility[LOOKS_TYPE_NUM][LOOKS_TYPE_NUM] = {
        {0x28, 0x60, 0x08, 0x08, 0x28, 0x60},
        {0x60, 0x08, 0x28, 0x28, 0x60, 0x08},
        {0x08, 0x28, 0x60, 0x60, 0x08, 0x28},
        {0x08, 0x28, 0x60, 0x28, 0x08, 0x60},
        {0x28, 0x60, 0x08, 0x08, 0x60, 0x28},
        {0x60, 0x08, 0x28, 0x60, 0x28, 0x08},
    };
    if (a < LOOKS_TYPE_NUM && b < LOOKS_TYPE_NUM) {
        return sPersonalityCompatibility[a][b];
    }
    return 0;
}

// 800EDF48: star sign -> element (lbl_80474FC4).
u32 getStarSignElement(u32 sign) {
    static const u8 sStarSignElements[12] = {1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0};
    if (sign < 12) {
        return sStarSignElements[sign];
    }
    return 4;
}

// 800EDF68: element x element (lbl_80474FD0).
u32 getStarSignCompatibility(u32 signA, u32 signB) {
    static const u8 sElementCompatibility[4][4] = {
        {0x80, 0x30, 0x30, 0x00},
        {0x30, 0x80, 0x00, 0x30},
        {0x30, 0x00, 0x80, 0x30},
        {0x00, 0x30, 0x30, 0x80},
    };
    u32 a = getStarSignElement(signA);
    u32 b = getStarSignElement(signB);
    if (a < 4 && b < 4) {
        return sElementCompatibility[a][b];
    }
    return 0;
}

// 800EDFD4
u32 getBirthdayCompatibility(int monthA, int dayA, int monthB, int dayB) {
    u32 a = dTime_c::getStarSign(monthA, dayA);
    return getStarSignCompatibility(a, dTime_c::getStarSign(monthB, dayB));
}

// 800EE030: finds the unordered pair (a, b) in `pairs`.
const u8 *findSpeciesPair(u32 a, u32 b, const u8 *pairs, int num) {
    if (a < NPC_SPECIES_NUM && b < NPC_SPECIES_NUM) {
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
u32 getSpeciesCompatibility(u32 a, u32 b) {
    static const u8 sCompatibleSpecies[7][2] = {
        {0x04, 0x09}, {0x0D, 0x0E}, {0x1D, 0x15}, {0x03, 0x19}, {0x05, 0x0A}, {0x12, 0x02}, {0x13, 0x00},
    };
    static const u8 sIncompatibleSpecies[4][2] = {{0x04, 0x1B}, {0x04, 0x20}, {0x09, 0x02}, {0x00, 0x0A}};

    u32 score = 0;
    if (a < NPC_SPECIES_NUM && b < NPC_SPECIES_NUM) {
        if (findSpeciesPair(a, b, sCompatibleSpecies[0], 7)) {
            score = 0x80;
        } else if (a == b) {
            score = 0x40;
        } else if (!findSpeciesPair(a, b, sIncompatibleSpecies[0], 4)) {
            score = 0x20;
        }
    }
    return score;
}

static inline u32 getBirthDayByte(dAnimal_c *animal) {
    return (u8)animal->getBirthDay();
}

static inline u32 getBirthMonthByte(dAnimal_c *animal) {
    return (u8)animal->getBirthMonth();
}

// 800EE138: compatibility score between two villagers.
u32 calcAnimalCompatibilityScore(dAnimal_c *a, dAnimal_c *b) {
    dAnmPersonalID_c *idA;
    dAnmPersonalID_c *idB;
    u32 score = 0;
    if (a != NULL && b != NULL) {
        idA = &a->mID;
        idB = &b->mID;
        if (idA->isValid() && idB->isValid()) {
            u32 looks = getPersonalityCompatibility(idA->getLooks(1), idB->getLooks(1));
            u8 dayB = getBirthDayByte(b);
            u8 monthB = getBirthMonthByte(b);
            u8 dayA = getBirthDayByte(a);
            u32 signs = getBirthdayCompatibility((u8)getBirthMonthByte(a), dayA, monthB, dayB);
            u32 speciesB = b->getSpecies();
            score = getSpeciesCompatibility((u8)a->getSpecies(), (u8)speciesB) + (signs + looks);
        }
    }
    return score;
}

// 800EE244: 0 = good, 1 = okay, 2 = bad.
int calcAnimalCompatibilityRank(dAnimal_c *a, dAnimal_c *b) {
    int rank = 2;
    u32 score = calcAnimalCompatibilityScore(a, b);
    if (score >= 200) {
        rank = 0;
    } else if (score >= 61) {
        rank = 1;
    }
    return rank;
}

// ---------------------------------------------------------------------------
// bufferedAnimal_c / animalBuffer_c: four spare villagers (lbl_805C1000).

// 800EE290
bufferedAnimal_c::bufferedAnimal_c() {}

// 800EE2C0
bufferedAnimal_c::~bufferedAnimal_c() {}

// 800EE318
void bufferedAnimal_c::clear() {
    mAnimal.clear();
}

// 800EE31C
animalBuffer_c::animalBuffer_c() {
    clear();
}

// 800EE36C
animalBuffer_c::~animalBuffer_c() {}

// 800EE3D0
void animalBuffer_c::clear() {
    bufferedAnimal_c *animal = mAnimals;
    for (int i = 0; i < NET_MEMBER_MAX; i++, animal++) {
        animal->clear();
    }
}

// 800EE41C
BOOL animalBuffer_c::isValidIdx(u32 i) {
    return i < NET_MEMBER_MAX;
}

// 800EE434
bufferedAnimal_c *animalBuffer_c::get(int i) {
    return &mAnimals[i];
}

// 800EE440
BOOL animalBuffer_c::takeMovedAnimal(int arg) {
    dSaveTown_c *save = dSaveData_c::getTown();
    bufferedAnimal_c *animal = get(0);
    animal->clear();
    return save->mAnimals.takeMovedAnimal(&animal->mAnimal, arg);
}

// 800EE4AC
BOOL animalBuffer_c::storeMovedAnimal() {
    dSaveTown_c *save = dSaveData_c::getTown();
    bufferedAnimal_c *animal = get(0);
    BOOL result = save->mAnimals.addMovedAnimal(&animal->mAnimal);
    animal->clear();
    return result;
}

// 800EE514
void animalBuffer_c::exchangeMovedAnimal() {
    dAnimalSave_c *animals;
    bufferedAnimal_c *first = get(0);
    animals = &dSaveData_c::getTown()->mAnimals;
    bufferedAnimal_c *second = get(1);
    BOOL moved = animals->takeMovedAnimal(&second->mAnimal, 1);
    animals->addMovedAnimal(&first->mAnimal);
    first->clear();
    if (moved) {
        cLib::memCpy(first, second, sizeof(bufferedAnimal_c));
        second->mAnimal.clear();
    }
}

// 800EE5C4
BOOL animalBuffer_c::exchangeWithRandomMember(const dAnimal_c *src, u32 idx) {
    if (!isValidIdx(idx)) {
        return FALSE;
    }

    u32 pick = NET_MEMBER_NONE;
    u32 count = 0;
    for (int i = 0; i < NET_MEMBER_MAX; i++) {
        if (i != (int)idx && dNet::isMember(i)) {
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

    bufferedAnimal_c *picked = get(pick);
    cLib::memCpy(get(idx), picked, sizeof(bufferedAnimal_c));
    if (src != NULL) {
        cLib::memCpy(picked, src, sizeof(bufferedAnimal_c));
    } else {
        picked->clear();
    }
    return TRUE;
}

// 800EE71C
BOOL animalBuffer_c::setFirst(const dAnimal_c *src) {
    if (src == NULL) {
        return FALSE;
    }
    cLib::memCpy(get(0), src, sizeof(bufferedAnimal_c));
    return TRUE;
}

// 800EE76C
u32 animalBuffer_c::take(dAnimal_c *dst, u32 idx) {
    if (dst == NULL) {
        return 0;
    }
    if (!isValidIdx(idx)) {
        return 0;
    }

    bufferedAnimal_c *animal = get(idx);
    if (animal->mAnimal.mID.isValid()) {
        cLib::memCpy(dst, animal, sizeof(bufferedAnimal_c));
        animal->clear();
        return sizeof(bufferedAnimal_c);
    }
    return 0;
}

// 800EE818
bufferedAnimal_c *animalBuffer_c::getValid(u32 idx) {
    if (!isValidIdx(idx)) {
        return NULL;
    }

    bufferedAnimal_c *animal = get(idx);
    if (animal->mAnimal.mID.isValid()) {
        return animal;
    }
    return NULL;
}

// 800EE890
BOOL animalBuffer_c::clearAt(u32 idx) {
    if (!isValidIdx(idx)) {
        return FALSE;
    }
    get(idx)->clear();
    return TRUE;
}

// ---------------------------------------------------------------------------
// daub_c: a 0x2F-byte packed record shared over the network.

// 800EE8F0
daub_c::daub_c() {}

// 800EE8F4
daub_c::daub_c(const daub_c &other) {
    copy(&other);
}

// 800EE924
daub_c::~daub_c() {}

// 800EE964
void daub_c::clear() {
    memset(this, 0, sizeof(*this));
    _0A = 4;
    _0B = 4;
    mScene = dScene::SCENE_NUM;
}

// 800EE9AC
void daub_c::copy(const daub_c *other) {
    cLib::memCpy(mPosX, other->mPosX, 4);
    cLib::memCpy(mPosZ, other->mPosZ, 4);
    cLib::memCpy(mAngleY, other->mAngleY, 2);
    cLib::memCpy(&_0A, &other->_0A, 1);
    cLib::memCpy(&_0B, &other->_0B, 1);
    cLib::memCpy(&mScene, &other->mScene, 1);
    cLib::memCpy(mHeadPitch, other->mHeadPitch, 2);
    cLib::memCpy(mHeadYaw, other->mHeadYaw, 2);
    cLib::memCpy(mNpcData, other->mNpcData, 8);
    cLib::memCpy(mActionData, other->mActionData, 0x15);
    mValid = other->mValid;
}

// 800EEA88: packs x/z of a vector into 8 bytes.
void daub_c::packXZ(u8 *dst, const mVec3_c *src) {
    cLib::memCpy(dst, &src->x, 4);
    cLib::memCpy(dst + 4, &src->z, 4);
}

// 800EEAD4
void daub_c::unpackXZ(const u8 *src, mVec3_c *dst) {
    cLib::memCpy(&dst->x, src, 4);
    cLib::memCpy(&dst->z, src + 4, 4);
}

// 800EEB28
void daub_c::setAngleY(const u16 *value) {
    cLib::memCpy(mAngleY, value, 2);
}

// 800EEB34
mAng daub_c::getAngleY() {
    mAng value;
    cLib::memCpy(&value, mAngleY, 2);
    return value;
}

// 800EEB6C
void daub_c::setHeadPitch(mAng value) {
    cLib::memCpy(mHeadPitch, &value, 2);
}

// 800EEB78
mAng daub_c::getHeadPitch() {
    mAng value;
    cLib::memCpy(&value, mHeadPitch, 2);
    return value;
}

// 800EEBB0
void daub_c::setHeadYaw(mAng value) {
    cLib::memCpy(mHeadYaw, &value, 2);
}

// 800EEBBC
mAng daub_c::getHeadYaw() {
    mAng value;
    cLib::memCpy(&value, mHeadYaw, 2);
    return value;
}

// 800EEBF4
void daub_c::getNpcData(void *dst, u32 size) {
    if (dst != NULL && size != 0) {
        cLib::memCpy(dst, mNpcData, size > 8 ? 8 : size);
    }
}

// 800EEC2C
void daub_c::setNpcData(const void *src, u32 size) {
    if (src != NULL && size != 0) {
        cLib::memCpy(mNpcData, src, size > 8 ? 8 : size);
    }
}

// 800EEC5C
void daub_c::getAction(u32 *action, u32 *priority) {
    if (action != NULL) {
        *action = mActionData[0];
    }
    if (priority != NULL) {
        *priority = mActionData[1];
    }
}

// 800EEC80
void daub_c::setAction(u32 action, u32 priority) {
    mActionData[0] = action;
    mActionData[1] = priority;
}

// 800EEC8C
void daub_c::getMove(mVec3_c *target, mVec3_c *goal, u16 *turnSpeed, f32 *morph) {
    if (target != NULL) {
        cLib::memCpy(&target->x, &mActionData[2], 4);
        cLib::memCpy(&target->z, &mActionData[6], 4);
    }
    if (goal != NULL) {
        cLib::memCpy(&goal->x, &mActionData[10], 4);
        cLib::memCpy(&goal->z, &mActionData[14], 4);
    }
    if (turnSpeed != NULL) {
        cLib::memCpy(turnSpeed, &mActionData[18], 2);
    }
    if (morph != NULL) {
        *morph = mActionData[20];
    }
}

// 800EED5C
void daub_c::setMove(const mVec3_c *target, const mVec3_c *goal, const u16 *turnSpeed, f32 morph) {
    cLib::memCpy(&mActionData[2], &target->x, 4);
    cLib::memCpy(&mActionData[6], &target->z, 4);
    cLib::memCpy(&mActionData[10], &goal->x, 4);
    cLib::memCpy(&mActionData[14], &goal->z, 4);
    cLib::memCpy(&mActionData[18], turnSpeed, 2);
    mActionData[20] = morph;
}

// 800EEE10
void daub_c::getTalk(u16 *angle, u16 *turnSpeed, f32 *morph) {
    if (angle != NULL) {
        cLib::memCpy(angle, &mActionData[2], 2);
    }
    if (turnSpeed != NULL) {
        cLib::memCpy(turnSpeed, &mActionData[4], 2);
    }
    if (morph != NULL) {
        *morph = mActionData[6];
    }
}

// 800EEEA8
void daub_c::setTalk(const u16 *angle, const u16 *turnSpeed, f32 morph) {
    cLib::memCpy(&mActionData[2], angle, 2);
    cLib::memCpy(&mActionData[4], turnSpeed, 2);
    mActionData[6] = morph;
}

// 800EEF14
void daub_c::getAct(u32 *animation, u8 *playMode, f32 *morph, f32 *frame) {
    if (animation != NULL) {
        u16 value = 0;
        cLib::memCpy(&value, &mActionData[2], 2);
        *animation = value;
    }
    if (playMode != NULL) {
        *playMode = mActionData[4];
    }
    if (morph != NULL) {
        *morph = mActionData[5];
    }
    if (frame != NULL) {
        *frame = mActionData[6];
    }
}

// 800EEFDC
void daub_c::setAct(u32 animation, u8 playMode, f32 morph, f32 frame) {
    u16 value = animation;
    cLib::memCpy(&mActionData[2], &value, 2);
    mActionData[4] = playMode;
    mActionData[5] = morph;
    mActionData[6] = frame;
}

// ---------------------------------------------------------------------------
// daubMng_c

// 800EF060
daubMng_c::daubMng_c(daub_c *daubs, u32 num) : mDaubs(daubs), mDaubCount(num) {}

// 800EF078
daubMng_c::~daubMng_c() {
    mDaubs = NULL;
    mDaubCount = 0;
}

// 800EF0C4
void daubMng_c::clear() {
    daub_c *daub = mDaubs;
    for (u32 i = 0; i < mDaubCount; i++, daub++) {
        daub->clear();
    }
}

// 800EF124
void daubMng_c::init() {
    clear();
}

// 800EF128
BOOL daubMng_c::isValidIdx(u32 idx) {
    return idx < mDaubCount;
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
        int base = getBase();
        return base + idx;
    }
    return NET_REC_NUM;
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
daub_c *daubMng_c::get(u32 idx) {
    if (isValidIdx(idx)) {
        return &mDaubs[idx];
    }
    return NULL;
}

// 800EF2EC
daub_c *daubMng_c::getByKey(const u16 *key) {
    return get(getIdx(key));
}

// 800EF330
daub_c *daubMng_c::getById(int id) {
    return get(idToIdx(id));
}

// 800EF368
daubMngNormal_c::daubMngNormal_c() : daubMng_c(mStorage, ANIMAL_NUM) {}

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
    BOOL type;
    BOOL valid = FALSE;
    type = FALSE;
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
    u16 value = *key;
    if (!isNormalKey(*key)) {
        return -1;
    }
    return value & 0xFFF;
}

// 800EF4B8: places each normal daub at its villager's house.
void daubMngNormal_c::initHousePositions() {
    dAnimalBlock_c *block = &dSaveData_c::getTown()->mAnimals.mTown;
    for (u32 i = 0; i < mDaubCount; i++) {
        daub_c *daub = get(i);
        if (daub != NULL) {
            mVec3_c pos = mVec3_c::Zero;
            if (block->getHouseFrontPos(&pos, i)) {
                daub_c::packXZ(daub->mPosX, &pos);
                daub->mScene = dScene::SCENE_FIELD;
                daub->mValid = 1;
            }
        }
    }
}

// 800EF57C
daubMngSpecial_c::daubMngSpecial_c() : daubMng_c(mStorage, NPC_SPECIAL_NUM) {}

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

// 800EF670: finds the placement whose key matches in layout `layout`.
BOOL getLayoutPlacement(mVec3_c *pos, s16 *angle, const dItem::Item &key, u8 layout) {
    layout_s *data = (layout_s *)dScene::getSceneData(layout);
    if (data != NULL) {
        int num = data->mGroupCount;
        layoutActor_s *entry;
        for (int i = 0; i < num; i++) {
            layoutGroup_s *group = &data->mGroups[i];
            layoutActor_s *entries;
            if (group->mType == dScene::SCENE_DATA_ACTOR && (entries = group->mEntries) != NULL) {
                entry = entries;
                for (int j = 0; j < group->mEntryCount; entry++, j++) {
                    if (entry->mNpcKey == key.mId) {
                        layoutActor_s *found = &entries[j];
                        pos->x = found->mPos.x;
                        pos->y = found->mPos.y;
                        pos->z = found->mPos.z;
                        *angle = found->mAngleY;
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}

listLand_c sTownEntries;            // lbl_805BF610
static animalBuffer_c sAnimalBuffer;            // lbl_805C1000
static daubMngNormal_c sNormalDaubs;   // lbl_805CD10C
static daubMngSpecial_c sSpecialDaubs; // lbl_805CD2FC

// 800EF768: places each special NPC's daub at its spot.
void daubMngSpecial_c::initSpecialPositions() {
    static const dItem::Item sSpecialNpcKeys[19] = {
        0x801E, 0x8006, 0x8024, 0x8001, 0x8019, 0x801A, 0x801D, 0x8026, 0x8027, 0x8002,
        0x800F, 0x8010, 0x8005, 0x8007, 0x8017, 0x8008, 0x8009, 0x800D, 0x800E,
    };

    const dItem::Item *key = sSpecialNpcKeys;
    for (u32 i = 0; i < 19; i++, key++) {
        daub_c *daub = getByKey(&key->mId);
        if (daub != NULL) {
            mVec3_c pos = mVec3_c::Zero;
            s16 angle = cM::rndInt(0xFFFF);
            u16 id = key->mId;
            switch (id) {
                case 0x801E:
                    if ((*((u8 *)dSaveData_c::getTown() + 0x66747) >> 6) & 1) {
                        dSaveTown_c *town = dSaveData_c::getTown();
                        pos = town->mHarvestSpot.getPos();
                    } else {
                        pickOutdoorPlacementPos(&pos, NULL, 4, NULL, BLOCK_KIND_FLAG_TOWN_HALL, TRUE, TRUE, 0.0f);
                    }
                    angle = 0;
                    break;
                case 0x8019:
                case 0x801A:
                    pickPosWithNorthClearance(&pos);
                    break;
                case 0x8026:
                case 0x8027:
                    getLayoutPlacement(&pos, &angle, id, dScene::SCENE_RM_CHKP);
                    break;
                case 0x8002:
                    getLayoutPlacement(&pos, &angle, id, dScene::SCENE_RM_SHOP0);
                    break;
                case 0x800F:
                case 0x8010:
                    getLayoutPlacement(&pos, &angle, id, dScene::SCENE_RM_SHOP3_F2);
                    break;
                case 0x8005:
                    getLayoutPlacement(&pos, &angle, id, dScene::SCENE_RM_MM_ENT);
                    break;
                case 0x8007:
                case 0x8017:
                    getLayoutPlacement(&pos, &angle, id, dScene::SCENE_RM_MM_CAFE);
                    break;
                case 0x8008:
                case 0x8009:
                    getLayoutPlacement(&pos, &angle, id, dScene::SCENE_RM_TAILOR);
                    break;
                case 0x800D:
                    getLayoutPlacement(&pos, &angle, id, dScene::SCENE_RM_OFFICE);
                    break;
                case 0x800E:
                    getLayoutPlacement(&pos, &angle, id, dScene::SCENE_RM_MM_ASTRO);
                    break;
                default:
                    pickOutdoorFreePos(&pos, NULL, 4, NULL, 0.0f);
                    break;
            }
            daub_c::packXZ(daub->mPosX, &pos);
            daub->setAngleY((u16 *)&angle);
            daub->mScene = dScene::SCENE_FIELD;
            daub->mValid = 1;
        }
    }
}

// ---------------------------------------------------------------------------
// Daub manager access

static daubMng_c *sDaubManagers[2] = {&sNormalDaubs, &sSpecialDaubs}; // lbl_8074AE88

// 800EFBBC
daubMng_c *getDaubManager(u32 i) {
    if (i < 2) {
        return sDaubManagers[i];
    }
    return NULL;
}

// 800EFBDC
daubMng_c *getDaubManagerByKey(const u16 *key) {
    for (int i = 0; i < 2; i++) {
        daubMng_c *mng = getDaubManager(i);
        if (mng != NULL && mng->isValidKey(key)) {
            return mng;
        }
    }
    return NULL;
}

// 800EFC54
daubMng_c *getDaubManagerById(int id) {
    for (int i = 0; i < 2; i++) {
        daubMng_c *mng = getDaubManager(i);
        if (mng != NULL && mng->isValidId(id)) {
            return mng;
        }
    }
    return NULL;
}

// 800EFCCC
void initDaubManagers() {
    sNormalDaubs.init();
    sSpecialDaubs.init();
    dNet::isSessionActive();
}

// 800EFD04
daub_c *getDaubByKey(const u16 *key) {
    daubMng_c *mng = getDaubManagerByKey(key);
    if (mng != NULL) {
        return mng->getByKey(key);
    }
    return NULL;
}

// 800EFD48
void initNormalDaubs() {
    daubMngNormal_c *mng = &sNormalDaubs;
    mng->initHousePositions();
    if (dNet::isSessionActive()) {
        for (int i = 0; i < ANIMAL_NUM; i++) {
            u16 key = (i & 0x3FF) + 0xE000;
            int id = mng->keyToId(&key);
            if (id != NET_REC_NUM) {
                dNet::setRecord(id, 0, 0);
            }
        }
    }
}

// 800EFDD0
void copyNormalDaubRecord(void *dst, int id) {
    copyDaubRecord(dst, id);
}

// 800EFDD4
void initSpecialDaubs() {
    daubMngSpecial_c *mng = &sSpecialDaubs;
    mng->initSpecialPositions();
    if (dNet::isSessionActive()) {
        u32 num = mng->mDaubCount;
        for (u32 i = 0; i < num; i++) {
            u16 key = (i & 0xFFF) | 0x8000;
            int id = mng->keyToId(&key);
            if (id != NET_REC_NUM) {
                dNet::setRecord(id, 0, 0);
            }
        }
    }
}

// 800EFE68
void copySpecialDaubRecord(void *dst, int id) {
    copyDaubRecord(dst, id);
}

// 800EFE6C
const daub_c *getReceivedDaub(const u16 *key) {
    if (!dNet::isSessionActive()) {
        return NULL;
    }
    daubMng_c *mng = getDaubManagerByKey(key);
    if (mng == NULL) {
        return NULL;
    }
    int id = mng->keyToId(key);
    if (id == NET_REC_NUM) {
        return NULL;
    }
    return (const daub_c *)dNet::getRecord(id);
}

// 800EFEDC
void copyDaubRecord(void *dst, int id) {
    daubMng_c *mng = getDaubManagerById(id);
    if (mng != NULL) {
        daub_c *daub = mng->getById(id);
        if (daub != NULL) {
            daub->mValid = 1;
            cLib::memCpy(dst, daub, dNet::getRecordSize(id));
        }
    }
}

// 800EFF60
void parseAnimalHeldItemPacket(int *idx, void *dst, const u8 *src) {
    if (src[2] < ANIMAL_NUM) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    cLib::memCpy(dst, src, 2);
}

// 800EFF8C
void buildAnimalHeldItemPacket(u8 *dst, int idx, const void *src) {
    dst[2] = idx;
    cLib::memCpy(dst, src, 2);
}

// 800EFFA0
void sendAnimalHeldItem(u32 idx, const void *data) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1 && idx < ANIMAL_NUM) {
        u8 buf[3];
        buildAnimalHeldItemPacket(buf, idx, data);
        dNet::beginPacket();
        dNet::addPacket(buf, 3);
        dNet::sendPacket(NET_PACKET_ANIMAL_HELD_ITEM, NET_MEMBER_NONE);
    }
}

// ---------------------------------------------------------------------------
// Network messages: a parse function (packet -> values), a build function (values -> packet)
// and a send function for each message type. Index values are villager slots (< 10).

// 800F0020
void parseAnimalHeldItemTimePacket(int *idx, u8 *value, const u8 *src) {
    if (src[1] < ANIMAL_NUM) {
        *idx = src[1];
    } else {
        *idx = -1;
    }
    *value = src[0];
}

// 800F0048
void buildAnimalHeldItemTimePacket(u8 *dst, int idx, int value) {
    dst[0] = value;
    dst[1] = idx;
}

// 800F0054
void sendAnimalHeldItemTime(u32 idx, u8 value) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1 && idx < ANIMAL_NUM) {
        u8 buf[2];
        buildAnimalHeldItemTimePacket(buf, idx, value);
        dNet::beginPacket();
        dNet::addPacket(buf, 2);
        dNet::sendPacket(NET_PACKET_ANIMAL_HELD_ITEM_TIME, NET_MEMBER_NONE);
    }
}

// 800F00D4
void parseAnimalEventFlagsPacket(int *idx, u8 *value, const u8 *src) {
    if (src[1] < ANIMAL_NUM) {
        *idx = src[1];
    } else {
        *idx = -1;
    }
    *value = src[0];
}

// 800F00FC
void buildAnimalEventFlagsPacket(u8 *dst, int idx, int value) {
    dst[1] = idx;
    dst[0] = value;
}

// 800F0108
void sendAnimalEventFlags(u32 idx, u8 value) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1 && idx < ANIMAL_NUM) {
        u8 buf[2];
        buildAnimalEventFlagsPacket(buf, idx, value);
        dNet::beginPacket();
        dNet::addPacket(buf, 2);
        dNet::sendPacket(NET_PACKET_ANIMAL_EVENT_FLAGS, NET_MEMBER_NONE);
    }
}

// 800F0188
void parseAnimalMemoryPacket(int *idx, int *kind, u8 *count, u16 *msg, u8 *flag, const u8 *src) {
    if (src[4] < ANIMAL_NUM) {
        *idx = src[4];
    } else {
        *idx = -1;
    }
    if (src[0] < ANIMAL_MEMORY_NUM) {
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
void buildAnimalMemoryPacket(u8 *dst, int idx, int kind, int count, int msg, int flag) {
    dst[4] = idx;
    dst[0] = kind;
    dst[1] = count;
    dst[3] = msg;
    dst[2] = flag;
}

// 800F0208
void sendAnimalMemoryInit(u32 idx, u32 kind, u8 count, u8 msg, s8 flag) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1 && idx < ANIMAL_NUM && kind < ANIMAL_MEMORY_NUM) {
        u8 buf[5];
        buildAnimalMemoryPacket(buf, idx, kind, count, msg, flag);
        dNet::beginPacket();
        dNet::addPacket(buf, 5);
        dNet::sendPacket(NET_PACKET_ANIMAL_MEMORY_INIT, NET_MEMBER_NONE);
    }
}

// 800F02A8
void sendAnimalMemorySet(u32 idx, u32 kind, u8 count, u8 msg, s8 flag) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1 && idx < ANIMAL_NUM && kind < ANIMAL_MEMORY_NUM) {
        u8 buf[5];
        buildAnimalMemoryPacket(buf, idx, kind, count, msg, flag);
        dNet::beginPacket();
        dNet::addPacket(buf, 5);
        dNet::sendPacket(NET_PACKET_ANIMAL_MEMORY_SET, NET_MEMBER_NONE);
    }
}

// 800F0348
void parseAnimalFriendshipPacket(int *idx, int *kind, u8 *value, const u8 *src) {
    if (src[2] < ANIMAL_NUM) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    if (src[0] < ANIMAL_MEMORY_NUM) {
        *kind = src[0];
    } else {
        *kind = -1;
    }
    *value = src[1];
}

// 800F038C
void buildAnimalFriendshipPacket(u8 *dst, int idx, int kind, int value) {
    dst[2] = idx;
    dst[0] = kind;
    dst[1] = value;
}

// 800F039C
void sendAnimalFriendship(u32 idx, u32 kind, s8 value) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1 && idx < ANIMAL_NUM && kind < ANIMAL_MEMORY_NUM) {
        u8 buf[3];
        buildAnimalFriendshipPacket(buf, idx, kind, value);
        dNet::beginPacket();
        dNet::addPacket(buf, 3);
        dNet::sendPacket(NET_PACKET_ANIMAL_FRIENDSHIP, NET_MEMBER_NONE);
    }
}

// 800F0434
void sendAnimalFriendshipById(const dAnmPersonalID_c *animal, u32 kind, s8 value) {
    sendAnimalFriendship(dSaveData_c::getRaw()->mAnimals.mTown.getAnimalIdx(animal), kind, value);
}

// 800F0494
void parseAnimalMemoryFlagOnPacket(int *idx, int *kind, int *value, const u8 *src) {
    if (src[2] < ANIMAL_NUM) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    if (src[0] < ANIMAL_MEMORY_NUM) {
        *kind = src[0];
    } else {
        *kind = -1;
    }
    *value = src[1];
}

// 800F04D8
void buildAnimalMemoryFlagOnPacket(u8 *dst, int idx, int kind, int value) {
    dst[2] = idx;
    dst[0] = kind;
    dst[1] = value;
}

// 800F04E8
void sendAnimalMemoryFlagOn(u32 idx, u32 kind, u8 value) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1 && idx < ANIMAL_NUM && kind < ANIMAL_MEMORY_NUM) {
        u8 buf[3];
        buildAnimalMemoryFlagOnPacket(buf, idx, kind, value);
        dNet::beginPacket();
        dNet::addPacket(buf, 3);
        dNet::sendPacket(NET_PACKET_ANIMAL_MEMORY_FLAG_ON, NET_MEMBER_NONE);
    }
}

// 800F0580
void parseAnimalEventTalkedPacket(int *idx, int *kind, const u8 *src) {
    if (src[1] < ANIMAL_NUM) {
        *idx = src[1];
    } else {
        *idx = -1;
    }
    if (src[0] < ANIMAL_MEMORY_NUM) {
        *kind = src[0];
    } else {
        *kind = -1;
    }
}

// 800F05BC
void buildAnimalEventTalkedPacket(u8 *dst, int idx, int kind) {
    dst[1] = idx;
    dst[0] = kind;
}

// 800F05C8
void sendAnimalEventTalked(u32 idx, u32 kind) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1 && idx < ANIMAL_NUM && kind < ANIMAL_MEMORY_NUM) {
        u8 buf[2];
        buildAnimalEventTalkedPacket(buf, idx, kind);
        dNet::beginPacket();
        dNet::addPacket(buf, 2);
        dNet::sendPacket(NET_PACKET_ANIMAL_EVENT_TALKED, NET_MEMBER_NONE);
    }
}

// 800F0650
void sendHideAndSeekClear() {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 value = 1;
        dNet::beginPacket();
        dNet::addPacket(&value, 1);
        dNet::sendPacket(NET_PACKET_HIDE_AND_SEEK_CLEAR, NET_MEMBER_NONE);
    }
}

// 800F06A8
void sendHideAndSeekState(u8 value) {
    u8 buf = value;
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1 && buf < 5) {
        dNet::beginPacket();
        dNet::addPacket(&buf, 1);
        dNet::sendPacket(NET_PACKET_HIDE_AND_SEEK_STATE, NET_MEMBER_NONE);
    }
}

// 800F0708
void sendHideAndSeekFlag(int value) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1 && value < 3) {
        u8 buf = value;
        dNet::beginPacket();
        dNet::addPacket(&buf, 1);
        dNet::sendPacket(NET_PACKET_HIDE_AND_SEEK_FLAG, NET_MEMBER_NONE);
    }
}

// 800F0770
void parseAnimalFtrTouchPacket(u8 *kind, int *x, int *z, const u8 *src) {
    u8 value = dScene::SCENE_NUM;
    cLib::memCpy(&value, src, 1);
    if (value < dScene::SCENE_NUM) {
        *kind = value;
    } else {
        *kind = dScene::SCENE_NUM;
    }
    if (src[1] < UT_X_NUM && src[2] < UT_Z_NUM) {
        *x = src[1];
        *z = src[2];
    } else {
        *x = -1;
        *z = -1;
    }
}

// 800F080C
void buildAnimalFtrTouchPacket(u8 *dst, int kind, int x, int z) {
    u8 value = kind;
    cLib::memCpy(dst, &value, 1);
    dst[1] = x;
    dst[2] = z;
}

// 800F0864
void sendAnimalFtrTouch(int kind, int x, int z) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[3];
        buildAnimalFtrTouchPacket(buf, kind, x, z);
        dNet::beginPacket();
        dNet::addPacket(buf, 3);
        dNet::sendPacket(NET_PACKET_ANIMAL_FTR_TOUCH, NET_MEMBER_NONE);
    }
}

// 800F08EC
void parseAnimalFtrPacket(int *idx, void *item, int *count, const u8 *src) {
    if (src[2] < ANIMAL_NUM) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    cLib::memCpy(item, src, 2);
    if (src[3] < ANIMAL_FTR_COUNT) {
        *count = src[3];
    } else {
        *count = ANIMAL_FTR_COUNT;
    }
}

// 800F0968
void buildAnimalFtrPacket(u8 *dst, int idx, const void *item, int count) {
    dst[2] = idx;
    cLib::memCpy(dst, item, 2);
    dst[3] = count;
}

// 800F09B4
void sendAnimalFtr(int idx, const void *item, int count) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[4];
        buildAnimalFtrPacket(buf, idx, item, count);
        dNet::beginPacket();
        dNet::addPacket(buf, 4);
        dNet::sendPacket(NET_PACKET_ANIMAL_FTR, NET_MEMBER_NONE);
    }
}

// 800F0A3C
void parseAnimalPlacePacket(int *idx, int *mode, const u8 *src) {
    if (src[1] < ANIMAL_NUM) {
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
void buildAnimalPlacePacket(u8 *dst, int idx, int mode) {
    dst[1] = idx;
    dst[0] = mode;
}

// 800F0A84
void sendAnimalPlace(int idx, int mode) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[2];
        buildAnimalPlacePacket(buf, idx, mode);
        dNet::beginPacket();
        dNet::addPacket(buf, 2);
        dNet::sendPacket(NET_PACKET_ANIMAL_PLACE, NET_MEMBER_NONE);
    }
}

// 800F0AFC
void parseAnimalSpotPacket(int *idx, int *a, int *b, u32 *c, const u8 *src) {
    if (src[3] < ANIMAL_NUM) {
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
void buildAnimalSpotPacket(u8 *dst, int idx, int a, int b, int c) {
    dst[3] = idx;
    dst[0] = a;
    dst[1] = b;
    dst[2] = c;
}

// 800F0B64
void sendAnimalSpot(int idx, int a, int b, int c) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[4];
        buildAnimalSpotPacket(buf, idx, a, b, c);
        dNet::beginPacket();
        dNet::addPacket(buf, 4);
        dNet::sendPacket(NET_PACKET_ANIMAL_SPOT, NET_MEMBER_NONE);
    }
}

// 800F0BFC
void parseAnimalSpotTimePacket(int *idx, u32 *kind, void *data, const u8 *src) {
    if (src[9] < ANIMAL_NUM) {
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
void buildAnimalSpotTimePacket(u8 *dst, int idx, int kind, const void *data) {
    dst[8] = kind;
    dst[9] = idx;
    cLib::memCpy(dst, data, 8);
}

// 800F0C5C
void sendAnimalSpotTime(int idx, int kind, const void *data) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[10];
        buildAnimalSpotTimePacket(buf, idx, kind, data);
        dNet::beginPacket();
        dNet::addPacket(buf, 10);
        dNet::sendPacket(NET_PACKET_ANIMAL_SPOT_TIME, NET_MEMBER_NONE);
    }
}

// 800F0CE4
void parseAnimalSpotClearPacket(int *idx, const u8 *src) {
    if (src[0] < ANIMAL_NUM) {
        *idx = src[0];
    } else {
        *idx = -1;
    }
}

// 800F0D04
void buildAnimalSpotClearPacket(u8 *dst, int idx) {
    dst[0] = idx;
}

// 800F0D0C
void sendAnimalSpotClear(int idx) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf;
        buildAnimalSpotClearPacket(&buf, idx);
        dNet::beginPacket();
        dNet::addPacket(&buf, 1);
        dNet::sendPacket(NET_PACKET_ANIMAL_SPOT_CLEAR, NET_MEMBER_NONE);
    }
}

// 800F0D74
void parseAnimalFtrSetPacket(int *idx, int *count, void *item, const u8 *src) {
    if (src[3] < ANIMAL_NUM) {
        *idx = src[3];
    } else {
        *idx = -1;
    }
    if (src[2] < ANIMAL_FTR_COUNT) {
        *count = src[2];
    } else {
        *count = ANIMAL_FTR_COUNT;
    }
    cLib::memCpy(item, src, 2);
}

// 800F0DBC
void buildAnimalFtrSetPacket(u8 *dst, int idx, int count, const void *item) {
    dst[2] = count;
    dst[3] = idx;
    cLib::memCpy(dst, item, 2);
}

// 800F0DD4
void sendAnimalFtrSet(int idx, int count, const void *item) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[4];
        buildAnimalFtrSetPacket(buf, idx, count, item);
        dNet::beginPacket();
        dNet::addPacket(buf, 4);
        dNet::sendPacket(NET_PACKET_ANIMAL_FTR_SET, NET_MEMBER_NONE);
    }
}

// 800F0E5C
void parseAnimalNewItemAddPacket(int *idx, void *item, const u8 *src) {
    if (src[2] < ANIMAL_NUM) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    cLib::memCpy(item, src, 2);
}

// 800F0E88
void buildAnimalNewItemAddPacket(u8 *dst, int idx, const void *item) {
    dst[2] = idx;
    cLib::memCpy(dst, item, 2);
}

// 800F0E9C
void sendAnimalNewItemAdd(int idx, const void *item) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[3];
        buildAnimalNewItemAddPacket(buf, idx, item);
        dNet::beginPacket();
        dNet::addPacket(buf, 3);
        dNet::sendPacket(NET_PACKET_ANIMAL_NEW_ITEM_ADD, NET_MEMBER_NONE);
    }
}

// 800F0F14
void parseAnimalNewItemClearPacket(int *idx, const u8 *src) {
    if (src[0] < ANIMAL_NUM) {
        *idx = src[0];
    } else {
        *idx = -1;
    }
}

// 800F0F34
void buildAnimalNewItemClearPacket(u8 *dst, int idx) {
    dst[0] = idx;
}

// 800F0F3C
void sendAnimalNewItemClear(int idx) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf;
        buildAnimalNewItemClearPacket(&buf, idx);
        dNet::beginPacket();
        dNet::addPacket(&buf, 1);
        dNet::sendPacket(NET_PACKET_ANIMAL_NEW_ITEM_CLEAR, NET_MEMBER_NONE);
    }
}

// 800F0FA4
void parseAnimalNewItemRemovePacket(int *idx, void *item, const u8 *src) {
    if (src[2] < ANIMAL_NUM) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    cLib::memCpy(item, src, 2);
}

// 800F0FD0
void buildAnimalNewItemRemovePacket(u8 *dst, int idx, const void *item) {
    dst[2] = idx;
    cLib::memCpy(dst, item, 2);
}

// 800F0FE4
void sendAnimalNewItemRemove(int idx, const void *item) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[3];
        buildAnimalNewItemRemovePacket(buf, idx, item);
        dNet::beginPacket();
        dNet::addPacket(buf, 3);
        dNet::sendPacket(NET_PACKET_ANIMAL_NEW_ITEM_REMOVE, NET_MEMBER_NONE);
    }
}

// 800F105C
void parseAnimalWallPacket(int *idx, void *item, const u8 *src) {
    if (src[2] < ANIMAL_NUM) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    cLib::memCpy(item, src, 2);
}

// 800F1088
void buildAnimalWallPacket(u8 *dst, int idx, const void *item) {
    dst[2] = idx;
    cLib::memCpy(dst, item, 2);
}

// 800F109C
void sendAnimalWall(int idx, const void *item) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[3];
        buildAnimalWallPacket(buf, idx, item);
        dNet::beginPacket();
        dNet::addPacket(buf, 3);
        dNet::sendPacket(NET_PACKET_ANIMAL_WALL, NET_MEMBER_NONE);
    }
}

// 800F1114
void parseAnimalCarpetPacket(int *idx, void *item, const u8 *src) {
    if (src[2] < ANIMAL_NUM) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    cLib::memCpy(item, src, 2);
}

// 800F1140
void buildAnimalCarpetPacket(u8 *dst, int idx, const void *item) {
    dst[2] = idx;
    cLib::memCpy(dst, item, 2);
}

// 800F1154
void sendAnimalCarpet(int idx, const void *item) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[3];
        buildAnimalCarpetPacket(buf, idx, item);
        dNet::beginPacket();
        dNet::addPacket(buf, 3);
        dNet::sendPacket(NET_PACKET_ANIMAL_CARPET, NET_MEMBER_NONE);
    }
}

// 800F11CC
void parseAnimalMusicPacket(int *idx, void *item, const u8 *src) {
    if (src[2] < ANIMAL_NUM) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    cLib::memCpy(item, src, 2);
}

// 800F11F8
void buildAnimalMusicPacket(u8 *dst, int idx, const void *item) {
    dst[2] = idx;
    cLib::memCpy(dst, item, 2);
}

// 800F120C
void sendAnimalMusic(int idx, const void *item) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[3];
        buildAnimalMusicPacket(buf, idx, item);
        dNet::beginPacket();
        dNet::addPacket(buf, 3);
        dNet::sendPacket(NET_PACKET_ANIMAL_MUSIC, NET_MEMBER_NONE);
    }
}

// 800F1284
void parseAnimalQuestStatePacket(int *idx, int *kind, u8 *value, const u8 *src) {
    if (src[2] < ANIMAL_NUM) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    if (src[0] < QUEST_KIND_NUM) {
        *kind = src[0];
    } else {
        *kind = QUEST_KIND_NONE;
    }
    *value = src[1];
}

// 800F12C8
void buildAnimalQuestStatePacket(u8 *dst, int idx, int kind, int value) {
    dst[2] = idx;
    dst[0] = kind;
    dst[1] = value;
}

// 800F12D8
void sendAnimalQuestState(int idx, int kind, int value) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[3];
        buildAnimalQuestStatePacket(buf, idx, kind, value);
        dNet::beginPacket();
        dNet::addPacket(buf, 3);
        dNet::sendPacket(NET_PACKET_ANIMAL_QUEST_STATE, NET_MEMBER_NONE);
    }
}

// 800F1360
void parseAnimalQuestClearPacket(int *idx, const u8 *src) {
    if (src[0] < ANIMAL_NUM) {
        *idx = src[0];
    } else {
        *idx = -1;
    }
}

// 800F1380
void buildAnimalQuestClearPacket(u8 *dst, int idx) {
    dst[0] = idx;
}

// 800F1388
void sendAnimalQuestClear(int idx) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf;
        buildAnimalQuestClearPacket(&buf, idx);
        dNet::beginPacket();
        dNet::addPacket(&buf, 1);
        dNet::sendPacket(NET_PACKET_ANIMAL_QUEST_CLEAR, NET_MEMBER_NONE);
    }
}

// 800F13F0
void parseAnimalPlaceTimeClearPacket(int *idx, const u8 *src) {
    if (src[0] < ANIMAL_NUM) {
        *idx = src[0];
    } else {
        *idx = -1;
    }
}

// 800F1410
void buildAnimalPlaceTimeClearPacket(u8 *dst, int idx) {
    dst[0] = idx;
}

// 800F1418
void sendAnimalPlaceTimeClear(int idx) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf;
        buildAnimalPlaceTimeClearPacket(&buf, idx);
        dNet::beginPacket();
        dNet::addPacket(&buf, 1);
        dNet::sendPacket(NET_PACKET_ANIMAL_PLACE_TIME_CLEAR, NET_MEMBER_NONE);
    }
}

// 800F1480
void parseAnimalPresentPacket(int *idx, int *kind, void *item, const u8 *src) {
    if (src[3] < ANIMAL_NUM) {
        *idx = src[3];
    } else {
        *idx = -1;
    }
    cLib::memCpy(item, src, 2);
    if (src[2] < ANIMAL_MEMORY_NUM) {
        *kind = src[2];
    } else {
        *kind = -1;
    }
}

// 800F14FC
void buildAnimalPresentPacket(u8 *dst, int idx, int kind, const void *item) {
    dst[2] = kind;
    dst[3] = idx;
    cLib::memCpy(dst, item, 2);
}

// 800F1514
void sendAnimalPresent(int idx, int kind, const void *item) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[4];
        buildAnimalPresentPacket(buf, idx, kind, item);
        dNet::beginPacket();
        dNet::addPacket(buf, 4);
        dNet::sendPacket(NET_PACKET_ANIMAL_PRESENT, NET_MEMBER_NONE);
    }
}

// 800F159C
void parsePermitStatePacket(void *item, int *mode, const u8 *src) {
    cLib::memCpy(item, src, 2);
    if (src[2] < NET_MEMBER_MAX) {
        *mode = src[2];
    } else {
        *mode = NET_MEMBER_NONE;
    }
}

// 800F15F8
void buildPermitStatePacket(u8 *dst, const void *item, int mode) {
    cLib::memCpy(dst, item, 2);
    dst[2] = mode;
}

// 800F1638
void sendPermitState(const void *item, int mode) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[3];
        buildPermitStatePacket(buf, item, mode);
        dNet::beginPacket();
        dNet::addPacket(buf, 3);
        dNet::sendPacket(NET_PACKET_PERMIT_STATE, NET_MEMBER_NONE);
    }
}

// 800F16B0
void sendPermitSetState(const void *item, int mode) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[3];
        buildPermitStatePacket(buf, item, mode);
        dNet::beginPacket();
        dNet::addPacket(buf, 3);
        dNet::sendPacket(NET_PACKET_PERMIT_SET_STATE, NET_MEMBER_NONE);
    }
}

// 800F1728
void parsePermitKeyPacket(void *item, const void *src) {
    cLib::memCpy(item, src, 2);
}

// 800F1730
void buildPermitKeyPacket(void *dst, const void *item) {
    cLib::memCpy(dst, item, 2);
}

// 800F1738: sends to one other connected player.
void sendPermitFlag(const void *item, u32 player) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1 && player < NET_MEMBER_MAX && (int)player != dNet::getMyNo() && dNet::isMember(player)) {
        u8 buf[2];
        buildPermitKeyPacket(buf, item);
        dNet::beginPacket();
        dNet::addPacket(buf, 2);
        dNet::sendPacket(NET_PACKET_PERMIT_FLAG, player);
    }
}

// 800F17D0
void sendPermitQuery(const void *item) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[2];
        buildPermitKeyPacket(buf, item);
        dNet::beginPacket();
        dNet::addPacket(buf, 2);
        dNet::sendPacket(NET_PACKET_PERMIT_QUERY, NET_MEMBER_NONE);
    }
}

// 800F1838
void sendPermitAnswer(const void *item, u32 player) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1 && player < NET_MEMBER_MAX && (int)player != dNet::getMyNo() && dNet::isMember(player)) {
        u8 buf[2];
        buildPermitKeyPacket(buf, item);
        dNet::beginPacket();
        dNet::addPacket(buf, 2);
        dNet::sendPacket(NET_PACKET_PERMIT_ANSWER, player);
    }
}

// 800F18D0
void sendPermitClearState(const void *item, int mode) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[3];
        buildPermitStatePacket(buf, item, mode);
        dNet::beginPacket();
        dNet::addPacket(buf, 3);
        dNet::sendPacket(NET_PACKET_PERMIT_CLEAR_STATE, NET_MEMBER_NONE);
    }
}

// 800F1948
void sendPermitFallbackPlayer(const void *item, int mode) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[3];
        buildPermitStatePacket(buf, item, mode);
        dNet::beginPacket();
        dNet::addPacket(buf, 3);
        dNet::sendPacket(NET_PACKET_PERMIT_SET2, NET_MEMBER_NONE);
    }
}

// 800F19C0
void sendPermitClear(const void *item) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[2];
        buildPermitKeyPacket(buf, item);
        dNet::beginPacket();
        dNet::addPacket(buf, 2);
        dNet::sendPacket(NET_PACKET_PERMIT_CLEAR, NET_MEMBER_NONE);
    }
}

// 800F1A28
void parseAnimalClothPacket(int *idx, void *item, const u8 *src) {
    if (src[2] < ANIMAL_NUM) {
        *idx = src[2];
    } else {
        *idx = -1;
    }
    cLib::memCpy(item, src, 2);
}

// 800F1A54
void buildAnimalClothPacket(u8 *dst, int idx, const void *item) {
    dst[2] = idx;
    cLib::memCpy(dst, item, 2);
}

// 800F1A68
void sendAnimalCloth(int idx, const void *item) {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        u8 buf[3];
        buildAnimalClothPacket(buf, idx, item);
        dNet::beginPacket();
        dNet::addPacket(buf, 3);
        dNet::sendPacket(NET_PACKET_ANIMAL_CLOTH, NET_MEMBER_NONE);
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

static inline BOOL isPlacementFurniture(u16 id) {
    int category = (id >> 12) & 0xF;
    BOOL result = FALSE;
    if (category >= 9 && category <= 12) {
        result = TRUE;
    }
    return result;
}

static inline int getFtrFunc(const dItem::BITM *bitm) {
    int value = bitm->m_ftrFunc;
    int func = 1;
    if (static_cast<u32>(value) < 0x41) {
        func = value;
    }
    return func;
}

// 800F1AE0: the player standing on unit (x, z).
dAcPlayer_c *getPlayerOnUnit(int x, int z) {
    for (int i = 0; i < PLAYER_NUM; i++) {
        dAcPlayer_c *player = dPlayerMgr::getPlayerActor(i);
        if (player != NULL) {
            f32 fx = player->mPos.x;
            f32 fz = player->mPos.z;
            int px = (int)fx >> 5;
            int pz = (int)fz >> 5;
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
    actor = dNpcList::getNmlAtUnit(x, z);
    if (actor != NULL) {
        return actor;
    }
    return dNpcList::getSpAtUnit(x, z);
}

// 800F1BE4: the furniture whose footprint covers unit (x, z).
dItem::Item getFurnitureCoveringUnit(int *outX, int *outZ, int x, int z, dFdBase_c *map, u8 kind) {
    if (kind == dScene::SCENE_NUM) {
        kind = dScene::getCurrentScene();
    }
    if (!dScene::isSceneAttr(kind, dScene::SCENE_ATTR_ROOM)) {
        return dItem::Item();
    }
    if (map == NULL) {
        map = dField::getInfo(FD_ID_CURRENT);
    }
    if (map == NULL) {
        return dItem::Item();
    }
    if (x <= 0 || x >= UT_X_NUM - 1 || z <= 0 || z >= UT_Z_NUM - 1) {
        return dItem::Item();
    }

    dItem::Item *item;
    int dummyX = 0;
    int dummyZ = 0;
    if (outX == NULL) {
        outX = &dummyX;
    }
    if (outZ == NULL) {
        outZ = &dummyZ;
    }
    int x0 = x - 1;
    int x1 = x + 1;
    int z1 = z + 1;
    for (int zz = z - 1; zz <= z1; zz++) {
        for (int xx = x0; xx <= x1; xx++) {
            item = map->getItem(xx, zz, 0);
            if (item != NULL && item->getId() != dItem::ITEM_ID_NONE) {
                const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
                if (bitm != NULL && getFtrFunc(bitm) != dItem::FTR_FUNC_NONE) {
                    dFTR::shape_c shape;
                    shape.set(*item);
                    for (u32 i = 0; i < shape.getNum(); i++) {
                        const int *ofs = shape.getOfs(i);
                        u32 unitX = xx + ofs[0];
                        u32 unitZ = zz + ofs[1];
                        if (x == unitX && z == unitZ) {
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
        map = dField::getInfo(FD_ID_CURRENT);
    }
    if (kind == dScene::SCENE_NUM) {
        kind = dScene::getCurrentScene();
    }

    if (map != NULL && ((checkA && map->canNpcPutItem(x, z)) || (!checkA && map->canPutItem(x, z)))) {
        dItem::Item *cur = map->getItem(x, z, 0);
        if (cur != NULL) {
            if (cur->mId == dItem::ITEM_ID_NONE ||
                (isPlacementFurniture(cur->mId) &&
                 (dScene::isSceneAttr(kind, dScene::SCENE_ATTR_TOWN) || !cur->hasFtrFunc())) ||
                (cur->mId < dItem::FG_COUNT &&
                 ((cur->getFgInfo() != NULL && cur->getFgInfo()->mTreeStage == 0) ||
                  (!allowFg94 && isIdInRange(cur->mId, dItem::FG_HOLE, dItem::FG_HOLE)) || cur->isFg74() ||
                  isIdInRange(cur->mId, dItem::FG_DEAD_SAPLING, dItem::FG_DEAD_PALM_SAPLING) ||
                  isIdInRange(cur->mId, dItem::FG_WEED_A, dItem::FG_WEED_D) ||
                  isIdInRange(cur->mId, dItem::FG_CLOVER, dItem::FG_LUCKY_CLOVER) || cur->isAnyFlower() ||
                  isIdInRange(cur->mId, dItem::FG_RED_TURNIP_WILTED, dItem::FG_RED_TURNIP_WILTED_WATERED)))) {
                dActor_c *actor = getActorOnUnit(x, z);
                if (actor == NULL || (item->mId != dItem::ITEM_ID_NONE && item->mId == actor->mParam)) {
                    result = TRUE;
                    if (dScene::isSceneAttr(kind, dScene::SCENE_ATTR_ROOM)) {
                        dItem::Item ftr = getFurnitureCoveringUnit(NULL, NULL, x, z, NULL, dScene::SCENE_NUM);
                        if (ftr.getId() != dItem::ITEM_ID_NONE) {
                            const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(ftr);
                            if (bitm != NULL && getFtrFunc(bitm) != dItem::FTR_FUNC_NONE) {
                                result = FALSE;
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}

// 800F20D0
BOOL canPutItemAt(const mVec3_c *pos, const dItem::Item &item, dFdBase_c *map,
                  u8 kind, BOOL allowFg94, BOOL checkA) {
    f32 x = pos->x;
    f32 z = pos->z;
    return canPutItemOnUnit((int)x >> 5, (int)z >> 5, &item, map, kind,
                            allowFg94, checkA);
}

// 800F2138: a random free unit closest to `pos`, searching up to `radius` rings out.
BOOL pickNearbyFreePos(mVec3_c *out, const mVec3_c *pos, u32 radius) {
    int cx;
    int cz;
    u32 maxX;
    u32 maxZ;
    u32 minX;
    dFdBase_c *map = dField::getInfo(FD_ID_CURRENT);
    if (map == NULL) {
        return FALSE;
    }

    f32 px = pos->x;
    f32 pz = pos->z;
    cx = (int)px >> 5;
    cz = (int)pz >> 5;
    int bestX = -1;
    int bestZ = -1;
    maxX = cx;
    maxZ = cz;
    for (u32 r = 0; r <= radius; maxX++, maxZ++, r++) {
        u32 num = 0;
        u32 z = cz - r;
        minX = cx - r;
        for (; z <= maxZ; z++) {
            for (u32 x = minX; x <= maxX; x++) {
                dItem::Item none;
                bool free = canPutItemOnUnit(x, z, &none, map, dScene::SCENE_NUM, TRUE, TRUE);
                if (free) {
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
BOOL isUnitWithinDistance(const mVec3_c *pos, int x, int z, f32 dist) {
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
BOOL pickFreeUnitInRect(int *outX, int *outZ, dFdBase_c *map, u8 kind, int x0, int x1, int z0, int z1,
                 spotFunc func, int arg, const mVec3_c *exclude, f32 dist) {
    if (outX == NULL || outZ == NULL) {
        return FALSE;
    }
    if (map == NULL) {
        return FALSE;
    }

    BOOL ok;
    BOOL free;
    u32 num = 0;
    for (int z = z0; z < z1; z++) {
        for (int x = x0; x < x1; x++) {
            ok = FALSE;
            free = FALSE;
            if (!isUnitWithinDistance(exclude, x, z, dist)) {
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

// 800F25A4: pickFreeUnitInRect over the outdoor map, keeping a one-block border.
BOOL pickOutdoorFreeUnit(int *outX, int *outZ, spotFunc func, int arg, const mVec3_c *exclude, f32 dist) {
    dFdBase_c *map = dField::getInfo(FD_ID_TOWN);
    if (map == NULL) {
        return FALSE;
    }
    return pickFreeUnitInRect(outX, outZ, map, dScene::SCENE_FIELD, UT_X_NUM, map->mUnitW - UT_X_NUM, UT_Z_NUM, map->mUnitH - UT_Z_NUM, func, arg, exclude,
                       dist);
}

// 800F2644
BOOL pickOutdoorFreePos(mVec3_c *out, spotFunc func, int arg, const mVec3_c *exclude, f32 dist) {
    if (out == NULL) {
        return FALSE;
    }
    int x = -1;
    int z = -1;
    if (pickOutdoorFreeUnit(&x, &z, func, arg, exclude, dist)) {
        dFdBase_c::getUnitCenterPos(out, x, z);
        return TRUE;
    }
    return FALSE;
}

// 800F26C8: a random free unit inside a block with two free units north of it.
BOOL pickUnitWithNorthClearance(int *outX, int *outZ) {
    if (outX == NULL || outZ == NULL) {
        return FALSE;
    }
    dFdBase_c *map = dField::getInfo(FD_ID_TOWN);
    if (map == NULL) {
        return FALSE;
    }

    int blockW = map->mBlockW;
    int blockH = map->mBlockH;
    int z0;
    int x0;
    u32 num = 0;
    dSaveBuildingList_c *fg = dSaveBuildingList_c::get();
    int bz;
    for (bz = 2, z0 = 0x20; bz < blockH - 1; z0 += UT_Z_NUM, bz++) {
        int bx;
        for (bx = 2, x0 = 0x20; bx < blockW - 2; x0 += UT_X_NUM, bx++) {
            for (int j = 4; j < 12; j++) {
                int i;
                int x;
                int z = z0 + j;
                for (i = 4; i < 12; i++) {
                    x = x0 + i;
                    bool free = canPlaceEmptyItemOnUnit(x, z, map);
                    if (free) {
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
BOOL pickPosWithNorthClearance(mVec3_c *out) {
    if (out == NULL) {
        return FALSE;
    }
    int x = 0;
    int z = 0;
    if (pickUnitWithNorthClearance(&x, &z)) {
        dFdBase_c::getUnitCenterPos(out, x, z);
        return TRUE;
    }
    return FALSE;
}

// 800F2920: pickFreeUnitInRect over the current scene's whole map.
BOOL pickSceneFreeUnit(int *outX, int *outZ, spotFunc func, int arg, const mVec3_c *exclude, f32 dist) {
    dFdBase_c *map = dField::getInfo(FD_ID_CURRENT);
    if (map == NULL) {
        return FALSE;
    }
    u8 kind = dScene::getCurrentScene();
    return pickFreeUnitInRect(outX, outZ, map, kind, 0, map->mUnitW, 0, map->mUnitH, func, arg, exclude, dist);
}

// 800F29C4
BOOL pickSceneFreePos(mVec3_c *out, spotFunc func, int arg, const mVec3_c *exclude, f32 dist) {
    if (out == NULL) {
        return FALSE;
    }
    int x = -1;
    int z = -1;
    if (pickSceneFreeUnit(&x, &z, func, arg, exclude, dist)) {
        dFdBase_c::getUnitCenterPos(out, x, z);
        return TRUE;
    }
    return FALSE;
}

// 800F2A48
BOOL hasEntranceSideSpots(const dItem::Item *item) {
    static dItem::Item sEntranceSideItems[] = {
        BUILDING_PLAYER_HOUSE_0, BUILDING_PLAYER_HOUSE_1, BUILDING_PLAYER_HOUSE_2,
        BUILDING_PLAYER_HOUSE_3, BUILDING_NPC_HOUSE_0, BUILDING_NPC_HOUSE_1,
        BUILDING_NPC_HOUSE_2, BUILDING_NPC_HOUSE_3, BUILDING_NPC_HOUSE_4,
        BUILDING_NPC_HOUSE_5, BUILDING_NPC_HOUSE_6, BUILDING_NPC_HOUSE_7,
        BUILDING_NPC_HOUSE_8, BUILDING_NPC_HOUSE_9, BUILDING_TOWN_HALL,
        BUILDING_SHOP, BUILDING_TAILOR, BUILDING_MUSEUM,
    };
    static u32 sEntranceSideItemCount = ARRAY_SIZE(sEntranceSideItems);

    dItem::Item *p = sEntranceSideItems;
    dItem::Item *end = p + sEntranceSideItemCount;
    for (; p != end; p++) {
        if (item->isSame(*p)) {
            return TRUE;
        }
    }
    return FALSE;
}

// Delay constant folding to preserve the placement call's argument-load order.
static inline int placementArgument(int value) {
    return *static_cast<const int *>(&value);
}

// 800F2C94: adds the free units of block (bx, bz) next to signs and walls to the pick.
u32 pickBuildingSideUnitInBlock(int *outX, int *outZ, u32 num, u32 bx, u32 bz, dFdBase_c *map, const mVec3_c *exclude,
                BOOL allowFg94, f32 dist) {
    if (map == NULL) {
        return num;
    }
    if (bx >= map->mBlockW || bz >= map->mBlockH) {
        return num;
    }

    BOOL ok;
    dSaveBuildingList_c *fg = dSaveBuildingList_c::get();
    int x0 = bx << 4;
    int z0 = bz << 4;
    int x1 = x0 + UT_X_NUM;
    int z1 = z0 + UT_Z_NUM;
    for (int z = z0; z < z1; z++) {
        for (int x = x0; x < x1; x++) {
            if (isUnitWithinDistance(exclude, x, z, dist)) {
                continue;
            }
            dItem::Item item = fg->getAt(x, z, 1);
            if (item.mId != dItem::ITEM_ID_NONE) {
                if (hasEntranceSideSpots(&item)) {
                    for (int i = 0; i < 2; i++) {
                        mVec3_c pos = mVec3_c::Zero;
                        if (dSTR::getEntranceSidePos(&pos, item, i)) {
                            f32 px = pos.x;
                            f32 pz = pos.z;
                            f32 chance = 100.0f / (num + 1);
                            int ix = px;
                            int iz = pz;
                            dItem::Item none;
                            bool canPut = canPutItemOnUnit(ix >> 5, iz >> 5, &none, map, dScene::SCENE_FIELD, allowFg94, TRUE);
                            if (canPut) {
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
                ok = FALSE;
                if (fg->isBuildSite(x, z)) {
                    dItem::Item none;
                    if (canPutItemOnUnit(x, z - 1, &none, map, placementArgument(dScene::SCENE_FIELD), allowFg94, placementArgument(TRUE))) {
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
u32 pickTreeSideUnitInBlock(int *outX, int *outZ, u32 num, u32 bx, u32 bz, dFdBase_c *map, const mVec3_c *exclude,
                BOOL allowFg94, f32 dist) {
    if (map == NULL) {
        return num;
    }
    if (bx >= map->mBlockW || bz >= map->mBlockH) {
        return num;
    }

    BOOL ok;
    int x0 = bx << 4;
    int z0 = bz << 4;
    int x1 = x0 + UT_X_NUM;
    int z1 = z0 + UT_Z_NUM;
    for (int z = z0; z < z1; z++) {
        for (int x = x0; x < x1; x++) {
            if (isUnitWithinDistance(exclude, x, z, dist)) {
                continue;
            }
            dItem::Item *item = map->getItem(x, z, 0);
            if (item == NULL || item->mId == dItem::ITEM_ID_NONE) {
                continue;
            }
            dItem::FgInfo *info = item->getFgInfo();
            ok = FALSE;
            BOOL tree = FALSE;
            if (info != NULL && info->mTreeStage >= 4) {
                tree = TRUE;
            }
            if (tree) {
                dItem::Item none;
                if (canPutItemOnUnit(x, z - 1, &none, map, placementArgument(dScene::SCENE_FIELD), allowFg94, placementArgument(TRUE))) {
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
BOOL pickOutdoorPlacementUnit(int *outX, int *outZ, spotFunc func, int arg, const mVec3_c *exclude, u32 type,
                 BOOL skipTrees, BOOL allowFg94, f32 dist) {
    dFdBase_c *map = dField::getInfo(FD_ID_TOWN);
    if (map == NULL) {
        return FALSE;
    }

    u8 used[7];
    memset(used, 0, sizeof(used));
    if ((int)type == BLOCK_KIND_FLAG_TOWN_HALL) {
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
    u8 *row = used;
    for (int z = 0; z < 5; z++, row++) {
        for (int x = 0; x < 5; x++) {
            if (((*row) >> x) & 1) {
                continue;
            }
            u32 next = pickBuildingSideUnitInBlock(outX, outZ, num, x + 1, z + 1, map, exclude, allowFg94, dist);
            BOOL busy = FALSE;
            if (next != num || map->hasBlockFlag(x + 1, z + 1, BLOCK_KIND_FLAG_FALL | BLOCK_KIND_FLAG_RACCO)) {
                busy = TRUE;
            } else {
                const dFdBlock_c *block = ((const dFdBase_c *)map)->getBlock(x + 1, z + 1);
                if (block != NULL && dRF::isPondType(block->mType)) {
                    busy = TRUE;
                }
            }
            num = next;
            if (!skipTrees && busy) {
                num = pickTreeSideUnitInBlock(outX, outZ, num, x + 1, z + 1, map, exclude, allowFg94, dist);
                *row |= 1 << x;
            }
        }
    }

    if (num == 0) {
        for (int x = 0; x < 5; x++) {
            if (!((used[4] >> x) & 1)) {
                num = pickTreeSideUnitInBlock(outX, outZ, num, x + 1, 5, map, exclude, allowFg94, dist);
                used[4] |= 1 << x;
            }
        }

        int fx = 0;
        int fz = 0;
        dItem::Item item((u16)BUILDING_GATE);
        bool found = dSaveBuildingList_c::get()->getPos(&fx, &fz, item, 1);
        if (found) {
            u32 bx = (fx >> 4) - 1;
            u32 bz = (fz >> 4) - 1;
            if (bx < 5 && bz < 5) {
                for (int x = 0; x < 5; x++) {
                    if (x != (int)bx && !((used[0] >> x) & 1)) {
                        num = pickTreeSideUnitInBlock(outX, outZ, num, x + 1, 1, map, exclude, allowFg94, dist);
                        used[0] |= 1 << x;
                    }
                }
                for (int z = 1; z < 4; z++) {
                    if (!((used[z] >> bx) & 1)) {
                        num = pickTreeSideUnitInBlock(outX, outZ, num, bx + 1, z + 1, map, exclude, allowFg94, dist);
                        used[z] |= 1 << bx;
                    }
                }
            }
        }

        for (int z = 1; z < 4; z++) {
            if (!(used[z] & 1)) {
                num = pickTreeSideUnitInBlock(outX, outZ, num, 1, z + 1, map, exclude, allowFg94, dist);
                used[z] |= 1;
            }
            if (!((used[z] >> 4) & 1)) {
                num = pickTreeSideUnitInBlock(outX, outZ, num, 5, z + 1, map, exclude, allowFg94, dist);
                used[z] |= 0x10;
            }
        }
    }

    if (num == 0) {
        for (int z = 0; z < 5; z++) {
            for (int x = 0; x < 5; x++) {
                if (!((used[z] >> x) & 1)) {
                    num = pickTreeSideUnitInBlock(outX, outZ, num, x + 1, z + 1, map, exclude, allowFg94, dist);
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
    return isIdInRange(item->mId, dItem::FG_HOLE, dItem::FG_HOLE);
}

// 800F3830: pickOutdoorPlacementUnit as a position; clears any 0x94 object on the spot and steps
// aside from a tree to the south.
BOOL pickOutdoorPlacementPos(mVec3_c *out, spotFunc func, int arg, const mVec3_c *exclude, u32 type, BOOL skipTrees,
                 BOOL allowFg94, f32 dist) {
    if (out == NULL) {
        return FALSE;
    }
    int x = -1;
    int z = -1;
    if (pickOutdoorPlacementUnit(&x, &z, func, arg, exclude, type, skipTrees, allowFg94, dist)) {
        dFdBase_c::getUnitCenterPos(out, x, z);
        dFdBase_c *map = dField::getInfo(FD_ID_TOWN);
        dItem::Item *item = map->getItem(x, z, 0);
        if (!dAcNpc_c::isMultiPlay() && item != NULL && isFg94(item)) {
            dItem::Item none;
            map->setItem(&none, x, z, 0);
        }

        dItem::Item *south = dField::getInfo(FD_ID_TOWN)->getItem(x, z + 1, 0);
        if (south != NULL && south->mId != dItem::ITEM_ID_NONE) {
            dItem::FgInfo *info = south->getFgInfo();
            if (info != NULL && info->mTreeStage >= 4) {
                mVec3_c leftHalf;
                mVec3_c rightHalf;
                mVec3_c left = *out;
                const f32 &step = mFI_UNIT_BASE_SIZE_F;
                const f32 &halfStep = 14.4f;
                leftHalf = left;
                const f32 &leftX = left.x - halfStep;
                leftHalf.x = leftX;
                rightHalf = left;
                const f32 &rightX = left.x + halfStep;
                rightHalf.x = rightX;
                left.x -= step;
                mVec3_c right = *out;
                right.x += step;
                dItem::Item none;
                if (canPutItemAt(&left, none, NULL, dScene::SCENE_NUM, TRUE,
                                 TRUE)) {
                    *out = leftHalf;
                    BOOL useRight = FALSE;
                    dItem::Item none2;
                    if (canPutItemAt(&right, none2, NULL, dScene::SCENE_NUM,
                                     TRUE, TRUE) &&
                        cM::rndInt(2) == 0) {
                        useRight = TRUE;
                    }
                    if (useRight) {
                        *out = rightHalf;
                    }
                } else {
                    dItem::Item none2;
                    if (canPutItemAt(&right, none2, NULL, dScene::SCENE_NUM,
                                     TRUE, TRUE)) {
                        *out = rightHalf;
                    } else if (!dAcNpc_c::isMultiPlay()) {
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
        }
        return TRUE;
    }
    return FALSE;
}

// 800F3C0C: whether unit (x, z) faces something to its south (tree, wall, or blocked unit).
BOOL isUnitBlockedToSouth(int x, int z) {
    dFdBase_c *map = dField::getInfo(FD_ID_TOWN);
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
BOOL isPosBlockedToSouth(const mVec3_c *pos) {
    f32 x = pos->x;
    f32 z = pos->z;
    return isUnitBlockedToSouth((int)x >> 5, (int)z >> 5);
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
            return dString::loadString(word, dNpcInfo::getSpNameStr(key), "sys_STRING/STR_SPNpc_name");
    }
    return result;
}

// 800F3E24
void initEntries() {
    list_c *list = &sTownEntries;
    list->init();
}

// 800F3E38
dTime_c *getEntryTime() {
    return &sTownEntries.mTime;
}

// 800F3E48
void setEntryTimeNow() {
    sTownEntries.setTimeNow();
}

// 800F3E54
entry_c *getEntry(u32 i) {
    return sTownEntries.get(i);
}

// 800F3E64: the entry of the villager with key `key`.
entry_c *getEntryByKey(const dItem::Item *key) {
    u16 value = key->mId;
    if (!isNormalKey(key->mId)) {
        return NULL;
    }
    return sTownEntries.get(value & 0xFFF);
}

// 800F3EE8: the entry of a town villager.
entry_c *getEntryByAnimal(const dAnmPersonalID_c *animal) {
    dItem::Item key = dSaveData_c::getTown()->mAnimals.getAnimalKey(animal);
    return getEntryByKey(&key);
}

// 800F3F30
u32 getAnimalCompatibilityScore(dAnimal_c *a, dAnimal_c *b) {
    return calcAnimalCompatibilityScore(a, b);
}

// 800F3F34
int getAnimalCompatibilityRank(dAnimal_c *a, dAnimal_c *b) {
    return calcAnimalCompatibilityRank(a, b);
}

// 800F3F38: the STR_Unit message for a player of gender `gender` talking to a villager
// of personality `looks`.
int getPlayerUnitMessage(u8 gender, u8 looks) {
    int msg = 0;
    switch (looks) {
        case LOOKS_TYPE_SNOOTY:
            if (gender == GENDER_FEMALE) {
                msg = 9;
            }
            break;
        case LOOKS_TYPE_LAZY:
        case LOOKS_TYPE_PEPPY:
            msg = gender == GENDER_MALE ? 8 : 9;
            break;
    }
    return msg;
}

// 800F3F84
int getPlayerUnitMessageForAnimal(const dPersonalID_c *pid, u8 looks, dAnimal_c *animal) {
    if (animal != NULL && animal->usesNickname(pid)) {
        switch (looks) {
            case LOOKS_TYPE_PEPPY:
            case LOOKS_TYPE_SNOOTY:
                return 0;
        }
    }
    return getPlayerUnitMessage(pid->player.mGender, looks);
}

// 800F3FFC
int getPersonalityVoiceType(u8 idx) {
    static const int sVoiceTypes[LOOKS_TYPE_NUM] = {0, 0, 2, 1, 1, 1};

    int result = 0;
    if (idx < LOOKS_TYPE_NUM) {
        result = sVoiceTypes[idx];
    }
    return result;
}

static inline BOOL isSameOrBefore(const dTime_c &a, const dTime_c &b) { return dTime_c::isSameOrAfter(b, a); }

// 800F4020: whether `now` is inside the sleep hours of `idx`.
BOOL isAnimalSleepTime(u8 idx, const dTime_c *now) {
    if (idx >= LOOKS_TYPE_NUM) {
        return FALSE;
    }
    if (now == NULL) {
        now = dTime_c::getCurrent();
    }

    const hours_s *hours = &sSleepHours[idx];
    dTime_c start;
    start.set(now->year, now->month, now->mday, hours->mStartHour, hours->mStartMin, 0);
    start.normalize();
    dTime_c end;
    end.set(now->year, now->month, now->mday, hours->mEndHour, hours->mEndMin, 0);
    end.normalize();

    BOOL result = FALSE;
    if (!dTime_c::isSameOrAfter(*now, end)) {
        if (isSameOrBefore(start, *now)) {
            result = TRUE;
        }
    }
    return result;
}

// 800F4250: whether `now` is past the wake time of `idx`.
BOOL isPastAnimalWakeTime(u8 idx, const dTime_c *now) {
    if (idx >= LOOKS_TYPE_NUM) {
        return FALSE;
    }
    if (now == NULL) {
        now = dTime_c::getCurrent();
    }

    const hours_s *hours = &sSleepHours[idx];
    if (now->hour > hours->mEndHour) {
        return TRUE;
    }
    if (now->hour == hours->mEndHour && now->min >= hours->mEndMin) {
        return TRUE;
    }
    return FALSE;
}

// 800F42E0: whether `now` is inside the daytime hours of `idx`.
BOOL isAnimalDaytime(u8 idx, const dTime_c *now) {
    if (idx >= LOOKS_TYPE_NUM) {
        return FALSE;
    }
    if (now == NULL) {
        now = dTime_c::getCurrent();
    }

    const hours_s *hours = &sDaytimeHours[idx];
    dTime_c start;
    start.set(now->year, now->month, now->mday, hours->mStartHour, hours->mStartMin, 0);
    start.normalize();
    dTime_c end;
    end.set(now->year, now->month, now->mday, hours->mEndHour, hours->mEndMin, 0);
    end.normalize();

    BOOL result = FALSE;
    if (!dTime_c::isSameOrAfter(*now, end)) {
        if (isSameOrBefore(start, *now)) {
            result = TRUE;
        }
    }
    return result;
}

struct itemRange_c {
    itemRange_c(int a, int b) : mKind(a), mFrom(b) {}

    /* 0x0 */ int mKind;
    /* 0x4 */ int mFrom;
};

// 800F4510
int selectPresentItems(dItem::Item *out, int num, const void *filter, const dItem::Item *exclude, int excludeNum) {
    static itemRange_c sItemRanges[4] = {
        itemRange_c(dItem::KIND_CLOTH, dItem::FROM_GROUP_ABC),
        itemRange_c(dItem::KIND_FTR, dItem::FROM_GROUP_ABC),
        itemRange_c(dItem::KIND_CARPET, dItem::FROM_GROUP_ABC),
        itemRange_c(dItem::KIND_WALL, dItem::FROM_GROUP_ABC),
    };
    return dItemSel::select(out, num, reinterpret_cast<const dItemSel::Range_c *>(sItemRanges), 4, reinterpret_cast<const dItemSel::Filter_c *>(filter), exclude, excludeNum, 0);
}

// 800F45A4
int selectRoomItems(dItem::Item *out, int num, const void *filter, const dItem::Item *exclude, int excludeNum) {
    static itemRange_c sItemRanges[3] = {
        itemRange_c(dItem::KIND_FTR, dItem::FROM_GROUP_ABC),
        itemRange_c(dItem::KIND_CARPET, dItem::FROM_GROUP_ABC),
        itemRange_c(dItem::KIND_WALL, dItem::FROM_GROUP_ABC),
    };
    return dItemSel::select(out, num, reinterpret_cast<const dItemSel::Range_c *>(sItemRanges), 3, reinterpret_cast<const dItemSel::Filter_c *>(filter), exclude, excludeNum, 0);
}

// 800F4608
int selectGroupARoomItems(dItem::Item *out, int num, const void *filter, const dItem::Item *exclude, int excludeNum) {
    static itemRange_c sItemRanges[3] = {
        itemRange_c(dItem::KIND_FTR, dItem::FROM_GROUP_A),
        itemRange_c(dItem::KIND_CARPET, dItem::FROM_GROUP_A),
        itemRange_c(dItem::KIND_WALL, dItem::FROM_GROUP_A),
    };
    return dItemSel::select(out, num, reinterpret_cast<const dItemSel::Range_c *>(sItemRanges), 3, reinterpret_cast<const dItemSel::Filter_c *>(filter), exclude, excludeNum, 0);
}

// 800F4668
int selectGroupCRoomItems(dItem::Item *out, int num, const void *filter, const dItem::Item *exclude, int excludeNum) {
    static itemRange_c sItemRanges[3] = {
        itemRange_c(dItem::KIND_FTR, dItem::FROM_GROUP_C),
        itemRange_c(dItem::KIND_CARPET, dItem::FROM_GROUP_C),
        itemRange_c(dItem::KIND_WALL, dItem::FROM_GROUP_C),
    };
    return dItemSel::select(out, num, reinterpret_cast<const dItemSel::Range_c *>(sItemRanges), 3, reinterpret_cast<const dItemSel::Filter_c *>(filter), exclude, excludeNum, 0);
}

// 800F46CC
void setAnimalNameWord(const dAnmPersonalID_c *animal, int slot) {
    if (animal->isValid()) {
        dLetter::setAnmPersonalName(slot, animal);
    }
}

// 800F4718
void setPlayerNameWord(const dPersonalID_c *pid, int slot) {
    if (pid->isValid()) {
        dLetter::setPersonalName(slot, pid);
    }
}

// 800F4764
void setAnimalLandNameWord(const dAnmPersonalID_c *animal, int slot) {
    dLetter::setLandName(slot, &animal->mLand);
}

// 800F4774
void setItemNameWord(const dItem::Item *item, int slot) {
    if (item->mId != dItem::ITEM_ID_NONE) {
        dLetter::setItemName(slot, *item);
    }
}

// 800F4794
void setPlayerUnitWord(u8 gender, u8 looks, int slot) {
    u16 msg = getPlayerUnitMessage(gender, looks);
    if (msg != 0) {
        dLetter::setStringWord(slot, msg, "sys_STRING/STR_Unit");
    }
}

// ---------------------------------------------------------------------------
// Spare villager buffer access

// 800F47D8
void clearAnimalBuffer() {
    sAnimalBuffer.clear();
}

// 800F47E4
BOOL takeMovedAnimalToBuffer(int arg) {
    return sAnimalBuffer.takeMovedAnimal(arg);
}

// 800F47F4
BOOL storeBufferedMovedAnimal() {
    return sAnimalBuffer.storeMovedAnimal();
}

// 800F4800
void exchangeBufferedMovedAnimal() {
    if (dNet::isSessionActive() && dNet::getMemberNum() == 2) {
        sAnimalBuffer.exchangeMovedAnimal();
    }
}

// 800F4840
BOOL exchangeBufferedAnimalWithMember(const dAnimal_c *src, u32 idx) {
    return sAnimalBuffer.exchangeWithRandomMember(src, idx);
}

// 800F4858
bufferedAnimal_c *getBufferedAnimal() {
    return sAnimalBuffer.getValid(0);
}

// 800F4868
BOOL clearBufferedAnimal() {
    return sAnimalBuffer.clearAt(0);
}

// 800F4878
u32 takeBufferedAnimal(dAnimal_c *dst, u32 idx) {
    return sAnimalBuffer.take(dst, idx);
}

// 800F4890
BOOL setBufferedAnimal(const dAnimal_c *src) {
    return sAnimalBuffer.setFirst(src);
}

// ---------------------------------------------------------------------------
// Villager moves

// 800F48A0
BOOL takeMovedAnimal(dAnimal_c *animal) {
    dSaveTown_c *town = dSaveData_c::getTown();
    if (town->mAnimals.takeMovedAnimal(animal, 0) && animal->mID.isValid()) {
        return TRUE;
    }
    return FALSE;
}

// 800F4904: removes the moved-out villager `id` from the town's second list.
BOOL removeMovedAnimal(const dAnmPersonalID_c *id) {
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
BOOL addMovedAnimal(dAnimal_c *animal) {
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
dAnimal_c *pickMovedAnimal() {
    return dSaveData_c::getTown()->mAnimals.pickMovedAnimal();
}

// 800F4A34
BOOL receiveMovedAnimal(dAnimal_c *animal) {
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
BOOL addRandomMovedAnimal(dAnimal_c **animals) {
    if (animals == NULL) {
        return FALSE;
    }

    dAnimalSave_c *save;
    dAnimal_c **p;
    dAnimal_c *tmp = &sAnimalBuffer.get(0)->mAnimal;
    save = &dSaveData_c::getTown()->mAnimals;
    p = animals;
    u32 mask = 0;
    int num = 0;
    for (u32 i = 0; i < 6; i++, p++) {
        tmp->clear();
        if (*p != NULL) {
            cLib::memCpy(tmp, *p, sizeof(dAnimal_c));
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
BOOL isAnimalOnAllowedSpot(dAnimal_c *animal, int arg) {
    if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)) {
        return FALSE;
    }
    const dAnmPersonalID_c *id = &animal->mID;
    if (!id->isValid()) {
        return FALSE;
    }
    dItem::Item key = dSaveData_c::getRaw()->mAnimals.mTown.getAnimalKey(id);
    dActor_c *npc = dNpcList::getNmlByKey(key);
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
    f32 x = pos->x;
    f32 z = pos->z;
    if (dAcNpc_c::checkSpotFunc((int)x >> 5, (int)z >> 5, arg)) {
        return FALSE;
    }
    return TRUE;
}

// 800F4D08
BOOL initTownAnimals() {
    dAnimalBlock_c *block = &dSaveData_c::getTown()->mAnimals.mTown;
    dLandID_c *land = &dSaveData_c::getTown()->mLandID;
    if (block->getAnimalNum() < 6) {
        u32 arg = 0;
        dAnimalTemplate_c *data = dNpcSetup::getList(&arg);
        block->initNewTown(data, arg, land);
    }
    return TRUE;
}

struct handItemOffset_s {
    /* 0x0 */ f32 mX;
    /* 0x4 */ f32 mZ;
};

// 800F4D84: hand item offset of villager species `idx` (axis 0: x, 1: z; dAcNpc_c::getHandItemOfsX/Z).
f32 getAnimalHandItemOfs(u8 idx, u32 axis) {
    static const handItemOffset_s sHandItemOffsets[NPC_SPECIES_NUM] = {
        {-4.0f, 0.0f}, {-8.0f, 0.0f}, {-10.0f, 0.0f}, {-8.0f, 0.0f}, {-7.0f, 0.0f}, {0.0f, 3.0f}, {-5.0f, 2.0f},
        {-8.0f, 0.0f}, {-7.0f, 2.0f}, {-7.0f, 0.0f}, {-4.0f, 6.0f}, {-6.0f, 0.0f}, {-6.0f, 0.0f}, {-8.0f, 0.0f},
        {-8.0f, 0.0f}, {-6.0f, 0.0f}, {-4.0f, 4.0f}, {-6.0f, 0.0f}, {-8.0f, 0.0f}, {-6.0f, 0.0f}, {-8.0f, 0.0f},
        {-4.0f, 6.0f}, {-6.0f, 0.0f}, {-4.0f, 4.0f}, {-6.0f, 0.0f}, {-6.0f, 4.0f}, {-6.0f, 0.0f}, {-8.0f, 0.0f},
        {-5.0f, 0.0f}, {-6.0f, 0.0f}, {-8.0f, 0.0f}, {-4.0f, 4.0f}, {-6.0f, 0.0f},
    };

    if (idx < NPC_SPECIES_NUM && axis < 2) {
        return (&sHandItemOffsets[idx].mX)[axis];
    }
    return 0.0f;
}

// 800F4DB8: hand item offset of special NPC `key` (axis 0: x, 1: z).
f32 getSpHandItemOfs(const dItem::Item *key, u32 axis) {
    static const handItemOffset_s sHandItemOffsets[NPC_SPECIAL_NUM] = {
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
        if (idx < NPC_SPECIAL_NUM && axis < 2) {
            return (&sHandItemOffsets[idx].mX)[axis];
        }
    }
    return 0.0f;
}

// ---------------------------------------------------------------------------
// Permits

// 800F4E00
permit_c::permit_c() {
    mOwnerPlayer = NET_MEMBER_NONE;
    mAckPlayers = 0;
    resetFallbackPlayer();
}

// 800F4E40
permit_c::~permit_c() {}

// 800F4E80
void permit_c::clear() {
    mOwnerPlayer = NET_MEMBER_NONE;
    mAckPlayers = 0;
    resetFallbackPlayer();
}

// 800F4E94
void permit_c::setState(u32 state) {
    if (!dNet::isSessionActive() || dNet::getMemberNum() <= 1) {
        return;
    }
    if (state >= NET_MEMBER_MAX) {
        state = NET_MEMBER_NONE;
    }
    mOwnerPlayer = state;
}

// 800F4EF4
BOOL permit_c::hasState() {
    if (!dNet::isSessionActive() || dNet::getMemberNum() <= 1) {
        return TRUE;
    }
    return (u32)mOwnerPlayer < NET_MEMBER_MAX;
}

// 800F4F54
BOOL permit_c::isFlag(u32 i) {
    if (i >= NET_MEMBER_MAX) {
        return FALSE;
    }
    return (mAckPlayers >> i) & 1;
}

// 800F4F74
void permit_c::setFlag(u32 i) {
    if (i < NET_MEMBER_MAX) {
        mAckPlayers |= 1 << i;
    }
}

// 800F4F94
void permit_c::setFallbackPlayer(u32 value) {
    if (!dNet::isSessionActive() || dNet::getMemberNum() <= 1) {
        return;
    }
    if (value >= NET_MEMBER_MAX) {
        value = NET_MEMBER_NONE;
    }
    mFallbackPlayer = value;
}

// 800F4FF4
void permit_c::resetFallbackPlayer() {
    mFallbackPlayer = NET_MEMBER_NONE;
}

// 800F5000
BOOL permit_c::isOwnedByMe() {
    if (!dNet::isSessionActive() || dNet::getMemberNum() <= 1) {
        return TRUE;
    }
    int state = mOwnerPlayer;
    if (state == NET_MEMBER_NONE) {
        if (dNet::isSceneOwner()) {
            return TRUE;
        }
    } else if (mAckPlayers == 0 && dNet::getMyNo() == state) {
        return TRUE;
    }
    return FALSE;
}

// 800F5094
BOOL permit_c::isReady() {
    if (!dNet::isSessionActive() || dNet::getMemberNum() <= 1) {
        return TRUE;
    }
    if (isOwnedByMe()) {
        return TRUE;
    }
    for (int i = 0; i < NET_MEMBER_MAX; i++) {
        if (dNet::isMember(i) && !isFlag(i)) {
            return FALSE;
        }
    }
    return TRUE;
}

// 800F513C
void permitMng_c::init() {
    permit_c *permit = get(0);
    if (permit != NULL) {
        for (u32 i = 0; i < mPermitCount; i++, permit++) {
            permit->clear();
        }
    }
}

// 800F51AC
permit_c *permitMng_c::get(u32 i) {
    if (mEntries == NULL) {
        return NULL;
    }
    if (i >= mPermitCount) {
        return NULL;
    }
    return &mEntries[i];
}

// 800F51E4
permit_c *permitMng_c::getByKey(const dItem::Item *key) {
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

static normalPermitMng_c *sNormalPermits;   // lbl_8074E568
static specialPermitMng_c *sSpecialPermits; // lbl_8074E56C

// 800F52D4
permitMng_c *getPermitManager(const dItem::Item *key) {
    if (key->mId == dItem::ITEM_ID_NONE) {
        return NULL;
    }
    u16 k = key->mId;
    if (isNormalKey(k)) {
        return sNormalPermits;
    }
    if (((k >> 12) & 0xF) == 8) {
        return sSpecialPermits;
    }
    return NULL;
}

// 800F5370
u32 getPermitHeapSize() {
    return sizeof(normalPermitMng_c) + sizeof(specialPermitMng_c);
}

// 800F5378
void createPermitManagers(EGG::Heap *heap) {
    if (heap != NULL) {
        if (sNormalPermits == NULL) {
            sNormalPermits = (normalPermitMng_c *)heap->alloc(sizeof(normalPermitMng_c), 4);
            if (sNormalPermits != NULL) {
                new (sNormalPermits) normalPermitMng_c();
            }
        }
        if (sSpecialPermits == NULL) {
            sSpecialPermits = (specialPermitMng_c *)heap->alloc(sizeof(specialPermitMng_c), 4);
            if (sSpecialPermits != NULL) {
                new (sSpecialPermits) specialPermitMng_c();
            }
        }
    }
}

// 800F548C
void initPermitManagers() {
    if (sNormalPermits != NULL) {
        sNormalPermits->init();
    }
    if (sSpecialPermits != NULL) {
        sSpecialPermits->init();
    }
}

// 800F54E0
permit_c *getPermitByKey(const dItem::Item *key) {
    permitMng_c *mng = getPermitManager(key);
    if (mng != NULL) {
        return mng->getByKey(key);
    }
    return NULL;
}

static u8 sPermitScene = dScene::SCENE_NUM; // lbl_8074AE94

// 800F5524
void setPermitScene(u8 scene) {
    sPermitScene = scene;
}

// 800F552C
BOOL hasPermitScene() {
    return (u32)sPermitScene < dScene::SCENE_NUM;
}

// 800F5548
void clearPermitScene() {
    setPermitScene(dScene::SCENE_NUM);
}

// 800F5550
BOOL isPermitSceneOwner() {
    if (dNet::isSceneOwner()) {
        if (!dNet::isSessionActive() || dNet::getMemberNum() <= 1) {
            return TRUE;
        }
        if (hasPermitScene()) {
            return TRUE;
        }
    }
    return FALSE;
}

// 800F55B0
void resetPermits() {
    initPermitManagers();
}

// 800F55B4
void onNetSceneOpen(int, int, int, u8 scene) {
    setPermitScene(scene);
}

// 800F55BC
int onNetSceneClose() {
    clearPermitScene();
    return -1;
}

// Definitions in this TU keep the weak copies in its .text before the static initializer.
// 800F55E0
inline specialPermitMng_c::~specialPermitMng_c() {}
// 800F5648
inline normalPermitMng_c::~normalPermitMng_c() {}
// 800F56B0
inline listLand_c::~listLand_c() {}
// 800F5718
inline list_c::~list_c() {}
// 800F5758
inline int daubMngSpecial_c::getBase() { return NET_REC_DAUB_SPECIAL; }
// 800F5760
inline int daubMngNormal_c::getBase() { return NET_REC_DAUB; }

// ---------------------------------------------------------------------------

class loaderEntry_c {
public:
    loaderEntry_c() {
        _14 = 0;
        _18 = 0xFFFF;
    }

    /* 0x00 */ dDvd::loader_c mLoader;
    /* 0x14 */ u32 _14; // Unidentified; only zero initialization is recovered.
    /* 0x18 */ u16 _18; // Unidentified; only 0xFFFF initialization is recovered.
}; // size 0x1C

class loaderPool_c {
public:
    loaderPool_c() {
        for (int i = 0; i < sizeof(mLoaders) / sizeof(*mLoaders); i++) {
            mLoaders[i]._14 = 0;
            mLoaders[i]._18 = 0xFFFF;
        }
    }

    /* 0x00 */ loaderEntry_c mLoaders[6];
}; // size 0xA8

static loaderPool_c sLoaders; // lbl_805CE758

} // namespace dNpc
