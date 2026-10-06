// An amount of Bells (dSaveMoney_c). .text 80119444..80119478.
#include <game/game/d_save_money.hpp>

// 80119444
int dSaveMoney_c::get() const {
    return mMoney;
}

// 8011944C
void dSaveMoney_c::set(int money) {
    if (money > SAVE_MONEY_MAX) {
        money = SAVE_MONEY_MAX;
    } else if (money < 0) {
        money = 0;
    }
    mMoney = money;
}
