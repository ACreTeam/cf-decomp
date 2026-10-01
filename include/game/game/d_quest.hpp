#pragma once

#include <types.h>
#include <game/game/d_date.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_animal_id.hpp>

// Quest categories returned by fn_80143764 from dQuestBase_c::mKind.
// Names past ERRAND are placeholders; the shapes are below.
enum {
    QUEST_TYPE_VILLAGER, // kinds 0-6   (dQuestVillager_c, one per villager)
    QUEST_TYPE_ERRAND,   // kinds 7-16  (dQuestErrand_c, in dPrivateData_c)
    QUEST_TYPE_2,        // kinds 17-18 (probably dQuestPlayerItem_c)
    QUEST_TYPE_3,        // kind 19     (dQuestPlayerPair_c)
    QUEST_TYPE_4,        // kind 20     (dQuestPlayerAnimal_c)
    QUEST_TYPE_NONE,     // kind 21+

    QUEST_TYPE_NUM
};

#define QUEST_KIND_NONE 0x15
#define QUEST_ERRAND_HANDLER_NUM 10 // lbl_805F2680, kinds 7-16

// 64-bit time limit stored as bytes so it can sit at 2-byte-aligned offsets.
// ctor fn_8014C5D0(s64) memcpys the value in; fn_8014C468 resets it to INT64_MAX (no limit).
struct dQuestTime_c;
extern "C" {
void fn_8014C5D0(dQuestTime_c *time, s64 value);
void fn_8014C538(dQuestTime_c *time, dTime_c *cal);
}

struct dQuestTime_c {
    dQuestTime_c(s64 value = 0x7FFFFFFFFFFFFFFFLL) { fn_8014C5D0(this, value); }
    dQuestTime_c(dTime_c* cal) { fn_8014C538(this, cal); }

    u8 mData[8];
};

// 0x0E; ctor fn_8013F718, clear fn_8013F7A0, set fn_8013F7F0
struct dQuestBase_c {
    /* 0x00 */ dQuestTime_c mTimeLimit;
    /* 0x08 */ dItem::Item mItem; // 0xFFF1 when empty
    /* 0x0A */ u8 mKind;          // QUEST_KIND_NONE when cleared
    /* 0x0B */ u8 _0B;
    /* 0x0C */ u8 _0C;            // 7 when cleared
};

// 0x80; ctor fn_80141814, clear fn_8014189C, start fn_80141910 (requires QUEST_TYPE_VILLAGER).
// Lives at +0x2BE6 of each 0x3040 villager (wrapper ctor fn_80143660 at +0x2BE4).
struct dQuestVillager_c {
    /* 0x00 */ dQuestBase_c mBase;
    /* 0x0E */ dPlayerID_c mPlayer;      // cleared by fn_80142120
    /* 0x24 */ dPlayerID_c _24[4];       // one per save player?
    /* 0x7C */ u8 _7C;
    /* 0x7D */ u8 _7D;
    /* 0x7E */ u8 _7E;
};

// 0x190; ctor fn_80140650, dtor fn_80140680, clear fn_801406D8, start fn_8014073C
// (requires QUEST_TYPE_ERRAND, then runs the per-kind handler table).
class dQuestErrand_c {
public:
    /* 0x000 */ dQuestBase_c mBase;
    /* 0x00E */ dAnmPersonalID_c mAnimals[2]; // fn_80140840/fn_80140850 index these; GC orders recipient, sender
    /* 0x18E */ u8 _18E;
};

// 0x190; ctor fn_80140860 constructs mErrands with __construct_array(count 1).
// fn_801409C4/fn_801409E0 return &mErrands[i] for i < 1, else NULL.
class dQuestErrandList_c {
public:
    dQuestErrandList_c(); // 80140860
    ~dQuestErrandList_c(); // 801408A8
    dQuestErrand_c *get(u32 i);

    /* 0x000 */ dQuestErrand_c mErrands[1];
};

// 0x2A; ctor fn_801429C8, clear fn_80142A08, start fn_80142DA0 (kind from caller).
// Town-level, at +0x1E2D4 of the villager block (fn_80129A3C).
struct dQuestPlayerItem_c {
    /* 0x00 */ dPlayerID_c mPlayer;
    /* 0x16 */ dQuestBase_c mBase;
    /* 0x24 */ dItem::Item mItem;
    /* 0x26 */ s8 _26; // -1 when cleared
    /* 0x27 */ u8 _27;
    /* 0x28 */ u8 _28;
    /* 0x29 */ u8 _29;
};

// 0xEC; ctor fn_80142E68, clear fn_80142EDC, start fn_80143028 (kind 20).
// Town-level, at +0x1E2FE of the villager block.
struct dQuestPlayerAnimal_c {
    /* 0x00 */ dPlayerID_c mPlayer;
    /* 0x16 */ dAnmPersonalID_c mAnimal;
    /* 0xD6 */ dQuestBase_c mBase;
    /* 0xE4 */ dItem::Item mItem;
    /* 0xE6 */ u8 _E6[3]; // set by index via fn_80142F7C
    /* 0xE9 */ u8 _E9;
    /* 0xEA */ u8 _EA;    // random pick from lbl_80750C28/lbl_80750C2C
    /* 0xEB */ u8 _EB;
};

// 0x60; ctor fn_80143438, clear fn_801434A0, start fn_80143520 (kind 19).
// Town-level, at +0x1E3EA of the villager block.
struct dQuestPlayerPair_c {
    /* 0x00 */ dPlayerID_c mPlayers[2];
    /* 0x2C */ dQuestBase_c mBase;
    /* 0x3A */ wchar_t mText[17]; // 0x22 bytes, memset by clear
    /* 0x5C */ u8 _5C;
    /* 0x5D */ u8 _5D;            // 5 when cleared
    /* 0x5E */ u8 _5E;
};
