#pragma once

#include <types.h>
#include <cstring>
#include <game/game/d_region.hpp>
#include <game/game/d_script.hpp>

#define LAND_NAME_SIZE 8

// Names are inferred.
struct dLandID_c {
    static u16 makeRandomId();                                 // 8011660C: random ID with bit 15 set
    void set(const wchar_t *name, u16 id, int region);         // 80116638
    void setRandomId(const wchar_t *name, int region);         // 801166A0: set() with makeRandomId()
    void setName(const wchar_t *name, int region);             // 801166FC: set() with ID 0xFFFF
    void clear();                                              // 80116710: region becomes LANGUAGE_NUM
    BOOL isValid() const;                                      // 80116758: ID is not 0
    void copy(const dLandID_c *other);                         // 8011676C
    void setWord(dScript::Word_c *word);                       // 80116774: puts the name into a script word
    BOOL isNewTown(const dLandID_c *other) const;              // 80116834: other town, and the current player has no record of it

    /* 0x00 */ u16 mId;
    /* 0x02 */ wchar_t mName[LAND_NAME_SIZE + 1];
    /* 0x14 */ u8 mRegion;

    inline bool operator==(const dLandID_c& other) const {
        return mId == other.mId && mRegion == other.mRegion && memcmp(mName, other.mName, sizeof(mName)) == 0;
    }

    inline bool operator!=(const dLandID_c& other) const {
        return !(*this == other);
    }
}; // size 0x16

// Script word holding a town name. Name from RTTI; vtable 804EE398.
class dLandNameWord_c : public dScript::Word_c {
public:
    dLandNameWord_c(); // 80116788
    virtual ~dLandNameWord_c(); // 801167CC
    virtual u32 getBufferSize(); // 80116824
    virtual wchar_t *getBuffer(); // 8011682C

    /* 0x24 */ wchar_t mBuffer[LAND_NAME_SIZE + 1];
}; // size 0x38
