#pragma once

#include <types.h>
#include <cstring>
#include <game/game/d_region.hpp>
#include <game/game/d_script.hpp>

#define LAND_NAME_SIZE 8

struct dLandID_c {
    u16 mId;
    wchar_t mName[LAND_NAME_SIZE + 1];
    u8 mRegion;

    inline bool operator==(const dLandID_c& other) const {
        return mId == other.mId && mRegion == other.mRegion && memcmp(mName, other.mName, sizeof(mName)) == 0;
    }

    inline bool operator!=(const dLandID_c& other) const {
        return !(*this == other);
    }
};

// Script word holding a town name. Name from RTTI; vtable 804EE398.
class dLandNameWord_c : public dScript::Word_c {
public:
    dLandNameWord_c(); // 80116788
    virtual ~dLandNameWord_c(); // 801167CC
    virtual u32 getBufferSize(); // 80116824
    virtual wchar_t *getBuffer(); // 8011682C

    /* 0x24 */ wchar_t mBuffer[LAND_NAME_SIZE + 1];
}; // size 0x38
