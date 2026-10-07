#pragma once

#include <types.h>
#include <game/game/d_date.hpp>
#include <game/game/d_save_money.hpp>

// Turnip price trends for the week.
enum {
    STALK_MARKET_TREND_RANDOM,     // random stretches and falling stretches
    STALK_MARKET_TREND_BIG_SPIKE,  // falls, then a spike of up to 6x
    STALK_MARKET_TREND_DECREASING, // falls all week
    STALK_MARKET_TREND_SMALL_SPIKE, // falls, then a spike of up to 2x
    STALK_MARKET_TREND_NUM,
};

#define STALK_MARKET_PRICE_NUM 14 // Sunday AM .. Saturday PM

// The stalk market (turnips): Joan's price and the week's prices at Nook's. In the save data at
// dSaveShops_c::mStalkMarket. Source: src/dol/game/d_save_shop_stalk_market.cpp (.text 80146D80..80147A48).
class dSaveStalkMarket_c { // 0x48
public:
    void clear(); // 80146D80
    void checkDate(); // 80146DD8: daily; a new week or a clock change resets mDate
    void decidePrices(); // 80146ED0: next trend, then the week's prices
    int calcPrice(f32 max, f32 min); // 80147984: mBuyPrice times a random rate in [min, max)
    void invalidate(); // 80147A08

    /* 0x00 */ dSaveMoney_c mBuyPrice; // Joan's price, 90..110
    /* 0x04 */ dSaveMoney_c mPrices[STALK_MARKET_PRICE_NUM];
    /* 0x3C */ int mTrend; // STALK_MARKET_TREND_*
    /* 0x40 */ int mSpikeIdx; // first price of the spike (BIG_SPIKE / SMALL_SPIKE)
    /* 0x44 */ dYMD_c mDate;
};
