// The stalk market (dSaveStalkMarket_c). .text 80146D80..80147A48, .sdata2 80750C68..80750CA0.
#include <game/game/d_save_stalk_market.hpp>
#include <game/game/d_save_data.hpp>
#include <game/cLib/c_math.hpp>
#include <cstring>

// Not split yet (C linkage keeps the target names).
extern "C" {
void fn_8014CCC4(dYMD_c *date, BOOL gameDay); // date = today
int fn_8014CFB8(dYMD_c *date, dTime_c *time, BOOL adjust); // days from date to time
BOOL fn_8014D07C(dSaveTimeOffset_c *offset); // the clock was changed
}

// 80146D80
void dSaveStalkMarket_c::clear() {
    memset(this, 0, sizeof(dSaveStalkMarket_c));
    mTrend = cM::rndInt(STALK_MARKET_TREND_NUM);
    decidePrices();
    mDate.clear();
}

// 80146DD8
void dSaveStalkMarket_c::checkDate() {
    BOOL reset = FALSE;
    if (mDate.year == 0 || mDate.day == 0) {
        reset = TRUE;
    }
    if (reset) {
        fn_80116540(dSaveData_c::getTown(), SAVE_FLAG_TURNIPS_SPOILED);
        fn_8014CCC4(&mDate, TRUE);
    } else if (fn_8014D07C(&dSaveData_c::getTown()->mTimeOffset)) {
        fn_80116510(dSaveData_c::getTown(), SAVE_FLAG_TURNIPS_SPOILED);
        fn_8014CCC4(&mDate, TRUE);
    } else {
        int days = fn_8014CFB8(&mDate, dTime_c::getCurrent(), FALSE);
        if (days < 0) {
            fn_80116510(dSaveData_c::getTown(), SAVE_FLAG_TURNIPS_SPOILED);
            fn_8014CCC4(&mDate, TRUE);
        } else if (days >= 7) {
            fn_80116540(dSaveData_c::getTown(), SAVE_FLAG_TURNIPS_SPOILED);
            fn_8014CCC4(&mDate, TRUE);
        }
    }
}

// 80146ED0
void dSaveStalkMarket_c::decidePrices() {
    // Joan sells at 90..110 Bells.
    mBuyPrice.set(cM::rndInt(21) + 90);

    int r = cM::rndInt(100);
    switch (mTrend) {
    case STALK_MARKET_TREND_RANDOM:
        if (r < 20) {
            mTrend = STALK_MARKET_TREND_RANDOM;
        } else if (r < 50) {
            mTrend = STALK_MARKET_TREND_BIG_SPIKE;
        } else if (r < 65) {
            mTrend = STALK_MARKET_TREND_DECREASING;
        } else {
            mTrend = STALK_MARKET_TREND_SMALL_SPIKE;
        }
        break;
    case STALK_MARKET_TREND_BIG_SPIKE:
        if (r < 50) {
            mTrend = STALK_MARKET_TREND_RANDOM;
        } else if (r < 55) {
            mTrend = STALK_MARKET_TREND_BIG_SPIKE;
        } else if (r < 75) {
            mTrend = STALK_MARKET_TREND_DECREASING;
        } else {
            mTrend = STALK_MARKET_TREND_SMALL_SPIKE;
        }
        break;
    case STALK_MARKET_TREND_DECREASING:
        if (r < 25) {
            mTrend = STALK_MARKET_TREND_RANDOM;
        } else if (r < 70) {
            mTrend = STALK_MARKET_TREND_BIG_SPIKE;
        } else if (r < 75) {
            mTrend = STALK_MARKET_TREND_DECREASING;
        } else {
            mTrend = STALK_MARKET_TREND_SMALL_SPIKE;
        }
        break;
    case STALK_MARKET_TREND_SMALL_SPIKE:
        if (r < 45) {
            mTrend = STALK_MARKET_TREND_RANDOM;
        } else if (r < 70) {
            mTrend = STALK_MARKET_TREND_BIG_SPIKE;
        } else if (r < 85) {
            mTrend = STALK_MARKET_TREND_DECREASING;
        } else {
            mTrend = STALK_MARKET_TREND_SMALL_SPIKE;
        }
        break;
    default:
        mTrend = STALK_MARKET_TREND_DECREASING;
        break;
    }

    if ((*(u16 *)&dSaveData_c::getTown()->_05EC68[0xC] & 2) && mTrend == STALK_MARKET_TREND_DECREASING) {
        mTrend = STALK_MARKET_TREND_RANDOM;
    }

    for (int i = 0; i < STALK_MARKET_PRICE_NUM; i++) {
        mPrices[i] = dSaveMoney_c(0);
    }
    // Nook's doesn't buy on Sunday.
    mPrices[0] = dSaveMoney_c(100);
    mPrices[1] = dSaveMoney_c(100);

    switch (mTrend) {
    case STALK_MARKET_TREND_RANDOM: {
        dSaveMoney_c *price;
        int idx, rand1, fall1, fall2, rand2, rand3;
        if (cM::rndInt(2) == 0) {
            fall1 = 2;
            fall2 = 3;
        } else {
            fall1 = 3;
            fall2 = 2;
        }
        rand1 = cM::rndInt(3);
        rand3 = cM::rndInt(3);
        idx = 2;
        price = &mPrices[idx];
        rand2 = 7 - rand1 - rand3;
        for (; rand1 > 0; rand1--, price++, idx++) {
            *price = dSaveMoney_c(calcPrice(1.4f, 0.8f));
        }
        f32 rate = 0.8f;
        rate -= cM::rndF(0.2f);
        for (; fall1 > 0; fall1--, idx++) {
            mPrices[idx] = dSaveMoney_c((int)(rate * mBuyPrice.get() + 0.99999f));
            rate -= 0.04f;
            rate -= cM::rndF(0.06f);
        }
        for (; rand2 > 0; rand2--, idx++) {
            mPrices[idx] = dSaveMoney_c(calcPrice(1.4f, 0.8f));
        }
        rate = 0.8f;
        rate -= cM::rndF(0.2f);
        for (; fall2 > 0; fall2--, idx++) {
            mPrices[idx] = dSaveMoney_c((int)(rate * mBuyPrice.get() + 0.99999f));
            rate -= 0.04f;
            rate -= cM::rndF(0.06f);
        }
        for (; rand3 > 0; rand3--, idx++) {
            mPrices[idx] = dSaveMoney_c(calcPrice(1.4f, 0.8f));
        }
        break;
    }
    case STALK_MARKET_TREND_BIG_SPIKE: {
        mSpikeIdx = cM::rndInt(5) + 5;
        int i = 2;
        f32 rate = 0.8f;
        rate -= cM::rndF(0.05f);
        for (; i < mSpikeIdx; i++) {
            mPrices[i] = dSaveMoney_c((int)(rate * mBuyPrice.get() + 0.99999f));
            rate -= 0.03f;
            rate -= cM::rndF(0.02f);
        }
        mPrices[i] = dSaveMoney_c(calcPrice(1.4f, 0.8f));
        mPrices[i + 1] = dSaveMoney_c(calcPrice(2.0f, 1.4f));
        mPrices[i + 2] = dSaveMoney_c(calcPrice(6.0f, 2.0f));
        mPrices[i + 3] = dSaveMoney_c(calcPrice(2.0f, 1.4f));
        mPrices[i + 4] = dSaveMoney_c(calcPrice(1.4f, 0.8f));
        i += 5;
        rate = 0.8f;
        rate -= cM::rndF(0.05f);
        for (; i < STALK_MARKET_PRICE_NUM; i++) {
            mPrices[i] = dSaveMoney_c((int)(rate * mBuyPrice.get() + 0.99999f));
            rate -= 0.03f;
            rate -= cM::rndF(0.02f);
        }
        break;
    }
    case STALK_MARKET_TREND_DECREASING: {
        f32 rate = 0.8f;
        rate -= cM::rndF(0.05f);
        for (int i = 2; i < STALK_MARKET_PRICE_NUM; i++) {
            mPrices[i] = dSaveMoney_c((int)(rate * mBuyPrice.get() + 0.99999f));
            rate -= 0.03f;
            rate -= cM::rndF(0.02f);
        }
        break;
    }
    case STALK_MARKET_TREND_SMALL_SPIKE: {
        mSpikeIdx = cM::rndInt(5) + 5;
        int i = 2;
        f32 rate = 0.8f;
        rate -= cM::rndF(0.05f);
        for (; i < mSpikeIdx; i++) {
            mPrices[i] = dSaveMoney_c((int)(rate * mBuyPrice.get() + 0.99999f));
            rate -= 0.03f;
            rate -= cM::rndF(0.02f);
        }
        mPrices[i] = dSaveMoney_c(calcPrice(1.4f, 0.8f));
        mPrices[i + 1] = dSaveMoney_c(calcPrice(1.4f, 0.8f));
        f32 peak = 1.4f + cM::rndF(0.6f);
        mPrices[i + 3] = dSaveMoney_c((int)(peak * mBuyPrice.get() + 0.99999f));
        mPrices[i + 2] = dSaveMoney_c(calcPrice(peak, 1.4f));
        mPrices[i + 4] = dSaveMoney_c(calcPrice(peak, 1.4f));
        i += 5;
        rate = 0.8f;
        rate -= cM::rndF(0.05f);
        for (; i < STALK_MARKET_PRICE_NUM; i++) {
            mPrices[i] = dSaveMoney_c((int)(rate * mBuyPrice.get() + 0.99999f));
            rate -= 0.03f;
            rate -= cM::rndF(0.02f);
        }
        break;
    }
    }
}

// 80147984
int dSaveStalkMarket_c::calcPrice(f32 max, f32 min) {
    f32 rate = min + cM::rndF(max - min);
    int base = mBuyPrice.get();
    return base * rate + 0.99999f;
}

// 80147A08
void dSaveStalkMarket_c::invalidate() {
    fn_80116510(dSaveData_c::getTown(), SAVE_FLAG_TURNIPS_SPOILED);
    fn_8014CCC4(&mDate, TRUE);
}
