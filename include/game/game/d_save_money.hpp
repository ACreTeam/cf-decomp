#pragma once

#include <types.h>

#define SAVE_MONEY_MAX 999999999

// An amount of Bells, clamped to 0..SAVE_MONEY_MAX. Used by the shop / turnip code (at +0x98 of its
// object). Source: src/dol/game/d_save_money.cpp (.text 80119444..80119478).
class dSaveMoney_c { // 0x4
public:
    dSaveMoney_c() {}
    dSaveMoney_c(int money) { set(money); }
    dSaveMoney_c &operator=(const dSaveMoney_c &other) {
        set(other.get());
        return *this;
    }

    int get() const; // 80119444
    void set(int money); // 8011944C

    /* 0x0 */ int mMoney;
};
