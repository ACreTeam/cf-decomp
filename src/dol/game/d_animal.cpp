// Villager (animal) save data TU. Draft: only the class anchors are written so far.
// .text 8011B400..80135E88 (sinit fn_80135E40). See notes/d_animal.txt.
#include <game/game/d_animal.hpp>
#include <game/cLib/c_lib.hpp>
#include <cstring>
#include <cstddef>

// Dependencies whose owners are not recovered yet.
extern "C" {
// dQuestTime_c.
void fn_8014C468(dQuestTime_c *time); // reset to INT64_MAX

// Member objects owned by other TUs.
void fn_801192F4(dUnk300C_c *obj); // clear
void fn_80143700(dUnk2BE4_c *obj); // clear
BOOL fn_8013F4B0(dUnk2BE4_c *obj);
void fn_8013F508(dUnk2BE4_c *obj);

// Town-level members of dAnimalBlock_c (d_quest).
void fn_801426E0(dUnk1E284_c *obj); // clear
void fn_801422A4(dUnk1E2CA_c *obj); // clear
void fn_80142A08(dQuestPlayerItem_c *quest); // clear
void fn_80142EA8(dQuestPlayerAnimal_c *quest); // clear
void fn_8014346C(dQuestPlayerPair_c *quest); // clear

// Memory-kind helpers.
u32 fn_80162548();
BOOL fn_80162594(u8 kind, int);

// CRC (seed/final args as in dPrivateData_c).
u32 fn_802A98FC(const void *data, u32 size, int, int);
}

// Later functions of this TU, not written yet (C linkage keeps the target names).
extern "C" {
void fn_80120548(dAnimal_c *animal);
void fn_80120584(dAnimal_c *animal);
void fn_8012209C(dAnimal_c *animal, const dItem::Item *item);
void fn_80122138(dAnimal_c *animal, const dItem::Item *item);
void fn_801223A0(dAnimal_c *animal);
void fn_80122628(dAnimal_c *animal);
void fn_8012A9AC(s8 *buf, u32 num); // fills with -1
void fn_80131704(dAnimalBlock_c *block);
}

// 805F15E8; constructed by __sinit (80135E40), first used by fn_80122A60.
static dMail_c sMail;

// 8011B400
dGreetingWord_c::dGreetingWord_c() {
    clear();
}

// 8011B444
dGreetingWord_c::~dGreetingWord_c() {}

// 8011B49C
u32 dGreetingWord_c::getBufferSize() {
    return sizeof(mBuffer);
}

// 8011B4A4
wchar_t *dGreetingWord_c::getBuffer() {
    return mBuffer;
}

// 8011B4AC
dHabitWord_c::dHabitWord_c() {
    clear();
}

// 8011B4F0
dHabitWord_c::~dHabitWord_c() {}

// 8011B548
u32 dHabitWord_c::getBufferSize() {
    return sizeof(mBuffer);
}

// 8011B550
wchar_t *dHabitWord_c::getBuffer() {
    return mBuffer;
}

// 8011B558
// Finds the smallest counts. Sets a bit in *mask for each index tied at the
// minimum and returns how many there are. Indices set in skipMask are ignored
// when useSkip is nonzero.
extern "C" int fn_8011B558(u32 *mask, const u32 *counts, u32 num, u32 skipMask, BOOL useSkip) {
    int found = 0;
    u32 min = 0xFFFFFFFF;
    *mask = 0;

    for (u32 i = 0; i < num; i++, counts++) {
        if (skipMask != 0 && useSkip != 0 && ((skipMask >> i) & 1)) {
            continue;
        }

        if (*counts < min) {
            min = *counts;
            *mask = 1 << i;
            found = 1;
        } else if (*counts == min) {
            *mask |= 1 << i;
            found++;
        }
    }

    return found;
}

// 8011B5E0
// Least common personality among the given villagers.
extern "C" int fn_8011B5E0(u32 *mask, dAnimal_c *animals, u32 num, u32 skipMask, BOOL useSkip) {
    u32 counts[LOOKS_TYPE_NUM];
    memset(counts, 0, sizeof(counts));

    for (u32 i = 0; i < num; i++, animals++) {
        dAnmPersonalID_c *id = &animals->mID;
        if (id->isValid() && id->getLooks(1) < LOOKS_TYPE_NUM) {
            counts[id->getLooks(1)]++;
        }
    }

    return fn_8011B558(mask, counts, LOOKS_TYPE_NUM, skipMask, useSkip);
}

// TODO: 8011B6F4..8011C618

// 8011C618
dAnimalMemoryCount_c::dAnimalMemoryCount_c() {}

// 8011C61C
dAnimalMemoryCount_c::~dAnimalMemoryCount_c() {}

// 8011C65C
void dAnimalMemoryCount_c::clear() {
    _0 = 0;
    _1 = 0;
}

// 8011C66C
void dAnimalMemoryCount_c::inc(u32 kind) {
    if (_0 < 5) {
        _0++;
    }

    if (_1 < 5) {
        if (kind == 0x44) {
            kind = fn_80162548();
        }
        if (!fn_80162594(kind, 5)) {
            _1++;
        }
    }
}

// 8011C6E4
u8 dAnimalMemoryCount_c::get(u32 kind) {
    if (kind == 0x44) {
        kind = fn_80162548();
    }
    if (fn_80162594(kind, 5)) {
        return _0;
    }
    return _1;
}

// 8011C73C
dAnimalMemory_c::dAnimalMemory_c() {
    clear();
}

// 8011C79C
dAnimalMemory_c::~dAnimalMemory_c() {}

// 8011C7F8
void dAnimalMemory_c::clear() {
    mPlayer.clear();
    fn_8014C468(&_04);
    _38.clear();
    memset(_50, 0, sizeof(_50));
    memset(_62, 0, sizeof(_62));
    _84 = dItem::ITEM_ID_NONE;
    memset(_00, 0, sizeof(_00));
    _88 = 0;
    _89 = '1';
    _4E.clear();
    _8A = 0;
    _86 = dItem::ITEM_ID_NONE;
}

// TODO: 8011C83C..8011D450

// 8011D450
dUnk2FF6_c::dUnk2FF6_c() {}

// 8011D454
dUnk2FF6_c::~dUnk2FF6_c() {}

// 8011D494
void dUnk2FF6_c::clear() {
    _0 = 3;
    _1 = 0;
    _2 = 0;
}

// TODO: 8011D4A8..8011D558

// 8011D558
dUnk301C_c::dUnk301C_c() {}

// 8011D594
dUnk301C_c::~dUnk301C_c() {}

// 8011D5D4
void dUnk301C_c::clear() {
    fn_8014C468(&_00);
    _08 = -1;
    _0C = -1;
    _11 = 0;
    _10 = 4;
}

// TODO: 8011D618..8011E2C0 (fn_8011E200 fills a dUnk301C_c)

// 8011E2C0
dAnimal_c::dAnimal_c() {
    clear();
}

// 8011E39C
dAnimal_c::~dAnimal_c() {}

// 8011E458
void dAnimal_c::clear() {
    memset(this, 0, sizeof(dAnimal_c));
    mID.clear();
    _3032 = 4;

    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++) {
        mMemories[i].clear();
    }

    _230C.clear();
    fn_80120584(this);
    _3004 = dItem::ITEM_ID_NONE;
    _3006 = dItem::ITEM_ID_NONE;
    mDesign.clear();
    mMail.clear();
    fn_80143700(&mQuest);
    _3033 = 3;
    _3034 = 4;
    _2FF6.clear();
    _300A = 0;
    _3008 = dItem::ITEM_ID_NONE;
    fn_801192F4(&_300C);
    _301C.clear();
    fn_801223A0(this);
}

// 8011E544
void dAnimal_c::copy(const dAnimal_c *other) {
    memcpy(this, other, sizeof(dAnimal_c));
}

// 8011E54C
BOOL dAnimal_c::isChecksumValid() {
    if (!mID.isValid() || !_3035) {
        return TRUE;
    }

    u32 stored = getChecksum();
    return calcChecksum() == stored;
}

// 8011E5C0
u32 dAnimal_c::getChecksum() {
    u32 sum = 0;
    if (mID.isValid() && this != NULL) {
        cLib::memCpy(&sum, &mChecksum, sizeof(sum));
    }
    return sum;
}

// 8011E61C
u32 dAnimal_c::calcChecksum() {
    u32 sum = 0;
    if (mID.isValid() && this != NULL) {
        sum = fn_802A98FC(this, offsetof(dAnimal_c, mChecksum), 0x04201018, -1);
    }
    return sum;
}

// 8011E688
void dAnimal_c::init(u16 npcIdx, u8 arg, const dLandID_c *land, const dAnimalTemplate_c *tmpl) {
    clear();
    mID.set(npcIdx, tmpl->mLooks, land, tmpl->mNames[0], tmpl->mNames[1], tmpl->mNames[2], tmpl->mNames[3],
            tmpl->mNames[4], tmpl->mNames[5], tmpl->mNames[6], tmpl->mNames[7]);
    _3032 = arg;
    _2FFA.mId = tmpl->_002;

    dItem::Item item0;
    item0.mId = tmpl->_006;
    fn_8012209C(this, &item0);
    dItem::Item item1;
    item1.mId = tmpl->_004;
    fn_80122138(this, &item1);

    memcpy(&mTemplate, tmpl, sizeof(dAnimalTemplate_c));

    dUnk2BE4_c *quest = &mQuest;
    fn_8013F508(quest);
    if (fn_8013F4B0(quest) && (int)quest->_00 == 2) {
        fn_80120548(this);
    }

    fn_80122628(this);
}

// 8011E790
void dAnimal_c::fn_8011E790(const void *src) {
    memcpy(this, src, offsetof(dAnimal_c, mTemplate));
    _3035 = 1;
}

// TODO: 8011E7C8..80129854

// 80129854
dUnk1E452_c::dUnk1E452_c() {}

// 80129858
void dUnk1E452_c::clear() {
    _0 = -1;
    _1 = 0;
}

// 8012986C
BOOL dUnk1E452_c::isValid() {
    if ((u32)_0 < ANIMAL_NUM && _1 < 4) {
        return TRUE;
    }
    return FALSE;
}

// TODO: 80129898..80129A3C

// 80129A3C
dAnimalBlock_c::dAnimalBlock_c() {}

// 80129AE4
void dAnimalBlock_c::clear() {
    memset(this, 0, sizeof(dAnimalBlock_c));

    for (int i = 0; i < ANIMAL_NUM; i++) {
        mAnimals[i].clear();
    }

    fn_801426E0(&_1E284);
    fn_801422A4(&_1E2CA);
    fn_80142A08(&mQuestItem);
    fn_80142EA8(&mQuestAnimal);
    fn_8014346C(&mQuestPair);
    _1E452.clear();
    fn_8012A9AC(_1E468, ANIMAL_NUM);
    _1E472 = 0x2F;
    _1E48F = -1;
    _1E490 = -1;
    fn_80131704(this);
}

// TODO: 80129BC4..80133AC4 (fn_80129BC4 fills the block for a new town)

// 80133AC4
dAnimalList_c::dAnimalList_c() {}

// TODO: 80133B0C..801345D8

// 801345D8
dAnimalSave_c::dAnimalSave_c() {}

// TODO: 80134614..80135E40
