#pragma once

#include <types.h>
#include <cstring>
#include <game/game/d_region.hpp>

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
