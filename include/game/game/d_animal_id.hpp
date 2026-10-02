#pragma once

#include <types.h>
#include <game/game/d_region.hpp>
#include <game/game/d_land.hpp>

#define ANIMAL_NAME_LEN 8

enum {
    LOOKS_TYPE_LAZY,
    LOOKS_TYPE_JOCK,
    LOOKS_TYPE_CRANKY,
    LOOKS_TYPE_NORMAL,
    LOOKS_TYPE_PEPPY,
    LOOKS_TYPE_SNOOTY,

    LOOKS_TYPE_NUM
};

namespace dScript {
class Word_c;
}

class dAnmPersonalID_c {
public:
    void clear();                                             // 80135E88
    static int getNameSlot(int language);                     // 80135EE0; lbl_804760D8
    void setName(int slot, const wchar_t *name);              // 80135F04
    void set(u16 npcIdx, u8 looks, const dLandID_c *land, const wchar_t *name0, const wchar_t *name1,
             const wchar_t *name2, const wchar_t *name3, const wchar_t *name4, const wchar_t *name5,
             const wchar_t *name6, const wchar_t *name7);     // 80135F20
    BOOL isValid() const;                                           // 80136010
    void copy(const dAnmPersonalID_c *other);                 // 80136068
    u8 getLooks(int unused);                                  // 80136070
    static int looksToGender(u8 looks);                       // 80136078
    static void makeResName(char *buf, u32 size, const char *name, u32 looks); // 801360A0
    int getGender(int unused);                                // 80136138
    const wchar_t *getName(int language);                     // 80136160; LANGUAGE_NUM or more uses the console language
    void setWord(dScript::Word_c *word, int language);        // 801361AC

    inline bool operator==(const dAnmPersonalID_c& other) {
        return mLand == other.mLand && mLand2 == other.mLand2 && mNpcIdx == other.mNpcIdx;
    }

    inline bool operator!=(const dAnmPersonalID_c& other) {
        return !(*this == other);
    }

    dLandID_c mLand;
    dLandID_c mLand2; // TODO: figure it out
    wchar_t mNameByRegion[REGION_NUM][ANIMAL_NAME_LEN+1];
    u16 mNpcIdx; // NPC index
    u8 mLooks; // Personality
};
