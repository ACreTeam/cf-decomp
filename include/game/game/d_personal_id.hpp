#pragma once

#include <types.h>
#include <game/game/d_land.hpp>

#define PLAYER_NAME_LEN 8

enum {
    GENDER_MALE,
    GENDER_FEMALE,
    GENDER_OTHER,

    GENDER_NUM
};

// Source for both classes: src/dol/game/d_personal_id.cpp (.text 8013E618..8013EBCC).
// Names are inferred (Ghidra names noted).
struct dPlayerID_c {
    void set(const wchar_t *name, u16 id, u8 gender); // 8013E978
    void clear();                                     // 8013E9CC (Ghidra: ClearPlayerId)
    BOOL isValid() const;                             // 8013EA14: mId != 0
    void copy(const dPlayerID_c *other);              // 8013EA28
    BOOL isSame(const dPlayerID_c *other) const;      // 8013EA30 (Ghidra: CmpPlayerIds)
    void setName(const wchar_t *name);                // 8013EA98
    void setWord(dScript::Word_c *word) const;        // 8013EAA4: name and gender
    BOOL fn_8013EB28(const dPlayerID_c *other) const; // 8013EB28
    u8 getGender() const { return mGender; }

    static const u16 ID_UNSET; // 80750BF0: 0xFFFF (defined out of line, so callers load it)

    /* 0x00 */ u16 mId;
    /* 0x02 */ wchar_t mName[PLAYER_NAME_LEN+1];
    /* 0x14 */ u8 mGender;
}; // size 0x16

class dPersonalID_c {
public:
    void setPlayer(const wchar_t *name, u16 id, u8 gender);     // 8013E618
    static u16 generateId(const u16 *used, int num);            // 8013E620: random id not in used
    static bool containsId(u16 id, const u16 *ids, int num);    // 8013E690
    static u16 randomId();                                      // 8013E6D0: 0x8000 | rnd(0x7FFF)
    void clear();                                               // 8013E6FC (Ghidra: ClearPersonalID)
    BOOL isSamePlayer(const dPersonalID_c *other) const;        // 8013E734 (Ghidra: CmpPlayerIdsFromPersonalIDs)
    BOOL fn_8013E740(const dPersonalID_c *other) const;         // 8013E740
    void setPlayerName(const wchar_t *name);                    // 8013E860
    BOOL isValid() const;                                       // 8013E868 (Ghidra: PersonalID_Exists)
    void copy(const dPersonalID_c *other);                      // 8013E8C0
    void setWord(dScript::Word_c *word) const;                  // 8013E8C8
    BOOL isLand(const dLandID_c *other) const;                  // 8013E8D0 (Ghidra: PersonalIDMatchesTownId)
    BOOL isFromTown() const;                                    // 8013E938 (Ghidra: IsPersonalIDFromTown)

    BOOL operator==(const dPersonalID_c& other) const {
        return land == other.land && isSamePlayer(&other);
    }

    BOOL operator!=(const dPersonalID_c& other) const {
        return !(*this == other);
    }

    /* 0x00 */ dLandID_c land;
    /* 0x16 */ dPlayerID_c player;
}; // size 0x2C

namespace dHmnName {

// Script word holding a player name. Name from RTTI ("dHmnName::Word_c"); vtable 804E6EE0.
class Word_c : public dScript::Word_c {
public:
    Word_c(); // 800B953C
    virtual ~Word_c(); // 800B9580
    virtual u32 getBufferSize(); // 800B95D8
    virtual wchar_t *getBuffer(); // 800B95E0

    /* 0x24 */ wchar_t mBuffer[PLAYER_NAME_LEN + 1];
}; // size 0x38

} // namespace dHmnName
