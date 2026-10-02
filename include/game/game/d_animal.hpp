#pragma once

// Villager (animal) save data. Draft; see notes/d_animal.txt.
// Class names except dGreetingWord_c / dHabitWord_c (RTTI) are inferred.

#include <types.h>
#include <game/game/d_item.hpp>
#include <game/game/d_land.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_animal_id.hpp>
#include <game/game/d_dsn.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_script.hpp>

#define ANIMAL_NUM 10        // villagers per town (fn_80129A3C)
#define ANIMAL_MEMORY_NUM 16 // fn_8011E2C0
#define ANIMAL_GREETING_LEN 16
#define ANIMAL_HABIT_LEN 10

// Script words for a villager's greeting and catchphrase. Names from RTTI.
// The vtables also hold many inherited dScript::Word_c entries not declared in d_script.hpp.
class dGreetingWord_c : public dScript::Word_c { // vtable 804EF0C8
public:
    dGreetingWord_c(); // 8011B400
    virtual ~dGreetingWord_c(); // 8011B444
    virtual u32 getBufferSize(); // 8011B49C
    virtual wchar_t *getBuffer(); // 8011B4A4

    /* 0x24 */ wchar_t mBuffer[ANIMAL_GREETING_LEN + 1];
}; // size 0x48

class dHabitWord_c : public dScript::Word_c { // vtable 804EEFD0
public:
    dHabitWord_c(); // 8011B4AC
    virtual ~dHabitWord_c(); // 8011B4F0
    virtual u32 getBufferSize(); // 8011B548
    virtual wchar_t *getBuffer(); // 8011B550

    /* 0x24 */ wchar_t mBuffer[ANIMAL_HABIT_LEN + 1];
}; // size 0x3C

// Two capped counters (max 5) at dAnimalMemory_c+0x4E. The kind argument goes
// through fn_80162548 / fn_80162594 to pick which counter applies.
class dAnimalMemoryCount_c {
public:
    dAnimalMemoryCount_c(); // 8011C618
    ~dAnimalMemoryCount_c(); // 8011C61C
    void clear(); // 8011C65C
    void inc(u32 kind); // 8011C66C
    u8 get(u32 kind); // 8011C6E4

    /* 0x0 */ u8 _0;
    /* 0x1 */ u8 _1;
}; // size 0x2

// A villager's memory of one player (AC's Anmmem_c equivalent). 0x8C.
class dAnimalMemory_c {
public:
    dAnimalMemory_c(); // 8011C73C
    ~dAnimalMemory_c(); // 8011C79C
    void clear(); // 8011C7F8

    /* 0x00 */ u8 _00[4];
    /* 0x04 */ dQuestTime_c _04;
    /* 0x0C */ dPersonalID_c mPlayer;
    /* 0x38 */ dLandID_c _38;           // cleared by fn_80116710
    /* 0x4E */ dAnimalMemoryCount_c _4E;
    /* 0x50 */ u8 _50[0x12];
    /* 0x62 */ wchar_t _62[17];          // 0x22 bytes, memset by clear
    /* 0x84 */ dItem::Item _84;
    /* 0x86 */ dItem::Item _86;
    /* 0x88 */ u8 _88;
    /* 0x89 */ u8 _89;                  // '1' when cleared
    /* 0x8A */ u8 _8A;
}; // size 0x8C

// 3 bytes at dAnimal_c+0x2FF6.
class dUnk2FF6_c {
public:
    dUnk2FF6_c(); // 8011D450
    ~dUnk2FF6_c(); // 8011D454
    void clear(); // 8011D494

    /* 0x0 */ u8 _0; // 3 when cleared
    /* 0x1 */ u8 _1;
    /* 0x2 */ u8 _2;
};

// 0x14 at dAnimal_c+0x301C. fn_8011E200 fills _08/_0C/_10/_11.
class dUnk301C_c {
public:
    dUnk301C_c(); // 8011D558
    ~dUnk301C_c(); // 8011D594
    void clear(); // 8011D5D4

    /* 0x00 */ dQuestTime_c _00;
    /* 0x08 */ s32 _08; // -1 when cleared
    /* 0x0C */ s32 _0C; // -1 when cleared
    /* 0x10 */ u8 _10;  // 4 when cleared
    /* 0x11 */ u8 _11;
}; // size 0x14

// Unrecovered member objects. Their constructors and destructors live in other TUs.
class dUnk2BE4_c { // 0x82; wraps a dQuestVillager_c at +2
public:
    dUnk2BE4_c(); // 80143660
    ~dUnk2BE4_c(); // 80143698

    /* 0x00 */ u8 _00; // fn_8011E688 checks for 2
    /* 0x01 */ u8 _01;
    /* 0x02 */ dQuestVillager_c mQuest;
};

class dUnk300C_c { // 0x10
public:
    dUnk300C_c(); // 8011927C
    ~dUnk300C_c(); // 80119280

    u8 _00[0x10];
};

// Per-species template record, copied whole into dAnimal_c+0x1824 by fn_8011E688.
struct dAnimalTemplate_c {
    /* 0x000 */ u8 _000[2];
    /* 0x002 */ s16 _002; // -> dAnimal_c::_2FFA
    /* 0x004 */ s16 _004; // fn_80122138
    /* 0x006 */ s16 _006; // fn_8012209C
    /* 0x008 */ u8 _008[0x1A];
    /* 0x022 */ wchar_t mNames[REGION_NUM][ANIMAL_NAME_LEN + 1];
    /* 0x0B2 */ u8 _0B2[0xE4];
    /* 0x196 */ u8 mLooks : 4;
    /* 0x196 */ u8 _196_lo : 4;
    /* 0x197 */ u8 _197;
}; // size 0x198

// One villager's save record. 0x3040, alignment 32 (dDesign_c).
class dAnimal_c {
public:
    dAnimal_c(); // 8011E2C0
    ~dAnimal_c(); // 8011E39C
    void clear(); // 8011E458
    void copy(const dAnimal_c *other); // 8011E544
    BOOL isChecksumValid(); // 8011E54C
    u32 getChecksum(); // 8011E5C0
    u32 calcChecksum(); // 8011E61C
    void init(u16 npcIdx, u8 arg, const dLandID_c *land, const dAnimalTemplate_c *tmpl); // 8011E688
    void fn_8011E790(const void *src); // copies the 0x1824-byte head and sets _3035

    /* 0x0000 */ u8 _0000[0x1820];     // checksummed area
    /* 0x1820 */ u32 mChecksum;        // fn_802A98FC over 0x0000..0x1820
    /* 0x1824 */ dAnimalTemplate_c mTemplate;
    /* 0x19BC */ u8 _19BC[4];
    /* 0x19C0 */ dDesign_c mDesign;
    /* 0x2240 */ dQuestTime_c _2240;
    /* 0x2248 */ u8 _2248[4];
    /* 0x224C */ dAnmPersonalID_c mID;
    /* 0x230C */ dLandID_c _230C;
    /* 0x2322 */ u8 _2322[2];
    /* 0x2324 */ dAnimalMemory_c mMemories[ANIMAL_MEMORY_NUM];
    /* 0x2BE4 */ dUnk2BE4_c mQuest;
    /* 0x2C66 */ dMail_c mMail;
    /* 0x2FF6 */ dUnk2FF6_c _2FF6;
    /* 0x2FFA */ dItem::Item _2FFA;
    /* 0x2FFC */ dItem::Item _2FFC[4];
    /* 0x3004 */ dItem::Item _3004;
    /* 0x3006 */ dItem::Item _3006;
    /* 0x3008 */ dItem::Item _3008;
    /* 0x300A */ u16 _300A;
    /* 0x300C */ dUnk300C_c _300C;
    /* 0x301C */ dUnk301C_c _301C;
    /* 0x3030 */ u8 _3030[2];
    /* 0x3032 */ u8 _3032; // 4 when cleared; set by init
    /* 0x3033 */ u8 _3033; // 3 when cleared
    /* 0x3034 */ u8 _3034; // 4 when cleared
    /* 0x3035 */ u8 _3035; // set by fn_8011E790; gates the checksum check
    /* 0x3036 */ u8 _3036[0xA];
}; // size 0x3040

// Town-level members of dAnimalBlock_c owned by other TUs.
class dUnk1E284_c { // 0x46
public:
    dUnk1E284_c(); // 801426DC

    u8 _00[0x46];
};

class dUnk1E2CA_c { // 0xA
public:
    dUnk1E2CA_c(); // 80142268

    u8 _00[0xA];
};

// {index, ?} at dAnimalBlock_c+0x1E452.
class dUnk1E452_c {
public:
    dUnk1E452_c(); // 80129854
    void clear(); // 80129858
    BOOL isValid(); // 8012986C: _0 < ANIMAL_NUM && _1 < 4

    /* 0x0 */ s8 _0; // -1 when cleared
    /* 0x1 */ u8 _1;
};

// All villagers of a town plus town-level quests. 0x1E4A0.
class dAnimalBlock_c {
public:
    dAnimalBlock_c(); // 80129A3C
    void clear(); // 80129AE4

    /* 0x00000 */ dAnimal_c mAnimals[ANIMAL_NUM];
    /* 0x1E280 */ u8 _1E280[4];
    /* 0x1E284 */ dUnk1E284_c _1E284;     // clear fn_801426E0
    /* 0x1E2CA */ dUnk1E2CA_c _1E2CA;     // clear fn_801422A4
    /* 0x1E2D4 */ dQuestPlayerItem_c mQuestItem;     // clear fn_80142A08
    /* 0x1E2FE */ dQuestPlayerAnimal_c mQuestAnimal; // clear fn_80142EA8
    /* 0x1E3EA */ dQuestPlayerPair_c mQuestPair;     // clear fn_8014346C
    /* 0x1E44A */ dQuestTime_c _1E44A;
    /* 0x1E452 */ dUnk1E452_c _1E452;
    /* 0x1E454 */ u8 _1E454[0x14];
    /* 0x1E468 */ s8 _1E468[ANIMAL_NUM]; // filled with -1 by fn_8012A9AC
    /* 0x1E472 */ u8 _1E472;             // 0x2F when cleared
    /* 0x1E473 */ u8 _1E473[0x1C];       // passed to fn_8011B838 by fn_80129BC4
    /* 0x1E48F */ s8 _1E48F;             // -1 when cleared
    /* 0x1E490 */ s8 _1E490;             // -1 when cleared
    /* 0x1E491 */ u8 _1E491[0xF];
}; // size 0x1E4A0

// Second villager array in the save, after dAnimalBlock_c. 0x1E2A0.
class dAnimalList_c {
public:
    dAnimalList_c(); // 80133AC4

    /* 0x00000 */ dAnimal_c mAnimals[ANIMAL_NUM];
    /* 0x1E280 */ u8 _1E280[0x20];
}; // size 0x1E2A0

// The villager part of the save file (dSaveData_c+0x21B20). 0x3C740.
class dAnimalSave_c {
public:
    dAnimalSave_c(); // 801345D8

    /* 0x00000 */ dAnimalBlock_c mBlock;
    /* 0x1E4A0 */ dAnimalList_c mList;
}; // size 0x3C740
