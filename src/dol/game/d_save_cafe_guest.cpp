// The cafe's guest schedule (dSaveCafeGuest_c). .text 80149608..80149B70.
#include <game/game/d_save_cafe_guest.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_time_stamp.hpp>
#include <game/cLib/c_math.hpp>
#include <string.h>

// 80149608
void dSaveCafeGuest_c::clear() {
    memset(this, 0, sizeof(dSaveCafeGuest_c));

    for (int i = 0; i < ARRAY_SIZE(mGuests); i++) {
        mGuests[i] = CAFE_GUEST_NONE;
    }
}

// 80149650
void dSaveCafeGuest_c::update() {
    if (!dSaveData_c::getRaw()->isFlag(0x15)) {
        for (int i = 0; i < ARRAY_SIZE(mGuests); i++) {
            mGuests[i] = CAFE_GUEST_NONE;
        }
        return;
    }

    dTime_c now = *dTime_c::getCurrent();
    mGuests[0] = CAFE_GUEST_NONE;
    if (cM::rndInt(100) < 50) {
        mGuests[0] = CAFE_GUEST_PELLY;
    }

    mGuests[1] = CAFE_GUEST_NONE;
    int r = cM::rndInt(100);
    if (now.wday == WEEKDAY_SATURDAY) {
        if (r < 10) {
            mGuests[1] = CAFE_GUEST_KAPPN;
        } else if (r < 25) {
            mGuests[1] = CAFE_GUEST_NONE;
        } else if (r < 28) {
            mGuests[1] = CAFE_GUEST_DON;
        }
    } else if (now.wday == WEEKDAY_SUNDAY) {
        if (r < 10) {
            mGuests[1] = CAFE_GUEST_KAPPN;
        } else if (r < 20) {
            mGuests[1] = CAFE_GUEST_NONE;
        } else if (r < 25) {
            mGuests[1] = CAFE_GUEST_RESETTI;
        }
    } else {
        if (r < 10) {
            mGuests[1] = CAFE_GUEST_KAPPN;
        } else if (r < 25) {
            mGuests[1] = CAFE_GUEST_NONE;
        }
    }

    // The CAFE_GUEST_NONE cases below write slot 1, not 2.
    mGuests[2] = CAFE_GUEST_NONE;
    r = cM::rndInt(100);
    if (now.wday == WEEKDAY_SATURDAY) {
        if (r < 15) {
            mGuests[2] = CAFE_GUEST_KAPPN;
        } else if (r < 25) {
            mGuests[1] = CAFE_GUEST_NONE;
        } else if (r < 28) {
            mGuests[2] = CAFE_GUEST_DON;
        }
    } else if (now.wday == WEEKDAY_SUNDAY) {
        if (r < 10) {
            mGuests[2] = CAFE_GUEST_KAPPN;
        } else if (r < 20) {
            mGuests[1] = CAFE_GUEST_NONE;
        } else if (r < 25) {
            mGuests[2] = CAFE_GUEST_RESETTI;
        }
    } else {
        if (r < 15) {
            mGuests[2] = CAFE_GUEST_KAPPN;
        } else if (r < 25) {
            mGuests[1] = CAFE_GUEST_NONE;
        }
    }

    if (now.wday == 6) {
        mGuests[3] = CAFE_GUEST_TOTAKEKE;
    } else {
        mGuests[3] = CAFE_GUEST_PHYLLIS;
    }

    if (mGuests[1] == mGuests[2]) {
        if (cM::rndInt(2) == 0) {
            mGuests[1] = CAFE_GUEST_NONE;
        } else {
            mGuests[2] = CAFE_GUEST_NONE;
        }
    }
}

// 801498DC
u8 dSaveCafeGuest_c::getCurrentGuest() {
    dTimeStamp_c rolled = *(dTimeStamp_c *)dSaveData_c::getTown()->_068372;
    if (!rolled.isToday(TRUE)) {
        return CAFE_GUEST_NONE;
    }

    dTime_c now = *dTime_c::getCurrent();
    if (now.hour == 6 && now.min < 55) {
        return getGuest(0);
    }
    if (now.hour == 12 || (now.hour == 13 && now.min < 30)) {
        if (mGuests[1] == CAFE_GUEST_DON && !dPlayerMgr_c::getCurrentPlayer()->fn_8013937C()) {
            return CAFE_GUEST_NONE;
        }
        return getGuest(1);
    }
    if ((now.hour == 14 && now.min >= 30) || now.hour == 15) {
        return getGuest(2);
    }
    if (now.wday != WEEKDAY_SATURDAY && now.hour == 21 && now.min < 55) {
        return getGuest(3);
    }
    return CAFE_GUEST_NONE;
}

// 80149A8C
u8 dSaveCafeGuest_c::getGuest(int slot) {
    dTimeStamp_c rolled = *(dTimeStamp_c *)dSaveData_c::getTown()->_068372;
    if (!rolled.isToday(TRUE)) {
        return CAFE_GUEST_NONE;
    }

    switch (slot) {
    case 0:
        return mGuests[0];
    case 1:
        return mGuests[1];
    case 2:
        return mGuests[2];
    case 3:
        return mGuests[3];
    }
    return CAFE_GUEST_NONE;
}
