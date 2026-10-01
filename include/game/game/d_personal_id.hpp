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

struct dPlayerID_c {
    u16 mId;
    wchar_t mName[PLAYER_NAME_LEN+1];
    u8 mGender;
};

class dPersonalID_c {
public:
    dLandID_c land;
    dPlayerID_c player;
};
